#include "common.h"
#include "network/NetworkUtility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <climits>
#include <iostream>

#ifdef WINDOWS
#include <Ws2tcpip.h>
#ifndef MINGW           //With MSVC2019 we need ip2string.h that is from the Windows SDK Kit
#include <ip2string.h>  //With MinGW everything is in ws2rcpip.h
#endif
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif


#ifdef WINDOWS
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#endif


// Data
#ifdef WINDOWS
static bool wsaStarted = false;
#endif


#ifdef WINDOWS
void Gicame::startWsaIfNeeded() {
	if(!wsaStarted) {
		WSADATA wsaData;
		int errorCode = WSAStartup(MAKEWORD(2,2), &wsaData);
	    if(errorCode != 0) {
			std::string errorMessage = std::string("WSAStartup(...) failed, result is ") + std::to_string(errorCode);
			throw RUNTIME_ERROR(errorMessage);
	    }
	}
}
#endif
