#ifndef __GICAME__NUMBERS_H__
#define __GICAME__NUMBERS_H__


#include <stdint.h>
#include <limits.h>
#include <stdexcept>


#define NUMBERS_RUNTIME_ERROR(MSG) std::runtime_error(std::string(__FUNCTION__) + "(...): " + (MSG))


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
        if constexpr (std::is_same<Target, Original>::value)
            return original;

        // Both unsigned
        if (!isSigned<Target>() && !isSigned<Original>() && uint64_t(original) > maxOf<Target, uint64_t>())
            throw NUMBERS_RUNTIME_ERROR("original too big");

        // Both signed
        if (isSigned<Target>() && isSigned<Original>() && int64_t(original) > maxOf<Target, int64_t>())
            throw NUMBERS_RUNTIME_ERROR("original too big");

        // Both signed
        if (isSigned<Target>() && isSigned<Original>() && int64_t(original) < minOf<Target, int64_t>())
            throw NUMBERS_RUNTIME_ERROR("original too negative");

        // From signed to unsigned (negative check)
        if (!isSigned<Target>() && isSigned<Original>() && original < 0)
            throw NUMBERS_RUNTIME_ERROR("original is negative and Target is unsigned");

        // From positive signed to unsigned
        if (!isSigned<Target>() && isSigned<Original>() && uint64_t(original) > maxOf<Target, uint64_t>())
            throw NUMBERS_RUNTIME_ERROR("original too big");

        // From unsigned to signed
        if (isSigned<Target>() && !isSigned<Original>() && uint64_t(original) > maxOf<Target, uint64_t>())
            throw NUMBERS_RUNTIME_ERROR("original too big");

        return (Target)original;
    }

}


#undef NUMBERS_RUNTIME_ERROR


#endif