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
#include "phy_802_15_4.h"


#define DEBUG 0
#define KANDEB 0


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
void Phy802_15_4ReportExtendedStatusToMac(
        Node* node,
        Int32 phyNum,
        PhyStatusType status,
        clocktype receiveDuration,
        const Message* potentialIncomingPacket)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: phy.cpp in the #ReportExtendedStatusToMac#\n",node->getNodeTime(),node->nodeId);
}

    PhyData* thisPhy = node->phyData[phyNum];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*)thisPhy->phyVar;

    ERROR_Assert(status == phy802_15_4->mode,"Invalid Phy State");

    MAC_ReceivePhyStatusChangeNotification(node,
                                           thisPhy->macInterfaceIndex,
                                           phy802_15_4->previousMode,
                                           status,
                                           receiveDuration,
                                           potentialIncomingPacket);
}
// Change status of the PHY
//
// \param node  Pointer to node.
// \param phyNum  Index of the PHY
// \param status  New status of the PHY
//


static
void Phy802_15_4ChangeState(Node* node,
                            Int32 phyIndex,
                            PhyStatusType newStatus)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #ChangeState#\n",node->getNodeTime(),node->nodeId);
}

    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4 *)thisPhy->phyVar;

    phy802_15_4->previousMode = phy802_15_4->mode;
    phy802_15_4->mode = newStatus;

    Phy_ReportStatusToEnergyModel(node,
                                  phyIndex,
                                  phy802_15_4->previousMode,
                                  newStatus);
}


// Report status of the PHY to MAC
//
// \param node  Pointer to node.
// \param phyNum  Index of the PHY
// \param status  New status of the PHY
//
static /*inline*/
void Phy802_15_4ReportStatusToMac(Node* node,
                                  Int32 phyNum,
                                  PhyStatusType status)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #ReportStatusToMac#\n",node->getNodeTime(),node->nodeId);
}

    Phy802_15_4ReportExtendedStatusToMac(node,
                                         phyNum,
                                         status,
                                         0,
                                         NULL);
}


// **
// FUNCTION   :: Phy802_15_4LockSignal
// LAYER      :: PHY
// PURPOSE    :: Lock the signal
// PARAMETERS ::
// + node      : Node*         : Pointer to node.
// + phy802_15_4   : PhyData802_15_4*  : Pointer to PHY data
// + msg          :  Message*          : Pointer to Message
// + rxPower_mW    : double            : Received signal power in mW
// + rxEndTime     : clocktype         : Rx end time
// RETURN     :: void : NULL

static // inline//
void Phy802_15_4LockSignal(Node* node,
                           PhyData802_15_4* phy802_15_4,
                           Message* msg,
                           double rxPower_mW,
                           clocktype rxEndTime)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #LockSignal#\n",node->getNodeTime(),node->nodeId);
}

    phy802_15_4->rxMsg = msg;
    phy802_15_4->rxMsgError = FALSE;
    phy802_15_4->rxMsgPower_mW = rxPower_mW;
    phy802_15_4->rxTimeEvaluated = node->getNodeTime();
    phy802_15_4->rxEndTime = rxEndTime;
    phy802_15_4->stats.totalSignalsLocked++;
}

// Unlock the signal
//
// \param phy802_15_4  Pointer to PHY data
//
static // inline//
void Phy802_15_4UnlockSignal(PhyData802_15_4* phy802_15_4)
{
if (KANDEB){
	printf("Phy.cpp in the #UnlockSignal#\n");
}

    phy802_15_4->rxMsg = NULL;
    phy802_15_4->rxMsgError = FALSE;
    phy802_15_4->rxMsgPower_mW = 0.0;
    phy802_15_4->rxTimeEvaluated = 0;
    phy802_15_4->rxEndTime = 0;
    phy802_15_4->linkQuality = 0.0;
}

// check if the channel is busy
//
// \param phy802_15_4  Pointer to PHY data
//
// \return TRUE if channel is busy, FALSE otherwise

static /*inline*/
BOOL Phy802_15_4CarrierSensing(Node* node,
                               PhyData802_15_4* phy802_15_4)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #CarrierSensing#\n",node->getNodeTime(),node->nodeId);
}

    double rxSensitivity_mW = phy802_15_4->rxSensitivity_mW;
    if (phy802_15_4->interferencePower_mW > rxSensitivity_mW*100)
    {
        return TRUE;
    }

    return FALSE;
}
// check if Medium Is Idle
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//
// \return TRUE if medium is idle, FALSE otherwise

BOOL Phy802_15_4MediumIsIdle(Node* node,
                             Int32 phyIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #MediumIsIdle#\n",node->getNodeTime(),node->nodeId);
}

    BOOL mediumIsIdle = TRUE;
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;

    double ED_threshold_mW = phy802_15_4->EDthreshold_mW;
    double rxSensitivity_mW = phy802_15_4->rxSensitivity_mW;

    if (phy802_15_4->CCAmode == ENERGY_ABOVE_THRESHOLD)
    {
        if (phy802_15_4->interferencePower_mW > ED_threshold_mW)
        {
            mediumIsIdle = FALSE;
        }
    }
    else if (phy802_15_4->CCAmode == CARRIER_SENSE)
    {
        if (phy802_15_4->interferencePower_mW > rxSensitivity_mW)
        {
            mediumIsIdle = FALSE;
        }
    }
    else if (phy802_15_4->CCAmode
                == CARRIER_SENSE_WITH_ENERGY_ABOVE_THRESHOLD)
    {
        if ((phy802_15_4->interferencePower_mW > ED_threshold_mW)
            && (phy802_15_4->interferencePower_mW > rxSensitivity_mW))
        {
            mediumIsIdle = FALSE;
        }
    }
    else
    {
        ERROR_ReportError("PHY802.15.4: Illegal CCA mode\n");
    }

    return mediumIsIdle;
}


// Function used to calculate BER
//
// \param n  Pointer to node.
// \param k  Index of the PHY
//
// \return calculate result
static
double combinationCalculation(Int32 n, Int32 k)
{
    double combination = 0;
    double numerator = 1.0;
    double denominator = 1.0;
    Int32 i = 0;

    for (i = 1; i < k + 1; i ++)
    {
        denominator = denominator * i;
    }

    for (i = n-k+1; i < n+1; i++)
    {
        numerator = numerator * i;
    }

    combination = numerator / denominator;
    return combination;
}



// Function used to calculate BER for O_QPSK
//
// \param sinr  Pointer to node.
// \param exponentfactor  Index of the PHY
//
// \return ber result
static
double O_QPSKberCalculation (double sinr, double exponentfactor)
{
    double ber = 0.0;
    double combinationpart = 0.0;
    double exponentpart = 0.0;
    double exponent = 0.0;
    Int32 upperBound = 0;
    Int32 i = 0;

    upperBound = PHY802_15_4_BER_UPPER_BOUND_SUMMATION;

    for (i = 2; i < upperBound; i++)
    {
        exponent = exponentfactor * sinr * ((1.0 / (double) i) - 1.0);
        combinationpart = combinationCalculation(upperBound - 1, i);
        exponentpart = exp(exponent);
        ber = ber + pow(-1.0, (double) i)*combinationpart * exponentpart;
    }

    ber = ber * PHY802_15_4_BER_CONSTANT;
    return ber;

}
// Check packet reception quality
//
// \param node  Pointer to node.
// \param phy802_15_4  Pointer to PHY data
// \param sinrp  Pointer to double
// \param phyIndex  Index of the PHY
//
static // inline//
BOOL Phy802_15_4CheckRxPacketError(Node* node,
                                   PhyData802_15_4* phy802_15_4,
                                   double* sinrp,
                                   Int32 phyIndex,
                                    BOOL isHeader,
                                    double *rxPower_mW)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #CheckRxPacketError#\n",node->getNodeTime(),node->nodeId);
}

    double sinr = 0.0;
    double BER = 0.0;
    BOOL packeterror = FALSE;
    Int32 channelIndex = 0;
    double exponentFactor = 0.0;
    PhyData* thisPhy = node->phyData[phyIndex];

    ERROR_Assert(phy802_15_4->rxMsgError == FALSE,"phy802_15_4->rxMsgError"
        " should be FALSE");

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);
    PropProfile *propProfile = node->propChannel[channelIndex].profile;

    if (isHeader)
    {
        sinr = *rxPower_mW /
            (phy802_15_4->interferencePower_mW +
             phy802_15_4->noisePower_mW);
    }
    else {

        sinr = phy802_15_4->rxMsgPower_mW /
            (phy802_15_4->interferencePower_mW +
             phy802_15_4->noisePower_mW);
    }

    if (thisPhy->phyRxModel == BER_BASED)
    {
        BER = PHY_BER(phy802_15_4->thisPhy, 0, sinr);
    }
    else if (thisPhy->phyRxModel == RX_802_15_4)
    {
        exponentFactor = PHY802_15_4_DEFAULT_EXPONENT_FACTOR;

        if ((propProfile->frequency >= PHY802_15_4_2400MHZ_RANGE_START)
            && (propProfile->frequency <= PHY802_15_4_2400MHZ_RANGE_END))
        {
            exponentFactor = PHY802_15_4_2400MHZ_DEFAULT_EXPONENT_FACTOR;
        }

        if ((propProfile->frequency >= PHY802_15_4_2400MHZ_RANGE_START)
            && (propProfile->frequency <= PHY802_15_4_2400MHZ_RANGE_END))
        {
            BER = O_QPSKberCalculation ( sinr, exponentFactor);
        }
        else if ((propProfile->frequency >= PHY802_15_4_900MHZ_RANGE_START)
            && (propProfile->frequency <= PHY802_15_4_900MHZ_RANGE_END))
        {
            if (phy802_15_4->modulation == BPSK)
            {
                BER = 0.5 * exp(-11.25 * sinr);
            }
            else if (phy802_15_4->modulation == O_QPSK)
            {
                BER = O_QPSKberCalculation (sinr, exponentFactor);
            }
            else if (phy802_15_4->modulation == ASK)
            {
                BER = 7.768 * exp(-21.93 * sinr)
                        - 12.85 * exp(-27.53 * sinr);
                if (BER < 0.0)
                {
                    BER = PHY802_15_4_DEFAULT_BER_CONSTANT;
                }
            }
            else
            {
                ERROR_ReportError(
                    "PHY802.15.4: Illegal modulation scheme\n");
            }
        }
        else if ((propProfile->frequency >= PHY802_15_4_800MHZ_RANGE_START)
            && (propProfile->frequency <= PHY802_15_4_800MHZ_RANGE_END))
        {
            if (phy802_15_4->modulation == BPSK)
            {
                BER = 0.5 * exp(-11.25 * sinr);
            }
            else if (phy802_15_4->modulation == O_QPSK)
            {
                BER = O_QPSKberCalculation (sinr, exponentFactor);
            }
            else if (phy802_15_4->modulation == ASK)
            {
                BER = 0.4146 * exp(-6.0871 * sinr);
            }
            else
            {
                ERROR_ReportError(
                    "PHY802.15.4: Illegal modulation scheme\n");
            }
        }
    }
    else
    {
         ERROR_ReportError(
        "PHY802.15.4: Illegal PHY-RX-MODEL\n");
    }

    if (sinrp)
    {
        *sinrp = sinr;
    }

    if (DEBUG) {
        printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4PHY :"
                " calculated SINR of %f for "
                "current signal\n",
                node->getNodeTime(), node->nodeId, IN_DB(sinr));
        fflush(stdout);
    }

    if (node->guiOption)
    {
        GUI_SendRealData(node->nodeId,
                         phy802_15_4->stats.currentSINRId,
                         IN_DB(sinr),
                         node->getNodeTime());
    }

    if (BER != 0.0)
    {
        double numBits = 0.0;

        if (isHeader)
        {
            numBits = ((double)(phy802_15_4->preambleHeaderPeriod)
                   * (double)phy802_15_4->dataRate /(double)SECOND);
        }
        else {
            numBits =
                 ((double)(node->getNodeTime() - phy802_15_4->rxTimeEvaluated)
                      * (double)phy802_15_4->dataRate /(double)SECOND);
        }

        double errorProbability = 1.0 - pow((1.0 - BER), numBits);
        double rand = RANDOM_erand(phy802_15_4->thisPhy->seed);

        ERROR_Assert((errorProbability >= 0.0) && (errorProbability <= 1.0),
            "Incorrect error probability");

        if (errorProbability > rand)
        {
            packeterror = TRUE;
        }
    } // if BER

    return packeterror;
}


// Initialize configurable parameters
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param nodeInput  Pointer to node input
//
void Phy802_15_4Init(Node* node,
                     const Int32 phyIndex,
                     const NodeInput* nodeInput)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #Init#\n",node->getNodeTime(),node->nodeId);
}

    double txPower_dBm = 0.0;
    BOOL   wasFound = FALSE;
    Int32 i = 0;
    char buf[MAX_STRING_LENGTH];

    Int32 channelIndex = 0;
    double bandwidth = 0.0;
    Int32 symbolRate = 0;
    Int32 bitsPerSymbol = 0;
    double preambleHeaderLength = 0.0;
    double threshold = 0.0;

    Int32 numChannels = PROP_NumberChannels(node);

    PhyData802_15_4* phy802_15_4
        = (PhyData802_15_4 *)MEM_malloc(sizeof(PhyData802_15_4));
    memset(phy802_15_4, 0, sizeof(PhyData802_15_4));

    node->phyData[phyIndex]->phyVar = (void *)phy802_15_4;

    phy802_15_4->thisPhy = node->phyData[phyIndex];
    bandwidth = PHY802_15_4_DEFAULT_CHANNEL_BANDWIDTH;

    // Setting up the channel to use for both TX and RX
    for (i = 0; i < numChannels; i++)
    {
        if (PHY_IsListeningToChannel(node, phyIndex, i))
        {
            break;
        }
    }
    ERROR_Assert(i != numChannels,"Invalid channel number");
    PHY_SetTransmissionChannel(node, phyIndex, i);

    // Antenna model initialization
    ANTENNA_Init(node, phyIndex, nodeInput);

    ERROR_Assert(((phy802_15_4->thisPhy->antennaData->antennaModelType
                == ANTENNA_OMNIDIRECTIONAL) ||
                (phy802_15_4->thisPhy->antennaData->antennaModelType
                == ANTENNA_PATTERNED)) ,
            "Illegal antennaModelType.\n");

    // Set phy_802_15_4-TX-POWER
    IO_ReadDouble(node,
                  node->nodeId,
                  phy802_15_4->thisPhy->macInterfaceIndex,
                  nodeInput,
                  "PHY802.15.4-TX-POWER",
                  &wasFound,
                  &txPower_dBm);

    if (wasFound)
    {
        phy802_15_4->txPower_dBm = (float)txPower_dBm;
    }
    else
    {
        phy802_15_4->txPower_dBm = PHY802_15_4_DEFAULT_TX_POWER_dBm;
    }

    IO_ReadString(node,
                  node->nodeId,
                  phy802_15_4->thisPhy->macInterfaceIndex,
                  nodeInput,
                  "PHY802.15.4-MODULATION",
                  &wasFound,
                  buf);

    if (wasFound)
    {
        if (strcmp(buf, "O-QPSK") == 0)
        {
            phy802_15_4->modulation = O_QPSK;
        }
        else if (strcmp(buf, "BPSK") == 0)
        {
            phy802_15_4->modulation = BPSK;
        }
        else if (strcmp(buf, "ASK") == 0)
        {
            phy802_15_4->modulation = ASK;
        }
        else
        {
            ERROR_ReportError("Unknown Modulation Scheme");
        }
    }
    else
    {
        phy802_15_4->modulation = PHY802_15_4_DEFAULT_MODULATION;
    }

    IO_ReadString(node,
                  node->nodeId,
                  phy802_15_4->thisPhy->macInterfaceIndex,
                  nodeInput,
                  "PHY802.15.4-CCA-MODE",
                  &wasFound,
                  buf);

    if (wasFound)
    {
        if (strcmp(buf, "ENERGY-ABOVE-THRESHOLD") == 0)
        {
            phy802_15_4->CCAmode = ENERGY_ABOVE_THRESHOLD;
            if (phy802_15_4->modulation == BPSK)
            {
                phy802_15_4->EDthreshold_mW =
                    NON_DB(PHY802_15_4_BPSK_ED_THRESHOLD);
            }
            else
            {
                phy802_15_4->EDthreshold_mW =
                    NON_DB(PHY802_15_4_ED_THRESHOLD);
            }
        }
        else if (strcmp(buf, "CARRIER-SENSE") == 0)
        {
            phy802_15_4->CCAmode = CARRIER_SENSE;
        }
        else if (strcmp(buf,
                 "CARRIER-SENSE-WITH-ENERGY-ABOVE-THRESHOLD") == 0)
        {
            phy802_15_4->CCAmode =
                CARRIER_SENSE_WITH_ENERGY_ABOVE_THRESHOLD;
        }
        else
        {
            ERROR_ReportError("Unknown PHY802.15.4-CCA-MODE");
        }
    }
    else
    {
        phy802_15_4->CCAmode =  PHY802_15_4_DEFAULT_CCA_MODE;
    }

    // Initialize phy statistics variables
    phy802_15_4->stats.totalTxSignals         = 0;
    phy802_15_4->stats.totalSignalsDetected   = 0;
    phy802_15_4->stats.totalSignalsLocked     = 0;
    phy802_15_4->stats.totalSignalsWithErrors = 0;
    phy802_15_4->stats.totalRxSignalsToMac    = 0;
    phy802_15_4->stats.energyConsumed         = 0.0;
    phy802_15_4->stats.turnOnTime             = node->getNodeTime();
    phy802_15_4->stats.turnOnPeriod           = 0;

#ifdef CYBER_LIB
    phy802_15_4->stats.totalRxSignalsToMacDuringJam = 0;
    phy802_15_4->stats.totalSignalsLockedDuringJam = 0;
    phy802_15_4->stats.totalSignalsWithErrorsDuringJam = 0;
#endif

    if (node->guiOption)
    {
         phy802_15_4->stats.totalTxSignalsId
             = GUI_DefineMetric("802.15.4 PHY: Number of Signals Transmitted",
                                node->nodeId,
                                GUI_PHY_LAYER,
                                0,
                                GUI_INTEGER_TYPE,
                                GUI_CUMULATIVE_METRIC);

         phy802_15_4->stats.totalSignalsDetectedId
             = GUI_DefineMetric("802.15.4 PHY: Number of Signals Detected",
                                node->nodeId,
                                GUI_PHY_LAYER,
                                0,
                                GUI_INTEGER_TYPE,
                                GUI_CUMULATIVE_METRIC);

         phy802_15_4->stats.totalSignalsLockedId
             = GUI_DefineMetric("802.15.4 PHY: Number of Signals Locked onto",
                                node->nodeId,
                                GUI_PHY_LAYER,
                                0,
                                GUI_INTEGER_TYPE,
                                GUI_CUMULATIVE_METRIC);

         phy802_15_4->stats.totalSignalsWithErrorsId
             = GUI_DefineMetric("802.15.4 PHY: Number of Signals Received"
                                " with Errors",
                                node->nodeId,
                                GUI_PHY_LAYER,
                                0,
                                GUI_INTEGER_TYPE,
                                GUI_CUMULATIVE_METRIC);

         phy802_15_4->stats.totalRxSignalsToMacId
             = GUI_DefineMetric("802.15.4 PHY: Number of Signals Forwarded"
                                "to MAC layer",
                                node->nodeId,
                                GUI_PHY_LAYER,
                                0,
                                GUI_INTEGER_TYPE,
                                GUI_CUMULATIVE_METRIC);

         phy802_15_4->stats.currentSINRId
             = GUI_DefineMetric("802.15.4 PHY: Signal-to-Noise Ratio (dB)",
                                node->nodeId,
                                GUI_PHY_LAYER,
                                0,
                                GUI_DOUBLE_TYPE,
                                GUI_AVERAGE_METRIC);

         phy802_15_4->stats.energyConsumedId
             = GUI_DefineMetric("802.15.4 PHY: Energy Consumption (mWhr)",
                                node->nodeId,
                                GUI_PHY_LAYER,
                                0,
                                GUI_DOUBLE_TYPE,
                                GUI_CUMULATIVE_METRIC);
    }

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);
    PropProfile *propProfile = node->propChannel[channelIndex].profile;

// set phy parameters such as data rate, preamble and header time and
// spread factor for different frequency bands and modulation schemes

    if ((propProfile->frequency >= PHY802_15_4_800MHZ_RANGE_START)
        && (propProfile->frequency <= PHY802_15_4_800MHZ_RANGE_END))
    {
        if (phy802_15_4->modulation == BPSK)
        {
            symbolRate = PHY802_15_4_800MHZ_BPSK_SYMBOL_RATE;
            bitsPerSymbol = PHY802_15_4_BPSK_BITS_PER_SYMBOL;
            phy802_15_4->spreadFactor = PHY802_15_4_BPSK_SPREAD_FACTOR;
            threshold = PHY802_15_4_BPSK_THRESHOLD;
            preambleHeaderLength = PHY802_15_4_BPSK_PREAMBLE_LENGTH
                                    + PHY802_15_4_BPSK_SFD_LENGTH
                                    + PHY802_15_4_PHR_LENGTH / bitsPerSymbol;
        }
        else if (phy802_15_4->modulation == ASK)
        {
            symbolRate = PHY802_15_4_800MHZ_ASK_SYMBOL_RATE;
            bitsPerSymbol = PHY802_15_4_800MHZ_ASK_BITS_PER_SYMBOL;
            phy802_15_4->spreadFactor = PHY802_15_4_ASK_SPREAD_FACTOR;
            preambleHeaderLength = PHY802_15_4_800MHZ_ASK_PREAMBLE_LENGTH
                                    + PHY802_15_4_ASK_SFD_LENGTH
                                    + PHY802_15_4_PHR_LENGTH / bitsPerSymbol;
        }
        else if (phy802_15_4->modulation == O_QPSK)
        {
            symbolRate = PHY802_15_4_800MHZ_O_QPSK_SYMBOL_RATE;
            bitsPerSymbol = PHY802_15_4_O_QPSK_BITS_PER_SYMBOL;
            phy802_15_4->spreadFactor = PHY802_15_4_O_QPSK_SPREAD_FACTOR;
            threshold = PHY802_15_4_O_QPSK_THRESHOLD;
            preambleHeaderLength = PHY802_15_4_O_QPSK_PREAMBLE_LENGTH
                                    + PHY802_15_4_O_QPSK_SFD_LENGTH
                                    + PHY802_15_4_PHR_LENGTH / bitsPerSymbol;
        }
        else
        {
            ERROR_ReportError("PHY802.15.4:\n Illegal modulation scheme\n"
                              "only BPSK, ASK, and O-QPSK are supported\n"
                              "in this frequency band\n" );
        }
    }
    else if ((propProfile->frequency >= PHY802_15_4_900MHZ_RANGE_START)
        && (propProfile->frequency <= PHY802_15_4_900MHZ_RANGE_END))
    {
        if (phy802_15_4->modulation == BPSK)
        {
            symbolRate = PHY802_15_4_900MHZ_BPSK_SYMBOL_RATE;
            bitsPerSymbol = PHY802_15_4_BPSK_BITS_PER_SYMBOL;
            phy802_15_4->spreadFactor = PHY802_15_4_BPSK_SPREAD_FACTOR;
            threshold = PHY802_15_4_BPSK_THRESHOLD;
            preambleHeaderLength = PHY802_15_4_BPSK_PREAMBLE_LENGTH
                                    + PHY802_15_4_BPSK_SFD_LENGTH
                                    + PHY802_15_4_PHR_LENGTH / bitsPerSymbol;
        }
        else if (phy802_15_4->modulation == ASK)
        {
            symbolRate = PHY802_15_4_900MHZ_ASK_SYMBOL_RATE;
            bitsPerSymbol = PHY802_15_4_900MHZ_ASK_BITS_PER_SYMBOL;
            phy802_15_4->spreadFactor = PHY802_15_4_ASK_SPREAD_FACTOR;
            threshold = PHY802_15_4_900MHZ_ASK_THRESHOLD;
            preambleHeaderLength = PHY802_15_4_900MHZ_ASK_PREAMBLE_LENGTH
                                    + PHY802_15_4_ASK_SFD_LENGTH
                                    + PHY802_15_4_PHR_LENGTH / bitsPerSymbol;
        }
        else if (phy802_15_4->modulation == O_QPSK)
        {
            symbolRate = PHY802_15_4_O_QPSK_SYMBOL_RATE;
            bitsPerSymbol = PHY802_15_4_O_QPSK_BITS_PER_SYMBOL;
            phy802_15_4->spreadFactor = PHY802_15_4_O_QPSK_SPREAD_FACTOR;
            threshold = PHY802_15_4_O_QPSK_THRESHOLD;
            preambleHeaderLength = PHY802_15_4_O_QPSK_PREAMBLE_LENGTH
                                    + PHY802_15_4_O_QPSK_SFD_LENGTH
                                    + PHY802_15_4_PHR_LENGTH / bitsPerSymbol;
        }
        else
        {
            ERROR_ReportError("PHY802.15.4:\n Illegal modulation scheme\n"
                              "only BPSK, ASK and O-QPSK are supported\n"
                              "in this frequency band\n" );
        }
    }
    else if ((propProfile->frequency >= PHY802_15_4_2400MHZ_RANGE_START)
        && (propProfile->frequency <= PHY802_15_4_2400MHZ_RANGE_END))
    {
        if (phy802_15_4->modulation == O_QPSK)
        {
            symbolRate = PHY802_15_4_O_QPSK_SYMBOL_RATE;
            bitsPerSymbol = PHY802_15_4_O_QPSK_BITS_PER_SYMBOL;
            phy802_15_4->spreadFactor = PHY802_15_4_2400MHZ_O_QPSK_SPREAD_FACTOR;
            threshold = PHY802_15_4_2400MHZ_O_QPSK_THRESHOLD;
            preambleHeaderLength = PHY802_15_4_O_QPSK_PREAMBLE_LENGTH
                                    + PHY802_15_4_O_QPSK_SFD_LENGTH
                                    + PHY802_15_4_PHR_LENGTH / bitsPerSymbol;
        }
        else {
            ERROR_ReportError("PHY802.15.4:\n Illegal modulation scheme\n"
                    "only O-QPSK is supported in this frequency band\n" );
        }
    }
     else
     {
        char buf[MAX_STRING_LENGTH];

        sprintf(buf,
               "PHY802.15.4:\n"
               "    the frequency %5.3f MHz is not in the specified range,\n"
               "    it should be in the range:\n"
               "          868-868.6 MHz or\n"
               "          902-928 MHZ or\n"
               "          2400-2483.5 MHz\n", propProfile->frequency / 1.0e6);
        ERROR_ReportError(buf);
    }

    phy802_15_4->symbolPeriod = SECOND / symbolRate;
    phy802_15_4->dataRate = symbolRate * bitsPerSymbol;
    phy802_15_4->preambleHeaderPeriod = (clocktype)
                        (phy802_15_4->symbolPeriod * preambleHeaderLength);

    phy802_15_4->RxTxTurnAroundTime
        = PHY802_15_4_RX_TX_TURNAROUND_TIME_IN_SYMBOL_PERIODS
            * phy802_15_4->symbolPeriod;

    // Initialize status of phy
    if ((propProfile->frequency >= PHY802_15_4_800MHZ_RANGE_START)
        && (propProfile->frequency <= PHY802_15_4_800MHZ_RANGE_END))
    {
        bandwidth = PHY802_15_4_800MHZ_DEFAULT_CHANNEL_BANDWIDTH;
    }

    phy802_15_4->rxMsg = NULL;
    phy802_15_4->rxMsgError = FALSE;
    phy802_15_4->rxMsgPower_mW = 0.0;
    phy802_15_4->interferencePower_mW = 0.0;
    phy802_15_4->noisePower_mW = phy802_15_4->thisPhy->noise_mW_hz * bandwidth;
    phy802_15_4->rxTimeEvaluated = 0;
    phy802_15_4->rxEndTime = 0;
    phy802_15_4->txOnPeriod = 0;
    phy802_15_4->rxOnPeriod = 0;
    phy802_15_4->turnOffPeriod = 0;
    phy802_15_4->linkQuality = 0.0;
    phy802_15_4->previousMode = PHY_IDLE;
    phy802_15_4->mode = PHY_IDLE;

    Phy802_15_4ChangeState(node,phyIndex, PHY_IDLE);
    phy802_15_4->rxSensitivity_mW = phy802_15_4->noisePower_mW * threshold;
    return;
}



// Handle signal arrival from the channel
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param channelIndex  Index of the channel receiving signal from
// \param propRxInfo  Propagation information
//
void Phy802_15_4SignalArrivalFromChannel(
    Node* node,
    Int32 phyIndex,
    Int32 channelIndex,
    PropRxInfo* propRxInfo)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #SignalArrivalFromChannel#\n",node->getNodeTime(),node->nodeId);
}

    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;

    if ((phy802_15_4->mode != PHY_TRX_OFF))
    {
        double rxPower_mW
            = NON_DB(ANTENNA_GainForThisSignal(node, phyIndex, propRxInfo)
                + propRxInfo->rxPower_dBm);

        phy802_15_4->stats.totalSignalsDetected++;

        if (node->guiOption)
        {
            GUI_SendUnsignedData(node->nodeId,
                                 phy802_15_4->stats.totalSignalsDetectedId,
                                 phy802_15_4->stats.totalSignalsDetected,
                                 node->getNodeTime());
        }

        switch (phy802_15_4->mode)
        {
            case PHY_RECEIVING:
            case PHY_BUSY_RX:
            {
                if (DEBUG) {
                    printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4PHY :"
                            " Already in receiving mode (%d)\n",
                            node->getNodeTime(),
                            node->nodeId,
                            phy802_15_4->mode);
                    fflush(stdout);
                }

                if (!phy802_15_4->rxMsgError) {

                    BOOL isheader = FALSE;

                    phy802_15_4->rxMsgError
                        = Phy802_15_4CheckRxPacketError(node,
                                                        phy802_15_4,
                                                        NULL,
                                                        phyIndex,
                                                        isheader,
                                                        NULL);
                } // if//

                phy802_15_4->rxTimeEvaluated = node->getNodeTime();
                phy802_15_4->interferencePower_mW += rxPower_mW;
                break;
            }

            // If the phy is idle or sensing,
            // check if it can receive this signal.

            case PHY_SUCCESS:
            case PHY_IDLE:
            case PHY_SENSING:
            {
                PHY_SignalInterference(
                         node,
                         phyIndex,
                         channelIndex,
                         propRxInfo->txMsg,
                         &rxPower_mW,
                         &(phy802_15_4->interferencePower_mW));

                BOOL isheader = TRUE;

                BOOL isHeaderError =
                     Phy802_15_4CheckRxPacketError(
                         node,
                         phy802_15_4,
                         NULL,
                         phyIndex,
                         isheader,
                         &rxPower_mW);

                if ((rxPower_mW >= phy802_15_4->rxSensitivity_mW) &&
                    (!isHeaderError))
                {
                    Phy802_15_4LockSignal(
                        node,
                        phy802_15_4,
                        propRxInfo->txMsg,
                        rxPower_mW,
                        (propRxInfo->rxStartTime + propRxInfo->duration));

#ifdef CYBER_LIB
                    if (node->phyData[phyIndex]->jammerStatistics == TRUE)
                    {
                        if (node->phyData[phyIndex]->jamInstances > 0)
                        {
                            phy802_15_4->stats.totalSignalsLockedDuringJam++;
                        }
                    }
#endif

                    if (DEBUG)
                    {
                        printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4PHY"
                               " : locked on to new signal "
                                "from node %d with rxPower (dB) = %f\n",
                                node->getNodeTime(),
                                node->nodeId,
                                propRxInfo->txMsg->originatingNodeId,
                                propRxInfo->rxPower_dBm);
                        fflush(stdout);
                    }

                    if (node->guiOption)
                    {
                        GUI_SendRealData(node->nodeId,
                                   phy802_15_4->stats.totalSignalsLockedId,
                                   phy802_15_4->stats.totalSignalsLocked,
                                   node->getNodeTime());
                    }

                    Phy802_15_4ChangeState(node,phyIndex,PHY_RECEIVING);
                }
                else
                {
                    // Otherwise, check if the signal change its status
                    PhyStatusType newMode;

                    if (DEBUG)
                    {
                        printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4PHY"
                               " : unable to lock on to new signal "
                               "from node %d with rxPower (dB) = %f\n",
                               node->getNodeTime(),
                               node->nodeId,
                               propRxInfo->txMsg->originatingNodeId,
                               propRxInfo->rxPower_dBm);
                        fflush(stdout);
                    }

                    phy802_15_4->interferencePower_mW += rxPower_mW;

                    if (Phy802_15_4CarrierSensing(node, phy802_15_4))\
                    {
                        newMode = PHY_SENSING;
                    }
                    else
                    {
                        newMode = PHY_IDLE;
                    } // if//

                    if (newMode != phy802_15_4->mode)
                    {
                        Phy802_15_4ChangeState(node,phyIndex, newMode);
                    } // if//
               }// if//
               break;
            }
            default:
            {
                abort();
            }
        }   // switch (phy802_15_4->mode)//

        phy802_15_4->stats.turnOnPeriod
            = node->getNodeTime() - phy802_15_4->stats.turnOnTime;
    }
    else
    {
        if (DEBUG)
        {
            printf("%" TYPES_64BITFMT "d Node %d Physical of node %d is"
                   " switched off\n",
                   node->getNodeTime(),
                   node->nodeId,
                   node->nodeId);
        }
        double rxPower_mW
            = NON_DB(ANTENNA_GainForThisSignal(node, phyIndex, propRxInfo)
                + propRxInfo->rxPower_dBm);
        phy802_15_4->interferencePower_mW += rxPower_mW;
    }
}

// **
// FUNCTION   :: Phy802_15_4TerminateCurrentReceive
// LAYER      :: PHY
// PURPOSE    :: Terminate all signals current under receiving.
// PARAMETERS ::
// + node         : Node*        : Pointer to node.
// + phyIndex     : Int32          : Index of the PHY
// + terminateOnlyOnReceiveError : const BOOL : Only terminate if in error
// + frameError   : BOOL*        : For returning if frame is in error
// + endSignalTime: clocktype*   : For returning expected signal end time
// RETURN     :: void : NULL

void Phy802_15_4TerminateCurrentReceive(Node* node,
                                        Int32 phyIndex,
                                        const BOOL terminateOnlyOnReceiveError,
                                        BOOL* frameError,
                                        clocktype* endSignalTime)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #TerminateCurrentReceive#\n",node->getNodeTime(),node->nodeId);
}

    if (DEBUG)
    {
        printf("%" TYPES_64BITFMT "d Node %d Current Receive is Terminated\n",
                                                node->getNodeTime(),
                                                node->nodeId);
    }
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4 *)thisPhy->phyVar;
    double sinr = 0.0;

    *endSignalTime = phy802_15_4->rxEndTime;

    if (!phy802_15_4->rxMsgError)
    {
        BOOL isheader = FALSE;

        phy802_15_4->rxMsgError =
                Phy802_15_4CheckRxPacketError(
                        node,
                        phy802_15_4,
                        &sinr,
                        phyIndex,
                        isheader,
                        NULL);

    } // if//

    *frameError = phy802_15_4->rxMsgError;

    if ((terminateOnlyOnReceiveError) && (!phy802_15_4->rxMsgError)) {
        return;
    } // if//

    phy802_15_4->rxOnPeriod +=
        (node->getNodeTime() - phy802_15_4->rxTimeEvaluated);

#if 0
        {
        // added for energy model
        double duration = (double)(node->getNodeTime() -
                            phy802_15_4->rxTimeEvaluated)/(double)SECOND;
        // end
        }
#endif

    Phy802_15_4UnlockSignal(phy802_15_4);

    if (Phy802_15_4CarrierSensing(node, phy802_15_4))
    {
        Phy802_15_4ChangeState(node,phyIndex, PHY_SENSING);
    }
    else
    {
        Phy802_15_4ChangeState(node,phyIndex, PHY_IDLE);
    }// if//
    phy802_15_4->stats.turnOnPeriod
        = node->getNodeTime() - phy802_15_4->stats.turnOnTime;
}/* End of Phy802_15_4TerminateCurrentReceive */

// Handle signal end from a channel
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param channelIndex  Index of the channel receiving signal from
// \param propRxInfo  Propagation information
//

void Phy802_15_4SignalEndFromChannel(Node* node,
                                     Int32 phyIndex,
                                     Int32 channelIndex,
                                     PropRxInfo *propRxInfo)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #SignalEndFromChannel#\n",node->getNodeTime(),node->nodeId);
}

    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;
    double sinr = 0.0;

    PropProfile* propProfile = node->propChannel[channelIndex].profile;

    if (propProfile->enableChannelOverlapCheck) {

        PHY_SignalInterference(
                node,
                phyIndex,
                channelIndex,
                NULL,
                NULL,
                &(phy802_15_4->interferencePower_mW));
    }

    BOOL receiveErrorOccurred = FALSE;

    ERROR_Assert(phy802_15_4->mode != PHY_TRANSMITTING,"phy802_15_4->mode"
                    " should not be PHY_TRANSMITTING");

    if (phy802_15_4->mode == PHY_RECEIVING)
    {
        if (phy802_15_4->rxMsgError == FALSE)
        {
            BOOL isheader = FALSE;

            phy802_15_4->rxMsgError =
                Phy802_15_4CheckRxPacketError(
                    node,
                    phy802_15_4,
                    &sinr,
                    phyIndex,
                    isheader,
                    NULL);

             phy802_15_4->rxTimeEvaluated = node->getNodeTime();
        } // if
    } // if//

    receiveErrorOccurred = phy802_15_4->rxMsgError;

    // If the phy is still receiving this signal, forward the frame
    // to the MAC layer.

    if ((phy802_15_4->mode == PHY_RECEIVING)
        && (phy802_15_4->rxMsg == propRxInfo->txMsg))
    {
        Message* newMsg = NULL;
        PHY_SignalInterference(
            node,
            phyIndex,
            channelIndex,
            NULL,
            NULL,
            &(phy802_15_4->interferencePower_mW));

        phy802_15_4->linkQuality
            = phy802_15_4->rxMsgPower_mW
                / (phy802_15_4->interferencePower_mW
                    + phy802_15_4->noisePower_mW);

        Phy802_15_4UnlockSignal(phy802_15_4);

        if (!receiveErrorOccurred)
        {
            newMsg = MESSAGE_Duplicate(node, propRxInfo->txMsg);

            MESSAGE_SetInstanceId(newMsg, (short) phyIndex);
           if (Phy802_15_4CarrierSensing(node, phy802_15_4)) {

               Phy802_15_4ChangeState(node,phyIndex, PHY_SENSING);
            }
            else {
                Phy802_15_4ChangeState(node,phyIndex, PHY_IDLE);
            }// if//
            MAC_ReceivePacketFromPhy(
                node,
                node->phyData[phyIndex]->macInterfaceIndex,
                newMsg);

            phy802_15_4->stats.totalRxSignalsToMac++;
#ifdef CYBER_LIB
            if (node->phyData[phyIndex]->jammerStatistics == TRUE)
            {
                if (node->phyData[phyIndex]->jamInstances > 0)
                {
                    phy802_15_4->stats.totalRxSignalsToMacDuringJam++;
                }
            }
#endif
            if (node->guiOption)
            {
                GUI_SendUnsignedData(node->nodeId,
                             phy802_15_4->stats.totalRxSignalsToMacId,
                               phy802_15_4->stats.totalRxSignalsToMac,
                               node->getNodeTime());
            }
        }
        else
        {
            phy802_15_4->stats.totalSignalsWithErrors++;

#ifdef CYBER_LIB
            if (node->phyData[phyIndex]->jammerStatistics == TRUE)
            {
                if (node->phyData[phyIndex]->jamInstances > 0)
                {
                    phy802_15_4->stats.totalSignalsWithErrorsDuringJam++;
                }
            }
#endif

            if (Phy802_15_4CarrierSensing(node, phy802_15_4))
            {
                Phy802_15_4ChangeState(node,phyIndex, PHY_SENSING);
            }
            else
            {
                Phy802_15_4ChangeState(node,phyIndex, PHY_IDLE);
            }// if//

            if (node->guiOption)
            {
                GUI_SendUnsignedData(node->nodeId,
                           phy802_15_4->stats.totalSignalsWithErrorsId,
                           phy802_15_4->stats.totalSignalsWithErrors,
                           node->getNodeTime());
            }
        }
    }
    else
    {
        PhyStatusType newMode;

        double rxPower_mW
            = NON_DB(ANTENNA_GainForThisSignal(node, phyIndex, propRxInfo)
                + propRxInfo->rxPower_dBm);

        phy802_15_4->interferencePower_mW -= rxPower_mW;

        if (phy802_15_4->interferencePower_mW < 0.0)
        {
            phy802_15_4->interferencePower_mW = 0.0;
        }

        if (phy802_15_4->mode != PHY_RECEIVING)
        {
            if (phy802_15_4->mode != PHY_TRX_OFF)
            {
                if (Phy802_15_4CarrierSensing(node, phy802_15_4) == TRUE) {
                    newMode = PHY_SENSING;
                }
                else {
                    newMode = PHY_IDLE;
                } // else//
                if (newMode != phy802_15_4->mode) {
                    Phy802_15_4ChangeState(node,phyIndex, newMode);
                } // if//
            }
        } // if//
    }// else//

    phy802_15_4->rxOnPeriod += (node->getNodeTime()
                                - phy802_15_4->rxTimeEvaluated);

#if 0
    {
    // added for energy model
    double duration = (double)(node->getNodeTime() -
                        phy802_15_4->rxTimeEvaluated)/(double)SECOND;
    // end
    }
#endif

    phy802_15_4->stats.turnOnPeriod = node->getNodeTime()
                                        - phy802_15_4->stats.turnOnTime;

}



// Finalize the 802.15.4 PHY, print out statistics
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//
void Phy802_15_4Finalize(Node* node, const Int32 phyIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #Finalize#\n",node->getNodeTime(),node->nodeId);
}

    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;
    char buf[MAX_STRING_LENGTH];

    Phy802_15_4ChangeState(node,phyIndex, PHY_IDLE);
    if (thisPhy->phyStats == FALSE)
    {
        return;
    }

    ERROR_Assert(thisPhy->phyStats == TRUE,"PHY statistics should be enabled");

    sprintf(buf, "Signals transmitted = %d",
            phy802_15_4->stats.totalTxSignals);
    IO_PrintStat(node, "Physical", "802_15_4", ANY_DEST, phyIndex, buf);

    sprintf(buf, "Signals detected = %d",
            phy802_15_4->stats.totalSignalsDetected);
    IO_PrintStat(node, "Physical", "802_15_4", ANY_DEST, phyIndex, buf);

    sprintf(buf, "Signals locked on by PHY = %d",
            phy802_15_4->stats.totalSignalsLocked);
    IO_PrintStat(node, "Physical", "802_15_4", ANY_DEST, phyIndex, buf);

    sprintf(buf, "Signals received but with errors = %d",
            phy802_15_4->stats.totalSignalsWithErrors);
    IO_PrintStat(node, "Physical", "802_15_4", ANY_DEST, phyIndex, buf);

    sprintf(buf, "Signals received and forwarded to MAC = %d",
            phy802_15_4->stats.totalRxSignalsToMac);
    IO_PrintStat(node, "Physical", "802_15_4", ANY_DEST, phyIndex, buf);
#ifdef CYBER_LIB
    if (node->phyData[phyIndex]->jammerStatistics == TRUE)
    {
        char durationStr[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(node->phyData[phyIndex]->jamDuration, durationStr, node);
        sprintf(buf, "Signals received and forwarded to MAC during the jam duration = %d",
            phy802_15_4->stats.totalRxSignalsToMacDuringJam);
        IO_PrintStat(node, "Physical", "802_15_4", ANY_DEST, phyIndex, buf);
        sprintf(buf, "Signals locked on by PHY during the jam duration = %d",
            phy802_15_4->stats.totalSignalsLockedDuringJam);
        IO_PrintStat(node, "Physical", "802_15_4", ANY_DEST, phyIndex, buf);

        sprintf(buf, "Signals received but with errors during the jam duration = %d",
            phy802_15_4->stats.totalSignalsWithErrorsDuringJam);
        IO_PrintStat(node, "Physical", "802_15_4", ANY_DEST, phyIndex, buf);
        sprintf(buf, "Total jam duration in (s) = %s", durationStr);
        IO_PrintStat(node, "Physical", "802_15_4", ANY_DEST, phyIndex, buf);
    }
#endif
}

// End of the transmission
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//

void Phy802_15_4TransmissionEnd(Node* node, Int32 phyIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #TransmissionEnd#\n",node->getNodeTime(),node->nodeId);
}

    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4 *)thisPhy->phyVar;
    Int32 channelIndex = 0;

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);

    // GuiStart
    if (node->guiOption == TRUE)
    {
        GUI_EndBroadcast(node->nodeId,
                     GUI_PHY_LAYER,
                     GUI_DEFAULT_DATA_TYPE,
                     thisPhy->macInterfaceIndex,
                     node->getNodeTime());
    }
    // GuiEnd

    if (DEBUG)
    {
        printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4PHY :"
               " Transmission ends\n",
               node->getNodeTime(),
               node->nodeId);
               fflush(stdout);
    }

     ERROR_Assert(phy802_15_4->mode == PHY_TRANSMITTING,"phy802_15_4->mode"
         " should be PHY_TRANSMITTING");

    // Since we reached this function, the timer has already expired,
    // just set the msg pointer to NULL
    phy802_15_4->phyTimerEndMessage = NULL;

    PHY_StartListeningToChannel(node, phyIndex, channelIndex);

    PHY_SignalInterference(node,
                           phyIndex,
                           channelIndex,
                           NULL,
                           NULL,
                           &(phy802_15_4->interferencePower_mW));

    if (Phy802_15_4CarrierSensing(node, phy802_15_4))
    {
        Phy802_15_4ChangeState(node,phyIndex, PHY_SENSING);
    }
    else
    {
        Phy802_15_4ChangeState(node,phyIndex, PHY_IDLE);
    }// if//

    phy802_15_4->stats.turnOnPeriod = node->getNodeTime()
                                        - phy802_15_4->stats.turnOnTime;

    Phy802_15_4ReportStatusToMac(node, phyIndex, phy802_15_4->mode);
}

// Start transmitting a frame
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param packet  Frame to be transmitted
//    + useMacLayerSpecifiedDelay : Use MAC specified delay or calculate it
// \param initDelayUntilAirborne  The MAC specified delay
//

//
// Used by the MAC layer to start transmitting a packet.
//
void Phy802_15_4StartTransmittingSignal(
    Node* node,
    Int32 phyIndex,
    Message* packet,
    BOOL useMacLayerSpecifiedDelay,
    clocktype initDelayUntilAirborne)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #StartTransmittingSignal#\n",node->getNodeTime(),node->nodeId);
}

    clocktype delayUntilAirborne = initDelayUntilAirborne;
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*)thisPhy->phyVar;
    Int32 channelIndex = 0;
    Message* endMsg = NULL;
    Int32 packetsize = MESSAGE_ReturnPacketSize(packet);
    clocktype duration = 0;

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);

    duration = Phy802_15_4GetFrameDuration(thisPhy,
                                          packetsize,
                                          (Int32)PHY_GetTxDataRate(node, phyIndex));
    if (!useMacLayerSpecifiedDelay) {
        delayUntilAirborne = phy802_15_4->RxTxTurnAroundTime;
    } // if//

    if (phy802_15_4->mode == PHY_RECEIVING)
    {
        phy802_15_4->interferencePower_mW += phy802_15_4->rxMsgPower_mW;
        Phy802_15_4UnlockSignal(phy802_15_4);
    }

    if (PHY_IsListeningToChannel(node, phyIndex, channelIndex))
    {
        PHY_StopListeningToChannel(node, phyIndex, channelIndex);
    }

    PROP_ReleaseSignal(node,
                       packet,
                       phyIndex,
                       channelIndex,
                       phy802_15_4->txPower_dBm,
                       duration,
                       delayUntilAirborne);

    Phy802_15_4ChangeState(node,phyIndex, PHY_TRANSMITTING);

    phy802_15_4->txOnPeriod += duration;
    // GuiStart
    if (node->guiOption == TRUE)
    {
        GUI_Broadcast(node->nodeId,
              GUI_PHY_LAYER,
              GUI_DEFAULT_DATA_TYPE,
              thisPhy->macInterfaceIndex,
              node->getNodeTime());
    }
    // GuiEnd

    endMsg = MESSAGE_Alloc(node,
                           PHY_LAYER,
                           0,
                           MSG_PHY_TransmissionEnd);

    // Assign the endMsg to 'phyTimerEndMessage' variable. This is
    // done in case we required to terminate the transmission, we
    // should not call 'Phy802_15_4TransmissionEnd' function.

    phy802_15_4->phyTimerEndMessage = endMsg ;

    MESSAGE_SetInstanceId(endMsg, (short) phyIndex);
    MESSAGE_Send(node, endMsg, delayUntilAirborne + duration + 1);

    /* Keep track of phy statistics */
    phy802_15_4->stats.totalTxSignals++;

    if (DEBUG)
    {
        printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4PHY : transmitting "
                "signal\n",
        node->getNodeTime(),
        node->nodeId);
        fflush(stdout);
    }

    if (node->guiOption)
    {
        GUI_SendUnsignedData(node->nodeId,
                     phy802_15_4->stats.totalTxSignalsId,
                     phy802_15_4->stats.totalTxSignals,
                     node->getNodeTime());
        GUI_SendRealData(node->nodeId,
                 phy802_15_4->stats.energyConsumedId,
                 phy802_15_4->stats.energyConsumed / 3600.0,
                 node->getNodeTime());
    }

    phy802_15_4->stats.turnOnPeriod = node->getNodeTime()
                                        - phy802_15_4->stats.turnOnTime;

}

// Used by the MAC layer to start transmitting
// a packet.Accepts a parameter called
// duration which specifies transmission delay.
// Function is being overloaded
//
// \param node  node pointer to node
// \param phyIndex  index
// \param packet  packet to be sent
// \param duration  packet transmission delay
// \param useMacLayerSpecifiedDelay  use delay specified by MAC
// \param initDelayUntilAirborne  delay until airborne
//


void Phy802_15_4StartTransmittingSignal(Node* node,
                                        Int32 phyIndex,
                                        Message* packet,
                                        clocktype duration,
                                        BOOL useMacLayerSpecifiedDelay,
                                        clocktype initDelayUntilAirborne)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #StartTransmittingSignal#\n",node->getNodeTime(),node->nodeId);
}

    clocktype delayUntilAirborne = initDelayUntilAirborne;
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*)thisPhy->phyVar;
    Int32 channelIndex = 0;
    Message* endMsg = NULL;
    clocktype packetDuration = 0;

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);

    packetDuration = duration + phy802_15_4->preambleHeaderPeriod;

    if (!useMacLayerSpecifiedDelay)
    {
        delayUntilAirborne = phy802_15_4->RxTxTurnAroundTime;
    }

    if (phy802_15_4->mode == PHY_RECEIVING)
    {
        phy802_15_4->interferencePower_mW += phy802_15_4->rxMsgPower_mW;

        Phy802_15_4UnlockSignal(phy802_15_4);
    }
    if (PHY_IsListeningToChannel(node, phyIndex, channelIndex))
    {
        PHY_StopListeningToChannel(node, phyIndex, channelIndex);
    }

    PROP_ReleaseSignal(node,
                       packet,
                       phyIndex,
                       channelIndex,
                       phy802_15_4->txPower_dBm,
                       packetDuration,
                       delayUntilAirborne);

    Phy802_15_4ChangeState(node,phyIndex, PHY_TRANSMITTING);

    phy802_15_4->txOnPeriod += packetDuration;

    // GuiStart
    if (node->guiOption == TRUE)
    {
        GUI_Broadcast(node->nodeId,
                      GUI_PHY_LAYER,
                      GUI_DEFAULT_DATA_TYPE,
                      thisPhy->macInterfaceIndex,
                      node->getNodeTime());
    }
    // GuiEnd

    endMsg = MESSAGE_Alloc(node,
                           PHY_LAYER,
                           0,
                           MSG_PHY_TransmissionEnd);

    // Assign the endMsg to 'phyTimerEndMessage' variable. This is
    // done in case we required to terminate the transmission, we
    // should not call 'Phy802_15_4TransmissionEnd' function.

    phy802_15_4->phyTimerEndMessage = endMsg ;


    MESSAGE_SetInstanceId(endMsg, (short) phyIndex);
    MESSAGE_Send(node, endMsg, delayUntilAirborne + duration + 1);

    phy802_15_4->stats.turnOnPeriod
        = node->getNodeTime() - phy802_15_4->stats.turnOnTime;

    // Keep track of phy statistics
    phy802_15_4->stats.totalTxSignals++;
}

// Start transmitting a frame
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param packet  Frame to be transmitted
// \param bitSize  Packet size in bits
//    + useMacLayerSpecifiedDelay : Use MAC specified delay or calculate it
// \param initDelayUntilAirborne  The MAC specified delay
//
//
// Used by the MAC layer to start transmitting a packet.
// Accepts message size in bits. Used for non-byte aligned messages
//

void Phy802_15_4StartTransmittingSignal(Node* node,
                                        Int32 phyIndex,
                                        Message* packet,
                                        Int32 bitSize,
                                        BOOL useMacLayerSpecifiedDelay,
                                        clocktype initDelayUntilAirborne)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #StartTransmittingSignal#\n",node->getNodeTime(),node->nodeId);
}

    clocktype delayUntilAirborne = initDelayUntilAirborne;
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*)thisPhy->phyVar;
    Int32 channelIndex = 0;
    Message* endMsg = NULL;
    clocktype duration = 0;

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);

    // compute duration from the bit size
    duration
        = Phy802_15_4GetFrameDurationFromBits(
                                          thisPhy,
                                          bitSize,
                                          (Int32)PHY_GetTxDataRate(node, phyIndex));

    if (!useMacLayerSpecifiedDelay)
    {
        delayUntilAirborne = phy802_15_4->RxTxTurnAroundTime;
    }

    if (phy802_15_4->mode == PHY_RECEIVING)
    {
        phy802_15_4->interferencePower_mW += phy802_15_4->rxMsgPower_mW;
        Phy802_15_4UnlockSignal(phy802_15_4);
    }

    if (PHY_IsListeningToChannel(node, phyIndex, channelIndex))
    {
        PHY_StopListeningToChannel(node, phyIndex, channelIndex);
    }

    PROP_ReleaseSignal(node,
                       packet,
                       phyIndex,
                       channelIndex,
                       phy802_15_4->txPower_dBm,
                       duration,
                       delayUntilAirborne);

    Phy802_15_4ChangeState(node,phyIndex, PHY_TRANSMITTING);
    phy802_15_4->txOnPeriod += duration;

    // GuiStart
    if (node->guiOption == TRUE)
    {
        GUI_Broadcast(node->nodeId,
                      GUI_PHY_LAYER,
                      GUI_DEFAULT_DATA_TYPE,
                      thisPhy->macInterfaceIndex,
                      node->getNodeTime());
    }
    // GuiEnd

    endMsg = MESSAGE_Alloc(node,
                           PHY_LAYER,
                           0,
                           MSG_PHY_TransmissionEnd);

    // Assign the endMsg to 'phyTimerEndMessage' variable. This is
    // done in case we required to terminate the transmission, we
    // should not call 'Phy802_15_4TransmissionEnd' function.

    phy802_15_4->phyTimerEndMessage = endMsg ;

    MESSAGE_SetInstanceId(endMsg, (short) phyIndex);
    MESSAGE_Send(node, endMsg, delayUntilAirborne + duration + 1);
    phy802_15_4->stats.turnOnPeriod = node->getNodeTime()
                                        - phy802_15_4->stats.turnOnTime;

    /* Keep track of phy statistics */
    phy802_15_4->stats.totalTxSignals++;
}


// Obtain the duration of frame
//
// \param thisPhy  Pointer to PhyData
// \param size  Packet size in Bytes
// \param dataRate  Data rate in bits/s
//
// \return Duration of the frame
clocktype Phy802_15_4GetFrameDuration(PhyData* thisPhy,
                                      Int32 size,
                                      Int32 dataRate)
{
if (KANDEB){
	printf("Phy.cpp in the #GetFrameDuration#\n");
}

    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;

    clocktype delay
        = size * 8 * SECOND / dataRate + phy802_15_4->preambleHeaderPeriod;

    return delay;
}

// **
// FUNCTION   :: Phy802_15_4GetFrameDurationFromBits
// LAYER      :: PHY
// PURPOSE    :: Obtain the duration of frame
// PARAMETERS ::
// + thisPhy   : PhyData*  : Pointer to PhyData
// + bitSize   : Int32       : Packet size in Bits
// + dataRate  : Int32       : Data rate in bits/s
// RETURN     :: clocktype : Duration of the frame
// Function to compute the duration of the frame, given the bit size of
// the frame used for non-byte aligned messages

clocktype Phy802_15_4GetFrameDurationFromBits(PhyData* thisPhy,
                                              Int32 bitSize,
                                              Int32 dataRate)
{
if (KANDEB){
	printf("Phy.cpp in the #GetFrameDurationFromBits#\n");
}

    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;

    clocktype delay
        = bitSize * SECOND / dataRate + phy802_15_4->preambleHeaderPeriod;

    return delay;
}
// Set the transmit power of the PHY
//
// \param thisPhy  Pointer to PHY data
// \param newTxPower_mW  Transmit power in mW
//

void Phy802_15_4SetTransmitPower(PhyData* thisPhy,
                                double newTxPower_mW)
{
if (KANDEB){
	printf("Phy.cpp in the #SetTransmitPower#\n");
}

    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;
    phy802_15_4->txPower_dBm = (float) IN_DB(newTxPower_mW);
}

// Retrieve the transmit power of the PHY in mW
//
// \param thisPhy  Pointer to PHY data
// \param txPower_mW  For returning the transmit power
//
void Phy802_15_4GetTransmitPower(PhyData* thisPhy,
                                 double* txPower_mW)
{
if (KANDEB){
	printf("Phy.cpp in the #GetTransmitPower#\n");
}

    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;
    *txPower_mW = (double) NON_DB(phy802_15_4->txPower_dBm);
}

// **
// FUNCTION   :: Phy802_15_4GetDataRate
// LAYER      :: PHY
// PURPOSE    :: Get the data rate of the PHY
// PARAMETERS ::
// + thisPhy   : PhyData* : Pointer to PHY structure
// RETURN     :: Int32      : Transmission data rate in bps
Int32 Phy802_15_4GetDataRate(PhyData* thisPhy)
{
if (KANDEB){
	printf("Phy.cpp in the #GetDataRate#\n");
}

    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;
    return phy802_15_4->dataRate;
}

// **
// FUNCTION   :: Phy802_15_4GetSymbolRate
// LAYER      :: PHY
// PURPOSE    :: Get the symbol rate of the PHY
// PARAMETERS ::
// + thisPhy   : PhyData* : Pointer to PHY structure
// RETURN     :: Int32      : Transmission data rate in bps
Int32 Phy802_15_4GetSymbolRate(PhyData* thisPhy)
{
if (KANDEB){
	printf("Phy.cpp in the #GetSymbolRate#\n");
}

    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;
    return (Int32)(SECOND / phy802_15_4->symbolPeriod);
}

// Returns the time required to transmit phy header at the
// current data rate (in symbols)
//
// \param thisPhy  Pointer to PHY structure
//
// \return Duration in symbols
Int32 Phy802_15_4GetPhyHeaderDurationInSymbols(PhyData* thisPhy)
{
if (KANDEB){
	printf("Phy.cpp in the #GetPhyHeaderDurationInSymbols#\n");
}

    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;
    return (Int32)(phy802_15_4->preambleHeaderPeriod / phy802_15_4->symbolPeriod);
}


// Obtain link quality.
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param sinr  For returning the sinr value
// \param signalPower_mW  For returning the received signal power
//

void Phy802_15_4GetLinkQuality(Node* node,
                               Int32 phyIndex,
                               double *sinr,
                               double *signalPower_mW)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #GetLinkQuality#\n",node->getNodeTime(),node->nodeId);
}

    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;

    *sinr = phy802_15_4->linkQuality;
    *signalPower_mW = phy802_15_4->rxMsgPower_mW;
}


// Activation Node
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//

void Phy802_15_4ActivationNode(Node* node, Int32 phyIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #ActivationNode#\n",node->getNodeTime(),node->nodeId);
}

    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;

    if (phy802_15_4->mode == PHY_TRX_OFF)
    {
        Phy802_15_4ChangeState(node,phyIndex, PHY_IDLE);
        phy802_15_4->activationTime = node->getNodeTime();
        phy802_15_4->turnOffPeriod
            += phy802_15_4->activationTime - phy802_15_4->deactivationTime;
    }
}


// Deactivation Node
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//
void Phy802_15_4DeactivationNode(Node* node, Int32 phyIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #DeactivationNode#\n",node->getNodeTime(),node->nodeId);
}

    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;

    if (phy802_15_4->mode != PHY_TRX_OFF)
    {
        phy802_15_4->deactivationTime = node->getNodeTime();
    }
    Phy802_15_4ChangeState(node,phyIndex, PHY_TRX_OFF);
}

// get node's status
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param status  node's status
//
void Phy802_15_4GetNodeStatus(Node* node,
                              Int32 phyIndex,
                              PhyStatusType status)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #GetNodeStatus#\n",node->getNodeTime(),node->nodeId);
}

    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;
    status = phy802_15_4->mode;
}
// PLME CCA request
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param channelIsFree  channel is free or not
//
void Phy802_15_4PLMECCArequest(Node* node,
                               Int32 phyIndex,
                               BOOL* channelIsFree )
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #PLMECCArequest#\n",node->getNodeTime(),node->nodeId);
}

    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;

    if (phy802_15_4->mode == PHY_TRX_OFF)
    {
        Phy802_15_4ReportStatusToMac(
                   node,
                   phyIndex,
                   PHY_TRX_OFF);
        *channelIsFree = FALSE;
        return;

    }
    else if (phy802_15_4->mode != PHY_IDLE
                && phy802_15_4->mode != PHY_SUCCESS)
    {
        *channelIsFree = FALSE;
        return;
    }
    else
    {
        *channelIsFree = Phy802_15_4MediumIsIdle(node, phyIndex);
        return;
    }
}


// PLME Set TRX_State request
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param state  state to be set
//
void Phy802_15_4PlmeSetTRX_StateRequest(Node* node,
                                        Int32 phyIndex,
                                        PLMEsetTrxState state)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #PlmeSetTRX_StateRequest#\n",node->getNodeTime(),node->nodeId);
}

    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*)thisPhy->phyVar;

    if (DEBUG)
    {
        if ((state == TRX_OFF)|| (state == FORCE_TRX_OFF))
            printf("Node %d switches to SLEEP state\n", node->nodeId);
        else if ((state == RX_ON) || (state == TX_ON))
            printf("Node %d switches to ACTIVE state\n", node->nodeId);
    }


    if ((state == RX_ON) || (state == TX_ON))
    {
        if (phy802_15_4->mode != PHY_RECEIVING
            && phy802_15_4->mode != PHY_TRANSMITTING)
        {
            Phy802_15_4ChangeState(node,phyIndex, PHY_SUCCESS);
        }
    }
    else if (state == TRX_OFF)
    {
        if (phy802_15_4->mode == PHY_TRANSMITTING)
        {
            // do nothing
            // Phy802_15_4ChangeState(node,phyIndex, PHY_BUSY_TX);
        }
        else if ((phy802_15_4->mode == PHY_IDLE)
                || (phy802_15_4->mode == PHY_SUCCESS)
                || (phy802_15_4->mode == PHY_SENSING))
        {
            Phy802_15_4ChangeState(node,phyIndex, PHY_TRX_OFF);
        }
        else if (phy802_15_4->mode == PHY_RECEIVING)
        {
            // Do nothing
            // Let PHY complete the current transaction
        }
    }
    else if (state == FORCE_TRX_OFF)
    {
	//送受信を強制的に停止させる
        if ((phy802_15_4->mode == PHY_TRANSMITTING)
            || (phy802_15_4->mode == PHY_BUSY_TX))
        {
            Phy802_15_4TerminateCurrentTransmission(
                node,
                phyIndex);//TerminateCurrentTransmission() : 現在の送信を停止させる
            Phy802_15_4ChangeState(node,phyIndex,PHY_TRX_OFF);//StateをOFFに変更
        }
        else if ((phy802_15_4->mode == PHY_IDLE)
            || (phy802_15_4->mode == PHY_SUCCESS)
            || (phy802_15_4->mode == PHY_SENSING)
            || (phy802_15_4->mode == PHY_TRX_OFF))
        {
            Phy802_15_4ChangeState(node,phyIndex,PHY_TRX_OFF);
        }
        else if ((phy802_15_4->mode == PHY_RECEIVING)
            || (phy802_15_4->mode == PHY_BUSY_RX))
        {
            BOOL frameError = FALSE;
            clocktype endSignalTime = 0;
            BOOL iFterminate = FALSE;

            PHY_TerminateCurrentReceive(
                node, phyIndex, iFterminate,
                &frameError, &endSignalTime);
            Phy802_15_4ChangeState(node,phyIndex, PHY_TRX_OFF);
        }
    }
}

// **
// FUNCTION   :: Phy802_15_4TerminateCurrentTransmission
// LAYER      :: PHY
// PURPOSE    :: Terminate current transmission
// PARAMETERS ::
// + node              : Node* : Pointer to node.
// + phyIndex          : Int32   : Index of the PHY
// RETURN     :: void : NULL
void Phy802_15_4TerminateCurrentTransmission(Node* node, Int32 phyIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #TerminateCurrentTransmission#\n",node->getNodeTime(),node->nodeId);
}

    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4 *)thisPhy->phyVar;
    Int32 channelIndex = 0;

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);

    // GuiStart
    if (node->guiOption == TRUE)
    {
        GUI_EndBroadcast(node->nodeId,
                         GUI_PHY_LAYER,
                         GUI_DEFAULT_DATA_TYPE,
                         thisPhy->macInterfaceIndex,
                         node->getNodeTime());
    }
    // GuiEnd
    ERROR_Assert(phy802_15_4->mode == PHY_TRANSMITTING,"phy802_15_4->mode"
                    " should not be PHY_TRANSMITTING");

    // Cancel the timer end message so that 'Phy802_15_4TransmissionEnd' is
    // not called.

    if (phy802_15_4->phyTimerEndMessage)
    {
        MESSAGE_CancelSelfMsg(node, phy802_15_4->phyTimerEndMessage);
        phy802_15_4->phyTimerEndMessage = NULL;
    }

    PHY_StartListeningToChannel(node, phyIndex, channelIndex);

    PHY_SignalInterference(node,
                           phyIndex,
                           channelIndex,
                           NULL,
                           NULL,
                           &(phy802_15_4->interferencePower_mW));

    if (Phy802_15_4CarrierSensing(node, phy802_15_4))
    {
        Phy802_15_4ChangeState(node,phyIndex, PHY_SENSING);
    }
    else
    {
        Phy802_15_4ChangeState(node,phyIndex, PHY_IDLE);
    } // if
    Phy802_15_4ReportStatusToMac(node, phyIndex, phy802_15_4->mode);
}
// Get the channel number
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
//
// \return channel number

Int32 Phy802_15_4getChannelNumber(Node* node, Int32 phyIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #getChannelNumber#\n",node->getNodeTime(),node->nodeId);
}

    Int32 channelIndex = 0;
    Int32 channelnumber = 0;
    double frequency = 0.0;

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);
    PropProfile* propProfile = node->propChannel[channelIndex].profile;

    frequency = propProfile->frequency;

    if ((frequency >= PHY802_15_4_800MHZ_RANGE_START)
        && (frequency <= PHY802_15_4_800MHZ_RANGE_END))
    {
        channelnumber = 0;
    }
    else if ((frequency >= PHY802_15_4_900MHZ_RANGE_START)
        && (frequency <= PHY802_15_4_900MHZ_RANGE_END))
    {
        channelnumber = Int32((frequency - 906000000.0)/ 2000000.0) + 1;
    }
    else if ((frequency >= PHY802_15_4_2400MHZ_RANGE_START)
        && (frequency <= PHY802_15_4_2400MHZ_RANGE_END))
    {
        channelnumber = Int32((frequency - 2405000000.0)/ 5000000.0) + 11;
    }

    return channelnumber;
}

// PLME Get Request
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param PIBAttribute  PPIB attribute
// \param ppib  Pointer to PHY_PIB
//
void Phy802_15_4PlmeGetRequest(Node* node,
                               Int32 phyIndex,
                               PPIBAenum PIBAttribute,
                               PHY_PIB* ppib)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #PlmeGetRequest#\n",node->getNodeTime(),node->nodeId);
}

    // PHYenum t_status;
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4 *)thisPhy->phyVar;

    switch(PIBAttribute)
    {
        case phyCurrentChannel:
            ppib->phyCurrentChannel = Phy802_15_4getChannelNumber(
                                        node,
                                        phyIndex);
            break;
        case phyTransmitPower:
            {
                double txPower_mW = 0.0;
                Phy802_15_4GetTransmitPower(thisPhy,&txPower_mW);
                ppib->phyTransmitPower = txPower_mW;
                break;
            }
        case phyCCAMode:
            ppib->phyCCAMode = phy802_15_4->CCAmode;
            break;
        default:
            break;
        // case phyChannelsSupported:
    }
}

// Handle signal arrival from the channel
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param channelIndex  Index of the channel receiving signal from
// \param propRxInfo  Propagation information
// \param inbandPower_mW  The inband power of the interference signal
//
void Phy802_15_4InterferenceArrivalFromChannel(
    Node* node,
    int phyIndex,
    int channelIndex,
    PropRxInfo *propRxInfo,
    double inbandPower_mW)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #InterferenceArrivalFromChannel#\n",node->getNodeTime(),node->nodeId);
}

    PhyData *thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;

    if ((phy802_15_4->mode != PHY_TRX_OFF))
    {
            double interferencePower_mW =
            NON_DB(ANTENNA_GainForThisSignal(node, phyIndex, propRxInfo) +
               IN_DB(inbandPower_mW));

        switch (phy802_15_4->mode) {
            case PHY_RECEIVING:
            case PHY_BUSY_RX:
            {
                if (!phy802_15_4->rxMsgError) {

                    BOOL isheader = FALSE;

                    phy802_15_4->rxMsgError =
                        Phy802_15_4CheckRxPacketError(
                            node,
                            phy802_15_4,
                            NULL,
                            phyIndex,
                            isheader,
                            NULL);
                 }//if//

                phy802_15_4->rxTimeEvaluated = node->getNodeTime();
                phy802_15_4->interferencePower_mW += interferencePower_mW;
                break;
             }

            //
            // If the phy is idle or sensing,
            // check check if the signal change its status.
            //
            case PHY_SUCCESS:
            case PHY_IDLE:
            case PHY_SENSING:
            {

                PhyStatusType newMode;

                phy802_15_4->interferencePower_mW += interferencePower_mW;

                if (Phy802_15_4CarrierSensing(node, phy802_15_4)) {

                    newMode = PHY_SENSING;
                }
                else {
                    newMode = PHY_IDLE;
                }//if//

                if (newMode != phy802_15_4->mode) {
                    Phy802_15_4ChangeState(node,phyIndex, newMode);
                }//if//

               break;

            }
            default:
                abort();

        }//switch (phy802_15_4->mode)//
    }
}


// Handle signal end from a channel
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param channelIndex  Index of the channel receiving signal from
// \param propRxInfo  Propagation information
// \param inbandPower_mW  Inband power in mW
//

void Phy802_15_4InterferenceEndFromChannel(
        Node* node,
        int phyIndex,
        int channelIndex,
        PropRxInfo *propRxInfo,
        double inbandPower_mW)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: Phy.cpp in the #InterferenceEndFromChannel#\n",node->getNodeTime(),node->nodeId);
}

    PhyData *thisPhy = node->phyData[phyIndex];
    PhyData802_15_4* phy802_15_4 = (PhyData802_15_4*) thisPhy->phyVar;
    double sinr;

    double interferencePower_mW =
            NON_DB(ANTENNA_GainForThisSignal(node, phyIndex, propRxInfo)
            + IN_DB(inbandPower_mW));

    if (phy802_15_4->mode == PHY_RECEIVING) {

        if (phy802_15_4->rxMsgError == FALSE) {

            BOOL isheader = FALSE;

            phy802_15_4->rxMsgError =
                    Phy802_15_4CheckRxPacketError(
                        node,
                        phy802_15_4,
                        &sinr,
                        phyIndex,
                        isheader,
                        NULL);

            phy802_15_4->rxTimeEvaluated = node->getNodeTime();

        }//if
    }//if//

    PhyStatusType newMode;
    phy802_15_4->interferencePower_mW -= interferencePower_mW ;

    if (phy802_15_4->interferencePower_mW < 0.0) {

        phy802_15_4->interferencePower_mW = 0.0;
    }

    if ((phy802_15_4->mode != PHY_RECEIVING) &&
        (phy802_15_4->mode != PHY_TRANSMITTING)){

        if (phy802_15_4->mode != PHY_TRX_OFF) {

            if (Phy802_15_4CarrierSensing(node, phy802_15_4) == TRUE) {

                newMode = PHY_SENSING;
            }
            else {

                newMode = PHY_IDLE;
            } //else//

            if (newMode != phy802_15_4->mode) {

                Phy802_15_4ChangeState(node,phyIndex, newMode);
            }//if//
        }
    }//if//
}

