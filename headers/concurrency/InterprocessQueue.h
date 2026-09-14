#ifndef __GICAME__CONCURRENCY__INTERPROCESSQUEUE_H__
#define __GICAME__CONCURRENCY__INTERPROCESSQUEUE_H__


#include "../common.h"
#include "../configuration.h"
#include "../utils/NotCopyable.h"
#include "../sm/SharedMemory.h"
#include "./InterprocessSignal.h"
#include "./ByteRing.h"
#include <string>


namespace Gicame::Concurrency::Impl {

	class IntPrQueueLockPolicy {

	private:
		InterprocessSignal dataPresentEvent;
		InterprocessSignal dataFreeEvent;

	public:
		IntPrQueueLockPolicy(const std::string& name, ConcurrencyRole cr) :
			dataPresentEvent(std::string("iq_dataPresentEvent_") + name, cr),
			dataFreeEvent(std::string("iq_dataFreeEvent_") + name, cr)
		{}
		void waitDataPresent() { dataPresentEvent.wait(); }
		void waitDataFree() { dataFreeEvent.wait(); }
		void signalDataPresent() { dataPresentEvent.signal(); }
		void signalDataFree() { dataFreeEvent.signal(); }
	};


	class SLIntPrQueueLockPolicy {

	public:
		SLIntPrQueueLockPolicy() noexcept {}
		void waitDataPresent() const noexcept {}
		void waitDataFree() const noexcept {}
		void signalDataPresent() const noexcept {}
		void signalDataFree() const noexcept {}

	};

};


namespace Gicame::Concurrency {

	
	/**
	 * @brief A byte-wise ring based interprocess queue.
	 * 
	 * Sync done via InterprocessSignal
	 */
	class InterprocessQueue : public ByteRing<Impl::IntPrQueueLockPolicy> {

		NOT_COPYABLE(InterprocessQueue)

	private:
		Gicame::SharedMemory shmem;

	public:
		GICAME_API InterprocessQueue(const std::string& name, const size_t capacity, const ConcurrencyRole cr);
		GICAME_API ~InterprocessQueue();

	};


	/**
	 * @brief A byte-wise ring based interprocess queue.
	 *
	 * Sync done via spinlocks.
	 * Spinlock in userspace are evil. Don't use them unless you -really- know what you're doing.
	 */
	class SLInterprocessQueue : public ByteRing<Impl::SLIntPrQueueLockPolicy> {

		NOT_COPYABLE(SLInterprocessQueue)

	private:
		Gicame::SharedMemory shmem;

	public:
		GICAME_API SLInterprocessQueue(const std::string& name, const size_t capacity, const ConcurrencyRole cr);
		GICAME_API ~SLInterprocessQueue();

	};

};

#endif
