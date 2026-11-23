#ifndef __GICAME__DYNLIBRARY_H__
#define __GICAME__DYNLIBRARY_H__


#include "../common.h"
#include "../utils/NotCopyable.h"
#include <string>
#include <type_traits>
#include <new>
#include <filesystem>


namespace Gicame::Os {

	class DynLibraryHandler {

		NOT_COPYABLE(DynLibraryHandler)

	private:   // Private types
		using NativeHandle = void*;

	private:   // Private data memebers
		NativeHandle nativeHandle;
		bool unloadOnDestruction;

	private:   // Private methods
		[[noreturn]] void xLibraryNotLoaded();
		
	public:    // Public methods
		GICAME_API DynLibraryHandler() noexcept;
		GICAME_API DynLibraryHandler(DynLibraryHandler&& other) noexcept;
		GICAME_API DynLibraryHandler& operator=(DynLibraryHandler&& other) noexcept;
		GICAME_API ~DynLibraryHandler();

		/**
		 * @brief Load the library (throws if one is already loaded)
		 * @return false if the system cannot load it
		 */
		GICAME_API bool load(const std::filesystem::path& modulePath);

		GICAME_API bool isLoaded() const noexcept;

		/**
		 * @brief Unload the library now (nothing happens if it is not loaded)
		 *
		 * Symbols taken from it are no longer valid afterwards.
		 */
		GICAME_API void unload() noexcept;

		GICAME_API void* getSymbolAddress(const char* symbolName);

		/**
		 * @brief Set whether the library is unloaded on destruction (default: false)
		 *
		 * Unloading invalidates the symbols taken from the library: enable it only when none of
		 * them outlives this object.
		 */
		GICAME_API void setUnloadOnDestruction(bool uod) noexcept;

		template <typename T, typename = std::enable_if_t<std::is_pointer_v<T>>>
		inline T getSymbol(const char* symbolName) {
			return std::launder(reinterpret_cast<T>(getSymbolAddress(symbolName)));
		}

	};

};


#endif
