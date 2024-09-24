#include "common.h"
#include "utils/Numbers.h"
#ifdef WINDOWS
#include <windows.h>
#else
#include <semaphore.h>
#include <fcntl.h>           // For O_CREAT and O_EXCL
#include <sys/stat.h>        // For mode constants
#include <errno.h>
#endif
#include "sm/Semaphore.h"

// Constants
#ifdef WINDOWS
constexpr DWORD ACQUISITION_TIMEOUT = 1;  // 1ms
#endif


using namespace Gicame;



Semaphore::Semaphore(const std::string& name, const size_t maxConcurrency, const size_t initialLockCount) :
    handle(NULL),
#ifndef WINDOWS
    posixName(std::string("/") + name),
#endif
    unlinkOnDestruction(true)
{
    if (unlikely(maxConcurrency == 0))
        throw RUNTIME_ERROR("maxConcurrency cannot be 0");

    if (unlikely(initialLockCount > maxConcurrency))
        throw RUNTIME_ERROR("initialLockCount cannot be greater than maxConcurrency");

    if (unlikely(name.empty()))
        throw RUNTIME_ERROR("name cannot be empty");

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

    // Open a named semaphore (O_CREAT | O_EXCL ensures that if the semaphore already exists, we get an error)
    handle = sem_open(posixName.c_str(), O_CREAT | O_EXCL, 0644, maxConcurrency - initialLockCount);

    if (handle == SEM_FAILED) {
        if (errno == EEXIST) {  // If the semaphore already exists, open the existing one
            handle = sem_open(posixName.c_str(), 0);
            if (unlikely(handle == SEM_FAILED))
                throw RUNTIME_ERROR("sem_open(...) failed");
        }
        else if (unlikely(errno == EINVAL)) {
            for (size_t i = 0; i < name.length(); ++i)
                if (name[i] == '/')
                    throw RUNTIME_ERROR("sem_open(...) failed, invalid semaphore name (cannot contain '/')");
            throw RUNTIME_ERROR("sem_open(...) failed");
        }
    }

#endif
}

Semaphore::~Semaphore() {
#ifdef WINDOWS

    if (likely(handle))
        CloseHandle(handle);

#else

    if (likely(handle != SEM_FAILED)) {
        sem_close(handle);
        if (unlinkOnDestruction)
            sem_unlink(posixName.c_str());
    }

#endif
}

bool Semaphore::acquire() {
#ifdef WINDOWS

    // WaitForSingleObject(...) either:
    //   - timeouts (and returns WAIT_TIMEOUT) if the current sempahore state is 0 or
    //   - returns WAIT_OBJECT_0 and decrements the current semaphore state
    DWORD result;
    do {
        result = WaitForSingleObject(handle, ACQUISITION_TIMEOUT);
        if (unlikely(result == WAIT_FAILED))
            return false;
    } while (result != WAIT_OBJECT_0);

#else

    // POSIX semaphore wait (blocks until the semaphore is greater than 0, then decrements it)
    if (unlikely(sem_wait(handle) != 0))
        return false;

#endif

    return true;
}

bool Semaphore::release() {
#ifdef WINDOWS

    // ReleaseSemaphore(...) increments the current semaphore state
    const BOOL success = ReleaseSemaphore(
        handle,       // handle to semaphore
        1,            // increase count by one
        NULL          // not interested in previous count
    );
    if (unlikely(!success))
        return false;

#else

    // POSIX semaphore post (increments the semaphore)
    if (unlikely(sem_post(handle) != 0))
        return false;

#endif

    return true;
}

void Semaphore::setUnlinkOnDestruction(bool uod) {
    unlinkOnDestruction = uod;
}
