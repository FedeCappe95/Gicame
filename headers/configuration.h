#ifndef __GICAME__CONFIGURATION_H__
#define __GICAME__CONFIGURATION_H__


#include "common.h"


#if defined(GICAME_FORCE_IPC_SIZE_64)
    namespace Gicame { using ipc_size_t = uint64_t; };
#elif defined(GICAME_FORCE_IPC_SIZE_32)
    namespace Gicame { using ipc_size_t = uint32_t; };
#else
    namespace Gicame { using ipc_size_t = size_t; };
#endif


// Uncomment to enable the usage of Futex under Linux
//#define GICAME_USE_FUTEX


#endif