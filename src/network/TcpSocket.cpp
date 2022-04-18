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


#ifdef WINDOWS
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#endif


using namespace std;
using namespace Gicame;


TcpSocket::TcpSocket(const InternetProtocolVersion ipv) :
	sockfd(0),
	socketStatus(SocketStatus::CLOSED)
{
	#ifdef WINDOWS
	Gicame::startWsaIfNeeded();
	#endif
	sin_family = (ipv == InternetProtocolVersion::IPv4 ? AF_INET : AF_INET6);
	sockfd = socket(sin_family, SOCK_STREAM, 0);
	if(sockfd < 0) {
		throw RUNTIME_ERROR("Invalid sockfd");
    }
}

// Do not close the socket in the destructor unless the TcpSocket is "MOVABLE_BUT_NOT_COPYABLE"!!
// If you close it here and your TcpSocket can be copied you will spend the entire day looking for
// bugs... Just like I did...
// When you copy a TcpSocket, the old one will have an invalid socketfd as a private data member!
TcpSocket::~TcpSocket() {
	if (socketStatus != SocketStatus::CLOSED) {
		this->close();
	}
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
		throw RUNTIME_ERROR("Not yet implemented!");
		//Connect using IPv6
		struct sockaddr_in6 serverAddr;
		memset(&serverAddr, 0x00, sizeof(serverAddr));
		serverAddr.sin6_family = sin_family;
	    serverAddr.sin6_port = htons(port);
		//serverAddr.sin6_addr = ipv6AddressFromString(ip);  TODO
		connectionStatus = connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	}

	socketStatus = connectionStatus == 0 ? SocketStatus::CONNECTED : SocketStatus::ERROR;

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
	socketStatus = connectionStatus == 0 ? SocketStatus::CONNECTED : SocketStatus::ERROR;

	return isConnected();
}

/*void TcpSocket::sendOrDie(const void* buffer, const uint32_t size) {
	#ifdef WINDOWS
	int actuallySent = ::send(sockfd, (const char*)buffer, size, 0);
	#else
	int actuallySent = ::send(sockfd, buffer, size, 0);
	#endif

	if(actuallySent != (int)size) {  // Unsafe check
		socketStatus = SocketStatus::ERROR;
		throw RUNTIME_ERROR("Error sending socket data");
	}
}*/

uint32_t TcpSocket::send(const void* buffer, const uint32_t size) {
	if (size > (uint32_t)INT_MAX)
		throw RUNTIME_ERROR("Size too big");
	if (!isConnected())
		throw RUNTIME_ERROR("It is impossible to send something using a non connected socket");

	#ifdef WINDOWS
	int actuallySent = ::send(sockfd, (const char*)buffer, size, 0);
	#else
	int actuallySent = ::send(sockfd, buffer, size, 0);
	#endif

	if (actuallySent < 0) {
		socketStatus = SocketStatus::ERROR;
		throw RUNTIME_ERROR("Error sending socket data");
	}

	return (uint32_t)actuallySent;
}

/*void TcpSocket::sendObject(const std::vector<byte_t>& buffer) {
	uint32_t size = buffer.size();
	sendOrDie(&size, sizeof(size));
	sendOrDie(buffer.data(), buffer.size());
}

void TcpSocket::sendObject(const void* buffer, const uint32_t size) {
	sendOrDie(&size, sizeof(size));
	sendOrDie(buffer, size);
}

void TcpSocket::sendString(const std::string& str) {
	uint32_t size = str.length();
	sendOrDie(&size, sizeof(size));
	sendOrDie(str.c_str(), size);
}*/

/*void TcpSocket::receiveOrDie(void* buffer, const uint32_t howManyBytes) {
	if(receive(buffer, howManyBytes) != (int)howManyBytes) {
		socketStatus = SocketStatus::ERROR;
		throw RUNTIME_ERROR("Error receving socket data");
	}
}*/

uint32_t TcpSocket::receive(void* buffer, const uint32_t size) {
	if (size > (uint32_t)INT_MAX) {
		throw RUNTIME_ERROR("Receiving too much");
	}
	if(!isConnected()) {
		throw RUNTIME_ERROR("It is impossible to send something using a non connected socket");
	}

	#ifdef WINDOWS
	int actuallyReceived = recv(sockfd, (char*)buffer, size, 0);
	#else
	//int actuallyReceived = recv(sockfd, buffer, howManyBytes, MSG_WAITALL);
	int actuallyReceived = recv(sockfd, buffer, size, 0);
	#endif

	if (actuallyReceived < 0) {
		socketStatus = SocketStatus::ERROR;
		throw RUNTIME_ERROR("Error receiving socket data");
	}

	return (uint32_t)actuallyReceived;
}

/*std::vector<byte_t> TcpSocket::receiveObject() {
	uint32_t size;
	receiveOrDie((byte_t*)&size, sizeof(size));
	if(size > maxObjectSize) {
		socketStatus = SocketStatus::ERROR;
		throw RUNTIME_ERROR("Peer wants to send an object too large");
	}
	std::vector<byte_t> buffer;
	try {
		buffer.resize(size);
	} catch(std::length_error& e) {
		socketStatus = SocketStatus::ERROR;
		throw RUNTIME_ERROR("Peer wants to send an object too large");
	} catch(std::bad_alloc& e) {
		socketStatus = SocketStatus::ERROR;
		throw RUNTIME_ERROR("Not enough memory to receive object");
	}
	receiveOrDie(buffer.data(), size);
	return buffer;
}

std::string TcpSocket::receiveString() {
	uint32_t size;
	receiveOrDie((byte_t*)&size, sizeof(size));
	if(size + 1 < size || size + 1 > maxObjectSize) {
		socketStatus = SocketStatus::ERROR;
		throw RUNTIME_ERROR("Peer wants to send a string too large");
	}
	std::string buffer;
	try {
		buffer.resize(size + 1);
	} catch(std::length_error& e) {
		socketStatus = SocketStatus::ERROR;
		throw RUNTIME_ERROR("Peer wants to send an object too large");
	} catch(std::bad_alloc& e) {
		socketStatus = SocketStatus::ERROR;
		throw RUNTIME_ERROR("Not enough memory to receive object");
	}
	receiveOrDie((void*)buffer.c_str(), size);
	buffer[size] = 0;
	return buffer;
}*/

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
		#ifdef WINDOWS
        return IPv4((uint32_t)addr.sin_addr.S_addr);
		#else
		return IPv4((uint32_t)addr.sin_addr.s_addr);
		#endif

    } else {

		throw RUNTIME_ERROR("Not yet implemented!");
        //IPv6 case
        struct sockaddr_in6 addr;
        socklen_t addr_size = sizeof(addr);
        if(getpeername(sockfd, (struct sockaddr*)&addr, &addr_size) < 0) {
            throw RUNTIME_ERROR("Error in getpeername(...)");
        }
        return IPv4(0u); //ipv6AddressToString(&addr.sin6_addr);  TODO

    }
}

/*uint32_t TcpSocket::getMaxObjectSize() const {
	return maxObjectSize;
}

bool TcpSocket::setMaxObjectSize(const uint32_t maxObjectSize) {
	if(maxObjectSize > ABSOLUTE_MAX_SOCKET_MAX_OBJECT_SIZE ||
	   maxObjectSize < ABSOLUTE_MIN_SOCKET_MAX_OBJECT_SIZE)
		return false;
	this->maxObjectSize = maxObjectSize;
	return true;
}*/

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
