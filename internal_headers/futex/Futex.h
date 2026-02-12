#ifndef __GICAME__INTERNAL_HEADERS__FUTEX_H__
#define __GICAME__INTERNAL_HEADERS__FUTEX_H__


#include "configuration.h"


#ifdef GICAME_USE_FUTEX


#include <errno.h>
#include <linux/futex.h>
#include <atomic>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>


namespace Gicame {

    template <typename FWord>  // Futex Word :)
    static inline long int futex(
        FWord* uaddr, int op, FWord val,
        const struct timespec* timeout, FWord* uaddr2, FWord val3
    ) {
        return syscall(SYS_futex, uaddr, op, val, timeout, uaddr2, val3);
    }

    // Acquire the futex pointed to by 'futexp': wait for its value to
    // become 1, and then set the value to 0.
    template <typename FWord>
    static inline void fwait(std::atomic<FWord>* futexp) {
        const FWord zero = 0;
        FWord one = 1;

        while (1) {
            // Is the futex available?
            if (std::atomic_compare_exchange_strong(futexp, &one, zero))
                break;      // Yes

            // Futex is not available; wait.
            long int s = futex<FWord>(reinterpret_cast<FWord*>(futexp), FUTEX_WAIT, 0, NULL, NULL, 0);
            if (s == -1 && errno != EAGAIN)
                throw RUNTIME_ERROR("futex failed");
        }
    }

    // Release the futex pointed to by 'futexp': if the futex currently
    // has the value 0, set its value to 1 and then wake any futex waiters,
    // so that if the peer is blocked in fwait(), it can proceed.
    template <typename FWord>
    static inline void fpost(std::atomic<FWord> *futexp) {
        FWord zero = 0;
        const FWord one = 1;

        if (std::atomic_compare_exchange_strong(futexp, &zero, one)) {
            long int s = futex<FWord>(reinterpret_cast<FWord*>(futexp), FUTEX_WAKE, 1, NULL, NULL, 0);
            if (s == -1)
                throw RUNTIME_ERROR("futex failed");
        }
    }

};


#endif


#endif