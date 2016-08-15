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


/// \file mac_dot11_channelmanager.cpp
///
/// \brief Dot11 channel controller implementation
///
/// This file contains the implementation of channel controller APIs

#include "phy_dot11ac.h"
#include "mac_dot11_channelmanager.h"


using namespace Dot11;
using namespace Qos;

/// \brief Implementation of init API
///
/// This function initializes the channel controller
///
/// \param mode Dot11 mode of operation
/// \param nodeInput Pointer to NodeInput
void ChannelController::init(AcMode mode, const NodeInput* nodeInput)
{
    setMode(mode);
    readCfgParams(nodeInput);
}

/// \brief Implementation of destructor
///
/// This function frees the initialized vectors
///
ChannelController::~ChannelController()
{
    while (!m_subChList.empty())
    {
        SubChannel* sc = m_subChList.back();
        delete sc;
        m_subChList.pop_back();
    }
    while (!m_sbList.empty())
    {
        m_sbList.pop_back();
    }
}

/// \brief Implementation of getNi API
///
/// This function returns the interfernce in a spectral band
///
/// \param bd Pointer to spectral band of interest
/// \param propRxInfo Pointer to propRxinfo structure
///
/// \return Interfernce in a spectral band
double ChannelController::getNi(spectralBand* bd, PropRxInfo *propRxInfo)
{
    Node& node = *(phyManager()->node());
    int ifIdx = phyManager()->ifIdx();

    return node.sl().ni(propRxInfo, bd, ifIdx);
}



/// \brief Implementation of getChannelIndex API
///
/// This function returns the channel index of a bandwidth
///
/// \param centerFreq    Center frequency of spectral band
/// \param frequencyBand Frequency band
///
/// \return Channel index of bandwidth
Int32 ChannelController::getChannelIndex(double centerFreq, int frequencyBand)
{
    Int32 channelIndex;
    if (frequencyBand == GHz_2_4)
    {
        channelIndex = (Int32)((centerFreq - 2407.0e6)/5.0e6);
    }
    else if (frequencyBand == GHz_5)
    {
        channelIndex = (Int32)((centerFreq - 5000.0e6)/5.0e6);
    }
    return channelIndex;
}

/// \brief Implementation of readCfgParams API
///
/// This function reads the common configuration parameters
///
/// \param nodeInput Pointer to NodeInput
void ChannelController::readCfgParams(const NodeInput* nodeInput)
{
    BOOL wasFound;
    int phyIndex = phyManager()->phy()->thisPhy->phyIndex;
    Node* nodePtr = phyManager()->node();
    int radioOverlayId = -1;
    double frequencyBand = 5000.0e6;
    int phyModel = phyManager()->phy()->thisPhy->phyModel;
    int numChannels = PROP_NumberChannels(nodePtr);
    int i;

    IO_ReadInt(nodePtr,
               nodePtr->nodeId,
               nodePtr->phyData[phyIndex]->macInterfaceIndex,
               nodeInput,
               "PHY802.11-RADIO-OVERLAY-ID",
               &wasFound,
               &radioOverlayId);
    if (wasFound)
    {
        char errMsg[MAX_STRING_LENGTH];
        if (radioOverlayId < 0)
        {
            sprintf(errMsg,
                    "PHY802.11-RADIO-OVERLAY-ID configured at Node [%d] "
                    "interface [%d] should be a non negative number",
                    nodePtr->nodeId,
                    nodePtr->phyData[phyIndex]->macInterfaceIndex);
            ERROR_ReportError(errMsg);
        }
        if (radioOverlayId >= numChannels)
        {
            sprintf(errMsg,
                    "PHY802.11-RADIO-OVERLAY-ID configured at Node [%d] "
                    "interface [%d] should be less than number of qualnet "
                    "channels. Number of Qualnet channels configured : %d",
                    nodePtr->nodeId,
                    nodePtr->phyData[phyIndex]->macInterfaceIndex,
                    numChannels);
            ERROR_ReportError(errMsg);
        }
        m_radioOverlayId = radioOverlayId;
    }
    else
    {
        PHY_GetTransmissionChannel(nodePtr, phyIndex, &m_radioOverlayId);
    }

    for (i = 0; i < numChannels; i++)
    {
        if (PHY_CanListenToChannel(nodePtr, phyIndex, i)
            && i != m_radioOverlayId)
        {
            if (PHY_IsListeningToChannel(nodePtr, phyIndex, i))
            {
                // As interface should listen to channel specified by
                // radio overlayId only, stop listening to any other channel
                PHY_StopListeningToChannel(nodePtr, phyIndex, i);
            }
            PHY_DisallowListeningToChannel(nodePtr, phyIndex, i);
        }
    }

    if (!PHY_CanListenToChannel(nodePtr, phyIndex, m_radioOverlayId))
    {
        // Allow listening to channel specified by radio overlayId
        PHY_AllowListeningToChannel(nodePtr,
                                    phyIndex,
                                    m_radioOverlayId);
    }
    // Set up the channel specified by radio OverlayId to use
    // for both TX and RX.
    PHY_SetTransmissionChannel(nodePtr, phyIndex, m_radioOverlayId);

    // Register JLM observer for Wi-Fi models
    boost::shared_ptr<JammerObserver> jobserver
        = boost::shared_ptr<JammerObserver>(
            new JamDurationObserver(nodePtr, phyIndex, m_radioOverlayId));
    nodePtr->jlm().register_observer(phyIndex, m_radioOverlayId, jobserver);

    PropProfile* propProfile =
                     nodePtr->propChannel[m_radioOverlayId].profile;
    if (propProfile->enableChannelOverlapCheck)
    {
        ERROR_ReportWarning("The Inter-channel interference model is not "
            "supported with Wi-Fi models. "
            "Disabling inter-channel interference model");
        propProfile->enableChannelOverlapCheck = FALSE;
    }

    IO_ReadDouble(nodePtr,
                  nodePtr->nodeId,
                  nodePtr->phyData[phyIndex]->macInterfaceIndex,
                  nodeInput,
                  "PHY802.11-FREQUENCY-BAND",
                  &wasFound,
                  &frequencyBand);

    if (!wasFound)
    {
       if (phyModel == PHY802_11a || phyModel == PHY802_11n ||
           phyModel == PHY802_11ac)
       {
           frequencyBand = 5000.0e6;
       }
    }

    if (phyModel == PHY802_11b)
    {
        frequencyBand = 2400.0e6;
    }

    setFreqBand(frequencyBand);

}

/// \brief Implementation of readCfgParamsFor20MHz API
///
/// This function reads the configuration parameters
///
/// \param nodeInput Pointer to NodeInput
void ChannelController::readCfgParamsFor10MHz(const NodeInput* nodeInput)
{
    BOOL wasFound;
    int phyIndex = this->phyManager()->phy()->thisPhy->phyIndex;
    Node* nodePtr = this->phyManager()->node();
    PartitionData* partitionData = nodePtr->partitionData;
    spectralBand* sb = NULL;
    Int32 numSubcarriers = 52;

    // For 802.11p currently 10MHz bandwidth is supported
    // Set bandwidth to 10 MHz
    setChBwdth(CHBWDTH_10MHZ);
    phyManager()->setOperationChBwdth(CHBWDTH_10MHZ);
    Int32 chIdx10Mhz = 172;

    if (this->phyManager()->phy()->thisPhy->phyModel == PHY802_11pCCH)
    {
        // Control channel is on channel index 178
        chIdx10Mhz = 178;
    }
    else if (this->phyManager()->phy()->thisPhy->phyModel == PHY802_11pSCH)
    {
        IO_ReadInt(nodePtr,
               nodePtr->nodeId,
               nodePtr->phyData[phyIndex]->macInterfaceIndex,
               nodeInput,
               "PHY802.11-10MHz-CHANNEL-INDEX",
               &wasFound,
               &chIdx10Mhz);
        if (wasFound)
        {
            if (chIdx10Mhz >= 177
                && chIdx10Mhz <= 179)
            {
                ERROR_ReportError("Invalid Service channel index,"
                  "overlapping with control channel. Range of values for "
                  "10MHz primary Service channel index is [172 to 184] "
                  "except 177, 178 and 179");
            }

            ERROR_Assert(chIdx10Mhz >= 172 && chIdx10Mhz <= 184,
                        "Invalid Service channel index. Range of "
                        "values for 10MHz primary Service channel index is "
                        "[172 to 184] except 177, 178 and 179");
        }
    }
    sb = spectralBand_Square::makeBand(
                         spectralBand_Square::chanFreq_59GHz(chIdx10Mhz),
                         spectralBand_Square::CHBWDTH_10MHZ,
                         "802.11p",
                         partitionData);

    SubChannel* subCh = new SubChannel(numSubcarriers,
                                       PHY_IDLE,
                                       0,
                                       chIdx10Mhz);
    m_subChList.push_back(subCh);
    m_sbList.push_back(sb);
}

/// \brief Implementation of readCfgParamsFor20MHz API
///
/// This function reads the configuration parameters
///
/// \param nodeInput Pointer to NodeInput
void ChannelController::readCfgParamsFor20MHz(const NodeInput* nodeInput)
{
    BOOL wasFound;
    int phyIndex = phyManager()->phy()->thisPhy->phyIndex;
    Node* nodePtr = phyManager()->node();
    PartitionData* partitionData = nodePtr->partitionData;
    int frequencyBand = getFreqBand();
    int phyModel = phyManager()->phy()->thisPhy->phyModel;

    if (phyModel == PHY802_11a || phyModel == PHY802_11n)
    {
        ERROR_Assert(frequencyBand == GHz_2_4 || frequencyBand == GHz_5,
                    "Only 2.4GHz or 5GHz bands are supported for 802.11a/n");
    }

    else if (phyModel == PHY802_11b)
    {
        ERROR_Assert(frequencyBand == GHz_2_4,
                    "Only 2.4GHz band is supported for 802.11b");
    }

    else if (phyModel == PHY802_11ac)
    {
        ERROR_Assert(frequencyBand == GHz_5,
                    "Only 5GHz band is supported for 802.11ac");
    }

    Int32 chIdx20Mhz = 40;
    const char* phyModeStr = getPhyModeStr(phyModel);
    Int32 numSubcarriers = 52;
    spectralBand* sb = NULL;

    setChBwdth(CHBWDTH_20MHZ);

    IO_ReadInt(nodePtr,
               nodePtr->nodeId,
               nodePtr->phyData[phyIndex]->macInterfaceIndex,
               nodeInput,
               "PHY802.11-20MHz-CHANNEL-INDEX",
               &wasFound,
               &chIdx20Mhz);

    if (frequencyBand == GHz_2_4)
    {
        if (!wasFound)
        {
            chIdx20Mhz = 6;
        }
        ERROR_Assert(chIdx20Mhz >= 1 && chIdx20Mhz <= 12,
                    "Range of values for 20MHz primary channel index"
                    " in 2.4Ghz Band is [1 to 12]");
        ERROR_Assert(m_mode == k_Mode_Dot11n ||
                     m_mode == k_Mode_Legacy,
                     "2.4Ghz Band is only applicable for 802.11a/b/n");
        sb = spectralBand_Square::makeBand(
                     spectralBand_Square::chanFreq_24GHz(chIdx20Mhz),
                     spectralBand_Square::CHBWDTH_20MHZ,
                     phyModeStr,
                     partitionData,
                     m_radioOverlayId);
    }

    else if (frequencyBand == GHz_5)
    {
        ERROR_Assert(chIdx20Mhz >= 9 && chIdx20Mhz <= 163,
                    "Range of values for 20MHz primary channel index is"
                    " [9 to 163]");
        sb = spectralBand_Square::makeBand(
                     spectralBand_Square::chanFreq_50GHz(chIdx20Mhz),
                     spectralBand_Square::CHBWDTH_20MHZ,
                     phyModeStr,
                     partitionData,
                     m_radioOverlayId);
    }

    SubChannel* subCh = new SubChannel(numSubcarriers,
                                       PHY_IDLE, 0, chIdx20Mhz);
    m_subChList.push_back(subCh);
    m_sbList.push_back(sb);
}



/// \brief Implementation of readCfgParamsFor40MHz API
///
/// This function reads the configuration parameters
///
/// \param nodeInput Pointer to NodeInput
void ChannelController::readCfgParamsFor40MHz(const NodeInput* nodeInput)
{
    BOOL wasFound;
    int phyIndex = phyManager()->phy()->thisPhy->phyIndex;
    Node* nodePtr = phyManager()->node();
    PartitionData* partitionData = nodePtr->partitionData;
    int frequencyBand = getFreqBand();
    spectralBand* sb = NULL;
    double centerFreq = getSBand(CHBWDTH_20MHZ)->getFrequency();
    Int32 chIdx20Mhz = getChannelIndex(centerFreq, frequencyBand);
    Int32 nxt20MHzChIdx = 0;
    BOOL is40MHzSupported = FALSE;
    int phyModel = phyManager()->phy()->thisPhy->phyModel;
    const char* phyModeStr = getPhyModeStr(phyModel);
    Int32 numSubcarriers = 52;

    if (m_mode == k_Mode_Dot11n)
    {
        char cfgStr[MAX_STRING_LENGTH];
        IO_ReadString(nodePtr,
                      nodePtr->nodeId,
                      nodePtr->phyData[phyIndex]->macInterfaceIndex,
                      nodeInput,
                      "PHY802.11n-CHANNEL-BANDWIDTH",
                      &wasFound,
                      cfgStr);

        if (wasFound)
        {
            if (!strcmp(cfgStr, "40MHz")
                || !strcmp(cfgStr, "40MHZ")
                || !strcmp(cfgStr, "40Mhz"))
            {
                is40MHzSupported = TRUE;
            }

            else if (! (!strcmp(cfgStr, "20MHz")
                        || !strcmp(cfgStr, "20MHZ")
                        || !strcmp(cfgStr, "20Mhz")))
            {
                ERROR_ReportError(
                    "Error: Value for PHY802_11n-CHANNEL-BANDWIDTH"
                    "must be either 20MHz or 40MHz");
            }
        }

    }

    IO_ReadBool(nodePtr,
                nodePtr->nodeId,
                nodePtr->phyData[phyIndex]->macInterfaceIndex,
                nodeInput,
                "PHY802.11-40MHz-SUPPORTED",
                &wasFound,
                &is40MHzSupported);

    if (wasFound)
    {
        if (is40MHzSupported)
        {
            setChBwdth(CHBWDTH_40MHZ);
            phyManager()->setOperationChBwdth(CHBWDTH_40MHZ);
        }
    }

    if (is40MHzSupported || m_mode == k_Mode_Dot11ac)
    {
        Int32 chIdx40Mhz = 38;
        IO_ReadInt(nodePtr,
                   nodePtr->nodeId,
                   nodePtr->phyData[phyIndex]->macInterfaceIndex,
                   nodeInput,
                   "PHY802.11-40MHz-CHANNEL-INDEX",
                   &wasFound,
                   &chIdx40Mhz);

        if (frequencyBand == GHz_2_4)
        {
            if (!wasFound)
            {
                chIdx40Mhz = 4;
            }
            ERROR_Assert(chIdx40Mhz >= 3 && chIdx40Mhz <= 10,
                "Range of values for 40MHz primary channel index is"
                " [3 to 10]");
            sb = spectralBand_Square::makeBand(
                    spectralBand_Square::chanFreq_24GHz(chIdx40Mhz),
                    spectralBand_Square::CHBWDTH_40MHZ,
                    phyModeStr,
                    partitionData,
                    m_radioOverlayId);
        }

        else
        {
            ERROR_Assert(chIdx40Mhz >= 11 && chIdx40Mhz <= 161,
                "Range of values for 40MHz primary channel index is"
                " [11 to 161]");
            sb = spectralBand_Square::makeBand(
                    spectralBand_Square::chanFreq_50GHz(chIdx40Mhz),
                    spectralBand_Square::CHBWDTH_40MHZ,
                    phyModeStr,
                    partitionData,
                    m_radioOverlayId);
        }

        ERROR_Assert(abs(chIdx40Mhz - chIdx20Mhz) == 2,
                     "40MHz and 20MHz primary channel indexes"
                     " should be 10MHz (2 channel indexes) apart");

        if (chIdx40Mhz > chIdx20Mhz)
        {
            nxt20MHzChIdx = chIdx40Mhz + 2;
        }
        else
        {
            nxt20MHzChIdx = chIdx40Mhz - 2;
        }

        SubChannel* subCh = new SubChannel(numSubcarriers,
                                           PHY_IDLE, 0, nxt20MHzChIdx);
        m_subChList.push_back(subCh);
        m_sbList.push_back(sb);
    }
}



/// \brief Implementation of readCfgParamsFor80MHz API
///
/// This function reads the configuration parameters
///
/// \param nodeInput Pointer to NodeInput
void ChannelController::readCfgParamsFor80MHz(const NodeInput* nodeInput)
{
    BOOL wasFound;
    int phyIndex = phyManager()->phy()->thisPhy->phyIndex;
    Node* nodePtr = phyManager()->node();
    PartitionData* partitionData = nodePtr->partitionData;
    int frequencyBand = getFreqBand();
    Int32 chIdx80Mhz = 42;
    spectralBand* sb = NULL;
    double centerFreq = getSBand(CHBWDTH_40MHZ)->getFrequency();
    Int32 chIdx40Mhz = getChannelIndex(centerFreq, frequencyBand);
    Int32 nxt20MHzChIdx = 0;

    Int32 numSubcarriers = 52;

    setChBwdth(CHBWDTH_80MHZ);
    phyManager()->setOperationChBwdth(CHBWDTH_80MHZ);

    ERROR_Assert(m_mode == k_Mode_Dot11ac && frequencyBand == GHz_5,
                 "80MHz is supported only for 802.11ac in 5GHz band");

    IO_ReadInt(nodePtr,
               nodePtr->nodeId,
               nodePtr->phyData[phyIndex]->macInterfaceIndex,
               nodeInput,
               "PHY802.11-80MHz-CHANNEL-INDEX",
               &wasFound,
               &chIdx80Mhz);

    ERROR_Assert(chIdx80Mhz >= 15 && chIdx80Mhz <= 157,
                 "Range of values for 80MHz primary channel index is"
                 " [15 to 157]");
    ERROR_Assert(abs(chIdx80Mhz - chIdx40Mhz) == 4,
                 "80MHz and 40MHz primary channel indexes"
                 " should be 20MHz (4 channel indexes) apart");

    if (chIdx80Mhz > chIdx40Mhz)
    {
        nxt20MHzChIdx = chIdx40Mhz + 6;
    }
    else
    {
        nxt20MHzChIdx = chIdx40Mhz - 6;
    }

    SubChannel* subCh = new SubChannel(numSubcarriers,
                                       PHY_IDLE, 0, nxt20MHzChIdx);
    m_subChList.push_back(subCh);

    if (chIdx80Mhz > chIdx40Mhz)
    {
        nxt20MHzChIdx = chIdx40Mhz + 10;
    }
    else
    {
        nxt20MHzChIdx = chIdx40Mhz - 10;
    }

    subCh = new SubChannel(numSubcarriers, PHY_IDLE, 0, nxt20MHzChIdx);
    m_subChList.push_back(subCh);

    sb = spectralBand_Square::make_80211ac80(chIdx80Mhz,
                                             partitionData,
                                             m_radioOverlayId);
    m_sbList.push_back(sb);

}



/// \brief Implementation of readCfgParamsFor160MHz API
///
/// This function reads the configuration parameters
///
/// \param nodeInput Pointer to NodeInput
void ChannelController::readCfgParamsFor160MHz(const NodeInput* nodeInput)
{
    BOOL wasFound;
    BOOL is160MHzSupported = FALSE;
    int phyIndex = phyManager()->phy()->thisPhy->phyIndex;
    Node* nodePtr = phyManager()->node();
    PartitionData* partitionData = nodePtr->partitionData;
    int frequencyBand = getFreqBand();
    spectralBand* sb = NULL;
    double centerFreq = getSBand(CHBWDTH_80MHZ)->getFrequency();
    Int32 chIdx80Mhz = getChannelIndex(centerFreq, frequencyBand);
    Int32 nxt20MHzChIdx = 0;

    Int32 numSubcarriers = 52;

    ERROR_Assert(m_mode == k_Mode_Dot11ac && frequencyBand == GHz_5,
                "80MHz is supported only for 802.11ac in 5GHz band");

    IO_ReadBool(nodePtr,
                nodePtr->nodeId,
                nodePtr->phyData[phyIndex]->macInterfaceIndex,
                nodeInput,
                "PHY802.11-160MHz-SUPPORTED",
                &wasFound,
                &is160MHzSupported);

    if (wasFound)
    {
        if (is160MHzSupported)
        {
            setChBwdth(CHBWDTH_160MHZ);
            phyManager()->setOperationChBwdth(CHBWDTH_160MHZ);
        }
    }

    if (is160MHzSupported)
    {
        Int32 chIdx160Mhz = 50;
        IO_ReadInt(nodePtr,
                   nodePtr->nodeId,
                   nodePtr->phyData[phyIndex]->macInterfaceIndex,
                   nodeInput,
                   "PHY802.11-160MHz-CHANNEL-INDEX",
                   &wasFound,
                   &chIdx160Mhz);

        ERROR_Assert(chIdx160Mhz >= 23 && chIdx160Mhz <= 149,
                    "Range of values for 160MHz primary channel index is"
                    " [23 to 149]");
        ERROR_Assert(abs(chIdx160Mhz - chIdx80Mhz) == 8,
                    "160MHz and 80MHz primary channel indexes"
                    " should be 40MHz (8 channel indexes) apart");

        if (chIdx160Mhz > chIdx80Mhz)
        {
            nxt20MHzChIdx = chIdx80Mhz + 10;
        }
        else
        {
            nxt20MHzChIdx = chIdx80Mhz - 10;
        }

        SubChannel* subCh = new SubChannel(numSubcarriers,
                                           PHY_IDLE, 0, nxt20MHzChIdx);
        m_subChList.push_back(subCh);

        if (chIdx160Mhz > chIdx80Mhz)
        {
            nxt20MHzChIdx = chIdx80Mhz + 14;
        }
        else
        {
            nxt20MHzChIdx = chIdx80Mhz - 14;
        }

        subCh = new SubChannel(numSubcarriers, PHY_IDLE, 0, nxt20MHzChIdx);
        m_subChList.push_back(subCh);

        if (chIdx160Mhz > chIdx80Mhz)
        {
            nxt20MHzChIdx = chIdx80Mhz + 18;
        }
        else
        {
            nxt20MHzChIdx = chIdx80Mhz - 18;
        }

        subCh = new SubChannel(numSubcarriers, PHY_IDLE, 0, nxt20MHzChIdx);
        m_subChList.push_back(subCh);

        if (chIdx160Mhz > chIdx80Mhz)
        {
            nxt20MHzChIdx = chIdx80Mhz + 22;
        }
        else
        {
            nxt20MHzChIdx = chIdx80Mhz - 22;
        }

        subCh = new SubChannel(numSubcarriers, PHY_IDLE, 0, nxt20MHzChIdx);
        m_subChList.push_back(subCh);

        sb = spectralBand_Square::make_80211ac160(chIdx160Mhz,
                                                  partitionData,
                                                  m_radioOverlayId);
        m_sbList.push_back(sb);
    }
}

spectralBand* ChannelController::getSBand(ChBandwidth chBwth)
{
    std::vector<spectralBand*> :: iterator it;
    for (it = m_sbList.begin(); it < m_sbList.end(); it++)
    {
        if (Phy802_11Manager::getChBwdth((*it)->getBandwidth()) == chBwth)
        {
            return *it;
        }
    }
    return NULL;
}

