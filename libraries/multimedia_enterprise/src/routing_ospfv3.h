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


// PROTOCOL :: OSPFV3.
// SUMMARY :: The principal characteristic of OSPF is that it is based on SPF
/// algorithm. The algorithm sometimes referred to as Dijkstra Algorithm.
/// Based on topology information this algorithm calculates a shortest path
/// tree with calculating node as root. This shortest path tree gives route
/// to each destination in the Autonomous System (AS). The topology
/// information is termed as Link State Database (LSDB), which consists of
/// Link State Advertisement (LSA) that every router in the domain originates
/// in a periodical and on-demand basis.
/// What we mean by Link State is a description of a router's usable interface
/// and its relation to its attached neighbors and networks. Each router in
/// the domain advertises its local state (e.g. router's usable interfaces and
/// attached neighbors) throughout the AS by flooding.
/// The flooding procedure needs to be adequately efficient so that each
/// router in the domain have the same LSDB.
/// One of the major advantages of OSPF over RIP is that it can operate within
/// a hierarchy. The largest entity within the hierarchy is
/// Autonomous System (AS), which is a collection of networks under a single
/// administrative control. Though OSPF is an Interior Gateway Protocol it can
/// receive and update routing information from other AS. An AS can further be
/// divided into a number of areas, which are groups of contiguous networks
/// and attached hosts. The topology of an area is hidden from the rest of the
/// AS.
/// Doing so it is no longer required that all router in the AS have exactly
/// same set of LSAs, thus one of the significant effect of this feature is
/// reduction in routing traffic. Routing within an area is solely depends on
/// areas own topology. For inter area routing some mechanism is required for
/// finding out routes to networks belong to other area. This is acheive with
/// help Area Border Router who inject inter area routes into the area.
/// An OSPF router built its routing table from the Shortest Path Tree it
/// constructs using its attached areas database. All router runs the exactly
/// same algorithm, in parallel. Whenever the topology changes occur, the
/// routing table is built again from the scratch. IP packets are routed
/// "as is" & they are not encapsulated in any further protocol headers as
/// they transit the Autonomous System. As a dynamic routing protocol OSPF
/// quickly detects topology changes in the AS and calculates new loop free
/// routes after a period of convergence.

// LAYER :: Network Layer Protocol runs directly over IPV6.
// STATISTICS :: Ospfv3Stats : numHelloSent;
//                             numHelloRecv;
//                             numLSUpdateSent;
//                             numLSUpdateRecv;
//                             numAckSent;
//                             numAckRecv;
//                             numLSUpdateRxmt;
//                             numExpiredLSAge;
//                             numDDPktSent;
//                             numDDPktRecv;
//                             numDDPktRxmt;
//                             numLSReqSent;
//                             numLSReqRecv;
//                             numLSReqRxmt;
//                             numRtrLSAOriginate;
//                             numNetLSAOriginate;
//                             numInterAreaPrefixLSAOriginate;
//                             numInterAreaRtrLSAOriginate;
//                             numLinkLSAOriginate;
//                             numIntraAreaPrefixLSAOriginate;
//                             numASExternalLSAOriginate
//                             numLSARefreshed;
//
// CONFIG_PARAM ::Parameter Name format
//
//           ROUTING-PROTOCOL               OSPFv3
//           OSPFv3-STAGGER-START           YES | NO
//
//           OSPFv3-INJECT-EXTERNAL-ROUTE   YES
//           OSPFv3-INJECT-ROUTE-FILE       <filename>
//
//           <node_id>  AS-NUMBER           <as_id>
//           <node_id>  AS-BOUNDARY-ROUTER  YES
//
//           OSPFv3-DEFINE-AREA             YES | NO
//           OSPFv3-CONFIG-FILE             <filename>

// OSPFv3 Configuration File Parameters
//           [<Network number>]  AREA-Id  <Area Id in IP address format>
//           AREA <Area Id> RANGE { <Network1>,  ... , <NetworkN> } [<as_id>]
//           AREA <Area Id> STUB <Stub Default Cost>    [<as_id>]
//
//           [Interface Qualifier] INTERFACE-COST       <cost>
//           [Interface Qualifier] RXMT-INTERVAL        <interval>
//           [Interface Qualifier] INF-TRANS-DELAY      <delay>
//           [Interface Qualifier] ROUTER-PRIORITY      <priority>
//           [Interface Qualifier] HELLO-INTERVAL       <interval>
//           [Interface Qualifier] ROUTER-DEAD-INTERVAL <interval>
//           [Interface Qualifier] INTERFACE-TYPE       <type>
//
//
// IMPLEMENTED_FEATURES :: Flooding scope
//                         Intra-area-prefix-LSA
//                         IPV6 prefix represention of group of addresses.
//                         Single Area and Splitting AS into areas
//                         Physically contiguous Backbone Area
//                         Inter area routing
//                         Hello protocol
//                         Broadcast Network
//                         Point-to-point networks
//                         Point-to-Multipoint networks
//                         Aging of LSA
//                         AS-External LSA

//
// Features to be considered for Future Work
// Phase-2 implementations:
//    1. Link local addresses
//    2. Link-LSA
//

// OMITTED_FEATURES :: (Title) : (Description) (list)
//                         1. Virtual link.
//                         2. Multiple OSPF instance on a single link.
//                         3. NBMA (Non broadcast Multiple Access).
//                         4. Equal cost Multi-path.
//                         5. Incremental LS update.
//                         6. Type-7 LSA.
//                         7. Unknown LSA types handling.
//                         8. Backbone that are physically discontinuous.

// ASSUMPTIONS :: (Description of Assumption) (list)
// 1.   While running over non broadcast network OSPFv3 will consider
//      underlying network as Point-to-Point unless user specifies/configures
//      the same.
// 2.   ASBR will not calculate AS-External routes. BGP will be responsible
//      to inject AS-External routes in IP Forwarding table.
// 3.   Assumptions regarding virtual links are same as for OSPFV2.
// 4.   Link local addressing is not used because of IPv6 Addressing
//      restriction. Although for future use, Link local LSA will be
//      generated and flooded across the defined scopes.
// 5.   Each router configured with multiple interfaces will originate single
//      aggregated router lsa and the intra
//      area prefix lsa for the OSPF node.
// 6.   Permissible Area-ID configuration is within the range of 0.0.0.0 to
//      255.255.255.253.
// 7.   Users need to insert appropriate Outgoing Interface Cost inside
//      the .ospf file in order to choose a High Bandwidth Route path.
// 8.   In a switch scenario, dr-bdr nodes are elected correctly around 60s.
// 9.  As Link-Local Addressing is not used, the Intra-Area-Prefix LSA with
//      reference to Network LSA is not generated.
// 10.  All Area Border Routers must have an interface connected to the
//      Backbone area.
// 11.  Multiple common interfaces between nodes are not supported.
// 12.  Interface type broadcast needs to be specified for Wireless Subnet.

// STANDARD :: (Standard used for implementation)
// Reference:
// 1. RFC 2740, "OSPF for IPv6" R. Coltun, D. Ferguson and J. Moy Dec 1999.
// 2. RFC 2328, "OSPF version 2" J. Moy April 1998.

// Other references:
// 3. OSPF: Frequently Asked Questions, Document ID: 9237,Author:
//    Syed Faraz Shamim, available at
//    http://www.cisco.com/warp/public/104/9.html
// 4. Configuring OSPFv3, available at
// http://www.foundrynet.com/services/documentation/srIPv6/OSPF_Version_3.html

// RELATED :: (Name of related protocol) (list) {optional}

#ifndef OSPFv3_H
#define OSPFv3_H

#include "routing_ospfv2.h"
#include "buffer.h"
#include "list.h"

// This the version of OSPFv3.
#define OSPFv3_CURRENT_VERSION 0x3


// Initial size of routing table maintained by OSPF.
#define OSPFv3_INITIAL_TABLE_SIZE (10)

// The metric value indicating that the destination described by an LSA is
// unreachable.
#define OSPFv3_LS_INFINITY 0xFFFFFF

// This is maximum Length String used in OSPFv3 implementation.
#define OSPFv3_MAX_STRING_LENGTH 500

// Jitter transmissions to avoid  sychronization.
#define OSPFv3_BROADCAST_JITTER (40 *  MILLI_SECOND)
//#define OSPFv3_BROADCAST_JITTER (100 * MILLI_SECOND)


// Interval after which we increment LS age. It must be greater than or
// equal to 1 second.
#define OSPFv3_LSA_INCREMENT_AGE_INTERVAL (1 * SECOND)

// Maximum age limit for LSAs.
#define OSPFv3_LSA_MAX_AGE (60 * MINUTE)

// The estimated number of seconds it takes to transmit a Link State Update
// Packet over this interface.  LSAs contained in the update packet must have
/// their age incremented by this amount before transmission.  This value
/// should take into account the transmission and propagation delays of the
/// interface.  It must be greater than 0. Default value for a local area
/// network is 1 second.
#define OSPFv3_INF_TRANS_DELAY (1 * SECOND)

// Interval of sending hello packets (in seconds).
#define OSPFv3_HELLO_INTERVAL (10 * SECOND)

// This is the interval in which after ceasing to hear a router's Hello
// Packets, all of its neighbors declare the router down. This is advertised
/// in the router's Hello Packets in their RouterDeadInterval field.
#define OSPFv3_ROUTER_DEAD_INTERVAL (4 * OSPFv3_HELLO_INTERVAL)

// This the number of seconds between LSA retransmissions, for adjacencies
// belonging to an interface. This is used when retransmitting Database
/// Description and Link State Request Packets. This should be well over the
/// expected round-trip delay between any two routers on the attached network.
#define OSPFv3_RXMT_INTERVAL (5 * SECOND)

// This is the maximum time between distinct originations of any particular
// LSA. If the LS age field of one of the router's self-originated LSAs
/// reaches the value LSRefreshTime, a new instance of the LSA is originated,
/// even though the contents of the LSA (apart from the LSA header) will be
/// the same. Its value must be 30 minutes.
#define OSPFv3_LS_REFRESH_TIME (30 * MINUTE)

// This is the minimum time that must elapse between reception of a new LSA
// instances during flooding. LSA instances received at higher frequencies
/// are discarded. The value of MinLSArrival must be 1 second.
#define OSPFv3_MIN_LS_ARRIVAL (1 * SECOND)

// This is the maximum time dispersion that can occur, as an LSA is flooded
// throughout the AS. Its value must be 15 minutes.
#define OSPFv3_MAX_AGE_DIFF (15 * MINUTE)

// This is an interval used to trigger a single shot wait timer that causes
// the interface to exit the Waiting state, and as a consequence select a
/// Designated Router on the network.  The length of the timer must be equal
/// to RouterDeadInterval.
#define OSPFv3_WAIT_TIMER OSPFv3_ROUTER_DEAD_INTERVAL

// This is an interval used to trigger a Flood Timer.
#define OSPFv3_FLOOD_TIMER (100 * MILLI_SECOND)

// This is the minimum time between distinct originations of any particular
// LSA. The value of MinLSInterval must be 5 seconds.
#define OSPFv3_MIN_LS_INTERVAL (5 * SECOND)

// This is the delay used for scheduling Interface and Neighbor Events.
// Its value must be 5 seconds.
#define OSPFv3_EVENT_SCHEDULING_DELAY (1 * NANO_SECOND)

// This is default value of Router Priority. It can be configured in OSPFv3
// configuration file. A router whose Router Priority is set to 0 is
/// ineligible to become Designated Router on the attached network.
#define OSPFv3_DEFAULT_PRIORITY 1

// This is default value of Router Priority. It can be configured in OSPFv3
// configuration file. A router whose Router Priority is set to 0 is
/// ineligible to become Designated Router on the attached network.
#define OSPFv3_DEFAULT_COST 1

// This is Area ID in case there is a single area domain for entire AS.
#define OSPFv3_SINGLE_AREA_ID 0xFFFFFFFF

// This is Backbone Area ID.
#define OSPFv3_BACKBONE_AREA 0x0

// This is Invalid Area ID used when flushing AS-External LSA.
#define OSPFv3_INVALID_AREA_ID 0xFFFFFFFE

// The value used for LS Sequence Number when originating the first instance
// of any LSA. Its value is the signed 0x80000001.
#define OSPFv3_INITIAL_SEQUENCE_NUMBER 0x80000001

// The maximum value that LS Sequence Number can attain. Its value is the
// signed integer 0x7fffffff.
#define OSPFv3_MAX_SEQUENCE_NUMBER 0x7FFFFFFF

// The default destination prefix length is 0.
#define OSPFv3_DEFAULT_PREFIX_LENGTH 0

// Enabling this might reduce some duplicate LSA transmission in a network.
// When enable this, a node wait for time [0, OSPFv3_FLOOD_TIMER] before
/// sending Update in response to a Request. Thus at synchronization time
/// a node send less number of Update packet in response to several duplicate
/// Request from different node in the network. This intern also reduce
/// number of Ack packet transmitted in the network.
#define OSPFv3_SEND_DELAYED_UPDATE 1

// Enable this if you want ABR to examine transit areas summary LSAs also
// other than examining backbone area only. This might be helpfull for
/// finding routes when link to backbone faults.
#define EXAMINE_TRANSIT_AREA 0

// The prefix length which is fixed to 64-bit in NLA-TLA-SLA Ipv6 address
// format.
#define OSPFv3_CONST_PREFIX_LENGTH (unsigned char)64

// Maximum possible prefix length for IPv6 address.
#define OSPFv3_MAX_PREFIX_LENGTH (unsigned char)128

// This is delay for which AS Boundary Router must wait before originating
// and flooding AS-External LSA. This is because all the routers within the AS
/// must know route to AS Boundary Router before calculating external routes.
#define OSPFv3_AS_EXTERNAL_LSA_ORIGINATION_DELAY (2 * MINUTE)

// This is maximum value for AS Identifier.
#define OSPFv3_MAX_AS_NUMBER 65535

// This is manimum value for AS Identifier.
#define OSPFv3_MIN_AS_NUMBER 1

// This specify maximum number of areas to which Area Border Router can be
// connected.
#define OSPFv3_MAX_ATTACHED_AREA 3

// Return option value stored in first 24-bit of a 32-bit
// packet structure member. The memebr variable is passed as argument.
#define OSPFV3_GetOptionBits(bits) (bits & 0x00ffffff)

// Return Router Priority stored in 8-bits of a 32-bit
// packet structure member. The memebr variable is passed as argument.
#define OSPFV3_GetHelloPacketPriorityFeild(bits) ((bits & 0xff000000) >> 24)

// Set the 8-bit priority in most significant 8-bits of an 32-bit variable.
#define OSPFv3_SetPriorityFeild(bits, val)\
        (*bits) = (((*bits) & 0x00ffffff) | ((unsigned)val << 24))

// Set the 16-bit IP MTU in most significant 16-bits of an 32-bit variable.
#define OSPFv3_SetDDPacketMTUFeild(bits, val)\
        (*bits) = (((*bits) & 0x0000ffff) | ((unsigned)val << 16))

// Clear reserved bits for Router LSA.
#define OSPFv3_ClearRouterLSAReservedBits(lsa) \
        lsa->bitsAndOptions = (lsa->bitsAndOptions & 0x0f00003f)\

/// Type of OSPFv3 nodes.
typedef enum
{
    OSPFv3_ROUTER_TYPE,
    OSPFv3_HOST_TYPE
} Ospfv3RouterType;

/// Used to calculate Shortest Path Tree.
typedef enum
{
    OSPFv3_VERTEX_ROUTER = 0,
    OSPFv3_VERTEX_NETWORK
} Ospfv3VertexType;

/// Next hope List Item.
typedef struct
{
    unsigned int outIntfId;
    in6_addr nextHop;
} Ospfv3NextHopListItem;

/// Identifier used if vertex is a network.
typedef struct
{
    unsigned int routerIdOfDR;
    unsigned int interfaceIdOfDR;
} Ospfv3NetworkVertexId;

/// Vertex Id used for both network vertex
/// as well as router vertex.
typedef union
{
    // Used if vertex is netork
    Ospfv3NetworkVertexId networkVertexId;

    // Used if vertex is router
    unsigned int routerVertexId;
} Ospfv3VertexId;


/// Represent an OSPFv3 node during
/// various SPF tree calculation.
typedef struct
{
    Ospfv3VertexId vertexId;
    Ospfv3VertexType vertexType;
    char* LSA;
    LinkedList* nextHopList;
    unsigned int distance;
} Ospfv3Vertex;

/// Link state types.
typedef enum
{
    OSPFv3_ROUTER = 0x2001,
    OSPFv3_NETWORK = 0x2002,
    OSPFv3_INTER_AREA_PREFIX = 0x2003,
    OSPFv3_INTER_AREA_ROUTER = 0x2004,
    OSPFv3_AS_EXTERNAL = 0x4005,
    OSPFv3_GROUP_MEMBERSHIP = 0x2006, // Not implemented
    OSPFv3_TYPE_7 = 0x2007, // Not implemented
    OSPFv3_LINK = 0x0008,
    OSPFv3_INTRA_AREA_PREFIX = 0x2009
} Ospfv3LinkStateType;

/// OSPF Link types.
typedef enum
{
    OSPFv3_POINT_TO_POINT = 1,
    OSPFv3_TRANSIT = 2,
    OSPFv3_RESERVED = 3,
    OSPFv3_VIRTUAL = 4
} Ospfv3LinkType;

/// 20 byte common OSPFv3 LSA header
typedef struct
{
    short linkStateAge;
    unsigned short linkStateType;
    unsigned int linkStateId;
    unsigned int advertisingRouter;
    int linkStateSequenceNumber;
    short linkStateCheckSum;
    short length;
} Ospfv3LinkStateHeader;

/// Each link of router LSA.
typedef struct
{
    unsigned char type;
    unsigned char reserved;    // not used set to 0
    unsigned short metric;
    unsigned int interfaceId;
    unsigned int neighborInterfaceId;
    unsigned int neighborRouterId;
} Ospfv3LinkInfo;

/// Various Router LSA bit Types. Used to set or reset bits.
/// Options bits will also be used by Packets.
enum
{
    OSPFv3_OPTIONS_V6_BIT = 0x00000001,
    OSPFv3_OPTIONS_E_BIT = 0x00000002,
    OSPFv3_OPTIONS_MC_BIT = 0x00000004,
    OSPFv3_OPTIONS_N_BIT = 0x00000008,
    OSPFv3_OPTIONS_R_BIT = 0x00000010,
    OSPFv3_OPTIONS_DC_BIT = 0x00000020,
    OSPFv3_B_BIT = 0x01000000,
    OSPFv3_E_BIT = 0x02000000,
    OSPFv3_V_BIT = 0x04000000,
    OSPFv3_W_BIT = 0x08000000
} ;

/// Represent OSPFv3 router LSA.
typedef struct
{
    Ospfv3LinkStateHeader LSHeader;

    // Next 32-bit variable will be used as follows
    // First 4 bits are  reserved and are set to 0.
    // Next bit is W-Bit. Set if the router is wild-Card Multicast Receiver.
    // Next bit is V-Bit. Set if the router is virtual Link End point.
    // Next bit is E-Bit. Set when the router is AS Boundary Router.
    // Next bit is B-Bit. Set when the router is area Border Router.
    // Rest 24-bits give options.
    unsigned int bitsAndOptions;

    // Rest part of LSA body is allocated dynamically by using
    // structure Ospfv3LinkInfo

} Ospfv3RouterLSA;

/// Represent OSPFv3 network LSA.
typedef struct
{
    Ospfv3LinkStateHeader LSHeader;

    // Next 32-bit variable will be used as follows
    // First 8-bits are reserved and set to 0.
    // Rest 24-bits gives options
    unsigned int bitsAndOptions;

    // Rest will be allocated dynamically.

} Ospfv3NetworkLSA;

/// Represent OSPFv3 Inter-Area-Prefix LSA
typedef struct
{
    Ospfv3LinkStateHeader LSHeader;

    // Next 32-bit variable will be used as follows
    // First 8-bits are reserved and set to 0.
    // Rest 24-bits gives options
    unsigned int bitsAndMetric;
    unsigned char prefixLength;
    unsigned char prefixOptions;
    short reserved; // set to 0
    in6_addr addrPrefix;
} Ospfv3InterAreaPrefixLSA;

/// Represent OSPFv3 Inter-Area-Router LSA
typedef struct
{
    Ospfv3LinkStateHeader LSHeader;

    // Next 32-bit variable will be used as follows
    // First 8-bits are reserved and set to 0.
    // Rest 24-bits gives options
    unsigned int bitsAndOptions;

    // Next 32-bit variable will be used as follows
    // First 8-bits are reserved and set to 0.
    // Rest 24-bits gives path metric.
    unsigned int bitsAndPathMetric;
    unsigned int destinationRouterId;
} Ospfv3InterAreaRouterLSA;

/// Bit types used to set or reset bits.
enum
{
    OSPFv3_T_BIT = 0x01000000,
    OSPFv3_F_BIT = 0x02000000,
    OSPFv3_EXTERNAL_LSA_E_BIT = 0x04000000
} ;

/// Represent OSPFv3 AS-external LSA
typedef struct
{
    Ospfv3LinkStateHeader LSHeader;

    // Next 32-bit variable will be used as follows
    // First 5-bits are reserved and set to 0.
    // Next bit is E-bit.
    // Next bit is F-bit.
    // Next bit is T-bit.
    // Rest 24-bits gives metric cost
    unsigned int OptionsAndMetric;
    unsigned char prefixLength;
    unsigned char prefixOptions;
    short referencedLSType;
    in6_addr addrPrefix;
} Ospfv3ASexternalLSA;

/// Prefix Options Bits type.
enum
{
    OSPFv3_PREFIX_NU_BIT = 0x01,
    OSPFv3_PREFIX_LA_BIT = 0x02,
    OSPFv3_PREFIX_MC_BIT = 0x04,
    OSPFv3_PREFIX_P_BIT = 0x08
};

/// Hold OSPFv3 Prefix Address information.
/// Used by Ospfv3LinkLSA and Ospfv3IntraAreaPrefixLSA.
typedef struct
{
    unsigned char prefixLength;
    unsigned char prefixOptions;
    unsigned short reservedOrMetric;
    in6_addr prefixAddr;
} Ospfv3PrefixInfo;

// STRUCT        :: Ospfv3LinkLSA;
// DESCRIPTION   :: Represent OSPFv3 Link LSA
typedef struct
{
    Ospfv3LinkStateHeader LSHeader;

    // Next 32-bit variable will be used as follows
    // First 8-bits gives router priority.
    // Rest 24-bits gives options.
    unsigned int priorityAndOptions;
    in6_addr linkLocalAddress;
    unsigned int numPrefixes;

    // Rest will be allocated dynamically

} Ospfv3LinkLSA;

/// Represent OSPFv3 Intra Area Prefix LSA
typedef struct
{
    Ospfv3LinkStateHeader LSHeader;
    unsigned short numPrefixes;
    unsigned short referencedLSType;
    unsigned int referencedLSId;
    unsigned int referencedAdvertisingRouter;

    // Rest will be allocated dynamically

} Ospfv3IntraAreaPrefixLSA;

/// Packet Type Consants.
typedef enum
{
    //fix for issue#1060 start
    OSPFv3_HELLO                    = 1,
    OSPFv3_DATABASE_DESCRIPTION     = 2,
    OSPFv3_LINK_STATE_REQUEST       = 3,
    OSPFv3_LINK_STATE_UPDATE        = 4,
    OSPFv3_LINK_STATE_ACK           = 5
    //fix for issue#1060 end
} Ospfv3PacketType;

/// 16 byte common packet header
typedef struct
{
    unsigned char version;
    unsigned char packetType;
    unsigned short packetLength;
    unsigned int routerId;
    unsigned int areaId;
    unsigned short checkSum; // Not used in simulation.

    // multiple ospf instance may be implemented later
    unsigned char instanceId;
    unsigned char reserved;    // set to 0
} Ospfv3CommonHeader;

/// ospfv3 Hello Packet
typedef struct
{
    Ospfv3CommonHeader commonHeader;
    unsigned int interfaceId;

    // Next 32-bit variable will be used as follows
    // First 8-bits gives router priority.
    // Rest 24-bits gives options.
    unsigned int priorityAndOptions;
    unsigned short helloInterval; // In seconds.
    unsigned short routerDeadInterval; // In seconds.
    unsigned int designatedRouter;
    unsigned int backupDesignatedRouter;

    // Neighbors will be dynamically allocated.

} Ospfv3HelloPacket;

/// Type of bits used during Database Description
/// Packet Exchanges.
enum
{
    OSPFv3_MS_BIT = 0x00000001,
    OSPFv3_M_BIT = 0x00000002,
    OSPFv3_I_BIT = 0x00000004
} ;

/// ospfv3 Database Description Packet
typedef struct
{
    Ospfv3CommonHeader commonHeader;

    // Next 32-bit variable will be used as follows
    // First 8-bits are reserved and set to 0.
    // Rest 24-bits gives options.
    unsigned int bitsAndOptions;

    // Next 32-bit variable is used as follows.
    // First 16-bit gives Interface MTU.
    // Next 13-bits are reserved. Set to 0.
    // Next bit is Init bit.
    // Next bit is More bit.
    // Next bit is Master/Slave bit
    unsigned int mtuAndBits;
    unsigned int ddSequenceNumber;

    // List of LSA Header (allocated dynamically)

} Ospfv3DatabaseDescriptionPacket;

/// ospfv3 Link State Update Packet
typedef struct
{
    Ospfv3CommonHeader commonHeader;
    int numLSA;

    // List of LSA (allocated dynamically).

} Ospfv3LinkStateUpdatePacket;

/// ospfv3 Link State Request info that will constitute
/// dynamic part of ospfv3 Link State Request Packet
typedef struct
{
    short reserved; // set to 0
    unsigned short linkStateType;
    unsigned int linkStateId;
    unsigned int advertisingRouter;
} Ospfv3LSRequestInfo;

/// ospfv3 Link State Request Packet
typedef struct
{
    Ospfv3CommonHeader commonHeader;

    // This part will be allocated dynamically

} Ospfv3LinkStateRequestPacket;

/// ospfv3 Link State Acknowledgment Packet
typedef struct
{
    Ospfv3CommonHeader commonHeader;

    // List of LSA Header (allocated dynamically)

} Ospfv3LinkStateAckPacket;

/// Possible OSPFv3 interface types.
typedef enum
{
    OSPFv3_NON_OSPF_INTERFACE,
    OSPFv3_POINT_TO_POINT_INTERFACE,
    OSPFv3_BROADCAST_INTERFACE,
    OSPFv3_NBMA_INTERFACE,
    OSPFv3_POINT_TO_MULTIPOINT_INTERFACE,
    OSPFv3_VIRTUAL_LINK_INTERFACE
} Ospfv3InterfaceType;

/// Different interface state.
/*
#ifndef OSPFv2_H
typedef enum
{
    S_Down,
    S_Loopback,
    S_Waiting,
    S_PointToPoint,
    S_DROther,
    S_Backup,
    S_DR
} Ospfv3InterfaceState;
#endif
*/

/// Event that cause interface state change.
/*
typedef enum
{
    E_InterfaceUp,
    E_WaitTimer,
    E_BackupSeen,
    E_NeighborChange,
    E_LoopInd,
    E_UnloopInd,
    E_InterfaceDown
} Ospfv3InterfaceEvent;
*/

/// Various values kept for each interface.
typedef struct
{
    unsigned int interfaceId;

    // multiple ospf instance may be implemented later
    unsigned int instanceId;
    LinkedList* linkLSAList;

    //list of unknown LSA with U bit zero
    LinkedList* unknownLSAlistUbitZero;
    in6_addr address;
    unsigned char prefixLen;
    LinkedList* linkPrefixList;
    Ospfv3InterfaceType type;
    Ospfv2InterfaceState state;
    unsigned int areaId;
    clocktype helloInterval;
    clocktype routerDeadInterval;
    clocktype infTransDelay;
    unsigned char routerPriority;
    LinkedList* neighborList;
    int neighborCount;
    unsigned int designatedRouter;
    in6_addr designatedRouterIPAddress;
    unsigned int backupDesignatedRouter;
    in6_addr backupDesignatedRouterIPAddress;
    int outputCost;
    clocktype rxmtInterval;
    BOOL floodTimerSet;
    LinkedList* updateLSAList;
    BOOL delayedAckTimer;
    LinkedList* delayedAckList;

    // Timer associated with self originated LSAs
    BOOL linkLSTimer;
    clocktype linkLSAOriginateTime;

    // Used only if DR on interface.
    clocktype networkLSAOriginateTime;

    // Used only if DR on interface.
    BOOL networkLSTimer;
    int networkLSTimerSeqNumber;

    BOOL ISMTimer;
} Ospfv3Interface;

/// Neighbor state type.
/*
typedef enum
{
    S_NeighborDown,
    S_Attempt,
    S_Init,
    S_TwoWay,
    S_ExStart,
    S_Exchange,
    S_Loading,
    S_Full
} Ospfv3NeighborState;
*/
/// Neighbor Events.
/*
typedef enum
{
    E_HelloReceived,
    E_Start,
    E_TwoWayReceived,
    E_NegotiationDone,
    E_ExchangeDone,
    E_BadLSReq,
    E_LoadingDone,
    E_AdjOk,
    E_SeqNumberMismatch,
    E_OneWay,
    E_KillNbr,
    E_InactivityTimer,
    E_LLDown
} Ospfv3NeighborEvent;
*/

#ifndef OSPFv2_H
/// Master or Slave type. During DD Packet Exchange two
/// adacent Router declare themself Master or Slave.
typedef enum
{
    T_Master,
    T_Slave
} Ospfv2MasterSlaveType;

#endif
/// Link state request item will be added to request list.
typedef struct
{
    Ospfv3LinkStateHeader* LSHeader;
    unsigned int seqNumber;
} Ospfv3LSReqItem;

/// Used to keep information needed for each neighbor.
typedef struct
{
    unsigned int neighborInterfaceId;
    Ospfv2NeighborState state;

    // Indicates that no Hello packet has been
    // seen from this neighbor recently.
    unsigned int inactivityTimerSequenceNumber;

    // Indicates that no ACK packet is received
    // for LSA sent out.
    unsigned int rxmtTimerSequenceNumber;
    Ospfv2MasterSlaveType masterSlave;
    unsigned int DDSequenceNumber;
    Message* lastReceivedDDPacket;
    unsigned int neighborId;
    int neighborPriority;
    in6_addr neighborIPAddress;
    int neighborOptions;
    unsigned int neighborDesignatedRouter;
    unsigned int neighborBackupDesignatedRouter;

    // Lists of LSA Update packets.
    LinkedList* linkStateRetxList;
    BOOL LSRetxTimer;
    LinkedList* DBSummaryList;
    LinkedList* linkStateRequestList;

    // Retransmission purpose
    clocktype lastDDPktSent;
    Message*    lastSentDDPkt;
    unsigned int LSReqTimerSequenceNumber;
    UInt32 optionsBits;
} Ospfv3Neighbor;

/// Store information for an eligible router.
typedef struct
{
    unsigned int routerId;
    int routerPriority;
    in6_addr routerIPAddress;
    int routerOptions;
    unsigned int DesignatedRouter;
    unsigned int BackupDesignatedRouter;
} Ospfv3DREligibleRouter;

/// OSPFv3 Destination Type.
typedef enum
{
    OSPFv3_DESTINATION_ABR,
    OSPFv3_DESTINATION_ASBR,
    OSPFv3_DESTINATION_ABR_ASBR,
    OSPFv3_DESTINATION_NETWORK
} Ospfv3DestType;

/// Various OSPFv3 Path Types.
typedef enum
{
    OSPFv3_INTRA_AREA,
    OSPFv3_INTER_AREA,
    OSPFv3_TYPE1_EXTERNAL,
    OSPFv3_TYPE2_EXTERNAL
} Ospfv3PathType;

/// Status of Route in routing table.
typedef enum
{
    OSPFv3_ROUTE_INVALID,
    OSPFv3_ROUTE_CHANGED,
    OSPFv3_ROUTE_NO_CHANGE,
    OSPFv3_ROUTE_NEW
} Ospfv3Flag;

/// A row struct for routing table.
typedef struct
{
    // Type of destination, either network or router
    Ospfv3DestType destType;
    in6_addr destAddr;

    //used if destination is network
    unsigned char prefixLength;

    // Optional capabilities
    unsigned char option;

    // Area whose LS info has led to this entry
    unsigned int areaId;

    // Type of path to destination
    Ospfv3PathType pathType;

    // Cost to destination
    Int32 metric;
    Int32 type2Metric;

    // Valid only for Intra-Area paths
    void* LSOrigin;

    // Next hop router address
    in6_addr nextHop;
    unsigned int outIntfId;

    // Valid only for intra-area and AS-External path
    unsigned int advertisingRouter;
    Ospfv3Flag flag;
} Ospfv3RoutingTableRow;

/// Routing table kept by ospf.
typedef struct
{
    int numRows;
    DataBuffer buffer;
} Ospfv3RoutingTable;

/// Statistics structure.
typedef struct
{
    unsigned int numHelloSent;
    unsigned int numHelloRecv;
    unsigned int numLSUpdateSent;
    unsigned int numLSUpdateRecv;
    unsigned int numAckSent;
    unsigned int numAckRecv;
    unsigned int numLSUpdateRxmt;
    unsigned int numExpiredLSAge;
    unsigned int numDDPktSent;
    unsigned int numDDPktRecv;
    unsigned int numDDPktRxmt;
    unsigned int numLSReqSent;
    unsigned int numLSReqRecv;
    unsigned int numLSReqRxmt;
    unsigned int numRtrLSAOriginate;
    unsigned int numNetLSAOriginate;
    unsigned int numInterAreaPrefixLSAOriginate;
    unsigned int numInterAreaRtrLSAOriginate;
    unsigned int numLinkLSAOriginate;
    unsigned int numIntraAreaPrefixLSAOriginate;
    unsigned int numASExternalLSAOriginate;
    unsigned int numLSARefreshed;
    BOOL alreadyPrinted;
} Ospfv3Stats;

/// Specify an address range for the area.
typedef struct
{
    in6_addr addressPrefix;
    unsigned char prefixLength;
    unsigned char options;
    BOOL advertise;
    BOOL advrtToArea[OSPFv3_MAX_ATTACHED_AREA];
    void* area;
} Ospfv3AreaAddressRange;

/// Ospfv3 area data structure.
typedef struct
{
    unsigned int areaId;
    LinkedList* areaAddrRange;
    LinkedList* connectedInterface;

    // LSDB for the area. All of these LSA have area flooding scope
    LinkedList* routerLSAList;
    LinkedList* networkLSAList;
    LinkedList* interAreaPrefixLSAList;
    LinkedList* interAreaRouterLSAList;
    LinkedList* intraAreaPrefixLSAList;

    //Unknown LSA handling may be implemented later
    LinkedList* unknownLSAUbitOne;

    // Shortest path tree of this area
    LinkedList* shortestPathList;

    BOOL transitCapability;
    BOOL externalRoutingCapability;
    Int32 stubDefaultCost;

    // Timer associated with self originated LSAs
    BOOL routerLSTimer;
    clocktype routerLSAOriginateTime;
    BOOL interAreaPrefixLSTimer;
    clocktype interAreaPrefixLSOriginateTime;
    BOOL groupMembershipLSTimer;
    clocktype groupMembershipLSAOriginateTime;

    // for group membership LSA
    LinkedList* groupMembershipLSAList;

} Ospfv3Area;

/// Ospfv3 Link State Id Item. A list of these items is kept
/// by each node.
typedef struct
{
    in6_addr destPrefix;
    unsigned int LSId;
} Ospfv3LinkStateIdItem;

/// Ospfv3 protocol data structure for a node
/// * */
typedef struct
{
    unsigned int asId;
    unsigned int routerId;
    LinkedList* area;
    BOOL partitionedIntoArea;
    BOOL areaBorderRouter;

    unsigned short seed[3];

    // Backbone kept seperately from other areas
    Ospfv3Area* backboneArea;
    clocktype SPFTimer;
    BOOL pWirelessFlag;

    // Virtual link not considered yet

// BGP-OSPF Patch Start
    // Only in ASBR
    LinkedList* unknownLSAUbitOne;
    LinkedList* asExternalLSAList;
    BOOL asBoundaryRouter;
// BGP-OSPF Patch End

    BOOL collectStat;
    Ospfv3Stats stats;
    Ospfv3RoutingTable routingTable;
    Ospfv3Interface* pInterface;
    int neighborCount;

    LinkedList* linkStateIdList;

    // holding delayed start time of ospf process.
    clocktype staggerStartTime;

} Ospfv3Data;

/// Keep information about various timers being triggred.
/// e.g Info field of Message structure used to simulate
/// various timer event may point to this structure.
typedef struct
{
    unsigned int sequenceNumber;
    unsigned int neighborId;

    // For retransmission timer.
    unsigned int advertisingRouter;

    Ospfv3PacketType type;
} Ospfv3TimerInfo;

/// Keep information about various neighbor for neighbor
/// events. e.g Info field of Message structure used to
/// simulate various neighbor event may point to
/// this structure.
typedef struct
{
    int interfaceId;
    unsigned int neighborId;
    Ospfv2NeighborEvent nbrEvent;
} Ospfv3NSMTimerInfo;

/// Keep information about various interface event being
/// triggred. e.g Info field of Message structure used to
/// simulate various interface event may point to
/// this structure.
typedef struct
{
    int interfaceId;
    Ospfv2InterfaceEvent intfEvent;
} Ospfv3ISMTimerInfo;

/// Initialize OSPFv3 routing protocol. This function is called
/// from IPv6Init of ipv6.cpp.
///
/// \param node  Pointer to node, used for
///    debugging.
/// \param ospfLayerPtr  Pointer to Pointer of OSPFv3
///    Protocol Data Structure.
/// \param nodeInput  Pointer to the configuration
///    file
/// \param interfaceIndex  Node's interface Index.
///
void Ospfv3Init(
    Node* node,
    Ospfv3Data** ospfLayerPtr,
    const NodeInput* nodeInput,
    int interfaceIndex);

/// Prints out the required statistics for OSPFv3 protocol after
/// completion of the simulation.
///
/// \param node  Pointer to node, used for
///    debugging.
/// \param interfaceIndex  Node's interface Index.
///
void Ospfv3Finalize(Node* node, int interfaceIndex);

/// Self-sent messages are handled in this function. Generally,
/// this function handles the timers required for the
/// implementation of OSPF. This function is called from the
/// funtion Ipv6Layer()in ipv6.cpp.
///
/// \param node  Pointer to node, used for
///    debugging.
/// \param msg  Message representing a timer
///    event for the protocol.
///
/// * */
void Ospfv3HandleRoutingProtocolEvent(Node* node, Message* msg);


/// Routing Protocol packet processor. This Function is called
/// from ip6_deliver of ip6_input.cpp.
///
///    node            :  Node*                  : Pointer to node, used for
///    debugging.
/// \param msg  Message representing a packet
///    event for the protocol.
/// \param sourceAddress  Source Address of Packet.
/// \param interfaceIndex  Node's interface Index.
///
void Ospfv3HandleRoutingProtocolPacket(
    Node* node,
    Message* msg,
    in6_addr sourceAddress,
    int interfaceIndex);

/// Address of this function is assigned in the IPv6 interface
/// structure during initialization. IP forwards packet by
/// getting the nextHop from this function. This fuction will
/// be called via a pointer, RouterFunction, in the IPv6
/// interface structure, from function
/// RoutePacketAndSendToMac() in ip.cpp
/// + node            :  Node*                  : Pointer to node, used for
/// debugging.
/// + msg             :  Message*               : Message representing a packet
/// event for the protocol.
/// + destAddr        :  in6_addr               : Destination Network Address.
/// + packetRouted    :  BOOL*                  : If set FALSE, then it allows
/// IPv6 to route the packet.
void Ospfv3RouterFunction(
    Node* node,
    Message* msg,
    in6_addr destAddr,
    in6_addr prevHopAddr,
    BOOL* packetRouted);

/// Handle mac interface status alert.
/// + node            :  Node*                    : Pointer to node, used for
/// debugging.
/// + interfaceIndex  :  interfaceIndex           : Node's interface Index.
/// + state           :  MacInterfaceState        : MAC status of interface.
void Ospfv3InterfaceStatusHandler(
    Node* node,
    int interfaceIndex,
    MacInterfaceState state);

/// Print out packet trace information in XML format.
///
/// \param node  Pointer to node, doing the
///    packet trace
/// \param msg  Message representing a packet
///    event for the protocol.
///
void Ospfv3PrintTraceXML(Node* node, Message* msg);


/// Flood LSA throughout the area.
///
/// \param node  Pointer to node, doing the
///    packet trace
/// \param LSA  Poiter to an LSA.
/// \param neighborId  Neighbor ID
/// \param areaId  Area Id to which LSA need
///    to be flooded.
///
/// \return TRUE if LSA flooded back to receiving interface,
/// FALSE otherwise.
BOOL Ospfv3FloodLSAThroughArea(
    Node* node,
    char* LSA,
    unsigned int neighborId,
    unsigned int areaId);

/// Get desired route from the OSPFv3 routing table.
///
/// \param node  Pointer to node, doing the
///    packet trace
/// \param LSA  Poiter to an LSA.
/// \param neighborId  Neighbor ID
/// \param areaId  Area Id to which LSA need
///    to be flooded.
///
/// \return Route entry. NULL if route not found.
Ospfv3RoutingTableRow* Ospfv3GetValidRoute(
    Node* node,
    in6_addr destAddr,
    Ospfv3DestType destType);

/// Search for the LSA in list.
///
/// \param node  Pointer to node.
/// \param list  Pointer to Linked list.
/// \param advertisingRouter  Advertising Router.
/// \param linkStateId  Link State Id.
///
/// \return Return pointer to Link State header if found,
/// otherwise return NULL.
Ospfv3LinkStateHeader* Ospfv3LookupLSAList(
    Node* node,
    LinkedList* list,
    unsigned int advertisingRouter,
    unsigned int linkStateId);

/// Print Routing Table of the node.
///
/// \param node  Pointer to Node.
///
void Ospfv3PrintRoutingTable(Node* node);

/// Print Network LSA List of an Area.
///
/// \param node  Pointer to node.
/// \param thisArea  Pointer to Area Structure.
///
void Ospfv3PrintNetworkLSAList(Node* node);

/// Create LS Header for newly created LSA.
///
///    +node:  Node*                              : Pointer to node.
///    +areaId:  unsigned int                     : Area Id.
///    +linkStateType: Ospfv3LinkStateType        : Type of Link State.
///    +linkStateHeader: Ospfv3LinkStateHeader*   : Pointer to Link State Header.
///    +oldLSHeader:  Ospfv3LinkStateHeader*      : Pointer to Old Link State
///    Header
///
/// \return FALSE if old LSA has MAX sequence number, TRUE otherwise.
BOOL Ospfv3CreateLSHeader(
    Node* node,
    unsigned int areaId,
    Ospfv3LinkStateType linkStateType,
    Ospfv3LinkStateHeader* linkStateHeader,
    Ospfv3LinkStateHeader* oldLSHeader);

/// Install LSAs in the database.
///
/// \param node  Pointer to node.
/// \param list  Pointer to linked list.
/// \param LSA  Pointer to LSA.
///
/// \return TRUE if LSDB changed, FALSE otherwise.
BOOL Ospfv3InstallLSAInLSDB(
    Node* node,
    LinkedList* list,
    char* LSA);

/// To remove AS-External LSA from LSDB call this function
/// with areaId argument as OSPFv3_INVALID_AREA_ID.
/// For Link LSA areaId will be interfaceId.
///
///    +node      :  Node* : Pointer to node.
///    +LSA       : char* : Pointer to LSA.
///    +areaId    : unsigned int : Area Id.
///
void Ospfv3RemoveLSAFromLSDB(
    Node* node,
    char* LSA,
    unsigned int areaId);

/// To Get Address Range For All Area.
///
///    +node      :  Node* : Pointer to node.
///    +destAddr  :  in6_addr : Dest Address.
///
/// \return Area Address Ragne Info.
Ospfv3AreaAddressRange* Ospfv3GetAddrRangeInfo(
    Node* node,
    unsigned int areaId,
    in6_addr destAddr);

/// Print Link State Header.
///
/// \param LSHeader  Pointer to node.
///
void Ospfv3PrintLSHeader(Ospfv3LinkStateHeader* LSHeader);

/// Get Area Id from chached .ospf file.
///
/// \param node  Pointer to Node.
/// \param interface  Interface Identifier.
/// \param ospfConfigFile  Pointer to chached input file.
/// \param areaId  Pointer to Area Id Variable.
///
/// \return Pointer to area if found, else NULL.
Ospfv3Area* Ospfv3GetArea(Node* node, unsigned int areaId);

/// Print the LSA.
///
/// \param LSA  Pointer to bytes of LSA.
///
void Ospfv3PrintLSA(char* LSA);

/// Originate router LSA for the specified area. If areaId
/// passed as OSPFv3_SINGLE_AREA_ID, then consider total
/// domain as single area, and include all functional
/// interfaces.
///
///    +node      :  Node* : Pointer to node.
///    +areaId    :  unsigned int : Area Id.
///    +flush     :  BOOL : Flush or not Flush.
///
void Ospfv3OriginateRouterLSAForThisArea(
    Node* node,
    unsigned int areaId,
    BOOL flush);

/// Print LSA header trace information in XML format
///
/// \param node  Pointer to node, doing the packet trace
/// \param lsaHdr  Pointer to Ospfv3LinkStateHeader
///
void Ospfv3PrintTraceXMLLsaHeader(
    Node* node,
    Ospfv3LinkStateHeader* lsaHdr);

/// To Originate Link LSA For This Interface.
///
///    +node        :  Node*            : Pointer to node.
///    +interfaceId :  unsigned int     : Interface Id.
///    +flush       :  BOOL             : Flush or Not to Flush.
///
void Ospfv3OriginateLinkLSAForThisInterface(
    Node* node,
    unsigned int interfaceId,
    BOOL flush);

/// To Flood LSA On Interface.
///
///    +node        :  Node*        : Pointer to node.
///    +LSA         :  char*        : Pointer to LSA
///    +neighborId  :  unsigned int : Neighbor Id.
///    +interfaceId :  unsigned int : Interface Id.
///
void Ospfv3FloodLSAOnInterface(
    Node* node,
    char* LSA,
    unsigned int neighborId,
    unsigned int interfaceId);

/// Check Interior Gateway protocol(IGP) is OSPF or NOT.
/// If IGP is OSPF then External routing information is injected.
/// If flush is TRUE then External routing Information is
/// flushed from routing domain.
///
///    +node     :  Node*             : Pointer to node, used for
///    debugging.
///    +destAddress :  in6_addr*      : External Destination Address
///    +prefixLength :  unsigned char : Destination Subnet Prefix Length
///    +nextHopAddress :  in6_addr    : Next Hop for external Destination
///    +external2Cost :  int          : Type 2 cost
///    +flush :  BOOL                 : Whether to flush External Info or NOT
///
/// \return Return TRUE if runnning OSPF Otherwise FALSE
BOOL Ospfv3IsEnabled(
    Node *node,
    in6_addr destAddress,
    unsigned char prefixLength,
    in6_addr nextHopAddress,
    int external2Cost,
    BOOL flush);

//---------------------------------------------------------------------------
// FUNCTION   :: RoutingOSPFv3HandleAddressChangeEvent
// LAYER      :: APPLICATION
// PURPOSE    :: Handles any change in the interface address
//               due to autoconfiguration feature
// PARAMETERS ::
// + node               : Node* : Pointer to Node structure
// + interfaceIndex     : Int32 : interface index
// + oldGlobalAddress   : in6_addr* old global address
// RETURN :: void : NULL
//---------------------------------------------------------------------------
void RoutingOSPFv3HandleAddressChangeEvent(
    Node* node,
    Int32 interfaceId,
    in6_addr* oldGlobalAddress);

#endif // OSPFv3_H
