#ifndef __NETWORK_DEFINITIONS_H__
#define __NETWORK_DEFINITIONS_H__


#include "../common.h"
#include "./NetworkUtility.h"


namespace Gicame {


    /*
     * Types and enums
     */

    typedef uint32_t IPv4NetMask;

    #ifdef WINDOWS
    typedef SOCKET SocketDescriptor;
    #else
    typedef int SocketDescriptor;
    #endif

    enum class InternetProtocolVersion { IPv4, IPv6 };
    enum class SocketStatus { CONNECTED, CLOSED, SS_ERROR };


    /*
     * Constants
     */

    static constexpr IPv4NetMask NETMASK_32BIT = 0xFFFFFFFFU;
    static constexpr IPv4NetMask NETMASK_24BIT = 0x00FFFFFFU;
    static constexpr IPv4NetMask NETMASK_16BIT = 0x0000FFFFU;
    static constexpr IPv4NetMask NETMASK_8BIT = 0x000000FFU;


    /*
     * Struct and classes
     */

    struct GICAME_API IPv4 {
        union {
            uint32_t value;         // Main data type
            uint8_t byteValues[4];  // Alternative type
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

    struct GICAME_API IPv6 {
        uint8_t value[16];
        constexpr IPv6() : value{} {}
        IPv6(const std::string& strValue);
        std::string toString() const;
        in6_addr toIn6Addr() const;
        bool operator==(const IPv6& other) const;
        static IPv6 fromIPv4(const IPv4& ipv4);
    };

	struct GICAME_API MacAddress {
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
     * IPv4 inline implementation
     */

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
     * IPv6 inline implementation
     */

    inline bool IPv6::operator==(const IPv6& other) const {
        for (size_t i = 0; i < 16u / sizeof(size_t); ++i)
            if (((size_t*)(value))[i] != ((size_t*)(other.value))[i])
                return false;
        return true;
    }


    /*
     * MacAddress inline implementation
     */

     inline bool MacAddress::operator==(const MacAddress& other) const {
         return value == other.value;
     }

};


#endif
