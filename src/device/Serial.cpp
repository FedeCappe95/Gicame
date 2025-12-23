#include "device/Serial.h"
#include "utils/Numbers.h"
#include <string>

#ifdef WINDOWS
#include <Windows.h>
#endif


using namespace Gicame::Device;


#define nyi() throw std::runtime_error(std::string("Gicame::Device::Serial::") + __FUNCTION__ + "(...): not yet implemented");
#define VP_DISPLACEMENT(PTR, OFF) ((void*)((uint8_t*)(PTR) + OFF))


Serial::Serial(const uint32_t comIndex, const uint32_t boud, const uint32_t byteSize, const uint32_t parity, const uint32_t stopBitsMode) :
    handle(NULL), comIndex(comIndex), boud(boud), byteSize(byteSize), parity(parity), stopBitsMode(stopBitsMode), blockingRecpt(false)
{}


bool Serial::open() {
#ifdef WINDOWS

    if (unlikely(handle))
        throw RUNTIME_ERROR("Trying to open an already open serial connection");

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
    dcb.BaudRate = Gicame::Utilities::safeNumericCast<DWORD>(boud);        // ex. CBR_115200  //  baud rate
    dcb.ByteSize = Gicame::Utilities::safeNumericCast<BYTE>(byteSize);     // ex. 8           //  data size, xmit and rcv
    dcb.Parity = Gicame::Utilities::safeNumericCast<BYTE>(parity);         // ex. NOPARITY    //  parity bit
    dcb.StopBits = Gicame::Utilities::safeNumericCast<BYTE>(stopBitsMode); // ex. ONESTOPBIT  //  stop bit
    dcb.fDtrControl = DTR_CONTROL_ENABLE;   // Control singal PC to device, Data Terminal Ready, 
    dcb.fRtsControl = RTS_CONTROL_ENABLE;   // Control signal PC to device, Request To Send, flow control or wake-up signal
    dcb.fOutxCtsFlow = FALSE;               // Control signal device to pc, Clear To Send
    dcb.fOutxDsrFlow = FALSE;               // Control signal device to pc, Data Set Ready

    // Set configuration
    if (unlikely(!SetCommState(handle, &dcb)))
        goto cleanup;

    return true;

cleanup:
    close();
    return false;

#else

    nyi();
    return false;

#endif
}

bool Serial::isReceiverConnected() const {
    return handle != NULL;
}

bool Serial::isSenderConnected() const {
    return handle != NULL;
}

size_t Serial::send(const void* buffer, const size_t size) {
#ifdef WINDOWS

    const DWORD dwSize = Gicame::Utilities::safeNumericCast<DWORD>(size);
    DWORD dNoOfBytesWritten;
    WriteFile(handle, buffer, dwSize, &dNoOfBytesWritten, NULL);
    return Gicame::Utilities::safeNumericCast<size_t>(dNoOfBytesWritten);

#else

    UNUSED(buffer);
    UNUSED(size);
    nyi();
    return 0;

#endif
}

size_t Serial::receive(void* buffer, const size_t size) {
#ifdef WINDOWS

    DWORD r = 0;
    size_t stillToRead = size;
    while (stillToRead) {
        if (!ReadFile(handle, buffer, Gicame::Utilities::safeNumericCast<DWORD>(stillToRead), &r, NULL) || r == 0u) {
            if (blockingRecpt)
                continue;
            else
                break;
        }
        buffer = VP_DISPLACEMENT(buffer, r);
        stillToRead -= r;
    }

    return size - stillToRead;

#else

    UNUSED(buffer);
    UNUSED(size);
    nyi();
    return 0;

#endif
}

void Serial::clear(const bool rx, const bool tx) {
#ifdef WINDOWS

    if (!handle || handle == INVALID_HANDLE_VALUE)
        return;

    // PURGE_RXCLEAR -> Discards all unread received bytes
    // PURGE_RXABORT -> Cancels pending read operations
    // PURGE_TXCLEAR -> Discards queued outgoing bytes
    // PURGE_TXABORT -> Cancels pending write operations
    DWORD flags = 0;
    if (rx)
        flags |= PURGE_RXCLEAR | PURGE_RXABORT;
    if (tx)
        flags |= PURGE_TXCLEAR | PURGE_TXABORT;

    PurgeComm(handle, flags);

#else

    UNUSED(rx);
    UNUSED(tx);
    nyi();

#endif
}

void Serial::flush() {
#ifdef WINDOWS

    FlushFileBuffers(handle);

#else

    nyi();

#endif
}

void Serial::close() {
#ifdef WINDOWS

    if (likely(handle && handle != INVALID_HANDLE_VALUE))
        CloseHandle(handle);
    handle = NULL;

#else

    nyi();

#endif
}

std::vector<Serial::SerialPort> Serial::enumerateSerialPorts() {
    std::vector<Serial::SerialPort> result;

#ifdef WINDOWS

    char outBuffer[5000];

    for (uint32_t i = 0; i < 255u; ++i) {
        const std::string name = "COM" + std::to_string(i);
        DWORD outSize = QueryDosDeviceA(name.c_str(), outBuffer, sizeof(outBuffer));

        if (unlikely(GetLastError() == ERROR_INSUFFICIENT_BUFFER))
            throw RUNTIME_ERROR("insufficient buffer");

        if (likely(outSize > 0))
            result.emplace_back(outBuffer, i);
    }

#else

    nyi();

#endif

    return result;
}

void Serial::setReceptionBlocking(const bool rb) {
    blockingRecpt = rb;
}