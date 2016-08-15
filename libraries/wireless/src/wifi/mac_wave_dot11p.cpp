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
// use in compliance with the license agreement as part of the EXata
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

//--------------------------------------------------------------------------
//                             WAVE 802.11p Qualnet Extension
//--------------------------------------------------------------------------
//
//These files extend Qualnet 4.0 to include the physical and link layers
//of the WAVE 802.11p protocols.  You are welcome to use this code for
//research, education, or commercial purposes.If you use this code as part
//of a research project, please cite the following paper in your manuscripts.
//(NOTE THAT THIS CITATION MAY CHANGE AFTER A MORE APPROPRIATE ONE IS
// PUBLISHED):
//Blum, J., Neiswender, A.5, and Eskandarian, A.1, "Denial of Service Attacks
//on Inter-Vehicle Networks,"
//IEEE Intelligent Transportation Systems Conference, pp. 797-802, Oct 2008.
//--------------------------------------------------------------------------


//Scalable Network Technologies has ported the code from QualNet 4.0
//We have resolved the following issues in the code while porting:

//1)Before start of a transmission, sender should verify if there is
//adequate time to transmit before the start of guard interval. If not,
//then transmission is deferred.
//2) Transmission initiation is stopped in the guard interval.
//3) Recpetion of packet is stopped in the guard interval.
//4) Test the model and fixes the bug found in the model.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>

#include "api.h"
#include "mac_dot11.h"
#include "mac_dot11-sta.h"
#include "phy_802_11.h"

#define DEBUG_WAVEDOT11P 0


/// \brief Function to manage SyncInterval
///
/// This function manages the syncInterval that coordinates
/// multi-channel access
///
/// \param node      Pointer to Node
/// \param dot11     Pointer to 802.11 structure
/// \param msg       Interval Message
void Mac802_11p_SyncInterval(Node* node, MacDataDot11* dot11, Message *msg)
{
    short nextEvent = 0;
    clocktype delay = 0;
    Message* newMsg = NULL;

    clocktype currentTime = node->getNodeTime();

    switch (msg->eventType)
    {
        case MSG_MAC_802_11p_CCH_Guard_Interval:
        {
            nextEvent = MSG_MAC_802_11p_CCH_Interval;

            // calculating duration of CCH interval
            delay = dot11->cchIntervalDuration -
                                        dot11->guardIntervalDuration;

            // calculating the start and end time of next guard interval
            dot11->guardIntervalStartTime = node->getNodeTime() + delay;
            dot11->guardIntervalEndTime = dot11->guardIntervalStartTime
                                          + dot11->guardIntervalDuration;

            Phy802_11ChangeState(node,
                                 dot11->myMacData->phyNumber,
                                 PHY_IDLE);
            MacDot11StationSetBackoffIfZero(node, dot11,
                                                dot11->currentACIndex);
            MacDot11StationPhyStatusIsNowIdleStartSendTimers(
                        node,
                        dot11);

            break;
        }
        case MSG_MAC_802_11p_CCH_Interval:
        {
            if (!dot11->dualBand)
            {
                nextEvent = MSG_MAC_802_11p_CCH_Guard_Interval;
            }
            else
            {
                nextEvent = MSG_MAC_802_11p_SCH_Guard_Interval;
            }
            delay = dot11->guardIntervalDuration;

            BOOL frameHeaderHadError;
            clocktype endSignalTime;

            if (PHY_GetStatus(node, dot11->myMacData->phyNumber) ==
                                            PHY_RECEIVING)
            {
                Phy802_11TerminateCurrentReceive(
                    node,
                    dot11->myMacData->phyNumber,
                    FALSE,
                    &frameHeaderHadError,
                    &endSignalTime);
            }

            Phy802_11ChangeState(node,
                                 dot11->myMacData->phyNumber,
                                 PHY_TRX_OFF);
            MacDot11StationCancelSomeTimersBecauseChannelHasBecomeBusy(node,
                                                                   dot11);
            break;
        }
        case MSG_MAC_802_11p_SCH_Guard_Interval:
        {
            nextEvent = MSG_MAC_802_11p_SCH_Interval;

            // calculating duration of SCH interval
            delay = dot11->schIntervalDuration -
                            dot11->guardIntervalDuration;

            // calculating the start and end time of next guard interval
            dot11->guardIntervalStartTime = node->getNodeTime() + delay;
            dot11->guardIntervalEndTime = dot11->guardIntervalStartTime
                                            + dot11->guardIntervalDuration;

            Phy802_11ChangeState(node,
                                 dot11->myMacData->phyNumber,
                                 PHY_IDLE);
            MacDot11StationSetBackoffIfZero(node, dot11,
                                            dot11->currentACIndex);
            MacDot11StationPhyStatusIsNowIdleStartSendTimers(
                    node,
                    dot11);

            break;
        }
        case MSG_MAC_802_11p_SCH_Interval:
        {
            BOOL frameHeaderHadError;
            clocktype endSignalTime;

            if (!dot11->dualBand)
            {
                nextEvent = MSG_MAC_802_11p_SCH_Guard_Interval;
            }
            else
            {
                nextEvent = MSG_MAC_802_11p_CCH_Guard_Interval;
            }

            delay = dot11->guardIntervalDuration;

            if (PHY_GetStatus(node, dot11->myMacData->phyNumber) ==
                                                    PHY_RECEIVING)
            {
                Phy802_11TerminateCurrentReceive(node,
                                                dot11->myMacData->phyNumber,
                                                FALSE,
                                                &frameHeaderHadError,
                                                &endSignalTime);
            }

            Phy802_11ChangeState(node,
                                 dot11->myMacData->phyNumber,
                                 PHY_TRX_OFF);
            MacDot11StationCancelSomeTimersBecauseChannelHasBecomeBusy(node,
                                                                   dot11);
            break;
        }
        default:
        {
            ERROR_ReportError("Mac802_11p_SyncInterval: "
                "Invalid value for event type.");
        }
    }
    newMsg = MESSAGE_Alloc(node, MAC_LAYER, MAC_PROTOCOL_DOT11, nextEvent);
    MESSAGE_SetInstanceId(newMsg, (short) dot11->myMacData->interfaceIndex);
    MESSAGE_InfoAlloc(node, newMsg, sizeof(int));
    ERROR_Assert(MESSAGE_ReturnInfo(newMsg) != NULL, "Info cann't be NULL");
    memcpy(MESSAGE_ReturnInfo(newMsg),
           &dot11->timerSequenceNumber,
           sizeof(int));

    MESSAGE_Send(node, newMsg, delay);
    MESSAGE_Free(node, msg);
}

/// \brief Initialization function for 802.11P
///
/// This function initializes various parameters of 802.11P protocol
///
/// \param node            Pointer to Node
/// \param interfaceIndex  Interface on which 802.11P is configured
/// \param nodeInput       Pointer to node input
/// \param phyModel        Phy Model configured on the interface
/// \param dot11           Pointer to 802.11 structure
void Mac802_11p_Init(Node *node,
                     unsigned interfaceIndex,
                     const NodeInput* nodeInput,
                     PhyModel phyModel,
                     MacDataDot11* dot11)
{
    clocktype aClockInput = 0;
    BOOL wasFound = FALSE;
    short nextEvent = 0;
    int phyIndex = dot11->myMacData->phyNumber;

    dot11->cwMin = M802_11p_CW_MIN;
    dot11->cwMax = M802_11p_CW_MAX;
    dot11->slotTime = M802_11p_SLOT_TIME;
    dot11->sifs = M802_11p_SIFS;
    dot11->delayUntilSignalAirborn = M802_11p_DELAY_UNTIL_SIGNAL_AIRBORN;

    ERROR_Assert(node->phyData[phyIndex]->phyModel == PHY802_11pCCH ||
            node->phyData[phyIndex]->phyModel == PHY802_11pSCH,
            "MAC802.11p requires PHY802.11pCCH or PHY802_11pSCH PHY model");

    IO_ReadTime(
        node,
        node->nodeId,
        interfaceIndex,
        nodeInput,
        "MAC-802.11-GUARD-INTERVAL-DURATION",
        &wasFound,
        &aClockInput);

    if (wasFound)
    {
        dot11->guardIntervalDuration = aClockInput;

        ERROR_Assert(dot11->guardIntervalDuration > 0,"Mac802_11Init: "
            "Value of MAC-802.11-GUARD-INTERVAL-DURATION should be greater"
            " than zero.\n");
    }
    else
    {
        dot11->guardIntervalDuration = M802_11p_GUARD_INTERVAL_DURATION;
    }

    IO_ReadTime(
        node,
        node->nodeId,
        interfaceIndex,
        nodeInput,
        "MAC-802.11-CCH-INTERVAL-DURATION",
        &wasFound,
        &aClockInput);

    if (wasFound)
    {
        dot11->cchIntervalDuration = aClockInput;
    }
    else
    {
        dot11->cchIntervalDuration = M802_11p_CCH_INTERVAL_DURATION;
    }

    ERROR_Assert(dot11->cchIntervalDuration > dot11->guardIntervalDuration,
        "Mac802_11Init: "
        "Value of MAC-802.11-CCH-INTERVAL-DURATION "
        "should be greater than the Guard Interval Duration.\n");

    IO_ReadTime(
        node,
        node->nodeId,
        interfaceIndex,
        nodeInput,
        "MAC-802.11-SCH-INTERVAL-DURATION",
        &wasFound,
        &aClockInput);

    if (wasFound)
    {
        dot11->schIntervalDuration = aClockInput;
    }
    else
    {
        dot11->schIntervalDuration = M802_11p_SCH_INTERVAL_DURATION;
    }
    ERROR_Assert(dot11->schIntervalDuration > dot11->guardIntervalDuration,
       "Mac802_11Init: "
       "Value of MAC-802.11-SCH-INTERVAL-DURATION "
       "should be greater than the Guard Interval Duration.\n");

    dot11->dualBand = FALSE;

    // Begin the first guard interval
    if (phyModel == PHY802_11pCCH)
    {
        nextEvent = MSG_MAC_802_11p_CCH_Guard_Interval;
    }
    else
    {
        nextEvent = MSG_MAC_802_11p_SCH_Guard_Interval;
    }

    Phy802_11ChangeState(node, dot11->myMacData->phyNumber, PHY_TRX_OFF);

    dot11->guardIntervalStartTime = node->getNodeTime();
    dot11->guardIntervalEndTime = dot11->guardIntervalStartTime +
                                    dot11->guardIntervalDuration;

    Message* newMsg = MESSAGE_Alloc(node,
                                    MAC_LAYER,
                                    MAC_PROTOCOL_DOT11,
                                    nextEvent);

    MESSAGE_SetInstanceId(newMsg, (short) dot11->myMacData->interfaceIndex);

    MESSAGE_InfoAlloc(node, newMsg, sizeof(int));
    *((int*)(MESSAGE_ReturnInfo(newMsg))) = dot11->timerSequenceNumber;

    MESSAGE_Send(node, newMsg, dot11->guardIntervalDuration);
}


/// \brief EDCA parameter initialization function for 802.11P
///
/// This function initialize EDCA parameters of 802.11P protocol
///
/// \param node            Pointer to Node
/// \param dot11           Pointer to 802.11 structure
/// \param phyModel        Phy Model configured on the interface
void Mac802_11p_AcInit(Node* node,
                       MacDataDot11* dot11,
                       PhyModel phyModel)
{
    if (phyModel == PHY802_11pCCH)
    {
        // Access Category 0
        AccessCategory& ac = getAc(dot11, DOT11e_AC_BK);
        ac.m_cwMin = M802_11p_CW_MIN;
       
        ac.m_cwMax = M802_11p_CW_MAX;

        ac.m_slotTime = M802_11p_SLOT_TIME;

        // Arbitary Inter space number
        ac.m_AIFSN = DOT11e_802_11p_CCH_AC_BK_AIFSN;

        // Transmission opportunity limit
        ac.m_TXOPLimit = DOT11e_802_11p_CCH_AC_BK_TXOPLimit;

        ac.m_AIFS = dot11->sifs +
                   ac.m_AIFSN * getAc(dot11,0).m_slotTime;

        // Access Category 1
        // SOURCE:  IEEE Trial-Use Standard for Wireless Access in Vehicular
        //          Environments (WAVE)- Multichannel Operation, p.12
        AccessCategory& ac1 = getAc(dot11,DOT11e_AC_BE);
        ac1.m_cwMin = ((M802_11p_CW_MIN + 1) / 2) - 1;
        ac1.m_cwMax = M802_11p_CW_MIN;
        ac1.m_slotTime = M802_11p_SLOT_TIME;

        // Arbitary Inter space number
        ac1.m_AIFSN = DOT11e_802_11p_CCH_AC_BE_AIFSN;

        // Transmission opportunity limit
        ac1.m_TXOPLimit = DOT11e_802_11p_CCH_AC_BE_TXOPLimit;

        ac1.m_AIFS = dot11->sifs +
                    ac.m_AIFSN * dot11->slotTime;

        // Access Category 2
        AccessCategory& ac2 = getAc(dot11, DOT11e_AC_VI);

        ac2.m_cwMin = ((M802_11p_CW_MIN + 1) / 4) - 1;
        ac2.m_cwMax = ((M802_11p_CW_MIN + 1) / 2) - 1;
        ac2.m_slotTime = M802_11p_SLOT_TIME;

        // Arbitary Inter space number
        ac2.m_AIFSN = DOT11e_802_11p_CCH_AC_VI_AIFSN;

        // Transmission opportunity limit
        ac2.m_TXOPLimit = DOT11e_802_11p_CCH_AC_VI_TXOPLimit;

        ac2.m_AIFS = dot11->sifs +
                    ac2.m_AIFSN * getAc(dot11, 2).m_slotTime;

        // Access Category 3
        AccessCategory& ac3 = getAc(dot11, DOT11e_AC_VO);

        ac3.m_cwMin = ((M802_11p_CW_MIN + 1) / 4) - 1;
        ac3.m_cwMax = ((M802_11p_CW_MIN + 1) / 2) - 1;
        ac3.m_slotTime = M802_11p_SLOT_TIME;

        // Arbitary Inter space number
        ac3.m_AIFSN = DOT11e_802_11p_CCH_AC_VO_AIFSN;

        // Transmission opportunity limit
        ac3.m_TXOPLimit = DOT11e_802_11p_CCH_AC_VO_TXOPLimit;
        ac3.m_AIFS = dot11->sifs +
                    ac3.m_AIFSN * getAc(dot11, 3).m_slotTime;
    }
    else if (phyModel == PHY802_11pSCH)
    {
        // Access Category 0
        AccessCategory& ac = getAc(dot11, DOT11e_AC_BK);
        ac = getAc(dot11, DOT11e_AC_BK);
        ac.m_cwMin = M802_11p_CW_MIN;
        ac.m_cwMax = M802_11p_CW_MAX;
        ac.m_slotTime = M802_11p_SLOT_TIME;

        // Arbitary Inter space number
        ac.m_AIFSN = DOT11e_802_11p_SCH_AC_BK_AIFSN;

        // Transmission opportunity limit
        ac.m_TXOPLimit = DOT11e_802_11p_SCH_AC_BK_TXOPLimit;

        ac.m_AIFS = dot11->sifs +
                   ac.m_AIFSN * getAc(dot11, 0).m_slotTime;

        // Access Category 1
        AccessCategory& ac1 = getAc(dot11, DOT11e_AC_BE);
        ac1.m_cwMin = M802_11p_CW_MIN;
        ac1.m_cwMax = M802_11p_CW_MAX;
        ac1.m_slotTime = M802_11p_SLOT_TIME;

        // Arbitary Inter space number
        ac1.m_AIFSN = DOT11e_802_11p_SCH_AC_BE_AIFSN;

        // Transmission opportunity limit
        ac1.m_TXOPLimit = DOT11e_802_11p_SCH_AC_BE_TXOPLimit;
        ac1.m_AIFS = dot11->sifs +
                        ac1.m_AIFSN * dot11->slotTime;

        // Access Category 2
        AccessCategory& ac2 = getAc(dot11, DOT11e_AC_VI);
        ac2.m_cwMin = ((M802_11p_CW_MIN + 1) / 2) - 1;
        ac2.m_cwMax = M802_11p_CW_MIN;
        ac2.m_slotTime = M802_11p_SLOT_TIME;

        // Arbitary Inter space number
        ac2.m_AIFSN = DOT11e_802_11p_SCH_AC_VI_AIFSN;

        // Transmission opportunity limit
        ac2.m_TXOPLimit = DOT11e_802_11p_SCH_AC_VI_TXOPLimit;
        ac2.m_AIFS = dot11->sifs +
                    ac2.m_AIFSN * getAc(dot11, 2).m_slotTime;

        // Access Category 3
        AccessCategory& ac3 = getAc(dot11, DOT11e_AC_VO);
        ac3.m_cwMin = ((M802_11p_CW_MIN + 1) / 4) - 1;
        ac3.m_cwMax = ((M802_11p_CW_MIN + 1) / 2) - 1;
        ac3.m_slotTime = M802_11p_SLOT_TIME;

        // Arbitary Inter space number
        ac3.m_AIFSN = DOT11e_802_11p_SCH_AC_VO_AIFSN;

        // Transmission opportunity limit
        ac3.m_TXOPLimit = DOT11e_802_11p_SCH_AC_VO_TXOPLimit;
        ac3.m_AIFS = dot11->sifs +
                    ac3.m_AIFSN * getAc(dot11, 3).m_slotTime;
    }

    for (int acCounter = 0; acCounter < DOT11e_NUMBER_OF_AC; acCounter++)
    {
        AccessCategory& accessCategory = getAc(dot11, acCounter);
        accessCategory.m_BO = 0;
        accessCategory.m_lastBOTimeStamp = 0;
        accessCategory.m_frameToSend = NULL;
        accessCategory.m_currentNextHopAddress = INVALID_802ADDRESS;
        accessCategory.m_totalNoOfthisTypeFrameQueued = 0;
        accessCategory.m_totalNoOfthisTypeFrameDeQueued = 0;
    }
}

/// \brief Function to check if packet transmission is possible before guard
/// interval
///
/// The function is used to check if transmission can finish
//  before the guard interval start time
///
/// \param node            Pointer to Node
/// \param dot11           Pointer to 802.11 structure
/// \param dataRateType    Data rate used to transmit
/// \param frameType       frame type to be transmitted
/// \param hdrDelay        transmission duration
///
/// \return True if transmission is possible False otherwise
BOOL Mac802_11p_CanStartTransmitting(
    Node* node,
    MacDataDot11* dot11,
    int dataRateType,
    DOT11_MacFrameType frameType,
    clocktype hdrDelay)
{
    clocktype delay = 0;
    clocktype packetTransmissionDuration = 0;
    clocktype transmissionEndTime = 0;

    int index = dot11->myMacData->phyNumber;

    if (node->phyData[index]->phyModel != PHY802_11pCCH
       && node->phyData[index]->phyModel != PHY802_11pSCH)
    {
        return TRUE;
    }

    if (frameType == DOT11_RTS && dot11->currentMessage == NULL)
    {
        return FALSE;
    }

    switch (frameType)
    {
        case DOT11_CTS:
        {
            delay = hdrDelay;
            break;
        }
        case DOT11_ACK:
        {
            delay = dot11->extraPropDelay +
                    dot11->sifs +
                    PHY_GetTransmissionDuration(
                        node, dot11->myMacData->phyNumber,
                        dataRateType,
                        DOT11_SHORT_CTRL_FRAME_SIZE);

            break;
        }
        case DOT11_DATA:
        {
            packetTransmissionDuration = PHY_GetTransmissionDuration(
                                    node,
                                    dot11->myMacData->phyNumber,
                                    dataRateType,
                                    MESSAGE_ReturnPacketSize(
                                        MacDot11StationGetCurrentMessage(
                                                                   dot11)));
            delay = dot11->extraPropDelay +
                dot11->sifs +
                packetTransmissionDuration +
                dot11->extraPropDelay +
                dot11->sifs +
                PHY_GetTransmissionDuration(
                        node, dot11->myMacData->phyNumber,
                        dataRateType,
                        DOT11_SHORT_CTRL_FRAME_SIZE);
            break;
        }
        case DOT11_RTS:
        default:
        {
            packetTransmissionDuration = PHY_GetTransmissionDuration(
                                    node,
                                    dot11->myMacData->phyNumber,
                                    dataRateType,
                                    MESSAGE_ReturnPacketSize(
                                        MacDot11StationGetCurrentMessage(
                                                                    dot11)));
            delay = dot11->extraPropDelay +
                dot11->sifs +
                PHY_GetTransmissionDuration(
                        node, dot11->myMacData->phyNumber,
                        dataRateType,
                        DOT11_SHORT_CTRL_FRAME_SIZE) +
                dot11->extraPropDelay +
                dot11->sifs +
                packetTransmissionDuration +
                dot11->extraPropDelay +
                dot11->sifs +
                PHY_GetTransmissionDuration(
                        node, dot11->myMacData->phyNumber,
                        dataRateType,
                        DOT11_SHORT_CTRL_FRAME_SIZE) +
                dot11->extraPropDelay +
                dot11->sifs +
                PHY_GetTransmissionDuration(
                        node, dot11->myMacData->phyNumber,
                        dataRateType,
                        DOT11_SHORT_CTRL_FRAME_SIZE);
            break;
        }
    }

    transmissionEndTime = delay + node->getNodeTime();

    if (DEBUG_WAVEDOT11P)
    {
        char clockStr[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(dot11->guardIntervalStartTime, clockStr);
        printf(" Guard Start Time %s", clockStr);
        TIME_PrintClockInSecond(dot11->guardIntervalEndTime, clockStr);
        printf(" Guard End Time %s", clockStr);
        TIME_PrintClockInSecond(node->getNodeTime(), clockStr);
        printf(" Transmission Start Time %s", clockStr);
        TIME_PrintClockInSecond(transmissionEndTime, clockStr);
        printf(" Transmission End time %s\n", clockStr );
    }

    // check if transmission end can occur before guard start time.
    // check to stop start transmitting in between the guard.
    if ((transmissionEndTime > dot11->guardIntervalStartTime &&
        node->getNodeTime() <= dot11->guardIntervalStartTime)||
        (node->getNodeTime() >= dot11->guardIntervalStartTime &&
        node->getNodeTime() < dot11->guardIntervalEndTime))
    {
        if (DEBUG_WAVEDOT11P)
        {
            printf("Node %d Can Not Transmit\n", node->nodeId);
        }
        return FALSE;
    }

    if (DEBUG_WAVEDOT11P)
    {
        printf("Node %d Can Transmit\n", node->nodeId);
    }
    return TRUE;
}
