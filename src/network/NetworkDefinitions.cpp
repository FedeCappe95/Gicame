#include <sstream>
#include <iomanip>
#include "common.h"
#include "network/NetworkDefinitions.h"

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

#include <cstring>


#ifdef WINDOWS
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#endif


using namespace Gicame;


IPv4::IPv4(const std::string& strValue) {
    inet_pton(AF_INET, strValue.c_str(), (void*)&value);
}

std::string IPv4::toString() const {
    char strBuffer[128] = { 0 };
    inet_ntop(AF_INET, &value, strBuffer, sizeof(strBuffer));
    return std::string(strBuffer);
}

in_addr IPv4::toInAddr() const {
    in_addr addr;
    addr.s_addr = value;
    return addr;
}

std::string MacAddress::toString() const {
    std::stringstream ss;
    ss << std::hex;

    uint64_t tempValue = value;
    for (size_t i = 0; i < 5u; ++i, tempValue >>= 8)
        ss << std::setfill('0') << std::setw(2) << (unsigned int)(tempValue & 0xFFull) << ":";
    ss << std::setfill('0') << std::setw(2) << (unsigned int)(tempValue & 0xFFull);

    return ss.str();
}

IPv6::IPv6(const std::string& strValue) {
    inet_pton(AF_INET6, strValue.c_str(), (void*)&value);
}

std::string IPv6::toString() const {
    char strBuffer[128] = { 0 };
    inet_ntop(AF_INET6, &value, strBuffer, sizeof(strBuffer));
    return std::string(strBuffer);
}

in6_addr IPv6::toIn6Addr() const {
    return *((in6_addr*)this);
}

IPv6 IPv6::fromIPv4(const IPv4& ipv4) {
    IPv6 ipv6;

    for (size_t i = 4; i < ipv6.value.size(); ++i)
        ipv6.value[i] = 0;

#ifdef MSVC
#pragma warning (push, 0)
#endif
    memcpy(ipv6.value.data(), &(ipv4.value), sizeof(ipv4.value));
#ifdef MSVC
#pragma warning (pop)
#endif

    return ipv6;
}
