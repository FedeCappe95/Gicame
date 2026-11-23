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

void DynLibraryHandler::unload() noexcept
{
	if (nativeHandle == nullptr)
		return;

#ifdef WINDOWS
	FreeLibrary(reinterpret_cast<HINSTANCE>(nativeHandle));
#else
	dlclose(nativeHandle);
#endif
	nativeHandle = nullptr;
}

DynLibraryHandler::DynLibraryHandler() noexcept :
	nativeHandle(nullptr),
	unloadOnDestruction(false)
{}

DynLibraryHandler::DynLibraryHandler(DynLibraryHandler&& other) noexcept :
	nativeHandle(other.nativeHandle),
	unloadOnDestruction(other.unloadOnDestruction)
{
	// The destructor keys off nativeHandle, so clearing it is what makes the source inert.
	other.nativeHandle = nullptr;
}

DynLibraryHandler& DynLibraryHandler::operator=(DynLibraryHandler&& other) noexcept {
	if (this != &other) {
		// The library held so far is let go as the destructor would do.
		if (unloadOnDestruction && isLoaded())
			unload();
		nativeHandle = other.nativeHandle;
		unloadOnDestruction = other.unloadOnDestruction;
		other.nativeHandle = nullptr;
	}
	return *this;
}

DynLibraryHandler::~DynLibraryHandler() {
	if (unloadOnDestruction && isLoaded())
		unload();
}

bool DynLibraryHandler::load(const std::filesystem::path& modulePath)
{
	if (isLoaded())
		throw RUNTIME_ERROR("Library already loaded");

	// WIP: must prevent to load the same lib twice, or, once unloaded, bad things may happen

#ifdef WINDOWS
	HINSTANCE libHandle = LoadLibraryW(modulePath.wstring().c_str());
#else
	void* libHandle = dlopen(modulePath.string().c_str(), RTLD_NOW);
#endif

	if (!libHandle)
		return false;
	
	nativeHandle = libHandle;
	return true;
}

bool DynLibraryHandler::isLoaded() const noexcept {
	return nativeHandle != nullptr;
}

void* DynLibraryHandler::getSymbolAddress(const char* symbolName) {
	if (!nativeHandle)
		xLibraryNotLoaded();

#ifdef WINDOWS
	return GetProcAddress(reinterpret_cast<HINSTANCE>(nativeHandle), symbolName);
#else
	return dlsym(nativeHandle, symbolName);
#endif
}

void DynLibraryHandler::setUnloadOnDestruction(bool uod) noexcept {
	unloadOnDestruction = uod;
}
