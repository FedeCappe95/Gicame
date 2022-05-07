#ifndef __COMMON_H__
#define __COMMON_H__


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


// Type definitions
typedef uint8_t byte_t;
typedef void* ptr_t;


// Macros
#if defined(_WIN32) && !defined(WINDOWS)
    #define WINDOWS
#endif
#if defined(_MSC_VER) && !defined(MSVC)
    #define MSVC
#endif
#define RUNTIME_ERROR(MSG) std::runtime_error(std::string(__FUNCTION__) + ": " + MSG)
#define MOVABLE_BUT_NOT_COPYABLE Gicame::MovableButNonCopyable movableButNonCopyable
#if defined(_MSC_VER)
	#define likely(A) (A)
	#define unlikely(A) (A)
#else
	#define likely(A) __builtin_expect((A) != 0, 1)
	#define unlikely(A) __builtin_expect((A), 0)
#endif
#if defined(_MSC_VER) && defined(GICAME_EXPORTS)
	#define GICAME_API __declspec(dllexport)
#elif defined(_MSC_VER) && !defined(GICAME_EXPORTS)
	#define GICAME_API __declspec(dllimport)
#else
	#define GICAME_API
#endif


// Classes and structs
namespace Gicame {

    class GICAME_API MovableButNonCopyable {

    public:
        constexpr MovableButNonCopyable() {}
        MovableButNonCopyable(const MovableButNonCopyable&&) noexcept {};
        MovableButNonCopyable& operator=(const MovableButNonCopyable&&) noexcept { return *this; };

    private:
        MovableButNonCopyable(const MovableButNonCopyable&) = delete;
        MovableButNonCopyable& operator=(const MovableButNonCopyable&) = delete;

    };

};


#endif
