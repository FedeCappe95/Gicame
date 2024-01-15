#ifndef __GICAME__COMMON_H__
#define __GICAME__COMMON_H__


/*
 * Notes on compilers
 *
 * Macro defined only by GCC:
 *   __GNUC__
 *   __GNUC_MINOR__
 *   __GNUC_PATCHLEVEL__
 *   __GNUG__  -->  Testing it is equivalent to testing (__GNUC__ && __cplusplus)
 * Macro defined only by clang:
 *   __clang__
 *   __clang_major__
 *   __clang_minor__
 *   __clang_patchlevel__
 *
 * Branch on compiler:
 *   #if defined(__clang__)
 *   ....
 *   #elif defined(__GNUC__) || defined(__GNUG__)
 *   ....
 *   #elif defined(_MSC_VER)
 *   ....
 *
 * Detect Windows:
 *   #ifdef _WIN32
 *   ...
 *   #endif
*/


// Includes
#include <stdint.h>
#include <stdexcept>
#include <limits.h>
#include <string>
//#include <type_traits>


// Type definitions
typedef uint8_t byte_t;
typedef void* ptr_t;


// Error message composition
namespace Gicame::Utilities {

    static inline std::string composeErrorMessage(const std::string& senderName, const std::string& message) {
        return senderName + std::string("(...): ") + message;
    }

}


// Macros
#if defined(_WIN32) && !defined(WINDOWS)
    #define WINDOWS
#endif

#if defined(_MSC_VER) && !defined(MSVC)
    #define MSVC
#endif

#ifdef __MINGW32__
    #define MINGW
#endif

#define RUNTIME_ERROR(MSG) std::runtime_error(Gicame::Utilities::composeErrorMessage(std::string(__FUNCTION__), (MSG)))

#define MOVABLE_BUT_NOT_COPYABLE Gicame::MovableButNonCopyable movableButNonCopyable

#if defined(_MSC_VER)
	#define likely(A) (A)
	#define unlikely(A) (A)
#else
	#define likely(A) __builtin_expect((A) != 0, 1)
	#define unlikely(A) __builtin_expect(bool(A), 0)
#endif

#ifndef GICAME_BUILD_STATIC

    #if defined(_MSC_VER) && defined(GICAME_EXPORTS)
    #define GICAME_API __declspec(dllexport)
    #elif defined(_MSC_VER) && !defined(GICAME_EXPORTS)
    #define GICAME_API __declspec(dllimport)
    #else
    #define GICAME_API
    #endif

    #if defined(_MSC_VER) && defined(GICAME_CRYPTO_EXPORTS)
    #define GICAME_CRYPTO_API __declspec(dllexport)
    #elif defined(_MSC_VER) && !defined(GICAME_CRYPTO_EXPORTS)
    #define GICAME_CRYPTO_API __declspec(dllimport)
    #else
    #define GICAME_CRYPTO_API
    #endif

    #if defined(_MSC_VER) && defined(GICAME_COMMON_EXPORTS)
    #define GICAME_COMMON_API __declspec(dllexport)
    #elif defined(_MSC_VER) && !defined(GICAME_COMMON_EXPORTS)
    #define GICAME_COMMON_API __declspec(dllimport)
    #else
    #define GICAME_COMMON_API
    #endif

#else

    #define GICAME_COMMON_API
    #define GICAME_CRYPTO_API
    #define GICAME_API

#endif

#ifdef MSVC
#define GICAME_PACK_BEGIN __pragma( pack(push, 1) )
#define GICAME_PACK_END ; __pragma( pack(pop) )
#else
#define GICAME_PACK_BEGIN
#define GICAME_PACK_END __attribute__((__packed__))
#endif

#define UNUSED(X) ((void)X)


// Classes and structs
namespace Gicame {

    class GICAME_COMMON_API MovableButNonCopyable {

    public:
        constexpr MovableButNonCopyable() {}
        MovableButNonCopyable(const MovableButNonCopyable&&) noexcept {};
        MovableButNonCopyable& operator=(const MovableButNonCopyable&&) noexcept { return *this; };

    private:
        MovableButNonCopyable(const MovableButNonCopyable&) = delete;
        MovableButNonCopyable& operator=(const MovableButNonCopyable&) = delete;

    };

};

// Classes and structs (utilities)
namespace Gicame::Utilities {

    template<typename Type>
    static constexpr bool isSigned() {
        constexpr Type allOnes = ~((Type)0);                                  // 8bit: 0b11111111
        constexpr Type maxIfSigned = ~((Type)1 << (sizeof(Type) * 8u - 1u));  // 8bit: 0b01111111
        return maxIfSigned > allOnes;
    }

    template<typename Type, typename OutType = uint64_t>
    static constexpr OutType maxOf() {
        if (isSigned<Type>())
            return OutType(~((Type)1 << (sizeof(Type) * 8u - 1u)));
        else
            return OutType(~((Type)0));
    }

    template<typename Type, typename OutType = uint64_t>
    static constexpr OutType minOf() {
        if (isSigned<Type>())
            return OutType(((Type)1 << (sizeof(Type) * 8u - 1u)));   // 8bit: 0b10000000
        else
            return OutType(0);                                       // 8bit: 0b00000000
    }

    template<typename Target, typename Original>
    static inline Target safeNumericCast(const Original original) {
        // Both unsigned
        if (!isSigned<Target>() && !isSigned<Original>() && uint64_t(original) > maxOf<Target, uint64_t>())
            throw RUNTIME_ERROR("original too big");

        // Both signed
        if (isSigned<Target>() && isSigned<Original>() && int64_t(original) > maxOf<Target, int64_t>())
            throw RUNTIME_ERROR("original too big");

        // Both signed
        if (isSigned<Target>() && isSigned<Original>() && int64_t(original) < minOf<Target, int64_t>())
            throw RUNTIME_ERROR("original too negative");

        // From signed to unsigned (negative check)
        if (!isSigned<Target>() && isSigned<Original>() && original < 0)
            throw RUNTIME_ERROR("original is negative and Target is unsigned");

        // From positive signed to unsigned
        if (!isSigned<Target>() && isSigned<Original>() && uint64_t(original) > maxOf<Target, uint64_t>())
            throw RUNTIME_ERROR("original too big");

        // From unsigned to signed
        if (isSigned<Target>() && !isSigned<Original>() && uint64_t(original) > maxOf<Target, uint64_t>())
            throw RUNTIME_ERROR("original too big");

        return (Target)original;
    }

    static inline void eraseMemory(void* addr, size_t size) {
#ifdef __STDC_LIB_EXT1__

        memset_s(pointer, size, 0, size);

#else

        volatile byte_t* p = (volatile byte_t*)addr;
        while (--size) {
            *p++ = 0;
        }

#endif
    }

}


#endif
