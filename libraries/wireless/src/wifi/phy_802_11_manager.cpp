/// \file phy_802_11_manager.cpp
///
/// \brief Phy 802_11 manager implementation
///
/// This file contains Class Phy802_11Manager implementation which serve
/// as base class for phy 802_11 family of protocols

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

#include "api.h"
#include "phy_802_11_manager.h"
#include "phy_802_11.h"
#include "antenna.h"

#include "phy_802_11p.h"

#define DEBUG_CHANMATRIX  0

const MAC_PHY_TxRxVector Phy802_11Manager::Def_TxRxVector;


/// \brief Finalize function for phy_802_11Manager class
///
/// For future use. Finalization and statistics printing is done in
/// Phy802_11Finalize() function.
///
/// \param node     Pointer to node
/// \param phyIndex Physical Layer index
///
void Phy802_11Manager::finalize(Node* node, int phyIndex){;}


/// \brief Get frame duration - 802.11n/ac
///
/// \param txParam  Reference to MAC_PHY_TxRxVector
///
/// \return Frame duration
clocktype Phy802_11Manager::getFrameDuration(const MAC_PHY_TxRxVector& txParam)
{
    return 0;
}


/// \brief Read basic 802_11 configuration parameters
///
/// This function reads basic 802_11 specific configuration parameters
/// from configuration file.
///
/// \param nodeInput Pointer to node input
void Phy802_11Manager::readCfgParams(const NodeInput* nodeInput)
{
    BOOL    wasFound;
    char    errStr[MAX_STRING_LENGTH];
    double  txPower_dBm;
    Node* node = m_node;
    int phyIndex = this->m_parentData->thisPhy->phyIndex;

    IO_ReadInt(
        node,
        node->nodeId,
        node->phyData[phyIndex]->macInterfaceIndex,
        nodeInput,
        "PHY802.11-NUM-ANTENNA-ELEMENTS",
        &wasFound,
        &m_NumAtnaElmts);

    if (m_NumAtnaElmts <= 0 || m_NumAtnaElmts > 8)
    {
        sprintf(errStr, "The range of PHY802.11-NUM-ANTENNA-ELEMENTS"
            " is [1 to 8]");
        ERROR_ReportError(errStr);
    }
    m_NumActiveAtnaElmts = m_NumAtnaElmts;

    IO_ReadDouble(
        node,
        node->nodeId,
        node->phyData[phyIndex]->macInterfaceIndex,
        nodeInput,
        "PHY802.11-ANTENNA-ELEMENT-SPACE",
        &wasFound,
        &m_AtnaSpace);

    if (m_AtnaSpace <= 0)
    {
        sprintf(errStr, "The value of PHY802.11-ANTENNA-ELEMENT-SPACE"
            " parameter should be greater than 0");
        ERROR_ReportError(errStr);
    }

    IO_ReadBool(
        node,
        node->nodeId,
        node->phyData[phyIndex]->macInterfaceIndex,
        nodeInput,
        "PHY802.11-SHORT-GI-CAPABLE",
        &wasFound,
        &m_ShortGiCapable);

    IO_ReadDouble(
        node,
        node->nodeId,
        node->phyData[phyIndex]->macInterfaceIndex,
        nodeInput,
        "PHY802.11-TX-POWER",
        &wasFound,
        &txPower_dBm);

    if (wasFound) {
        m_parentData->txPower_dBm = (float)txPower_dBm;
    }

}

/// \brief Sets Tx Vector
///
/// This function sets Transmission parameters for the outgoing packet.
///
/// \param txParam Reference of tx vector
void Phy802_11Manager::setTxVector(const MAC_PHY_TxRxVector& txVector)
{
    m_TxVector = txVector;
}

/// \brief Initializes Phy802_11Manager
///
/// This function initializes Phy802_11manager parameters
///
/// \param mode Operational mode
/// \parentData Pointer to Phy802_11 data structure
Phy802_11Manager::Phy802_11Manager(Mode mode, PhyData802_11* parentData,
        Dot11::Qos::ChannelController* p_ch,
        Node* node,
        int interfaceIndex,
        const NodeInput *nodeInput,
        AcMode phyMode)
{
    this->m_Mode = mode;
    this->m_parentData = parentData;
    this->m_node = node;
    this->m_interfaceIndex = interfaceIndex;
    m_ChBwdth       = CHBWDTH_20MHZ;
    m_NumAtnaElmts  = 1;
    m_AtnaSpace     = 0.5;
    m_ShortGiCapable= FALSE;
    m_StbcCapable   = FALSE;
    m_ShortGiEnabled= FALSE;
    m_chController = p_ch;
    m_chController->setManager(this);
    m_chController->init(phyMode, nodeInput);
}

/// \brief Fills plcp header
///
/// This function fills plcp header from txvector
///
/// \param Pointer to plcp header
void Phy802_11Manager::fillPlcpHdr(Phy802_11PlcpHeader* plcpHdr) const
{
    plcpHdr->format     = m_TxVector.format;
    plcpHdr->chBwdth    = m_TxVector.chBwdth;
    plcpHdr->length     = m_TxVector.length;
    plcpHdr->sounding   = m_TxVector.sounding;
    plcpHdr->containAMPDU = m_TxVector.containAMPDU ;
    plcpHdr->gi         = m_TxVector.gi;
    plcpHdr->mcs        = m_TxVector.mcs;
    plcpHdr->numEss     = m_TxVector.numEss;
    plcpHdr->dBwdthNonHT = m_TxVector.dBwdthNonHT;
    plcpHdr->nonHTMod    = m_TxVector.nonHTMod;

/************** Interoperability feature *********START*******************/
    // For supporting 11a/b/g devices.
    plcpHdr->rate = m_TxVector.mcs;
/************** Interoperability feature *********END*******************/
}

/// \brief Locks signal
///
/// This function locks incoming signal
///
/// \param node             Pointer to node
/// \param channelIndex     Transmission channel index
/// \param plcpHdr          Pointer to plcp header
/// \param txDOA
/// \param rxDOA
/// \param txNumAtnaElmts   Number of antenna array elements
/// \param txAtnaElmtSpace  Space between antenna array elements
void Phy802_11Manager::lockSignal(Node* node,
                            int channelIndex,
                            Phy802_11PlcpHeader* plcpHdr,
                            const  Orientation& txDOA,
                            const  Orientation& rxDOA,
                            int    txNumAtnaElmts,
                            double txAtnaElmtSpace)
{
    //Store tx parameters for the receiving parameters
    m_RxSignalInfo.m_rxVector.format      = plcpHdr->format;
    m_RxSignalInfo.m_rxVector.chBwdth     = plcpHdr->chBwdth;
    m_RxSignalInfo.m_rxVector.length      = plcpHdr->length;
    m_RxSignalInfo.m_rxVector.sounding    = plcpHdr->sounding;
    m_RxSignalInfo.m_rxVector.containAMPDU= plcpHdr->containAMPDU;
    m_RxSignalInfo.m_rxVector.gi          = plcpHdr->gi;
    m_RxSignalInfo.m_rxVector.mcs         = plcpHdr->mcs;
/************** Interoperability feature *********START*******************/
    m_RxSignalInfo.m_rxVector.phyType     = plcpHdr->txPhyModel;
/************** Interoperability feature *********END*******************/
    m_RxSignalInfo.m_rxVector.numEss      = plcpHdr->numEss;
    m_RxSignalInfo.m_rxVector.dBwdthNonHT = plcpHdr->dBwdthNonHT;
    m_RxSignalInfo.m_rxVector.nonHTMod    = plcpHdr->nonHTMod;
    m_RxSignalInfo.m_txNumAtnaElmts       = txNumAtnaElmts;
    m_RxSignalInfo.m_txAtnaElmtSpace      = txAtnaElmtSpace;

    // Calculate MIMO channel matrix estimation
    PropProfile* propProfile =
    node->partitionData->propChannel[channelIndex].profile;

    m_RxSignalInfo.m_chnlEstMatrix = MIMO_EstimateChnlMatrix(
                                     propProfile,
                                     m_parentData->thisPhy->seed,
                                     txAtnaElmtSpace,
                                     txNumAtnaElmts,
                                     m_AtnaSpace,
                                     m_NumAtnaElmts,
                                     txDOA,
                                     rxDOA);

    m_PrevRxVector = m_RxSignalInfo.m_rxVector;

    if (DEBUG_CHANMATRIX)
    {
        std::cout << "At time:" << node->getNodeTime() << " node:"
            << node->nodeId <<"receives a signal.Number of transmit antennas:"
            << txNumAtnaElmts << " number of receive antennas "
            << m_NumAtnaElmts <<" Channel matrix: " << std::endl;
        std::cout << m_RxSignalInfo.m_chnlEstMatrix << std::endl;
    }
}

/// \brief Unlocks signal
void Phy802_11Manager::unlockSignal()
{
    m_RxSignalInfo.m_rxVector = Def_TxRxVector;
    m_RxSignalInfo.m_chnlEstMatrix.Clear();
}

/// \brief Releases signalto channel
///
///
/// \param node                       Pointer to node
/// \param packet                     Pointer to message
/// \param phyIndex                   Physical layer index
/// \param channelIndex               Transmission channel index
/// \param txPower_dBm                Transmission power in dBm
/// \param duration                   Frame duration
/// \param delayUntilAirborne         Delay until airborne
/// \param directionalAntennaGain_dB  Antenna Gain in dB
void Phy802_11Manager::releaseSignalToChannel(
        Node* node,
        Message* packet,
        int phyIndex,
        int channelIndex,
        float txPower_dBm,
        clocktype duration,
        clocktype delayUntilAirborne,
        double directionalAntennaGain_dB)
{
    unsigned char mcs = getTxVector().mcs;
    STA_Capability_Mode mode = (STA_Capability_Mode)getTxVector().phyType;
    unsigned char numA = 1;
    if (mode == k_Dot11ac)
    {
        ERROR_Assert(mcs < 80, "Invalid MCS");
        numA = mcs / 10 + 1;
    }
    else if (k_Dot11n)
    {
        if (mcs >= 8 && mcs < 16)
            numA = 2;
        else if (mcs >= 16 && mcs < 24)
            numA = 3;
        else if (mcs >= 24 && mcs < 32)
            numA = 4;
        else 
            assert(mcs < 32);
    }

    double txPowerPerAtna_mw = NON_DB(txPower_dBm)/numA;

    spectralBand* sp =
                cc()->getSBand(getTxVector().chBwdth);
    node->setMIMO_Data(phyIndex, numA, m_AtnaSpace, sp);

    // Attach mimo data to message
    ERROR_Assert(!packet->m_mimoData,"Incorrect MIMO Data");
    packet->m_mimoData = new MIMO_Data();
    *packet->m_mimoData = node->phyData[phyIndex]->mimoData[
            node->phyData[phyIndex]->mimoElementCount -1];

    PROP_ReleaseSignal(
       node,
       packet,
       phyIndex,
       channelIndex,
       (float)(IN_DB(txPowerPerAtna_mw) - directionalAntennaGain_dB),
       duration,
       delayUntilAirborne,
       numA,
       m_AtnaSpace);
}

/// \brief Returns channel bandwidth enumerator
///
/// This function converts bandwidth into its enum identifier
///
/// \param bw Channel Bandwidth
///
/// \return Channel bandwidth enum
ChBandwidth Phy802_11Manager::getChBwdth(double bw)
{
    ERROR_Assert(bw > 0, "Invalid Bandwidth");
    ChBandwidth ch = CHBWDTH_10MHZ;
    const int baseBw = 10000000;
    int bandwidth = (int)bw;
    switch (bandwidth)
    {
        case baseBw:
        {
            ch = CHBWDTH_10MHZ;
            break;
        }
        case baseBw * 2:
        {
            ch = CHBWDTH_20MHZ;
            break;
        }
        case baseBw * 4:
        {
            ch = CHBWDTH_40MHZ;
            break;
        }
        case baseBw * 8:
        {
            ch = CHBWDTH_80MHZ;
            break;
        }
        case baseBw * 16:
        {
            ch = CHBWDTH_160MHZ;
            break;
        }
        default:
        {
             ch = CHBWDTH_10MHZ;
             break;
        }
    }
    return ch;
}

/// \brief Returns non Ht mode bandwidth
///
/// \param count
///
/// \return Channel Bandwidth
ChBandwidth Phy802_11Manager::getNonHtBwdth(UInt8 count)
{
    ChBandwidth bwdth = CHBWDTH_20MHZ;

    if (count == 1)
    {
        bwdth = CHBWDTH_20MHZ;
    }
    else if (count < 4)
    {
        bwdth = CHBWDTH_40MHZ;
    }
    else if (count < 8)
    {
        bwdth = CHBWDTH_80MHZ;
    }
    else if (count == 8)
    {
        bwdth = CHBWDTH_160MHZ;
    }
    else
    {
        ERROR_ReportError("Wrong bandwidth count");
    }

    return bwdth;
}

/// \brief Returns channel bandwidth in Mhz
///
/// \param bwdth Channel bandwidth
///
/// \return Channel bandwidth
double Phy802_11Manager::getBwdth_MHz(ChBandwidth bwdth)
{
    double bwdth_MHz = 20.0e6;
    switch (bwdth)
    {
        case CHBWDTH_10MHZ:
        {
            bwdth_MHz = 10.0e6;
            break;
        }
        case CHBWDTH_20MHZ:
        {
            bwdth_MHz = 20.0e6;
            break;
        }
        case CHBWDTH_40MHZ:
        {
            bwdth_MHz = 40.0e6;
            break;
        }
        case CHBWDTH_80MHZ:
        {
            bwdth_MHz = 80.0e6;
            break;
        }
        case CHBWDTH_160MHZ:
        {
            bwdth_MHz = 160.0e6;
            break;
        }
        default:
        {
            ERROR_ReportError("Wrong bandwidth used");
            break;
        }
    }
    return bwdth_MHz;
}

void Phy802_11Manager::initDefaultParam()
{
    this->phy802_11aNdbps[PHY802_11a__6M] =
        PHY802_11a_NUM_DATA_BITS_PER_SYMBOL__6M;
    this->phy802_11aNdbps[PHY802_11a__9M] =
        PHY802_11a_NUM_DATA_BITS_PER_SYMBOL__9M;
    this->phy802_11aNdbps[PHY802_11a_12M] =
        PHY802_11a_NUM_DATA_BITS_PER_SYMBOL_12M;
    this->phy802_11aNdbps[PHY802_11a_18M] =
        PHY802_11a_NUM_DATA_BITS_PER_SYMBOL_18M;
    this->phy802_11aNdbps[PHY802_11a_24M] =
        PHY802_11a_NUM_DATA_BITS_PER_SYMBOL_24M;
    this->phy802_11aNdbps[PHY802_11a_36M] =
        PHY802_11a_NUM_DATA_BITS_PER_SYMBOL_36M;
    this->phy802_11aNdbps[PHY802_11a_48M] =
        PHY802_11a_NUM_DATA_BITS_PER_SYMBOL_48M;
    this->phy802_11aNdbps[PHY802_11a_54M] =
        PHY802_11a_NUM_DATA_BITS_PER_SYMBOL_54M;

    this->phy802_11bNdbps[PHY802_11b__1M] =
        PHY802_11b_NUM_DATA_BITS_PER_SYMBOL__1M;
    this->phy802_11bNdbps[PHY802_11b__2M] =
        PHY802_11b_NUM_DATA_BITS_PER_SYMBOL__2M;
    this->phy802_11bNdbps[PHY802_11b__5_5M] =
        PHY802_11b_NUM_DATA_BITS_PER_SYMBOL__5_5M;
    this->phy802_11bNdbps[PHY802_11b_11M] =
        PHY802_11b_NUM_DATA_BITS_PER_SYMBOL_11M;
 }


/// \brief Function to terminate the current receive
///
/// This function terminates the current receive
///
/// \param phyIndex  Physical index
void Phy802_11Manager::terminateCurrentReceive(int phyIndex)
{
    PhyData802_11* phy802_11 = m_parentData;
    spectralBand* msgBand = MESSAGE_GetSpectralBand(phy802_11->rxMsg);
    double rssi_dBm = node()->sl().rssi(msgBand,
                                            node()->getNodeTime(),
                                            phyIndex);
    double rssi_mw = NON_DB(rssi_dBm);
    double intf_mw = rssi_mw - phy802_11->rxMsgPower_mW;
    Int32 chIndex;
    PHY_GetTransmissionChannel(node(), phyIndex, &chIndex);
    PHY_NotificationOfPacketDrop(
        node(),
        phyIndex,
        chIndex,
        phy802_11->rxMsg,
        "Rx Terminated by MAC",
        phy802_11->rxMsgPower_mW,
        intf_mw,
        0.0);

    if (phy802_11->thisPhy->phyStats)
    {
        phy802_11->thisPhy->stats->AddSignalTerminatedDataPoints(
                                        node(),
                                        phy802_11->thisPhy,
                                        phy802_11->rxMsg,
                                        phy802_11->rxChannelIndex,
                                        phy802_11->txNodeId,
                                        phy802_11->pathloss_dB,
                                        phy802_11->rxTimeEvaluated,
                                        intf_mw,
                                        phy802_11->rxMsgPower_mW);
    }

    Phy802_11UnlockSignal(phy802_11);

    if (carrierSensing(FALSE, phyIndex))
    {
        Phy802_11ChangeState(node(), phyIndex, PHY_SENSING);
    }
    else
    {
        Phy802_11ChangeState(node(), phyIndex, PHY_IDLE);
    }
}

PhyRxModel Phy802_11Manager::getRxModel(MAC_PHY_TxRxVector rxVector)
{
    STA_Capability_Mode mode = (STA_Capability_Mode)rxVector.phyType;
    PhyRxModel rxModel = RX_802_11b;
    if (m_parentData->thisPhy->phyRxModel == RX_802_11n
        || m_parentData->thisPhy->phyRxModel == RX_802_11ac)
    {
        switch (mode)
        {
        case k_Dot11a:
            rxModel = RX_802_11a;
            break;
        case k_Dot11b:
            rxModel = RX_802_11b;
            break;
        case k_Dot11n:
            rxModel = RX_802_11n;
            break;
        case k_Dot11ac:
            if (m_parentData->thisPhy->phyModel == PHY802_11n)
            {
                rxModel = RX_802_11n;
            }
            else
            {
                ERROR_Assert(m_parentData->thisPhy->phyModel == PHY802_11ac,
                    "Invalid PHY mode");
                rxModel = RX_802_11ac;
            }
            break;
        default:
            ERROR_ReportError("Invalid PHY model");
            break;
        }
    }
    else
    {
        rxModel = m_parentData->thisPhy->phyRxModel;
    }
    return rxModel;
}


/// \brief Set lowest tx data rate type
void Phy802_11Manager::setLowestTxDataRateType()
{
    PhyData802_11* phy802_11 = m_parentData;

    this->txDataRateType = phy802_11->lowestDataRateType;
    if (phy802_11->thisPhy->phyModel == PHY802_11a
        ||phy802_11->thisPhy->phyModel == PHY802_11b
        || phy802_11->thisPhy->phyModel == PHY802_11pCCH
        || phy802_11->thisPhy->phyModel == PHY802_11pSCH)
    {
        phy802_11->txPower_dBm =
                    this->txDefaultPower_dBm[this->txDataRateType];
    }
}

/// \brief Function to get frame duration
///
/// This function calculates frame duration
///
/// \param dataRateType int
/// \param size         int
///
/// \return Frame duration
clocktype Phy802_11Manager::getFrameDuration(int dataRateType,
                                             int size)
{
    PhyData802_11* phy802_11 = m_parentData;

    switch (phy802_11->thisPhy->phyModel) {
        case PHY802_11a: {
            const PhyData802_11* phy802_11a = m_parentData;
            if (phy802_11a->txPhyType == k_Dot11b)
            {
                const int numSymbols = (int)ceil
                             (size * 8 / this->numDBPS[dataRateType]);
                return PHY802_11b_SYNCHRONIZATION_TIME + numSymbols
                                                             * MICRO_SECOND;
            }
            else
            {
                const int numOfdmSymbols =
                    (int)ceil((size * 8 +
                               PHY802_11a_SERVICE_BITS_SIZE +
                               PHY802_11a_TAIL_BITS_SIZE) /
                              this->numDataBitsPerSymbol[dataRateType]);
                return
                    PHY802_11a_SYNCHRONIZATION_TIME +
                    (numOfdmSymbols * PHY802_11a_OFDM_SYMBOL_DURATION) *
                    MICRO_SECOND;
            }
        }

        case PHY802_11b: {
            const int numSymbols =
                (int)ceil(size * 8 / this->numDataBitsPerSymbol
                                                       [dataRateType]);
            return PHY802_11b_SYNCHRONIZATION_TIME + numSymbols
                                                    * MICRO_SECOND;
        }
        case PHY802_11pCCH:
        case PHY802_11pSCH: {
            const int numOfdmSymbols =
                (int)ceil((size * 8 +
                           PHY802_11p_SERVICE_BITS_SIZE +
                           PHY802_11p_TAIL_BITS_SIZE) /
                           this->numDataBitsPerSymbol[dataRateType]);
            return
                PHY802_11p_SYNCHRONIZATION_TIME +
                (numOfdmSymbols * PHY802_11p_OFDM_SYMBOL_DURATION) *
                MICRO_SECOND;
        }
        default:
        {
            ERROR_ReportError("Unknown PHY model!\n");
            break;
        }
    }
}


/// \brief Function to terminate the current transmission
///
/// This function terminates the current transmission
///
/// \param node      Pointer to Node
/// \param phyIndex  Physical index
void Phy802_11Manager::terminateCurrentTransmission(Node* node, int phyIndex)
{
    PhyData802_11* phy802_11 = m_parentData;
    int channelIndex;
    PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);

    //GuiStart
    if (node->guiOption == TRUE) {
        GUI_EndBroadcast(node->nodeId,
                         GUI_PHY_LAYER,
                         GUI_DEFAULT_DATA_TYPE,
                         this->m_parentData->thisPhy->macInterfaceIndex,
                         node->getNodeTime());
    }
    //GuiEnd
    assert(phy802_11->mode == PHY_TRANSMITTING);

    if (!ANTENNA_IsLocked(node, phyIndex)) {
        ANTENNA_SetToDefaultMode(node, phyIndex);
    }

    //Cancel the timer end message so that 'Phy802_11TransmissionEnd' is
    // not called.
    if (phy802_11->txEndTimer)
    {
        MESSAGE_CancelSelfMsg(node, phy802_11->txEndTimer);
        phy802_11->txEndTimer = NULL;
    }
}

/// \brief Function to get default channel estimation matrix
///
/// This function return default channel estimation matrix which is used
/// for node's radio range calculation
///
/// \return Channel estimation matrix
CplxMiniMatrix Phy802_11Manager::getDefaultChEstimationMatrix(Node* txNode,
                                                              Node* rxNode)
{
    PropProfile* propProfile =
      m_node->partitionData->propChannel[cc()->getRadioOverlayId()].profile;

    Orientation txNodeOrientation;
    Orientation rxNodeOrientation;

    MOBILITY_ReturnOrientation(txNode, &txNodeOrientation);
    MOBILITY_ReturnOrientation(rxNode, &rxNodeOrientation);

    return  MIMO_EstimateChnlMatrix(propProfile,
                                    m_parentData->thisPhy->seed,
                                    m_AtnaSpace,
                                    1,
                                    m_AtnaSpace,
                                    1,
                                    txNodeOrientation,
                                    rxNodeOrientation);
}



