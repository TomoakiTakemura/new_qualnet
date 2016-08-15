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

// PROTOCOL ::

// SUMMARY ::
/// In order to transition the entire Internet from Ipv4 to Ipv6, the already
/// deployed Ipv4-system requires the capability to carry Ipv6 datagrams
/// across the chain of Ipv4 routing infrastructures. One of the obvious
/// solutions is providing Ipv6 routers/hosts with Ipv4 compatibility
/// mechanisms. Such mechanisms equip Ipv6 routers/hosts with dual-stack
/// implementation so that they can concurrently support both the IPv4 and
/// IPv6 protocol stacks. While forwarding an Ipv6 packet via an Ipv4
/// interface, these dual-stack enabled routers encapsulate the packet in Ipv4
/// datagram and tunnel them over the Ipv4 infrastructure. In similar fashion,
/// Ipv4 datagrams can also be routed across a chain of intervening Ipv6
/// routers (i.e. over an Ipv6 tunnel) to an Ipv4 router/host.

// LAYER ::  Network Layer

// STATISTICS ::

// APP_PARAM ::

// CONFIG_PARAM ::

// SPECIAL_CONFIG :: <filenale>.tunnel : Tunnel configuration File


// VALIDATION ::$QUALNET_HOME/verification/dualip

// IMPLEMENTED_FEATURES ::


// OMITTED_FEATURES :: (Title) : (Description) (list)


// ASSUMPTIONS ::

// STANDARD :: Coding standard follows the following Coding Guidelines
//             1. SNT C Coding Guidelines for QualNet 4.0

// RELATED :: IPv4, IPv6

#ifndef DUALIP_H_
#define DUALIP_H_

#include "api.h"
#include "network_ip.h"
#include "ipv6_route.h"
#include "ip6_opts.h"

/// Types of various tunnels
typedef enum
{
    UNKNOWN_TUNNEL,
    IPv4_TUNNEL,
    IPv6_TUNNEL,
    SixToFour_TUNNEL
} TunnelType;

// STRUCT      :: TunnelStatsType
// DESCRIPTION :: It is used to hold stats of a Tunnel
typedef struct
{
    unsigned int numUnicstSent;
    unsigned int numBroadcastSent;
    unsigned int numMulticastSent;
    unsigned int numUnicstRecv;
    unsigned int numBroadcastRecv;
    unsigned int numMulticastRecv;
    unsigned int numPktDroppedTunnelFailure;
} TunnelStatsType;

/// A tunnel structure to hold tunnel information
typedef struct
{
    TunnelType      tunnelType;
    int             pseudoIntIndex;
    int             originalIntIndex;
    int             borrowedIntIndex;
    Address         tunnelStartAddress;
    Address         tunnelEndAddress;
    TunnelStatsType tunnelStats;
    Int64             tunnelBandwidth;
    clocktype             propagationDelay;
    LinkedList      *interfaceStatusHandlerList;
} TunnelInfoType;

/// Tunnel Initialization.
///
/// \param node  Pointer to node structure.
/// \param nodeInput  Node input.
///
void TunnelInit(Node* node, const NodeInput* nodeInput);

/// Tunnel Finalization.
///
/// \param node  Pointer to node structure.
/// \param interfaceIndex  interface Index.
///
void TunnelFinalize(Node* node, int interfaceIndex);

// FUNCTION            :: Is6to4Interface
// LAYER               :: Network
// PURPOSE             :: checking for 6to4 Interface.
// PARAMETERS          ::
// + node              :: Node*   : Pointer to node structure.
// + intIndex        ::   int     :.Interface Index
// RETURN              :: BOOL
BOOL Is6to4Interface(Node* node, int intIndex);

// FUNCTION            :: TunnelIsVirtualInterface
// LAYER               :: Network
// PURPOSE             :: checking for virtual Interface.
// PARAMETERS          ::
// + node              :: Node*   : Pointer to node structure.
// + intIndex        ::   int     :.Interface Index
// RETURN              :: BOOL
BOOL TunnelIsVirtualInterface(Node* node, int intIndex);

/// Encapsulates IPv6 pkt within IPv4 pkt to
/// route it through IPv4-tunnel.
///
/// \param node  Pointer to node structure.
/// \param msg  Ptr to Messasge containing
///    IPv6 pkt to be encapsulated.
///    +imo                 : ip_moptions *imo: Multicast Option Pointer.
///    +interfaceID         : int interfaceID : Index of the concerned Interface
///
/// \return BOOL
BOOL TunnelSendIPv6Pkt(
    Node* node,
    Message* msg,
    ip_moptions* imo,
    int interfaceID);

/// Decapsulates IPv6 pkt from IPv4 pkt
///
/// \param node  Pointer to node structure.
/// \param msg  Ptr to Messasge containing
///    IPv6 pkt encapsulated within IPv4 pkt.
///    +interfaceIndex      : int : Interface Index
///
void TunnelHandleIPv6Pkt(
    Node *node,
    Message* msg,
    int interfaceIndex);

/// Encapsulates IPv4 pkt within IPv6 pkt to
/// route it through IPv6-tunnel.
///
/// \param node  Pointer to node structure.
/// \param msg  Ptr to Messasge containing
///    IPv6 pkt to be encapsulated.
/// \param outgoingInterface  Index of outgoing interface.
/// \param nextHop  Next hop IP address.
///
/// \return BOOL
BOOL TunnelSendIPv4Pkt (
    Node *node,
    Message *msg,
    int outgoingInterface,
    NodeAddress nextHop);

/// Decapsulates IPv4 pkt from IPv6 pkt and send
/// it to the IPv4 layer's pkt-receiving function.
///
/// \param node  Pointer to node structure.
/// \param msg  Ptr to Messasge containing
///    IPv4 pkt encapsulated within IPv6 pkt.
///    +interfaceIndex      : int : Interface Index
///
void TunnelHandleIPv4Pkt(
    Node *node,
    Message* msg,
    int interfaceIndex);

/// Borrow Routing Protocol type from a interface.
///
/// \param node  node structure pointer.
/// \param interfaceIndex  interface Index.
///
void TunnelSetRoutingProtocolType(Node* node, int interfaceIndex);

/// Get Bandwidth of the tunnel.
///
/// \param node  node structure pointer.
/// \param interfaceIndex  interface Index.
///
/// \return Int64
Int64 getTunnelBandwidth(Node* node, int interfaceIndex);

/// Get Propagaion delay for the tunnel.
///
/// \param node  node structure pointer.
/// \param interfaceIndex  interface Index.
///
/// \return clocktype
clocktype getTunnelPropDelay(Node* node, int interfaceIndex);

/// To check the tunnel is enabled or not?
///
/// \param node  node structure pointer.
/// \param interfaceIndex  interface Index.
///
/// \return BOOL
BOOL TunnelInterfaceIsEnabled(Node* node, int interfaceIndex);

/// parse and initialize 6to4 tunnels.
///
/// \param node  node structure pointer.
///    + nodeInput          : const NodeInput*.
///
void Initialize6to4Tunnel(Node* node, const NodeInput* nodeInput);

/// Call the interface handler functions when interface
/// goes up and down.
///
/// \param node  Pointer to node structure.
///    +interfaceIndex      : int : Interface Index
///    +state               : MacInterfaceState : current Interface state
///
void TunnelInformInterfaceStatusHandler(Node *node,
                                      int interfaceIndex,
                                      MacInterfaceState state);

/// Set the interface handler function to be called
/// when interface faults occur.
///
/// \param node  Pointer to node structure.
///    +interfaceIndex      : int : Interface Index
///    +statusHandler       : MacReportInterfaceStatus : status Handler
///
void
TunnelSetInterfaceStatusHandlerFunction(Node *node,
                                      int interfaceIndex,
                                      MacReportInterfaceStatus statusHandler);

/// Tunnel callback functions when a packet is dropped.
///
/// \param node  Pointer to node structure.
/// \param msg  Ptr to Messasge containing
///    IPv6 pkt encapsulated within IPv4 pkt.
///    +interfaceIndex      : int : Interface Index
///
void
TunnelNotificationOfIPV6PacketDrop(Node *node,
                                  Message *msg,
                                  int interfaceIndex);

#endif
