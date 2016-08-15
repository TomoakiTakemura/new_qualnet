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

#ifndef MAC_DOT16_BACKBONE_H
#define MAC_DOT16_BACKBONE_H

#include "mac_dot16.h"
#include "mac_dot16e.h"

// /*
// ENUM        :: Dot16BackbobeMgmtMsgType
// DESCRIPTION :: Type of Management messages sent over backbone
typedef enum
{
    DOT16_BACKBONE_MGMT_BS_HELLO = 0, // hello msg between beighbor BSs
    DOT16_BACKBONE_MGMT_HO_FINISH_NOTIFICATION = 1, // HO finish
                                                    // notification
    //idle/paging
    DOT16e_BACKBONE_MGMT_IM_Entry_State_Change_req = 2,
    DOT16e_BACKBONE_MGMT_IM_Entry_State_Change_rsp = 3,
    DOT16e_BACKBONE_MGMT_IM_Exit_State_Change_req = 4,
    DOT16e_BACKBONE_MGMT_IM_Exit_State_Change_rsp = 5,
    DOT16e_BACKBONE_MGMT_LU_Req = 6,
    DOT16e_BACKBONE_MGMT_LU_Rsp = 7,
    DOT16e_BACKBONE_MGMT_Initiate_Paging_Req = 8,
    DOT16e_BACKBONE_MGMT_Initiate_Paging_Rsp = 9,
    DOT16e_BACKBONE_MGMT_Paging_announce = 10,

    DOT16e_BACKBONE_MGMT_Paging_info = 11,
    DOT16e_BACKBONE_MGMT_Delete_Ms_entry = 12,
    // More
}Dot16BackbobeMgmtMsgType;

/// Message heaer for the message sent over backbone
typedef struct dot16_backbone_mgmt_msg_header
{
    Dot16BackbobeMgmtMsgType msgType;
    // more
}Dot16BackboneMgmtMsgHeader;

/// Structure for Paging info backbone msg
typedef struct dot16e_backbone_paging_info_pdu
{
    Address bsIPAddress;
    NodeAddress bsId;
    UInt16 pagingGroupId;
}Dot16eBackbonePagingInfo;

/// Structure for Location Update backbone msg
typedef struct dot16e_backbone_LU_pdu
{
    NodeAddress BSId;
    unsigned char msMacAddress[DOT16_MAC_ADDRESS_LENGTH];
    UInt16 pagingGroupId;
    UInt16 pagingCycle;
    UInt8 pagingOffset;
}Dot16eLUPdu;

/// Structure for Initiate Paging backbone msg
typedef struct dot16e_backbone_Initiate_paging_pdu
{
    NodeAddress BSId;
    unsigned char msMacAddress[DOT16_MAC_ADDRESS_LENGTH];
}Dot16eInitiatePagingPdu;

/// Structure for Paging announce backbone msg
typedef struct dot16e_backbone_Paging_announce_pdu
{
    UInt16 pagingGroupId;
    UInt16 pagingCycle;
    UInt8 pagingOffset;
    NodeAddress lastBSId;
    unsigned char msMacAddress[DOT16_MAC_ADDRESS_LENGTH];
    BOOL pagingStart;
}Dot16ePagingAnnouncePdu;

/// Structure for IM Exit Req backbone msg
typedef struct dot16e_backbone_IM_Exit_req_pdu
{
    NodeAddress BSId;
    unsigned char msMacAddress[DOT16_MAC_ADDRESS_LENGTH];
}Dot16eIMExitReqPdu;

/// Structure for IM Exit Req backbone msg
typedef struct dot16e_backbone_IM_Entry_req_pdu
{
    Address bsIpAddress;
    Address initialBsIpAddress;
    NodeAddress BSId;
    NodeAddress initialBsId;
    UInt16 pagingGroupId;
    UInt16 pagingCycle;
    // management CIDs assigned
    Dot16CIDType basicCid;                             // basic mgmt CID
    Dot16CIDType primaryCid;                           // primary mgmt CID
    Dot16CIDType secondaryCid;                         // secondary mgmt CID
    short numCidSupport;
    UInt8 pagingOffset;
    UInt8 idleModeRetainInfo;
    unsigned char msMacAddress[DOT16_MAC_ADDRESS_LENGTH];
    // auth and key info
    MacDot16SsAuthKeyInfo ssAuthKeyInfo;
    //basic capability of SS
    MacDot16SsBasicCapability ssBasicCapability;
}Dot16eIMEntryReqPdu;

/// Structure for IM Exit Rsp backbone msg
typedef struct dot16e_backbone_IM_Entry_rsp_pdu
{
    NodeAddress BSID;
}Dot16eIMEntryRspPdu;

/// Structure of the Inter BS Ho finished notification message
typedef struct
{
     // the MAC address of the handovered SS
    unsigned char macAddress[DOT16_MAC_ADDRESS_LENGTH];
}Dot16InterBsHoFinishNotificationMsg;

// Key APIs
/// Send a MAC msg to nbr BS over backbone for management
/// purpose currently only IP is supported
///
/// \param node  Pointer to node
/// \param dot16  Pointer to DOT16 structure
/// \param destNodeAddress  Node address of the neighboring BS
/// \param msg  Message to be sent
/// \param headerInfo  header info
/// \param delay  delay for the transmission of msg
///
/*
void Dot16BsSendMgmtMsgToNbrBsOverBackbone(
         Node* node,
         MacDataDot16* dot16,
         NodeAddress destNodeAddress,
         Message* msg,
         Dot16BackboneMgmtMsgHeader headerInfo,
         clocktype delay);
*/
/// Send a MAC msg to nbr BS over backbone for management
/// purpose currently only IP is supported
///
/// \param node  Pointer to node
/// \param dot16  Pointer to DOT16 structure
/// \param destNodeAddress  Node address of the beighboring BS
/// \param msg  Message to be sent
/// \param headerInfo  header info
/// \param delay  delay for the transmission of msg
///
void Dot16BsSendMgmtMsgToNbrBsOverBackbone(
         Node* node,
         MacDataDot16* dot16,
         Address destNodeAddress,
         Message* msg,
         Dot16BackboneMgmtMsgHeader headerInfo,
         clocktype delay);

/// Handle timers and layer messages.
///
/// \param node  Pointer to node.
/// \param msg  Message for node to interpret
/// \param sourceAddress  Message from node
///
void Dot16BackboneReceivePacketOverIp(Node *node,
                                      Message *msg,
                                      NodeAddress sourceAddress);

/// Handle timers and layer messages. (reloaded for ipv6)
///
/// \param node  Pointer to node.
/// \param msg  Message for node to interpret
/// \param sourceAddress  Message from node
///
void Dot16BackboneReceivePacketOverIp(Node *node,
                                      Message *msg,
                                      Address sourceAddress);

/// Handle Idle Mode System timeout
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to dot16 data structure
/// \param msg  Message for node to interpret
///
void Dot16eBackboneHandleIdleModeSystemTimeout(Node* node,
                                               MacDataDot16* dot16,
                                               Message* msg);

/// Print out statistics
///
/// \param node  Pointer to node
/// \param dot16  Pointer to DOT16 structure
/// \param interfaceIndex  Interface index
///
void Dot16BackbonePrintStats(Node* node,
                             MacDataDot16* dot16,
                             int interfaceIndex);
#endif // DOT16_BACKBONE_H
