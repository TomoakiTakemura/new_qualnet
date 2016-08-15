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

#ifndef MAC_DOT16_SCH_H
#define MAC_DOT16_SCH_H

//
// This is the header file of the implementation of the scheduling functions
// of IEEE 802.16 MAC. In theory, there are 3 schedulers needed. One is for
// outbound transmission scheduling at the BS for downlink, and one is for
// uplink burst scheduling at the BS. And the last is the outbound
// transmission scheduling at the SS.
//

//--------------------------------------------------------------------------
// Default values of various parameters
//--------------------------------------------------------------------------

/// Default outbound transmission scheduler at the BS node
#define DOT16_BS_DEFAULT_OUTBOUND_SCHEDULER    "WEIGHTED-FAIR"

/// Default outbound transmission scheduler at the SS node
#define DOT16_SS_DEFAULT_OUTBOUND_SCHEDULER    "WEIGHTED-FAIR"

/// Default outbound transmission scheduler for mgmt msgs
#define DOT16_DEFAULT_MGMT_SCHEDULER    "STRICT-PRIORITY"

/// Default size of the queue for holding data PDUs of a conn.
#define DOT16_DEFAULT_DATA_QUEUE_SIZE    500000

/// Default size of the queue for holding mgmt PDUs of a conn.
#define DOT16_DEFAULT_MGMT_QUEUE_SIZE    30000

/// Priority of the queue for holding broadcast mgmt messages
/// We give the highest priority to broadcast mgmt messages
#define DOT16_SCH_BCAST_MGMT_QUEUE_PRIORITY    0x7FFFFFFF

/// Priority of the queue for holding mgmt msgs on basic conn.
/// We give the second highest priority to such messages
#define DOT16_SCH_BASIC_MGMT_QUEUE_PRIORITY    0x7FFFFFFE

/// Priority of the queue for mgmt msgs on primary conn.
/// We give the third highest priority to such messages
#define DOT16_SCH_PRIMARY_MGMT_QUEUE_PRIORITY    0x7FFFFFFD

/// Priority of the queue for mgmt msgs on secondary conn.
/// We give the fourth highest priority to such messages
#define DOT16_SCH_SECONDARY_MGMT_QUEUE_PRIORITY    0x7FFFFFFC

/// Weight to the queue for broadcast mgmt messages
#define DOT16_SCH_BCAST_MGMT_QUEUE_WEIGHT    500000.0

/// Weight to the queue for mgmt messages on basic conn.
#define DOT16_SCH_BASIC_MGMT_QUEUE_WEIGHT    400000.0

/// Weight to the queue for mgmt messages on primary conn.
#define DOT16_SCH_PRIMARY_MGMT_QUEUE_WEIGHT    300000.0

/// Weight to the queue for mgmt messages on primary conn.
#define DOT16_SCH_SECONDARY_MGMT_QUEUE_WEIGHT    200000.0

#define DOT16_ARQ_TX_NEXT_BSN (sFlow->arqControlBlock->arqTxNextBsn)
#define DOT16_ARQ_TX_WINDOW_START (sFlow->arqControlBlock->arqTxWindowStart)

#define DOT16_ARQ_WINDOW_REAR (sFlow->arqControlBlock->rear)
#define DOT16_ARQ_WINDOW_FRONT (sFlow->arqControlBlock->front)
#define DOT16_ARQ_WINDOW_SIZE (sFlow->arqParam->arqWindowSize)
#define DOT16_ARQ_ARRAY_SIZE (sFlow->arqParam->arqWindowSize+ 1 )
#define DOT16_ARQ_BLOCK_SIZE (sFlow->arqParam->arqBlockSize)
#define MacDot16ARQSetARQBlockPointer(arrayIndex)\
arqBlockPtr =\
 &(sFlow->arqControlBlock->arqBlockArray[arrayIndex])

#define DOT16_ARQ_FRAG_SUBHEADER &(arqBlockPtr->arqFragSubHeader)

#define MacDot16ARQWindowFreeSpace(freeWindowSpace, front, rear, windowSize)\
                    do { \
                        if (rear >= front )\
                        {\
                          freeWindowSpace = (windowSize - 1) - (rear - front);\
                        } \
                        else { freeWindowSpace = front - rear - 1 ;} \
                      }\
                      while (0)


#define MacDot16ARQCalculateNumBlocks(numARQBlocks, pduSize) \
       if (pduSize <= DOT16_ARQ_BLOCK_SIZE )\
           {\
               numARQBlocks = 1 ;\
           }\
       else\
           {\
                numARQBlocks = (UInt16)(pduSize /DOT16_ARQ_BLOCK_SIZE);\
                if (pduSize % DOT16_ARQ_BLOCK_SIZE)\
                    {\
                        numARQBlocks++;\
                    }\
            }

#define MacDot16ARQIncIndex(index) \
    (index = (index + 1)% (Int16) DOT16_ARQ_WINDOW_SIZE)

#define MacDot16ARQIncBsnId(bsnId) \
    (bsnId = (bsnId + 1)% DOT16_ARQ_BSN_MODULUS)

#define MacDot16ARQDecIndex(index) \
    (index = (index - 1 + DOT16_ARQ_WINDOW_SIZE)% DOT16_ARQ_WINDOW_SIZE)

#define MacDot16ARQDecBsnId(bsnId) \
    (bsnId = (bsnId - 1 + DOT16_ARQ_BSN_MODULUS)% DOT16_ARQ_BSN_MODULUS)



//--------------------------------------------------------------------------
//  API functions
//--------------------------------------------------------------------------

/// Initialize scheduler
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param nodeInput  Pointer to node input.
///
void MacDot16SchInit(Node* node,
                     int interfaceIndex,
                     const NodeInput* nodeInput);

/// Initialize scheduler
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param sFlow  Pointer to the service flow
///
void MacDot16SchAddQueueForServiceFlow(
         Node* node,
         MacDataDot16* dot16,
         MacDot16ServiceFlow* sFlow);

/// BS node will call this function to schedule PDUs to be
/// transmitted in the downlink subframe. The outbound
/// scheduler is triggered to do this.
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param duration  The duration of the DL subframe
///
/// \return duration left unused. If no enough PDUs to
/// fully utlize the dl duration, rest of time
/// can be used for uplink transmissions.
clocktype MacDot16ScheduleDlSubframe(
              Node* node,
              MacDataDot16* dot16,
              clocktype duration);

/// SS node will call this function to schedule PDUs to be
/// transmitted in its uplink burst. The outbound
/// scheduler is triggered to do this.
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param bwGranted  UL BW granted in # of bytes
///
/// \return Number of bytes left unused. There is a possibility
/// that the node has no enough PDUs to fully utilize
/// the whole allocation.
int MacDot16ScheduleUlBurst(Node* node, MacDataDot16* dot16, int bwGranted);

/// BS node will call this function to schedule uplink bursts
/// for uplink transmissions. This is mainly based on the BW
/// allocated to each SS.
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param duration  The duration of the UL subframe
///
void MacDot16ScheduleUlSubframe(
         Node* node,
         MacDataDot16* dot16,
         clocktype duration);

/// Reset the allocation map of the DL sub-frame in order
/// to schedule for a new frame.
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param dlDuration  Duration of the DL-subframe
///
void MacDot16SchResetDlAllocation(Node* node,
                                  MacDataDot16* dot16,
                                  clocktype dlDuration);

/// Search a space in the DL allocation map for specified size
/// in bytes. Allocation is always Physical Slot (PS) based.
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param sizeInBytes  Size in bytes of the requested burst
/// \param diuc  DIUC referring to the burst profile
/// \param msg  For returning info of alloced burst
///
/// \return successful, FALSE, failed
BOOL MacDot16SchAllocDlBurst(Node* node,
                             MacDataDot16* dot16,
                             int sizeInBytes,
                             unsigned char diuc,
                             Dot16BurstInfo* burstInfo);

// FUNCTION   :: MacDot16SchPriorityToCid
// LAYER      :: MAC
// PURPOSE    :: TO convert a priority Value to a CID value.
// PARAMETERS ::
// + int      :: priority:priority Value
// RETURN     :: Dot16CIDType:Dot16CIDType Value to be returned

Dot16CIDType MacDot16SchPriorityToCid(int priority);

//--------------------------------------------------------------------------
//  ARQ related functions
//--------------------------------------------------------------------------
/**
// FUNCTION   :: MacDot16PrintARQParameter
// LAYER      :: MAC
// PURPOSE    :: Print ARQ parameters
// PARAMETERS ::
   + arqParam : MacDot16ARQParam* : Pointer to an ARQ structure.
// RETURN     :: void : NULL
**/
void MacDot16PrintARQParameter(MacDot16ARQParam* arqParam);

/// Initialize ARQ control block
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param sFlow  Pointer to service flow
///
void MacDot16ARQCbInit(Node* node,
                       MacDataDot16*,
                       MacDot16ServiceFlow* sFlow);

Message* MacDot16ARQHandleDataPdu(Node* node,
                              MacDataDot16* dot16,
                              MacDot16ServiceFlow* sFlow,
                              MacDot16BsSsInfo* ssInfo,
                              Message* msg,
                              int pduLength,
                              BOOL isPackingHeaderPresent);

void MacDot16BuildAndSendARQFeedback(Node *node,
                                     MacDataDot16* dot16,
                                     MacDot16ServiceFlow* sFlow,
                                     MacDot16BsSsInfo* ssInfo);

void MacDot16ARQBuildAndSendDiscardMsg(Node *node,
                                     MacDataDot16* dot16,
                                     MacDot16ServiceFlow* sFlow,
                                     MacDot16BsSsInfo* ssInfo,
                                     Dot16CIDType cid,
                                     UInt16 bsnId);


int MacDot16ARQHandleFeedback (Node* node,
                               MacDataDot16* dot16,
                               unsigned char* macFrame,
                               int startIndex);
int MacDot16ARQHandleDiscardMsg(Node* node,
                                MacDataDot16* dot16,
                                unsigned char* macFrame,
                                int startIndex);

BOOL MacDot16ARQCheckIfBSNInWindow(MacDot16ServiceFlow* sFlow,
                                   Int16 incomingBsnId);

void MacDot16ARQPrintControlBlock(Node* node, MacDot16ServiceFlow* sFlow);

void MacDot16ARQBuildSDU(Node* node,
                    MacDataDot16* dot16,
                    MacDot16ServiceFlow* sFlow,
                    MacDot16BsSsInfo* ssInfo);
#endif // MAC_DOT16_SCH_H
