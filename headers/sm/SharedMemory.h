#ifndef __GICAME__SHAREDMEMORY_H__
#define __GICAME__SHAREDMEMORY_H__


#include <stdint.h>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <new>
#include "../common.h"
#include "../utils/Memory.h"


namespace Gicame {

	class SharedMemory {

	private:
		const std::string name;
		const size_t size;
		void* ptr;
#ifdef WINDOWS
		void* fileHandle;
#endif
		bool unlinkOnDestruction;

	public:
		/**
		 * @brief Whether the operating system accepts name for a shared memory
		 *
		 * The rules are the system ones (not empty, no '/' on POSIX, no '\' on Windows, a length
		 * limit). The constructor throws for a name that fails this check.
		 */
		GICAME_API static bool isValidName(const std::string& name);

		GICAME_API SharedMemory(const std::string& name, const size_t size);
		GICAME_API SharedMemory(SharedMemory&& other);
		GICAME_API ~SharedMemory();
		GICAME_API void close();
		/**
		 * @brief Map the memory, creating it first if createIfNotExisting
		 *
		 * Opening without creating never resizes the object and fails if it is smaller than the
		 * size given to the constructor.
		 */
		GICAME_API bool open(const bool createIfNotExisting);
		/**
		 * @brief Close and remove the name, so nobody else can open it (no-op for the name on
		 *        Windows, where the object goes away with its last handle)
		 */
		GICAME_API void unlink();
		GICAME_API void setUnlinkOnDestruction(bool uod);
		inline void* get() { return ptr; }
		inline size_t getSize() { return size; }

		/**
		 * @brief Return the memory area as a pointer to type T
		 *
		 * @param construct If true, the resulting pointer points to an object
		 *                  of type T whose life cicle begins with a new, so
		 *                  it is fully initialized. If false, the user
		 *                  assumes the memory area contains an already
		 *                  initialized object. Please, be extremely cautious
		 *                  about setting this parameter to false.
		 * @return A pointer to an object of type T in case of success, NULL
		 *         otherwise.
		 */
		template<class T>
		inline T* getAs(const bool construct) {
			// Virtual functions or virtual bases would put process-local pointers in the object.
			static_assert(std::is_standard_layout_v<T>, "T must be standard-layout (e.g. no virtual functions or bases)");
			auto [objPtr, resultingSize] = Gicame::Utilities::align<T>(ptr, size);
			if (!objPtr)
				return NULL;
			if (construct)
				return new (objPtr) T;
			return std::launder(reinterpret_cast<T*>(objPtr));
		}

	};

};


#endif
