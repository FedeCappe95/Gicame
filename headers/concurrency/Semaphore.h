#ifndef __GICAME__SEMAPHORE_H__
#define __GICAME__SEMAPHORE_H__


#include <string>
#include <stdexcept>
#include <stdint.h>
#include "../common.h"
#ifndef WINDOWS
#include <semaphore.h>
#endif



namespace Gicame {

	class Semaphore {

	private:
#ifdef WINDOWS
		void* handle;
#else
		sem_t* handle;
		const std::string posixName;
#endif
		bool unlinkOnDestruction;

	public:
		GICAME_API Semaphore(const std::string& name, const size_t maxConcurrency);
		GICAME_API ~Semaphore();
		GICAME_API bool acquire();
		GICAME_API bool release();
		GICAME_API void setUnlinkOnDestruction(bool uod);

	};

};


#endif
