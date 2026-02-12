#ifndef __GICAME__CONCURRENCY__INTERPROCESSSIGNAL_H__
#define __GICAME__CONCURRENCY__INTERPROCESSSIGNAL_H__


#include "../common.h"
#include "../configuration.h"
#include "../utils/NotCopyable.h"
#include "./ConcurrencyRole.h"
#include "./WaitResult.h"
#include "../sm/SharedMemory.h"
#include <string>
#if defined(GICAME_USE_FUTEX)
#include <atomic>
#endif


namespace Gicame::Concurrency::Impl { struct PosixMutexCV; };


namespace Gicame::Concurrency {

	class InterprocessSignal {

		NOT_COPYABLE(InterprocessSignal)

	private:
#if defined(WINDOWS)
		void* eventHandle;
#elif defined(GICAME_USE_FUTEX)
		Gicame::SharedMemory shmem;
		std::atomic<uint32_t>* futexp;
#else
		Gicame::SharedMemory shmem;
		Impl::PosixMutexCV* sharedData;
#endif

	public:
		GICAME_API InterprocessSignal(const std::string& name, const ConcurrencyRole cr);
		GICAME_API ~InterprocessSignal();
		GICAME_API WaitResult wait();
		GICAME_API void signal();

	};

};

#endif
