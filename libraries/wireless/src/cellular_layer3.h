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

/// \defgroup Package_CELLULAR_LAYER3 CELLULAR_LAYER3

/// \file
/// \ingroup Package_CELLULAR_LAYER3
/// Defines the data structures used in the Cellular Framework
/// Most of the time the data structures and functions start
/// with CellularLayer3**

#ifndef _CELLULAR_LAYER3_H_
#define _CELLULAR_LAYER3_H_

#include "cellular.h"

#ifdef CELLULAR_LIB
//Abstrat
#include "cellular_abstract.h"
#include "cellular_abstract_layer3.h"
#endif

#ifdef UMTS_LIB
//UMTS
#include "cellular_umts.h"
#include "layer3_umts.h"
#endif


//define the maximum cellular interfece a cellular node could have.
//Currently only one cellular interface is supported besides the IP interface
//6 would match the definition of protocol type
//#define CELLULAR_MAXIMUM_INTERFACE_PER_NODE 1
//#define CELLULAR_INTERFACE_ABSTRACT   0
//#define CELLULAR_INTERFACE_GSM        1
//#define CELLULAR_INTERFACE_GPRS       2
//#define CELLULAR_INTERFACE_UMTS       3
//#define CELLULAR_INTERFACE_CDMA2000   4

/// Enlisted different Cellular Protocols
typedef enum
{
    Cellular_ABSTRACT_Layer3 = 0,
    Cellular_GSM_Layer3,
    Cellular_GPRS_Layer3,
    Cellular_UMTS_Layer3,
    Cellular_CDMA2000_Layer3
}CellularLayer3ProtocolType;

/// Definition of the data structure for the cellular Layer3
typedef struct struct_cellular_layer3_str
{
    // Must be set at initialization for every Cellular node
    CellularNodeType nodeType;

    RandomSeed randSeed;
    int interfaceIndex;
    CellularLayer3ProtocolType cellularLayer3ProtocolType;

    // Each interface has only one of the following based on its type,
    // one node cannot have two identical types of interface,
    // so the total interface is liitted by the number of protocol type
#ifdef CELLULAR_LIB
    // Abstract Cellular Model
    CellularAbstractLayer3Data *cellularAbstractLayer3Data;

    //The following not defined yet
    //CellularGSMLayer3Data *cellularGSMLayer3Data;
    //CellularGPRSLayer3Data *cellularGPRSLayer3Data;
#endif

#ifdef UMTS_LIB
    // UMTS model
    UmtsLayer3Data *umtsL3;
#endif


    BOOL collectStatistics;
}CellularLayer3Data;

//--------------------------------------------------------------------------
//  API functions
//--------------------------------------------------------------------------

/// Preinitialize Cellular Layer protocol
///
/// \param node  Pointer to node.
/// \param nodeInput  Pointer to node input.
///
void CellularLayer3PreInit(Node *node, const NodeInput *nodeInput);

/// Initialize Cellular Layer protocol
///
/// \param node  Pointer to node.
/// \param nodeInput  Pointer to node input.
///
void CellularLayer3Init(Node *node, const NodeInput *nodeInput);

/// Print stats and clear protocol variables
///
/// \param node  Pointer to node.
///
void CellularLayer3Finalize(Node *node);

/// Handle timers and layer messages.
///
/// \param node  Pointer to node.
/// \param msg  Message for node to interpret.
///
void CellularLayer3Layer(Node *node, Message *msg);
/// Handle timers and layer messages.
///
/// \param node  Pointer to node.
/// \param msg  Message for node to interpret
/// \param interfaceIndex  Interface from which packet was received
/// \param sourceAddress  Node address of the source node
///
void CellularLayer3ReceivePacketOverIp(Node *node,
                                       Message *msg,
                                       int interfaceIndex,
                                       Address sourceAddress);
/// Handle timers and layer messages.
///
/// \param node  Pointer to node.
/// \param msg  Message for node to interpret.
/// \param lastHopAddress  Address of the last hop
/// \param interfaceIndex  Interface from which the packet is received
///
void CellularLayer3ReceivePacketFromMacLayer(Node *node,
                                            Message *msg,
                                            NodeAddress lastHopAddress,
                                            int interfaceIndex);

/// TO see if it is a user devices ranther than fixed nodes
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface from which the packet is received
///
BOOL CellularLayer3IsUserDevices(Node *node, int interfaceIndex);

/// To check if the node is a packet gateway which connects
/// cellular network with packet data network (PDN).
///
/// \param node  Pointer to node.
///
BOOL CellularLayer3IsPacketGateway(Node *node);

/// Check if destination of the packet is in my PLMN
///
/// \param node  Pointer to node.
/// \param msg  Packet to be checked
/// \param inIfIndex  Interface from which the packet is received
/// \param network  network type. IPv4 or IPv6
///
/// \return TRUE if pkt for my PLMN, FALSE otherwise
BOOL CellularLayer3IsPacketForMyPlmn(
        Node* node,
        Message* msg,
        int inIfIndex,
        int networkType);

/// Handle packets from upper layer app or outside PDN
///
/// \param node  Pointer to node.
/// \param msg  Message to be sent onver the air interface
/// \param interfaceIndex  Interface from which the packet is received
/// \param networkType  use IPv4 or IPv6
///
void CellularLayer3HandlePacketFromUpperOrOutside(
           Node* node,
           Message* msg,
           int interfaceIndex,
           int networkType);

#endif /* _CELLULAR_LAYER3_H_ */
