#include "common.h"
#include "network/TcpSocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WINDOWS
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <codecvt>
#endif


#ifdef MSVC
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#endif


using namespace Gicame;


TcpSocket::TcpSocket(
	const SocketDescriptor sockfd, const SocketStatus socketStatus, const short sin_family
) : 
	sockfd(sockfd), socketStatus(socketStatus), sin_family(sin_family), flagReceptionBlocking(false)
{
	// Nothing to do
}

TcpSocket::TcpSocket(const InternetProtocolVersion ipv) :
	sockfd(0),
	socketStatus(SocketStatus::CLOSED),
	flagReceptionBlocking(false)
{
	#ifdef WINDOWS
	Gicame::startWsaIfNeeded();
	#endif
	sin_family = (ipv == InternetProtocolVersion::IPv4 ? AF_INET : AF_INET6);
	sockfd = socket(sin_family, SOCK_STREAM, 0);
	if (sockfd < 0) {
		throw RUNTIME_ERROR("Invalid sockfd");
    }
}

TcpSocket::TcpSocket(TcpSocket&& other) noexcept :
	sockfd(other.sockfd),
	socketStatus(other.socketStatus),
	sin_family(other.sin_family),
	flagReceptionBlocking(other.flagReceptionBlocking)
{
	other.sockfd = 0;  // Prevent close() on destructor
}

// Do not close the socket in the destructor unless the TcpSocket is "NOT_COPYABLE"!!
// If you close it here and your TcpSocket can be copied you will spend the entire day looking for
// bugs... Just like I did...
// When you copy a TcpSocket, the old one will have an invalid socketfd as a private data member!
// In this implementation, TcpSocket is "NOT_COPYABLE", so it's safe (and required) to
// close the socket here.
TcpSocket::~TcpSocket() {
	this->close();
}

bool TcpSocket::connectTo(const std::string& ip, const uint16_t port) {
	if (isConnected()) {
		throw RUNTIME_ERROR("Already connected socket");
	}

	int connectionStatus;
	if (sin_family == AF_INET) {
		//Connect using IPv4
		struct sockaddr_in serverAddr;
		memset(&serverAddr, 0x00, sizeof(serverAddr));
		serverAddr.sin_family = sin_family;
	    serverAddr.sin_port = htons(port);
		serverAddr.sin_addr = IPv4(ip).toInAddr();
		connectionStatus = connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	} else {
		//Connect using IPv6
		struct sockaddr_in6 serverAddr;
		memset(&serverAddr, 0x00, sizeof(serverAddr));
		serverAddr.sin6_family = sin_family;
	    serverAddr.sin6_port = htons(port);
		serverAddr.sin6_addr = IPv6(ip).toIn6Addr();
		connectionStatus = connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	}

	socketStatus = connectionStatus == 0 ? SocketStatus::CONNECTED : SocketStatus::SS_ERROR;

	return isConnected();
}

bool TcpSocket::connectTo(const IPv4& ip, const uint16_t port) {
	if (isConnected())
		throw RUNTIME_ERROR("Already connected socket");

	if (sin_family != AF_INET) {
		throw RUNTIME_ERROR("Trying to connect to a IPv4 using a IPv6 socket");
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0x00, sizeof(serverAddr));
	serverAddr.sin_family = sin_family;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr = ip.toInAddr();
	int connectionStatus = connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	socketStatus = connectionStatus == 0 ? SocketStatus::CONNECTED : SocketStatus::SS_ERROR;

	return isConnected();
}

bool TcpSocket::connectTo(const IPv6& ip, const uint16_t port) {
	if (isConnected())
		throw RUNTIME_ERROR("Already connected socket");

	if (sin_family != AF_INET6) {
		throw RUNTIME_ERROR("Trying to connect to a IPv6 using a IPv4 socket");
	}

	struct sockaddr_in6 serverAddr;
	memset(&serverAddr, 0x00, sizeof(serverAddr));
	serverAddr.sin6_family = sin_family;
	serverAddr.sin6_port = htons(port);
	serverAddr.sin6_addr = ip.toIn6Addr();
	int connectionStatus = connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	socketStatus = connectionStatus == 0 ? SocketStatus::CONNECTED : SocketStatus::SS_ERROR;

	return isConnected();
}

size_t TcpSocket::send(const void* buffer, const size_t size) {
	if (size > (size_t)INT_MAX)
		throw RUNTIME_ERROR("Size too big");
	if (!isConnected())
		throw RUNTIME_ERROR("It is impossible to send something using a non connected socket");

	#ifdef WINDOWS
	int actuallySent = ::send(sockfd, (const char*)buffer, (int)size, 0);
	#else
	int actuallySent = ::send(sockfd, buffer, size, 0);
	#endif

	if (actuallySent < 0) {
		socketStatus = SocketStatus::SS_ERROR;
		throw RUNTIME_ERROR("Error sending socket data");
	}

	return (uint32_t)actuallySent;
}

size_t TcpSocket::receive(void* buffer, const size_t size) {
	if (size > (size_t)INT_MAX) {
		throw RUNTIME_ERROR("Receiving too much");
	}
	if(!isConnected()) {
		throw RUNTIME_ERROR("It is impossible to send something using a non connected socket");
	}

	#ifdef WINDOWS
	int actuallyReceived = recv(sockfd, (char*)buffer, (int)size, flagReceptionBlocking ? MSG_WAITALL : 0);
	#else
	int actuallyReceived = recv(sockfd, buffer, size, flagReceptionBlocking ? MSG_WAITALL : 0);
	#endif

	if (actuallyReceived < 0) {
		socketStatus = SocketStatus::SS_ERROR;
		throw RUNTIME_ERROR("Error receiving socket data");
	}

	return (uint32_t)actuallyReceived;
}

void TcpSocket::close() {
	if(sockfd > 0) {
		#ifdef WINDOWS
		closesocket(sockfd);
		#else
		::close(sockfd);
		#endif
		sockfd = 0;
	}
	socketStatus = SocketStatus::CLOSED;
}

bool TcpSocket::isConnected() const {
	if (socketStatus != SocketStatus::CONNECTED)
		return false;
	int errorCode;
	this->getSockOptions(SOL_SOCKET, SO_ERROR, &errorCode);
	return errorCode >= 0;
}

TcpSocket TcpSocket::fromSockfd(SocketDescriptor sockfd, const SocketStatus socketStatus, const short sin_family) {
	if(sockfd < 0) {
		throw RUNTIME_ERROR("Invalid sockfd");
	}

	return TcpSocket(sockfd, socketStatus, sin_family);
}

IPv4 TcpSocket::getPeerIPv4() const {
	if(sin_family == AF_INET) {

        //IPv4 case
        struct sockaddr_in addr;
        socklen_t addr_size = sizeof(addr);
        if(getpeername(sockfd, (struct sockaddr *)&addr, &addr_size) < 0) {
            throw RUNTIME_ERROR("Error in getpeername(...)");
        }
		return IPv4((uint32_t)addr.sin_addr.s_addr);

    } else {

		throw RUNTIME_ERROR("Socket is IPv6");

    }
}

void TcpSocket::setReceptionBlocking(const bool rb) {
	flagReceptionBlocking = rb;
}

bool TcpSocket::getSockOptions(const int level, const int optname, void* outValue, const uint32_t outValueSize) const {
	#ifdef WINDOWS
	if(outValueSize > INT_MAX)
		throw RUNTIME_ERROR("outValueSize too large. Winsock2 take an integer, max value is INT_MAX");
	#endif

	int success = getsockopt(
		sockfd, level, optname,
		#ifdef WINDOWS
		(char*)outValue, (int*)&outValueSize
		#else
		outValue, (socklen_t*)&outValueSize
		#endif
	);

	return (success == 0);
}

bool TcpSocket::setSockOptions(const int level, const int optname, const void* inValue, const uint32_t inValueSize) {
	#ifdef WINDOWS
	if(inValueSize > INT_MAX)
		throw RUNTIME_ERROR("inValueSize too large. Winsock2 take an integer, max value is INT_MAX");
	#endif

	int success = setsockopt(
		sockfd, level, optname,
		#ifdef WINDOWS
		(const char*)inValue, (int)inValueSize
		#else
		inValue, (socklen_t)inValueSize
		#endif
	);

	return (success == 0);
}
