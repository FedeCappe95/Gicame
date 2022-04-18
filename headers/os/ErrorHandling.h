#ifndef __ERRORHANDLING_H__
#define __ERRORHANDLING_H__


#include "../common.h"


namespace Gicame {

    static inline int getLastErrorCode() {
    	#ifdef WINDOWS
    	return WSAGetLastError();
    	#else
    	return errno;
    	#endif
    }

    static std::string getLastErrorString() {
    	#ifdef WINDOWS

    	wchar_t *s = NULL;
    	DWORD ec = FormatMessageW(
    		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    		NULL, WSAGetLastError(),
    	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    	    (LPWSTR)&s, 0, NULL
    	);
    	if(ec <= 0) {
    		LocalFree(s);
    		return std::string("Unknown Winsock2 error code");
    	}
    	#ifdef MSVC
    	std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt_utf8;
    	std::string strUtf8 = cvt_utf8.to_bytes(s);
    	LocalFree(s);
    	return strUtf8;
    	#else
    	std::wstring ws(s);
    	LocalFree(s);
    	return std::string(ws.begin(), ws.end());
    	#endif

    	#else

    	return std::string(strerror(errno));

    	#endif
    }

};


#endif
