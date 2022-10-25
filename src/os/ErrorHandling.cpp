#include "os/ErrorHandling.h"

#ifdef WINDOWS
#include <Windows.h>
#else
#include <string.h>
#endif


using namespace Gicame::Error;


int getLastCode() {
    #ifdef WINDOWS
    return WSAGetLastError();
    #else
    return errno;
    #endif
}

std::string getLastErrorAsString() {
    #ifdef WINDOWS

    char* s = NULL;
    DWORD ec = FormatMessageA(
    	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    	NULL, WSAGetLastError(),
    	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    	(LPSTR)&s, 0, NULL
    );
    if(ec <= 0) {
    	LocalFree(s);
    	return std::string("Unknown error");
    }
    const std::string errStr(s);
    LocalFree(s);
    return errStr;

    #else

    return std::string(strerror(errno));

    #endif
}
