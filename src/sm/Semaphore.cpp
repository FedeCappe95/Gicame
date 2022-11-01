#include "sm/Semaphore.h"


#ifdef WINDOWS
#include <windows.h>
#else
#error "NYI"
#endif


//#ifdef MSVC
//#pragma comment(lib, "user32.lib")
//#endif


// Constants
#ifdef WINDOWS
constexpr DWORD ACQUISITION_TIMEOUT = 1;  // 1ms
#endif


using namespace Gicame;



Semaphore::Semaphore(const std::string& name, const size_t maxConcurrency, const size_t initialLockCount) {
    if (unlikely(maxConcurrency == 0))
        throw RUNTIME_ERROR("maxConcurrency cannot be 0");

    if (unlikely(initialLockCount > maxConcurrency))
        throw RUNTIME_ERROR("initialLockCount cannot be greater than maxConcurrency");

#ifdef WINDOWS

    /*
     * From Microsoft DOC:
     *   The state of a semaphore object is signaled when its count is greater than zero and
     *   nonsignaled when its count is equal to zero.
     */

    const std::string windowsName = std::string("Local\\") + name;

    if (unlikely(windowsName.length() > MAX_PATH - 1))
        throw RUNTIME_ERROR("name too long");

    const LONG windowsMaxSemCount = Gicame::Utilities::safeNumericCast<LONG>(maxConcurrency);
    const LONG windowsIniSemCount = Gicame::Utilities::safeNumericCast<LONG>(maxConcurrency - initialLockCount);

    handle = CreateSemaphoreA(
        NULL,                // default security attributes
        windowsIniSemCount,  // initial count
        windowsMaxSemCount,  // maximum count
        windowsName.c_str()
    );

    if (unlikely(!handle))
        throw RUNTIME_ERROR("CreateSemaphoreA(...) failed");

#else

#error "NYI"

#endif
}

Semaphore::~Semaphore() {
#ifdef WINDOWS

    if (likely(handle))
        CloseHandle(handle);

#else

#error "NYI"

#endif
}

void Semaphore::acquire() {
#ifdef WINDOWS

    // WaitForSingleObject(...) either:
    //   - timeouts (and returns WAIT_TIMEOUT) if the current sempahore state is 0 or
    //   - returns WAIT_OBJECT_0 and decrements the current semaphore state
    DWORD result;
    do {
        result = WaitForSingleObject(handle, ACQUISITION_TIMEOUT);
    } while (result != WAIT_OBJECT_0);

#else

#error "NYI"

#endif
}

void Semaphore::release() {
#ifdef WINDOWS

    // ReleaseSemaphore(...) increments the current semaphore state
    if (unlikely(!ReleaseSemaphore(
        handle,       // handle to semaphore
        1,            // increase count by one
        NULL)         // not interested in previous count
    ))
        throw RUNTIME_ERROR("ReleaseSemaphore(...) failed");

#else

#error "NYI"

#endif
}
