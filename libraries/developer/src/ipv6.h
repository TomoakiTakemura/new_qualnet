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

/// \defgroup Package_IPv6 IPv6

/// \file
/// \ingroup Package_IPv6
/// Data structures and parameters used in network layer
/// are defined here.

// Objective: Header file for IPv6 (Internet Protocol version 6)
// Reference: RFC 2460, RFC 2461

#ifndef IPV6_H
#define IPV6_H

#include "api.h"
#include "ipv6_route.h"
#include "ip6_opts.h"
#include "network_ip.h"
#include "ip6_input.h"
#include "ipv6_auto_config.h"

/// Maximum Key length of ipv6 address.
#define MAX_KEY_LEN 128

/// Maximum Prefix length of ipv6 address.
#define MAX_PREFIX_LEN 64

/// Current Hop limit a packet will traverse.
#define CURR_HOP_LIMIT  255

/// Ipv6 Address Lenght.
#define IPV6_ADDR_LEN       16

/// Hop-by_hop IPv6 Next header field value.
#define IP6_NHDR_HOP    0

/// Routing IPv6 Next header field value.
#define IP6_NHDR_RT     43

/// Fragment IPv6 Next header field value.
#define IP6_NHDR_FRAG   44

/// Authentication IPv6 Next header field value.
#define IP6_NHDR_AUTH   51

/// Encryption IPv6 Next header field value.
#define IP6_NHDR_ESP    50

/// Compression IPv6 Next header field value.
#define IP6_NHDR_IPCP   108

/// Compression IPv6 Next header field value.
#define IP6_NHDR_OSPF   89

/// Destination IPv6 Next header field value.
#define IP6_NHDR_DOPT   60  // destination options IPv6 header

/// No next header IPv6 Next header field value.
#define IP6_NHDR_NONH   59

/// Flow infromation version.
#define IPV6_FLOWINFO_VERSION   0x0000000f

/// IPv6 version no.
#define IPV6_VERSION            6

/// Minimal MTU and reassembly.
#define IP6_MMTU            1280

// If not Using Linux version of ipv6
#ifndef _NETINET_IN_H

/// ICMPv6 protocol no.
#define IPPROTO_ICMPV6      58
#endif

/// IPv6 anycast.
#define IP6ANY_ANYCAST      3

/// Node Discovery hop count.
#define ND_DEFAULT_HOPLIM       255

/// Node Discovery sets class.
#define ND_DEFAULT_CLASS        IPV6_SET_CLASS(0xe0)

/// IPv6 insert option with no allocation.
#define IP6_INSOPT_NOALLOC  1

/// IPv6 insert raw option.
#define IP6_INSOPT_RAW      2

/// IPv6 forwarding flag.
#define IP_FORWARDING       1

/// Reserved fragment flag.
#define IP6F_RESERVED_MASK 0x0600

/// Don't fragment flag.
#define IP_DF 0x4000

/// More fragments flag.
#define IP6F_MORE_FRAG 0x01

/// Mask for fragmenting bits.
#define IP6F_OFF_MASK 0xf8ff

/// Time to live for frags.
#define IP6_FRAGTTL 120

// Multicast Related Constants.
/// T Flag if set indicates transient multicast address.
#define IP6_T_FLAG 0x10

#define IP6_MULTI_INTERFACE_SCOPE 0x01
#define IP6_MULTI_LINK_SCOPE 0x02
#define IP6_MULTI_ADMIN_SCOPE 0x04
#define IP6_MULTI_SITE_SCOPE 0x05
#define IP6_MULTI_ORG_SCOPE 0x08
#define IP6_MULTI_GLOBAL_SCOPE 0x0e

/// IP Fragment hold time.
#define IP_FRAGMENT_HOLD_TIME  60 * SECOND

/// IPv6 route to interface.
#define IP_ROUTETOIF        4

/// IPv6 route to interface.
#ifndef IP_DEFAULT_MULTICAST_TTL
#define IP_DEFAULT_MULTICAST_TTL 255
#endif


/// TTL decrement.
#define IPTTLDEC 1


/// Network unreachable.
#ifndef ENETUNREACH
#define ENETUNREACH         1
#endif

/// Host unreachable.
#ifndef EHOSTUNREACH
#define EHOSTUNREACH        2
#endif

/// Router Advertisement timer.
#define MAX_INITIAL_RTR_ADVERT_INTERVAL  16 * SECOND

/// Maximum Router Advertisement.
#define MAX_INITIAL_RTR_ADVERTISEMENTS  3

/// Maximum Router Advertisement timer.
#define MAX_RTR_ADVERT_INTERVAL  (600 * SECOND)

/// Minimum Router Advertisement timer.
#define MIN_RTR_ADVERT_INTERVAL  (MAX_RTR_ADVERT_INTERVAL * 0.33)

/// Router Solicitation timer.
#define RTR_SOLICITATION_INTERVAL  4 * SECOND

/// reachable time
#define REACHABLE_TIME          (30 * SECOND)

/// unreachable time
#define UNREACHABLE_TIME          (30 * SECOND)

/// retransmission timer
#define RETRANS_TIMER           (2 * SECOND)

/// maximum neighbor advertisement
#define MAX_NEIGHBOR_ADVERTISEMENT    3

/// maximum Router Solicitations
/// NOTE         : Sending only one Solicitation; modify it once
/// autoconfiguration supported.
#define MAX_RTR_SOLICITATIONS    1

/// maximum multicast solicitation
#define MAX_MULTICAST_SOLICIT    3

/// maximum unicast solicitation
#define MAX_UNICAST_SOLICIT    3

/// Packet expiration interval
#define PKT_EXPIRE_DURATION  (MAX_MULTICAST_SOLICIT * RETRANS_TIMER)

/// Invalid Link Layer Address
#define INVALID_LINK_ADDR   -3



/// Maximum size of Hash-Table
//#define MAX_HASHTABLE_SIZE       4
#define MAX_HASHTABLE_SIZE       20

/// Maximum Rev Look up hash table size
#define MAX_REVLOOKUP_SIZE 100

/// Length of IPv6 header
#define IPV6_HEADER_LENGTH  40


/// QualNet typedefs struct ip6_hdr_struct to ip6_hdr.
/// struct ip6_hdr_struct is 40 bytes,
/// just like in the BSD code.
typedef struct ip6_hdr_struct
{
    UInt32 ipv6HdrVcf;//ip6_v:4,        // version first, must be 6
                 //ip6_class:8,    // traffic class
                 //ip6_flow:20;    // flow label
#ifndef ip6_plen
    unsigned short ip6_plen;     // payload length
#else
    unsigned short ip6_plength;
#endif

#ifndef ip6_nxt
    unsigned char  ip6_nxt;      // next header
#else
    unsigned char ip6_next;
#endif

#ifndef ip6_hlim
    unsigned char  ip6_hlim;     // hop limit
#else
    unsigned char ip6_hoplim;
#endif

    in6_addr ip6_src;            // source address
    in6_addr ip6_dst;            // destination address
} ip6_hdr;

#ifdef _WIN32
#define ipv6_plength ip6_plength
#define ipv6_nhdr    ip6_next
#define ipv6_hlim    ip6_hoplim
#else
#define ipv6_plength ip6_plen
#define ipv6_nhdr    ip6_nxt
#define ipv6_hlim    ip6_hlim
#endif



/// Set the value of version for ip6_hdr
///
/// \param ipv6HdrVcf  The variable containing the value of ip6_v,ip6_class
///    and ip6_flow
/// \param version  Input value for set operation
///
inline void ip6_hdrSetVersion(UInt32 *ipv6HdrVcf, UInt32 version)
{
    //masks ip6_v within boundry range
    version = version & maskInt(29, 32);

    //clears the first four bits of ipv6HdrVcf
    *ipv6HdrVcf = *ipv6HdrVcf & maskInt(5, 32);

    //Setting the value of version in ipv6HdrVcf
    *ipv6HdrVcf = *ipv6HdrVcf | LshiftInt(version, 4);
}


/// Set the value of class for ip6_hdr
///
/// \param ipv6HdrVcf  The variable containing the value of ip6_v,ip6_class
///    and ip6_flow
/// \param ipv6Class  Input value for set operation
///
inline void ip6_hdrSetClass(UInt32 *ipv6HdrVcf, unsigned char ipv6Class)
{
    unsigned int ipv6Class_int = (unsigned int)ipv6Class;

    //masks ip6_class within boundry range
    ipv6Class_int = ipv6Class_int & maskInt(25, 32);

    //clears the 5-12 bits of ipv6HdrVcf
    *ipv6HdrVcf = *ipv6HdrVcf & (~(maskInt(5, 12)));

    //Setting the value of class in ipv6HdrVcf
    *ipv6HdrVcf = *ipv6HdrVcf | LshiftInt(ipv6Class_int, 12);
}


/// Set the value of flow for ip6_hdr
///
/// \param ipv6HdrVcf  The variable containing the value of ip6_v,ip6_class
///    and ip6_flow
/// \param flow  Input value for set operation
///
inline void ip6_hdrSetFlow(UInt32 *ipv6HdrVcf, UInt32 flow)
{
    //masks ip6_flow within boundry range
    flow = flow & maskInt(13, 32);

    //clears the last 20 bits of ipv6HdrVcf
    *ipv6HdrVcf = *ipv6HdrVcf & maskInt(1, 12);

    //Setting the value of flow in ipv6HdrVcf
    *ipv6HdrVcf = *ipv6HdrVcf | flow;
}


/// Returns the value of version for ip6_hdr
///
/// \param ipv6HdrVcf  The variable containing the value of ip6_v,ip6_class
///    and ip6_flow
///
/// \return UInt32
inline unsigned int ip6_hdrGetVersion(unsigned int ipv6HdrVcf)
{
    unsigned int version = ipv6HdrVcf;

    //clears the first 4 bits
    version = version & maskInt(1, 4);

    //right shifts so that last four bits represent version
    version = RshiftInt(version, 4);

    return version;
}

/// Returns the value of ip6_class for ip6_hdr
///
/// \param ipv6HdrVcf  The variable containing the value of ip6_v,ip6_class
///    and ip6_flow
///
/// \return UInt32
inline unsigned char ip6_hdrGetClass(unsigned int ipv6HdrVcf)
{
    unsigned int ipv6Class = ipv6HdrVcf;

    //clears all the bits except 5-12
    ipv6Class = ipv6Class & maskInt(5, 12);

    //right shifts so that last 8 bits represent class
    ipv6Class = RshiftInt(ipv6Class, 12);

    return (unsigned char)ipv6Class;
}


/// Returns the value of ip6_flow for ip6_hdr
///
/// \param ipv6HdrVcf  The variable containing the value of ip6_v,ip6_class
///    and ip6_flow
///
/// \return UInt32
inline unsigned int ip6_hdrGetFlow(unsigned int ipv6HdrVcf)
{
    unsigned int ipv6_flow = ipv6HdrVcf;

    //clears the 1-12 bits
    ipv6_flow = ipv6_flow & maskInt(13, 32);

    return ipv6_flow;
}


/// QualNet typedefs struct in6_multi_struct to in6_multi.
/// struct in6_multi_struct is just like in the BSD code.
struct in6_multi_struct         //Taken from in6_var.h-new
{
    int mop;
    in6_addr inm6_addr;         // IPv6 multicast address
    int inm6_ifIndex;           // back pointer to ifIndex
    NodeAddress inm6_ifma;      // back pointer to ifmultiaddr
    unsigned int inm6_timer;    // ICMPv6 membership report timer
    unsigned int inm6_state;    // state of the membership
};
typedef struct in6_multi_struct in6_multi;

/// QualNet typedefs struct ipv6_h2hhdr_struct to ipv6_h2hhdr.
/// struct ipv6_h2hhdr_struct is hop-by-Hop Options Header of
/// 14 bytes, just like in the BSD code.
typedef struct ipv6_h2hhdr_struct
{
    unsigned char  ih6_nh;   // next header
    unsigned char  ih6_hlen; // header extension length
    unsigned short ih6_pad1; // to 4 byte length
    unsigned int   ih6_pad2; // to 8 byte length
} ipv6_h2hhdr;

// Routing Header.


/// type 0: loose source route
#define IP6_LSRRT   0   // type 0: loose source route


/// type 1: Nimrod
#define IP6_NIMRT   1   // type 1: Nimrod

/// QualNet typedefs struct ipv6_rthdr_struct to ipv6_rthdr.
/// struct ipv6_h2hhdr_struct is routing options header of
/// 8 bytes, just like in the BSD code.
typedef struct ipv6_rthdr_struct
{
    unsigned char  ir6_nh;    // next header
    unsigned char  ir6_hlen;  // header extension length
    unsigned char  ir6_type;  // routing type
    unsigned char  ir6_sglt;  // index of next address
    unsigned int   ir6_slmsk; // was strict/loose bit mask
} ipv6_rthdr;

/// Maximum number of addresses.
#define IP6_RT_MAX  23

/// QualNet typedefs struct ipv6_rthdr_struct to ipv6_rthdr.
/// struct ipv6_h2hhdr_struct is destination options header
/// of 8 bytes, just like in the BSD code.
typedef struct ipv6_dopthdr_struct
{
    unsigned char  io6_nh;      // next header
    unsigned char  io6_hlen;    // header extension length
    unsigned short io6_pad1;    // to 4 byte length
    unsigned int   io6_pad2;    // to 8 byte length
} ipv6_dopthdr;

/// QualNet typedefs struct ip_moptions_struct to ip_moptions.
/// struct ip_moptions_struct is multicast option structure,
/// just like in the BSD code.
struct ip_moptions_struct
{
    char* moptions;
    int imo_multicast_interface;
    int imo_multicast_ttl;
    int imo_multicast_loop;
};
typedef struct ip_moptions_struct ip_moptions;

/// QualNet typedefs struct ip6_frag_struct to ipv6_fraghdr.
/// struct ip6_frag_struct is fragmentation header structure.
typedef struct ip6_frag_struct
{
    unsigned char  if6_nh;       // next header
    unsigned char  if6_reserved; // reserved field
    unsigned short if6_off;      // offset, reserved, and flag
    unsigned int if6_id;         // identification
} ipv6_fraghdr;

/// QualNet typedefs struct ip6stat_struct to ip6Stat.
/// struct ip6stat_struct is statistic information structure.
typedef struct  ip6stat_struct
{
    unsigned int ip6_invalidHeader;
    unsigned int ip6s_noproto;      // unknown or unsupported protocol
    unsigned int ip6s_total;        // total packets received
    unsigned int ip6s_delivered;    // datagrams delivered to upper level
    unsigned int ip6s_forward;      // packets forwarded
    unsigned int ip6s_localout;     // total ipv6 packets generated here
    unsigned int ip6s_cantforward;  // packets rcvd for unreachable dest
    unsigned int ip6s_noroute;      // packets discarded due to no route
    unsigned int ip6s_outDiscarded; // packet output discarded.
    unsigned int ip6s_badvers;      // ipv6 version != 6
    unsigned int ip6s_badsource;    // packets rcvd from bad sources
    unsigned int ip6s_toobig;       // not forwarded because size > MTU
    unsigned int ip6s_toosmall;     // not enough data
    unsigned int ip6s_tooshort;     // packet too short
    unsigned int ip6s_fragmented;   // datagrams successfully fragmented
    unsigned int ip6s_ofragments;   // output fragments created
    unsigned int ip6s_fragments;    // fragments received
    unsigned int ip6s_fragdropped;  // fragments dropped
    unsigned int ip6s_fragtimeout;  // fragments timed out
    unsigned int ip6s_reassembled;  // total packets reassembled ok
    unsigned int ip6s_macpacketdrop;// MAC Packet Drop Notifications Received.
} ip6Stat;

// Function Pointer Type. Instance of this type is used to register
// Router Function.
typedef
void (*Ipv6RouterFunctionType)(
    Node *node,
    Message *msg,
    in6_addr destAddr,
    in6_addr previousHopAddress,
    BOOL *PacketWasRouted);

// Function Pointer Type. Instance of this type is used to register
// Router Function.
typedef
void (*Ipv6MacLayerStatusEventHandlerFunctionType)(
    Node* node,
    const Message* msg,
    const in6_addr nextHop,
    const int interfaceIndex);

/// Structure of an entity of multicast forwarding table.
typedef
struct
{
    in6_addr sourceAddress;
    unsigned char srcAddrPrefixLength;        // Not used
    in6_addr multicastGroupAddress;
    LinkedList *outInterfaceList;
} Ipv6MulticastForwardingTableRow;

/// Structure of multicast forwarding table
typedef
struct
{
    int size;
    int allocatedSize;
    Ipv6MulticastForwardingTableRow *row;
} Ipv6MulticastForwardingTable;

/// Structure for Multicast Group Entry
typedef struct
{
    in6_addr groupAddress;
    int memberCount;// Not used presently

    // Not used.
    int interfaceIndex;

} Ipv6MulticastGroupEntry;

/// QualNet typedefs struct ipv6_interface_struct to
/// IPv6InterfaceInfo. struct ipv6_interface_struct is
/// interface information structure.
typedef struct ipv6_interface_struct
{
    // Different IP addresses
    in6_addr linkLocalAddr;
    in6_addr siteLocalAddr;
    in6_addr globalAddr;
    unsigned int prefixLen;

    // multicast address
    BOOL multicastEnabled;
    in6_addr multicastAddr;

    BOOL is6to4Enabled;
    char interfaceName[10];

    // Maximum transmission unit through this interface
    int mtu;

    // Routing related information

    Ipv6RouterFunctionType     routerFunction;

    NetworkRoutingProtocolType routingProtocolType;
    void*                      routingProtocol;

    Ipv6MacLayerStatusEventHandlerFunctionType
        macLayerStatusEventHandlerFunction;

    // IPv6 auto-config
    Ipv6AutoConfigInterfaceParam autoConfigParam;
} IPv6InterfaceInfo;

/// QualNet typedefs struct messageBufferStruct to
/// messageBuffer. struct messageBufferStruct is the
/// buffer to hold messages when neighbour discovery
/// is not done.
struct messageBufferStruct
{
    Message* msg;
    in6_addr* dst;
    int inComing;
    rn_leaf* rn;
};
typedef struct messageBufferStruct messageBuffer;

/// QualNet typedefs struct ip6q_struct to
/// ip6q. struct ip6q is a simple queue to hold
/// fragmented packets.
typedef struct frag_data
{
    Message* msg;
    struct frag_data* nextMsg;
}Ipv6FragData;



/// Ipv6 fragment queue structure.
typedef struct ip6_frag_q_struct
{
    unsigned int ip6Frg_id;
    in6_addr ip6Frg_src;
    in6_addr ip6Frg_dst;
    unsigned int ip6Frg_nh;
    clocktype fragHoldTime;
    Ipv6FragData* firstMsg;
    Ipv6FragData* lastMsg;
    struct ip6_frag_q_struct* next;
}Ipv6FragQueue;

typedef struct ip6q_struct ip6q;

/// QualNet typedefs struct fragmeted_msg_struct to
/// ip6q. struct fragmeted_msg_struct is a simple
/// fragmented packets msg hold structure.
struct fragmeted_msg_struct
{
    Message* msg;
    struct fragmeted_msg_struct* next;
};

typedef struct fragmeted_msg_struct FragmetedMsg;



/// default router list structure.
struct default_router_list
{
    in6_addr routerAddr;
    int outgoingInterface;
    MacHWAddress linkLayerAddr;
    struct default_router_list* next;
};
typedef struct default_router_list defaultRouterList;

/// QualNet typedefs struct destination_route_struct to
/// destinationRoute. struct destination_route_struct is
/// destination information structure of a node.
struct route_struct;

struct destination_route_struct
{
    struct route_struct* ro;
    unsigned char inUse;
    struct destination_route_struct* nextRoute;
};

typedef struct destination_route_struct DestinationRoute;



/// Destination cache entry structure
typedef struct destination_cache_struct
{
    DestinationRoute* start;
    DestinationRoute* end;
} DestinationCache;

// Hash data structure.


/// Ipv6 hash data structure.
struct ipv6_hash_data_struct
{
    void* data;
    struct ipv6_hash_data_struct* nextDataPtr;
};
typedef struct ipv6_hash_data_struct Ipv6HashData;



/// Ipv6 hash block-data structure.
typedef struct ipv6_hash_blockdata_struct
{
    Ipv6HashData* firstDataPtr;
    Ipv6HashData* lastDataPtr;
    in6_addr ipAddr;
}Ipv6HashBlockData;



/// Ipv6 hash block structure.
struct ipv6_hash_block_struct
{
    unsigned int totalDataElements;
    Ipv6HashBlockData* blockData;
    struct ipv6_hash_block_struct* nextBlock;
};
typedef struct ipv6_hash_block_struct Ipv6HashBlock;



/// Ipv6 hash table structure
typedef struct ipv6_hash_table_struct
{
    Ipv6HashBlock* firstBlock;
}Ipv6HashTable;

/// QualNet typedefs struct ipv6_data_struct to
/// IPv6Data. struct ipv6_data_struct is
/// ipv6 information structure of a node.
typedef struct ipv6_data_struct
{

    // Route table related Informations
    int max_keylen;
    radix_node_head* rt_tables;
    unsigned int noOfPrefix;
    struct prefixListStruct* prefixHead;
    struct prefixListStruct* prefixTail;
    struct default_router_list* defaultRouter;
    // Routes not in table but not freed
    int rttrash;

    // NDP Cache
    radix_node_head* ndp_cache;

    // Hash for reverse NDP cache
    rn_rev_ndplookup* reverse_ndp_cache[MAX_REVLOOKUP_SIZE];

    // Destination cache.
    DestinationCache destination_cache;

    // Packet hold Structure before neighbor discovery.
    Ipv6HashTable* hashTable;

    // Multicast
    LinkedList* multicastGroupList;
    Ipv6MulticastForwardingTable multicastForwardingTable;

    // Fragmentation List.
    Ipv6FragQueue* fragmentListFirst;
    unsigned int ip6_id;

    char* addmask_key;
    char* rn_zeros;
    char* rn_ones;

    RandomSeed jitterSeed;

    // Ipv6 Statistics
    ip6Stat ip6_stat;

    // icmp6 related data structure pointer.
    struct network_icmpv6_struct* icmp6;

    // Back Pointer to Ip data for reverse traversal.
    NetworkDataIp* ip;

    in6_addr broadcastAddr;

    in6_addr loopbackAddr;
//Flag for default route entry
    BOOL defaultRouteFlag;
// For Dymo
    BOOL isManetGateway;
    UInt8 manetPrefixlength;
    Address manetPrefixAddr;
// End for Dymo
// For frag hold time
    clocktype ipFragHoldTime;
// End for frag hold time
    RandomDistribution<clocktype> randomNum;

    // Ipv6 auto-config
    Ipv6AutoConfigParam autoConfigParameters;
} IPv6Data;

/// QualNet typedefs struct ndp_event_struct to
/// IPv6Data. struct ndp_event_struct is
/// neighbor advertisement information structure.
typedef struct ndp_event_struct
{
    rn_leaf* ln;
}NadvEvent;

/// NDP neighbor advertisement delay.
#define NDP_DELAY 1 * MICRO_SECOND

/// IPv6 jitter timer.
#define IPV6JITTER_RANGE   (1000 * MILLI_SECOND)


/// Sets the flow class.
//#define IPV6_SET_CLASS(hdr, priority)       ((hdr)->ip6_tos = priority)

/// Gets the flow class.
//#define IPV6_GET_CLASS(hdr)   ((hdr)->ip6_class)

/// proxy (host)
#define IP6ANY_HOST_PROXY   1       // proxy (host)


/// proxy (router)
#define IP6ANY_ROUTER_PROXY 2       // proxy (router)


/// Checks whether the address is anycast address
/// of the node.
///
/// \param node  Pointer to node structure.
/// \param addr  ipv6 address.
int
in6_isanycast(Node* node, in6_addr* addr);

//----------------------------------------------------------
// IPv6 header
//----------------------------------------------------------

/// Add an IPv6 packet header to a message.
/// Just calls AddIpHeader.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message.
/// \param srcaddr  Source IPv6 address.
/// \param dst_addr  Destination IPv6 address.
/// \param priority  Current type of service
///    (values are not standard for "IPv6 type of
///    service field" but has correct function)
/// \param protocol  IPv6 protocol number.
/// \param hlim  Hop limit.
///
void
Ipv6AddIpv6Header(
    Node *node,
    Message *msg,
    int payloadLen,
    in6_addr src_addr,
    in6_addr dst_addr,
    TosType priority,
    unsigned char protocol,
    unsigned hlim);




/// Adds fragment header
///
/// \param node  Pointer to node
/// \param msg  Pointer to Message
/// \param nextHeader  nextHeader
/// \param offset  offset
/// \param id  id
///
void
Ipv6AddFragmentHeader(
    Node *node,
    Message *msg,
    unsigned char nextHeader,
    unsigned short offset,
    unsigned int id);



/// Removes Ipv6 header
///
/// \param node  Pointer to node
/// \param msg  Pointer to message
/// \param sourceAddress  Poineter Source address
/// \param destinationAddress  Destination address
/// \param priority  Priority
/// \param protocol  protocol
/// \param hLim  hLim
///
void
Ipv6RemoveIpv6Header(
    Node *node,
    Message *msg,
    Address* sourceAddress,
    Address* destinationAddress,
    TosType *priority,
    unsigned char *protocol,
    unsigned *hLim);


/// IPv6 Pre Initialization.
///
/// \param node  Pointer to node structure.
///
void
Ipv6PreInit(Node* node);

/// IPv6 Initialization.
///
/// \param node  Pointer to node structure.
/// \param nodeInput  Node input.
///
void
IPv6Init(Node *node, const NodeInput* nodeInput);

/// Checks whether the packet is the nodes packet.
/// if the packet is of the node then returns TRUE,
/// otherwise FALSE.
///
/// \param node  Pointer to node structure.
/// \param dst_addr  ipv6 packet destination address.
BOOL Ipv6IsMyPacket(Node* node, in6_addr* dst_addr);

/// Checks whether the address is in the same network.
/// : if in the same network then returns TRUE,
/// otherwise FALSE.
///
/// \param globalAddr  Pointer to ipv6 address.
/// \param tla  Top level ipv6 address.
/// \param vla  Next level ipv6 address.
/// \param sla  Site local ipv6 address.
BOOL
Ipv6IsAddressInNetwork(
    const in6_addr* globalAddr,
    unsigned int tla,
    unsigned int nla,
    unsigned int sla);


/// Returns 32 bit link layer address of the interface.
///
/// \param node  Pointer to node structure.
/// \param interfaceId  Interface Id.
/// \param ll_addr_str  Pointer to character link layer
///    address.
MacHWAddress
Ipv6GetLinkLayerAddress(
    Node* node,
    int interfaceId);

/// Adds an ipv6 interface to the node.
///
/// \param node  Pointer to node structure.
/// \param globalAddr  Global ipv6 address pointer.
/// \param tla  Top level id.
/// \param nla  Next level id.
/// \param sla  Site level id.
/// \param newinterfaceIndex  Pointer to new interface index.
/// \param nodeInput  Node Input.
///
void Ipv6AddNewInterface(
    Node* node,
    in6_addr* globalAddr,
    in6_addr* subnetAddr,
    unsigned int subnetPrefixLen,
    int* newinterfaceIndex,
    const NodeInput* nodeInput,
    unsigned short siteCounter,
    BOOL isNewInterface);


/// Checks whether the node is forwarding enabled.
///
/// \param ipv6  Pointer to ipv6 data structure.
BOOL Ipv6IsForwardingEnabled(IPv6Data* ipv6);

/// Handle IPv6 layer events, incoming messages and messages
/// sent to itself (timers, etc.).
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message.
///
void
Ipv6Layer(Node* node, Message* msg);

/// Finalize function for the IPv6 model.  Finalize functions
/// for all network-layer IPv6 protocols are called here.
///
/// \param node  Pointer to node.
///
void
Ipv6Finalize(Node* node);

/// Returns the maximum transmission unit of the interface.
///
/// \param node  Pointer to node.
/// \param interfaceId  Interface Id.
///
/// \return int
int Ipv6GetMTU(Node* node, int interfaceId);

/// Returns interface index of the specified address.
///
/// \param node  Pointer to node.
/// \param dst  IPv6 address.
///
/// \return int
int Ipv6GetInterfaceIndexFromAddress(Node* node, in6_addr* dst);

//-----------------------------------------------------------------------//
// A wrapper function for general use
//----------------------------------------------------------------------//

/// Calls the cpu packet scheduler for an interface to retrieve
/// an IPv6 packet from a queue associated with the interface.
/// The dequeued packet, since it's already been routed,
/// has an associated next-hop IPv6 address.  The packet's
/// priority value is also returned.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with IPv6 packet.
/// \param nextHopAddress  Packet's next hop link layer address.
/// \param destinationAddress  Packet's destination address.
/// \param outgoingInterface  Used to determine where packet
///    should go after passing through
///    the backplane.
/// \param networkType  Type of network packet is using (IPv6,
///    Link-16, ...)
/// \param queueIsFull  Storage for boolean indicator.
///    If TRUE, packet was not queued because
///    scheduler reported queue was
///    (or queues were) full.
///
void
Ipv6CpuQueueInsert(
    Node *node,
    Message *msg,
    NodeAddress nextHopLinkAddress,
    in6_addr destinationAddress,
    int outgoingInterface,
    int networkType,
    BOOL *queueIsFull);

/// Calls input packet scheduler for an interface to retrieve
/// an IP packet from a queue associated with the interface.
/// The dequeued packet, since it's already been routed,
/// has an associated next-hop IPv6 address.  The packet's
/// priority value is also returned.
///
/// \param node  Pointer to node.
/// \param incomingInterface  interface of input queue.
/// \param msg  Pointer to message with IPv6 packet.
/// \param nextHopAddress  Packet's next hop link layer address.
/// \param destinationAddress  Packet's destination address.
/// \param outgoingInterface  Used to determine where packet
///    should go after passing through
///    the backplane.
/// \param networkType  Type of network packet is using (IPv6,
///    Link-16, ...)
/// \param queueIsFull  Storage for boolean indicator.
///    If TRUE, packet was not queued because
///    scheduler reported queue was
///    (or queues were) full.
///
void
Ipv6InputQueueInsert(
    Node* node,
    int incomingInterface,
    Message* msg,
    NodeAddress nextHopLinkAddress,
    in6_addr destinationAddress,
    int outgoingInterface,
    int networkType,
    BOOL* queueIsFull);

/// Calls output packet scheduler for an interface to retrieve
/// an IP packet from a queue associated with the interface.
/// The dequeued packet, since it's already been routed,
/// has an associated next-hop IPv6 address.  The packet's
/// priority value is also returned.
/// Called by QueueUpIpFragmentForMacLayer().
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface of input queue.
/// \param msg  Pointer to message with IPv6 packet.
/// \param nextHopAddress  Packet's next link layer hop address.
/// \param destinationAddress  Packet's destination address.
/// \param networkType  Type of network packet is using (IPv6,
///    Link-16, ...)
/// \param queueIsFull  Storage for boolean indicator.
///    If TRUE, packet was not queued because
///    scheduler reported queue was
///    (or queues were) full.
///
void
Ipv6OutputQueueInsert(
    Node* node,
    int outgoingInterface,
    Message* msg,
    MacHWAddress& nextHopAddress,
    in6_addr destinationAddress,
    int networkType,
    BOOL* queueIsFull);

/// Calls output packet scheduler for an interface to retrieve
/// an IP packet from a queue associated with the interface.
/// The dequeued packet, since it's already been routed,
/// has an associated next-hop IPv6 address.  The packet's
/// priority value is also returned.
/// Called by QueueUpIpFragmentForMacLayer().
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface of input queue.
/// \param msg  Pointer to message with IPv6 packet.
/// \param nextHopAddress  Packet's next hop address.
/// \param destinationAddress  Packet's destination address.
/// \param networkType  Type of network packet is using (IPv6,
///    Link-16, ...)
/// \param queueIsFull  Storage for boolean indicator.
///    If TRUE, packet was not queued because
///    scheduler reported queue was
///    (or queues were) full.
///
void
QueueUpIpv6FragmentForMacLayer(
    Node *node,
    Message *msg,
    int interfaceIndex,
    //NodeAddress nextHop);
    MacHWAddress& nextHop,
    int incomingInterface);

/// This function is called once the outgoing interface
/// index and next hop address to which to route an IPv6
/// packet are known.  This queues an IPv6 packet for delivery
/// to the MAC layer.  This functions calls
/// QueueUpIpFragmentForMacLayer().
/// This function is used to initiate fragmentation if
/// required,before calling the next function.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with ip packet.
/// \param incommingInterface  Index of incoming interface.
/// \param outgoingInterface  Index of outgoing interface.
/// \param nextHop  Next hop link layer address.
///
void
Ipv6SendPacketOnInterface(
    Node* node,
    Message* msg,
    int incomingInterface,
    int outgoingInterface,
    MacHWAddress& nextHopLinkAddr);

/// This function is called when the packet delivered through
/// backplane delay.
/// required,before calling the next function.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with ip packet.
/// \param incommingInterface  Index of incomming interface.
/// \param outgoingInterface  Index of outgoing interface.
/// \param hopAddr  Next hop link layer address.
///
void Ipv6SendOnBackplane(
     Node* node,
     Message* msg,
     int incomingInterface,
     int outgoingInterface,
     MacHWAddress& hopAddr,
     optionList* opts);

/// Called by NetworkIpReceivePacketFromTransportLayer() to
/// send to send UDP datagrams using IPv6.
/// This function adds an IPv6 header and calls
/// RoutePacketAndSendToMac().
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with payload data
///    for IP packet.(IP header needs to
///    be added)
/// \param sourceAddress  Source IPv6 address.
///    If sourceAddress is ANY_IP, lets IPv6 assign the
///    source  address (depends on the route).
/// \param destinationAddress  Destination IPv6 address.
/// \param outgoingInterface  outgoing interface to use to
///    transmit packet.
/// \param priority  Priority of packet.
/// \param protocol  IPv6 protocol number.
/// \param ttl  Time to live.
///    See AddIpHeader() for special values.
///
void
Ipv6SendRawMessage(
    Node* node,
    Message* msg,
    in6_addr sourceAddress,
    in6_addr destinationAddress,
    int outgoingInterface,
    TosType priority,
    unsigned char protocol,
    unsigned ttl = IPDEFTTL);

/// Sends a UDP packet to UDP in the transport layer.
/// The source IPv6 address, destination IPv6 address, and
/// priority of the packet are also sent.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with UDP packet.
/// \param priority  Priority of UDP
///    packet.
/// \param sourceAddress  Source IP address info.
/// \param destinationAddress  Destination IP address info.
/// \param incomingInterfaceIndex  interface that received the packet
///
void
Ipv6SendToUdp(
    Node* node,
    Message* msg,
    TosType priority,
    Address sourceAddress,
    Address destinationAddress,
    int incomingInterfaceIndex);

/// Sends a TCP packet to UDP in the transport layer.
/// The source IPv6 address, destination IPv6 address, and
/// priority of the packet are also sent.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with UDP packet.
/// \param priority  Priority of TCP
///    packet.
/// \param sourceAddress  Source IP address info.
/// \param destinationAddress  Destination IP address info.
/// \param incomingInterfaceIndex  interface that received the packet
///
void
Ipv6SendToTCP(
    Node* node,
    Message* msg,
    TosType priority,
    Address sourceAddress,
    Address destinationAddress,
    int incomingInterfaceIndex);

/// IPv6 received IPv6 packet from MAC layer.  Determine whether
/// the packet is to be delivered to this node, or needs to
/// be forwarded.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message with ip packet.
/// \param previousHopNodeId  nodeId of the previous hop.
/// \param interfaceIndex  Index of interface on which packet arrived.
///
void
Ipv6ReceivePacketFromMacLayer(
    Node* node,
    Message* msg,
    MacHWAddress* lastHopAddress,
    int interfaceIndex);

//------------------------------------------------------------------------//
// Hold Buffer Functions.
//------------------------------------------------------------------------//

/// Adds an ipv6 packet in message in the hold buffer
///
/// \param node  Pointer to node structure.
/// \param msg  Pointer to message with ip packet.
/// \param nextHopAddr  Source IPv6 address.
/// \param inCommingInterface  Incoming interface
///
/// \return BOOL
BOOL
Ipv6AddMessageInBuffer(
    Node* node,
    Message* msg,
    in6_addr* nextHopAddr,
    int inComingInterface,
    rn_leaf* rn);


/// Delets an ipv6 packet in the hold buffer
///
/// \param node  Pointer to node structure.
/// \param mBuf  Pointer to messageBuffer tail.
///
/// \return BOOL
BOOL
Ipv6DeleteMessageInBuffer(
        Node* node,
        in6_addr nextHopAddr,
        rn_leaf* ln);

/// Drops an ipv6 packet from the hold buffer
///
/// \param node  Pointer to node structure.
/// \param mBuf  Pointer to messageBuffer tail.
///
BOOL
Ipv6DropMessageFromBuffer(
        Node* node,
        in6_addr nextHopAddr,
        rn_leaf* ln);


//-----------------------------------------------------------------------//
// Utility Functions
//------------------------------------------------------------------------//

/// Returns network type from string ip address.
///
/// \param interfaceAddr  Character Pointer to ip address.
///
/// \return NetworkType
NetworkType
Ipv6GetAddressTypeFromString(char* interfaceAddr);

/// Get multicast address of this interface
///
/// \param node  Node pointer
/// \param interfaceIndex  interface for which multicast is required
///
/// \return IPv6 multicast address
in6_addr
Ipv6GetInterfaceMulticastAddress(Node* node, int interfaceIndex);

/// Copies multicast solicitation address.
///
/// \param dst_addr  ipv6 address pointer.
/// \param target  ipv6 multicast address pointer.
///
void
Ipv6SolicitationMulticastAddress(
    in6_addr* dst_addr,
    in6_addr* target);

// FUNCTION             : Ipv6AllRoutersMulticastAddress
/// Function to assign all routers multicast address.
///
/// \param dst  IPv6 address pointer,
///    output destination multicast address.
///    RETURN               : None

void
Ipv6AllRoutersMulticastAddress(
       in6_addr* dst);

/// Gets ipv6 link local address of the interface in
/// output parameter addr.
///
///    + node      :           : Pointer to the node structure.
/// \param interface  interface Index.
/// \param addr  ipv6 address pointer.
///
void
Ipv6GetLinkLocalAddress(
    Node* node,
    int interfaceId,
    in6_addr* addr);

/// Gets ipv6 site local address of the interface in
/// output parameter addr.
///
///    + node      :           : Pointer to the node structure.
/// \param interface  interface Index.
/// \param addr  ipv6 address pointer.
///
void
Ipv6GetSiteLocalAddress(
    Node* node,
    int interfaceId,
    in6_addr* addr);

/// Gets ipv6 global agreeable address of the interface in
/// output parameter addr.
///
///    + node      :           : Pointer to the node structure.
/// \param interface  interface Index.
/// \param addr  ipv6 address pointer.
///
void
Ipv6GetGlobalAggrAddress(
    Node* node,
    int interfaceId,
    in6_addr* addr);

/// Gets ipv6 prefix from address.
///
/// \param addr  ipv6 address pointer.
/// \param prefix  ipv6 prefix pointer.
///
void
Ipv6GetPrefix(
    const in6_addr* addr,
    in6_addr* prefix,
    unsigned int length);

/// Gets ipv6 prefix from address.
///
/// \param node  Node pointer
/// \param interfaceIndex  interface for which multicast is required
///
/// \return Prefix for this interface
in6_addr
Ipv6GetPrefixFromInterfaceIndex(
    Node* node,
    int interfaceIndex);

//---------------------------------------------------------------------------
// FUNCTION             : Ipv6ConvertStrLinkLayerToMacHWAddress
// PURPOSE             :: Converts from character formatted link layer address
//                        to NodeAddress format.
// PARAMETERS ::
// + llAddrStr          : char* llAddrStr : Link Layer Address String
// RETURN               : MacHWAddress: returns mac hardware address.
//---------------------------------------------------------------------------
MacHWAddress Ipv6ConvertStrLinkLayerToMacHWAddress(
    Node* node,
    int interfaceId,
    unsigned char* llAddrStr);

/// Check weather output queue is empty
///
/// \param node  Pointer to Node
/// \param interfaceIndex  interfaceIndex
///
/// \return BOOL
BOOL
Ipv6OutputQueueIsEmpty(Node *node, int interfaceIndex);


/// Ipv6 Static routing initialization function.
///
/// \param node  Pointer to node
/// \param nodeInput  *nodeInput
/// \param type  type
///
void
Ipv6RoutingStaticInit(
    Node *node,
    const NodeInput *nodeInput,
    NetworkRoutingProtocolType type);


/// Static routing route entry function
///
/// \param node  Pointer to node
/// \param currentLine  Static entry's current line.
///
void
Ipv6RoutingStaticEntry(
    Node *node,
    char currentLine[]);



/// Adds destination in the destination cache.
///
/// \param node  Pointer to node
/// \param ro  Pointer to destination route.
///
void
Ipv6AddDestination(
   Node* node,
   struct route_struct* ro);



/// Deletes destination from the destination cache.
///
/// \param node  Pointer to node
///
void
Ipv6DeleteDestination(Node* node);



/// Checks the packet's validity
///
/// \param node  Pointer to node
/// \param scheduler  pointer to scheduler
/// \param pIndex  packet index
///
/// \return int
int
Ipv6CheckForValidPacket(
    Node* node,
    Scheduler* scheduler,
    unsigned int* pIndex);



/// Ipv6 Destination cache and neighbor cache
/// :   processing function
///
/// \param node  Pointer to node
///
void
Ipv6NdpProcessing(Node* node);

void ndsol6_retry(Node* node, Message* msg);

/// Updates Ipv6 Forwarding Table
///
/// \param node  Pointer to node
/// \param destPrefix  IPv6 destination address
/// \param nextHopPrefix  IPv6 next hop address for this destination
/// \param interfaceIndex  interfaceIndex
/// \param metric  hop count between source and destination
///

void Ipv6UpdateForwardingTable(
    Node* node,
    in6_addr destPrefix,
    unsigned int destPrefixLen,
    in6_addr nextHopPrefix,
    int interfaceIndex,
    int metric,
    NetworkRoutingProtocolType type);

/// Empties Ipv6 Forwarding Table for a particular
/// routing protocol entry
///
/// \param node  Pointer to node
/// \param type  Routing protocol type
///
void Ipv6EmptyForwardingTable(Node *node,
                              NetworkRoutingProtocolType type);

/// Prints the forwarding table.
///
/// \param node  Pointer to node
///
void Ipv6PrintForwardingTable(Node *node);


// FUNCTION            :: Ipv6SetRouterFunction
// LAYER               :: Network
// PURPOSE             :: Sets the router function.
// PARAMETERS          ::
// + node               : Node* node : Pointer to node
// + routerFunctionPtr  : Ipv6RouterFunctionType routerFunctionPtr: router
//                      : function pointer.
// + interfaceIndex     : int interfaceIndex : Interface index
// RETURN              :: None
void Ipv6SetRouterFunction(Node *node,
                           Ipv6RouterFunctionType routerFunctionPtr,
                           int interfaceIndex);

// FUNCTION            :: Ipv6GetRoutingProtocolType
// LAYER               :: Network
// PURPOSE             :: Return The routing Protocol Type.
// PARAMETERS          ::
// + node               : Node* node : Pointer to node
// + interfaceIndex     : int interfaceIndex : Interface index
// RETURN              :: Routing Protocol Type
NetworkRoutingProtocolType Ipv6GetRoutingProtocolType(Node *node,
                                                int interfaceIndex);

/// Get the interface index from an IPv6 subnet address.
///
/// \param node  Pointer to node
/// \param address  Subnet Address
///
/// \return Interface index associated with specified subnet address.
int Ipv6InterfaceIndexFromSubnetAddress(
    Node *node,
    in6_addr address);

/// Do a lookup on the routing table with a destination IPv6
/// address to obtain an outgoing interface and a next hop
/// Ipv6 address.
///
/// \param node  Pointer to node
/// \param destAddr  Destination Address
/// \param interfaceIndex  Pointer to interface index
/// \param nextHopAddr  Next Hop Addr for destination.
///
void Ipv6GetInterfaceAndNextHopFromForwardingTable(
    Node* node,
    in6_addr destAddr,
    int* interfaceIndex,
    in6_addr* nextHopAddress);

/// Get interface for the destination address.
///
/// \param node  Pointer to node
/// \param destAddr  Destination Address
///
/// \return interface index associated with destination.
int Ipv6GetInterfaceIndexForDestAddress(
    Node* node,
    in6_addr destAddr);

/// Get the cost metric for a destination from the
/// forwarding table.
///
/// \param node  Pointer to node
/// \param destAddr  Destination Address
///
/// \return interface index associated with destination.
unsigned Ipv6GetMetricForDestAddress(
    Node* node,
    in6_addr destAddr);

/// This function looks at the network address of each of a
/// node's network interfaces. When nextHopAddress is
/// matched to a network, the interface index corresponding
/// to the network is returned.
///
/// \param node  Pointer to node
/// \param destAddr  Destination Address
///
/// \return Interface index associated with destination if found,
/// -1 otherwise.
int Ipv6IpGetInterfaceIndexForNextHop(
    Node *node,
    in6_addr address);

/// Get the router function pointer.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface associated with router function
///
/// \return router function pointer.
Ipv6RouterFunctionType Ipv6GetRouterFunction(
    Node* node,
    int interfaceIndex);

/// Used if IPv6 next hop address and outgoing interface is
/// known.
///
/// \param node  Pointer to node
/// \param msg  Pointer to message
/// \param destAddr  Destination Address
/// \param nextHopAddr  Next Hop Addr for destination.
/// \param interfaceIndex  Pointer to interface index
///
void Ipv6SendPacketToMacLayer(
    Node* node,
    Message* msg,
    in6_addr destAddr,
    in6_addr nextHopAddr,
    Int32 interfaceIndex);

//---------------------------------------------------------------------------
// FUNCTION             : Ipv6SendRawMessageToMac
// PURPOSE             :: Ipv6 packet sending function.
// PARAMETERS          ::
// + node               : Node *node : Pointer to node
// + msg                : Message *msg : Pointer to message
// + sourceAddress      : in6_addr sourceAddress:
//                          IPv6 source address of packet
// + destinationAddress : in6_addr destinationAddress:
//                          IPv6 destination address of packet.
// + outgoingInterface  : int outgoingInterface
// + priority           : TosType priority: Type of service of packet.
// + protocol           : unsigned char protocol: Next protocol number.
// + ttl                : unsigned ttl: Time to live.
// RETURN               : None
//---------------------------------------------------------------------------
void
Ipv6SendRawMessageToMac(
    Node *node,
    Message *msg,
    in6_addr sourceAddress,
    in6_addr destinationAddress,
    int outgoingInterface,
    TosType priority,
    unsigned char protocol,
    unsigned ttl);

/// Join a multicast group.
///
/// \param node  Pointer to node.
/// \param mcastAddr  multicast group to join.
/// \param delay  delay.
///
void Ipv6JoinMulticastGroup(
    Node* node,
    in6_addr mcastAddr,
    clocktype delay);

/// Add group to multicast group list.
///
/// \param node  Pointer to node.
/// \param groupAddress  Group to add to multicast group list.
///
void Ipv6AddToMulticastGroupList(Node* node, in6_addr groupAddress);

/// Leave a multicast group.
///
/// \param node  Pointer to node.
/// \param mcastAddr  multicast group to leave.
///
void Ipv6LeaveMulticastGroup(
    Node* node,
    in6_addr mcastAddr,
    clocktype delay);

/// Remove group from multicast group list.
///
/// \param node  Pointer to node.
/// \param groupAddress  Group to be removed from multicast
///    group list.
///
void Ipv6RemoveFromMulticastGroupList(Node* node, in6_addr groupAddress);

/// Invoke callback functions when a packet is dropped.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message.
/// \param nextHopAddress  Next Hop Address
/// \param interfaceIndex  Interface Index
///
void
Ipv6NotificationOfPacketDrop(
    Node* node,
    Message* msg,
    MacHWAddress& nextHopAddress,
    int interfaceIndex);

/// Check if destination is part of the multicast group.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message.
/// \param groupAddress  Multicast Address
///
/// \return TRUE if node is part of multicast group,
/// FALSE otherwise.
BOOL Ipv6IsPartOfMulticastGroup(Node *node, in6_addr groupAddress);

/// Check if address is reserved multicast address.
///
/// \param node  Pointer to node.
/// \param mcastAddr  multicast group to join.
///
/// \return TRUE if reserved multicast address, FALSE otherwise.
BOOL Ipv6IsReservedMulticastAddress(Node* node, in6_addr mcastAddr);

/// Determine if interface is in multicast outgoing
/// interface list.
///
/// \param node  Pointer to node.
/// \param list  Pointer to Linked List.
/// \param interfaceIndex  Interface Index.
///
/// \return TRUE if interface is in multicast outgoing interface
/// list, FALSE otherwise.
BOOL Ipv6InMulticastOutgoingInterface(
    Node* node,
    LinkedList* list,
    int interfaceIndex);

/// update entry with (sourceAddress,
/// multicastGroupAddress) pair. search for the row with
/// (sourceAddress, multicastGroupAddress) and update its
/// interface.
///
/// \param node  Pointer to node.
/// \param sourceAddress  Source Address.
/// \param multicastGroupAddress  multicast group.
///
void Ipv6UpdateMulticastForwardingTable(
    Node* node,
    in6_addr sourceAddress,
    in6_addr multicastGroupAddress,
    int interfaceIndex);

/// get the interface List that lead to the (source,
/// multicast group) pair.
///
/// \param node  Pointer to node.
/// \param sourceAddress  Source Address
/// \param groupAddress  multicast group address
///
/// \return Interface List if match found, NULL otherwise.
LinkedList* Ipv6GetOutgoingInterfaceFromMulticastTable(
    Node* node,
    in6_addr sourceAddress,
    in6_addr groupAddress);

/// Create IPv6 Broadcast Address (ff02 followed by all one).
///
///    +node:  Node* : Pointer to Node.
///    +multicastAddr: in6_addr* : Pointer to broadcast address.
///
void
Ipv6CreateBroadcastAddress(Node* node, in6_addr* multicastAddr);

/// Get prefix length of an interface.
///
///    +node           : Node* : Pointer to Node.
///    +interfaceIndex : int   : Inetrafce Index.
///
/// \return Prefix Length.
UInt32
Ipv6GetPrefixLength(Node* node, int interfaceIndex);

/// Allows the MAC layer to send status messages (e.g.,
/// packet drop, link failure) to a network-layer routing
/// protocol for routing optimization.
///
/// \param node  Pointer to node.
/// \param StatusEventHandlerPtr  Function Pointer
/// \param interfaceIndex  Interface Index
///
void
Ipv6SetMacLayerStatusEventHandlerFunction(
    Node *node,
    Ipv6MacLayerStatusEventHandlerFunctionType StatusEventHandlerPtr,
    int interfaceIndex);

/// Deletes all packets in the queue going to the
/// specified next hop address. There is option to return
/// all such packets back to the routing protocols.
///
/// \param node  Pointer to node.
/// \param nextHopAddress  Next Hop Address.
/// \param destinationAddress  Destination Address
/// \param returnPacketsToRoutingProtocol  bool
///
void
Ipv6DeleteOutboundPacketsToANode(
    Node *node,
    in6_addr ipv6NextHopAddress,
    in6_addr destinationAddress,
    const BOOL returnPacketsToRoutingProtocol);

int Ipv6GetFirstIPv6Interface(Node* node);

/// Check if address is self loopback address.
///
/// \param node  Pointer to node.
/// \param address  ipv6 address
///
BOOL Ipv6IsLoopbackAddress(Node* node, in6_addr address);

/// Check if address is self loopback address.
///
/// \param node  Pointer to node.
/// \param dst_addr  Pointer to ipv6 address
///
/// \return TRUE if my Ip, FALSE otherwise.
BOOL Ipv6IsMyIp(Node* node, in6_addr* dst_addr);

/// Check if multicast address has valid scope.
///
/// \param node  Pointer to node.
/// \param multiAddr  multicast address.
///
/// \return Scope value if valid multicast address, 0 otherwise.
int Ipv6IsValidGetMulticastScope(Node* node, in6_addr multiAddr);

//------------------------------------------------------------------------
// FUNCTION     IPv6RoutingInit()
// PURPOSE      Initialization function for network layer.
//              Initializes IP.
// PARAMETERS   Node *node
//                  Pointer to node.
//              const NodeInput *nodeInput
//                  Pointer to node input.
//-----------------------------------------------------------------------
void
IPv6RoutingInit(Node *node, const NodeInput *nodeInput);

/// To check if IPV6 Routing is enabled on interface?
///
/// \param node  node structure pointer.
/// \param interfaceIndex  interface Index.
///
/// \return BOOL
BOOL IsIPV6RoutingEnabledOnInterface(Node* node,
                                 int interfaceIndex);

//--------------------------------------------------------------------------
// FUNCTION             : Ipv6InterfaceInitAdvt
// PURPOSE             :: This is an overloaded function to
//                        re-initialize advertisement by an interface
//                        that recovers from interface fault.
// PARAMETERS          ::
// + node               : Node* node : Pointer to node
// + interfaceindex     : int interfaceIndex : interface that recovers
//                        from interface fault
// RETURN               : None
//--------------------------------------------------------------------------
void
Ipv6InterfaceInitAdvt(Node* node, int interfaceIndex);
#endif // IPV6_H


