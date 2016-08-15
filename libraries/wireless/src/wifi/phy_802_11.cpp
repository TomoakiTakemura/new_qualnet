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
#include "partition.h"
#include "antenna.h"
#include "antenna_switched.h"
#include "antenna_steerable.h"
#include "antenna_patterned.h"
#include "phy_802_11.h"
#include "phy_802_11n.h"

#include "mac_csma.h"
#include "mac_dot11.h"
#include "mac_dot11-sta.h"
#include "mac_phy_802_11n.h"
#include "spectrum.h"
#include "phy_dot11ac.h"
#include "phy_dot11a.h"

#include "phy_802_11_manager.h"

#include "phy_802_11p.h"

#ifdef ADDON_DB
#include "dbapi.h"
#endif

#undef DEBUG
#define DEBUG 0

void Phy802_11ChangeState(
    Node* node,
    int phyIndex,
    PhyStatusType newStatus)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11 *phy802_11 = (PhyData802_11 *)thisPhy->phyVar;
    phy802_11->previousMode = phy802_11->mode;
    phy802_11->mode = newStatus;


    Phy_ReportStatusToEnergyModel(
        node,
        phyIndex,
        phy802_11->previousMode,
        newStatus);
}

double
Phy802_11GetSignalStrength(Node *node,PhyData802_11* phy802_11)
{
    double rxThreshold_mW;
    int dataRateToUse;
    Phy802_11GetLowestTxDataRateType(phy802_11->thisPhy, &dataRateToUse);
    rxThreshold_mW = phy802_11->rxSensitivity_mW[dataRateToUse];
    return IN_DB(rxThreshold_mW);
}


double
Phy802_11nGetSignalStrength(Node* node,PhyData802_11* phy802_11)
{
    double rxThreshold_dBm = phy802_11->pDot11Base->getMinSensitivity_dBm(
                                   CHBWDTH_20MHZ);
    return rxThreshold_dBm;
}

double
Phy802_11acGetSignalStrength(Node* node,PhyData802_11* phy802_11)
{
    double rxThreshold_dBm = phy802_11->pDot11Base->getMinSensitivity_dBm(
                                   CHBWDTH_20MHZ);
    return rxThreshold_dBm;
}

//static int Phy802_11_OverheadSize(int model, Phy802_11n* pDot11n)
//{
//    switch(model)
//    {
//    case PHY802_11a:
//        return PHY802_11a_CONTROL_OVERHEAD_SIZE;
//
//    case PHY802_11b:
//        return PHY802_11b_CONTROL_OVERHEAD_SIZE;
//
//    case PHY802_11n:
//    {
//        MAC_PHY_TxRxVector rxVector;
//        pDot11n->GetRxVectorOfLockedSignal(rxVector);
//        int numSts = pDot11n->GetNSts(rxVector.stbc,
//                Phy802_11n::MCS_Params[rxVector.chBwdth][rxVector.mcs].nSpatialStream);
//        clocktype dataRate =
//                Phy802_11n::MCS_Params[CHBWDTH_20MHZ][0].dataRate[0];
//        clocktype preamble = Phy802_11n::PreambDur_HtGf(numSts, rxVector.numEss);
//        int overheadSize = (int)((preamble * dataRate) / SECOND
//                        + Phy802_11n::Ppdu_Service_Bits_Size
//                        + Phy802_11n::Ppdu_Tail_Bits_Size);
//        return overheadSize/8;
//    }
//
//    default:
//        return -1;
//    }
//}

 void Phy802_11ReportExtendedStatusToMac(
    Node *node,
    int phyNum,
    PhyStatusType status,
    clocktype receiveDuration,
    Message* potentialIncomingPacket)
{
    PhyData* thisPhy = node->phyData[phyNum];
    PhyData802_11* phy802_11 = (PhyData802_11 *)thisPhy->phyVar;

    assert(status == phy802_11->mode);

    if (potentialIncomingPacket != NULL) {
        MESSAGE_RemoveHeader(
            node,
            potentialIncomingPacket,
            sizeof(Phy802_11PlcpHeader),
            TRACE_802_11);
    }

    MAC_ReceivePhyStatusChangeNotification(
        node, thisPhy->macInterfaceIndex,
        phy802_11->previousMode, status,
        receiveDuration, potentialIncomingPacket);

    if (potentialIncomingPacket != NULL) {
        MESSAGE_AddHeader(
            node,
            potentialIncomingPacket,
            sizeof(Phy802_11PlcpHeader),
            TRACE_802_11);
    }
}

void Phy802_11ReportStatusToMac(
    Node *node, int phyNum, PhyStatusType status)
{
    Phy802_11ReportExtendedStatusToMac(
        node, phyNum, status, 0, NULL);
}

static
void Phy802_11AddPlcpHeader(Node* node,
                            PhyData802_11* phy802_11,
                            Message* packet)
{
    MESSAGE_AddHeader(node, packet, sizeof(Phy802_11PlcpHeader),
                      TRACE_802_11);

    Phy802_11PlcpHeader *plcp = (Phy802_11PlcpHeader*)
                                    MESSAGE_ReturnPacket(packet);
    plcp->txPhyModel = phy802_11->thisPhy->phyModel;
    if (phy802_11->thisPhy->phyModel == PHY802_11n
        || phy802_11->thisPhy->phyModel == PHY802_11ac) 
    {
        phy802_11->pDot11Base->fillPlcpHdr(plcp);
    }
    else
    {
/************** Interoperability feature *********START*******************/
        plcp->rate = phy802_11->pDot11Base->txDataRateType;
        plcp->format  = MODE_NON_HT;
        plcp->chBwdth    = CHBWDTH_20MHZ;
        plcp->length     = MESSAGE_ReturnPacketSize(packet)
                                - sizeof(Phy802_11PlcpHeader);
        plcp->sounding  = FALSE;
        plcp->containAMPDU = FALSE;
        plcp->gi     = GI_LONG;
        plcp->mcs       = phy802_11->pDot11Base->txDataRateType;
        plcp->dBwdthNonHT = STATIC;
        plcp->nonHTMod    = OFDM;
/************** Interoperability feature *********END*******************/
    }
}

void Phy802_11LockSignal(
    Node* node,
    PhyData802_11* phy802_11,
    PropRxInfo* propRxInfo,
    Message* msg,
    double rxPower_mW,
    clocktype rxEndTime,
    Int32 channelIndex,
    const Orientation& txDOA,
    const Orientation& rxDOA)
{

    Phy802_11PlcpHeader* plcp = (Phy802_11PlcpHeader*)
                                    MESSAGE_ReturnPacket(msg);
    if (DEBUG)
    {
        char currTime[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(node->getNodeTime(), currTime);
        printf("\ntime %s: LockSignal from %d at %d\n",
               currTime,
               msg->originatingNodeId,
               node->nodeId);
    }

    phy802_11->rxMsg = msg;
    phy802_11->rxMsgPower_mW = rxPower_mW;
    phy802_11->rxTimeEvaluated = node->getNodeTime();
    phy802_11->rxEndTime = rxEndTime;
    phy802_11->txNodeId = propRxInfo->txNodeId;
    phy802_11->rxChannelIndex = propRxInfo->channelIndex;
    phy802_11->pathloss_dB = propRxInfo->pathloss_dB;

    if (phy802_11->thisPhy->phyModel == PHY802_11n
        || phy802_11->thisPhy->phyModel == PHY802_11ac)
    {
        phy802_11->pDot11Base->lockSignal(node,
                               channelIndex,
                               plcp,
                               txDOA,
                               rxDOA,
                               propRxInfo->txMsg->m_mimoData->m_elementCount,
                               propRxInfo->txMsg->m_mimoData->m_elementSpace);
    }
    else
    {
        phy802_11->pDot11Base->rxDataRateType = plcp->rate;
        phy802_11->rxPhyType = plcp->txPhyModel;
    }

    if (phy802_11->thisPhy->phyStats)
    {
        phy802_11->thisPhy->stats->AddSignalLockedDataPoints(
            node,
            propRxInfo,
            phy802_11->thisPhy,
            channelIndex,
            rxPower_mW);
    }
}

//inline//
void Phy802_11UnlockSignal(PhyData802_11* phy802_11) {
    phy802_11->rxMsg = NULL;
    phy802_11->rxMsgPower_mW = 0.0;
    phy802_11->rxTimeEvaluated = 0;
    phy802_11->rxEndTime = 0;
    phy802_11->txNodeId = 0;
    phy802_11->rxChannelIndex = -1;
    phy802_11->pathloss_dB = 0.0;

    if (phy802_11->pDot11Base)
    {
        phy802_11->pDot11Base->unlockSignal();
    }
}



static void StatsControllerInit(PhyData802_11* phy802_11,
                                Node* node,
                                int ifidx,
                                const NodeInput* nodeInput)
{
    NodeEventController* nec = new NodeEventController(node, ifidx);
    DataModel* dm = new MappedDataModel<>(*nec, nodeInput);
    phy802_11->sController = new StatsController(*dm);
    phy802_11->sController->initialize();
    phy802_11->sController->createTableSchema();
}

void Phy802_11Init(
    Node *node,
    const int phyIndex,
    const NodeInput *nodeInput,
    BOOL  initN)
{
    BOOL   wasFound;
    int i;
    int numChannels = PROP_NumberChannels(node);

    PhyData802_11 *phy802_11 =
        (PhyData802_11 *)MEM_malloc(sizeof(PhyData802_11));

    memset(phy802_11, 0, sizeof(PhyData802_11));

    node->phyData[phyIndex]->phyVar = (void*)phy802_11;

    phy802_11->thisPhy = node->phyData[phyIndex];

    std::string path;
    D_Hierarchy *h = &node->partitionData->dynamicHierarchy;

    if (h->CreatePhyPath(
            node,
            phyIndex,
            "802.11",
            "energyConsumed",
            path))
    {
        h->AddObject(
            path,
            new D_Float64Obj(&phy802_11->stats.energyConsumed));
    }

    if (h->CreatePhyPath(
            node,
            phyIndex,
            "802.11",
            "turnOnTime",
            path))
    {
        h->AddObject(
            path,
            new D_ClocktypeObj(&phy802_11->stats.turnOnTime));
    }

    if (h->CreatePhyPath(
            node,
            phyIndex,
            "802.11",
            "txPower_dBm",
            path))
    {
        h->AddObject(
            path,
            new D_Float32Obj(&phy802_11->txPower_dBm));
    }

    if (h->CreatePhyPath(
            node,
            phyIndex,
            "802.11",
            "channelBandwidth",
            path))
    {
        h->AddObject(
            path,
            new D_Int32Obj(&phy802_11->channelBandwidth));
    }

    if (phy802_11->thisPhy->phyStats)
    {
        phy802_11->thisPhy->stats = new STAT_PhyStatistics(node);
    }

    //
    // Antenna model initialization
    //
    ANTENNA_Init(node, phyIndex, nodeInput);

    ERROR_Assert(phy802_11->thisPhy->phyRxModel != SNR_THRESHOLD_BASED,
                 "802.11 PHY model does not work with "
                 "SNR_THRESHOLD_BASED reception model.\n");

    //
    // Set PHY802_11-ESTIMATED-DIRECTIONAL-ANTENNA-GAIN
    //
    IO_ReadDouble(
        node,
        node->nodeId,
        node->phyData[phyIndex]->macInterfaceIndex,
        nodeInput,
        "PHY802.11-ESTIMATED-DIRECTIONAL-ANTENNA-GAIN",
        &wasFound,
        &(phy802_11->directionalAntennaGain_dB));

    if (!wasFound &&
        (phy802_11->thisPhy->antennaData->antennaModelType
            != ANTENNA_OMNIDIRECTIONAL &&
          phy802_11->thisPhy->antennaData->antennaModelType
            != ANTENNA_PATTERNED))
    {
        ERROR_ReportError(
            "PHY802.11-ESTIMATED-DIRECTIONAL-ANTENNA-GAIN is missing\n");
    }


    //
    // Initialize phy statistics variables
    //
    phy802_11->stats.energyConsumed = 0.0;
#ifdef CYBER_LIB
    phy802_11->stats.totalRxSignalsToMacDuringJam = 0;
    phy802_11->stats.totalSignalsLockedDuringJam = 0;
    phy802_11->stats.totalSignalsWithErrorsDuringJam = 0;
#endif
    phy802_11->stats.turnOnTime = node->getNodeTime();
#ifdef ADDON_DB
    StatsDb* db = node->partitionData->statsDb;

    // For phy summary table
    if (db != NULL
        && (db->statsSummaryTable->createPhySummaryTable
            || db->statsAggregateTable->createPhyAggregateTable))
    {
        node->phyData[phyIndex]->oneHopData =
            new std::vector<PhyOneHopNeighborData>();
    }
#endif

    //
    // Initialize status of phy
    //
    phy802_11->rxMsg = NULL;
    phy802_11->rxMsgPower_mW = 0.0;
    phy802_11->interferencePower_mW = 0.0;
    phy802_11->txNodeId = 0;
    phy802_11->rxChannelIndex = -1;
    phy802_11->pathloss_dB = 0.0;
    phy802_11->noisePower_mW =
        phy802_11->thisPhy->noise_mW_hz * phy802_11->channelBandwidth;
    phy802_11->rxTimeEvaluated = 0;
    phy802_11->rxEndTime = 0;
    phy802_11->rxDOA.azimuth = 0;
    phy802_11->rxDOA.elevation = 0;
    phy802_11->previousMode = PHY_IDLE;
    phy802_11->mode = PHY_IDLE;
    Phy802_11ChangeState(node,phyIndex, PHY_IDLE);

    //
    // Setting up the channel to use for both TX and RX
    //
    for (i = 0; i < numChannels; i++) {
        if (PHY_IsListeningToChannel(node, phyIndex, i)) {
            break;
        }
    }
    ERROR_Assert(
        i != numChannels,
        "802.11 radio must listen to at least one channel");
    PHY_SetTransmissionChannel(node, phyIndex, i);

    //Initialize 802.11n data
    if (initN) {
        Phy802_11n *pDot11n = new Phy802_11n(phy802_11,
                              node,
                              phyIndex,
                              nodeInput);
        phy802_11->pDot11Base = (Phy802_11Manager*)pDot11n;
    }

    if (phy802_11->thisPhy->phyModel == PHY802_11ac)
    {
        Phy802_11ac* phy802_11ac = new Phy802_11ac(phy802_11,
                                   node,
                                   phyIndex,
                                   nodeInput);
        phy802_11->pDot11Base = (Phy802_11Manager*)phy802_11ac;
    }

    if (phy802_11->thisPhy->phyModel == PHY802_11a || 
        phy802_11->thisPhy->phyModel == PHY802_11b)
    {
        Phy802_11a* phy802_11a = new Phy802_11a(phy802_11,
                                 node,
                                 phyIndex,
                                 nodeInput);
        phy802_11->pDot11Base = (Phy802_11Manager*)phy802_11a;
    }
    if (node->phyData[phyIndex]->phyModel == PHY802_11pCCH ||
         node->phyData[phyIndex]->phyModel == PHY802_11pSCH)
    {
        Phy802_11a* phy802_11p = new Phy802_11a(phy802_11,
                                   node,
                                   node->phyData[phyIndex]->macInterfaceIndex,
                                   nodeInput);
        phy802_11->pDot11Base = (Phy802_11Manager*)phy802_11p;
    }

    phy802_11->pDot11Base->readBwdthParams(nodeInput);

    StatsControllerInit(phy802_11, node, phyIndex, nodeInput);
    return;
}


void Phy802_11ChannelListeningSwitchNotification(
   Node* node,
   int phyIndex,
   int channelIndex,
   BOOL startListening)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11 *)thisPhy->phyVar;

   if (phy802_11 == NULL)
    {
        // not initialized yet, return;
        return;
    }

    if (startListening == TRUE)
    {
        if (phy802_11->mode == PHY_TRANSMITTING)
        {
            phy802_11->pDot11Base->terminateCurrentTransmission(node,phyIndex);
        }

        if (phy802_11->pDot11Base->carrierSensing(false,
                                                  phyIndex) == TRUE)
        {
            Phy802_11ChangeState(node,phyIndex, PHY_SENSING);
        }
        else
        {
            Phy802_11ChangeState(node,phyIndex, PHY_IDLE);
        }

        if (phy802_11->previousMode != phy802_11->mode)
            Phy802_11ReportStatusToMac(node, phyIndex, phy802_11->mode);
    }
    else if (phy802_11->mode != PHY_TRANSMITTING)
    {
        if (phy802_11->mode == PHY_RECEIVING)
        {
            BOOL frameError;
            clocktype endTime;
            Phy802_11TerminateCurrentReceive(node,
                                             phyIndex,
                                             FALSE,
                                             &frameError,
                                             &endTime);
        }
        Phy802_11ChangeState(node,phyIndex, PHY_TRX_OFF);
    }
}


void Phy802_11TransmissionEnd(Node *node, int phyIndex) {
    if (DEBUG)
    {
        char clockStr[20];
        TIME_PrintClockInSecond(node->getNodeTime(), clockStr);
        printf("PHY_802_11: node %d transmission end at time %s\n",
               node->nodeId, clockStr);
    }
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11 *)thisPhy->phyVar;
    int channelIndex;

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);

    assert(phy802_11->mode == PHY_TRANSMITTING);

    phy802_11->txEndTimer = NULL;
    //GuiStart
    if (node->guiOption == TRUE) {
        GUI_EndBroadcast(node->nodeId,
                         GUI_PHY_LAYER,
                         GUI_DEFAULT_DATA_TYPE,
                         thisPhy->macInterfaceIndex,
                         node->getNodeTime());
    }
    //GuiEnd

    if (!ANTENNA_IsLocked(node, phyIndex)) {
        ANTENNA_SetToDefaultMode(node, phyIndex);
    }//if//

    //node->setRadioListen(phyIndex, true);
    PHY_StartListeningToChannel(node, phyIndex, channelIndex);

}


BOOL Phy802_11MediumIsIdle(Node* node, int phyIndex) {
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11*)thisPhy->phyVar;
    BOOL IsIdle;

    assert((phy802_11->mode == PHY_IDLE) ||
           (phy802_11->mode == PHY_SENSING));

    IsIdle = (!phy802_11->pDot11Base->carrierSensing(FALSE,
                                                     thisPhy->phyIndex));

    return IsIdle;
}


BOOL Phy802_11MediumIsIdleInDirection(Node* node, int phyIndex,
                                      double azimuth) {
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11*)thisPhy->phyVar;
    BOOL IsIdle;
    int channelIndex;
    double oldInterferencePower = phy802_11->interferencePower_mW;

    assert((phy802_11->mode == PHY_IDLE) ||
           (phy802_11->mode == PHY_SENSING));

    if (ANTENNA_IsLocked(node, phyIndex)) {
       return (phy802_11->mode == PHY_IDLE);
    }//if//

    // ZEB - Removed to support patterned antenna types
//    assert(ANTENNA_IsInOmnidirectionalMode(node, phyIndex));

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);


    ANTENNA_SetBestConfigurationForAzimuth(node, phyIndex, azimuth);

    PHY_SignalInterference(
        node,
        phyIndex,
        channelIndex,
        NULL,
        NULL,
        &(phy802_11->interferencePower_mW));

    IsIdle = (!phy802_11->pDot11Base->carrierSensing(FALSE,
                                                     thisPhy->phyIndex));

    phy802_11->interferencePower_mW = oldInterferencePower;
    ANTENNA_SetToDefaultMode(node, phyIndex);

    return IsIdle;
}


void Phy802_11SetSensingDirection(Node* node, int phyIndex,
                                  double azimuth) {
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11*)thisPhy->phyVar;
    int channelIndex;

    assert((phy802_11->mode == PHY_IDLE) ||
           (phy802_11->mode == PHY_SENSING));

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);
    ANTENNA_SetBestConfigurationForAzimuth(node, phyIndex, azimuth);

    PHY_SignalInterference(
        node,
        phyIndex,
        channelIndex,
        NULL,
        NULL,
        &(phy802_11->interferencePower_mW));
}



void Phy802_11Finalize(Node *node, const int phyIndex) {
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;
    Phy802_11ChangeState(node,phyIndex, PHY_IDLE);

    if (phy802_11->pDot11Base)
    {
        delete phy802_11->pDot11Base;
    }

    delete(phy802_11->sController);

    if (thisPhy->phyStats == FALSE) {
        return;
    }

    phy802_11->thisPhy->stats->Print(node,
                                     "Physical",
                                     "802.11",
                                     ANY_ADDRESS,
                                     phyIndex);

#ifdef CYBER_LIB
    if (node->phyData[phyIndex]->jammerStatistics == TRUE)
    {
        char buf[MAX_STRING_LENGTH];
        char durationStr[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(node->phyData[phyIndex]->jamDuration, durationStr, node);
        sprintf(buf, "Signals received and forwarded to MAC during the jam duration = %d",
            (int) phy802_11->stats.totalRxSignalsToMacDuringJam);
        IO_PrintStat(node, "Physical", "802.11", ANY_DEST, phyIndex, buf);
        sprintf(buf, "Signals locked on by PHY during the jam duration = %d",
            (int) phy802_11->stats.totalSignalsLockedDuringJam);
        IO_PrintStat(node, "Physical", "802.11", ANY_DEST, phyIndex, buf);

        sprintf(buf, "Signals received but with errors during the jam duration = %d",
            (int) phy802_11->stats.totalSignalsWithErrorsDuringJam);
        IO_PrintStat(node, "Physical", "802.11", ANY_DEST, phyIndex, buf);
        sprintf(buf, "Total jam duration in (s) = %s", durationStr);
        IO_PrintStat(node, "Physical", "802.11", ANY_DEST, phyIndex, buf);
    }
#endif

}



void Phy802_11SignalArrivalFromChannel(
    Node* node,
    int phyIndex,
    int channelIndex,
    PropRxInfo *propRxInfo)
{
    PhyData *thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;
    assert(phy802_11->mode != PHY_TRANSMITTING);

    spectralBand* msb = MESSAGE_GetSpectralBand(propRxInfo->txMsg);
    if (!msb)
    {
        ERROR_ReportError("Communication between Wi-Fi and non-Wi-Fi nodes "
            "is not supported. Please check configuration.");
    }

    if (phy802_11->thisPhy->phyStats)
    {
        phy802_11->thisPhy->stats->AddSignalDetectedDataPoints(node);
    }
    if (DEBUG)
    {
        char currTime[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(node->getNodeTime(), currTime);
        printf("\ntime %s: SignalArrival from %d at %d\n",
               currTime,
               propRxInfo->txMsg->originatingNodeId,
               node->nodeId);
    }

    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    pDot11BasePtr->signalArrival(node,
                                 phyIndex,
                                 channelIndex,
                                 propRxInfo);
}



void Phy802_11TerminateCurrentReceive(Node* node,
                   int phyIndex,
                   const BOOL terminateOnlyOnReceiveError,
                   BOOL* frameError,
                   clocktype* endSignalTime)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11*)thisPhy->phyVar;

    phy802_11->pDot11Base->terminateCurrentReceive(phyIndex);
}


void Phy802_11SignalEndFromChannel(
    Node* node,
    int phyIndex,
    int channelIndex,
    PropRxInfo *propRxInfo)
{
    PhyData *thisPhy = node->phyData[phyIndex];

    Phy802_11Manager* pDot11Mgr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;

    pDot11Mgr->signalEnd(node,
                         phyIndex,
                         channelIndex,
                         propRxInfo);
}



void Phy802_11SetTransmitPower(PhyData *thisPhy, double newTxPower_mW) {
    PhyData802_11 *phy802_11 = (PhyData802_11*) thisPhy->phyVar;

    phy802_11->txPower_dBm = (float)IN_DB(newTxPower_mW);
}

StatsController* Phy802_11GetStatsController(PhyData* thisPhy)
{
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;
    return phy802_11->sController;

}

void Phy802_11GetTransmitPower(PhyData *thisPhy, double *txPower_mW) {
    PhyData802_11 *phy802_11 = (PhyData802_11*) thisPhy->phyVar;

    *txPower_mW = NON_DB(phy802_11->txPower_dBm);
}



int Phy802_11GetTxDataRate(PhyData *thisPhy) {
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;

    return phy802_11->pDot11Base->dataRate[phy802_11->pDot11Base->
                                                        txDataRateType];
}


int Phy802_11GetRxDataRate(PhyData *thisPhy) {
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;

    return phy802_11->pDot11Base->dataRate[phy802_11->pDot11Base->
                                                        rxDataRateType];
}


int Phy802_11GetTxDataRateType(PhyData *thisPhy) {
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;

    return phy802_11->pDot11Base->txDataRateType;
}

int Phy802_11GetRxDataRateType(PhyData *thisPhy, unsigned char& phyType) {
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;
    phyType = phy802_11->rxPhyType;
    return phy802_11->pDot11Base->rxDataRateType;
}


int Phy802_11GetRxDataRateType(PhyData *thisPhy) {
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;

    return phy802_11->pDot11Base->rxDataRateType;
}


void Phy802_11SetTxDataRateType(PhyData* thisPhy,
                                int dataRateType,
                                unsigned char phyType)
{
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;

    assert(dataRateType >= 0 && dataRateType < phy802_11->numDataRates);
    phy802_11->pDot11Base->txDataRateType = dataRateType;
    phy802_11->txPhyType = phyType;

    if (phy802_11->thisPhy->phyModel == PHY802_11a
        || phy802_11->thisPhy->phyModel == PHY802_11b
        || phy802_11->thisPhy->phyModel == PHY802_11pCCH
        || phy802_11->thisPhy->phyModel == PHY802_11pSCH)
    {
        phy802_11->txPower_dBm =
            phy802_11->pDot11Base->txDefaultPower_dBm[phy802_11->
                                                 pDot11Base->txDataRateType];
    }
    return;
}


void Phy802_11GetLowestTxDataRateType(PhyData* thisPhy, int* dataRateType) {
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;

    *dataRateType = phy802_11->lowestDataRateType;

    return;
}


void Phy802_11SetLowestTxDataRateType(PhyData* thisPhy) {
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    pDot11BasePtr->setLowestTxDataRateType();

    return;
}


void Phy802_11GetHighestTxDataRateType(PhyData* thisPhy,
                                       int* dataRateType) {
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;

    *dataRateType = phy802_11->highestDataRateType;

    return;
}


void Phy802_11SetHighestTxDataRateType(PhyData* thisPhy) {
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;

    phy802_11->pDot11Base->txDataRateType = phy802_11->highestDataRateType;
    phy802_11->txPower_dBm =
        phy802_11->pDot11Base->txDefaultPower_dBm[phy802_11->
                                                  pDot11Base->txDataRateType];

    return;
}

void Phy802_11GetHighestTxDataRateTypeForBC(
    PhyData* thisPhy,
    int* dataRateType)
{
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;

    *dataRateType = phy802_11->pDot11Base->txDataRateTypeForBC;

    return;
}


void Phy802_11SetHighestTxDataRateTypeForBC(
    PhyData* thisPhy)
{
    PhyData802_11* phy802_11 = (PhyData802_11*) thisPhy->phyVar;

    phy802_11->pDot11Base->txDataRateType = phy802_11->pDot11Base->
                                                        txDataRateTypeForBC;

    if (phy802_11->thisPhy->phyModel == PHY802_11a
        || phy802_11->thisPhy->phyModel == PHY802_11b
        || phy802_11->thisPhy->phyModel == PHY802_11pCCH
        || phy802_11->thisPhy->phyModel == PHY802_11pSCH)
    {
    phy802_11->txPower_dBm =
        phy802_11->pDot11Base->txDefaultPower_dBm[phy802_11->
                                                  pDot11Base->txDataRateType];
    }
    return;
}



clocktype Phy802_11GetFrameDuration(
    PhyData *thisPhy,
    int dataRateType,
    int size)
{
    Phy802_11Manager* pDot11BasePtr =
                                ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11BasePtr->getFrameDuration(dataRateType, size);
}



double Phy802_11GetLastAngleOfArrival(Node* node, int phyIndex)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11 *)thisPhy->phyVar;
    return phy802_11->rxDOA.azimuth;

}

static
void ReleaseSignalToChannel(
    Node *node,
    Message *packet,
    int phyIndex,
    int channelIndex,
    float txPower_dBm,
    clocktype duration,
    clocktype delayUntilAirborne,
    double directionalAntennaGain_dB)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11 *)thisPhy->phyVar;
    if (thisPhy->phyModel == PHY802_11n) {
        phy802_11->pDot11Base->releaseSignalToChannel(
           node,
           packet,
           phyIndex,
           channelIndex,
           txPower_dBm,
           duration,
           delayUntilAirborne,
           directionalAntennaGain_dB);
    }
    else if (thisPhy->phyModel == PHY802_11ac)
    {
        phy802_11->pDot11Base->releaseSignalToChannel(
           node,
           packet,
           phyIndex,
           channelIndex,
           txPower_dBm,
           duration,
           delayUntilAirborne,
           directionalAntennaGain_dB);
    }
    else
    {
        spectralBand* sb = phy802_11->pDot11Base->cc()->getSBand(
                            phy802_11->pDot11Base->cc()->getConfChBwdth());
        node->setMIMO_Data(phyIndex, 1, 0, sb);
        ERROR_Assert(!packet->m_mimoData,"Incorrect MIMO Data");
        packet->m_mimoData = new MIMO_Data();
        *packet->m_mimoData = node->phyData[phyIndex]->mimoData[
                node->phyData[phyIndex]->mimoElementCount -1];
        PROP_ReleaseSignal(
            node,
            packet,
            phyIndex,
            channelIndex,
            (float)(txPower_dBm - directionalAntennaGain_dB),
            duration,
            delayUntilAirborne);
    }
}

static
void StartTransmittingSignal(
    Node* node,
    int phyIndex,
    Message* packet,
    BOOL useMacLayerSpecifiedDelay,
    clocktype initDelayUntilAirborne,
    BOOL sendDirectionally,
    double azimuthAngle)
{
    if (DEBUG)
    {
        char clockStr[20];
        TIME_PrintClockInSecond(node->getNodeTime(), clockStr);
        printf("802_11.cpp: node %d start transmitting at time %s "
               "originated from node %d at time %15" TYPES_64BITFMT "d\n",
               node->nodeId, clockStr,
               packet->originatingNodeId, packet->packetCreationTime);
    }

    clocktype delayUntilAirborne = initDelayUntilAirborne;
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11 *)thisPhy->phyVar;
    int channelIndex;
    Message *endMsg;
    int packetsize = MESSAGE_ReturnPacketSize(packet);
    clocktype duration;

    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);
    if (!useMacLayerSpecifiedDelay) {
        delayUntilAirborne = phy802_11->rxTxTurnaroundTime;
#ifdef CYBER_LIB
        // Wormhole nodes do not delay
        if (node->macData[thisPhy->macInterfaceIndex]->isWormhole)
        {
            delayUntilAirborne = 0;
        }
#endif // CYBER_LIB
    }//if//

    assert(phy802_11->mode != PHY_TRANSMITTING);

    if (sendDirectionally) {
        ANTENNA_SetBestConfigurationForAzimuth(node, phyIndex,
                                               azimuthAngle);
    }


    if (phy802_11->mode == PHY_RECEIVING) {

        spectralBand* nodeSb = NULL;
        nodeSb = phy802_11->pDot11Base->cc()->getSBand(
                            phy802_11->pDot11Base->getOperationChBwdth());

        double rssi = node->sl().rssi(nodeSb,
                                      phy802_11->rxTimeEvaluated,
                                      phyIndex);
        double ni = NON_DB(rssi) - phy802_11->rxMsgPower_mW;

        PHY_NotificationOfPacketDrop(node,
                                     phyIndex,
                                     channelIndex,
                                     phy802_11->rxMsg,
                                     "PHY Stop Rx for Tx",
                                     phy802_11->rxMsgPower_mW,
                                     ni,
                                     0.0);

        if (phy802_11->thisPhy->phyStats)
        {
            phy802_11->thisPhy->stats->AddSignalTerminatedDataPoints(
                                        node,
                                        thisPhy,
                                        phy802_11->rxMsg,
                                        phy802_11->rxChannelIndex,
                                        phy802_11->txNodeId,
                                        phy802_11->pathloss_dB,
                                        phy802_11->rxTimeEvaluated,
                                        ni,
                                        phy802_11->rxMsgPower_mW);
        }
        Phy802_11UnlockSignal(phy802_11);
    }
    Phy802_11ChangeState(node, phyIndex, PHY_TRANSMITTING);

    if (thisPhy->phyModel == PHY802_11n
             || thisPhy->phyModel == PHY802_11ac)
    {
        const MAC_PHY_TxRxVector txVector =
                ((PhyData802_11*)thisPhy->phyVar)->pDot11Base->getTxVector();

        duration =
            Phy802_11GetFrameDuration(
                thisPhy,
                phy802_11->pDot11Base->getTxVector());

        spectralBand* sp =
                phy802_11->pDot11Base->cc()->getSBand(txVector.chBwdth);

        if (DEBUG)
        {
            std::cout << "setting outgoing sb: " << sp << std::endl;
        }

        MESSAGE_SetSpectralBand(node, packet, sp);
    }

    else
    {
        spectralBand* sp =
                phy802_11->pDot11Base->cc()->getSBand(
                                phy802_11->pDot11Base->cc()->getConfChBwdth());
        duration = Phy802_11GetFrameDuration(thisPhy,
                                             phy802_11->pDot11Base->
                                             txDataRateType,
                                             packetsize);
        MESSAGE_SetSpectralBand(node, packet, sp);
    }

    Phy802_11AddPlcpHeader(node, phy802_11, packet);

    if (PHY_IsListeningToChannel(node, phyIndex, channelIndex))
    {
        //node->setRadioListen(phyIndex, false);
        PHY_StopListeningToChannel(node, phyIndex, channelIndex);
    }

    int msgSize = 0;
    int overheadSize = 0;

    if (!packet->isPacked)
    {
        msgSize = MESSAGE_ReturnPacketSize(packet);
    }
    else
    {
        msgSize = MESSAGE_ReturnActualPacketSize(packet);
    }

    switch(node->phyData[phyIndex]->phyModel)
    {
        case PHY802_11a:
        {
            overheadSize = PHY802_11a_CONTROL_OVERHEAD_SIZE;
            break;
        }
        case PHY802_11b:
        {
            overheadSize = PHY802_11b_CONTROL_OVERHEAD_SIZE;
            break;
        }
        case PHY802_11n:
        {
            const MAC_PHY_TxRxVector txVector
                = ((PhyData802_11*)thisPhy->phyVar)->pDot11Base->getTxVector();

            int numSts = Phy802_11n::MCS_Params[txVector.chBwdth - 1]
                                            [txVector.mcs].m_nSpatialStream;
            clocktype dataRate
                = Phy802_11n::MCS_Params[CHBWDTH_20MHZ - 1][0].m_dataRate[0];
            clocktype preamble
                = Phy802_11n::preambDur_HtGf(numSts, txVector.numEss);
            overheadSize = (int)((preamble * dataRate) / SECOND
                           + Phy802_11n::Ppdu_Service_Bits_Size
                           + Phy802_11n::Ppdu_Tail_Bits_Size);
            overheadSize /= 8;
            break;
        }
        case PHY802_11ac:
        {
            const MAC_PHY_TxRxVector txVector
               = ((PhyData802_11*)thisPhy->phyVar)->pDot11Base->getTxVector();
            int numSts = Phy802_11ac::MCS_Params[txVector.chBwdth - 1]
                                               [txVector.mcs].m_nSpatialStream;
            clocktype dataRate
                = Phy802_11ac::MCS_Params[CHBWDTH_20MHZ - 1][0].m_dataRate[0];
            clocktype preamble
                = Phy802_11ac::preambDur_VHT(numSts);
            overheadSize = (int)((preamble * dataRate) / SECOND
                           + Phy802_11ac::Ppdu_Service_Bits_Size
                           + Phy802_11ac::Ppdu_Tail_Bits_Size);
            overheadSize /= 8;
            break;
        }
    }

    phy802_11->sController->updateStat(
            "PhySent",
            msgSize,
            overheadSize,
            phy802_11->txPower_dBm);

    if (thisPhy->antennaData->antennaModelType == ANTENNA_PATTERNED)
    {
        if (!sendDirectionally) {
            ReleaseSignalToChannel(
                node,
                packet,
                phyIndex,
                channelIndex,
                phy802_11->txPower_dBm,
                duration,
                delayUntilAirborne,
                0.0);
        }
        else {
            ReleaseSignalToChannel(
                node,
                packet,
                phyIndex,
                channelIndex,
                phy802_11->txPower_dBm,
                duration,
                delayUntilAirborne,
                phy802_11->directionalAntennaGain_dB);
        }
    }
    else {
        if (ANTENNA_IsInOmnidirectionalMode(node, phyIndex)) {
            ReleaseSignalToChannel(
                node,
                packet,
                phyIndex,
                channelIndex,
                phy802_11->txPower_dBm,
                duration,
                delayUntilAirborne,
                0.0);
        } else {
            ReleaseSignalToChannel(
                node,
                packet,
                phyIndex,
                channelIndex,
                phy802_11->txPower_dBm,
                duration,
                delayUntilAirborne,
                phy802_11->directionalAntennaGain_dB);
        }//if//
    }

    //GuiStart
     if (node->guiOption == TRUE) {
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
    MESSAGE_Send(node, endMsg, delayUntilAirborne + duration + 1);

    phy802_11->txEndTimer = endMsg;
    /* Keep track of phy statistics and battery computations */

    phy802_11->stats.energyConsumed
        += duration * (BATTERY_TX_POWER_COEFFICIENT
                       * NON_DB(phy802_11->txPower_dBm)
                       + BATTERY_TX_POWER_OFFSET
                       - BATTERY_RX_POWER);
    if (phy802_11->thisPhy->phyStats)
    {
        phy802_11->thisPhy->stats->AddSignalTransmittedDataPoints(node,
                                                                  duration);
    }
}



//
// Used by the MAC layer to start transmitting a packet.
//
void Phy802_11StartTransmittingSignal(
    Node* node,
    int phyIndex,
    Message* packet,
    BOOL useMacLayerSpecifiedDelay,
    clocktype initDelayUntilAirborne)
{
    StartTransmittingSignal(
        node, phyIndex,
        packet,
        useMacLayerSpecifiedDelay,
        initDelayUntilAirborne,
        FALSE, 0.0);
}


void Phy802_11StartTransmittingSignalDirectionally(
    Node* node,
    int phyIndex,
    Message* packet,
    BOOL useMacLayerSpecifiedDelay,
    clocktype initDelayUntilAirborne,
    double azimuthAngle)
{
    StartTransmittingSignal(
        node, phyIndex,
        packet,
        useMacLayerSpecifiedDelay,
        initDelayUntilAirborne,
        TRUE, azimuthAngle);
}



void Phy802_11LockAntennaDirection(Node* node, int phyNum) {
    ANTENNA_LockAntennaDirection(node, phyNum);
}



void Phy802_11UnlockAntennaDirection(Node* node, int phyIndex) {
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11 *)thisPhy->phyVar;

    ANTENNA_UnlockAntennaDirection(node, phyIndex);

    if ((phy802_11->mode != PHY_RECEIVING) &&
        (phy802_11->mode != PHY_TRANSMITTING))
    {
        int channelIndex;

        PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);

        ANTENNA_SetToDefaultMode(node, phyIndex);

        PHY_SignalInterference(
            node,
            phyIndex,
            channelIndex,
            NULL,
            NULL,
            &(phy802_11->interferencePower_mW));
    }//if//
}

// for phy connectivity table used in stats db and connection manager
BOOL Phy802_11ProcessSignal(Node* node,
                            int phyIndex,
                            PropRxInfo* propRxInfo,
                            double rxPower_dBm)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11 *)thisPhy->phyVar;
    BOOL ps = FALSE;

    if (thisPhy->phyModel == PHY802_11n ||
        thisPhy->phyModel == PHY802_11ac)
    {
        ps = phy802_11->pDot11Base->processSignal(phyIndex,
                                                  propRxInfo,
                                                  rxPower_dBm);
    }

    else if (thisPhy->phyModel == PHY802_11a ||
            thisPhy->phyModel == PHY802_11b)
    {
        ps = phy802_11->pDot11Base->processSignal(phyIndex);
    }

    return ps;
}

double Phy802_11GetTxPower(Node* node, int phyIndex) {

    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11 *)thisPhy->phyVar;

    if (ANTENNA_IsInOmnidirectionalMode(node, phyIndex)) {
        return phy802_11->txPower_dBm;
    } else {
        return (double)(phy802_11->txPower_dBm -
             phy802_11->directionalAntennaGain_dB);
    }//if//
}

#ifdef ADDON_DB
//--------------------------------------------------------------------------
//  FUNCTION: Phy802_11UpdateEventsTable
//  PURPOSE : Updates Stats-DB phy events table
//  PARAMETERS: Node* node
//                  Pointer to node
//              Int32 phyIndex
//                  Phy Index
//              Int32 interfaceIndex
//                  Interface Index
//              PropRxInfo* propRxInfo
//                  Pointer to propRxInfo
//              double rxMsgPower_mW
//                  Received signal power
//              Message* msgToMac
//                  Pointer to the Message
//              std::string eventStr
//                  Receives eventType
//  RETURN TYPE: NONE
//--------------------------------------------------------------------------
void Phy802_11UpdateEventsTable(Node* node,
                                Int32 phyIndex,
                                Int32 channelIndex,
                                PropRxInfo* propRxInfo,
                                double rxMsgPower_mW,
                                Message* msgToMac,
                                const char* eventStr)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11*)thisPhy->phyVar;
    StatsDb* db = node->partitionData->statsDb;

    NodeId* nextHop = (NodeId*)MESSAGE_ReturnInfo(propRxInfo->txMsg,
                                              INFO_TYPE_MessageNextPrevHop);
    NodeId rcvdId;

    if (nextHop != NULL)
    {
        // use ANY_ADDRESS
        rcvdId  = *nextHop;
    }
    else
    {
        rcvdId = ANY_DEST;
    }

    if (db != NULL
        && db->statsEventsTable->createPhyEventsTable
        && (rcvdId == node->nodeId || rcvdId == ANY_DEST))
    {
        Int32 controlSize = 0;
        StatsDBMappingParam* mapParamInfo = (StatsDBMappingParam*)
            MESSAGE_ReturnInfo(propRxInfo->txMsg,INFO_TYPE_StatsDbMapping);
        ERROR_Assert(mapParamInfo,
            "Error in StatsDB handling ReceiveSignal!");
        Int32 msgSize = 0;

        if (!msgToMac->isPacked)
        {
            msgSize = MESSAGE_ReturnPacketSize(msgToMac);
        }
        else
        {
            msgSize = MESSAGE_ReturnActualPacketSize(msgToMac);
        }

        StatsDBPhyEventParam phyParam(node->nodeId,
                                      (std::string)mapParamInfo->msgId,
                                      phyIndex,
                                      msgSize,
                                      eventStr);

        phyParam.SetChannelIndex(channelIndex);

        if (strcmp(eventStr, "PhyReceiveSignal") == 0)
        {
            switch(node->phyData[phyIndex]->phyModel)
            {
                case PHY802_11a:
                {
                    controlSize = PHY802_11a_CONTROL_OVERHEAD_SIZE;
                    break;
                }
                case PHY802_11b:
                {
                    controlSize = PHY802_11b_CONTROL_OVERHEAD_SIZE;
                    break;
                }
                case PHY802_11n:
                {
                    Phy802_11n* dot11n
                        = dynamic_cast<Phy802_11n*>(phy802_11->pDot11Base);
                    controlSize = dot11n->getControlOverhead();
                    break;
                }
                case PHY802_11ac:
                {
                    Phy802_11ac* dot11ac
                        = dynamic_cast<Phy802_11ac*>(phy802_11->pDot11Base);
                    controlSize = dot11ac->getControlOverhead();
                    break;
                }
                case PHY802_11pCCH:
                case PHY802_11pSCH:
                {
                    controlSize = PHY802_11p_CONTROL_OVERHEAD_SIZE;
                    break;
                }
                default:
                {
                    return;
                }
            }

            phyParam.SetControlSize(controlSize);
            phyParam.SetPathLoss_db(propRxInfo->pathloss_dB);
            phyParam.SetSignalPower(rxMsgPower_mW);
            phyParam.SetInterference(phy802_11->interferencePower_mW);
            STATSDB_HandlePhyEventsTableInsert(node, phyParam);
        }
    }
}
#endif

/*
 * 802.11n APIs
 */

int Phy802_11nGetNumAtnaElems(PhyData* thisPhy)
{
    int numAtnaElems = 1;
    switch(thisPhy->phyModel)
    {
        case PHY802_11ac:
        case PHY802_11n:
        {
            Phy802_11Manager* pDot11Base = ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
            numAtnaElems = pDot11Base->getNumAtnaElems();
            break;
        }
        default:
        {
            numAtnaElems = 1;
            break;
        }
    }
    return numAtnaElems;
}

double Phy802_11nGetAtnaElemSpace(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    if (pDot11BasePtr == 0) return 0.0;
    return pDot11BasePtr->getAtnaElemSpace();
}

BOOL Phy802_11nShortGiCapable(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11BasePtr->shortGiCapable();
}

BOOL Phy802_11nStbcCapable(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11BasePtr->stbcCapable();
}

ChBandwidth Phy802_11nGetOperationChBwdth(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11BasePtr->getOperationChBwdth();
}

void Phy802_11nGetTxVectorForHighestDataRate(PhyData* thisPhy,
                            MAC_PHY_TxRxVector& txVector)
{
    Phy802_11Manager* pDot11BasePtr =
                        ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    int numAntennaElements = pDot11BasePtr->getNumAtnaElems();
    ERROR_Assert(numAntennaElements <=4, "");

    // currently only 20Mhz & greenfiled mode is supported
    txVector.chBwdth = Phy802_11nGetOperationChBwdth(thisPhy);
    txVector.containAMPDU = FALSE;
    txVector.format = (Mode)MODE_HT_GF;
    txVector.gi = (GI)GI_LONG;
    txVector.length = 0;
    txVector.mcs =(numAntennaElements * 8) - 1;
    txVector.numEss = 0;
    txVector.sounding = FALSE;
    txVector.phyType = PHY802_11n;
}

/*
 * 802.11ac APIs
 */
clocktype Phy802_11GetFrameDuration(
    PhyData* thisPhy,
    const MAC_PHY_TxRxVector& txVector)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11BasePtr->getFrameDuration(txVector);
}

void Phy802_11SetTxVector(PhyData* thisPhy,
                           const MAC_PHY_TxRxVector& txVector)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11BasePtr->setTxVector(txVector);
}

int Phy802_11GetNumAtnaElems(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11BasePtr->getNumAtnaElems();
}

int Phy802_11GetNumSpatialStream(PhyData* thisPhy,
                                 MAC_PHY_TxRxVector* txVector)
{
    return Phy802_11n::MCS_Params[0][txVector->mcs].m_nSpatialStream;
}

double Phy802_11GetAtnaElemSpace(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11BasePtr->getAtnaElemSpace();
}

BOOL Phy802_11ShortGiCapable(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11BasePtr->shortGiCapable();
}

BOOL Phy802_11StbcCapable(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11BasePtr->stbcCapable();
}

ChBandwidth Phy802_11GetOperationChBwdth(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;

    return pDot11BasePtr->getOperationChBwdth();
}

void Phy802_11SetOperationChBwdth(PhyData* thisPhy,
                                   ChBandwidth chBwdth)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    pDot11BasePtr->setOperationChBwdth(chBwdth);
}

void Phy802_11GetTxVector(PhyData* thisPhy, MAC_PHY_TxRxVector& txVector)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    txVector = pDot11BasePtr->getTxVector();
}

BOOL Phy802_11IsGiEnabled(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11BasePtr->isGiEnabled();
}

void Phy802_11EnableGi(PhyData* thisPhy, BOOL enable)
{
   Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    pDot11BasePtr->enableGi(enable);
}

void Phy802_11GetTxVectorForBC(PhyData* thisPhy,
                                MAC_PHY_TxRxVector& txVector)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    txVector.chBwdth = Phy802_11GetOperationChBwdth(thisPhy);
    txVector.containAMPDU = FALSE;
    txVector.format = pDot11BasePtr->getDefaultMode();
    txVector.gi = (GI)GI_LONG;
    txVector.length = 0;
    txVector.mcs = 0;
    txVector.numEss = 0;
    txVector.sounding = FALSE;
    txVector.phyType =  txVector.format == (Mode)MODE_VHT? PHY802_11ac: PHY802_11n;
}

void Phy802_11GetTxVectorForHighestDataRate(PhyData* thisPhy,
                            MAC_PHY_TxRxVector& txVector)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    int numAntennaElements = pDot11BasePtr->getNumAtnaElems();

    ERROR_Assert(numAntennaElements <=8,
                 "Number of Antennas should be less than or equal to 8");

    txVector.chBwdth = Phy802_11GetOperationChBwdth(thisPhy);
    txVector.containAMPDU = FALSE;
    assert(pDot11BasePtr->getDefaultMode() == (Mode)MODE_VHT);
    txVector.format = (Mode)MODE_VHT;
    txVector.gi = (GI)GI_LONG;
    txVector.length = 0;
    txVector.mcs =(numAntennaElements * 10) - 1;
    txVector.numEss = 0;
    txVector.sounding = FALSE;
    txVector.phyType = PHY802_11ac;
}

void Phy802_11GetTxVectorForLowestDataRate(PhyData* thisPhy,
                            MAC_PHY_TxRxVector& txVector)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;

    txVector.chBwdth = Phy802_11GetOperationChBwdth(thisPhy);
    txVector.containAMPDU = FALSE;
    txVector.format = pDot11BasePtr->getDefaultMode();
    txVector.gi = (GI)GI_LONG;
    txVector.length = 0;
    txVector.mcs =0;
    txVector.numEss = 0;
    txVector.sounding = FALSE;
    txVector.phyType =  txVector.format == MODE_VHT? PHY802_11ac: PHY802_11n;
}

void Phy802_11GetRxVector(PhyData* thisPhy,
                           MAC_PHY_TxRxVector& rxVector)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    pDot11BasePtr->getPreviousRxVector(rxVector);
}

ChBandwidth Phy802_11CCA(PhyData* thisPhy, ChBandwidth chBwdth)
{
    Phy802_11Manager* pDot11BasePtr = ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11BasePtr->cca(chBwdth);
}

UInt8 PHY802_11GetPrimary20MHzChIdx(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11BasePtr =
                    ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return (UInt8)pDot11BasePtr->cc()->getPChIndex(CHBWDTH_20MHZ);
}

UInt8 PHY802_11GetSecondaryChOffset(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11BasePtr =
                       ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11BasePtr->cc()->is20MHzSChAbovePCh()? 1:3;

}

UInt8 PHY802_11GetChIdxHighestBwdth(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11BasePtr =
                   ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    ChBandwidth chBwdth = pDot11BasePtr->getOperationChBwdth();
    return (UInt8)pDot11BasePtr->cc()->getPChIndex(chBwdth);
}

void PHY802_11AllignSpectralBand(PhyData* thisPhy, UInt8 offset,
        ChBandwidth chBwdth, UInt8 ChIdx)
{
    Phy802_11Manager* pDot11BasePtr =
                       ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    std::vector<SubChannel*> chList = pDot11BasePtr->cc()->getSubChList();
    if (offset == 1)
    {
        chList[1]->setChIndex(chList[0]->getChIndex() + 4);
    }
    else
    {
        ERROR_Assert(offset == 3,"Wrong offset");
        chList[1]->setChIndex(chList[0]->getChIndex() - 4);
    }

    int chIdx40 = pDot11BasePtr->cc()->getPChIndex(CHBWDTH_40MHZ);

    if (ChIdx > chList[0]->getChIndex())
    {
        chList[2]->setChIndex(chIdx40 + 6);
        chList[3]->setChIndex(chIdx40 + 10);
    }
    else
    {
        chList[2]->setChIndex(chIdx40 - 6);
        chList[3]->setChIndex(chIdx40 - 10);
    }

    if (chBwdth == CHBWDTH_160MHZ && pDot11BasePtr->getOperationChBwdth()
             == CHBWDTH_160MHZ)
    {
        int chIdx80 = pDot11BasePtr->cc()->getPChIndex(CHBWDTH_80MHZ);
        if (ChIdx > chList[0]->getChIndex())
        {
            chList[4]->setChIndex(chIdx80 + 10);
            chList[5]->setChIndex(chIdx80 + 14);
            chList[6]->setChIndex(chIdx80 + 18);
            chList[7]->setChIndex(chIdx80 + 22);
        }
        else
        {
            chList[4]->setChIndex(chIdx80 - 10);
            chList[5]->setChIndex(chIdx80 - 14);
            chList[6]->setChIndex(chIdx80 - 18);
            chList[7]->setChIndex(chIdx80 - 22);
        }
    }
}

void Phy802_11SetNumActiveAtnaElems(PhyData* thisPhy, RfChainMode mode)
{
    Phy802_11Manager* pDot11Base = ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    pDot11Base->setNumActiveAtnaElems(mode);
}

int Phy802_11GetNumActiveAtnaElems(PhyData* thisPhy)
{
    Phy802_11Manager* pDot11Base = ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    return pDot11Base->getNumActiveAtnaElems();    
}

void Phy802_11nSetOperationChBwdth(PhyData* thisPhy,
                                   ChBandwidth chBwdth)
{
    Phy802_11Manager* pDot11BasePtr =
                            ((PhyData802_11*)thisPhy->phyVar)->pDot11Base;
    pDot11BasePtr->setOperationChBwdth(chBwdth);
}

void Phy802_11TuneRadio(Node* node, int phyIndex)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyData802_11* phy802_11 = (PhyData802_11*)thisPhy->phyVar;

    int radioOverlayId = phy802_11->pDot11Base->cc()->getRadioOverlayId();
    if (radioOverlayId >= 0)
    {
        node->setRadioListen(phyIndex, true);
    }
}

int Phy802_11GetDataRate(PhyData* thisPhy,
                         Phy802_11PlcpHeader* plcpHdr)
{
    int txDataRate = 0;
    switch (plcpHdr->txPhyModel)
    {
    case PHY802_11a:
    {
        switch (plcpHdr->rate)
        {
        case PHY802_11a__6M:
            txDataRate = PHY802_11a_DATA_RATE__6M;
            break;
        case PHY802_11a__9M:
            txDataRate = PHY802_11a_DATA_RATE__9M;
            break;
        case PHY802_11a_12M:
            txDataRate = PHY802_11a_DATA_RATE_12M;
            break;
        case PHY802_11a_18M:
            txDataRate = PHY802_11a_DATA_RATE_18M;
            break;
        case PHY802_11a_24M:
            txDataRate = PHY802_11a_DATA_RATE_24M;
            break;
        case PHY802_11a_36M:
            txDataRate = PHY802_11a_DATA_RATE_36M;
            break;
        case PHY802_11a_48M:
            txDataRate = PHY802_11a_DATA_RATE_48M;
            break;
        case PHY802_11a_54M:
            txDataRate = PHY802_11a_DATA_RATE_54M;
            break;
        }
        break;
    }
    case PHY802_11b:
    {
        switch (plcpHdr->rate)
        {
        case PHY802_11b__1M:
            txDataRate = PHY802_11b_DATA_RATE__1M;
            break;
        case PHY802_11b__2M:
            txDataRate = PHY802_11b_DATA_RATE__2M;
            break;
        case PHY802_11b__5_5M:
            txDataRate = PHY802_11b_DATA_RATE__5_5M;
            break;
        case PHY802_11b_11M:
            txDataRate = PHY802_11b_DATA_RATE_11M;
            break;
        }
        break;
    }
    case PHY802_11n:
    {
        txDataRate = (int)Phy802_11n::MCS_Params[plcpHdr->chBwdth - 1][plcpHdr->mcs]
            .m_dataRate[plcpHdr->gi];
        break;
    }
    case PHY802_11ac:
    {
        txDataRate = (int)Phy802_11ac::MCS_Params[plcpHdr->chBwdth - 1][plcpHdr->mcs]
            .m_dataRate[plcpHdr->gi];
        break;
    }
    case PHY802_11pCCH:
    case PHY802_11pSCH:
    {
        switch (plcpHdr->rate)
        {
        case PHY802_11p__3M:
            txDataRate = PHY802_11p_DATA_RATE__3M;
            break;
        case PHY802_11p_4_5M:
            txDataRate = PHY802_11p_DATA_RATE_4_5M;
            break;
        case PHY802_11p__6M:
            txDataRate = PHY802_11p_DATA_RATE__6M;
            break;
        case PHY802_11p__9M:
            txDataRate = PHY802_11p_DATA_RATE__9M;
            break;
        case PHY802_11p_12M:
            txDataRate = PHY802_11p_DATA_RATE_12M;
            break;
        case PHY802_11p_18M:
            txDataRate = PHY802_11p_DATA_RATE_18M;
            break;
        case PHY802_11p_24M:
            txDataRate = PHY802_11p_DATA_RATE_24M;
            break;
        case PHY802_11p_27M:
            txDataRate = PHY802_11p_DATA_RATE_27M;
            break;
        }
        break;
    }
    default:
    {
        ERROR_ReportError("Invalid phy model");
    }
    }
    return txDataRate;
}


int Phy802_11GetDataRate(PhyData* thisPhy,
                         MAC_PHY_TxRxVector* txVector)
{
    switch(thisPhy->phyModel)
    {
        case PHY802_11n:
        {
            return (int)Phy802_11n::MCS_Params[txVector->chBwdth - 1][txVector->mcs]
                                                    .m_dataRate[txVector->gi];
            break;
        }
        case PHY802_11ac:
        {
            return (int)Phy802_11ac::MCS_Params[txVector->chBwdth - 1][txVector->mcs]
                                                    .m_dataRate[txVector->gi];
            break;
        }
        default:
        {
            ERROR_ReportError("Invalid phy model");
        }
    }
}

double Phy802_11GetFrequencyForRadioRange(PhyData* thisPhy)
{
    PhyData802_11* phy802_11 = (PhyData802_11*)thisPhy->phyVar;
    spectralBand* sb = NULL;
    if (thisPhy->phyModel == PHY802_11n
        || thisPhy->phyModel == PHY802_11ac)
    {
        sb = phy802_11->pDot11Base->cc()->getSBand(CHBWDTH_20MHZ);
    }
    else
    {
        sb = phy802_11->pDot11Base->cc()->getSBand(
            phy802_11->pDot11Base->cc()->getConfChBwdth());
    }
    return sb->getFrequency();
}