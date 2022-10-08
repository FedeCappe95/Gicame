#include "device/Serial.h"
#include <string>

#ifdef WINDOWS
#include <Windows.h>
#endif


using namespace Gicame::Device;


#define nyi() throw std::runtime_error("Gicame::Device::Serial::" __FUNCTION__ "(...): not yet implemented")


bool Serial::open() {
#ifdef WINDOWS

    if (unlikely(handle))
        RUNTIME_ERROR("Trying to open an already open serial connection");

    const std::string portName = std::string("\\\\.\\COM") + std::to_string(comIndex);
    handle = CreateFileA(
        portName.c_str(),
        GENERIC_READ | GENERIC_WRITE, // Read/Write
        0,                            // No Sharing
        NULL,                         // No Security
        OPEN_EXISTING,                // Open existing port only
        0,                            // Non Overlapped I/O
        NULL                          // Null for Comm Devices
    );
    if (unlikely(handle == INVALID_HANDLE_VALUE))
        goto cleanup;

    //  Initialize the DCB structure.
    DCB dcb;
    SecureZeroMemory(&dcb, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);

    //  Build on the current configuration by first retrieving all current settings.
    if (unlikely(!GetCommState(handle, &dcb)))
        goto cleanup;
    dcb.BaudRate = boud; //CBR_115200;     //  baud rate
    dcb.ByteSize = byteSize; // 8;             //  data size, xmit and rcv
    dcb.Parity = parity; // NOPARITY;      //  parity bit
    dcb.StopBits = stopBitsMode; // ONESTOPBIT;    //  stop bit

    // Set configuration
    if (unlikely(!SetCommState(handle, &dcb)))
        goto cleanup;

    return true;

cleanup:
    close();
    return false;

#elif

    nyi();

#endif
}

size_t Serial::send(const uint8_t* what, const size_t size) {
#ifdef WINDOWS

    if (unlikely(size > UINT32_MAX))
        RUNTIME_ERROR("size exceeded max value of 2^32-1");
    DWORD dNoOfBytesWritten;
    WriteFile(handle, what, (DWORD)size, &dNoOfBytesWritten, NULL);
    return dNoOfBytesWritten;

#elif

    nyi();

#endif
}

uint8_t* Serial::receive(uint8_t* outBuffer, const size_t size) {
#ifdef WINDOWS

    if (unlikely(size > UINT32_MAX))
        RUNTIME_ERROR("size exceeded max value of 2^32-1");

    size_t count = 0;
    DWORD r;
    while (count < size) {
        if (!ReadFile(handle, &outBuffer[count], 1, &r, NULL))
            continue;
        count += r;
    }

    return outBuffer;

#elif

    nyi();

#endif
}

void Serial::close() {
#ifdef WINDOWS

    if (likely(handle && handle != INVALID_HANDLE_VALUE))
        CloseHandle(handle);
    handle = NULL;

#elif

    nyi();

#endif
}
