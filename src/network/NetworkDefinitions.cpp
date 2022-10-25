#include <sstream>
#include <iomanip>
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
    uint32_t i;
    for (i = 0; i < sizeof(byteValues) - 1; ++i)
        ss << std::setfill('0') << std::setw(2) << (unsigned int)byteValues[i] << ":";
    ss << std::setfill('0') << std::setw(2) << (unsigned int)byteValues[i];
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
    *((uint32_t*)ipv6.value) = ipv4.value;
    for (size_t i = 1u; i < 4u; ++i)
        ((uint32_t*)ipv6.value)[i] = 0;
    return ipv6;
}
