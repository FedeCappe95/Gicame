#ifndef __GICAME__CONCURRENCY__BYTERING_H__
#define __GICAME__CONCURRENCY__BYTERING_H__


#include "../common.h"
#include "../configuration.h"
#include "../utils/NotCopyable.h"
#include "../interfaces/IDataExchanger.h"
#include "./IDataExchangerQueueAdapter.h"
#include "./ConcurrencyRole.h"
#include "./implementation_details/CircularBuffer.h"
#include <atomic>
#include <cstring>
#include <utility>


namespace Gicame::Concurrency {

	/**
	 * @brief A byte-wise single producer single consumer ring
	 */
	template <typename WaitPolicy>
	class ByteRing : public IDataExchangerQueueAdapter<ByteRing<WaitPolicy>> {

		NOT_COPYABLE(ByteRing)

	protected:
		Impl::CircularBufferDescriptor* header;
		uint8_t* buffer;
		size_t capacity;
		ipc_size_t cachedHead;
		ipc_size_t cachedTail;
		WaitPolicy waiter;

	protected:
		void waitElemPresent(const size_t dataSize);
		void waitFreeSpace(const size_t dataSize);
		ByteRing(WaitPolicy&& waiter_);
		void initRing(
			Impl::CircularBufferDescriptor* header_, uint8_t* buffer_, size_t capacity_,
			ConcurrencyRole cr
		);

	public:
		virtual ~ByteRing() = default;
		void push(const void* data, size_t dataSize);
		void pop(void* outBuffer, size_t dataSize);
		size_t size() const noexcept;
		size_t freeSpace() const noexcept;

	};


	/*
	 * Inline implementation
	 */

	template <typename WaitPolicy>
	inline void ByteRing<WaitPolicy>::waitElemPresent(const size_t dataSize) {
		// Consumer view: tail is ours, head comes from the snapshot. A stale snapshot can only
		// under-report how much data is available, because the producer only ever advances head.
		const ipc_size_t t = header->tail.load(std::memory_order_relaxed);
		if (Impl::circularBufferSize(capacity, cachedHead, t) >= dataSize)
			return;

		// cachedHead may be too old or not updated yet
		cachedHead = header->head.load(std::memory_order_acquire);
		while (Impl::circularBufferSize(capacity, cachedHead, t) < dataSize) {
			waiter.waitDataPresent();
			cachedHead = header->head.load(std::memory_order_acquire);
		}
	}

	template <typename WaitPolicy>
	inline void ByteRing<WaitPolicy>::waitFreeSpace(const size_t dataSize) {
		// Producer view: head is ours, tail comes from the snapshot. Symmetrically, a stale snapshot
		// can only under-report the free space.
		const ipc_size_t h = header->head.load(std::memory_order_relaxed);
		if (Impl::circularBufferFreeSpace(capacity, h, cachedTail) >= dataSize)
			return;

		cachedTail = header->tail.load(std::memory_order_acquire);
		while (Impl::circularBufferFreeSpace(capacity, h, cachedTail) < dataSize) {
			waiter.waitDataFree();
			cachedTail = header->tail.load(std::memory_order_acquire);
		}
	}

	template <typename WaitPolicy>
	ByteRing<WaitPolicy>::ByteRing(WaitPolicy&& waiter_) :
		header(nullptr),
		buffer(nullptr),
		capacity(0u),
		cachedHead(0u),
		cachedTail(0u),
		waiter(std::move(waiter_))
	{}

	template <typename WaitPolicy>
	void ByteRing<WaitPolicy>::initRing(
		Impl::CircularBufferDescriptor* header_, uint8_t* buffer_, size_t capacity_,
		ConcurrencyRole cr
	) {
		constexpr ipc_size_t maxCapacity = ~ipc_size_t(0);
		if (capacity_ > maxCapacity)
			throw RUNTIME_ERROR("Capacity too big");
		if (capacity_ < 2u)
			throw RUNTIME_ERROR("Insufficient capacity");

		header = header_;
		buffer = buffer_;
		capacity = capacity_;

		if (cr == ConcurrencyRole::MASTER) {
			header->head = cachedHead = 0u;
			header->tail = cachedTail = 0u;
		}
		else {
			cachedHead = header->head.load(std::memory_order_acquire);
			cachedTail = header->tail.load(std::memory_order_acquire);
		}
	}

	template <typename WaitPolicy>
	inline void ByteRing<WaitPolicy>::push(const void* data, size_t dataSize) {
		const uint8_t* ptr = static_cast<const uint8_t*>(data);

		while (dataSize) {
			const size_t chunkSize = likely(dataSize < (capacity - 1u)) ? dataSize : (capacity - 1u);

			waitFreeSpace(chunkSize);

			// Producer-private index: nobody else writes head, so no ordering is needed to read it.
			const ipc_size_t h = header->head.load(std::memory_order_relaxed);

			// At most one wrap: fill up to the end of the ring, then the remainder from its start.
			const size_t avail = capacity - h;
			const size_t first = (chunkSize < avail) ? chunkSize : avail;

			std::memcpy(buffer + h, ptr, first);
			if (first != chunkSize)
				std::memcpy(buffer, ptr + first, chunkSize - first);

			// h <= capacity - 1 and chunkSize <= capacity - 1, so h + chunkSize < 2 * capacity:
			// a single conditional subtraction does what the modulo used to do.
			size_t nextHead = static_cast<size_t>(h) + chunkSize;
			if (nextHead >= capacity)
				nextHead -= capacity;

			// Release: the bytes written above must be visible to whoever acquires head.
			header->head.store(static_cast<ipc_size_t>(nextHead), std::memory_order_release);

			waiter.signalDataPresent();

			dataSize -= chunkSize;
			ptr = ptr + chunkSize;
		}
	}

	template <typename WaitPolicy>
	inline void ByteRing<WaitPolicy>::pop(void* outBuffer, size_t dataSize) {
		uint8_t* ptr = static_cast<uint8_t*>(outBuffer);

		while (dataSize) {
			const size_t chunkSize = likely(dataSize < (capacity - 1u)) ? dataSize : (capacity - 1u);

			waitElemPresent(chunkSize);

			// Consumer-private index: nobody else writes tail, so no ordering is needed to read it.
			const ipc_size_t t = header->tail.load(std::memory_order_relaxed);

			// At most one wrap: drain up to the end of the ring, then the remainder from its start.
			const size_t avail = capacity - t;
			const size_t first = (chunkSize < avail) ? chunkSize : avail;

			std::memcpy(ptr, buffer + t, first);
			if (first != chunkSize)
				std::memcpy(ptr + first, buffer, chunkSize - first);

			// t <= capacity - 1 and chunkSize <= capacity - 1, so t + chunkSize < 2 * capacity:
			// a single conditional subtraction does what the modulo used to do.
			size_t nextTail = static_cast<size_t>(t) + chunkSize;
			if (nextTail >= capacity)
				nextTail -= capacity;

			// Release: the reads above must complete before the producer is told the space is free.
			header->tail.store(static_cast<ipc_size_t>(nextTail), std::memory_order_release);

			waiter.signalDataFree();

			dataSize -= chunkSize;
			ptr += chunkSize;
		}
	}

	template <typename WaitPolicy>
	inline size_t ByteRing<WaitPolicy>::size() const noexcept {
		// Acquire on both: this function is used by producer and consumer alike, and each one
		// has to see the other's index together with the data published before it.
		const ipc_size_t h = header->head.load(std::memory_order_acquire);
		const ipc_size_t t = header->tail.load(std::memory_order_acquire);
		return Impl::circularBufferSize(capacity, h, t);
	}

	template <typename WaitPolicy>
	inline size_t ByteRing<WaitPolicy>::freeSpace() const noexcept {
		// Acquire on both: this function is used by producer and consumer alike, and each one
		// has to see the other's index together with the data published before it.
		const ipc_size_t h = header->head.load(std::memory_order_acquire);
		const ipc_size_t t = header->tail.load(std::memory_order_acquire);
		return Impl::circularBufferFreeSpace(capacity, h, t);
	}

};

#endif
