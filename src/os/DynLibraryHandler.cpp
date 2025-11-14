#include "os/DynLibraryHandler.h"
#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <dlfcn.h>
#endif


using namespace Gicame::Os;


void DynLibraryHandler::xLibraryNotLoaded() {
	throw RUNTIME_ERROR("Library not loaded");
}

void DynLibraryHandler::unload()
{
#ifdef WINDOWS
	if (nativeHandle) FreeLibrary(reinterpret_cast<HINSTANCE>(nativeHandle));
#else
	if (nativeHandle) dlclose(nativeHandle);
#endif
	nativeHandle = NULL;
}

DynLibraryHandler::~DynLibraryHandler() {
	unload();
}

DynLibraryHandler DynLibraryHandler::load(const std::filesystem::path& modulePath)
{
	// WIP: must prevent to load the same lib twice, or, once unloaded, bad things may happen

#ifdef WINDOWS
	HINSTANCE libHandle = LoadLibraryW(modulePath.wstring().c_str());
#else
	void* libHandle = dlopen(modulePath.string().c_str(), RTLD_NOW);
#endif

	if (!libHandle)
		throw RUNTIME_ERROR("Unable to load library");   // WIP: use a better exc
	
	return DynLibraryHandler(libHandle);
}

void* DynLibraryHandler::getSymbolAddress(const char* symbolName) {
	if (!nativeHandle)
		xLibraryNotLoaded();

#ifdef WINDOWS
	return GetProcAddress(reinterpret_cast<HINSTANCE>(nativeHandle), symbolName);
#else
	return dlsym(nativeHandler, symbolName);
#endif
}
