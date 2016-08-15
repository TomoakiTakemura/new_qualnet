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

/// \defgroup Package_NETWORK_LAYER NETWORK LAYER

/// \file
/// \ingroup Package_NETWORK_LAYER
/// This file describes the data structures and functions used by the Network Layer.

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include "mapping.h"

//---------------------------------------------------------------------------
// DEFINES
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Queues
//---------------------------------------------------------------------------

/// Default number of output queue per interface
#define DEFAULT_IP_QUEUE_COUNT  3

/// Default size of CPU queue (in byte)
#define DEFAULT_CPU_QUEUE_SIZE                  640000

/// Default size in bytes of an input queue, if it's not
/// specified in the input file with the
/// IP-QUEUE-PRIORITY-INPUT-QUEUE-SIZE parameter.
#define DEFAULT_NETWORK_INPUT_QUEUE_SIZE        150000

/// Default size in bytes of an output queue, if it's not
/// specified in the input file with the
/// IP-QUEUE-PRIORITY-QUEUE-SIZE parameter.
#define DEFAULT_NETWORK_OUTPUT_QUEUE_SIZE       150000

/// Default Ethernet MTU(Maximum transmission unit) in bytes.
/// QualNet does not model Ethernet yet, but this value is
/// used (in the init functions in network/fifoqueue.c and
/// network/redqueue.c) to compute the initial number of
/// Message * instances that are used to store packets in
/// queues.Regardless, the buffer capacity of a queue is not
/// the number of Message * instances, but a certain number
/// of bytes, as expected.
#define DEFAULT_ETHERNET_MTU                      1500

/// Maximum IP packet size
#ifndef IP_MAXPACKET
#define IP_MAXPACKET    65535
#endif

/// Maximum throughput of backplane of network.
#define NETWORK_IP_UNLIMITED_BACKPLANE_THROUGHPUT (0)

/// Status of backplane (either busy or idle)
enum NetworkIpBackplaneStatusType
{
    NETWORK_IP_BACKPLANE_IDLE,
    NETWORK_IP_BACKPLANE_BUSY
};


//-----------------------------------------------------------------------------
// STRUCTS, ENUMS, AND TYPEDEFS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Queues
//-----------------------------------------------------------------------------

//
// The priority value of queues and packets is type unsigned.
// The priority value is used in the application layer to specify
// priority values for packets; in the network layer to create and
// access queues of different priorities; and in the MAC layer also
// to access the network-layer queues.
//

typedef int TosType;
typedef TosType QueuePriorityType;

//
// Enumeration values assigned to different routing protocols of both
// the network and application layers.  These values are used by layer
// functions and when sending messages.
//

//-----------------------------------------------------------------------------
// Routing
//-----------------------------------------------------------------------------

//
// Administrative distances.  [These values don't match the
// recommended Cisco values, yet. -Jeff]
//

/// Administrative distance of different routing protocol
enum NetworkRoutingAdminDistanceType
{
    ROUTING_ADMIN_DISTANCE_STATIC = 1,

    ROUTING_ADMIN_DISTANCE_EBGPv4_HANDOFF = 111,

    ROUTING_ADMIN_DISTANCE_EBGPv4 = 20,
    ROUTING_ADMIN_DISTANCE_IBGPv4 = 200,
    ROUTING_ADMIN_DISTANCE_BGPv4_LOCAL = 200,
    ROUTING_ADMIN_DISTANCE_OSPFv2 = 110,
    ROUTING_ADMIN_DISTANCE_IGRP = 100,
    ROUTING_ADMIN_DISTANCE_STAR,
    ROUTING_ADMIN_DISTANCE_RIPv3,
    ROUTING_ADMIN_DISTANCE_BELLMANFORD,
    ROUTING_ADMIN_DISTANCE_FISHEYE,

    ROUTING_ADMIN_DISTANCE_OSPFv2_TYPE1_EXTERNAL = 120,
    ROUTING_ADMIN_DISTANCE_OSPFv2_TYPE2_EXTERNAL,


    ROUTING_ADMIN_DISTANCE_OLSR,
    ROUTING_ADMIN_DISTANCE_EIGRP,

    ROUTING_ADMIN_DISTANCE_RIP,
    ROUTING_ADMIN_DISTANCE_RIPNG,

    ROUTING_ADMIN_DISTANCE_OSPFv3 = 115,

    ROUTING_ADMIN_DISTANCE_OLSRv2_NIIGATA,

    ROUTING_ADMIN_DISTANCE_FSRL = 210,

    // Should always have the highest adminstrative distance
    // (ie, least important).
    ROUTING_ADMIN_DISTANCE_DEFAULT = 255
};

/// Enlisted different network/routing protocol
enum NetworkRoutingProtocolType
{
    NETWORK_PROTOCOL_IP = 0,
    NETWORK_PROTOCOL_IPV6,
    NETWORK_PROTOCOL_MOBILE_IP,
    NETWORK_PROTOCOL_NDP,
    NETWORK_PROTOCOL_SPAWAR_LINK16,
    NETWORK_PROTOCOL_ICMP,
    ROUTING_PROTOCOL_AODV,
    ROUTING_PROTOCOL_DSR,
    ROUTING_PROTOCOL_FSRL,
    ROUTING_PROTOCOL_STAR,
    ROUTING_PROTOCOL_LAR1,
    ROUTING_PROTOCOL_ODMRP,
    ROUTING_PROTOCOL_OSPF,
    ROUTING_PROTOCOL_OSPFv2,
    ROUTING_PROTOCOL_SDR,  //military radios eplrs is deprecated
    ROUTING_PROTOCOL_BELLMANFORD,
    ROUTING_PROTOCOL_STATIC,
    ROUTING_PROTOCOL_ICMP_REDIRECT,
    ROUTING_PROTOCOL_DEFAULT,
    ROUTING_PROTOCOL_FISHEYE,
    ROUTING_PROTOCOL_OLSR_INRIA,
    ROUTING_PROTOCOL_IGRP,
    ROUTING_PROTOCOL_EIGRP,
    ROUTING_PROTOCOL_BRP,
    ROUTING_PROTOCOL_RIP,
    ROUTING_PROTOCOL_RIPNG,
    ROUTING_PROTOCOL_IARP,
    ROUTING_PROTOCOL_ZRP,
    ROUTING_PROTOCOL_IERP,

    EXTERIOR_GATEWAY_PROTOCOL_EBGPv4,
    EXTERIOR_GATEWAY_PROTOCOL_IBGPv4,
    EXTERIOR_GATEWAY_PROTOCOL_BGPv4_LOCAL,

    GROUP_MANAGEMENT_PROTOCOL_IGMP,
    LINK_MANAGEMENT_PROTOCOL_CBQ,

    MULTICAST_PROTOCOL_STATIC,
    MULTICAST_PROTOCOL_DVMRP,
    MULTICAST_PROTOCOL_MOSPF,
    MULTICAST_PROTOCOL_ODMRP,

    MULTICAST_PROTOCOL_PIM,

    // ADDON_MAODV
    MULTICAST_PROTOCOL_MAODV,

    NETWORK_PROTOCOL_GSM,
    NETWORK_PROTOCOL_ARP,

    ROUTING_PROTOCOL_OSPFv3,
    ROUTING_PROTOCOL_OLSRv2_NIIGATA,

    ROUTING_PROTOCOL_ALL,

    NETWORK_PROTOCOL_CELLULAR,


    ROUTING_PROTOCOL_AODV6,
    ROUTING_PROTOCOL_DYMO,
    ROUTING_PROTOCOL_DYMO6,

    // Network Security
    ROUTING_PROTOCOL_ANODR,
    NETWORK_PROTOCOL_SECURENEIGHBOR,
    NETWORK_PROTOCOL_SECURECOMMUNITY,
    NETWORK_PROTOCOL_ATTACK,
    NETWORK_PROTOCOL_IPSEC_AH,
    NETWORK_PROTOCOL_IPSEC_ESP = 5199,

    // NGCNMS
    NETWORK_PROTOCOL_NGC_HAIPE,

    NETWORK_ROUTE_REDISTRIBUTION,

    // Military Radios
    ROUTING_PROTOCOL_ODR,  //military radios eplrs is deprecated
    NETWORK_PROTOCOL_EPLRS,   //military radios eplrs is deprecated

    ROUTING_PROTOCOL_OSPFv2_TYPE1_EXTERNAL,
    ROUTING_PROTOCOL_OSPFv2_TYPE2_EXTERNAL,

    // MSDP
    MULTICAST_PROTOCOL_MSDP,
    ROUTING_PROTOCOL_NONE // this must be the last one
};

//-----------------------------------------------------------------------------
// Network layer
//-----------------------------------------------------------------------------

//
// typedef to NetworkDataIp in network/ip.h.
//
struct NetworkDataIp;

#ifdef CELLULAR_LIB
// defined in gsm_layer3.h
struct struct_layer3_gsm_str;
#endif // CELLULAR_LIB

//Function ptrs for Reseting Network Layer.  Used for NMS.
typedef void (*NetworkSetFunctionPtr)(Node*, int);

struct NetworkResetFunction
{
    // Corresponding set function
    NetworkSetFunctionPtr funcPtr;

    // the next match command
    NetworkResetFunction* next;
};

struct NetworkResetFunctionList
{
    NetworkResetFunction* first;
    NetworkResetFunction* last;
};

/// Main data structure of network layer
struct NetworkData
{
    NetworkDataIp* networkVar;  // IP state

    NetworkProtocolType networkProtocol;
#ifdef CELLULAR_LIB
    struct struct_layer3_gsm_str *gsmLayer3Var;
#endif
    struct struct_cellular_layer3_str *cellularLayer3Var;

    BOOL networkStats;  // TRUE if network statistics are collected

    //It is true if ARP is enabled
    BOOL isArpEnable;
    //It is true if RARP is enabled
    BOOL isRarpEnable;
    struct address_resolution_module *arModule;

    BOOL useNetworkCesQosDiffServ;

#ifdef ADDON_NGCNMS
    NetworkResetFunctionList* resetFunctionList;
#endif

    struct PKIData* pkiData;

};

//////////////////////////////
// MI Management API header //
//////////////////////////////

/// Type of management report message
typedef enum ManagementReportTypeT {
    ManagementReportUnspecified = 1,
    ManagementReportEcho,
    ManagementReportWnwMiMdlBandwidth,
    ManagementReportWnwMiMdlTxStatus,
    ManagementReportWnwMiMdlLinkStatus,
    ManagementReportWnwMiMdlQueueStatus,
    ManagementReportWnwMiMdlPushPacket
} ManagementReportType;

/// data structure of management report
typedef struct ManagementReportT {
    ManagementReportType type;
    void* data;
} ManagementReport;

/// Type of management response message
typedef enum ManagementReportResponseTypeT {
    ManagementReportResponseOK = 1,
    ManagementReportResponseUnsupported,
    ManagementReportResponseIllformedRequest,
    ManagementReportResponseUnspecifiedFailure
} ManagementReportResponseType;

/// data structure of management response
typedef struct ManagementReportResponseT {
    ManagementReportResponseType result;
    void *data;
} ManagementReportResponse;

/// Deliver a MAC management request to the NETWORK layer
///
/// \param node  Pointer to a network node
/// \param interfaceIndex  index of interface
/// \param report  Pointer to a management report
/// \param resp  Pointer to a management response
void
NETWORK_ManagementReport(
    Node *node, 
    int interfaceIndex,
    ManagementReport* report, 
    ManagementReportResponse* resp);


//-----------------------------------------------------------------------------
// INLINED FUNCTIONS (none)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PROTOTYPES FOR FUNCTIONS WITH EXTERNAL LINKAGE
//-----------------------------------------------------------------------------

/// Returns interface information for a interface. Information
/// means its address and type
/// 
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index for which info required.
/// \param address  interface info returned
///
void NetworkGetInterfaceInfo(
    Node* node,
    int interfaceIndex,
    Address *address,
    NetworkType networkType = NETWORK_IPV4);

/// ipAddrString is filled in by interface's ipv6 address
/// in character format.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index.
/// \param ipAddrString  Pointer to string ipv6 address.
void
NetworkIpGetInterfaceAddressString(
    Node *node,
    const int interfaceIndex,
    char ipAddrString[]);

/// Returns type of network (ipv4 or ipv6) the interface.
/// 
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index.
NetworkType NetworkIpGetInterfaceType(
    Node* node,
    int interfaceIndex);


/// Network-layer receives packets from MAC layer, now check
/// Overloaded Function to support Mac Address
/// type of IP and call proper function
///
/// \param node  Pointer to node
/// \param message  Message received
/// \param lastHopAddress  last hop address
/// \param interfaceIndex  incoimg interface
void NETWORK_ReceivePacketFromMacLayer(Node* node,
    Message* msg,
    MacHWAddress* macAddr,
    int interfaceIndex);

/// Reset Network protocols and/or layer.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index.
void
NETWORK_Reset(Node *node,
              int interfaceIndex);

/// Add which protocols to be reset to a
/// fuction list pointer.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index.
void
NETWORK_AddResetFunctionList(Node* node,
                             void *param);

#endif // _NETWORK_H_
