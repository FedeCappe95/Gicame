#ifndef __GICAME__MEMORY_H__
#define __GICAME__MEMORY_H__


#include "../common.h"
#include <stdint.h>


// Classes and structs (utilities)
namespace Gicame::Utilities {

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