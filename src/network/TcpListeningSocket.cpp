#include "common.h"
#include "network/TcpSocket.h"
#include "network/TcpListeningSocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <climits>
#include <string>
#include <iostream>

#ifdef WINDOWS
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#endif


#ifdef MSVC
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#endif


using namespace Gicame;


TcpListeningSocket::TcpListeningSocket(const InternetProtocolVersion ipv) :
	sockfd(0),
	listening(false)
{
	sin_family = (ipv == InternetProtocolVersion::IPv4 ? AF_INET : AF_INET6);
}

// Do not close the socket in the destructor unless the class is "NOT_COPYABLE"!!
// If you close it here and your instance can be copied you will spend the entire day looking for
// bugs... Just like I did...
// When you copy an instance, the old one will have an invalid socketfd as a private data member!
TcpListeningSocket::~TcpListeningSocket() {
	this->close();
}

bool TcpListeningSocket::listenTo(const uint16_t port, const uint16_t backlog) {
	if (listening)
		throw RUNTIME_ERROR("Socket already in listening state");

	int errorCode;

	#ifdef WINDOWS

	// I'm going to copy all the procedure from the Microsoft offcial documentation
	startWsaIfNeeded();
	struct addrinfo *result = NULL;
    struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = this->sin_family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
	// Resolve the server address and port
	std::string portString = std::to_string(port);
    errorCode = getaddrinfo(NULL, portString.c_str(), &hints, &result);
    if(errorCode != 0) {
		throw RUNTIME_ERROR("getaddrinfo(...) failed");
    }

	sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if(sockfd == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();
        throw RUNTIME_ERROR("socket(...) failed");
    }

	errorCode = bind(sockfd, result->ai_addr, (int)result->ai_addrlen);
    if(errorCode == SOCKET_ERROR) {
        freeaddrinfo(result);
        closesocket(sockfd);
        WSACleanup();
        throw RUNTIME_ERROR("bind(...) failed");
    }

    freeaddrinfo(result);

    errorCode = listen(sockfd, backlog);
    if (errorCode == SOCKET_ERROR) {
        closesocket(sockfd);
        WSACleanup();
        throw RUNTIME_ERROR("listen(...) failed");
    }

	#else

	sockfd = socket(sin_family, SOCK_STREAM, 0);
	if (sockfd < 0) {
		throw RUNTIME_ERROR("socket(...) failed");
    }

	//The bind procedure is different if IPv4 or IPv6 is used
	if (sin_family == AF_INET) {

		//IPv4 case
		struct sockaddr_in myAddr;
		memset(&myAddr, 0x00, sizeof(myAddr));
		myAddr.sin_family = AF_INET;
		myAddr.sin_addr.s_addr = INADDR_ANY;
		myAddr.sin_port = htons(port);
		errorCode = bind(sockfd, (struct sockaddr*) &myAddr, sizeof(myAddr));
		if(errorCode < 0) {
			::close(sockfd);
			throw RUNTIME_ERROR("bind(...) failed");
		}

	} else {

		//IPv6 case
		struct sockaddr_in6 myAddr;
		memset(&myAddr, 0x00, sizeof(myAddr));
		myAddr.sin6_family = AF_INET6;
		myAddr.sin6_addr = in6addr_any;
		myAddr.sin6_port = htons(port);
		errorCode = bind(sockfd, (struct sockaddr*) &myAddr, sizeof(myAddr));
		if(errorCode < 0) {
			::close(sockfd);
			throw RUNTIME_ERROR("bind(...) failed");
		}

	}

	errorCode = listen(sockfd,backlog);
	if (errorCode < 0) {
        ::close(sockfd);
        throw RUNTIME_ERROR("listen(...) failed");
    }

	#endif

	listening = true;

	return listening;
}

TcpSocket TcpListeningSocket::acceptRequest() {
	SocketDescriptor newsockfd;

	//The accept procedure is "different" if IPv4 or IPv6 is used
	//The client address is stored and never used! This is just for debug reasons.
	/*if(sin_family == AF_INET) {
		//IPv4 case
		struct sockaddr_in requestingAddr;
		socklen_t requestingAddrSize = sizeof(requestingAddr);
		newsockfd = accept(sockfd, (struct sockaddr*)&requestingAddr, &requestingAddrSize);
	} else {
		//IPv6 case
		struct sockaddr_in6 requestingAddr;
		socklen_t requestingAddrSize = sizeof(requestingAddr);
		newsockfd = accept(sockfd, (struct sockaddr*)&requestingAddr, &requestingAddrSize);
	}*/
	newsockfd = accept(sockfd, NULL, NULL);

	#ifdef WINDOWS
	if(newsockfd == INVALID_SOCKET)
	#else
	if(newsockfd < 0)
	#endif
		throw RUNTIME_ERROR("accept(...) failed");

	return TcpSocket::fromSockfd(newsockfd, SocketStatus::CONNECTED, sin_family);
}

void TcpListeningSocket::close() {
	if(sockfd > 0) {
		#ifdef WINDOWS
		closesocket(sockfd);
		#else
		::close(sockfd);
		#endif
		sockfd = 0;
	}
	listening = false;
}
