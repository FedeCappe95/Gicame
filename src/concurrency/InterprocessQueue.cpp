#include "concurrency/InterprocessQueue.h"
#include "concurrency/implementation_details/CircularBuffer.h"
#include "utils/Memory.h"
#include <atomic>
#include <string>
#include <new>
#include <cstddef>
#include <cstring>
#include <algorithm>


using namespace Gicame;
using namespace Gicame::Concurrency;
using namespace Gicame::Concurrency::Impl;


// Note: code is duplicated. It will be fixed soon.


/*
 * ===== InterprocessQueue implementation =====
 */

void InterprocessQueue::waitElemPresent(const size_t dataSize) {
	size_t present = size();
	while (present < dataSize) {
		dataPresentEvent.wait();
		present = size();
	}
}

void InterprocessQueue::waitFreeSpace(const size_t dataSize) {
	size_t free = freeSpace();
	while (free < dataSize) {
		dataFreeEvent.wait();
		free = freeSpace();
	}
}

InterprocessQueue::InterprocessQueue(const std::string& name, const size_t capacity_, const ConcurrencyRole cr) :
	header(NULL),
	buffer(NULL),
	capacity(0),
	shmem(std::string("iq_shmem_") + name, capacity_ + sizeof(Gicame::Concurrency::Impl::CircularBufferDescriptor) + alignof(Gicame::Concurrency::Impl::CircularBufferDescriptor)),
	dataPresentEvent(std::string("iq_dataPresentEvent_") + name, cr),
	dataFreeEvent(std::string("iq_dataFreeEvent_") + name, cr)
{
	constexpr ipc_size_t maxCapacity = ~ipc_size_t(0);
	if (capacity_ > maxCapacity)
		throw RUNTIME_ERROR("Capacity too big");

	const bool success = shmem.open(cr == ConcurrencyRole::MASTER);
	if (!success)
		throw RUNTIME_ERROR("Unable to open shared memory");

	const auto[memPtr, newSize] = Utilities::align<CircularBufferDescriptor>(shmem.get(), shmem.getSize());
	if (!memPtr)
		throw RUNTIME_ERROR("Insufficient capacity");

	header = new (memPtr) CircularBufferDescriptor;
	buffer = Utilities::advance<uint8_t>(memPtr, sizeof(CircularBufferDescriptor));
	capacity = newSize - sizeof(CircularBufferDescriptor);
	capacity = std::min(capacity, capacity_);  // not to go over capacity_
	if (capacity < 2u)
		throw RUNTIME_ERROR("Insufficient capacity");

	if (cr == ConcurrencyRole::MASTER) {
		header->head = 0;
		header->tail = 0;
	}
}

InterprocessQueue::~InterprocessQueue() {}

void InterprocessQueue::push(const void* data, size_t dataSize) {
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

		dataPresentEvent.signal();

		dataSize -= chunkSize;
		ptr = ptr + chunkSize;
	}
}

void InterprocessQueue::pop(void* outBuffer, size_t dataSize) {
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

		dataFreeEvent.signal();

		dataSize -= chunkSize;
		ptr += chunkSize;
	}
}

size_t InterprocessQueue::size() const noexcept {
	// Acquire on both: size() is used by producer and consumer alike, and each one has to see
	// the other's index together with the data published before it.
	const ipc_size_t h = header->head.load(std::memory_order_acquire);
	const ipc_size_t t = header->tail.load(std::memory_order_acquire);
	if (h >= t)
		return static_cast<size_t>(h - t);
	else
		return static_cast<size_t>(capacity - (t - h));
}

size_t InterprocessQueue::freeSpace() const noexcept {
	return capacity - size() - 1u;
}


/*
 * ===== SLInterprocessQueue implementation =====
 */

void SLInterprocessQueue::waitElemPresent(const size_t dataSize) {
	size_t present = size();
	while (present < dataSize)
		present = size();
}

void SLInterprocessQueue::waitFreeSpace(const size_t dataSize) {
	size_t free = freeSpace();
	while (free < dataSize)
		free = freeSpace();
}

SLInterprocessQueue::SLInterprocessQueue(const std::string& name, const size_t capacity_, const ConcurrencyRole cr) :
	header(NULL),
	buffer(NULL),
	capacity(0),
	shmem(std::string("iq_shmem_") + name, capacity_ + sizeof(Gicame::Concurrency::Impl::CircularBufferDescriptor) + alignof(Gicame::Concurrency::Impl::CircularBufferDescriptor))
{
	constexpr ipc_size_t maxCapacity = ~ipc_size_t(0);
	if (capacity_ > maxCapacity)
		throw RUNTIME_ERROR("Capacity too big");

	const bool success = shmem.open(cr == ConcurrencyRole::MASTER);
	if (!success)
		throw RUNTIME_ERROR("Unable to open shared memory");

	const auto [memPtr, newSize] = Utilities::align<CircularBufferDescriptor>(shmem.get(), shmem.getSize());
	if (!memPtr)
		throw RUNTIME_ERROR("Insufficient capacity");

	header = new (memPtr) CircularBufferDescriptor;
	buffer = Utilities::advance<uint8_t>(memPtr, sizeof(CircularBufferDescriptor));
	capacity = newSize - sizeof(CircularBufferDescriptor);
	capacity = std::min(capacity, capacity_);  // not to go over capacity_
	if (capacity < 2u)
		throw RUNTIME_ERROR("Insufficient capacity");

	if (cr == ConcurrencyRole::MASTER) {
		header->head = 0;
		header->tail = 0;
	}
}

SLInterprocessQueue::~SLInterprocessQueue() {}

void SLInterprocessQueue::push(const void* data, size_t dataSize) {
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

		dataSize -= chunkSize;
		ptr = ptr + chunkSize;
	}
}

void SLInterprocessQueue::pop(void* outBuffer, size_t dataSize) {
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

		dataSize -= chunkSize;
		ptr += chunkSize;
	}
}

size_t SLInterprocessQueue::size() const noexcept {
	// Acquire on both: size() is used by producer and consumer alike, and each one has to see
	// the other's index together with the data published before it.
	const ipc_size_t h = header->head.load(std::memory_order_acquire);
	const ipc_size_t t = header->tail.load(std::memory_order_acquire);
	if (h >= t)
		return static_cast<size_t>(h - t);
	else
		return static_cast<size_t>(capacity - (t - h));
}

size_t SLInterprocessQueue::freeSpace() const noexcept {
	return capacity - size() - 1u;
}
