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

/// \defgroup Package_Logical_Link_Control_header Logical Link Control header

/// \file
/// \ingroup Package_Logical_Link_Control_header
/// Data structures and parameters used in mac layer
/// are defined here.

#ifndef MAC_LLC_H
#define MAC_LLC_H

/// Size of ethernet frame header.
#define LLC_HEADER_SIZE              8

/// Destination Service Access Point
#define LLC_DSAP                     0xAA 

/// Source Service Access Point
#define LLC_SSAP                     0xAA

/// Control value
#define LLC_CONTROL_VALUE            0x03 

/// Organization Code
#define LLC_PROTOCOL_ID              0x000000

/// Ether Type Field value for protocol MPLS TYPE
#define PROTOCOL_TYPE_MPLS                   0x8847

/// Struture of the Service access point
typedef struct llc_lsap
{
   unsigned char dsap;      // dest service access point, 8 bits
   unsigned char ssap;      // source service access point, 8 bits
   unsigned char ctrl;      // Control, 8 bits
}LlcLsapHeader;


/// Struture of LLC header
struct LlcHeader
{
    LlcLsapHeader lsap;

    unsigned char orgCode[3];
    //unsigned short etherType; //whether IP, ARP, RARP
    UInt16 etherType; //whether IP, ARP, RARP
};


//LAYER         :: MAC
// PURPOSE      :: Add the LLC header with incomping packet to MAC
/// NOTES- The type 1 process of LLC header implemented.
/// The details of LLC is out of scope in this code
///
/// \param node  Pointer to node.
/// \param msg  Incoming message to MAC
/// \param type  Whether ARP packet or IP packet
///

void LlcAddHeader(
         Node* node, 
         Message* msg, 
         UInt16 type);

/// Remove the LLC header of outgoing packet from MAC.
/// NOTES- The type 1 process of LLC header implemented.
/// The details of LLC is out of scope in this code
///
/// \param node  Pointer to node which received the message.
/// \param interfaceIndex  outgoing interface.
/// \param msg : Message*a  outgoing message of MAC to IP or ARP
///
void LlcRemoveHeader(
         Node* node, 
         Message* msg);

/// To print the Header information of LLC for debug
/// NOTES-  The type 1 process of LLC header implemented.
/// The details of LLC is out of scope in this code
///
///    + msg  : Message* : outgoing or incoming message of MAC of IP or 
///    ARP vice-versa
///
void LlcHeaderPrint(Message* msg);

/// Check the whether LLC is enabled or not
///
/// \param node  Pointer to node.
/// \param interface  The interface on which the LLC works
///
/// \return TRUE if LLC found otherwise FALSE
BOOL LlcIsEnabled(Node* node, int interfaceindex);


    
/// Remove the LLC header with outgoing packet from MAC
///
/// \param node  Pointer to node.
///    + interfaceIndex : int : interface of the node 
/// \param msg  Incoming message to MAC
/// \param lastHopAddress  last hop address
///

void
MAC_LLCHandOffSuccessfullyReceivedPacket(
    Node *node,
    int interfaceIndex,
    Message *msg,
    NodeAddress lastHopAddress);


#endif //MAC_LLC_H
