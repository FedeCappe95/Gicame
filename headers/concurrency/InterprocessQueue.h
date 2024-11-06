#ifndef __GICAME__CONCURRENCY__INTERPROCESSQUEUE_H__
#define __GICAME__CONCURRENCY__INTERPROCESSQUEUE_H__


#include "../common.h"
#include "../utils/NotCopyable.h"
#include "./SPSCQueue.h"
#include "../sm/Semaphore.h"
#include "../sm/SharedMemory.h"
#include <string>


namespace Gicame::Concurrency::Impl {
	struct SPSCQueueMeta;
};


// Help-functions usefull in some classes
namespace Gicame::Concurrency {

	/**
	 * @brief Implementation of InterprocessQueue
	 */
	class InterprocessQueue {

		NOT_COPYABLE(InterprocessQueue)

	private:
		const size_t capacity;
		Gicame::SharedMemory shmem;
		Gicame::Semaphore sem;
#ifdef WINDOWS
		void* dataPresentEvent;
		void* dataFreeEvent;
		void* mutex;
#endif

	private:
		void waitElemPresent(const size_t dataSize);
		void waitFreeSpace(const size_t dataSize);
		void notifyElemPresent();
		void notifyFreeSpace();
		Gicame::Concurrency::Impl::SPSCQueueMeta* getHeader();
		uint8_t* getBuffer();

	public:
		GICAME_API InterprocessQueue(const std::string& name, const size_t capacity, const ConcurrencyRole cr);
		GICAME_API ~InterprocessQueue();
		GICAME_API void push(const void* data, const size_t dataSize);
		GICAME_API void pop(void* outBuffer, const size_t dataSize);
		GICAME_API size_t size();
		GICAME_API size_t freeSpace();

	};

};

#endif
