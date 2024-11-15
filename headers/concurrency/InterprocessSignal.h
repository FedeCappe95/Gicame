#ifndef __GICAME__CONCURRENCY__INTERPROCESSSIGNAL_H__
#define __GICAME__CONCURRENCY__INTERPROCESSSIGNAL_H__


#include "../common.h"
#include "../utils/NotCopyable.h"
#include "./ConcurrencyRole.h"
#include "./WaitResult.h"
#include "../sm/SharedMemory.h"
#include <string>


namespace Gicame::Concurrency {

	class InterprocessSignal {

		NOT_COPYABLE(InterprocessSignal)

	private:
#ifdef WINDOWS
		void* eventHandle;
#else
		Gicame::SharedMemory shmem;
#endif

	public:
		GICAME_API InterprocessSignal(const std::string& name, const ConcurrencyRole cr);
		GICAME_API ~InterprocessSignal();
		GICAME_API WaitResult wait();
		GICAME_API void signal();

	};

};

#endif
