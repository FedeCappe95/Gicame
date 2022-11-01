#include "sm/SharedMemory.h"


#ifdef WINDOWS
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>        // For mode constants
#include <fcntl.h>           // For O_* constants
#include <unistd.h>
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
	fileHandle(NULL)
#else
	name(std::string("/") + name),
	size(size),
	ptr(NULL),
	fd(-1)
#endif
{}

SharedMemory::~SharedMemory() {
	if (ptr)
		close();
}

void SharedMemory::create() {
	if (ptr)
		throw RUNTIME_ERROR("SharedMemory already created or opened");

#ifdef WINDOWS
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
		throw RUNTIME_ERROR("Could not create file mapping");

	ptr = MapViewOfFile(
		fileHandle,           // handle to map object
		FILE_MAP_ALL_ACCESS,  // read/write permission
		0, 0, size
	);

	if (!ptr) {
		CloseHandle(fileHandle);
		throw RUNTIME_ERROR("Could not map view of file");
	}
#else
	open();
#endif
}

void SharedMemory::open() {
	if (ptr)
		throw RUNTIME_ERROR("SharedMemory already created or opened");

#ifdef WINDOWS
	fileHandle = OpenFileMappingA(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		name.c_str()           // name of mapping object
	);

	if (!fileHandle)
		throw RUNTIME_ERROR("Could not open file mapping object");

	ptr = MapViewOfFile(
		fileHandle,           // handle to map object
		FILE_MAP_ALL_ACCESS,  // read/write permission
		0, 0, size
	);

	if (!ptr) {
		CloseHandle(fileHandle);
		throw RUNTIME_ERROR("Could not map view of file");
	}
#else
	fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
	if (unlikely(fd < 0)) {
		throw RUNTIME_ERROR("shm_open(...) failed");
	}

	if (unlikely(ftruncate(fd, size) < 0)) {
		shm_unlink(name.c_str());
		throw RUNTIME_ERROR("ftruncate(...) failed");
	}

	ptr = mmap(0, size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (unlikely(!ptr)) {
		shm_unlink(name.c_str());
		throw RUNTIME_ERROR("mmap(...) failed");
	}
#endif
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

void SharedMemory::destroy() {
	if (ptr)
		close();

#ifdef WINDOWS
	CloseHandle(fileHandle);
#else
	shm_unlink(name.c_str());
#endif
}
