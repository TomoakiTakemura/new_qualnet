// Copyright (c) 2001-2015, SCALABLE Network Technologies, Inc.  All Rights Reserved.
//                          600 Corporate Pointe
//                          Suite 1200
//                          Culver City, CA 90230
//                          info@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the QualNet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

/// \defgroup Package_MAIN MAIN

/// \file
/// \ingroup Package_MAIN
/// This file contains some common definitions.

/// \mainpage
/// This documents the QualNet APIs.

#ifndef MAIN_H
#define MAIN_H

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#endif

#ifdef _WIN32
    #define snprintf _snprintf
#endif

#include "types.h"
#include <stdlib.h>
#include <math.h>

// Use Linux version of ipv6
#ifndef WINDOWS_OS
#include "netinet/in.h"
#endif // WINDOWS_OS

///
/// Maximum number of Physical channel
#define MAX_NUM_PHYS       64

///
/// Maximum number of Interfaces.

#define MAX_NUM_INTERFACES 2048

/// Length Field value for protocol IP TYPE
#define PROTOCOL_TYPE_IP                   0x0800

/// ARP type
#define PROTOCOL_TYPE_ARP                            0x0806

#ifndef HAS_UTIL_COMPARATOR
# define HAS_UTIL_COMPARATOR
/// Comparison operator (compatibility)
typedef BOOL (*UTIL_Comparator)(void*,void*);
#endif // HAS_UTIL_COMPARATOR


/// Node Identifier
typedef unsigned NodeId;

/// NodeAddress is used from the phy layer to the application layer
/// for IP, MAC, and phy addresses.  It is an unsigned 32-bit
/// integer.  Note that IPv4 addresses are 32-bits long (IPv6 addresses
/// are 128 bits), and MAC addresses are typically 48-bit.
typedef unsigned NodeAddress;

// This is done to avoid conflict with the
// system defined in6_addr in solaris and linux,
// when gui.cpp includes in.h for socket creation
// and socket addressing.
#ifndef _NETINET_IN_H
#ifndef _NETINET_IN_H_ // For OpenBSD
#ifndef __NETINET_IN_H__ // For IRIX
#define _NETINET_IN_H_
#define __NETINET_IN_H__
#ifndef _WS2IPDEF_

/// Describes the IPv6 address
typedef struct in6_addr_struct
{
    union
    {
        UInt32 u6_addr32[4];
        UInt16 u6_addr16[8];
        UInt8  u6_addr8[16];
    } in6_u;
} in6_addr;

#define s6_addr32   in6_u.u6_addr32
#define s6_addr16   in6_u.u6_addr16
#define s6_addr     in6_u.u6_addr8

#endif // _WS2IPDEF_
#endif // __NETINET_IN_H__
#endif // _NETINET_IN_H_
#endif // _NETINET_IN_H

#ifdef __APPLE__
#ifndef s6_addr32
#define s6_addr32   __u6_addr.__u6_addr32
#endif
#ifndef s6_addr16
#define s6_addr16   __u6_addr.__u6_addr16
#endif
#endif

#define s6_addr8 s6_addr

/// Enlisted different network type
enum NetworkType
{
    NETWORK_INVALID,
    NETWORK_IPV4,
    NETWORK_IPV6,
    NETWORK_ATM,
    NETWORK_DUAL
};

// Any ICD AES Address maintains the following 20 octets address

// AFI(AddressFormat Identifier)             : 1 octet
// ICD (International Code designator)       : 2 octet
// AssignedId by the ICD client              : 4 octet
// DeviceAddr (MAC Address)                  : 6 octet
// ESI (End System Identifier)               : 6 octet
// SEL (Selection field)                     : 1 octet


/// Describes the ATM address
struct AtmAddress
{
    unsigned char AFI;              //Authority and Format Identifier.
    unsigned char SEL;              //Selector, unused.
    unsigned short ICD;             //International Code Designator.
    unsigned short aid_pt1;         //Assigned Identifier.
    unsigned short aid_pt2;         //PTP value, specified into input file.
    unsigned int deviceAddr_pt1;    //Store the value hw Addr.
    unsigned short deviceAddr_pt2;  //Store the value hw Addr
    unsigned short ESI_pt2;         //End System Identifier this part,
                                    //store interface Id.
    unsigned int ESI_pt1;           //End System Identifier other part,
                                    //store node Id.
};


/// Describes the generic address union
union GenericAddress
{
    NodeAddress ipv4;
    in6_addr    ipv6;
    AtmAddress  atm;
};


/// Describes the address structure
/// which contains the interface address
/// and network type
struct Address
{
    NetworkType networkType;
    GenericAddress   interfaceAddr;

    bool operator< (const Address& rhs) const;
    bool operator> (const Address& rhs) const;
};


/// Get IPv4 address from generic address
///
/// \param addr  generic address.
///
/// \return IPv4 address
inline NodeAddress GetIPv4Address(Address addr)
    { return addr.interfaceAddr.ipv4;};


/// Get IPv6 address from generic address
///
/// \param addr  generic address.
///
/// \return IPv6 address
inline in6_addr GetIPv6Address(Address addr)
    { return addr.interfaceAddr.ipv6;};


/// Set IPv4 address and network type to generic
/// address
///
/// \param address  generic address.
/// \param addr  IPv4 interface address.
///
inline void SetIPv4AddressInfo(Address* address, NodeAddress addr)
    { address->interfaceAddr.ipv4 = addr;
      address->networkType = NETWORK_IPV4;};


/// Set IPv6 address and network type to generic
/// address
///
/// \param address  generic address.
/// \param addr  IPv6 interface address.
///
inline void SetIPv6AddressInfo(Address* address, in6_addr& addr)
    { address->interfaceAddr.ipv6 = addr;
      address->networkType = NETWORK_IPV6;};


///
/// This is a special addresses used in the MAC and network
/// layers. It defines any destination.
#define ANY_DEST        0xffffffff


///
/// This is a special addresses used in the MAC and network
/// layers. It defines any destination of six byte.
const char ANY_MAC802[6]={'\xff', '\xff', '\xff', '\xff', '\xff', '\xff'};

///
/// This is a special addresses used in the MAC and network
/// layers. It is used for invalid address
const char INVALID_802ADDRESS[6] = {'\xff', '\xff', '\xff', '\xff', '\xff', '\xfe'};



///
/// This is a special addresses used in the MAC and network
/// layers. It defines any source.
#define ANY_SOURCE_ADDR      0xffffffff

///
/// This is a special addresses used in the MAC and network
/// layers. It defines any IP.
#define ANY_IP          0xffffffff

///
/// This is a special addresses used in the MAC and network
/// layers. It defines any Interface.
#define ANY_INTERFACE   -1

///
/// This is a special addresses used in the MAC and network
/// layers. It defines CPU Interface.
#define CPU_INTERFACE   -2

///
/// It defines Invalid Address. Used only by mac/mac_802_11.c.
#define INVALID_ADDRESS 987654321

#ifndef HAS_UTIL_COMPARATOR
#define HAS_UTIL_COMPARATOR
typedef BOOL (*UTIL_Comparator)(void*,void*);
#endif

///
/// Generic maximum length of a string. The maximum length of any
/// line in the input file is 3x this value.
#define MAX_STRING_LENGTH 200

///
/// maximum length of a string.
#define BIG_STRING_LENGTH 512

///
/// Generic maximum length of a clock string.
#define MAX_CLOCK_STRING_LENGTH 24

///
/// Generic maximum length of a HID string.
#ifndef MAX_HID_LENGTH
#define MAX_HID_LENGTH 55
#endif // MAX_HID_LENGTH

///
/// Utility function MAX. Calculates the Maximum one from two
/// given numbers.
#ifndef MAX
#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))
#endif
//template <class T> T MAX(T a, T b)
//{
//    if (a < b) return b;
//    return a;
//}

///
/// Utility function MIN. Calculates the Minimum one from two
/// given numbers.
#ifndef MIN
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#endif
//template <class T> T MIN(T a, T b)
//{
//    if (a < b) return a;
//    return b;
//}

///
/// Utility function ABS. Return the absolute value of a
/// given number.
#ifdef ABS
#undef ABS
#endif
#define ABS(a) std::abs(a)

///
/// Utility function, decibel converter. Performs the 10 base
/// log operation on the given number and then multiply with 10.
#define IN_DB(x)  (10.0 * log10(x))

// Here -120 is a very small value (its close enough to log10(0) for
// our purposes.
inline double in_db(double value) {
    if (value <= 0.0) return -120;
    return IN_DB(value);
}

inline double in_db(double value, int denominator) {
    if (value <= 0.0) return -120.0;
    if (denominator == 0) return -120.0;
    return IN_DB(value/denominator);
}

///
/// Utility function, decibel converter. Performs power operation
/// on the given number.
#define NON_DB(x) (pow(10.0, (x) / 10.0))

/// Round a float point number to an integer. This function
/// tries to get consistent value on different platforms
///
/// \param x  The float point number to be rounded
///
/// \return Returns the rounded integer
inline
int RoundToInt(double x)
{
    int a,b;

    a = (int)floor(x);

    if (x > (a + 0.51))
        b = a + 1;
    else
        b = a;

    return b;
}

/// Adds filename and line number parameters to
/// the MEM_malloc function
#define MEM_malloc(size) MEM_Malloc(size, __FILE__, __LINE__)

/// Allocates memory block of a given size.
///
/// \param size  Size of the memory block to be allocated.
/// \param filename  Name of file allocating the memory
/// \param lineno  Line in the file where the API is called
///
/// \return Returns the pointer of allocated memory
/// otherwise NULL if allocation fails.
void *MEM_Malloc(size_t size, const char* filename, int lineno);

/// Deallocates the memory in turn it calls free().
///
/// \param ptr  Pointer of memory to be freed.
void MEM_free(void *ptr);

///
/// Defines the Maximum Network Packet Size which can handled by
/// the physical network. In QualNet, its value is 2048. Packet
/// larger than this will be fragmented by IP.
#define MAX_NW_PKT_SIZE       2048

///
/// Defines the Minimum Network Packet Size which can be handled by
/// the physical network. In QualNet, its value is 40. Packets
/// smaller than this will not have room for transport headers
/// and most firewall-type devices will drop an initial fragment
/// that does not contain enough data to hold the transport headers.
#define MIN_NW_PKT_SIZE       40

///
/// Defines the Minimum Network Packet Size which can be handled by
/// the IPv6 physical network. In QualNet, its value is 60. Packets
/// smaller than this will not have room for transport headers
/// and most firewall-type devices will drop an initial fragment
/// that does not contain enough data to hold the transport headers.
/// The additional space is to allow for IPv6's larger headers.
#define MIN_IPv6_PKT_SIZE       60

///
/// enum for the various layers in QualNet. New layers added to the
/// simulation should be added here as well.
///
/// Used by models at all layers in the protocol stack to mark newly
/// created messages to be destined to the right layer/module.
enum
{
    // Special control information
    MOBILITY_SPECIAL,
    PARTITION_COMMUNICATION,

    // Various layers
    PROP_LAYER,
    PHY_LAYER,
    MAC_LAYER,
    NETWORK_LAYER,
    TRANSPORT_LAYER,
    APP_LAYER,
    UTIL_LAYER,
    WEATHER_LAYER, // probably will rename to ENVIRONMENT_LAYER
    EXTERNAL_LAYER,
    DYNAMIC_LAYER,
    CONFIG_LAYER,

    //receive-only
    MI_LAYER,

    ATM_LAYER2,
    ADAPTATION_LAYER,

    USER_LAYER,
//#ifdef BATTERY_MODEL
    BATTERY_MODEL,
//#endif
    QOS_LAYER,
    STATSDB_LAYER,
    STATSDB_CONTROLLER_LAYER,
    // Any other layers which have to be added should be added before
    // DEFAULT_LAYER.  Otherwise the program will not work correctly.
    DEFAULT_LAYER
};

struct AdaptationData;
struct AntennaModelGlobal;
struct AntennaPattern;
struct AppData;
struct IpHeaderType;
struct MacData;
struct MacHWAddress;
struct MacSwitch;
class  Message;
struct MobilityData;
struct NetworkData;
struct Node;
struct NodeInput;
struct NodePositions;
class  PARALLEL_PropDelay_NodePositionData;
struct PartitionData;
struct PhyData;
struct PropChannel;
struct PropData;
struct PropProfile;
struct PropRxInfo;
struct PropTxInfo;
class  TimerManager;
struct TrafficPatternDataMapping;
struct TransportData;
struct UserData;
struct UserProfileDataMapping;
class  WallClock;
struct WirelessLinkSiteParameters;

typedef unsigned short                  VlanId;

/// Return 1's in all bit positions between sposition
/// and eposition
///
/// \param sposition  starting bit position
/// \param eposition  last bit position set to 1
///
/// \return UInt8
// ASSUMPTION          :: Bit Position starts from 1
inline UInt8 maskChar(UInt8 sposition, UInt8 eposition)
{
    if (eposition >= sposition)
    {
      return (UInt8((UInt8)(0xff) << (8 - ((eposition-sposition) + 1)))
      >> (sposition - 1));
    }
    else
    {
      return 0;
    }
};


/// Return 1's in all bit positions between sposition
/// and eposition
///
/// \param sposition  starting bit position
/// \param eposition  last bit position set to 1
///
/// \return UInt16
// ASSUMPTION          :: Bit Position starts from 1
inline UInt16 maskShort(UInt16 sposition, UInt16 eposition)
{
    if (eposition >= sposition)
    {
      return (UInt16((UInt16)(0xffff) << (16 - ((eposition-sposition) + 1)))
      >> (sposition - 1));
    }
    else
    {
      return 0;
    }
};


/// Return 1's in all bit positions between sposition
/// and eposition
///
/// \param sposition  starting bit position
/// \param eposition  last bit position set to 1
///
/// \return UInt32
// ASSUMPTION          :: Bit Position starts from 1
inline UInt32  maskInt (int sposition, int eposition)
{
    if (eposition >= sposition)
    {
      return (UInt32((UInt32)(0xffffffff) << (32 - ((eposition-sposition) +
          1))) >> (sposition - 1));
    }
    else
    {
      return 0;
    }
};


/// Left shifts data where eposition determines the
/// position of thelast bit after the shift and
/// (size-eposition) determines the number of bits to
/// be shifted
///
/// \param x  the data to be shifted
/// \param eposition  last bit position set to 1
///
/// \return UInt8
// ASSUMPTION          :: Bit Position starts from 1
inline UInt8 LshiftChar(UInt8 x, UInt8 eposition)
{
    return ((UInt8)(x << (8 - eposition)));
};


/// Left shifts data where eposition determines the
/// position of thelast bit after the shift and
/// (size-eposition) determines the number of bits to
/// be shifted
///
/// \param x  the data to be shifted
/// \param eposition  last bit position set to 1
///
/// \return UInt16
// ASSUMPTION          :: Bit Position starts from 1
inline UInt16 LshiftShort(UInt16 x, UInt16 eposition)
{
    return ((UInt16)(x << (16 - eposition)));
};


/// Left shifts data where eposition determines the
/// position of thelast bit after the shift and
/// (size-eposition) determines the number of bits to
/// be shifted
///
/// \param x  the data to be shifted
/// \param eposition  last bit position set to 1
///
/// \return UInt32
// ASSUMPTION          :: Bit Position starts from 1
inline UInt32 LshiftInt(UInt32 x, int eposition)
{
    return (x << (32 - eposition));
};


/// Right shifts data where eposition determines the
/// position of thelast bit after the shift and
/// (size-eposition) determines the number of bits to
/// be shifted
///
/// \param x  the data to be shifted
/// \param eposition  last bit position set to 1
///
/// \return UInt8
// ASSUMPTION          :: Bit Position starts from 1
inline UInt8 RshiftChar(UInt8 x, UInt8 eposition)
{
    return ((unsigned char)(x >> (8 - eposition)));
};


/// Right shifts data where eposition determines the
/// position of thelast bit after the shift and
/// (size-eposition) determines the number of bits to
/// be shifted
///
/// \param x  the data to be shifted
/// \param eposition  last bit position set to 1
///
/// \return UInt16
// ASSUMPTION          :: Bit Position starts from 1
inline UInt16 RshiftShort(UInt16 x, UInt16 eposition)
{
    return ((UInt16)(x >> (16 - eposition)));
};


/// Right shifts data where eposition determines the
/// position of thelast bit after the shift and
/// (size-eposition) determines the number of bits to
/// be shifted
///
/// \param x  the data to be shifted
/// \param eposition  last bit position set to 1
///
/// \return UInt32
// ASSUMPTION          :: Bit Position starts from 1
inline UInt32 RshiftInt(UInt32 x, int eposition)
{
    return (x >> ( 32 - eposition));
};


#endif // _MAIN_H_

