#include "sm/SharedMemory.h"


#ifdef WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif


#ifdef MSVC
#pragma comment(lib, "user32.lib")
#endif


using namespace Gicame;



SharedMemory::SharedMemory(const std::string& name, const size_t size) :
#ifdef WINDOWS
	name(std::string("Local\\") + name),
	fileHandle(NULL),
#else
	name(std::string("/") + name),
	fd(-1),
#endif
	size(size),
	ptr(NULL)
{}

void SharedMemory::create() {
	if (ptr)
		throw std::runtime_error(__FUNCTION__ ": SharedMemory already created or opened");

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
#error "NYI"
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
#error "NYI"
#endif
}

void SharedMemory::close() {
	if (!ptr)
		throw RUNTIME_ERROR("SharedMemory not created or opened");

#ifdef WINDOWS
	UnmapViewOfFile(ptr);
	CloseHandle(fileHandle);
#else
#error "NYI"
#endif
}
