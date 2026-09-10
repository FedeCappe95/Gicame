#ifndef __GICAME__CONCURRENCY__INTERPROCESSQUEUE_H__
#define __GICAME__CONCURRENCY__INTERPROCESSQUEUE_H__


#include "../common.h"
#include "../utils/NotCopyable.h"
#include "../sm/SharedMemory.h"
#include "../interfaces/IDataExchanger.h"
#include "./InterprocessSignal.h"
#include <string>


namespace Gicame::Concurrency::Impl {
	struct CircularBufferDescriptor;
};


namespace Gicame::Concurrency {

	template <typename Queue>
	class IDataExchangerQueueAdapter : public IDataExchanger {

	public:
		virtual size_t send(const void* data, const size_t dataSize) override final;
		virtual bool isSenderConnected() const override final;
		virtual size_t receive(void* outBuffer, const size_t dataSize) override final;
		virtual bool isReceiverConnected() const override final;

	};

	/**
	 * @brief A byte-wise ring based interprocess queue.
	 * 
	 * Sync done via InterprocessSignal
	 */
	class InterprocessQueue : public IDataExchangerQueueAdapter<InterprocessQueue> {

		NOT_COPYABLE(InterprocessQueue)

	private:
		Gicame::Concurrency::Impl::CircularBufferDescriptor* header;
		uint8_t* buffer;
		size_t capacity;
		Gicame::SharedMemory shmem;
		InterprocessSignal dataPresentEvent;
		InterprocessSignal dataFreeEvent;

	private:
		void waitElemPresent(const size_t dataSize);
		void waitFreeSpace(const size_t dataSize);

	public:
		GICAME_API InterprocessQueue(const std::string& name, const size_t capacity, const ConcurrencyRole cr);
		GICAME_API ~InterprocessQueue();
		GICAME_API void push(const void* data, size_t dataSize);
		GICAME_API void pop(void* outBuffer, size_t dataSize);
		GICAME_API size_t size() const noexcept;
		GICAME_API size_t freeSpace() const noexcept;

	};


	/**
	 * @brief A byte-wise ring based interprocess queue.
	 *
	 * Sync done via spinlocks.
	 * Spinlock in userspace are evil. Don't use them unless you -really- know what you're doing.
	 */
	class SLInterprocessQueue : public IDataExchangerQueueAdapter<SLInterprocessQueue> {

		NOT_COPYABLE(SLInterprocessQueue)

	private:
		Gicame::Concurrency::Impl::CircularBufferDescriptor* header;
		uint8_t* buffer;
		size_t capacity;
		Gicame::SharedMemory shmem;

	private:
		void waitElemPresent(const size_t dataSize);
		void waitFreeSpace(const size_t dataSize);

	public:
		GICAME_API SLInterprocessQueue(const std::string& name, const size_t capacity, const ConcurrencyRole cr);
		GICAME_API ~SLInterprocessQueue();
		GICAME_API void push(const void* data, size_t dataSize);
		GICAME_API void pop(void* outBuffer, size_t dataSize);
		GICAME_API size_t size() const noexcept;
		GICAME_API size_t freeSpace() const noexcept;

	};


	/*
	 * Inline implementation
	 */

	template <typename Queue>
	inline size_t IDataExchangerQueueAdapter<Queue>::send(const void* data, const size_t dataSize) {
		dynamic_cast<Queue*>(this)->push(data, dataSize);
		return dataSize;
	}

	template <typename Queue>
	inline bool IDataExchangerQueueAdapter<Queue>::isSenderConnected() const { return true; }

	template <typename Queue>
	inline size_t IDataExchangerQueueAdapter<Queue>::receive(void* outBuffer, const size_t dataSize) {
		dynamic_cast<Queue*>(this)->pop(outBuffer, dataSize);
		return dataSize;
	}

	template <typename Queue>
	inline bool IDataExchangerQueueAdapter<Queue>::isReceiverConnected() const { return true; }

};

#endif
