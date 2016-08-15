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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "api.h"
#include "antenna.h"
#include "phy_dot16.h"
#include "mac_dot16_phy.h"
#include "mac_dot16_bs.h"
#include "mac_dot16_ss.h"
#include "mac_dot16.h"
#include "mac_dot16_cs.h"

#define DEBUG          0
#define DEBUG_DETAIL   0
#define DEBUG_SNR      0
#define DEBUG_CDMA_SNR 0

// Print out the maintained info of a receiving signal or
// interference signal for debug purpose.
//
// \param node  Pointer to node.
// \param rxMsgInfo  Pointer to info of a signal
//

static
void PhyDot16PrintRxMsgInfo(Node* node, PhyDot16RxMsgInfo* rxMsgInfo)
{
    PhyDot16OfdmaBurst* ofdmaBurst;
    char clockStr1[MAX_STRING_LENGTH];
    char clockStr2[MAX_STRING_LENGTH];
    int i;

    printf("Node%d receiving msg information of the signal\n",
           node->nodeId);
    printf("    isDownlink = %d\n", rxMsgInfo->isDownlink);
    printf("    startSubchannel = %d\n", rxMsgInfo->startSubchannel);
    printf("    endSubchannel = %d\n", rxMsgInfo->endSubchannel);

    for (i = rxMsgInfo->startSubchannel; i <= rxMsgInfo->endSubchannel; i++)
    {
        ctoa(rxMsgInfo->subchannelStartTime[i], clockStr1);
        ctoa(rxMsgInfo->subchannelEndTime[i], clockStr2);
        printf("      subchannel %d: startTime = %s, endTime = %s\n",
               i, clockStr1, clockStr2);
    }

    printf("    numBursts = %d\n", rxMsgInfo->numBursts);
    ofdmaBurst = rxMsgInfo->ofdmaBurst;
    while (ofdmaBurst != NULL)
    {
        printf("      burst %d: modCodeType = %d, startSubchannel = %d, "
               "endSubchannel %d, symbolOffset = %d, numSymbols = %d\n",
               i, ofdmaBurst->modCodeType, ofdmaBurst->startSubchannel,
               ofdmaBurst->endSubchannel, ofdmaBurst->symbolOffset,
               ofdmaBurst->numSymbols);

        ofdmaBurst = ofdmaBurst->next;
    }
}

// To change the phy status
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param newStatus  New phy status
//
static
void PhyDot16ChangeState(
                       Node* node,
                       int phyIndex,
                       PhyStatusType newStatus)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16 *)thisPhy->phyVar;

    phyDot16->previousMode = phyDot16->mode;
    phyDot16->mode = newStatus;

    //added for energy model
    Phy_ReportStatusToEnergyModel(node, phyIndex,
                                  phyDot16->previousMode,
                                  phyDot16->mode);
}// end of PhyDot16ChangePhyStatus

// Retrive the burst information from the signal and create
// a structure to store them
//
// \param node  Pointer to node.
// \param phyDot16  Pointer to the 802.16 PHY structure
// \param propRxInfo  Information of the arrived signal
// \param rxPower_mW  Receiving power of the signal
//
// \return Created structure keepting burst info

static
PhyDot16RxMsgInfo* PhyDot16CreateRxMsgInfo(
                       Node* node,
                       PhyDataDot16* phyDot16,
                       PropRxInfo* propRxInfo,
                       double rxPower_mW)
{
    PhyDot16PlcpHeader* plcpHeader;
    PhyDot16RxMsgInfo* rxMsgInfo;
    PhyDot16OfdmaBurst* ofdmaBurst;
    Dot16BurstInfo* burstInfo;
    int numBursts;

    Message* msgList;
    Message* tmpMsg;
    Message* lastBurstPdu;

    clocktype relativeStartTime;
    clocktype currentTime;
    clocktype startTime;
    clocktype endTime;
    int i;

    rxMsgInfo = (PhyDot16RxMsgInfo*) MEM_malloc(sizeof(PhyDot16RxMsgInfo));
    ERROR_Assert(rxMsgInfo != NULL, "PHY802.16: Out of memory!");
    memset(rxMsgInfo, 0, sizeof(PhyDot16RxMsgInfo));

    // get pseudo PHY header
    plcpHeader = (PhyDot16PlcpHeader*)
                 MESSAGE_ReturnPacket(propRxInfo->txMsg);

    currentTime = propRxInfo->rxStartTime;

    rxMsgInfo->propRxInfo = propRxInfo;
    rxMsgInfo->rxMsg = propRxInfo->txMsg;
    rxMsgInfo->rxBeginTime = currentTime;
    rxMsgInfo->rxEndTime = currentTime + plcpHeader->duration;

    rxMsgInfo->isDownlink = plcpHeader->isDownlink;

    MESSAGE_RemoveHeader(node,
                         propRxInfo->txMsg,
                         sizeof(PhyDot16PlcpHeader),
                         TRACE_PHY_DOT16);

    msgList = MESSAGE_UnpackMessage(node, propRxInfo->txMsg, FALSE, FALSE);

    // put back the header, it is still there
    MESSAGE_AddHeader(node,
                      propRxInfo->txMsg,
                      sizeof(PhyDot16PlcpHeader),
                      TRACE_PHY_DOT16);

    int num_Subchannels;

    if (plcpHeader->isDownlink)
    {
        num_Subchannels = phyDot16->downlink_numSubchannels;
    }
    else
    {
        num_Subchannels = phyDot16->uplink_numSubchannels;
    }

    rxMsgInfo->startSubchannel = (UInt8)num_Subchannels;
    rxMsgInfo->endSubchannel = 0;
    endTime = currentTime + propRxInfo->duration;
    for (i = 0; i < MAX_NUM_SUBCHANNELS; i ++)
    {
        rxMsgInfo->subchannelStartTime[i] = endTime;
        rxMsgInfo->subchannelEndTime[i] = currentTime;
    }

    // get the information of bursts
    numBursts = 0;
    tmpMsg = msgList;
    lastBurstPdu = NULL;
    while (tmpMsg != NULL)
    {
        burstInfo = (Dot16BurstInfo*)
                    MESSAGE_ReturnInfo(tmpMsg, INFO_TYPE_Dot16BurstInfo);
        if (burstInfo != NULL)
        {
            // a new burst

            if (lastBurstPdu != NULL)
            {
                lastBurstPdu->next = NULL;
            }

            // alloc and store information of the new burst
            ofdmaBurst = (PhyDot16OfdmaBurst*)
                         MEM_malloc(sizeof(PhyDot16OfdmaBurst));
            ERROR_Assert(ofdmaBurst != NULL, "PHY802.16: Out of memory!");
            memset(ofdmaBurst, 0, sizeof(PhyDot16OfdmaBurst));

            numBursts ++;

            ofdmaBurst->pduMsgList = tmpMsg;
            ofdmaBurst->modCodeType = burstInfo->modCodeType;
            ofdmaBurst->startSubchannel = burstInfo->subchannelOffset;
            ofdmaBurst->endSubchannel = (UInt8)(burstInfo->subchannelOffset
                + burstInfo->numSubchannels - 1);
            ofdmaBurst->symbolOffset = burstInfo->symbolOffset;
            ofdmaBurst->numSymbols = burstInfo->numSymbols;

            ofdmaBurst->signalType = burstInfo->signalType;
            ofdmaBurst->cdmaCodeIndex = burstInfo->cdmaCodeIndex;
            ofdmaBurst->cdmaSpreadingFactor = burstInfo->cdmaSpreadingFactor;
            if (rxMsgInfo->isDownlink)
            {
                // downlink burst
                ofdmaBurst->burstStartTime = currentTime +
                                             burstInfo->symbolOffset *
                                             phyDot16->ofdmSymbolDuration;
                ofdmaBurst->burstEndTime = ofdmaBurst->burstStartTime +
                                           burstInfo->numSymbols *
                                           phyDot16->ofdmSymbolDuration;
                ofdmaBurst->firstStartTime = ofdmaBurst->burstStartTime;
                ofdmaBurst->lastEndTime = ofdmaBurst->burstEndTime;
            }
            else
            {
                // uplink burst
                ofdmaBurst->burstStartTime = currentTime;
                if (burstInfo->numSubchannels == 1)
                {
                    ofdmaBurst->burstEndTime = currentTime +
                                               burstInfo->numSymbols *
                                               phyDot16->ofdmSymbolDuration;
                    ofdmaBurst->firstStartTime = ofdmaBurst->burstStartTime;
                    ofdmaBurst->lastEndTime = ofdmaBurst->burstEndTime;
                }
                else
                {
                    ofdmaBurst->burstEndTime = currentTime +
                                               plcpHeader->duration;
                    relativeStartTime = burstInfo->symbolOffset *
                                        phyDot16->ofdmSymbolDuration;
                    ofdmaBurst->firstStartTime = currentTime +
                                                 relativeStartTime;
                    ofdmaBurst->lastEndTime =
                            currentTime + (burstInfo->numSymbols *
                            phyDot16->ofdmSymbolDuration +
                            relativeStartTime) % plcpHeader->duration;
                }
            }

            // add into the end of the burst list
            ofdmaBurst->next = NULL;
            if (rxMsgInfo->ofdmaBurst == NULL)
            {
                rxMsgInfo->ofdmaBurst = ofdmaBurst;
                rxMsgInfo->ofdmaBurstTail = ofdmaBurst;
            }
            else
            {
                rxMsgInfo->ofdmaBurstTail->next = ofdmaBurst;
                rxMsgInfo->ofdmaBurstTail = ofdmaBurst;
            }

            // update aggregated information stored in rxMsgInfo
            if (ofdmaBurst->startSubchannel < rxMsgInfo->startSubchannel)
            {
                rxMsgInfo->startSubchannel = ofdmaBurst->startSubchannel;
            }

            if (ofdmaBurst->endSubchannel > rxMsgInfo->endSubchannel)
            {
                rxMsgInfo->endSubchannel = ofdmaBurst->endSubchannel;
            }

            ERROR_Assert(rxMsgInfo->startSubchannel <
                         num_Subchannels &&
                         rxMsgInfo->endSubchannel < num_Subchannels,
                         "PHY802.16: Wrong subchannel info in OFDMA burst. "
                         "May due to different 802.16 PHY parameters at "
                         "transmitter and receiver.");

            int numSubchannels = 0;

            numSubchannels =
                rxMsgInfo->endSubchannel - rxMsgInfo->startSubchannel + 1;

            ERROR_Assert(numSubchannels > 0,
                "PHY802.16: Number of subchannels should be larger"
                " than zero!");

            double rxPower_mWsubchannel =
                    rxPower_mW / (double)numSubchannels;

            for (i = 0; i < MAX_NUM_SUBCHANNELS; i ++) {

                rxMsgInfo->rxMsgPower_mW[i] = 0.0;
            }

            for (i = rxMsgInfo->startSubchannel;
                 i <= rxMsgInfo->endSubchannel; i ++)
            {
                rxMsgInfo->rxMsgPower_mW[i] = rxPower_mWsubchannel;

                if (i == ofdmaBurst->startSubchannel)
                {
                    startTime = ofdmaBurst->firstStartTime;
                    endTime = ofdmaBurst->burstEndTime;
                }
                else if (i == ofdmaBurst->endSubchannel)
                {
                    startTime = ofdmaBurst->burstStartTime;
                    endTime = ofdmaBurst->lastEndTime;
                }
                else
                {
                    startTime = ofdmaBurst->burstStartTime;
                    endTime = ofdmaBurst->burstEndTime;
                }

                if (startTime < rxMsgInfo->subchannelStartTime[i])
                {
                    rxMsgInfo->subchannelStartTime[i] = startTime;
                }

                if (endTime > rxMsgInfo->subchannelEndTime[i])
                {
                    rxMsgInfo->subchannelEndTime[i] = endTime;
                }

                if (endTime > rxMsgInfo->rxEndTime)
                {
                    rxMsgInfo->rxEndTime = endTime;
                }
            }

            if (rxMsgInfo->isDownlink == FALSE)
            {
                // uplink only has one burst, so stop
                break;
            }
        }

        lastBurstPdu = tmpMsg;
        tmpMsg = tmpMsg->next;
    }

    rxMsgInfo->numBursts = numBursts;

    if (DEBUG_DETAIL)
    {
        PhyDot16PrintRxMsgInfo(node, rxMsgInfo);
    }

    return rxMsgInfo;
}

// Free the structure keeping info of a signal.
//
// \param node  Pointer to node.
// \param rxMsgInfo  Pointer to the struc of a signal
//
static
void PhyDot16FreeRxMsgInfo(Node* node, PhyDot16RxMsgInfo* rxMsgInfo)
{
    PhyDot16OfdmaBurst* ofdmaBurst;
    PhyDot16OfdmaBurst* nextBurst;

    // free the burst list
    ofdmaBurst = rxMsgInfo->ofdmaBurst;

    while (ofdmaBurst != NULL)
    {
        nextBurst = ofdmaBurst->next;

        if (ofdmaBurst->pduMsgList != NULL)
        {
            MESSAGE_FreeList(node, ofdmaBurst->pduMsgList);
        }

        MEM_free(ofdmaBurst);
        ofdmaBurst = nextBurst;
    }

    // free the rxMsgInfo itself
    MEM_free(rxMsgInfo);
}

// Find the info of a signal from the maintained list.
// If requested, may remove it from the list
//
// \param node  Pointer to node.
// \param rxMsgList  List of signal info structures
// \param rxMsg  Message of the signal as a key
// \param removeIt  Indicate whether remove from the list
//
// \return Pointer to the structure if found
static
PhyDot16RxMsgInfo* PhyDot16GetRxMsgInfo(Node* node,
                                        PhyDot16RxMsgInfo** rxMsgList,
                                        Message* rxMsg,
                                        BOOL removeIt)
{
    PhyDot16RxMsgInfo* rxMsgInfo;
    PhyDot16RxMsgInfo* prevMsgInfo;

    rxMsgInfo = *rxMsgList;
    prevMsgInfo = rxMsgInfo;
    while (rxMsgInfo != NULL)
    {
        if (rxMsgInfo->rxMsg == rxMsg)
        { // found it
            if (removeIt)
            {
                if (prevMsgInfo == rxMsgInfo)
                {
                    // first msg in the list
                    *rxMsgList = rxMsgInfo->next;
                }
                else
                {
                    prevMsgInfo->next = rxMsgInfo->next;
                }
            }

            break;
        }

        prevMsgInfo = rxMsgInfo;
        rxMsgInfo = rxMsgInfo->next;
    }

    return rxMsgInfo;
}

// Check if the new signal has overlap with existing signals
// under receiving already.
//
// \param node  Pointer to node.
// \param phyDot16  Pointer to the 802.16 PHY structure
// \param newRxMsgList  Information of new signal
//
// \return TRUE if has collision, FALSE otherwise
static
BOOL PhyDot16NoOverlap(Node* node,
                         PhyDataDot16* phyDot16,
                         PhyDot16RxMsgInfo* newRxMsgInfo)
{
    PhyDot16RxMsgInfo* rxMsgInfo;
    clocktype oldStartTime;
    clocktype oldEndTime;
    clocktype newStartTime;
    clocktype newEndTime;
    int startIndex;
    int endIndex;
    int i;

    rxMsgInfo = phyDot16->rxMsgList;

    // if no signal under receiving, return TRUE
    if (rxMsgInfo == NULL)
    {
        return TRUE;
    }

    // go through receiving signal list to compare with each of them
    while (rxMsgInfo != NULL)
    {
        if (rxMsgInfo->startSubchannel > newRxMsgInfo->startSubchannel)
        {
            startIndex = rxMsgInfo->startSubchannel;
        }
        else
        {
            startIndex = newRxMsgInfo->startSubchannel;
        }

        if (rxMsgInfo->endSubchannel < newRxMsgInfo->endSubchannel)
        {
            endIndex = rxMsgInfo->endSubchannel;
        }
        else
        {
            endIndex = newRxMsgInfo->endSubchannel;
        }

        for (i = startIndex; i <= endIndex; i ++)
        {
            oldStartTime = rxMsgInfo->subchannelStartTime[i];
            oldEndTime = rxMsgInfo->subchannelEndTime[i];
            newStartTime = newRxMsgInfo->subchannelStartTime[i];
            newEndTime = newRxMsgInfo->subchannelEndTime[i];

            // if overlap, then there is collision
            if (!(newStartTime >= oldEndTime || newEndTime <= oldStartTime))
            {
                return FALSE;
            }
        }

        rxMsgInfo = rxMsgInfo->next;
    }

    return TRUE;
}

BOOL PhyDot16IsSymbolOverlap(PhyDot16OfdmaBurst* ofdmaBurst1,
                       PhyDot16OfdmaBurst* ofdmaBurst2)
{
    BOOL result =  FALSE;
    UInt8 burst1StartSymbolOffset = ofdmaBurst1->symbolOffset;
    UInt8 burst2StartSymbolOffset = ofdmaBurst2->symbolOffset;
    UInt8 burst1EndSymbolOffset = ofdmaBurst1->symbolOffset
        + ofdmaBurst1->numSymbols - 1;
    UInt8 burst2EndSymbolOffset = ofdmaBurst2->symbolOffset
        + ofdmaBurst2->numSymbols - 1;
    if ((burst2StartSymbolOffset >= burst1StartSymbolOffset)
        && (burst2StartSymbolOffset <= burst1EndSymbolOffset))
        result = TRUE;

    if (!result && (burst2EndSymbolOffset >= burst1StartSymbolOffset)
        && (burst2EndSymbolOffset <= burst1EndSymbolOffset))
        result = TRUE;

    if (!result && (burst1StartSymbolOffset >= burst2StartSymbolOffset)
        && (burst1StartSymbolOffset <= burst2EndSymbolOffset))
        result = TRUE;

    if (!result && (burst1EndSymbolOffset >= burst2StartSymbolOffset)
        && (burst1EndSymbolOffset <= burst2EndSymbolOffset))
        result = TRUE;
    return result;
}// end of PhyDot16IsOverlap


// Check if a burst of the receiving signal has error.
//
// \param node  Pointer to node.
// \param phyDot16  Pointer to the 802.16 PHY structure
// \param rxMsgList  Info of signal containing the burst
// \param ofdmaBurst  Info of the OFDMA burst to be checked
//



static
void PhyDot16CheckBurstError(Node* node,
                             PhyDataDot16* phyDot16,
                             PhyDot16RxMsgInfo* rxMsgInfo,
                             PhyDot16OfdmaBurst* ofdmaBurst)
{
    BOOL needCheck = FALSE;
    clocktype currentTime;
    clocktype startTime;
    clocktype endTime;
    double sinr;
    double BER;
    int i;
    double signalPower = 0.0;
    double interferencePower = 0.0;
    double numBits = 0;
    double numBitsSubchannel = 0.0;
    double noisePower_mW = 0.0;

    double noisePowerPerSubchannel_mW = phyDot16->noisePower_mW /
                                        (rxMsgInfo->endSubchannel -
                                         rxMsgInfo->startSubchannel + 1);

    currentTime = node->getNodeTime();

    if (ofdmaBurst->modCodeType == PHY802_16_BPSK)
    {
        // go through all signals under receiving
        PhyDot16RxMsgInfo* tempRxMsgInfo = phyDot16->rxMsgList;
        PhyDot16RxMsgInfo* ifTempMsgInfo = phyDot16->ifMsgList;

        double spreadingFactor;

        signalPower +=
            (rxMsgInfo->rxMsgPower_mW[ofdmaBurst->startSubchannel] *
            PHY802_16_CDMA_RANGING_OCCUPIED_SUBCHANNELS);

        while (tempRxMsgInfo != NULL)
        {

            if (rxMsgInfo != tempRxMsgInfo)
            {
                if (PhyDot16IsSymbolOverlap(
                        rxMsgInfo->ofdmaBurst,
                        tempRxMsgInfo->ofdmaBurst))
                {
                    spreadingFactor = 1.0;

                    if (tempRxMsgInfo->ofdmaBurst->cdmaCodeIndex !=
                       ofdmaBurst->cdmaCodeIndex)
                    {
                        spreadingFactor = ofdmaBurst->cdmaSpreadingFactor;
                    }

                    interferencePower +=
                        ((tempRxMsgInfo->
                        rxMsgPower_mW[ofdmaBurst->startSubchannel]) *
                        PHY802_16_CDMA_RANGING_OCCUPIED_SUBCHANNELS
                        / spreadingFactor);
                }
            }

            tempRxMsgInfo = tempRxMsgInfo->next;
        }

        while (ifTempMsgInfo != NULL)
        {

            if (PhyDot16IsSymbolOverlap(
                    rxMsgInfo->ofdmaBurst,
                    ifTempMsgInfo->ofdmaBurst))
            {
                spreadingFactor = 1.0;

                if (ifTempMsgInfo->ofdmaBurst->cdmaCodeIndex !=
                   ofdmaBurst->cdmaCodeIndex)
                {
                    spreadingFactor = ofdmaBurst->cdmaSpreadingFactor;
                }

                interferencePower +=
                    ((ifTempMsgInfo->
                    rxMsgPower_mW[ofdmaBurst->startSubchannel]) *
                    PHY802_16_CDMA_RANGING_OCCUPIED_SUBCHANNELS
                    / spreadingFactor);
            }

            ifTempMsgInfo = ifTempMsgInfo->next;
       }

        sinr =
            signalPower / (interferencePower + noisePowerPerSubchannel_mW *
            PHY802_16_CDMA_RANGING_OCCUPIED_SUBCHANNELS);

        ofdmaBurst->inError = FALSE;

        if (DEBUG_CDMA_SNR)
        {
            printf("Node %d check burst error with\n"
                "   rxPower_mW            = %e\n"
                "   interference power mW = %e\n"
                "   sinr                  = %f\n",
                node->nodeId, signalPower, interferencePower, sinr);
        }

        if (sinr <= PHY802_16_CDMA_RANGING_THRESHOLD)
        {
            ofdmaBurst->inError = TRUE;
        }

        return;
    }// end of if for CDMA burst

    for (i = ofdmaBurst->startSubchannel;
         i <= ofdmaBurst->endSubchannel;
         i ++)
    {
        startTime = ofdmaBurst->burstStartTime;
        endTime = ofdmaBurst->burstEndTime;
        if (i == ofdmaBurst->startSubchannel)
        {
            startTime = ofdmaBurst->firstStartTime;
        }
        if (i == ofdmaBurst->endSubchannel)
        {
            endTime = ofdmaBurst->lastEndTime;
        }

        if (startTime < phyDot16->rxTimeEvaluated)
        {
            startTime = phyDot16->rxTimeEvaluated;
        }

        if (endTime > currentTime)
        {
            endTime = currentTime;
        }

        if (endTime > startTime) {

            needCheck = TRUE;
            signalPower += rxMsgInfo->rxMsgPower_mW[i];
            interferencePower += phyDot16->interferencePower_mW[i];
            noisePower_mW += noisePowerPerSubchannel_mW;

            numBitsSubchannel =
                phyDot16->uplinkNumDataBitsPerSubchannel
                [ofdmaBurst->modCodeType];

            if (rxMsgInfo->isDownlink)
            {
                numBitsSubchannel =
                phyDot16->downlinkNumDataBitsPerSubchannel
                [ofdmaBurst->modCodeType];
            }

            numBits +=
                ((endTime - startTime) / phyDot16->ofdmSymbolDuration)
                * numBitsSubchannel;
        }
    }

    ofdmaBurst->inError = FALSE;

    if (needCheck)
    {
        signalPower *= phyDot16->PowerLossDueToCPTime;
        sinr =  signalPower / (interferencePower + noisePower_mW);
        ofdmaBurst->cinr = sinr;

        BER = PHY_BER(phyDot16->thisPhy, ofdmaBurst->modCodeType, sinr);

        if (DEBUG_SNR )
        {
            printf("Node%d check burst error with "
                "rxPower_mW = %e, interferencePower_mW = %e "
                "sinr = %f BER = %f\n",
                node->nodeId, signalPower,
                interferencePower,
                IN_DB(sinr), BER);
        }

        if (BER != 0.0)
        {
            double errorProbability = 1.0 - pow((1.0 - BER), numBits);
            ERROR_Assert(errorProbability >= 0 &&
                             errorProbability <= 1.0,
                             "PHY802.16: Wrong error probability!");

            double rand = RANDOM_erand(phyDot16->thisPhy->seed);

            if (errorProbability > rand)
            {
                ofdmaBurst->inError = TRUE;
            }
        }
    }// end of if needCheck
}


// Check if any receiving frame has error.
//
// \param node  Pointer to node.
// \param phyDot16  Pointer to the 802.16 PHY structure
//
static
void PhyDot16CheckRxPacketError(Node* node, PhyDataDot16* phyDot16)
{
    PhyDot16RxMsgInfo* rxMsgInfo;
    PhyDot16OfdmaBurst* ofdmaBurst;

    // go through all signals under receiving
    rxMsgInfo = phyDot16->rxMsgList;
    while (rxMsgInfo != NULL)
    {
        // only check if not already in error
        if (rxMsgInfo->inError == FALSE)
        {
            // check each burst one by one
            ofdmaBurst = rxMsgInfo->ofdmaBurst;
            while (ofdmaBurst != NULL)
            {
                // only check if not already in error
                if (ofdmaBurst->inError == FALSE)
                {
                    PhyDot16CheckBurstError(node,
                                            phyDot16,
                                            rxMsgInfo,
                                            ofdmaBurst);
                }

                // if the first burst (containing DL-MAP) is wrong, then
                // the whole DL subframe is wrong. For UL, there is only
                // one burst. So no special handling needed.
                if (ofdmaBurst->inError &&
                    ofdmaBurst == rxMsgInfo->ofdmaBurst)
                {
                    rxMsgInfo->inError = TRUE;

                    // no need to continue, whole subframe is wrong
                    break;
                }

                ofdmaBurst = ofdmaBurst->next;
            }
        }

        rxMsgInfo = rxMsgInfo->next;
    }

    phyDot16->rxTimeEvaluated = node->getNodeTime();

    return;
}

// Recompute the interference power on each subchannel
// Expired signals may be removed from maintained list
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param channelIndex  Index of the channel
// \param msg  Message under receiving, not used
//    here
// \param signalPower_mW  For returning recv power, not used
// \param interferencePower_mW  For returning interference power
//
static
void PhyDot16SignalInterference(
         Node *node,
         int phyIndex,
         int channelIndex,
         Message *msg,
         double signalPower_mW[],
         double interferencePower_mW[])
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16 *)thisPhy->phyVar;
    PhyDot16RxMsgInfo* rxMsgList;
    PhyDot16RxMsgInfo* ifMsgList;
    PhyDot16RxMsgInfo* rxMsgInfo;

    int numSignals;
    PropRxInfo *ptr;
    clocktype currentTime;
    int i;

    rxMsgList = phyDot16->rxMsgList;
    ifMsgList = phyDot16->ifMsgList;
    phyDot16->rxMsgList = NULL;
    phyDot16->ifMsgList = NULL;

    numSignals = node->propData[channelIndex].numSignals;
    ptr = node->propData[channelIndex].rxSignalList;

    for (i = 0; i < numSignals; i ++)
    {
        // check if it is in the rxMsgList
        rxMsgInfo = PhyDot16GetRxMsgInfo(node,
                                         &rxMsgList,
                                         ptr->txMsg,
                                         TRUE);
        if (rxMsgInfo != NULL)
        {
            // if still in rxMsgList, add it
            rxMsgInfo->next = phyDot16->rxMsgList;
            phyDot16->rxMsgList = rxMsgInfo;
        }
        else
        {
            // check if it is in the interference list
            rxMsgInfo = PhyDot16GetRxMsgInfo(node,
                                             &ifMsgList,
                                             ptr->txMsg,
                                             TRUE);
            if (rxMsgInfo != NULL)
            {
                rxMsgInfo->next = phyDot16->ifMsgList;
                phyDot16->ifMsgList = rxMsgInfo;
            }
            else
            {
                // not in interference list, add it
                double rxPower_mW = NON_DB(ANTENNA_GainForThisSignal(
                                               node,
                                               phyIndex,
                                               ptr)
                                               + ptr->rxPower_dBm);

                rxMsgInfo = PhyDot16CreateRxMsgInfo(
                                node,
                                phyDot16,
                                ptr,
                                rxPower_mW);

                rxMsgInfo->next = phyDot16->ifMsgList;
                phyDot16->ifMsgList = rxMsgInfo;
            }
        }

        ptr = ptr->next;
    }

    // free signals which already expire
    while (rxMsgList != NULL)
    {
        rxMsgInfo = rxMsgList;
        rxMsgList = rxMsgList->next;
        PhyDot16FreeRxMsgInfo(node, rxMsgInfo);
    }

    // free if signals which already expire
    while (ifMsgList != NULL)
    {
        rxMsgInfo = ifMsgList;
        ifMsgList = ifMsgList->next;
        PhyDot16FreeRxMsgInfo(node, rxMsgInfo);
    }

    int numSubchannels =
                PhyDot16GetNumberSubchannelsReceiving(node, phyIndex);
    // re-calculate the interference power on each subchannel.
    for (i = 0; i < numSubchannels; i ++)
    {
        phyDot16->interferencePower_mW[i] = 0.0;
    }

    currentTime = node->getNodeTime();
    rxMsgInfo = phyDot16->ifMsgList;
    while (rxMsgInfo != NULL)
    {
        for (i = rxMsgInfo->startSubchannel;
             i <= rxMsgInfo->endSubchannel; i ++)
        {
            if (rxMsgInfo->subchannelStartTime[i] >= currentTime &&
                rxMsgInfo->subchannelEndTime[i] < currentTime)
            {
                phyDot16->interferencePower_mW[i] +=
                            rxMsgInfo->rxMsgPower_mW[i];

            }
        }

        rxMsgInfo = rxMsgInfo->next;
    }
}

// Report extended status of the PHY to MAC upon change
//
// \param node  Pointer to node.
// \param phyNum  Index of the PHY
// \param status  New status of the PHY
// \param receivingDuration  If receiving status, duration of recv
// \param potentialIncomingPacket  If recving status, potential
//    packet
//
static
void PhyDot16ReportExtendedStatusToMac(
         Node *node,
         int phyNum,
         PhyStatusType status,
         clocktype receiveDuration,
         Message* potentialIncomingPacket)
{
    PhyData* thisPhy = node->phyData[phyNum];
    PhyDataDot16* phyDot16 = (PhyDataDot16 *)thisPhy->phyVar;

    ERROR_Assert(status == phyDot16->mode,
            "PHY802.16: Unexpected physical status!");

    if (potentialIncomingPacket != NULL) {
        MESSAGE_RemoveHeader(
            node,
            potentialIncomingPacket,
            sizeof(PhyDot16PlcpHeader),
            TRACE_PHY_DOT16);
    }

    MAC_ReceivePhyStatusChangeNotification(
        node, thisPhy->macInterfaceIndex,
        phyDot16->previousMode, status,
        receiveDuration, potentialIncomingPacket);

    if (potentialIncomingPacket != NULL) {

        MESSAGE_AddHeader(
            node,
            potentialIncomingPacket,
            sizeof(PhyDot16PlcpHeader),
            TRACE_PHY_DOT16);

    }
}

// Report status of the PHY to MAC upon change
//
// \param node  Pointer to node.
// \param phyNum  Index of the PHY
// \param status  New status of the PHY
//
static inline
void PhyDot16ReportStatusToMac(Node *node, int phyNum, PhyStatusType status)
{
    PhyDot16ReportExtendedStatusToMac(node, phyNum, status, 0, NULL);
}

// Set the number of subchannels used by this PHY
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//
static
void PhyDot16SetNumberSubchannels(Node* node, int phyIndex)
{
    PhyDataDot16* phyDot16 =
        (PhyDataDot16*)(node->phyData[phyIndex]->phyVar);

    if (phyDot16->fftSize == FFT_2048)
    {
        phyDot16->downlink_numSubchannels =
                PHY802_16_DOWNLINK_NUMBER_SUBCHANNEL_FFT_2048;
        phyDot16->uplink_numSubchannels =
                PHY802_16_UPLINK_NUMBER_SUBCHANNEL_FFT_2048;
    }
    else if (phyDot16->fftSize == FFT_1024)
    {
        phyDot16->downlink_numSubchannels =
                PHY802_16_DOWNLINK_NUMBER_SUBCHANNEL_FFT_1024;
        phyDot16->uplink_numSubchannels =
                PHY802_16_UPLINK_NUMBER_SUBCHANNEL_FFT_1024;

    }
    else if (phyDot16->fftSize == FFT_512)
    {
        phyDot16->downlink_numSubchannels =
                PHY802_16_DOWNLINK_NUMBER_SUBCHANNEL_FFT_512;
        phyDot16->uplink_numSubchannels =
                PHY802_16_UPLINK_NUMBER_SUBCHANNEL_FFT_512;

    }
    else if (phyDot16->fftSize == FFT_128)
    {
        phyDot16->downlink_numSubchannels =
                PHY802_16_DOWNLINK_NUMBER_SUBCHANNEL_FFT_128;
        phyDot16->uplink_numSubchannels =
                PHY802_16_UPLINK_NUMBER_SUBCHANNEL_FFT_128;

    }
    else
    {
        ERROR_ReportError("PHY802.16: Wrong FFT size not supported!");
    }

    return;
}

// Initialize non-configurable parameters
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//
static
void PhyDot16InitDefaultParameters(Node* node, int phyIndex)
{
    PhyDataDot16* phyDot16 =
        (PhyDataDot16*)(node->phyData[phyIndex]->phyVar);
    double bwDb;
    double AdjustmentFactorDueNoiseFactor;

    bwDb = 10.0 * log10((double)phyDot16->channelBandwidth);

    AdjustmentFactorDueNoiseFactor =
            IN_DB(phyDot16->thisPhy->noiseFactor)
            - PHY802_16_NOISE_FACTOR_REFERENCE
            + PHY802_16_IMPLEMENTATION_LOSS;
    phyDot16->numDataRates = PHY802_16_NUM_DATA_RATES;

    phyDot16->uplinkNumDataBitsPerSubchannel[PHY802_16_QPSK_R_1_2] =
        PHY802_16_UPLINK_DATA_PAYLOAD_PER_SUBCHANNEL_QPSK_R_1_2;
    phyDot16->uplinkNumDataBitsPerSubchannel[PHY802_16_QPSK_R_3_4] =
        PHY802_16_UPLINK_DATA_PAYLOAD_PER_SUBCHANNEL_QPSK_R_3_4;
    phyDot16->uplinkNumDataBitsPerSubchannel[PHY802_16_16QAM_R_1_2] =
        PHY802_16_UPLINK_DATA_PAYLOAD_PER_SUBCHANNEL_16QAM_R_1_2;
    phyDot16->uplinkNumDataBitsPerSubchannel[PHY802_16_16QAM_R_3_4] =
        PHY802_16_UPLINK_DATA_PAYLOAD_PER_SUBCHANNEL_16QAM_R_3_4;
    phyDot16->uplinkNumDataBitsPerSubchannel[PHY802_16_64QAM_R_1_2] =
        PHY802_16_UPLINK_DATA_PAYLOAD_PER_SUBCHANNEL_64QAM_R_1_2;
    phyDot16->uplinkNumDataBitsPerSubchannel[PHY802_16_64QAM_R_2_3] =
        PHY802_16_UPLINK_DATA_PAYLOAD_PER_SUBCHANNEL_64QAM_R_2_3;
    phyDot16->uplinkNumDataBitsPerSubchannel[PHY802_16_64QAM_R_3_4] =
        PHY802_16_UPLINK_DATA_PAYLOAD_PER_SUBCHANNEL_64QAM_R_3_4;
    phyDot16->uplinkNumDataBitsPerSubchannel[PHY802_16_BPSK] =
        PHY802_16_CDMA_RANGING_PAYLOAD_PER_SUBCHANNEL_BPSK;


    phyDot16->downlinkNumDataBitsPerSubchannel[PHY802_16_QPSK_R_1_2] =
        PHY802_16_DOWNLINK_DATA_PAYLOAD_PER_SUBCHANNEL_QPSK_R_1_2;
    phyDot16->downlinkNumDataBitsPerSubchannel[PHY802_16_QPSK_R_3_4] =
        PHY802_16_DOWNLINK_DATA_PAYLOAD_PER_SUBCHANNEL_QPSK_R_3_4;
    phyDot16->downlinkNumDataBitsPerSubchannel[PHY802_16_16QAM_R_1_2] =
        PHY802_16_DOWNLINK_DATA_PAYLOAD_PER_SUBCHANNEL_16QAM_R_1_2;
    phyDot16->downlinkNumDataBitsPerSubchannel[PHY802_16_16QAM_R_3_4] =
        PHY802_16_DOWNLINK_DATA_PAYLOAD_PER_SUBCHANNEL_16QAM_R_3_4;
    phyDot16->downlinkNumDataBitsPerSubchannel[PHY802_16_64QAM_R_1_2] =
        PHY802_16_DOWNLINK_DATA_PAYLOAD_PER_SUBCHANNEL_64QAM_R_1_2;
    phyDot16->downlinkNumDataBitsPerSubchannel[PHY802_16_64QAM_R_2_3] =
        PHY802_16_DOWNLINK_DATA_PAYLOAD_PER_SUBCHANNEL_64QAM_R_2_3;
    phyDot16->downlinkNumDataBitsPerSubchannel[PHY802_16_64QAM_R_3_4] =
        PHY802_16_DOWNLINK_DATA_PAYLOAD_PER_SUBCHANNEL_64QAM_R_3_4;
    phyDot16->downlinkNumDataBitsPerSubchannel[PHY802_16_BPSK] =
        PHY802_16_CDMA_RANGING_PAYLOAD_PER_SUBCHANNEL_BPSK;

    phyDot16->rxSensitivity_mW[PHY802_16_QPSK_R_1_2] =
            NON_DB(PHY802_16_DEFAULT_SENSITIVITY_QPSK_R_1_2 + bwDb
            - PHY802_16_SENSITIVITY_ADJUSTMENT_FACTOR
            + AdjustmentFactorDueNoiseFactor);
    phyDot16->rxSensitivity_mW[PHY802_16_QPSK_R_3_4] =
            NON_DB(PHY802_16_DEFAULT_SENSITIVITY_QPSK_R_3_4 + bwDb
            - PHY802_16_SENSITIVITY_ADJUSTMENT_FACTOR
            + AdjustmentFactorDueNoiseFactor);
    phyDot16->rxSensitivity_mW[PHY802_16_16QAM_R_1_2] =
            NON_DB(PHY802_16_DEFAULT_SENSITIVITY_16QAM_R_1_2 + bwDb
            - PHY802_16_SENSITIVITY_ADJUSTMENT_FACTOR
            + AdjustmentFactorDueNoiseFactor);
    phyDot16->rxSensitivity_mW[PHY802_16_16QAM_R_3_4] =
            NON_DB(PHY802_16_DEFAULT_SENSITIVITY_16QAM_R_3_4 + bwDb
            - PHY802_16_SENSITIVITY_ADJUSTMENT_FACTOR
            + AdjustmentFactorDueNoiseFactor);
    phyDot16->rxSensitivity_mW[PHY802_16_64QAM_R_1_2] =
            NON_DB(PHY802_16_DEFAULT_SENSITIVITY_64QAM_R_1_2 + bwDb
            - PHY802_16_SENSITIVITY_ADJUSTMENT_FACTOR
            + AdjustmentFactorDueNoiseFactor);
    phyDot16->rxSensitivity_mW[PHY802_16_64QAM_R_2_3] =
            NON_DB(PHY802_16_DEFAULT_SENSITIVITY_64QAM_R_2_3 + bwDb
            - PHY802_16_SENSITIVITY_ADJUSTMENT_FACTOR
            + AdjustmentFactorDueNoiseFactor);
    phyDot16->rxSensitivity_mW[PHY802_16_64QAM_R_3_4] =
            NON_DB(PHY802_16_DEFAULT_SENSITIVITY_64QAM_R_3_4 + bwDb
            - PHY802_16_SENSITIVITY_ADJUSTMENT_FACTOR
            + AdjustmentFactorDueNoiseFactor);
    phyDot16->rxSensitivity_mW[PHY802_16_BPSK] =
           NON_DB(PHY802_16_DEFAULT_SENSITIVITY_BPSK + bwDb
            - PHY802_16_SENSITIVITY_ADJUSTMENT_FACTOR
            + AdjustmentFactorDueNoiseFactor);

    phyDot16->rxTxTurnaroundTime = PHY802_16_RX_TX_TURNAROUND_TIME;

    return;
}

// Initialize configurable parameters
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param nodeInput  Pointer to node input
//
static
void PhyDot16InitConfigurableParameters(
         Node* node,
         int phyIndex,
         const NodeInput *nodeInput)
{
    PhyDataDot16* phyDot16 =
        (PhyDataDot16*)(node->phyData[phyIndex]->phyVar);

    double txPower_dBm;
    BOOL   wasFound;
    double samplingFrequency;
    double deltaFrequency;
    double fftsize;
    double channelbandwidth;
    double cyclicprefix;
    int    fftSize;

    //
    // Set PHY802_16-TX-POWER's
    //
    IO_ReadDouble(node,
                  node->nodeId,
                  node->phyData[phyIndex]->macInterfaceIndex,
                  nodeInput,
                  "PHY802.16-TX-POWER",
                  &wasFound,
                  &txPower_dBm);

    if (wasFound)
    {
        phyDot16->txPower_dBm = (float)txPower_dBm;
    }
    else
    {
        phyDot16->txPower_dBm = PHY802_16_DEFAULT_TX_POWER_dBm;
    }

    IO_ReadDouble(node,
                  node->nodeId,
                  node->phyData[phyIndex]->macInterfaceIndex,
                  nodeInput,
                  "PHY802.16-MAX-TX-POWER",
                  &wasFound,
                  &txPower_dBm);

    if (wasFound)
    {
        phyDot16->txMaxPower_dBm = (float)txPower_dBm;
    }
    else
    {
        phyDot16->txMaxPower_dBm = PHY802_16_DEFAULT_TX_MAX_POWER_dBm;
    }
    if (phyDot16->txPower_dBm >= phyDot16->txMaxPower_dBm)
    {
        char errorStr[MAX_STRING_LENGTH];
        sprintf(errorStr,
                "PHY802.16: PHY802.16-MAX-TX-POWER should be configured "
                "> PHY802.16-TX-POWER");
        ERROR_ReportError(errorStr);
    }

    IO_ReadDouble(node,
                  node->nodeId,
                  node->phyData[phyIndex]->macInterfaceIndex,
                  nodeInput,
                  "PHY802.16-CHANNEL-BANDWIDTH",
                  &wasFound,
                  &channelbandwidth);

    if (wasFound)
    {

        ERROR_Assert(channelbandwidth > 0.0,
            "PHY802.16: Channel bandwidth should be larger than zero!");
        phyDot16->channelBandwidth = channelbandwidth;
    }
    else
    {
        phyDot16->channelBandwidth = PHY802_16_CHANNEL_BANDWIDTH;
    }

    IO_ReadInt(node,
               node->nodeId,
               node->phyData[phyIndex]->macInterfaceIndex,
               nodeInput,
               "PHY802.16-FFT-SIZE",
               &wasFound,
               &fftSize);

    if (wasFound)
    {
        if (fftSize == 2048 )
        {
            phyDot16->fftSize = FFT_2048;
        }
        else if (fftSize == 1024 )
        {
            phyDot16->fftSize = FFT_1024;
        }
        else if (fftSize == 512 )
        {
            phyDot16->fftSize = FFT_512;
        }
        else if (fftSize == 128 )
        {
            phyDot16->fftSize = FFT_128;
        }
        else
        {
            char errorStr[MAX_STRING_LENGTH];
            sprintf(errorStr,
                    "PHY802.16: FFT size as %d is not supported!"
                    " Supported values are 2048, 1024, 512 and 128",
                    fftSize);
            ERROR_ReportError(errorStr);
        }
    }
    else
    {
        phyDot16->fftSize = PHY802_16_DEFAULT_FFT_SIZE;
    }

    IO_ReadDouble(node,
                  node->nodeId,
                  node->phyData[phyIndex]->macInterfaceIndex,
                  nodeInput,
                  "PHY802.16-CYCLIC-PREFIX",
                  &wasFound,
                  &cyclicprefix);

    if (wasFound)
    {
        if (cyclicprefix == 4.0 || cyclicprefix == 8.0 ||
            cyclicprefix == 16.0 || cyclicprefix == 32.0) {

            phyDot16->cyclicPrefix = cyclicprefix;
        }
        else {
            char errorStr[MAX_STRING_LENGTH];
            sprintf(errorStr,
                    "PHY802.16: CYCLIC PREFIX as %f is not supported!"
                    " Supported values are 4.0, 8.0, 16.0 and 32.0",
                    cyclicprefix);
            ERROR_ReportError(errorStr);
        }

    }
    else
    {
        phyDot16->cyclicPrefix = PHY802_16_CYCPREFIX;
    }

    samplingFrequency =
            floor(PHY802_16_SAMPING_FACTOR * phyDot16->channelBandwidth /
            PHY802_16_BANDWIDTH_FACTOR) * PHY802_16_BANDWIDTH_FACTOR;

    fftsize = PhyDot16GetFFTSize(node, phyIndex);
    deltaFrequency = samplingFrequency / fftsize;
    double cptime = 1.0 / phyDot16->cyclicPrefix;

    phyDot16->ofdmSymbolDuration = (clocktype)
        ((1.0 / deltaFrequency) * SECOND * (1.0 + cptime));

    ERROR_Assert(phyDot16->ofdmSymbolDuration != 0,
        "PHY802.16: Channel Bandwidth seems to be inconsistently configured");
    phyDot16->PowerLossDueToCPTime =
            NON_DB(10 * log(1.0 - cptime / (1.0 + cptime)) / log(10.0));

    PhyDot16SetNumberSubchannels(node, phyIndex);
}

// Initiate dynamic statistics
//
// \param node  Pointer to node
// \param phyIndex  PHY index
//
static
void PhyDot16InitDynamicStats(Node* node, int phyIndex)
{
    PhyDataDot16* phyDot16 = (PhyDataDot16*)node->phyData[phyIndex]->phyVar;

    if (!node->guiOption)
    {
        return;
    }

    phyDot16->stats.rxBurstsToMacGuiId =
        GUI_DefineMetric("802.16 PHY: Number of OFDMA Bursts Received",
                         node->nodeId,
                         GUI_PHY_LAYER,
                         node->phyData[phyIndex]->macInterfaceIndex,
                         GUI_INTEGER_TYPE,
                         GUI_CUMULATIVE_METRIC);
}

//--------------------------------------------------------------------------
//  API functions for setting and getting information of the 802.16 PHY
//--------------------------------------------------------------------------

// Get the transmission data rate
//
// \param thisPhy  Pointer to PHY structure
//
// \return Transmission data rate in bps
int PhyDot16GetTxDataRate(PhyData *thisPhy)
{
    int dataRate;
    dataRate = (int) PhyDot16GetDataRate(thisPhy, 0);

    return dataRate;
}


// Get the raw data rate
//
// \param thisPhy  Pointer to PHY structure
// \param moduCodeType  Modulation encoding type
//
// \return Data rate in bps


double PhyDot16GetDataRate(PhyData *thisPhy,
                           int moduCodeType)
{
    double PayloadPerSubchannel;
    double datarate;
    int numSubchannels;
    clocktype OfdmSymbolDuration;

    PhyDataDot16* phyDot16 =
        (PhyDataDot16*) thisPhy->phyVar;

    numSubchannels = phyDot16->uplink_numSubchannels;
    PayloadPerSubchannel =
        phyDot16->uplinkNumDataBitsPerSubchannel[moduCodeType];

    if (phyDot16->stationType == DOT16_BS)
    {
        numSubchannels = phyDot16->downlink_numSubchannels;
        PayloadPerSubchannel =
            phyDot16->downlinkNumDataBitsPerSubchannel[moduCodeType];
    }

    OfdmSymbolDuration = phyDot16->ofdmSymbolDuration;

    datarate = (SECOND / OfdmSymbolDuration)
                * numSubchannels * PayloadPerSubchannel;

   return datarate;
}

// Get the raw data rate
//
// \param thisPhy  Pointer to PHY structure
// \param moduCodeType  Modulation encoding type
//
// \return Data rate in bps

static
double PhyDot16GetRxDataRate(PhyData *thisPhy,
                           int moduCodeType)
{
    double PayloadPerSubchannel;
    double datarate;
    int numSubchannels;
    clocktype OfdmSymbolDuration;

    PhyDataDot16* phyDot16 =
        (PhyDataDot16*) thisPhy->phyVar;

    numSubchannels = phyDot16->downlink_numSubchannels;
    PayloadPerSubchannel =
        phyDot16->downlinkNumDataBitsPerSubchannel[moduCodeType];

    if (phyDot16->stationType == DOT16_BS)
    {
        numSubchannels = phyDot16->uplink_numSubchannels;
        PayloadPerSubchannel =
            phyDot16->uplinkNumDataBitsPerSubchannel[moduCodeType];
    }

    OfdmSymbolDuration = phyDot16->ofdmSymbolDuration;

    datarate = (SECOND / OfdmSymbolDuration)
                * numSubchannels * PayloadPerSubchannel;

   return datarate;
}

// Get the reception data rate
//
// \param thisPhy  Pointer to PHY structure
//
// \return Reception data rate in bps
int PhyDot16GetRxDataRate(PhyData *thisPhy)
{
    int dataRate;
    dataRate = (int) PhyDot16GetRxDataRate(thisPhy, 0);

    return dataRate;

}

// Obtain the number of subchannels for receiving.
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//
// \return Number of subchannels

int PhyDot16GetNumberSubchannelsReceiving(
                Node* node,
                int phyIndex)
{
     PhyDataDot16* phyDot16 =
        (PhyDataDot16*)(node->phyData[phyIndex]->phyVar);
    int numSubchannels;

    if (phyDot16->stationType == DOT16_BS)
    {
        numSubchannels = phyDot16->uplink_numSubchannels;
    }
    else
    {
        numSubchannels = phyDot16->downlink_numSubchannels;
    }

    return numSubchannels;
}

// Obtain the number of subchannels.
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//
// \return Number of subchannels
int PhyDot16GetNumberSubchannels(
        Node* node,
        int phyIndex)
{
    PhyDataDot16* phyDot16 =
        (PhyDataDot16*)(node->phyData[phyIndex]->phyVar);
    int numSubchannels;

    if (phyDot16->stationType == DOT16_BS)
    {
        numSubchannels = phyDot16->downlink_numSubchannels;
    }
    else
    {
        numSubchannels = phyDot16->uplink_numSubchannels;
    }

    return numSubchannels;
}

// Obtain the number of subchannels.
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//
// \return Number of subchannels
int PhyDot16GetUplinkNumberSubchannels(
        Node* node,
        int phyIndex)
{
    PhyDataDot16* phyDot16 =
        (PhyDataDot16*)(node->phyData[phyIndex]->phyVar);

    int numSubchannels;
    numSubchannels = phyDot16->uplink_numSubchannels;

    return numSubchannels;
}


// Obtain the duration of an OFDM symbol
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//
// \return Duration of a OFDM symbol
clocktype PhyDot16GetOfdmSymbolDuration(Node* node, int phyIndex)
{
    PhyDataDot16* phyDot16 =
        (PhyDataDot16*)(node->phyData[phyIndex]->phyVar);

    return phyDot16->ofdmSymbolDuration;
}

// Obtain data payload in bits per subchannel.
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param moduCodeType  Modulation encoding type
// \param subframeType  subframe type
//
// \return Data payload in bytes

int PhyDot16GetDataBitsPayloadPerSubchannel(
        Node* node,
        int phyIndex,
        int moduCodeType,
        MacDot16SubframeType subframeType)
{
    double PayloadPerSubchannel;

    PhyDataDot16* phyDot16 =
        (PhyDataDot16*)(node->phyData[phyIndex]->phyVar);

   PayloadPerSubchannel =
            phyDot16->uplinkNumDataBitsPerSubchannel[moduCodeType];

    if (subframeType == DOT16_DL)
    {
        PayloadPerSubchannel =
           phyDot16->downlinkNumDataBitsPerSubchannel[moduCodeType];
    }

    return (int)PayloadPerSubchannel;
}

// Get the modulation encoding combined type based on
// modulation type and encoding rate.
//
// \param moduType  Modulation type.
// \param encodingRate  Encoding rate
//
// \return Modulation and encoding combined type
int PhyDot16SetModulationEncoding(ModulationType moduType,
                                  EncodingRate encodingRate)
{
    int moduCodeType = PHY802_16_BPSK;

    if (moduType == BPSK_802_16)
    {
        moduCodeType = PHY802_16_BPSK;

    }
    else if (moduType == QPSK)
    {
        if (encodingRate == encodingRate_1_2)
        {
            moduCodeType = PHY802_16_QPSK_R_1_2;
        }
        else if (encodingRate == encodingRate_3_4)
        {
            moduCodeType = PHY802_16_QPSK_R_3_4;
        }
        else
        {
            ERROR_ReportError(
                "PHY802.16: Wrong encoding rate, not supported!");
        }
    }
    else if (moduType == QAM_16)
    {
        if (encodingRate == encodingRate_1_2)
        {
            moduCodeType = PHY802_16_16QAM_R_1_2;
        }
        else if (encodingRate == encodingRate_3_4)
        {
            moduCodeType = PHY802_16_16QAM_R_3_4;
        }
        else
        {
            ERROR_ReportError(
                "PHY802.16: Wrong encoding rate, not supported!");
        }
    }
    else if (moduType == QAM_64)
    {
        if (encodingRate == encodingRate_1_2)
        {
            moduCodeType = PHY802_16_64QAM_R_1_2;
        }
        else if (encodingRate == encodingRate_2_3)
        {
            moduCodeType = PHY802_16_64QAM_R_2_3;
        }
        else if (encodingRate == encodingRate_3_4)
        {
            moduCodeType = PHY802_16_64QAM_R_3_4;
        }
        else
        {
            ERROR_ReportError(
                "PHY802.16: Wrong encoding rate, not supported!");
        }
    }
    else
    {

        ERROR_ReportError(
            "PHY802.16: Wrong modulation scheme, not supported!");
    }

    return moduCodeType;
}

// Get the FFT size.
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//
// \return FFT size
double PhyDot16GetFFTSize(Node* node, int phyIndex)
{
    PhyDataDot16* phyDot16 =
        (PhyDataDot16*)(node->phyData[phyIndex]->phyVar);
    double fftsize = 0.0;

    if (phyDot16->fftSize == FFT_2048)
    {
        fftsize = 2048.0;
    }
    else if (phyDot16->fftSize == FFT_1024)
    {
        fftsize = 1024.0;
    }
    else if (phyDot16->fftSize == FFT_512)
    {
        fftsize = 512.0;
    }
    else if (phyDot16->fftSize == FFT_128)
    {
        fftsize = 128.0;
    }
    else
    {
        ERROR_ReportError("PHY802.16: Unknow FFT size!");
    }

    return fftsize;
}

// Used by MAC layer to tell PHY whether this station
// is Base Station (BS) or Subscriber Station (SS)
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param stationType  Type of the station
//
void PhyDot16SetStationType(Node* node, int phyIndex, char stationType)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16 *)thisPhy->phyVar;

    phyDot16->stationType = stationType;
}

// Return the receiving sensitivity in mW for a
// givin modulation and encoding combined type
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param moduCodeType  Modulation & encoding type
//
// \return Receiving sensitivity in mW
double PhyDot16GetRxSensitivity_mW(Node *node,
                                   int phyIndex,
                                   int moduCodeType)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16 *)thisPhy->phyVar;

    return phyDot16->rxSensitivity_mW[moduCodeType];
}

// Set the transmit power of the PHY
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param txPower_mW  Transmit power in mW unit
//
void PhyDot16SetTransmitPower(Node *node, int phyIndex, double txPower_mW)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16 *)thisPhy->phyVar;

    phyDot16->txPower_dBm = (float)IN_DB(txPower_mW);
}

// Retrieve the transmit power of the PHY in mW unit
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param txPower_mW  For returning the transmit power
//
void PhyDot16GetTransmitPower(Node *node, int phyIndex, double *txPower_mW)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16 *)thisPhy->phyVar;

    *txPower_mW = NON_DB(phyDot16->txPower_dBm);
}


//--------------------------------------------------------------------------
//  Key API functions of the 802.16 PHY
//--------------------------------------------------------------------------

// Initialize the 802.16 PHY
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param nodeInput  Pointer to the node input
//
void PhyDot16Init(Node *node,
                  const int phyIndex,
                  const NodeInput *nodeInput)
{
    PhyDataDot16* phyDot16;
    int numChannels;
    int memSize;
    int i;

    phyDot16 = (PhyDataDot16 *) MEM_malloc(sizeof(PhyDataDot16));
    ERROR_Assert(phyDot16 != NULL, "PHY802.16: Out of memory!");
    memset(phyDot16, 0, sizeof(PhyDataDot16));

    // set pointer in upper structure
    node->phyData[phyIndex]->phyVar = (void*)phyDot16;

    // get a reference pointer to upper structure
    phyDot16->thisPhy = node->phyData[phyIndex];

    // Initialize the antenna model
    ANTENNA_Init(node, phyIndex, nodeInput);

    // initialize parameters
    PhyDot16InitConfigurableParameters(node, phyIndex, nodeInput);
    PhyDot16InitDefaultParameters(node, phyIndex);

    // init dynamic stats
    PhyDot16InitDynamicStats(node, phyIndex);

    // Initialize status and other variables
    phyDot16->mode = PHY_IDLE;
    phyDot16->previousMode = PHY_IDLE;
    phyDot16->noisePower_mW =
        phyDot16->thisPhy->noise_mW_hz * phyDot16->channelBandwidth
        * NON_DB(PHY802_16_IMPLEMENTATION_LOSS);

    int numSubchannels = phyDot16->uplink_numSubchannels;

    memSize = sizeof(double) * numSubchannels;
    phyDot16->interferencePower_mW = (double*) MEM_malloc(memSize);
    ERROR_Assert(phyDot16->interferencePower_mW != NULL,
                 "PHY802.16: Out of memory!");
    memset(phyDot16->interferencePower_mW, 0, memSize);

    phyDot16->txEndMsgList = new std::list<Message*>;

    // set the default transmission channel, MAC may reset
    numChannels = PROP_NumberChannels(node);
    for (i = 0; i < numChannels; i++)
    {
        if (PHY_IsListeningToChannel(node, phyIndex, i))
        {
            break;
        }
    }

    if (i == numChannels)
    {
        i = 0;
    }

    PHY_SetTransmissionChannel(node, phyIndex, i);

    return;
}

// Finalize the 802.16 PHY, print out statistics
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//
void PhyDot16Finalize(Node *node, const int phyIndex)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16*) thisPhy->phyVar;
    char buf[MAX_STRING_LENGTH];

    if (thisPhy->phyStats == FALSE)
    {
        return;
    }

    sprintf(buf, "Signals transmitted = %d",
            (Int32)phyDot16->stats.totalTxSignals);
    IO_PrintStat(node, "Physical", "802.16", ANY_DEST, phyIndex, buf);

    sprintf(buf, "Signals received and forwarded to MAC = %d",
            (Int32)phyDot16->stats.totalRxSignalsToMac);
    IO_PrintStat(node, "Physical", "802.16", ANY_DEST, phyIndex, buf);

    sprintf(buf, "Signals locked on by PHY = %d",
            (Int32)phyDot16->stats.totalSignalsLocked);
    IO_PrintStat(node, "Physical", "802.16", ANY_DEST, phyIndex, buf);

    sprintf(buf, "Signals received but with errors = %d",
            (Int32)phyDot16->stats.totalSignalsWithErrors);
    IO_PrintStat(node, "Physical", "802.16", ANY_DEST, phyIndex, buf);

    sprintf(buf, "OFDMA bursts received and forwarded to MAC = %d",
            (Int32)phyDot16->stats.totalRxBurstsToMac);
    IO_PrintStat(node, "Physical", "802.16", ANY_DEST, phyIndex, buf);

    sprintf(buf, "OFDMA bursts received but with errors = %d",
            (Int32)phyDot16->stats.totalRxBurstsWithErrors);
    IO_PrintStat(node, "Physical", "802.16", ANY_DEST, phyIndex, buf);
}

// Handle signal arrival from the chanel
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param channelIndex  Index of the channel receiving signal from
// \param propRxInfo  Propagation information
//
void PhyDot16SignalArrivalFromChannel(
         Node* node,
         int phyIndex,
         int channelIndex,
         PropRxInfo *propRxInfo)
{
    PhyDataDot16* phyDot16 = (PhyDataDot16*)node->phyData[phyIndex]->phyVar;
    PhyDot16RxMsgInfo* rxMsgInfo;
    double rxPower_mW;
    int i;

    rxPower_mW =
        NON_DB(ANTENNA_GainForThisSignal(node, phyIndex, propRxInfo)
        + propRxInfo->rxPower_dBm);

    if (DEBUG)
    {
        char clockStr[MAX_STRING_LENGTH];

        ctoa(node->getNodeTime(), clockStr);
        printf("PHY802.16: Node%d receives a signal arrival from "
               "channel %d with power %e at time %s\n",
               node->nodeId, channelIndex, rxPower_mW, clockStr);
    }

    // get rx msg info of the incoming signal
    rxMsgInfo = PhyDot16CreateRxMsgInfo(node,
                                        phyDot16,
                                        propRxInfo,
                                        rxPower_mW);

    // if signal is stronger than sensitivity, check if needs to receive
    if (phyDot16->mode != PHY_TRANSMITTING &&
        (rxPower_mW >= phyDot16->rxSensitivity_mW[0]
        ||((rxPower_mW >= phyDot16->rxSensitivity_mW[7] /
        PHY802_16_DEFAULT_CDMA_SPREADING_FACTOR) &&
        rxMsgInfo->ofdmaBurst->modCodeType == PHY802_16_BPSK)) &&
        (phyDot16->stationType != DOT16_SS || rxMsgInfo->isDownlink) &&
        (PhyDot16NoOverlap(node, phyDot16, rxMsgInfo) ||
        rxMsgInfo->ofdmaBurst->modCodeType == PHY802_16_BPSK))
    {
        // start receiving it

        if (DEBUG)
        {
            printf("    This is a good signal to be received\n");
        }

        // change mode to receiving
        PhyDot16ChangeState(node, phyIndex, PHY_RECEIVING);

        // put into the receiving signal list
        if (phyDot16->rxMsgList == NULL)
        {
            phyDot16->rxMsgList = rxMsgInfo;
        }
        else
        {
            rxMsgInfo->next = phyDot16->rxMsgList;
            phyDot16->rxMsgList = rxMsgInfo;
        }

        PhyDot16ReportExtendedStatusToMac(
            node,
            phyIndex,
            PHY_RECEIVING,
            propRxInfo->duration,
            propRxInfo->txMsg);

        phyDot16->stats.totalSignalsLocked++;
    }
    else
    {
        // treat as interference signal

        if (DEBUG)
        {
            printf("    This is an interference signal\n");
        }

        // add into the interference signal list
        if (phyDot16->ifMsgList == NULL)
        {
            phyDot16->ifMsgList = rxMsgInfo;
        }
        else
        {
            rxMsgInfo->next = phyDot16->ifMsgList;
            phyDot16->ifMsgList = rxMsgInfo;
        }

        double numSubchannels =
                rxMsgInfo->endSubchannel - rxMsgInfo->startSubchannel + 1.0;

        ERROR_Assert(numSubchannels > 0.0,
                 "PHY802.16: Number of subchannels should be larger"
                 " than zero!");
        // add to the interference power for used subchannels
        for (i = rxMsgInfo->startSubchannel;
             i <= rxMsgInfo->endSubchannel; i ++)
        {
            phyDot16->interferencePower_mW[i] +=
                        (rxPower_mW / numSubchannels);

        }
    }
}

// Handle signal end from a chanel
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param channelIndex  Index of the channel receiving signal from
// \param propRxInfo  Propagation information
//
void PhyDot16SignalEndFromChannel(
        Node* node,
        int phyIndex,
        int channelIndex,
        PropRxInfo *propRxInfo)
{
    PhyData *thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16*) thisPhy->phyVar;
    PhyDot16RxMsgInfo* rxMsgInfo;

    int i;

    if (DEBUG)
    {
        char clockStr[MAX_STRING_LENGTH];

        ctoa(node->getNodeTime(), clockStr);
        printf("PHY802.16: Node%d receives a signal end from "
               "channel %d at time %s\n",
               node->nodeId, channelIndex, clockStr);
    }

    PhyDot16CheckRxPacketError(node, phyDot16);

    // search if in receiving signal list, if found, remove it too
    rxMsgInfo = PhyDot16GetRxMsgInfo(node,
                                     &(phyDot16->rxMsgList),
                                     propRxInfo->txMsg,
                                     TRUE);

    //
    // If the phy is still receiving this signal, forward the frame
    // to the MAC layer if no error.
    //
    if (rxMsgInfo != NULL)
    {
        PhySignalMeasurement* signalMea;
        Message *newMsg;

        // check if any error
        if (!rxMsgInfo->inError)
        {
            PhyDot16OfdmaBurst* ofdmaBurst;

            if (DEBUG)
            {
                printf("Node%d passes a packet to upper layer\n",
                       node->nodeId);
            }

            ofdmaBurst = rxMsgInfo->ofdmaBurst;
            ERROR_Assert(ofdmaBurst != NULL, "PHY802.16: Empty frame!");

            // first burst containing DL-MAP must not be in error.
            // Otherwise, whole signal should be in error already.
            // If UL signal, there is only one burst, so if
            // in error, whole signal in error too.
            ERROR_Assert(ofdmaBurst->inError == FALSE,
                         "PHY802.16: Unexpected reception situation!");

            // if I am SS, remove the FCH field
            if (phyDot16->stationType == DOT16_SS && rxMsgInfo->isDownlink)
            {
                 MESSAGE_RemoveHeader(node,
                                      ofdmaBurst->pduMsgList,
                                      sizeof(MacDot16PhyOfdmaFch),
                                      TRACE_DOT16);
            }

            while (ofdmaBurst != NULL)
            {
                // if the burst not in error, then pass to MAC
                if (ofdmaBurst->inError == FALSE)
                {
                    if (DEBUG)
                    {
                        printf("    Successfully pass a burt to MAC!\n");
                    }

                    newMsg = ofdmaBurst->pduMsgList;
                    ERROR_Assert(newMsg != NULL, "PHY802.16: Empty burst!");

                    MESSAGE_InfoAlloc(node,
                                      newMsg,
                                      sizeof(PhySignalMeasurement));
                    signalMea = (PhySignalMeasurement*)
                                MESSAGE_ReturnInfo(newMsg);

                    signalMea->rxBeginTime = rxMsgInfo->rxBeginTime;

                    double signalPower_mW = 0.0;

                    for (i = rxMsgInfo->startSubchannel;
                         i <= rxMsgInfo->endSubchannel; i ++) {

                        signalPower_mW +=  rxMsgInfo->rxMsgPower_mW[i];

                    }

                    signalMea->rss = IN_DB(signalPower_mW *
                                           phyDot16->PowerLossDueToCPTime);
                    signalMea->snr = IN_DB((signalPower_mW *
                                           phyDot16->PowerLossDueToCPTime /
                                           phyDot16->noisePower_mW));
                    signalMea->cinr = IN_DB(ofdmaBurst->cinr);

                    signalMea->fecCodeModuType = ofdmaBurst->modCodeType;

                    MESSAGE_SetInstanceId(newMsg, (short) phyIndex);

                    MAC_ReceivePacketFromPhy(
                        node,
                        node->phyData[phyIndex]->macInterfaceIndex,
                        newMsg);

                    phyDot16->stats.totalRxBurstsToMac ++;

                    // update dynamic stats
                    if (node->guiOption)
                    {
                        GUI_SendIntegerData(
                            node->nodeId,
                            phyDot16->stats.rxBurstsToMacGuiId,
                            phyDot16->stats.totalRxBurstsToMac,
                            node->getNodeTime());
                    }

                    ofdmaBurst->pduMsgList = NULL;
                }
                else
                {
                    // burst has error, drop it
                    phyDot16->stats.totalRxBurstsWithErrors ++;

                    if (DEBUG)
                    {
                        printf("    Burst with error, dropped\n");
                    }
                }

                ofdmaBurst = ofdmaBurst->next;
            }

            phyDot16->stats.totalRxSignalsToMac ++;
        }
        else
        {
            if (DEBUG)
            {
                printf("    Signal with error, dropped\n");
            }

            phyDot16->stats.totalSignalsWithErrors ++;
        }

        // free the rxMsgInfo
        PhyDot16FreeRxMsgInfo(node, rxMsgInfo);
        rxMsgInfo = NULL;

        // if no more receiving signal, change to IDLE mode
        if (phyDot16->rxMsgList == NULL)
        {
            // no more signal under receiving, change to IDLE
            PhyDot16ChangeState(node, phyIndex, PHY_IDLE);
            PhyDot16ReportStatusToMac(node, phyIndex, phyDot16->mode);
        }
    }
    else
    {
        // search if in interference signal list, if found, remove it
        rxMsgInfo = PhyDot16GetRxMsgInfo(node,
                                         &(phyDot16->ifMsgList),
                                         propRxInfo->txMsg,
                                         TRUE);

        // if in interference list, delete from interference power
        if (rxMsgInfo != NULL)
        {

            if (DEBUG)
            {
                printf("    Signal end of interference signal\n");
            }

            for (i = rxMsgInfo->startSubchannel;
                 i <= rxMsgInfo->endSubchannel; i ++)
            {
                phyDot16->interferencePower_mW[i] -=
                                rxMsgInfo->rxMsgPower_mW[i];

                if (phyDot16->interferencePower_mW[i] < 0.0)
                {
                    phyDot16->interferencePower_mW[i] = 0.0;
                }
            }

            // free the rxMsgInfo
            PhyDot16FreeRxMsgInfo(node, rxMsgInfo);
            rxMsgInfo = NULL;
        }
        else
        {
            if (DEBUG)
            {
                printf("    Signal not found in list\n");
            }
        }
    }

}

// Terminate all signals current under receiving. Move them
// to be as interference signals
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param terminateOnlyOnReceiveError  Only terminate if in error
// \param frameError  For returning if frame is in error
// \param endSignalTime  For returning expected signal end time
//
void PhyDot16TerminateCurrentReceive(
         Node* node,
         int phyIndex,
         const BOOL terminateOnlyOnReceiveError,
         BOOL* frameError,
         clocktype* endSignalTime)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16 *)thisPhy->phyVar;
    PhyDot16RxMsgInfo* rxMsgInfo;
    PhyDot16RxMsgInfo* tmpMsgInfo;
    int i;

    if (DEBUG)
    {
        printf("Node%d terminate current receiving action\n", node->nodeId);
    }

    rxMsgInfo = phyDot16->rxMsgList;

    if (rxMsgInfo == NULL)
    {
        // no signal under receiving, return;
        *frameError = FALSE;
        *endSignalTime = node->getNodeTime();

        return;
    }

    if (rxMsgInfo->inError == FALSE)
    {
        PhyDot16CheckRxPacketError(node, phyDot16);
    }

    // use the informtion of the first receiving signal
    *frameError = rxMsgInfo->inError;
    *endSignalTime = rxMsgInfo->rxEndTime;

    if (terminateOnlyOnReceiveError && !rxMsgInfo->inError)
    {
        return;
    }

    // move receiving signals to be interference signals
    while (rxMsgInfo != NULL)
    {
        tmpMsgInfo = rxMsgInfo->next;
        rxMsgInfo->next = phyDot16->ifMsgList;
        phyDot16->ifMsgList = rxMsgInfo;

        // add to the interference power for used subchannels
        for (i = rxMsgInfo->startSubchannel;
             i <= rxMsgInfo->endSubchannel; i ++)
        {
            phyDot16->interferencePower_mW[i] +=
                            rxMsgInfo->rxMsgPower_mW[i];

        }

        rxMsgInfo = tmpMsgInfo;
        phyDot16->stats.totalSignalsWithErrors ++;
    }

    phyDot16->rxMsgList = NULL;
    PhyDot16ChangeState(node, phyIndex, PHY_IDLE);
}

// Terminate all ongoing transmissions
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//
void PhyDot16TerminateCurrentTransmissions(Node* node, int phyIndex)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16 *)thisPhy->phyVar;

    if (phyDot16->numTxMsgs > 0)
    {
        // cancel all scheduled Transmission End msgs
        std::list<Message*>::iterator it;

        for (it = phyDot16->txEndMsgList->begin();
             it != phyDot16->txEndMsgList->end();
             it ++)
        {
            MESSAGE_CancelSelfMsg(node, (*it));
        }
        phyDot16->txEndMsgList->clear();
        phyDot16->numTxMsgs = 0;

        //GuiStart
        if (node->guiOption == TRUE)
        {
            GUI_EndBroadcast(node->nodeId,
                             GUI_PHY_LAYER,
                             GUI_DEFAULT_DATA_TYPE,
                             thisPhy->macInterfaceIndex,
                             node->getNodeTime());
        }
        //GuiEnd
    }
}

// Start transmitting a frame
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param packet  Frame to be transmitted
//    + useMacLayerSpecifiedDelay : Use MAC specified delay or calculate it
// \param initDelayUntilAirborne  The MAC specified delay
//
void PhyDot16StartTransmittingSignal(
         Node* node,
         int phyIndex,
         Message* packet,
         BOOL useMacLayerSpecifiedDelay,
         clocktype initDelayUntilAirborne)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16 *)thisPhy->phyVar;
    clocktype delayUntilAirborne = initDelayUntilAirborne;
    int channelIndex;
    Message *endMsg;
    clocktype duration;
    PhyDot16PlcpHeader* plcp;
    PhyDot16TxInfo* txInfo;

    if (DEBUG)
    {
        char clockStr[MAX_STRING_LENGTH];
        ctoa(node->getNodeTime(), clockStr);
        printf("Time %s: Node%d starts transmitting a signal\n",
               clockStr, node->nodeId);
    }

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);

    if (!useMacLayerSpecifiedDelay)
    {
        delayUntilAirborne = phyDot16->rxTxTurnaroundTime;
    }

    if (phyDot16->mode == PHY_TRANSMITTING &&
        phyDot16->stationType == DOT16_BS)
    {
        char errorStr[MAX_STRING_LENGTH];

        sprintf(errorStr,
                "PHY802.16: BS %d starts new tx while still in tx mode!",
                node->nodeId);
        ERROR_ReportError(errorStr);
    }

    // if some signals are still under receiving, stop them and move them
    // to be interference signals
    if (phyDot16->rxMsgList != NULL)
    {
        BOOL frameError;
        clocktype endSignalTime;
        PhyDot16TerminateCurrentReceive(node,
                                        phyIndex,
                                        FALSE,
                                        &frameError,
                                        &endSignalTime);
    }

    PhyDot16ChangeState(node, phyIndex, PHY_TRANSMITTING);

    txInfo = (PhyDot16TxInfo*) MESSAGE_ReturnInfo(packet);
    duration = txInfo->duration;

    MESSAGE_AddHeader(node,
                      packet,
                      sizeof(PhyDot16PlcpHeader),
                      TRACE_PHY_DOT16);

    plcp = (PhyDot16PlcpHeader*)MESSAGE_ReturnPacket(packet);
    if (phyDot16->stationType == DOT16_BS)
    {
        plcp->isDownlink = TRUE;
    }
    else
    {
        plcp->isDownlink = FALSE;
    }
    plcp->duration = duration;

    if (PHY_IsListeningToChannel(node, phyIndex, channelIndex))
    {
        PHY_StopListeningToChannel(node, phyIndex, channelIndex);
    }

    PROP_ReleaseSignal(
        node,
        packet,
        phyIndex,
        channelIndex,
        phyDot16->txPower_dBm,
        duration,
        delayUntilAirborne);

    // SS node can transmits on different subchannels concurrently
    // This is to track how many transmissions active now
    phyDot16->numTxMsgs ++;

    //GuiStart
    if (node->guiOption == TRUE && phyDot16->numTxMsgs == 1)
    {
        GUI_Broadcast(node->nodeId,
                      GUI_PHY_LAYER,
                      GUI_DEFAULT_DATA_TYPE,
                      thisPhy->macInterfaceIndex,
                      node->getNodeTime());
    }
    //GuiEnd

    endMsg = MESSAGE_Alloc(node,
                           PHY_LAYER,
                           0,
                           MSG_PHY_TransmissionEnd);

    MESSAGE_SetInstanceId(endMsg, (short) phyIndex);
    phyDot16->txEndMsgList->push_back(endMsg);
    MESSAGE_Send(node, endMsg, delayUntilAirborne + duration + 1);

    // Keep track of phy statistics
    phyDot16->stats.totalTxSignals ++;
}

// End of the transmission
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param msg  The Tx End event
//
void PhyDot16TransmissionEnd(Node *node, int phyIndex, Message* msg)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16 *)thisPhy->phyVar;
    int channelIndex;

    if (DEBUG)
    {
        char clockStr[MAX_STRING_LENGTH];
        ctoa(node->getNodeTime(), clockStr);
        printf("Time %s: Node%d transmission end\n",
               clockStr, node->nodeId);
    }

    ERROR_Assert(phyDot16->mode == PHY_TRANSMITTING,
        "PHY802.16: Unexpected node status!");

    // remove from the message list kept for cancel purpose
    std::list<Message*>::iterator it;
    for (it = phyDot16->txEndMsgList->begin();
         it != phyDot16->txEndMsgList->end();
         it ++)
    {
        if ((*it) == msg)
        {
            phyDot16->txEndMsgList->erase(it);
            break;
        }
    }

    phyDot16->numTxMsgs --;

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);
    // if all transmissions end, clean the status
    if (phyDot16->numTxMsgs == 0)
    {
        PhyDot16SignalInterference(
            node,
            phyIndex,
            channelIndex,
            NULL,
            NULL,
            phyDot16->interferencePower_mW);

        //GuiStart
        if (node->guiOption == TRUE)
        {
            GUI_EndBroadcast(node->nodeId,
                             GUI_PHY_LAYER,
                             GUI_DEFAULT_DATA_TYPE,
                             thisPhy->macInterfaceIndex,
                             node->getNodeTime());
        }
        //GuiEnd

        if (!PHY_IsListeningToChannel(node, phyIndex, channelIndex))
        {
            PHY_StartListeningToChannel(node, phyIndex, channelIndex);
        }


    }
}

// Response to the channel listening status changes when
// MAC switches channels.
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param channelIndex  channel that the node starts/stops listening to
// \param startListening  TRUE if the node starts listening to the ch
//    FALSE if the node stops listening to the ch
//
void PhyDot16ChannelListeningSwitchNotification(Node *node,
                                                int phyIndex,
                                                int channelIndex,
                                                BOOL startListening)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16 *)thisPhy->phyVar;

    if (phyDot16 == NULL)
    {
        // not initialized yet, return;
        return;
    }

    if (startListening == TRUE)
    {
        PhyDot16TerminateCurrentTransmissions(node, phyIndex);

        PhyDot16SignalInterference(
            node,
            phyIndex,
            channelIndex,
            NULL,
            NULL,
            phyDot16->interferencePower_mW);

        PhyDot16ChangeState(node, phyIndex, PHY_IDLE);
        if (phyDot16->mode != phyDot16->previousMode)
        {
            PhyDot16ReportStatusToMac(node, phyIndex, phyDot16->mode);
        }
    }
    else if (phyDot16->mode != PHY_TRANSMITTING)
    {
        BOOL frameError;
        clocktype endSignalTime;
        PhyDot16TerminateCurrentReceive(node,
            phyIndex,
            FALSE,
            &frameError,
            &endSignalTime);
        PhyDot16ChangeState(node, phyIndex, PHY_TRX_OFF);
    }
}
/*As per section 10.3.4 and 8.4.6 (802.16-2004.pdf) Ps is calculated to
   send RTG and TTG value */
// Calculate PS and convert RTG and TTG to PS
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param rtgTtgValue  value of RTG/TTG in clocktype
//
// \return number of PS
unsigned char PhyDot16ConvertTTGRTGinPS(
         Node* node,
         int phyIndex,
         clocktype rtgTtgValue)
{
    /*As per section 8.4.6 Fs = floor(8/7 * Bandwidth/8000)*8000
     and section 10.3.4 PS = 1/Fs */
    double samplingFrequency = 0;
    double Ps = 0;
    PhyDataDot16* phyDot16 =
        (PhyDataDot16*)(node->phyData[phyIndex]->phyVar);

    samplingFrequency =
            floor(PHY802_16_SAMPING_FACTOR * phyDot16->channelBandwidth /
            PHY802_16_BANDWIDTH_FACTOR) * PHY802_16_BANDWIDTH_FACTOR;
    Ps = (4 / samplingFrequency) * SECOND;
    unsigned char returnValue;
    returnValue = (unsigned char)(rtgTtgValue/Ps);
    return(returnValue);

}

// Calculate PS and convert RTG and TTG to time duration
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param rtgTtgValue  number of PS
//
// \return RTG/TTG in time duration
clocktype PhyDot16ConvertTTGRTGinclocktype(
         Node* node,
         int phyIndex,
         unsigned char rtgTtgValue)
{
    /*As per section 8.4.6 Fs = floor(8/7 * Bandwidth/8000)*8000
     and section 10.3.4 PS = 1/Fs */
    double samplingFrequency = 0;
    double Ps = 0;
    PhyDataDot16* phyDot16 =
        (PhyDataDot16*)(node->phyData[phyIndex]->phyVar);

    samplingFrequency =
            floor(PHY802_16_SAMPING_FACTOR * phyDot16->channelBandwidth /
            PHY802_16_BANDWIDTH_FACTOR) * PHY802_16_BANDWIDTH_FACTOR;
    Ps = (4 / samplingFrequency) * SECOND;
    clocktype returnValue = (clocktype)(rtgTtgValue * Ps);
    return(returnValue);

}

// Handle the interference signal subchannel mapping
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param channelIndex  Index of the channel receiving signal from
// \param propRxInfo  Propagation information
//
// \return mapped subchannel

static
int PhyDot16GetInterferenceSubchannelMapping(
         Node* node,
         int phyIndex,
         int channelIndex,
         PropRxInfo *propRxInfo)
{
    PhyDataDot16* phyDot16 = (PhyDataDot16*)node->phyData[phyIndex]->phyVar;

    double receivingFrequency =
                    PHY_GetFrequency(
                        node,
                        channelIndex);

    double signalFrequency = propRxInfo->frequency;

    int numSubchannel =
            PhyDot16GetNumberSubchannels(
                                    node,
                                    phyIndex);

    double freqDiff =  signalFrequency - receivingFrequency;

    if (fabs(freqDiff) > phyDot16->channelBandwidth)
    {
        if (freqDiff > 0.0)
        {
            freqDiff = phyDot16->channelBandwidth;
        }
        else {
           freqDiff = - phyDot16->channelBandwidth;
        }
    }

    double bandwidthPerSubchannel =
            phyDot16->channelBandwidth / double (numSubchannel);

    double subchannelDiff = freqDiff / bandwidthPerSubchannel;

    int subchannelMapping = 0;

    if (subchannelDiff > 0.0)
    {
        subchannelMapping = (int) floor(subchannelDiff);
    }
    else {

        subchannelMapping = (int) floor(subchannelDiff) + 1;
    }

    return subchannelMapping;
}




// Handle interference signal subchannel mapping
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param channelIndex  Index of the channel receiving interference signal from
// \param propRxInfo  Propagation information
// \param rxMsgInfo  Pointer to the received message info
// \param subchannelStart  Mapped subchannel start
//
// \return Mapped subchannel end

static
int PhyDot16MappingSubchannel(
         Node* node,
         int phyIndex,
         int channelIndex,
         PropRxInfo *propRxInfo,
         PhyDot16RxMsgInfo* rxMsgInfo,
         int *subchannelStart)
{
    int subchannelMapping = 0;
    int numSubchannel = 0;
    int endSubchannel = 0;
    int startSubchannel = 0;

    subchannelMapping =
            PhyDot16GetInterferenceSubchannelMapping(
                                     node,
                                     phyIndex,
                                     channelIndex,
                                     propRxInfo);

    numSubchannel =
            PhyDot16GetNumberSubchannels(
                                    node,
                                    phyIndex);

    endSubchannel =  rxMsgInfo->endSubchannel + subchannelMapping;
    startSubchannel = rxMsgInfo->startSubchannel + subchannelMapping;

    if (endSubchannel <= 0)
    {
        endSubchannel = 0;
        startSubchannel = 0;
    }

    if (endSubchannel > numSubchannel)
    {
        endSubchannel = numSubchannel;
    }

    if (startSubchannel < 0)
    {
        startSubchannel = 0;
    }

    if (startSubchannel >= numSubchannel)
    {
        startSubchannel = numSubchannel;
        endSubchannel = numSubchannel;
    }

    *subchannelStart = startSubchannel;

    return endSubchannel;
}


// Handle interference signal arrival from the chanel
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param channelIndex  Index of the channel receiving signal from
// \param propRxInfo  Propagation information
// \param sigPower_mw  The inband interference signal power in mW
//
void PhyDot16InterferenceArrivalFromChannel(
         Node* node,
         int phyIndex,
         int channelIndex,
         PropRxInfo *propRxInfo,
         double sigPower_mW)
{
    PhyDataDot16* phyDot16 = (PhyDataDot16*)node->phyData[phyIndex]->phyVar;
    PhyDot16RxMsgInfo* rxMsgInfo;
    double interferencePower_mW;
    int i;
    int endSubchannel = 0;
    int startSubchannel = 0;

    PhyDot16CheckRxPacketError(node, phyDot16);

    interferencePower_mW =
        NON_DB(ANTENNA_GainForThisSignal(node, phyIndex, propRxInfo)
        + IN_DB(sigPower_mW));

    // get rx msg info of the interference signal
    rxMsgInfo = PhyDot16CreateRxMsgInfo(node,
                                        phyDot16,
                                        propRxInfo,
                                        interferencePower_mW);

    // add into the interference signal list
    if (phyDot16->ifMsgList == NULL)
    {
        phyDot16->ifMsgList = rxMsgInfo;
    }
    else
    {
        rxMsgInfo->next = phyDot16->ifMsgList;
        phyDot16->ifMsgList = rxMsgInfo;
    }

    endSubchannel = PhyDot16MappingSubchannel(
         node,
         phyIndex,
         channelIndex,
         propRxInfo,
         rxMsgInfo,
         &startSubchannel);

    double occupiedSubchannels =
            (double) endSubchannel - (double) startSubchannel + 1.0;

    rxMsgInfo->startSubchannel = startSubchannel;
    rxMsgInfo->endSubchannel = endSubchannel;

    ERROR_Assert(occupiedSubchannels > 0.0,
             "PHY802.16: Number of subchannels should be larger"
             " than zero!");

    // add to the interference power for used subchannels
    for (i = startSubchannel;
         i <= endSubchannel; i ++)
    {
        phyDot16->interferencePower_mW[i] +=
                    (interferencePower_mW / occupiedSubchannels);

    }
}

// Handle interference signal end from a chanel
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param channelIndex  Index of the channel receiving signal from
// \param propRxInfo  Propagation information
// \param sigPower_mW  The inband interference power in mW
//
void PhyDot16InterferenceEndFromChannel(
        Node* node,
        int phyIndex,
        int channelIndex,
        PropRxInfo *propRxInfo)
{
    PhyData *thisPhy = node->phyData[phyIndex];
    PhyDataDot16* phyDot16 = (PhyDataDot16*) thisPhy->phyVar;
    PhyDot16RxMsgInfo* rxMsgInfo;

    int i;

    PhyDot16CheckRxPacketError(node, phyDot16);

    // search it in interference signal list
    rxMsgInfo = PhyDot16GetRxMsgInfo(node,
                                     &(phyDot16->ifMsgList),
                                     propRxInfo->txMsg,
                                     TRUE);

    // if in interference list, delete from interference power
    if (rxMsgInfo != NULL)
    {
        for (i = rxMsgInfo->startSubchannel;
             i <= rxMsgInfo->endSubchannel; i ++)
        {
            phyDot16->interferencePower_mW[i] -=
                            rxMsgInfo->rxMsgPower_mW[i];

            if (phyDot16->interferencePower_mW[i] < 0.0)
            {
                phyDot16->interferencePower_mW[i] = 0.0;
            }
        }

        // free the rxMsgInfo
        PhyDot16FreeRxMsgInfo(node, rxMsgInfo);
        rxMsgInfo = NULL;
    }
}
