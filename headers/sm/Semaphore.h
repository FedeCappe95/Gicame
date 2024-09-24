#ifndef __GICAME__SEMAPHORE_H__
#define __GICAME__SEMAPHORE_H__


#include <string>
#include <stdexcept>
#include <stdint.h>
#include "../common.h"


namespace Gicame {

	class Semaphore {

	private:
#if defined(WINDOWS) || !defined(GICAME_EXPORTS)
		void* handle;
#else
		sem_t* handle;
#endif
#ifndef WINDOWS
		const std::string posixName;
#endif
		bool unlinkOnDestruction;

	public:
		GICAME_API Semaphore(const std::string& name, const size_t maxConcurrency, const size_t initialLockCount = 0u);
		GICAME_API ~Semaphore();
		GICAME_API bool acquire();
		GICAME_API bool release();
		GICAME_API void setUnlinkOnDestruction(bool uod);

	};

};


#endif
