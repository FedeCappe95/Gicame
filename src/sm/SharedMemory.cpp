#include "sm/SharedMemory.h"
#include "utils/Numbers.h"


#ifdef __CYGWIN__
#ifdef __POSIX_VISIBLE
#undef __POSIX_VISIBLE
#endif
#define __POSIX_VISIBLE 200112
#endif

#ifdef WINDOWS
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>        // For mode constants
#include <fcntl.h>           // For O_* constants
#include <unistd.h>
#include <sys/types.h>
#endif


#ifdef MSVC
#pragma comment(lib, "user32.lib")
#endif


using namespace Gicame;



SharedMemory::SharedMemory(const std::string& name, const size_t size) :
#ifdef WINDOWS
	name(std::string("Local\\") + name),
	size(size),
	ptr(NULL),
	fileHandle(NULL),
#else
	name(std::string("/") + name),
	size(size),
	ptr(NULL),
	fd(-1),
#endif
	unlinkOnDestruction(true)
{}

SharedMemory::~SharedMemory() {
	if (ptr) {
		close();
		if (unlinkOnDestruction)
			unlink();
	}
}

bool SharedMemory::open(const bool createIfNotExisting) {
	if (ptr)
		throw RUNTIME_ERROR("SharedMemory already open");

#ifdef WINDOWS

	if (createIfNotExisting) {
		const DWORD sizeAsDword = Gicame::Utilities::safeNumericCast<DWORD>(size);
		fileHandle = CreateFileMappingA(
			INVALID_HANDLE_VALUE,    // use paging file
			NULL,                    // default security
			PAGE_READWRITE,          // read/write access
			0,                       // maximum object size (high-order DWORD)
			sizeAsDword,             // maximum object size (low-order DWORD)
			name.c_str()             // name of mapping object
		);
		if (!fileHandle)
			return false;
	}
	else {
		fileHandle = OpenFileMappingA(
			FILE_MAP_ALL_ACCESS,   // read/write access
			FALSE,                 // do not inherit the name
			name.c_str()           // name of mapping object
		);

		if (!fileHandle)
			return false;
	}

	ptr = MapViewOfFile(
		fileHandle,           // handle to map object
		FILE_MAP_ALL_ACCESS,  // read/write permission
		0, 0, size
	);

	if (!ptr) {
		CloseHandle(fileHandle);
		return false;
	}

#else

	const int flags = createIfNotExisting ? (O_CREAT | O_RDWR) : (O_RDWR);
	fd = shm_open(name.c_str(), flags, 0666);
	if (unlikely(fd < 0))
		return false;

	if (unlikely(ftruncate(fd, size) < 0)) {
		shm_unlink(name.c_str());
		return false;
	}

	ptr = mmap(0, size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (unlikely(!ptr)) {
		shm_unlink(name.c_str());
		return false;
	}

#endif

	return true;
}

void SharedMemory::close() {
	if (!ptr)
		throw RUNTIME_ERROR("SharedMemory not created or opened");

#ifdef WINDOWS
	UnmapViewOfFile(ptr);
#else
	munmap(ptr, size);
#endif

	ptr = NULL;
}

void SharedMemory::unlink() {
	if (ptr)
		close();

#ifdef WINDOWS
	CloseHandle(fileHandle);
#else
	shm_unlink(name.c_str());
#endif
}

void SharedMemory::setUnlinkOnDestruction(bool uod) {
	unlinkOnDestruction = uod;
}
