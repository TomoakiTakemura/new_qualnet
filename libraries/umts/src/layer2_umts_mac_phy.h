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

#ifndef _LAYER2_UMTS_MAC_PHY_H
#define _LAYER2_UMTS_MAC_PHY_H

#include <utility>
#include <deque>
#include <list>
#include <deque>
#include <vector>
#include <set>

#include "cellular.h"
#include "cellular_umts.h"

// In this file, it will implement the functions related
// CCrCh, Spreading, Scambling, combining and 
// others not related to RX and RX
// as well as Ch start/stop listening

// CONSTANT    :: UMTS_DELAY_UNTIL_SIGNAL_AIRBORN
// DESCRIPTION :: A delay to support look ahead in parallel
#define UMTS_DELAY_UNTIL_SIGNAL_AIRBORN 100 * NANO_SECOND

/// Start listening to a channel
///
/// \param node  Pointer to node
/// \param phyNumber  PHY number
/// \param channelIndex  channel to listen to
///
void UmtsMacPhyStartListeningToChannel(Node* node,
                                       int phyNumber,
                                       int channelIndex);

/// Stop listening to a channel
///
/// \param node  Pointer to node
/// \param phyNumber  PHY number of the interface
/// \param channelIndex  channel to stop listening to
///
void UmtsMacPhyStopListeningToChannel(Node* node,
                                      int phyNumber,
                                      int channelIndex);

/// Receive a packet from physical layer.
/// The PHY sublayer will first handle it
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param msg  Packet received from PHY
///
void UmtsMacReceivePacketFromPhy(Node* node,
                                 int interfaceIndex,
                                 Message* msg);

/// Handle timers out messages.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface Index
/// \param mac  Mac Data
/// \param msg  msg List to send to phy
///
void UmtsMacUeTransmitMacBurst(Node* node,
                               UInt32 interfaceIndex,
                               UmtsMacData* mac,
                               Message* msg);

/// Handle timers out messages.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface Index
/// \param mac  Mac Data
/// \param msg  msg List to send to phy
///
void UmtsMacNodeBTransmitMacBurst(Node* node,
                                  UInt32 interfaceIndex,
                                  UmtsMacData* mac,
                                  Message* msg);

// FUNCITON   :: UmtsMacPhyHandleInterLayerCommand
// LAYER      :: UMTS L2 MAC
// PURPOSE    :: Handle Interlayer command CPHY- 
// PARAMETERS :: 
// + node             : Node*             : Pointer to node.
// + interfaceIndex   : UInt32            : interface Index
// + cmdType          : UmtsInterlayerCommandType : command type
// + cmd              : void*          : cmd to handle
// RETURN     :: void : NULL
void UmtsMacPhyHandleInterLayerCommand(
            Node* node,
            UInt32 interfaceIndex,
            UmtsInterlayerCommandType cmdType,
            void* cmd);

// FUNCITON   :: UmtsMacPhyMappingTrChPhCh
// LAYER      :: UMTS L2 MAC
// PURPOSE    :: Handle Interlayer command CPHY- 
// PARAMETERS :: 
// + node             : Node*             : Pointer to node.
// + interfaceIndex   : UInt32            : interface Index
// + mappingInfo      : UmtsTrCh2PhChMappingInfo* : Mapping Info
// RETURN     :: void : NULL
void UmtsMacPhyMappingTrChPhCh(
            Node* node,
            UInt32 interfaceIndex,
            UmtsTrCh2PhChMappingInfo* mappingInfo);

// FUNCITON   :: UmtsMacPhyConfigRcvPhCh
// LAYER      :: UMTS L2 MAC
// PURPOSE    :: Config receiving side DPDCH information 
//               (channelization code)
// PARAMETERS :: 
// + node             : Node*                   : Pointer to node.
// + interfaceIndex   : UInt32                  : interface Index
// + priSc            : UInt32                  : primary scrambling code
// + phChInfo         : const UmtsRcvPhChInfo*  : receiving physical 
//                                                channel information
// RETURN     :: void : NULL
void UmtsMacPhyConfigRcvPhCh(
        Node* node,
        UInt32 interfaceIndex,
        UInt32 priSc,
        const UmtsRcvPhChInfo* phChInfo);

//  /** 
// FUNCITON   :: UmtsMacPhyReleaseRcvPhCh
// LAYER      :: UMTS L2 MAC
// PURPOSE    :: Release receiving side DPDCH information 
//               (channelization code)
// PARAMETERS :: 
// + node             : Node*                   : Pointer to node.
// + interfaceIndex   : UInt32                  : interface Index
// + priSc            : UInt32                  : primary scrambling code
// + phChInfo         : const UmtsRcvPhChInfo*  : receiving physical 
//                                                channel information
// RETURN     :: void : NULL
void UmtsMacPhyReleaseRcvPhCh(
    Node* node,
    UInt32 interfaceIndex,
    UInt32 priSc,
    const UmtsRcvPhChInfo* phChInfo = NULL);

/// enqueue packet to PhCh txBuffer
///
///    node        : Node*                     : Pointer to the node
///    interfaceIndex : UInt32                 : interface Index
///    chType      : UmtsPhysicalChannelType   : physical channel type
///    chId        : unsigned int              : channel Idetifier
///    pduList     : std::list<Message*>&      : pointer to msg List
///    to be enqueued
///
void UmtsMacPhyNodeBEnqueuePacketToPhChTxBuffer(
                     Node* node,
                     UInt32 interfaceIndex,
                     UmtsPhysicalChannelType chType,
                     unsigned int chId,
                     std::list<Message*>& pduList);

/// enqueue packet to PhCh txBuffer
///
///    node        : Node*                     : Pointer to the node
///    interfaceIndex : UInt32                 : interface Index
///    chType      : UmtsPhysicalChannelType   : physical channel type
///    chId        : unsigned int              : channel Idetifier
///    pduList     : std::list<Message*>&      : pointer to msg List
///    to be enqueued
///
void UmtsMacPhyUeEnqueuePacketToPhChTxBuffer(
                     Node* node,
                     UInt32 interfaceIndex,
                     UmtsPhysicalChannelType chType,
                     unsigned int chId,
                     std::list<Message*>& pduList);

/// return the PCCPCH SFN at nodeB
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
/// \param priSc  primary sc code of the nodeB
///
/// \return PCCPCH SFN index of the nodeB
unsigned int UmtsMacPhyGetPccpchSfn(Node*node, 
                                     int interfaceIndex,
                                     unsigned int priSc = 0);

/// return the active ul channel index 
/// of the interface at UE/NodeB
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
/// \param pccpchSfn  PCCPCH FSN
/// \param priSc  primary sc code of the nodeB
///
void UmtsMacPhySetPccpchSfn(Node*node, 
                             int interfaceIndex, 
                             unsigned int pccpchSfn,
                             unsigned int priSc = 0);

/// return the active ul channel index 
/// of the interface at UE/NodeB
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
/// \param ueId  UE Node Id
/// \param priSc  primary sc code of the nodeB
/// \param selfPrimCell  Is it a priCell for UE
///
void UmtsMacPhyNodeBAddUeInfo(Node*node, 
                              int interfaceIndex, 
                              NodeAddress ueId,
                              unsigned int priSc,
                              BOOL selfPrimCell);

/// return the active ul channel index 
/// of the interface at UE/NodeB
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
/// \param ueId  UE Node Id
/// \param priSc  primary sc code of the nodeB
///
void UmtsMacPhyNodeBRemoveUeInfo(Node*node, 
                                 int interfaceIndex, 
                                 NodeAddress ueId,
                                 unsigned int priSc);

/// return the active ul channel index 
/// of the interface at UE/NodeB
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
///
void UmtsMacPhyUeCheckUlPowerControlAdjustment(Node* node,
                                               UInt32 interfaceIndex);

/// Enable self as the prim cell for this UE
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
/// \param priSc  Ue Primary Scrambling code
///
void UmtsMacPhyNodeBEnableSelfPrimCell(
            Node*node, 
            int interfaceIndex, 
            unsigned int priSc);

/// Enable self as the prim cell for this UE
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
/// \param priSc  Ue Primary Scrambling code
///
void UmtsMacPhyNodeBDisableSelfPrimCell(
            Node*node, 
            int interfaceIndex, 
            unsigned int priSc);

/// release all  dedicated physical channel
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
///
void UmtsMacPhyUeRelAllDedctPhCh(Node* node,
                                 UInt32 interfaceIndex);

/// release all  dedicated Dedicated Transport channels
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
///
void UmtsMacPhyUeRelAllDch(Node* node,
                           UInt32 interfaceIndex);

/// Set the number of data bit in BUffer
///
///    node        : Node*                     : Pointer to the node
///    interfaceIndex : UInt32                 : interface Index
///    numDataBitInBuffer : int                : number of data bit in buffer       
///
void UmtsMacPhyUeSetDataBitSizeInBuffer(
                     Node* node,
                     UInt32 interfaceIndex,
                     int numDataBitInBuffer);

/// Get the number of data bit in BUffer
///
///    node        : Node*                     : Pointer to the node
///    interfaceIndex : UInt32                 : interface Index
///
/// \return numDataBitInBuffer
int UmtsMacPhyUeGetDataBitSizeInBuffer(
                     Node* node,
                     UInt32 interfaceIndex);

/// Get the number of DPDCH channels
///
///    node        : Node*                     : Pointer to the node
///    interfaceIndex : UInt32                 : interface Index
///
/// \return number of dedicated DPDCH
unsigned char UmtsMacPhyUeGetDpdchNumber(
                     Node* node,
                     UInt32 interfaceIndex);

/// Get the number of data bit in BUffer
///
///    node        : Node*                     : Pointer to the node
///    interfaceIndex : UInt32                 : interface Index
///    numUlDpdch  : unsigned char             : umber of dedicated DPDCH       
///
void UmtsMacPhyUeSetDpdchNumber(
                     Node* node,
                     UInt32 interfaceIndex,
                     unsigned char numUlDpdch);

/// Set the hsdpa Cqi report request indicator
///
///    node        : Node*                     : Pointer to the node
///    interfaceIndex : UInt32                 : interface Index
///
void UmtsMacPhyUeSetHsdpaCqiReportInd(
                     Node* node,
                     UInt32 interfaceIndex); 

/// Set the hsdpa configuration
///
///    node        : Node*                     : Pointer to the node
///    interfaceIndex : UInt32                 : interface Index
///    numHspdsch  : unsigned char             : numer of HS-PDSCH
///    hspdschId   : unsigned int*             : channel Id
///    numHsscch   : unsigned char             : number of HS-SCCH
///    hsscchId    : unsigned int*             : channel Id
///
void UmtsMacPhyNodeBSetHsdpaConfig(
                     Node* node,
                     UInt32 interfaceIndex,
                     unsigned char numHspdsch,
                     unsigned int* hspdschId,
                     unsigned char numHsscch,
                     unsigned int* hsscchId);

/// Get the hsdpa configuration
///
///    node        : Node*                     : Pointer to the node
///    interfaceIndex : UInt32                 : interface Index
///    numHspdsch  : unsigned char*             : numer of HS-PDSCH
///    hspdschId   : unsigned int*             : channel Id
///    numHsscch   : unsigned char*             : number of HS-SCCH
///    hsscchId    : unsigned int*             : channel Id
///
void UmtsMacPhyNodeBGetHsdpaConfig(
                     Node* node,
                     UInt32 interfaceIndex,
                     unsigned char* numHspdsch,
                     unsigned int* hspdschId,
                     unsigned char* numHsscch,
                     unsigned int* hsscchId);

/// return the active HSDPA channels
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
/// \param numActiveCh  Number of active HSDPA Channels
///
void UmtsMacPhyNodeBGetActiveHsdpaChNum(Node*node,
                                        UInt32 interfaceIndex,
                                        unsigned int* numActiveCh);

/// set the active HSDPA channels
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
/// \param numActiveCh  Number of active HSDPA Channels
///
void UmtsMacPhyNodeBSetActiveHsdpaChNum(Node*node,
                                        UInt32 interfaceIndex,
                                        unsigned int numActiveCh);

/// return the CQI value at UE/NodeB
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
/// \param priSc  primry sc code
/// \param cqiVal  cqivl reproted from UE
///
void UmtsMacPhyNodeBGetUeCqiInfo(Node*node,
                                 UInt32 interfaceIndex,
                                 unsigned int priSc,
                                 unsigned char* cqiVal);

/// Update Physical channel info
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
/// \param trChType  physical channel type
/// \param chId  channel Identifier
/// \param upInfo  updated info
///
BOOL UmtsMacPhyNodeBUpdatePhChInfo(
                     Node* node,
                     UInt32 interfaceIndex,
                     UmtsPhysicalChannelType chType,
                     unsigned int chId,
                     UmtsPhChUpdateInfo* upInfo);

/// return the CQI value at UE/NodeB
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
///    + codingType : UmtsCodingType  : Coding type 
/// \param moduType  Modulation type
/// \param sinr  sinr
///
/// \return BER
double UmtsMacPhyGetSignalBitErrorRate(
                     Node* node,
                     UInt32 interfaceIndex,
                     UmtsCodingType codingType,
                     UmtsModulationType moduType,
                     double sinr);
#endif //_LAYER2_UMTS_MAC_PHY_H



