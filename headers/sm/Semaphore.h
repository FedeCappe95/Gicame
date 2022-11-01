#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__


#include <string>
#include <stdexcept>
#include <stdint.h>
#include "../common.h"


namespace Gicame {

	class Semaphore {

	private:
#ifdef WINDOWS
		void* handle;
#endif

	public:
		GICAME_API Semaphore(const std::string& name, const size_t maxConcurrency, const size_t initialLockCount = 0u);
		GICAME_API ~Semaphore();
		GICAME_API void acquire();
		GICAME_API void release();

	};

};


#endif
