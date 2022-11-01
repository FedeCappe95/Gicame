#ifndef __GICAME__SHAREDMEMORY_H__
#define __GICAME__SHAREDMEMORY_H__


#include <string>
#include <stdexcept>
#include <stdint.h>
#include "../common.h"


namespace Gicame {

	class SharedMemory {

	private:
		const std::string name;
		const size_t size;
		void* ptr;
#ifdef WINDOWS
		void* fileHandle;
#else
		int fd;
#endif

	public:
		GICAME_API SharedMemory(const std::string& name, const size_t size);
		GICAME_API ~SharedMemory();
		GICAME_API void create();
		GICAME_API void open();
		GICAME_API void close();
		GICAME_API void destroy();
		inline void* get() { return ptr; }
		inline size_t getSize() { return size; }
		template<class T>
		inline T* getAs() { return (T*)ptr; }

	};

};


#endif
