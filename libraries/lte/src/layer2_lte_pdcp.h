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

#ifndef _MAC_LTE_PDCP_H_
#define _MAC_LTE_PDCP_H_

#include <node.h>

//--------------------------------------------------------------------------
// Enumulation
//--------------------------------------------------------------------------
typedef enum {
    PDCP_LTE_POWER_OFF,
    PDCP_LTE_POWER_ON,
    PDCP_LTE_STATUS_NUM
} PdcpLteState;



/// ho status
typedef enum {
    PDCP_LTE_SOURCE_E_NB_IDLE,
    PDCP_LTE_SOURCE_E_NB_WAITING_SN_STATUS_TRANSFER_REQ,
    PDCP_LTE_SOURCE_E_NB_BUFFERING,
    PDCP_LTE_SOURCE_E_NB_FORWARDING,
    PDCP_LTE_SOURCE_E_NB_FORWARDING_END,
    PDCP_LTE_TARGET_E_NB_IDLE,
    PDCP_LTE_TARGET_E_NB_FORWARDING,
    PDCP_LTE_TARGET_E_NB_CONNECTED,
    PDCP_LTE_TARGET_E_NB_WAITING_END_MARKER,
    PDCP_LTE_UE_IDLE,
    PDCP_LTE_UE_BUFFERING
} PdcpLteHoStatus;



/// buffer type for ho
typedef enum {
    PDCP_LTE_REORDERING,
    PDCP_LTE_RETRANSMISSION,
    PDCP_LTE_TRANSMISSION,
} PdcpLteHoBufferType;

//--------------------------------------------------------------------------
// Constant
//--------------------------------------------------------------------------
/// pdcp sequence number limit

#define PDCP_LTE_SEQUENCE_LIMIT (0xFFF)
#define PDCP_LTE_REORDERING_WINDOW (2048) // ((0xFFF) / 2)
#define PDCP_LTE_INVALID_PDCP_SN (PDCP_LTE_SEQUENCE_LIMIT + 1)

//  According to the specifications,
//  select from ms50, ms100, ms150, ms300, ms500, ms750, ms1500
#define PDCP_LTE_DEFAULT_DISCARD_TIMER_DELAY (500 * MILLI_SECOND)
#define PDCP_LTE_DEFAULT_BUFFER_BYTE_SIZE (200000) // 200KB

// parameter strings
#define LTE_PDCP_STRING_DISCARD_TIMER_DELAY "PDCP-LTE-DISCARD-TIMER-DELAY"
#define LTE_PDCP_STRING_BUFFER_BYTE_SIZE "PDCP-LTE-BUFFER-BYTE-SIZE"

// parameter LIMIT
#define LTE_PDCP_MIN_DISCARD_TIMER_DELAY (1*MILLI_SECOND)
#define LTE_PDCP_MAX_DISCARD_TIMER_DELAY (CLOCKTYPE_MAX)
#define LTE_PDCP_MIN_BUFFER_BYTE_SIZE (0)
#define LTE_PDCP_MAX_BUFFER_BYTE_SIZE (INT_MAX)

//--------------------------------------------------------------------------
// Struct
//--------------------------------------------------------------------------
/// PDCP Header
typedef struct
{
    //unsigned char pdcpSN;         // uchar=8bit=XRRRSSSS
                                    // X=DetaOrCtrlFlag
                                    // S=PDCPSN R=RESERVED=0
    //unsigned char pdcpSNCont;
    UInt8 octet[2];
} LtePdcpHeader;



/// for SN status transfer request
typedef struct PdcpLteReceiveStatus {
    // managed only the last submitted PDCP received SN
    // in place of the bitmap
    UInt16 lastSubmittedPdcpRxSn;

    PdcpLteReceiveStatus(
            UInt16 argLastSubmittedPdcpRxSn)
        : lastSubmittedPdcpRxSn(argLastSubmittedPdcpRxSn)
    {
    }

   // get last submitted PDCP rx SN
   UInt16 GetLastSubmittedPdcpRxSn(void) {return lastSubmittedPdcpRxSn;}
} PdcpLteReceiveStatus;



/// for sn status transfer request
typedef struct PdcpLteSnStatusTransferItem {
    PdcpLteReceiveStatus receiveStatus;
    const UInt16 nextPdcpRxSn;
    const UInt16 nextPdcpTxSn;

    PdcpLteSnStatusTransferItem(
            PdcpLteReceiveStatus argReceiveStatus,
            const UInt16 argNextPdcpRxSn,
            const UInt16 argNextPdcpTxSn)
        : receiveStatus(argReceiveStatus),
        nextPdcpRxSn(argNextPdcpRxSn),
        nextPdcpTxSn(argNextPdcpTxSn)
    {
    }

    PdcpLteSnStatusTransferItem()
        : receiveStatus(0),
        nextPdcpRxSn(0),
        nextPdcpTxSn(0)
    {
    }

    struct PdcpLteSnStatusTransferItem& operator=(
        const struct PdcpLteSnStatusTransferItem& /*another*/)
    {
        // For supressing warning C4512
        ERROR_Assert(FALSE,"PdcpLteSnStatusTransferItem::operator= cannot be called");
        return *this;
    }
} PdcpLteSnStatusTransferItem;



/// infomation for discard timer msg
typedef struct PdcpLteDiscardTimerInfo {
    const LteRnti dstRnti;
    const int bearerId;
    const UInt16 sn;

    PdcpLteDiscardTimerInfo(
            const LteRnti argDstRnti,
            const int argBearerId,
            const UInt16 argSn)
        : dstRnti(argDstRnti), bearerId(argBearerId), sn(argSn)
    {
    }

    struct PdcpLteDiscardTimerInfo& operator=(
        const struct PdcpLteDiscardTimerInfo& /*another*/)
    {
        // For supressing warning C4512
        ERROR_Assert(FALSE,"PdcpLteDiscardTimerInfo::operator= cannot be called");
        return *this;
    }
} PdcpLteDiscardTimerInfo;



/// pdcp lte ho manager
typedef struct PdcpLteHoManager
{
    PdcpLteHoStatus hoStatus;

    UInt16 lastSubmittedPdcpRxSn;
    UInt16 nextPdcpRxSn;

    // managed buffer
    list<Message*> reorderingBuffer;
    list<Message*> retransmissionBuffer;
    list<Message*> transmissionBuffer;
    list<Message*> holdingBuffer;

    map<UInt16,Message*> discardTimer;

    // statistics
    UInt16 numEnqueuedRetransmissionMsg;
    UInt32 byteEnqueuedRetransmissionMsg;

    PdcpLteHoManager()
        : // hoStatus(),
            lastSubmittedPdcpRxSn(PDCP_LTE_SEQUENCE_LIMIT),
            nextPdcpRxSn(0),
            numEnqueuedRetransmissionMsg(0),
            byteEnqueuedRetransmissionMsg(0)
    {
    }
} PdcpLteHoManager;



/// PDCP Entity
typedef struct
{
    UInt16 pdcpSN; // Sequence Number

    // For processing delayed PDCP PDU sent to upper layer
    clocktype lastPdcpSduSentToNetworkLayerTime;

   PdcpLteHoManager hoManager;

   // incremented next PDCP tx SN
   void SetNextPdcpTxSn(UInt16 nextPdcpTxSn) {
       ERROR_Assert(nextPdcpTxSn <= PDCP_LTE_SEQUENCE_LIMIT, "failed to SetNextPdcpTxSn()");
       pdcpSN = nextPdcpTxSn;
   }
   // get "next PDCP SN" and set to next value
   UInt16 GetAndSetNextPdcpTxSn(void);
   // get next PDCP tx SN
   UInt16 GetNextPdcpTxSn(void) {return pdcpSN;}
} LtePdcpEntity;

typedef struct
{
    clocktype txTime;
} LtePdcpTxInfo;



/// PDCP statistics
typedef struct
{
    UInt32 numPktsFromUpperLayer;// Number of data packets from Upper Layer
    UInt32 numPktsFromUpperLayerButDiscard; // Number of data packets from
                                            // Upper Layer
    UInt32 numPktsToLowerLayer; // Number of data packets to Lower Layer
    UInt32 numPktsFromLowerLayer; // Number of data packets from Lower Layer
    UInt32 numPktsToUpperLayer; // Number of data packets to Upper Layer
    // Number of data packets enqueued in retransmission buffer
    UInt32 numPktsEnqueueRetranamissionBuffer;
    // Number of data packets discarded
    // due to retransmission buffer overflow
    UInt32 numPktsDiscardDueToRetransmissionBufferOverflow;
    // Number of data packets discarded
    // due to RLC's tx buffer overflow
    UInt32 numPktsDiscardDueToRlcTxBufferOverflow;
    // Number of data packets discarded from retransmission buffer
    // due to discard timer expired
    UInt32 numPktsDiscardDueToDiscardTimerExpired;
    // Number of data packets dequeued from retransmission buffer
    UInt32 numPktsDequeueRetransmissionBuffer;
    // Number of data packets discarded due to ack received
    UInt32 numPktsDiscardDueToAckReceived;
    // Number of data packets enqueued in reordering buffer
    UInt32 numPktsEnqueueReorderingBuffer;
    // Number of data packets discarded due to already received
    UInt32 numPktsDiscardDueToAlreadyReceived;
    // Number of data packets dequeued from reordering buffer
    UInt32 numPktsDequeueReorderingBuffer;
    // Number of data packets discarded from reordering buffer
    // due to invalid PDCP SN received
    UInt32 numPktsDiscardDueToInvalidPdcpSnReceived;
} LtePdcpStats;

/// PDCP sublayer structure
typedef struct struct_lte_pdcp_data
{
    LtePdcpStats stats;

    PdcpLteState pdcpLteState;

    clocktype discardTimerDelay;
    UInt32 bufferByteSize;
} LtePdcpData;

//--------------------------------------------------------------------------
// Function
//--------------------------------------------------------------------------
/// Pdcp Initalization
/// 
///
///    + node:       pointer to the network node
///    + interfaceIndex: interdex of interface
///    + nodeInput:  Input from configuration file
///
void PdcpLteInit(Node* node,
    unsigned int interfaceIndex,
    const NodeInput* nodeInput);

/// Initialize configurable parameters of LTE PDCP Layer.
/// 
///
///    + node:       pointer to the network node
///    + interfaceIndex: interdex of interface
///    + nodeInput:  Input from configuration file
///
void PdcpLteInitConfigurableParameters(Node* node,
    unsigned int interfaceIndex,
    const NodeInput* nodeInput);

/// Pdcp finalization function
///
///    + node:       pointer to the network node
///    + interfaceIndex: interdex of interface
///
void PdcpLteFinalize(Node* node, unsigned int interfaceIndex);

/// Pdcp event handling function
///
///    + node:       pointer to the network node
///    + interfaceIndex: interdex of interface
///    + message     Message to be handled
///
void PdcpLteProcessEvent(Node* node,
    unsigned int interfaceIndex,
    Message* msg);

/// Notify Upper layer has packet to send
///
///    + node           : pointer to the network node
///    + interfaceIndex : index of interface
///    + pdcpData       : pointer of PDCP Protpcol Data
///
void PdcpLteUpperLayerHasPacketToSend(
    Node* node,
    int interfaceIndex,
    LtePdcpData* pdcpData);



/// Notify Upper layer has packet to send
///
///    + node           : pointer to the network node
///    + interfaceIndex : index of interface
///    + srcRnti        : Source RNTI
///    + bearerId       : Radio Bearer ID
///    + pdcpPdu        : PDCP PDU
///
void PdcpLteReceivePduFromRlc(
    Node* node,
    int interfaceIndex,
    const LteRnti& srcRnti,
    const int bearerId,
    Message* pdcpPdu,
    BOOL isReEstablishment);



/// Power ON
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
///
void PdcpLteNotifyPowerOn(Node* node, int interfaceIndex);

/// Power OFF
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
///
void PdcpLteNotifyPowerOff(Node* node, int interfaceIndex);

/// Set state
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param state  State
///
void PdcpLteSetState(Node* node, int interfaceIndex, PdcpLteState state);

/// Init PDCP Entity
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param rnti  RNTI
/// \param pdcpEntiry  PDCP Entity
/// \param isTargetEnb  whether target eNB or not
///
void PdcpLteInitPdcpEntity(
    Node* node, int interfaceIndex,
    const LteRnti& rnti, LtePdcpEntity* pdcpEntity, BOOL isTargetEnb);



/// Finalize PDCP Entity
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param rnti  RNTI
/// \param pdcpEntiry  PDCP Entity
///
void PdcpLteFinalizePdcpEntity(
    Node* node, int interfaceIndex,
    const LteRnti& rnti, LtePdcpEntity* pdcpEntiry);



/// event3
/// cancel such discard timer
/// and dequeue retransmission buffer head PDCP PDU
// // PARAMETERS ::
/// + node               : Node*          : Pointer to node.
/// + interfaceIndex     : const int      : Interface index
/// + srcRnti            : const LteRnti& : source RNTI
/// + bearerId           : const int      : Radio Bearer ID
/// + rxAckSn            : const UInt16   : SN of received msg
void PdcpLteReceivePdcpStatusReportFromRlc(
    Node* node,
    const int interfaceIndex,
    const LteRnti& srcRnti,
    const int bearerId,
    const UInt16 rxAckSn);



/// event4
/// return sn status transfer item
// // PARAMETERS ::
/// + node               : Node*          : Pointer to node.
/// + interfaceIndex     : const int      : Interface index
/// + rnti               : const LteRnti& : RNTI
/// + bearerId           : const int      : Radio Bearer ID
/// \return sn status transfer item
PdcpLteSnStatusTransferItem PdcpLteGetSnStatusTransferItem(
    Node* node,
    const int interfaceIndex,
    const LteRnti& rnti,
    const int bearerId);



/// event5
/// set arg to own sn status transfer item
// // PARAMETERS ::
/// + node               : Node*          : Pointer to node.
/// + interfaceIndex     : const int      : Interface index
/// + rnti               : const LteRnti& : RNTI
/// + bearerId           : const int      : Radio Bearer ID
/// + item : PdcpLteSnStatusTransferItem& : set sn status transfer item
void PdcpLteSetSnStatusTransferItem(
    Node* node,
    const int interfaceIndex,
    const LteRnti& rnti,
    const int bearerId,
    PdcpLteSnStatusTransferItem& item);



/// event6
/// make list of msgs in Buffer
/// and call function for forwarding
// // PARAMETERS ::
/// + node               : Node*          : Pointer to node.
/// + interfaceIndex     : const int      : Interface index
/// + rnti               : const LteRnti& : RNTI
/// + bearerId           : const int      : Radio Bearer ID
void PdcpLteForwardBuffer(
        Node* node,
        const int interfaceIndex,
        const LteRnti& rnti,
        const int bearerId);



/// event7
/// enqueue forwarded msg in such buffer
// // PARAMETERS ::
/// + node               : Node*          : Pointer to node.
/// + interfaceIndex     : const int      : Interface index
/// + rnti               : const LteRnti& : RNTI
/// + bearerId           : const int      : Radio Bearer ID
/// + forwardedMsg       : Message*       : forwarded msg from source eNB
void PdcpLteReceiveBuffer(
        Node* node,
        const int interfaceIndex,
        const LteRnti& rnti,
        const int bearerId,
        Message* forwardedMsg);



/// event8
/// dequeue msg from buffer and send upper/lower layer
// // PARAMETERS ::
/// + node               : Node*          : Pointer to node.
/// + interfaceIndex     : const int      : Interface index
/// + rnti               : const LteRnti& : RNTI
/// + bearerId           : const int      : Radio Bearer ID
void PdcpLteNotifyRrcConnected(
        Node* node,
        const int interfaceIndex,
        const LteRnti& rnti,
        const int bearerId);



/// event9
/// target eNB or UE dequeue msg from buffer
/// and send upper/lower layer
/// or source eNB is to be finalize
// // PARAMETERS ::
/// + node               : Node*          : Pointer to node.
/// + interfaceIndex     : const int      : Interface index
/// + rnti               : const LteRnti& : RNTI
/// + bearerId           : const int      : Radio Bearer ID
void PdcpLteNotifyEndMarker(
        Node* node,
        const int interfaceIndex,
        const LteRnti& rnti,
        const int bearerId);



/// event11
/// discard msg in retransmission buffer
/// and send msg in reordering buffer to upper layer
// // PARAMETERS ::
/// + node               : Node*          : Pointer to node.
/// + interfaceIndex     : const int      : Interface index
/// + rRnti              : const LteRnti& : RNTI
/// + bearerId           : const int      : Radio Bearer ID
void PdcpLteNotifyRlcReset(
        Node* node,
        const int interfaceIndex,
        const LteRnti& rnti,
        const int bearerId);



/// event12
/// begin buffering received msg
// // PARAMETERS ::
/// + node               : Node*          : Pointer to node.
/// + interfaceIndex     : const int      : Interface index
/// + rnti               : const LteRnti& : RNTI
/// + bearerId           : const int      : Radio Bearer ID
void PdcpLteReEstablishment (
    Node* node,
    const int interfaceIndex,
    const LteRnti& rnti,
    const int bearerId);
#endif // _MAC_LTE_PDCP_H_

