#ifndef __SHAREDMEMORY_H__
#define __SHAREDMEMORY_H__


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
		int fd
#endif

	public:
		SharedMemory(const std::string& name, const size_t size);
		void create();
		void open();
		void close();
		inline void* get() { return ptr; }
		inline size_t getSize() { return size; }
		template<class T>
		inline T* getAs() { return (T*)ptr; }

	};

};


#endif