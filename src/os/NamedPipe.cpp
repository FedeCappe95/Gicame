#ifdef __CYGWIN__
#ifdef __POSIX_VISIBLE
#undef __POSIX_VISIBLE
#endif
#define __POSIX_VISIBLE 200112
#endif


#include "os/NamedPipe.h"
#include "utils/Numbers.h"
#ifdef WINDOWS
#include <Windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#endif


using namespace Gicame::Os;


/*
 * ===== Help function =====
 */
static inline std::string getFullName(const std::string& name) {
#ifdef WINDOWS
    return std::string("\\\\.\\pipe\\GicamePipe-") + name;
#else
    return std::string("/tmp/GicamePipe-") + name;
#endif
}


NamedPipe::NamedPipe(const std::string& name) :
#ifdef WINDOWS
    handle(INVALID_HANDLE_VALUE),
#else
    fd(-1),
#endif
    fullName(getFullName(name)),
    state(State::DISCONNECTED)
{
    // Nothing to do
}

NamedPipe::~NamedPipe() {
    close();
}

bool NamedPipe::create(const size_t bufferSize) {
#ifdef WINDOWS

    const DWORD dwBufferSize = Gicame::Utilities::safeNumericCast<DWORD>(bufferSize);
    handle = CreateNamedPipeA(
        fullName.c_str(),
        PIPE_ACCESS_DUPLEX,            // read/write access 
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        dwBufferSize,                  // output buffer size 
        dwBufferSize,                  // input buffer size 
        0,                             // client time-out 
        NULL                           // default security attribute
    );

    if (handle == INVALID_HANDLE_VALUE)
        return false;

    DWORD mode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
    const bool success = SetNamedPipeHandleState(handle, &mode, NULL, NULL);

    if (!success) {
        close();
        return false;
    }

    state = State::CREATED;

    return true;

#else

    UNUSED(bufferSize);

    const auto retCode = mkfifo(fullName.c_str(), 0666);
    if (retCode != 0) {
        close();
        return false;
    }
    state = State::CREATED;
    return true;

#endif
}

bool NamedPipe::open() {
    if (state == State::CONNECTED)
        return true;

#ifdef WINDOWS

    if (state == State::DISCONNECTED) {
        handle = CreateFileA(
            fullName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,              // no sharing
            NULL,           // default security attributes
            OPEN_EXISTING,  // opens existing pipe
            0,              // default attributes
            NULL            // no template file
        );

        if (handle == INVALID_HANDLE_VALUE)
            return false;

        DWORD mode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
        const bool success = SetNamedPipeHandleState(handle, &mode, NULL, NULL);
        if (!success) {
            close();
            return false;
        }
    }
    else if (state == State::CREATED) {
        const bool success = (bool)ConnectNamedPipe(handle, NULL);
        if (!success) {
            close();
            return false;
        }
    }
    else
        throw RUNTIME_ERROR("Invalid state");

    state = State::CONNECTED;

    return true;

#else

    fd = ::open(fullName.c_str(), O_RDWR);
    if (fd < 0) {
        close();
        return false;
    }
    state = State::CONNECTED;
    return true;

#endif
}

size_t NamedPipe::send(const void* buffer, const size_t size) {
    if (state != State::CONNECTED)
        throw RUNTIME_ERROR("Pipe not connected");

#ifdef WINDOWS

    const DWORD dwSize = Gicame::Utilities::safeNumericCast<DWORD>(size);
    DWORD writtenByteCount = 0;
    const bool success = WriteFile(handle, buffer, dwSize, &writtenByteCount, NULL);
    if (!success)
        close();
    return Gicame::Utilities::safeNumericCast<size_t>(writtenByteCount);

#else

    const auto writtenByteCount = write(fd, buffer, size);
    if (writtenByteCount < 0) {
        close();
        return ~size_t(0);
    }
    return Gicame::Utilities::safeNumericCast<size_t>(writtenByteCount);

#endif
}

bool NamedPipe::isSenderConnected() const {
    return state == State::CONNECTED;
}

size_t NamedPipe::receive(void* buffer, const size_t size) {
    if (state != State::CONNECTED)
        throw RUNTIME_ERROR("Pipe not connected");

#ifdef WINDOWS

    const DWORD dwSize = Gicame::Utilities::safeNumericCast<DWORD>(size);
    DWORD readByteCount = 0;
    const bool success = ReadFile(handle, buffer, dwSize, &readByteCount, NULL);
    if (!success)
        close();
    return Gicame::Utilities::safeNumericCast<size_t>(readByteCount);

#else

    const auto readByteCount = read(fd, buffer, size);
    if (readByteCount < 0) {
        close();
        return ~size_t(0);
    }
    return Gicame::Utilities::safeNumericCast<size_t>(readByteCount);

#endif
}

bool NamedPipe::isReceiverConnected() const {
    return state == State::CONNECTED;
}

void NamedPipe::close() {
#ifdef WINDOWS

    if (handle == INVALID_HANDLE_VALUE)
        return;
    CloseHandle(handle);
    handle = INVALID_HANDLE_VALUE;

#else

    if (fd >= 0)
        ::close(fd);
    fd = -1;

#endif

    state = State::DISCONNECTED;
}

void NamedPipe::unlink() {
    if (state == State::CONNECTED)
        throw RUNTIME_ERROR("NamedPipe cannot be unlinked if connected");

    if (state != State::DISCONNECTED)
        close();

#ifdef WINDOWS

    DeleteFileA(fullName.c_str());

#else

    ::unlink(fullName.c_str());

#endif
}
