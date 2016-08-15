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

/// \file mac_dot11_ac_manager.cpp
///
/// \brief Dot11 Access Category Class hierarchy Implementation
///
/// This file contains the class hierarchy of access categories
/// and its implementation


#include "mac_dot11_ac_manager.h"
#include "mac_dot11-sta.h"
#include "message.h"
#include "mac_dot11n.h"
using namespace Dot11;
using namespace Qos;


/// \brief Implementation of start contention timer Function
///
/// This function will start the AC contention timer
///
/// \param delay contention timer delay
/// \param acIndex index of the access category
Message* AcManager::
             startContentionTimer(MacDataDot11* dot11, clocktype delay, UInt8 acIndex)
{
    Message *newMsg;
    MacDot11StationCancelTimer(node(),dot11);
    newMsg = MESSAGE_Alloc(node(),
                           MAC_LAYER, MAC_PROTOCOL_DOT11,
                           MSG_MAC_TimerExpired);
    MESSAGE_SetInstanceId(newMsg, (short) ifidx());

    // Add default info
    MESSAGE_InfoAlloc(node(), newMsg, sizeof(dot11->timerSequenceNumber));
    *((int*)(MESSAGE_ReturnInfo(newMsg))) = dot11->timerSequenceNumber;

    // Add AC Index in the info
    UInt8* ac = (UInt8*)MESSAGE_AddInfo(node(),
                                        newMsg,
                                        sizeof(UInt8),
                                        INFO_TYPE_DOT11_AC_INDEX);
    *ac = acIndex;

    newMsg->originatingNodeId = node()->nodeId;

    MESSAGE_Send(node(), newMsg, delay);
    return newMsg;
}

/// \brief Implementation of pause access category functionality
///
/// This function will pause the access categories and save the remaining
/// time in the BO of each Access category
///
/// \test Checks if the Access category is in Contending state
/// then put the remaining time in Bo End Time parameter.
void AcManager::pauseAc(MacDataDot11* dot11)
{
    for (int i = 0; i < DOT11e_NUMBER_OF_AC; i++)
    {
        if (ac[i].getState() != k_State_IDLE)
        {
           // Calculate time according to difs + bo end time
           clocktype now = node()->getNodeTime();
           clocktype remainingTime = ac[i].getBoEndTime() - now;
           if ((now >= ac[i].getBoEndTime()) ||
               (remainingTime > ac[i].getBackoffInterval()))
           {
               // timer has expired, or AC is in DIFS, set state Idle
               ac[i].setState(k_State_IDLE);
               ac[i].setBoEndTime(0);
           }
           else if (remainingTime < ac[i].getBackoffInterval())
           {
               // Ac is in BO, save remaining time
               ac[i].setState(k_State_IDLE);
               ac[i].setBackoffInterval(remainingTime);
               ac[i].setBoEndTime(0);
           }
        }
    }
}


/// \brief Check if the state of Access Category is DIFS or BO
///
/// This function will Check if the state of Access Category is DIFS or BO
///
/// \param dot11 dot11 structure pointer
BackoffState AcManager::CheckStateIfDifsOrBo(MacDataDot11* dot11)
{
    UInt8 acIndex = 0;
    if (m_mode != k_Mode_Legacy)
    {
        acIndex = getMinDifsBoIntervalAc(dot11);
    }

    // Calculate time according to difs + bo end time
    clocktype now = node()->getNodeTime();
    clocktype remainingTime = ac[acIndex].getBoEndTime() - now;
    if ((now < ac[acIndex].getBoEndTime()) &&
            (remainingTime > ac[acIndex].getBackoffInterval()))
    {
        return k_State_DIFS;
    }
    else
    {
        return k_State_BO;
    }
}

/// \brief Implementation of pause access category functionality
///
/// This function will pause the access category and save the remaining
/// time in the BO of legacy access category
///
/// \test Checks if the Access category 0 is in Contending state
/// then put the remaining time in Bo End Time parameter.
void LegacyAcManager::pauseAc(MacDataDot11* dot11)
{
    if (ac[0].getState() != k_State_IDLE)
    {
       // Calculate time according to difs + bo end time
       clocktype now = node()->getNodeTime();
       clocktype remainingTime = ac[0].getBoEndTime() - now;
       if ((now >= ac[0].getBoEndTime()) ||
           (remainingTime > ac[0].getBackoffInterval()))
       {
           // timer has expired, or AC is in DIFS, set state Idle
           ac[0].setState(k_State_IDLE);
           ac[0].setBoEndTime(0);
       }
       else if (remainingTime < ac[0].getBackoffInterval())
       {
           // Ac is in BO, save remaining time
           ac[0].setState(k_State_IDLE);
           ac[0].setBackoffInterval(remainingTime);
           dot11->BO = remainingTime;
           ac[0].setBoEndTime(0);
       }
    }
}


/// \brief Implementation of start DIFS duration
///
/// This function will set the state k_State_Contention
/// of the access category and set the DIFS + BO duration
/// in m_difs_bo_endTime
///
/// \param endTime End time of the overall contention and back off duration
void AccessCategory::startDifs(clocktype endTime)
{
    setState(k_State_Contention);
    m_difs_bo_endTime = endTime;
}
/*** UAPSD *********************************START*******************/

/// \brief Implementation of canThisACContend function
///
/// This function checks if a particular access category
/// is allowed to content
///
/// \param dot11 Pointer to MacDot11 data strucure
/// \param acIndex Access categorty
bool AcManager::canThisACContend(
    MacDataDot11* dot11,
    int acIndex)
{
    if (!ac[acIndex].isAcHasPacket() && !ac[acIndex].m_frameInfo)
    {
        return false;
    }

    if (!MacDot11IsAp(dot11)
        || (MacDot11IsAp(dot11) && !dot11->isUapsdEnable))
    {
        if (ac[acIndex].isAcHasPacket() || ac[acIndex].m_frameInfo)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    if (ac[acIndex].m_frameInfo)
    {
        if (ac[acIndex].m_frameInfo->frameType >= DOT11_ASSOC_REQ
             && ac[acIndex].m_frameInfo->frameType <= DOT11_DEAUTHENTICATION
             || ac[acIndex].m_frameInfo->frameType == DOT11_QOS_NULL)
        {
            return true;
        }
    }

    bool canContend = FALSE;
    Mac802Address staAddr;
    std::map<std::pair<Mac802Address,TosType>,MapValue>::iterator keyItr;
    keyItr = ac[acIndex].getBuffer()->OutputBufferMap.begin();
    while (keyItr != ac[acIndex].getBuffer()->OutputBufferMap.end())
    {
        staAddr = keyItr->first.first;
        if (keyItr->second.numPackets > 0)
        {
            if (MAC_IsBroadcastMac802Address(&staAddr))
            {
                if (dot11->apVars->isBroadcastPacketTxContinue)
                {
                    canContend = TRUE;
                    break;
                }
            }
            else
            {
                DOT11_ApStationListItem* stationItem = NULL;
                stationItem =
                    MacDot11ApStationListGetItemWithGivenAddress(
                                                m_node,
                                                dot11,
                                                staAddr);
                if (!stationItem
                    || !stationItem->data->uapsdInfo.uApsdEnabled
                    || (stationItem->data->uapsdInfo.uApsdEnabled
                        && stationItem->data->uapsdInfo.spInProgress))
                {
                    canContend = TRUE;
                    break;
                }
            }
        }
        keyItr++;
    }
    return canContend;
}
/*** UAPSD *********************************END*******************/

/// \brief Implementation to find minimum difs and bo interval
///
/// This function will find the index of the access category
/// whose DIFS + BO duration is the minimum among all the Ac's
///
/// \test Checks if the message to be sent is an instant frame
/// then set isBeacon true and finds the minimum Difs + BO interval AcIndex
UInt8 AcManager::getMinDifsBoIntervalAc(MacDataDot11* dot11)
{
    Int8 acIndex = -1;
    clocktype tmpInterval = CLOCKTYPE_MAX;
    BOOL isBeacon = FALSE;
    if (MacDot11StationHasInstantFrameToSend(dot11)
        || dot11->beaconSet)
    {
        isBeacon = TRUE;
    }

    for (int i = 0; i < 4; i++)
    {
        if (((ac[i].getAifsInterval() + ac[i].getBackoffInterval()) < tmpInterval) &&
            ((canThisACContend(dot11, i)) || (isBeacon && i == 3)))
        {
            tmpInterval = ac[i].getAifsInterval() + ac[i].getBackoffInterval();
            acIndex = i;
        }
    }

/*** UAPSD *********************************START*******************/
    if (!dot11->isUapsdEnable)
    {
        ERROR_Assert(acIndex != -1, "Invalid Ac Index");
    }
/*** UAPSD *********************************END*******************/
    return acIndex;
}

/// \brief Implementation of start contention
///
/// This function will set the backoff is zero  and then start the
/// difs + bo duration timer
void LegacyAcManager::startContention(MacDataDot11* dot11)
{
     if (ac[0].m_frameInfo != NULL
         || (dot11->currentMessage)
         || (dot11->instantMessage))
     {
         // Check if BO is zero, then set
         SetBackoffIfZero(node(), dot11, 0);

         clocktype extradelay = 0;
         clocktype timerdelay = 0;

         CalculateExtraDelay(dot11, extradelay);
         timerdelay = extradelay + dot11->txDifs + ac[0].getBackoffInterval();

         // Start the DIFS + BO timer for the ac
         m_timerMsg = startContentionTimer(dot11, timerdelay, 0);

         ac[0].startDifs(node()->getNodeTime() +
                         timerdelay);
         MacDot11StationSetState(node(), dot11, DOT11_CONTENDING);
     }
}

/// \brief Implementation of Interupt contention function for legacy Ac manager
///
/// This function will pause the Ac's if the timer is already started
/// and the timer message is cancelled
///
/// \test check if m_timerMsg exist, then pause all the Ac's and cancel
/// the timer message.
void LegacyAcManager::interruptContention(MacDataDot11* dot11)
{
    if (m_timerMsg)
    {
        pauseAc(dot11);
        MESSAGE_CancelSelfMsg(node(), m_timerMsg);
        m_timerMsg = NULL;
    }
}


/// \brief Implementation of Calculating Extra Delay for current Ac Index
///
/// This function calculate the extra delay for current Ac Index and
/// gives the total delay as out parameter
void AcManager::CalculateExtraDelay(MacDataDot11* dot11,
                                    clocktype& totaldelay)
{
    clocktype extraWait = 0;
    if (node()->getNodeTime() < dot11->noOutgoingPacketsUntilTime)
    {
        extraWait = dot11->noOutgoingPacketsUntilTime - node()->getNodeTime();
    }

    if (dot11->IsInExtendedIfsMode)
    {
//---------------------DOT11e--Updates------------------------------------//
        // for DOT11e set with AIFS...
        if (MacDot11IsQoSEnabled(node(), dot11))
        {
            if (dot11->currentACIndex != DOT11e_INVALID_AC)
            {
/*********HT START*************************************************/
                clocktype extendedIfsDelay = 0;
                if (dot11->isHTEnable)
                {
                        MAC_PHY_TxRxVector tempTxRxVector
                            =dot11->txVectorForLowestDataRate;
                        tempTxRxVector.length =
                            DOT11_SHORT_CTRL_FRAME_SIZE;
                        extendedIfsDelay = dot11->sifs +
                                PHY_GetTransmissionDuration(
                                    node(), dot11->myMacData->phyNumber,
                                    tempTxRxVector);
                }
                else
                {
                    extendedIfsDelay = dot11->sifs +
                    PHY_GetTransmissionDuration(
                        node(), dot11->myMacData->phyNumber,
                        MAC_PHY_LOWEST_MANDATORY_RATE_TYPE,
                        DOT11_SHORT_CTRL_FRAME_SIZE);
                }
/*********HT END****************************************************/
                totaldelay = extraWait + extendedIfsDelay;
//--------------------DOT11e-End-Updates---------------------------------//
            }
            else
            {
                // for dot11e if access category is not used, say for
                // management frames resume usual operation;
                totaldelay = extraWait + dot11->extendedIfsDelay;
            }
        }
        else
        {
            totaldelay = extraWait + dot11->extendedIfsDelay;
        }
        mc(dot11)->updateStats("IFS", "EIFS");
    }
    else
    {
//---------------------DOT11e--Updates------------------------------------//
        // for DOT11e set with AIFS...
        if (MacDot11IsQoSEnabled(node(), dot11))
        {
            if (dot11->currentACIndex != DOT11e_INVALID_AC)
            {
                if (dot11->isHTEnable)
                {
                    clocktype timerDelay = 0;
                    if (MacDot11StationHasInstantFrameToSend(dot11)
                        || dot11->beaconSet)
                    {
                        timerDelay = extraWait + dot11->txDifs;
                    }
                    else
                    {
                        timerDelay = extraWait
                            - dot11->delayUntilSignalAirborn;
                    }

                    totaldelay = timerDelay;
                }
                else
                {
                    totaldelay = extraWait - dot11->delayUntilSignalAirborn;
                }
                return;
            }
            // if access category is not used, say for
            // management frames use difs time;
        }
//--------------------DOT11e-End-Updates---------------------------------//
        totaldelay = extraWait;
        mc(dot11)->updateStats("IFS", "DIFS");
    }
}

/// \brief Implementation of start contention
///
/// This function will set the backoff is zero  and then start the
/// difs + bo duration timer of the Ac whose Difs + Bo is minimum
void AcManager::startContention(MacDataDot11* dot11)
{
    if ((ac[0].m_frameInfo != NULL || ac[0].isAcHasPacket())
        || (ac[1].m_frameInfo != NULL || ac[1].isAcHasPacket())
        || (ac[2].m_frameInfo != NULL || ac[2].isAcHasPacket())
        || (ac[3].m_frameInfo != NULL || ac[3].isAcHasPacket())
        || (dot11->currentMessage)
        || (dot11->instantMessage))
    {
        // Check for all Access Categories if BO is zero, then set BO
        for (int i = 0; i < 4; i++)
        {
            if ((ac[i].m_frameInfo != NULL || ac[i].isAcHasPacket()))
            {
                SetBackoffIfZero(node(), dot11, i);
            }
        }

        // Find the Minimum DIFS + BO interval among all the AC's
        Int8 acIndex = getMinDifsBoIntervalAc(dot11);
/*** UAPSD *********************************START*******************/
        if (acIndex == -1)
        {
            // This can happen for a UAPSD enabled AP. It has packets
            // in the queue but coudn't send due to power save
            // restrictions.
            ERROR_Assert(MacDot11IsAp(dot11), "Should be an AP");
            return;
        }
/*** UAPSD *********************************END*******************/
        clocktype extradelay = 0;
        clocktype timerdelay = 0;

        CalculateExtraDelay(dot11, extradelay);
        timerdelay = extradelay + ac[acIndex].getAifsInterval() + ac[acIndex].getBackoffInterval();

        // Start the DIFS + BO timer for the AC[acIndex]
        m_timerMsg = startContentionTimer(dot11, timerdelay, acIndex);

        for (int i = 0; i < 4; i++)
        {
            if (ac[i].isAcHasPacket())
            {
                ac[i].startDifs(node()->getNodeTime() +
                                extradelay +
                                ac[i].getAifsInterval() +
                                ac[i].getBackoffInterval());
            }
        }
        MacDot11StationSetState(node(), dot11, DOT11_CONTENDING);
    }
}

/// \brief Implementation to handle timer for Ac manager
///
/// This function will pause the Ac's if the timer is already started
/// and the contention callback is called with the acIndex
void AcManager::handleTimer(MacDataDot11* dot11, UInt8 timerIndex)
{
    // Handle the backoff timer expiration
    if (m_timerMsg)
    {
        pauseAc(dot11);
        UInt8 acIndex = *((UInt8*)(MESSAGE_ReturnInfo(m_timerMsg,
                                                INFO_TYPE_DOT11_AC_INDEX)));
        if (timerIndex == acIndex)
        {
            mc(dot11)->contentionCallback(dot11, acIndex);
            m_timerMsg = NULL;
        }
        else
        {
            assert(false);
        }
    }
    else
    {
        assert(false);
    }
}

/// \brief Implementation of Dequeue Packet from Network Layer
///
/// This function will dequeue packets from the network layer
/// and the packet is inserted in the mac output buffer
///
/// \test This function checks if the packet can be dequeued from
/// the network layer, yes then dequeue packets and drop the extra
/// packets from network layer if the Mac queue is full.
BOOL LegacyAcManager::dqFromNetwork(MacDataDot11* dot11)
{
    BOOL dequeued = FALSE;
    Message* tempMsg = NULL;
    Mac802Address tmpNextHopAddr;
    int networkType = 0;
    TosType priority = 0;
    Message* newMsg = NULL;

    ERROR_Assert(dot11->currentMessage == NULL,
    "MacDot11MoveAPacketFromTheNetworkLayerToTheLocalBuffer: "
    "A packet exists in the local buffer.\n");
    tempMsg = NULL;
    dequeued =
        MAC_OutputQueueTopPacket(node(),
                                 ifidx(),
                                 &(tempMsg),
                                 &(tmpNextHopAddr),
                                 &networkType,
                                 &priority);

    if (MacDot11IsAPSupportPSMode(dot11)||MacDot11IsIBSSStationSupportPSMode(dot11))
    {
        if (dequeued)
        {
            if (ac[0].m_frameToSend == NULL)
            {
                dot11->currentACIndex = 0;
                ac[0].m_currentNextHopAddress = INVALID_802ADDRESS;

                MAC_OutputQueueDequeuePacket(
                    node(),
                    ifidx(),
                    &(ac[0].m_frameToSend),
                    &(ac[0].m_currentNextHopAddress),
                    &networkType,
                    &priority);

                MESSAGE_AddHeader(node(),
                                  ac[dot11->currentACIndex].m_frameToSend,
                                  sizeof(DOT11_FrameHdr),
                                  TRACE_DOT11);

                MacDot11StationSetFieldsInDataFrameHdr(dot11,
                                            (DOT11_FrameHdr*) MESSAGE_ReturnPacket(
                                            ac[0].m_frameToSend),
                                            ac[0].m_currentNextHopAddress,
                                            DOT11_DATA);

            }
            else
            {
                dequeued = FALSE;
            }
        }
    }
    else
    {
        if (dequeued)
        {
            ClassifyPacket(dot11,
                           tempMsg,
                           priority,
                           tmpNextHopAddr,
                           FALSE,
                           TRUE);
        }
    }
    return dequeued;
}

/*** UAPSD *********************************START*******************/
/// \brief Function to check if the packet exists for a destination
///
/// Function checks if there are packet queued in output buffer for a
/// for a particulat destination
///
/// \param dot11 Pointer to MacDot11 data strucure
/// \param address Destination address
int AcManager::packetsForThisRA(
        MacDataDot11* dot11,
        Mac802Address address)
{
    char acIndex = 0;
    unsigned char priority = 0;
    int pkts = 0;
    char lastAcIndex = -1;
    for (priority = 0; priority <= MAX_802_11_PRIORITY; priority++)
    {
        acIndex = (char) MacDot11ReturnAccessCategory(priority);
        if (lastAcIndex != acIndex)
        {
            if (getAc(acIndex).m_frameInfo
                && getAc(acIndex).m_frameInfo->frameType != DOT11_QOS_DATA)
            {
                if (getAc(acIndex).m_frameInfo->RA == address)
                {
                    pkts += 1;
                }
            }
            lastAcIndex = acIndex;
        }
        OutputBuffer* outputBuffer = getAc(acIndex).getBuffer();
        outputBuffer->tmpKey.first = address;
        outputBuffer->tmpKey.second = priority;
        if (outputBuffer->checkAndGetKeyPosition())
        {
            pkts += outputBuffer->Mapitr->second.numPackets;
        }
    }
    return pkts;
}
/*** UAPSD *********************************END*******************/

/// \brief Function to check if the packet can be queued in output buffer
///
/// Function to check if the packet can be queued in output buffer
///
/// \test check if the packets can be queued further in the output buffer
BOOL AcManager::CanPacketBeQueuedInOutputBuffer(
                        MacDataDot11* dot11,
                        Mac802Address nextHopAddress,
                        TosType priority,
                        BOOL* isKeyAlreadyPresent)
{
    UInt8  acIndex = 0;
    acIndex = (UInt8) MacDot11ReturnAccessCategory(priority);
    OutputBuffer *outputBuffer = getAc(acIndex).getBuffer();
    outputBuffer->tmpKey.first = nextHopAddress;
    outputBuffer->tmpKey.second = priority;

    UInt32 numPacketsInMacQueue = 0;
    if (outputBuffer->checkAndGetKeyPosition())
    {
        *isKeyAlreadyPresent = TRUE;
        ERROR_Assert(outputBuffer->Mapitr->second.numPackets
                     <= dot11->macOutputQueueSize,
                     "Outputbuffer exceeds mac output queue");
        ERROR_Assert(outputBuffer->Mapitr->second.frameQueue.size() ==
                     outputBuffer->Mapitr->second.numPackets,
                     "Frame queue size is not equal to numpackets");

        numPacketsInMacQueue = outputBuffer->Mapitr->second.numPackets;
        if (dot11->isAmsduEnable)
        {
            std::pair<Mac802Address,TosType> tmpKey;
            tmpKey.first = nextHopAddress;
            tmpKey.second = priority;
            std::map<std::pair<Mac802Address,
                TosType>,MapValue>::iterator tempMapitr;
            tempMapitr = dot11->amsduBuffer->AmsduBufferMap.find(tmpKey);
            if (tempMapitr != dot11->amsduBuffer->AmsduBufferMap.end())
            {
                numPacketsInMacQueue += tempMapitr->second.numPackets;
            }
        }
        if (numPacketsInMacQueue < dot11->macOutputQueueSize)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        *isKeyAlreadyPresent = FALSE;
        if (dot11->isAmsduEnable)
        {
            std::pair<Mac802Address,TosType> tmpKey;
            tmpKey.first = nextHopAddress;
            tmpKey.second = priority;
            std::map<std::pair<Mac802Address,TosType>,MapValue>
                                                      ::iterator tempMapitr;
            tempMapitr = dot11->amsduBuffer->AmsduBufferMap.find(tmpKey);
            if (tempMapitr != dot11->amsduBuffer->AmsduBufferMap.end())
            {
                numPacketsInMacQueue += tempMapitr->second.numPackets;
            }
        }
        if (numPacketsInMacQueue < dot11->macOutputQueueSize)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

/// \brief Implementation of Dequeue Packet from Network Layer for Dot11n Ac manager
///
/// This function will dequeue packets from the network layer
/// and the packet is inserted in the mac output buffer of dot11n
///
/// \test This function checks if the packet can be dequeued from
/// the network layer, yes then dequeue packets and drop the extra
/// packets from network layer if the Mac queue is full.
BOOL Dot11nAcManager::dqFromNetwork(MacDataDot11* dot11)
{
    int interfaceIndex = ifidx();
    BOOL dequeued = FALSE;
    Message* tempMsg = NULL;
    Mac802Address tempNextHopAddress;
    BOOL isTopPacketPresent = FALSE;
    NetworkDataIp* ip = (NetworkDataIp*) node()->networkData.networkVar;
    Scheduler *schedulerPtr
        = ip->interfaceInfo[interfaceIndex]->scheduler;
    int highestPriority = schedulerPtr->numQueue() - 1;
    int currPriority = highestPriority;
    int networkType = 0;
    TosType priority = 0;
    int acIndex;
    Message* newMsg = NULL;
    BOOL IsBroadcast = FALSE;
    BOOL pktQueuedInAmsduBuffer = FALSE;
    BOOL sendQOSFrame = TRUE;

    while (currPriority >= 0)
    {
        tempMsg = NULL;
        isTopPacketPresent = MAC_OutputQueueTopPacketForAPriority(
                                node(),
                                interfaceIndex,
                                (TosType) currPriority,
                                &(tempMsg),
                                &(tempNextHopAddress));

        if (isTopPacketPresent)
        {
            BOOL isKeyAlreadyPresent = FALSE;
            if (MacDot11IsBssStation(dot11))
            {
                if (!MAC_IsBroadcastMac802Address(&tempNextHopAddress) &&
                        tempNextHopAddress != dot11->bssAddr)
                {
                    Message* msg = MESSAGE_Duplicate(node(), tempMsg);
                    MacDot11StationInformNetworkOfPktDrop(node(),
                                                        dot11,
                                                        msg);
                }
                tempNextHopAddress = dot11->bssAddr;
            }

            if (MAC_IsBroadcastMac802Address(&tempNextHopAddress))
            {
                sendQOSFrame = TRUE;
            }
            else
            {
                sendQOSFrame = MacDot11IsDestinationQosEnable(
                    node(),
                    dot11,
                    tempNextHopAddress);
            }
            if (sendQOSFrame)
            {
                priority = MAC_GetPacketsPriority(node(), tempMsg);
            }
            else
            {
                priority = 0;
            }
            //priority = MAC_GetPacketsPriority(node(), tempMsg);
            BOOL canBeQueued =
                    CanPacketBeQueuedInOutputBuffer(dot11,
                                                    tempNextHopAddress,
                                                    priority,
                                                    &isKeyAlreadyPresent);
            if (canBeQueued)
            {
                dequeued = MAC_OutputQueueDequeuePacketForAPriority(
                                                    node(),
                                                    interfaceIndex,
                                                    currPriority,
                                                    &(tempMsg),
                                                    &(tempNextHopAddress),
                                                    &networkType);
                if (dequeued)
                {
                    dot11->pktsToSend++;
                    if (MacDot11IsBssStation(dot11))
                    {
                        if (!MAC_IsBroadcastMac802Address(
                            &tempNextHopAddress)
                            && tempNextHopAddress != dot11->bssAddr)
                        {
                            Message* msg = MESSAGE_Duplicate(node(), tempMsg);
                            MacDot11StationInformNetworkOfPktDrop(node(),
                                                                    dot11,
                                                                    msg);
                        }
                        tempNextHopAddress = dot11->bssAddr;
                    }

                    if (MAC_IsBroadcastMac802Address(&tempNextHopAddress))
                    {
                       sendQOSFrame = TRUE;
                    }
                    else
                    {
                        sendQOSFrame = MacDot11IsDestinationQosEnable(
                            node(),
                            dot11,
                            tempNextHopAddress);
                    }
                    if (sendQOSFrame)
                    {
                        priority = MAC_GetPacketsPriority(node(), tempMsg);
                    }
                    else
                    {
                        priority = 0;
                    }
                    //priority = MAC_GetPacketsPriority(node(), tempMsg);
                    acIndex = MacDot11ReturnAccessCategory(priority) ;
                    IsBroadcast
                        = MAC_IsBroadcastMac802Address(&tempNextHopAddress);

                    if (dot11->isAmsduEnable && !IsBroadcast)
                    {
                        pktQueuedInAmsduBuffer = TRUE;
                        MacDot11nEnqueuePacketInAmsduBuffer(node(),
                                                dot11,
                                                tempMsg,
                                                tempNextHopAddress,
                                                priority);
                    }
                    else
                    {
                        DOT11_FrameInfo* frameInfo
                                            = (DOT11_FrameInfo*)
                                                MEM_malloc(sizeof(DOT11_FrameInfo));
                        memset(frameInfo, 0, sizeof(DOT11_FrameInfo));
                        frameInfo->msg = tempMsg;
                        frameInfo->RA = tempNextHopAddress;
                        frameInfo->TA = dot11->selfAddr;
                        frameInfo->SA = dot11->selfAddr;
                        frameInfo->DA = dot11->ipDestAddr;
                        frameInfo->insertTime = node()->getNodeTime();
                        frameInfo->frameType = DOT11_QOS_DATA;
                        newMsg = frameInfo->msg;
                        MESSAGE_AddHeader(node(),
                                            newMsg,
                                            sizeof(DOT11e_FrameHdr),
                                            TRACE_DOT11);

                        MacDot11nSetFieldsInMacHeader(dot11,
                                                (DOT11n_FrameHdr*)
                                                MESSAGE_ReturnPacket(frameInfo->msg),
                                                frameInfo->RA,
                                                frameInfo->frameType,
                                                acIndex,
                                                priority,
                                                FALSE);
                        enqueue(dot11,
                                frameInfo,
                                priority,
                                tempNextHopAddress,
                                isKeyAlreadyPresent);
                    }
                }
            }
            else
            {
                currPriority--;
            }
        }
        else
        {
            currPriority--;
        }
    }

    // Check if any Ac is empty
    BOOL isAnyAcEmpty = FALSE;
    for (int i = 0; i < 4; i++)
    {

        if (!(ac[i].isAcHasPacket()) && !(ac[i].m_frameInfo))

        {
            isAnyAcEmpty = TRUE;
            break;
        }
    }

    BOOL deque = FALSE;
    if (isAnyAcEmpty)
    {
        do
        {
            deque = MAC_OutputQueueDequeuePacket(
                        node(),
                        interfaceIndex,
                        &(tempMsg),
                        &(tempNextHopAddress),
                        &networkType,
                        &priority);

            if (deque)
            {
                dot11->numPktsDroppedFromOutputBuffer++;
                MESSAGE_Free(node(), tempMsg);
                tempMsg = NULL;
            }
        } while (deque);
    }

    // Amsdu Handling
    if (dot11->amsduBuffer)
    {
        std::map<std::pair<Mac802Address,TosType>,MapValue>::iterator keyItr
            = dot11->amsduBuffer->AmsduBufferMap.begin();
        while (keyItr != dot11->amsduBuffer->AmsduBufferMap.end())
        {
            while (keyItr->second.numPackets > 0)
            {
                UInt8 numPackets = 0;
                BOOL canAmsduBeCreated =
                Macdot11nCanAmsduBeCreated(
                    node(),
                    dot11,
                    keyItr->first.first,
                    keyItr->first.second,
                    &numPackets);
                if (canAmsduBeCreated)
                {
                    ERROR_Assert(numPackets >= 2,
                        "num packets is less than 2");
                    Macdot11nCancelAmsduBufferTimer(
                        node(),
                        dot11,
                        keyItr->first.first,
                        keyItr->first.second);
                    Macdot11nCreateAmsdu(
                        node(),
                        dot11,
                        keyItr->first.first,
                        keyItr->first.second,
                        numPackets);
                    continue;
                }
                else
                {
                    if (numPackets != 0)
                    {
                        // cant send an amsdu packet
                        Macdot11nCancelAmsduBufferTimer(
                            node(),
                            dot11,
                            keyItr->first.first,
                            keyItr->first.second);
                        Macdot11nDequeueAllPacketsFromAmsduBuffer(
                            node(),
                            dot11,
                            keyItr->first.first,
                            keyItr->first.second);
                    }
                    else
                    {
                        // one packet in the amsdu buffer queue.
                        // start timer
                        Macdot11nSetAmsduBufferTimer(
                            node(),
                            dot11,
                            keyItr->first.first,
                            keyItr->first.second);
                    }
                    break;
                }
            }
            keyItr++;
        }
    }
    return dequeued;
}


/// \brief Implementation of enqueue Packet from to mac output buffer
///
/// This function will insert the packet in the mac output buffer
void Dot11nAcManager::enqueue(MacDataDot11* dot11,
                              DOT11_FrameInfo *frameInfo,
                              TosType priority,
                              Mac802Address NextHopAddress,
                              BOOL isKeyAlreadyPresent)
{
    UInt8  acIndex;
    acIndex = (UInt8)MacDot11ReturnAccessCategory(priority);
    OutputBuffer *outputBuffer = getAc(acIndex).getBuffer();
    dot11->numPktsQueuedInOutputBuffer++;
    DOT11n_FrameHdr* hdr =
            (DOT11n_FrameHdr*)MESSAGE_ReturnPacket(frameInfo->msg);
    if (isKeyAlreadyPresent)
    {
        hdr->seqNo = outputBuffer->Mapitr->second.seqNum;
    }
    else
    {
        hdr->seqNo = 0;
    }
    ERROR_Assert(hdr->seqNo <= 4095,"Header Sequence no is above 4095");
    outputBuffer->Enqueue(dot11,
                          node(),
                          NextHopAddress,
                          frameInfo,
                          isKeyAlreadyPresent);

    ac[acIndex].setIsAcHasPacket(TRUE);
    MacDot11nSetMoreFramesFieldForAnAC(dot11, acIndex, outputBuffer);
}

/// \brief Implementation of Interupt contention function
///
/// This function will pause the Ac's if the timer is already started
/// and the timer message is cancelled
void AcManager::interruptContention(MacDataDot11* dot11)
{
    if (m_timerMsg)
    {
        pauseAc(dot11);
        MESSAGE_CancelSelfMsg(node(), m_timerMsg);
        m_timerMsg = NULL;
    }
}

/// \brief Function to set backoff if zero
///
/// This is the function to set the BO of Ac's if its zero
///
/// \param node Node Pointer
/// \param dot11 Dot11 pointer
/// \param currentACIndex Current Ac Index
void AcManager::SetBackoffIfZero(
    Node* node,
    MacDataDot11* dot11,
    int currentACIndex = DOT11e_INVALID_AC)
{
    // DOT11e enabled
    if (MacDot11IsQoSEnabled(node, dot11))
    {
        if (currentACIndex != DOT11e_INVALID_AC)
        {
            if (ac[currentACIndex].getBackoffInterval() == 0)
            {
                ac[currentACIndex].setBackoffInterval(
                    (RANDOM_nrand(dot11->seed) %
                    (ac[currentACIndex].getCW() + 1)) *
                        ac[currentACIndex].getSlotTime());
            }
        }
        else
        {
            if (dot11->BO == 0)
            {
                dot11->BO = (RANDOM_nrand(dot11->seed) %
                        (dot11->CW + 1))
                        * dot11->slotTime;
            }
        }
    }
    else // Legacy mode
    {
        assert(currentACIndex != DOT11e_INVALID_AC);
        if (currentACIndex != DOT11e_INVALID_AC)
        {
            if (ac[currentACIndex].getBackoffInterval() == 0)
            {
                 ac[currentACIndex].setBackoffInterval(
                    (RANDOM_nrand(dot11->seed) %
                    (ac[currentACIndex].getCW() + 1)) *
                     ac[currentACIndex].getSlotTime());
                 dot11->BO = ac[currentACIndex].getBackoffInterval();
            }
        }
        else
        {
            if (dot11->BO == 0)
            {
//---------------------------Power-Save-Mode-Updates---------------------//
                if (MacDot11IsATIMDurationActive(dot11))
                {
                    dot11->BO =
                        (RANDOM_nrand(dot11->seed) % (dot11->cwMin + 1))
                        * dot11->slotTime;
                }
                else
                {
                     dot11->BO =
                         (RANDOM_nrand(dot11->seed) % (dot11->CW + 1))
                         * dot11->slotTime;
                }
//---------------------------Power-Save-Mode-End-Updates-----------------//
            }
        }
    }
}

/// \brief Function to classify packet
///
/// This function will calssify the incoming packet to the respective
/// Access Categories
///
/// \param dot11 Dot11 structure pointer
/// \param msg Incoming message
/// \param queuePriority priority of the queue
/// \param NextHopAddress next hop address
/// \param SendQosFrame has to send qos or not
BOOL AcManager::ClassifyPacket(MacDataDot11* dot11,
                               Message* msg,
                               TosType queuePriority,
                               Mac802Address NextHopAddress,
                               BOOL SendQosFrame,
                               BOOL legacyMode)
{
    BOOL dequeued = FALSE;
    int networkType;
    int interfaceIndex = ifidx();

//---------------------DOT11e--Updates------------------------------------//
//--------------------HCCA-Updates Start---------------------------------//
    Message* tempMsg = NULL;
    Mac802Address tempNextHopAddress = NextHopAddress ;
    Message* newMsg;
    TosType priority;
    int acIndex;

    BOOL isACHasMsgForCurrentAccessCategory = FALSE;
    BOOL isEDCAPacket = FALSE;
    BOOL isHCCAPacket = FALSE;
    int highestPriority = 7;
    if (tempNextHopAddress == ANY_MAC802 &&
        (MacDot11IsAp(dot11)) &&
        MacDot11IsQoSEnabled(node(), dot11))
    {
        priority = MAC_GetPacketsPriority(node(), msg);
        acIndex = MacDot11ReturnAccessCategory(priority) ;
    }
    else
    {
        if (SendQosFrame)
        {
              priority = MAC_GetPacketsPriority(node(), msg);
              acIndex = MacDot11ReturnAccessCategory(priority) ;
        }
        else
        {
            priority = 0;
            acIndex = DOT11e_AC_BK;
        }
    }
    //Find if this a HCCA packet
    if (dot11->isHCCAEnable &&
        (MacDot11IsHC(dot11) || dot11->associatedWithHC) &&
        (priority == highestPriority ||
         priority >= DOT11e_HCCA_LOWEST_PRIORITY ))
    {
        //check if it is a broadcast packet
        if (tempNextHopAddress == ANY_MAC802)
        {
            //It is a broadcast packet
            isHCCAPacket = FALSE;
        }
        else
        {
            isHCCAPacket = TRUE;
        }
    }

    //If it is HC check if the STA supports HCCA
    if (MacDot11IsHC(dot11) && isHCCAPacket)
    {
        DOT11_ApStationListItem* stationItem =
            MacDot11ApStationListGetItemWithGivenAddress(
                node(),
                dot11,
                tempNextHopAddress);

        if (stationItem)
        {
            if (stationItem->data->flags)
            {
                isHCCAPacket = TRUE;
            }
            else
            {
                isHCCAPacket = FALSE;
            }
        }
        else
        {
            isHCCAPacket = FALSE;
        }
    }

    // Find if this an EDCA packet or DCF packet
    if (ac[acIndex].m_frameToSend !=  NULL) {
        isACHasMsgForCurrentAccessCategory = TRUE;
    }
    else
    {
        isACHasMsgForCurrentAccessCategory = FALSE;
        assert(ac[acIndex].m_BO == 0);
    }

    // If it is not a HCCA packet then it is an EDCA packet
    // check if the AC has space and the state is correct to dequeue
    if (!isHCCAPacket){
        isEDCAPacket = TRUE;
    }

    //It is a HCCA packet, dequeue it and put in TS Buffer
    if (isHCCAPacket)
    {
        if (!MacDot11eIsTSBuffreFull(node(), dot11, priority))
        {
            dequeued = MAC_OutputQueueDequeuePacketForAPriority(
                            node(),
                            interfaceIndex,
                            queuePriority,
                            &(tempMsg),
                            &(tempNextHopAddress),
                            &networkType);

            if (dequeued)
            {
                if (MacDot11IsBssStation(dot11))
                {

                    if (!MAC_IsBroadcastMac802Address(&tempNextHopAddress) &&
                        tempNextHopAddress != dot11->bssAddr)
                    {
                        Message* msg = MESSAGE_Duplicate(node(), tempMsg);
                        MacDot11StationInformNetworkOfPktDrop(node(),
                                                          dot11,
                                                          msg);
                    }
                }
                MacDot11eStationMoveAPacketFromTheNetworkToTheTSsBuffer(
                    node(),
                    dot11,
                    (unsigned)priority,
                    tempMsg,
                    tempNextHopAddress);
                dot11->pktsToSend++;

                //It can potentially take more messages
                return TRUE;
            }
            else {
                //unable to dequeue
                return FALSE;
            }
        }
        else {
            //Buffer is full cannot take in more packets of this priority
            return FALSE;
        }
    }

    // This is EDCA packet dequeue it to the corresponding AC
    if (isEDCAPacket == TRUE)
    {
        BOOL canBeQueuedinAc = FALSE;
        if (!legacyMode)
        {
            if (!isACHasMsgForCurrentAccessCategory
                || (ac[acIndex].frameInfoList.size()
                <= dot11->macOutputQueueSize))
            {
                canBeQueuedinAc = TRUE;
            }
        }
        else
        {
            if (!isACHasMsgForCurrentAccessCategory)
            {
                canBeQueuedinAc = TRUE;
            }
        }

        if (canBeQueuedinAc)
        {
            dequeued = MAC_OutputQueueDequeuePacketForAPriority(
                                node(),
                                interfaceIndex,
                                queuePriority,
                                &(tempMsg),
                                &(tempNextHopAddress),
                                &(ac[acIndex].m_networkType));

            if (dequeued)
            {
                dot11->ipNextHopAddr = tempNextHopAddress;
                dot11->ipDestAddr = tempNextHopAddress;
                if (MacDot11IsBssStation(dot11))
                {
                    if (!MAC_IsBroadcastMac802Address(&tempNextHopAddress) &&
                        tempNextHopAddress != dot11->bssAddr)
                    {
                        Message* msg = MESSAGE_Duplicate(node(), tempMsg);
                        MacDot11StationInformNetworkOfPktDrop(
                            node(),
                            dot11,
                            msg);
                    }
                    tempNextHopAddress = dot11->bssAddr;
                }

                DOT11_FrameInfo* frameInfo =
                    (DOT11_FrameInfo*) MEM_malloc(sizeof(DOT11_FrameInfo));
                memset(frameInfo, 0, sizeof(DOT11_FrameInfo));
                frameInfo->msg = tempMsg;
                frameInfo->RA = tempNextHopAddress;
                frameInfo->TA = dot11->selfAddr;
                frameInfo->SA = dot11->selfAddr;
                frameInfo->DA = dot11->ipDestAddr;
                frameInfo->insertTime =
                        node()->getNodeTime();

                // Add frame info to AC and set is AC has packet
                ac[acIndex].setIsAcHasPacket(TRUE);

                if (SendQosFrame)
                {
                    frameInfo->frameType = DOT11_QOS_DATA;
                    newMsg = frameInfo->msg;
                    MESSAGE_AddHeader(node(),
                            newMsg,
                            sizeof(DOT11e_FrameHdr),
                            TRACE_DOT11);


                    MacDot11eStationSetFieldsInDataFrameHdr(dot11,
                                                        (DOT11e_FrameHdr*) MESSAGE_ReturnPacket(newMsg),
                                                        frameInfo->RA,
                                                        frameInfo->frameType,
                                                        acIndex);

                     ((DOT11e_FrameHdr*) MESSAGE_ReturnPacket(newMsg))->qoSControl.TID
                        = (UInt16)priority;

                     DOT11_SeqNoEntry *entry = MacDot11StationGetSeqNo(
                                                     node(),
                                                     dot11,
                                                     frameInfo->RA,
                                                     priority);
                     ((DOT11e_FrameHdr*)MESSAGE_ReturnPacket(newMsg))->seqNo
                                                            = entry->toSeqNo;
                     entry->toSeqNo++;
                }
                else
                {
                    //for DCF data processing in AC[BE]
                    frameInfo->frameType = DOT11_DATA;
                    newMsg = frameInfo->msg;

                    MESSAGE_AddHeader(node(),
                            newMsg,
                            sizeof(DOT11_FrameHdr),
                            TRACE_DOT11);

                    MacDot11StationSetFieldsInDataFrameHdr(
                        dot11,
                        (DOT11_FrameHdr*) MESSAGE_ReturnPacket(newMsg),
                        frameInfo->RA,
                        frameInfo->frameType);
                }

                frameInfo->msg = newMsg;
                dot11->pktsToSend++;
                ac[acIndex].m_totalNoOfthisTypeFrameQueued++;

                // Push back into frame info list
                ac[acIndex].frameInfoList.push_back(frameInfo);

                if (!isACHasMsgForCurrentAccessCategory)
                {
                    DOT11_FrameInfo* tmpFrameInfo = ac[acIndex].frameInfoList.front();
                    ac[acIndex].frameInfoList.pop_front();

                    ac[acIndex].m_frameInfo = tmpFrameInfo;
                    ac[acIndex].m_frameToSend = tmpFrameInfo->msg;

                    //Reset AC veriables
                    ac[acIndex].m_priority = priority;
                    ac[acIndex].m_QSRC = 0;
                    ac[acIndex].m_QLRC = 0;
                    if (ac[acIndex].getCW() == 0)
                    {
                        ac[acIndex].setCW(ac[acIndex].getMinCW());
                    }
                }

                if (DEBUG_QA)
                {
                    unsigned char* frame_type =
                        (unsigned char*) MESSAGE_ReturnPacket(ac[acIndex].m_frameInfo->msg);
                    printf(" Node %u ac[ %d] used priority %d for frame %x\n",
                        node()->nodeId,
                        acIndex,
                        ac[acIndex].m_priority,*frame_type);
                }

                //Return true if the next packet could be a potentially HCCA packet
                if (dot11->isHCCAEnable &&
                    (MacDot11IsHC(dot11) || dot11->associatedWithHC) &&
                    (queuePriority == highestPriority ||
                    queuePriority >= DOT11e_HCCA_LOWEST_PRIORITY ))
                {
                    return TRUE;
                }

                // Added to enable dequeue of other packets in qos mode
                if (SendQosFrame)
                {
                    return TRUE;
                }
            }
            else
            {
                return FALSE; // can't dequeue more for this priority
            }
            return TRUE; // now we have multiple packet queues so dequeue more
        }
        else
        {
            return FALSE; // Packets can't be queued further in this AC
        }
    } // EDCA mode
//--------------------HCCA-Updates End---------------------------------//
//--------------------DOT11e-End-Updates---------------------------------//
    return FALSE;
} // ClassifyPacket

/// \brief Implementation of Dequeue Packet from Network Layer
///
/// This function will dequeue packets from the network layer
/// and the packet is inserted in the mac output buffer
///
/// \test This function checks if the packet can be dequeued from
/// the network layer, yes then dequeue packets and drop the extra
/// packets from network layer if the Mac queue is full.
BOOL Dot11eAcManager::dqFromNetwork(MacDataDot11* dot11)
{
    // if Qos enabled then put the packet to proper AC or to correct Traffic stream
    //using Traffic Classifier

    // This is used to get the highest priority packets.
    // However, this is dependent on No. of Network Queues.
    // The best performance is achieved if there is 8 (0-7) queues.
    // If no of queues is less than the highestPriority(7) then
    // all the packtes of higher priorities will
    // pile up in the last queue, treated as maximum queue.

    // First get the heighest priority queue no, It holds all the
    // heighest priority packets specified in application.

    BOOL peek = FALSE;
    int interfaceIndex = ifidx();
    Message* tempMsg = NULL;
    Mac802Address tempNextHopAddress;
    BOOL SendQOSFrame = FALSE;
    int networkType = 0;
    TosType priority = 0;

    NetworkDataIp *ip = (NetworkDataIp *) node()->networkData.networkVar;
    Scheduler *schedulerPtr =
                        ip->interfaceInfo[interfaceIndex]->scheduler;
    int highestPriority = schedulerPtr->numQueue() - 1;
    int currPriority = highestPriority;
    while (currPriority >= 0)
    {
        tempMsg = NULL;
        peek = MAC_OutputQueueTopPacketForAPriority(
                                node(),
                                interfaceIndex,
                                (TosType) currPriority,
                                &(tempMsg),
                                &(tempNextHopAddress));
        if (peek == TRUE)
        {
            SendQOSFrame = MacDot11IsDestinationQosEnable(
                node(),
                dot11,
                tempNextHopAddress);

            if (!ClassifyPacket(dot11,
                                tempMsg,
                                (TosType) currPriority,
                                tempNextHopAddress,
                                SendQOSFrame))
            {
                currPriority--;
            }
        }
        else
        {
            currPriority--;
        }
    }

    // Additional code added to handle dequeuing more packet
    // and dropping packets if any AC is empty
    BOOL isAnyAcEmpty = FALSE;
    for (int i = 0; i < 4; i++)
    {
        if (!(ac[i].isAcHasPacket()) && !(ac[i].m_frameInfo))
        {
            isAnyAcEmpty = TRUE;
            break;
        }
    }

    BOOL deque = FALSE;
    if (isAnyAcEmpty)
    {
        do
        {
            deque = MAC_OutputQueueDequeuePacket(
                        node(),
                        interfaceIndex,
                        &(tempMsg),
                        &(tempNextHopAddress),
                        &networkType,
                        &priority);

            if (deque)
            {
                MESSAGE_Free(node(), tempMsg);
                tempMsg = NULL;
//TODO: Update a statistic here
            }
        } while (deque);
    }
    return FALSE;
}

/// \brief Initialize Access Category
///
/// This function  is used to initialize all the Access Categories
///
/// \param node Node Pointer
/// \param dot11 Dot11 structure pointer
/// \param phyModel Phy Model at the node
void LegacyAcManager::InitAc(Node* node, MacDataDot11* dot11, PhyModel phyModel)
{
    // Resize ac with one element
    ac.resize(1);
    mc(dot11)->ac()->setNumAc(1);

    // EDCA default values for Contention window and
    // parameters of Access categories.
    ac[DOT11e_AC_BK].m_cwMin = dot11->cwMin;
    ac[DOT11e_AC_BK].m_cwMax = dot11->cwMax;
    ac[DOT11e_AC_BK].m_slotTime = dot11->slotTime;
    ac[DOT11e_AC_BK].m_AIFS = dot11->difs;
    ac[DOT11e_AC_BK].m_CW = ac[DOT11e_AC_BK].m_cwMin;
    ac[DOT11e_AC_BK].m_BO = 0;
    ac[DOT11e_AC_BK].m_frameInfo = NULL;
    ac[DOT11e_AC_BK].m_Difs = dot11->difs;
    ac[DOT11e_AC_BK].m_TXOPLimit = DOT11e_802_11a_AC_BK_TXOPLimit;
}


/// \brief Initialize Access Category
///
/// This function  is used to initialize all the Access Categories
///
/// \param node Node Pointer
/// \param dot11 Dot11 structure pointer
/// \param phyModel Phy Model at the node
void AcManager::InitAc(Node* node, MacDataDot11* dot11, PhyModel phyModel)
{
    // Resize the Ac Vector to four elements
    ac.resize(4);
    mc(dot11)->ac()->setNumAc(4);

    // EDCA default values for Contention window and
    // parameters of Access categories.
    dot11->macOutputQueueSize = DEFAULT_MAC_QUEUE_SIZE;

    if (phyModel == PHY802_11a || phyModel == PHY802_11n || phyModel == PHY802_11ac){

        // Access Category 0
        ac[DOT11e_AC_BK].m_cwMin = DOT11_802_11a_CW_MIN;
        ac[DOT11e_AC_BK].m_cwMax = DOT11_802_11a_CW_MAX;
        ac[DOT11e_AC_BK].m_slotTime = DOT11_802_11a_SLOT_TIME;

        if (phyModel == PHY802_11n || phyModel == PHY802_11ac)
        {
           ac[DOT11e_AC_BK].m_outputBuffer = new OutputBuffer;
           ac[DOT11e_AC_BK].m_isACHasPacket = FALSE;
        }

        // Arbitary Inter space number
        ac[DOT11e_AC_BK].m_AIFSN = DOT11e_802_11a_AC_BK_AIFSN;

        // Transmission opportunity limit
        ac[DOT11e_AC_BK].m_TXOPLimit = DOT11e_802_11a_AC_BK_TXOPLimit;

        ac[DOT11e_AC_BK].m_AIFS = dot11->sifs +
                   ac[DOT11e_AC_BK].m_AIFSN * ac[0].m_slotTime;

        // Access Category 1
        ac[DOT11e_AC_BE].m_cwMin = DOT11_802_11a_CW_MIN;
        ac[DOT11e_AC_BE].m_cwMax = DOT11_802_11a_CW_MAX;
        ac[DOT11e_AC_BE].m_slotTime = DOT11_802_11a_SLOT_TIME;

        if (phyModel == PHY802_11n || phyModel == PHY802_11ac)
        {
           ac[DOT11e_AC_BE].m_outputBuffer = new OutputBuffer;
           ac[DOT11e_AC_BE].m_isACHasPacket = FALSE;
        }

        // Arbitary Inter space number
        ac[DOT11e_AC_BE].m_AIFSN = DOT11e_802_11a_AC_BE_AIFSN;

        // Transmission opportunity limit
        ac[DOT11e_AC_BE].m_TXOPLimit = DOT11e_802_11a_AC_BE_TXOPLimit;
        ac[DOT11e_AC_BE].m_AIFS= dot11->sifs +
                    ac[DOT11e_AC_BE].m_AIFSN * dot11->slotTime;

        // Access Category 2
        ac[DOT11e_AC_VI].m_cwMin = ((DOT11_802_11a_CW_MIN + 1) / 2) - 1;
        ac[DOT11e_AC_VI].m_cwMax = DOT11_802_11a_CW_MIN;
        ac[DOT11e_AC_VI].m_slotTime = DOT11_802_11a_SLOT_TIME;

        if (phyModel == PHY802_11n || phyModel == PHY802_11ac)
        {
           ac[DOT11e_AC_VI].m_outputBuffer = new OutputBuffer;
           ac[DOT11e_AC_VI].m_isACHasPacket = FALSE;
        }

        // Arbitary Inter space number
        ac[DOT11e_AC_VI].m_AIFSN = DOT11e_802_11a_AC_VI_AIFSN;;

        // Transmission opportunity limit
        ac[DOT11e_AC_VI].m_TXOPLimit = DOT11e_802_11a_AC_VI_TXOPLimit;
        ac[DOT11e_AC_VI].m_AIFS= dot11->sifs +
                    ac[DOT11e_AC_VI].m_AIFSN * ac[2].m_slotTime;

        // Access Category 3
        ac[DOT11e_AC_VO].m_cwMin = ((DOT11_802_11a_CW_MIN + 1) / 4) - 1;
        ac[DOT11e_AC_VO].m_cwMax = ((DOT11_802_11a_CW_MIN + 1) / 2) - 1;
        ac[DOT11e_AC_VO].m_slotTime = DOT11_802_11a_SLOT_TIME;

        if (phyModel == PHY802_11n || phyModel == PHY802_11ac)
        {
           ac[DOT11e_AC_VO].m_outputBuffer = new OutputBuffer;
           ac[DOT11e_AC_VO].m_isACHasPacket = FALSE;
        }

        // Arbitary Inter space number
        ac[DOT11e_AC_VO].m_AIFSN = DOT11e_802_11a_AC_VO_AIFSN;

        // Transmission opportunity limit
        ac[DOT11e_AC_VO].m_TXOPLimit = DOT11e_802_11a_AC_VO_TXOPLimit;
        ac[DOT11e_AC_VO].m_AIFS= dot11->sifs +
        ac[DOT11e_AC_VO].m_AIFSN * ac[3].m_slotTime;
   } else if (phyModel == PHY802_11b || phyModel == PHY_ABSTRACT) {

       // Access Category 0
       ac[DOT11e_AC_BK].m_cwMin = DOT11_802_11b_CW_MIN;
       ac[DOT11e_AC_BK].m_cwMax = DOT11_802_11b_CW_MAX;
       ac[DOT11e_AC_BK].m_slotTime = DOT11_802_11b_SLOT_TIME;

       // Arbitary Inter space number
       ac[DOT11e_AC_BK].m_AIFSN = DOT11e_802_11b_AC_BK_AIFSN ;

       // Transmission opportunity limit
       ac[DOT11e_AC_BK].m_TXOPLimit = DOT11e_802_11a_AC_BK_TXOPLimit;
       ac[DOT11e_AC_BK].m_AIFS= dot11->sifs +
                  ac[DOT11e_AC_BK].m_AIFSN * ac[0].m_slotTime;

       // Access Category 1
       ac[DOT11e_AC_BE].m_cwMin = DOT11_802_11b_CW_MIN;
       ac[DOT11e_AC_BE].m_cwMax = DOT11_802_11b_CW_MAX;
       ac[DOT11e_AC_BE].m_slotTime = DOT11_802_11b_SLOT_TIME;

       // Arbitary Inter space number
       ac[DOT11e_AC_BE].m_AIFSN = DOT11e_802_11b_AC_BE_AIFSN;

       // Transmission opportunity limit
       ac[DOT11e_AC_BE].m_TXOPLimit = DOT11e_802_11b_AC_BE_TXOPLimit;
       ac[DOT11e_AC_BE].m_AIFS= dot11->sifs +
                   ac[DOT11e_AC_BE].m_AIFSN * dot11->slotTime;

       // Access Category 2
       ac[DOT11e_AC_VI].m_cwMin = ((DOT11_802_11b_CW_MIN + 1) / 2) - 1;
       ac[DOT11e_AC_VI].m_cwMax = DOT11_802_11b_CW_MIN;
       ac[DOT11e_AC_VI].m_slotTime = DOT11_802_11b_SLOT_TIME;

       // Arbitary Inter space number
       ac[DOT11e_AC_VI].m_AIFSN = DOT11e_802_11b_AC_VI_AIFSN;

       // Transmission opportunity limit
       ac[DOT11e_AC_VI].m_TXOPLimit = DOT11e_802_11b_AC_VI_TXOPLimit;
       ac[DOT11e_AC_VI].m_AIFS= dot11->sifs +
                   ac[DOT11e_AC_VI].m_AIFSN * ac[2].m_slotTime;

       // Access Category 3
       ac[DOT11e_AC_VO].m_cwMin = ((DOT11_802_11b_CW_MIN + 1) / 4) - 1;
       ac[DOT11e_AC_VO].m_cwMax = ((DOT11_802_11b_CW_MIN + 1) / 2) - 1;
       ac[DOT11e_AC_VO].m_slotTime = DOT11_802_11b_SLOT_TIME;

       // Arbitary Inter space number
       ac[DOT11e_AC_VO].m_AIFSN = DOT11e_802_11b_AC_VO_AIFSN;

       // Transmission opportunity limit
       ac[DOT11e_AC_VO].m_TXOPLimit = DOT11e_802_11b_AC_VO_TXOPLimit;
       ac[DOT11e_AC_VO].m_AIFS= dot11->sifs +
       ac[DOT11e_AC_VO].m_AIFSN * ac[3].m_slotTime;
    }
    else if (phyModel == PHY802_11pCCH || phyModel == PHY802_11pSCH)
    {
        Mac802_11p_AcInit(node, dot11, phyModel);
    }
} // end of AC init

/*!
* \brief Function to recalculate difs and bo and return min Ac Index
*/
void AcManager::ReCalculateDifsBoandStartContention(MacDataDot11* dot11)
{
    // If there is any Ac which got new packet calculate its endtime
    // and recalculate for contention
    for (int i = 0; i < 4; i++)
    {
        if ((ac[i].getState() == k_State_IDLE) && (ac[i].m_frameInfo || ac[i].isAcHasPacket()))
        {
            clocktype extradelay = 0;
            clocktype timerdelay = 0;
            CalculateExtraDelay(dot11, extradelay);

            SetBackoffIfZero(node(), dot11, i);

            timerdelay = extradelay + ac[i].getAifsInterval() + ac[i].getBackoffInterval();
            ac[i].startDifs(node()->getNodeTime() + timerdelay);
        }
    }

    UInt8 minAcIndex = 0;
    clocktype minValue = CLOCKTYPE_MAX;
    for (int acIndex = 0; acIndex < 4; acIndex++)
    {
        if ((ac[acIndex].getBoEndTime() < minValue)
            && canThisACContend(dot11, acIndex)
            && (ac[acIndex].m_frameInfo || ac[acIndex].isAcHasPacket()))
        {
            minValue = ac[acIndex].getBoEndTime();
            minAcIndex = acIndex;
        }
    }

    ERROR_Assert(
      MESSAGE_ReturnInfo(m_timerMsg,INFO_TYPE_DOT11_AC_INDEX) != NULL,
      "Access category information should be present in the timer message");

    // Check if acIndex already running timer is same as new minimum value or not
    UInt8 oldAcIndex = *((UInt8*)(MESSAGE_ReturnInfo(m_timerMsg,
                                                INFO_TYPE_DOT11_AC_INDEX)));
    if (oldAcIndex == minAcIndex)
    {
        return;
    }
    else
    {
        MESSAGE_CancelSelfMsg(node(), m_timerMsg);
        m_timerMsg = NULL;

        // Start the DIFS + BO timer for the AC[acIndex]
        clocktype extradelay = 0;
        clocktype timerdelay = 0;
        CalculateExtraDelay(dot11, extradelay);

        timerdelay = extradelay + ac[minAcIndex].getAifsInterval() + ac[minAcIndex].getBackoffInterval();

        m_timerMsg = startContentionTimer(dot11, timerdelay, minAcIndex);
        MacDot11StationSetState(node(), dot11, DOT11_CONTENDING);
    }
}


/// \brief Function to set the current message
///
/// Set current message function will set the dot11->currentMessage with the message
/// which won in the Access category contention
///
/// \param dot11 Dot11 Structure pointer
/// \param acIndex Index of Ac who won the contention
void LegacyAcManager::setCurrentMessage(MacDataDot11* dot11, UInt8 acIndex)
{
    // Rank the Access Category according to transmission opportunity.
    // the highest opportunity limit wins the contention.
    if (ac[dot11->currentACIndex].m_frameInfo == NULL
        && (dot11->dot11TxFrameInfo != NULL && (dot11->dot11TxFrameInfo->frameType
            != DOT11_BEACON && dot11->dot11TxFrameInfo->frameType != DOT11_ATIM))) {
        ERROR_ReportError("There is no frame in AC\n");
    }

    if (!dot11->currentMessage)
    {
        if (ac[dot11->currentACIndex].m_frameInfo)
        {
            assert(!dot11->dot11TxFrameInfo);
            dot11->dot11TxFrameInfo = ac[dot11->currentACIndex].m_frameInfo;
            dot11->currentMessage = ac[dot11->currentACIndex].m_frameInfo->msg;
            dot11->ipNextHopAddr = ac[dot11->currentACIndex].m_frameInfo->RA;

            if (MacDot11IsBssStation(dot11)&& (dot11->bssAddr!= dot11->selfAddr))
            {
                dot11->currentNextHopAddress = dot11->bssAddr;
            }
            else
            {
                dot11->currentNextHopAddress =
                     ac[dot11->currentACIndex].m_frameInfo->RA;
            }
            dot11->BO = ac[dot11->currentACIndex].m_BO;
            dot11->SSRC = ac[acIndex].m_QSRC ;
            dot11->SLRC = ac[acIndex].m_QLRC ;
        }
        else
        {
            assert(dot11->instantMessage);
        }
    }
}

/// \brief Function to set the current message
///
/// Set current message function will set the dot11->currentMessage with the message
/// which won in the Access category contention
///
/// \param dot11 Dot11 Structure pointer
/// \param acIndex Index of Ac who won the contention
void Dot11eAcManager::setCurrentMessage(MacDataDot11* dot11, UInt8 acIndex)
{
    if (ac[dot11->currentACIndex].m_frameInfo == NULL
        && (dot11->dot11TxFrameInfo != NULL && (dot11->dot11TxFrameInfo->frameType
            != DOT11_BEACON))
            && !dot11->instantMessage) {
        ERROR_ReportError("There is no frame to send\n");
    }

    BOOL switchPkts = TRUE;

    if (dot11->currentMessage)
    {
        assert(dot11->dot11TxFrameInfo);
        if (dot11->dot11TxFrameInfo->frameType == DOT11_BEACON)
        {
            switchPkts = FALSE;
        }
    }

    if (switchPkts)
    {
        if (ac[dot11->currentACIndex].m_frameInfo)
        {
            dot11->dot11TxFrameInfo
                = ac[dot11->currentACIndex].m_frameInfo;
            dot11->currentMessage
                = ac[dot11->currentACIndex].m_frameInfo->msg;
            dot11->ipNextHopAddr
                = ac[dot11->currentACIndex].m_frameInfo->RA;
            if (MacDot11IsBssStation(dot11)&&
                (dot11->bssAddr!= dot11->selfAddr))
            {
                dot11->currentNextHopAddress = dot11->bssAddr;
            }
            else
            {
                dot11->currentNextHopAddress
                    = ac[dot11->currentACIndex].m_frameInfo->RA;
            }
            dot11->BO = ac[dot11->currentACIndex].m_BO;
            dot11->SSRC = ac[dot11->currentACIndex].m_QSRC ;
            dot11->SLRC = ac[dot11->currentACIndex].m_QLRC ;
        }
        else
        {
            assert(dot11->instantMessage);
        }
    }
}

/// \brief Function to set the current message
///
/// Set current message function will set the dot11->currentMessage with the message
/// which won in the Access category contention
///
/// \param dot11 Dot11 Structure pointer
/// \param acIndex Index of Ac who won the contention
void Dot11nAcManager::setCurrentMessage(MacDataDot11* dot11, UInt8 acIndex)
{
    BOOL switchPkts = TRUE;
    if (dot11->currentMessage)
    {
        assert(dot11->dot11TxFrameInfo);
        if (dot11->dot11TxFrameInfo->frameType == DOT11_BEACON)
        {
            switchPkts = FALSE;
        }
    }

    if (switchPkts)
    {
        if (!ac[acIndex].m_frameInfo)
        {
            if (!ac[acIndex].isAcHasPacket())
            {
                assert(dot11->instantMessage);
            }
            else
            {
                MacDot11nSetMessageInAC(node(), dot11, acIndex);
            }
        }

        if (ac[acIndex].m_frameInfo)
        {
            dot11->dot11TxFrameInfo
                = ac[acIndex].m_frameInfo;
            dot11->currentMessage
                = ac[acIndex].m_frameInfo->msg;
            dot11->ipNextHopAddr
                = ac[acIndex].m_frameInfo->RA;
            if (MacDot11IsBssStation(dot11)&&
                (dot11->bssAddr!= dot11->selfAddr))
            {
                dot11->currentNextHopAddress = dot11->bssAddr;
            }
            else
            {
                dot11->currentNextHopAddress
                    = ac[acIndex].m_frameInfo->RA;
            }
            dot11->BO = ac[acIndex].m_BO;
            dot11->SSRC = ac[acIndex].m_QSRC ;
            dot11->SLRC = ac[acIndex].m_QLRC ;
        }
    }
}

/// \brief Dequeue packet from Mac Queue
///
/// This function will dequeue packet from the Mac Queue and
/// set it in ac[dot11->currentAcIndex].m_frameInfo
///
/// \param dot11 Dot11 Structure pointer
void LegacyAcManager::dqFromMac(MacDataDot11* dot11)
{
    if ((ac[0].m_frameInfo == NULL) &&
        !(ac[0].frameInfoList.empty()))
    {
        DOT11_FrameInfo* frameInfo =
            (DOT11_FrameInfo*) MEM_malloc(sizeof(DOT11_FrameInfo));
        memset(frameInfo, 0, sizeof(DOT11_FrameInfo));
        DOT11_FrameInfo* tmpFrameInfo = ac[0].frameInfoList.front();

        *frameInfo = *tmpFrameInfo;
        ac[0].m_frameInfo = frameInfo;
        ac[0].m_frameToSend = frameInfo->msg;
        ac[0].m_priority = MAC_GetPacketsPriority(node(), frameInfo->msg);
        ac[0].frameInfoList.pop_front();
    }
}

/// \brief Dequeue packet from Mac Queue
///
/// This function will dequeue packet from the Mac Queue and
/// set it in ac[dot11->currentAcIndex].m_frameInfo
///
/// \param dot11 Dot11 Structure pointer
void Dot11eAcManager::dqFromMac(MacDataDot11* dot11)
{
    for (int i = 0; i < 4; i++)
    {
        if ((ac[i].m_frameInfo == NULL) &&
            !(ac[i].frameInfoList.empty()))
        {
            DOT11_FrameInfo* frameInfo =
                (DOT11_FrameInfo*) MEM_malloc(sizeof(DOT11_FrameInfo));
            memset(frameInfo, 0, sizeof(DOT11_FrameInfo));
            DOT11_FrameInfo* tmpFrameInfo = ac[i].frameInfoList.front();

            *frameInfo = *tmpFrameInfo;
            ac[i].m_frameInfo = frameInfo;
            ac[i].m_frameToSend = frameInfo->msg;
            ac[i].m_priority = MAC_GetPacketsPriority(node(), frameInfo->msg);
            ac[i].frameInfoList.pop_front();
        }
    }
}

/// \brief Dequeue packet from Mac Queue
///
/// This function will dequeue packet from the Mac Queue and
/// set it in ac[dot11->currentAcIndex].m_frameInfo
///
/// \param dot11 Dot11 Structure pointer
void Dot11nAcManager::dqFromMac(MacDataDot11* dot11)
{
    // To be Implemented
}

/// \brief Set More Fields in Ac
///
/// This is a function to set field in AC
///
/// \param acIndex Ac index
void Dot11eAcManager::setMoreFrameField(UInt8 acIndex)
{
    if (ac[acIndex].m_frameInfo || ac[acIndex].frameInfoList.size() > 0)
    {
        ac[acIndex].setIsAcHasPacket(TRUE);
    }
    else
    {
        ac[acIndex].setIsAcHasPacket(FALSE);
    }
}
