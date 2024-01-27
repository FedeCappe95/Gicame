#include "os/NamedPipe.h"
#ifdef WINDOWS
#include <Windows.h>
#endif


using namespace Gicame::Os;


/*
 * ===== Help function =====
 */
static inline std::string getFullName(const std::string& name) {
#ifdef WINDOWS
    return std::string("\\\\.\\pipe\\") + name;
#else
#error "NYI"
#endif
}


NamedPipe::NamedPipe(const std::string& name) :
#ifdef WINDOWS
    handle(INVALID_HANDLE_VALUE),
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

    state = State::SERVER;

    return true;

#else

#error "NYI"

#endif
}

bool NamedPipe::open() {
#ifdef WINDOWS

    if (state == State::CONNECTED) {
        return true;
    }
    else if (state == State::DISCONNECTED) {
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
    else if (state == State::SERVER) {
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

#error "NYI"

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

#error "NYI"

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

#error "NYI"

#endif
}

bool NamedPipe::isReceiverConnected() const {
    return state == State::CONNECTED;
}

void NamedPipe::close() {
#ifdef WINDOWS

    state = State::DISCONNECTED;
    if (handle == INVALID_HANDLE_VALUE)
        return;
    CloseHandle(handle);
    handle = INVALID_HANDLE_VALUE;

#else

#error "NYI"

#endif
}
