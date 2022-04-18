#ifndef __NETWORK_DEFINITIONS_H__
#define __NETWORK_DEFINITIONS_H__


#include "common.h"
#include "NetworkUtility.h"
#include <sstream>
#include <iomanip>


namespace Gicame {

    /*
     * Constants
     */

    static constexpr uint32_t NETMASK_32BIT = 0xFFFFFFFFU;
    static constexpr uint32_t NETMASK_24BIT = 0x00FFFFFFU;
    static constexpr uint32_t NETMASK_16BIT = 0x0000FFFFU;
    static constexpr uint32_t NETMASK_8BIT  = 0x000000FFU;


    /*
     * Types and enums
     */

    typedef uint32_t IPv4NetMask;

    #ifdef WINDOWS
    typedef SOCKET SocketDescriptor;
    #else
    typedef int SocketDescriptor;
    #endif

    enum class InternetProtocolVersion {IPv4, IPv6};


    /*
     * Struct and classes
     */
    
    struct IPv4 {
        union {
            uint32_t value;
            uint8_t byteValues[4];
        };
        constexpr IPv4(const uint32_t value) : value(value) {}
        IPv4(const std::string& strValue);
        std::string toString() const;
        in_addr toInAddr() const;
        bool matchSubnet(const IPv4& subnet, const IPv4NetMask netmask = NETMASK_24BIT) const;
        IPv4 getSubnet(const IPv4NetMask netmask = NETMASK_24BIT) const;
        IPv4 broadcastAddress(const IPv4NetMask netmask = NETMASK_24BIT) const;
        bool isBroadcastAddress(const IPv4NetMask netmask = NETMASK_24BIT) const;
        bool operator==(const IPv4& other) const;
	};

	struct MacAddress {
        union {
            uint64_t value;
            uint8_t byteValues[6];
        };
        constexpr MacAddress() : value(0x0ULL) {}
        constexpr MacAddress(const uint64_t value) : value(value) {}
        std::string toString() const;
        bool operator==(const MacAddress& other) const;
	};


    /*
     * IPv4 implementation
     */

    inline IPv4::IPv4(const std::string& strValue) {
        inet_pton(AF_INET, strValue.c_str(), (void*)&value);
    }

    inline std::string IPv4::toString() const {
        char strBuffer[128] = { 0 };
        inet_ntop(AF_INET, &value, strBuffer, sizeof(strBuffer));
        return std::string(strBuffer);
    }

    inline in_addr IPv4::toInAddr() const {
        in_addr addr;
        #ifdef WINDOWS
        addr.S_addr = value;
        #else
        addr.s_addr = value;
        #endif
        return addr;
    }

    inline bool IPv4::matchSubnet(const IPv4& subnet, const IPv4NetMask netmask) const {
        return (value & netmask) == (subnet.value & netmask);
    }

    inline IPv4 IPv4::getSubnet(const IPv4NetMask netmask) const {
        return IPv4(value & netmask);
    }

    inline IPv4 IPv4::broadcastAddress(const IPv4NetMask netmask) const {
        return IPv4(value | ~netmask);
    }

    inline bool IPv4::isBroadcastAddress(const IPv4NetMask netmask) const {
        return (value & ~netmask) == ~netmask;
    }

    inline bool IPv4::operator==(const IPv4& other) const {
        return value == other.value;
    }


    /*
     * MacAddress implementation
     */

     inline std::string MacAddress::toString() const {
         std::stringstream ss;
         ss << std::hex;
         uint32_t i;
         for (i = 0; i < sizeof(byteValues) - 1; ++i)
             ss << std::setfill('0') << std::setw(2) << (unsigned int)byteValues[i] << ":";
         ss << std::setfill('0') << std::setw(2) << (unsigned int)byteValues[i];
         return ss.str();
     }

     inline bool MacAddress::operator==(const MacAddress& other) const {
         return value == other.value;
     }

};


#endif
