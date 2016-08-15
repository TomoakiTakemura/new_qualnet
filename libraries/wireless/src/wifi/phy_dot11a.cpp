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
#include "phy_802_11.h"
#include "phy_dot11a.h"
#include "partition.h"
#include "propagation.h"

#include "phy_802_11p.h"
#include "mimo-mapper.h"

#define DEBUG 0
#define DEBUG_CHANMATRIX  0

using namespace Dot11;
using namespace Qos;


/// \brief Initializes Phy802_11a
///
/// This function initializes 802_11 a/b by calling initialization
/// function for channel controller and reading configuration parameters
///
/// \param nodeInput Pointer to node input
void Phy802_11a::init(const NodeInput* nodeInput)
{
    readCfgParams(nodeInput);
}


/// \brief Finalize function for Phy802_11a
///
/// For future use. Finalization and statistics printing is done in
/// Phy802_11Finalize() function.
///
/// \param node     Pointer to Node
/// \param phyIndex Phy Index
void Phy802_11a::finalize(Node* node, int phyIndex)
{
}


/// \brief Read bandwidth parameters for Phy802_11a
///
/// This function reads the bandwidth parameters
///
/// \param nodeInput Pointer to node input
void Phy802_11a::readBwdthParams(const NodeInput* nodeInput)
{
    if (m_parentData->thisPhy->phyModel == PHY802_11pCCH
                 || m_parentData->thisPhy->phyModel == PHY802_11pSCH)
    {
        m_chController->readCfgParamsFor10MHz(nodeInput);
    }
    else
    {
        m_chController->readCfgParamsFor20MHz(nodeInput);
    }

    spectralBand* sp = m_chController->getSBand(
                m_chController->getConfChBwdth());
    m_node->setRadioBand(
                m_parentData->thisPhy->phyIndex, sp);
}


/// \brief Set user configurable parameters of Phy802_11a and Phy802_11b
///
/// This function sets the user configurable parameters of 802_11a and 802_11b
///
/// \param NodeInput Pointer to NodeInput
void Phy802_11a::readCfgParams(const NodeInput *nodeInput)
{
    Node* node = m_node;
    int phyIndex = m_parentData->thisPhy->phyIndex;
    BOOL wasFound = TRUE;
    BOOL yes = FALSE;
    int i = 0;
    int dataRateForBroadcast = 0;

    IO_ReadBool(node,
                node->nodeId,
                node->phyData[phyIndex]->macInterfaceIndex,
                nodeInput,
                "PHY802.11-AUTO-RATE-FALLBACK",
                &wasFound,
                &yes);

    if (!wasFound || yes == FALSE)
    {
        BOOL wasFound1;
        int dataRate1;

        IO_ReadInt(node,
                   node->nodeId,
                   node->phyData[phyIndex]->macInterfaceIndex,
                   nodeInput,
                   "PHY802.11-DATA-RATE",
                   &wasFound1,
                   &dataRate1);

        if (wasFound1)
        {
            for (i = 0; i < m_parentData->numDataRates; i++)
            {
                if (dataRate1 == this->dataRate[i])
                {
                    break;
                }
            }

            if (i >= m_parentData->numDataRates)
            {
                ERROR_ReportError(
                            "Specified PHY802.11-DATA-RATE is not "
                            "in the supported data rate set");
            }

            m_parentData->lowestDataRateType = i;
            m_parentData->highestDataRateType = i;
        }

        else
        {
            ERROR_ReportError(
                        "PHY802.11-DATA-RATE not set without "
                        "PHY802.11-AUTO-RATE-FALLBACK turned on");
        }
    }

    this->setLowestTxDataRateType();

    //
    // Set PHY802_11-DATA-RATE-FOR-BROADCAST
    //
    IO_ReadInt(node,
               node->nodeId,
               node->phyData[phyIndex]->macInterfaceIndex,
               nodeInput,
               "PHY802.11-DATA-RATE-FOR-BROADCAST",
               &wasFound,
               &dataRateForBroadcast);

    if (wasFound)
    {
        for (i = 0; i < m_parentData->numDataRates; i++)
        {
            if (dataRateForBroadcast == this->dataRate[i])
            {
                break;
            }
        }

        if (i < m_parentData->lowestDataRateType ||
            i > m_parentData->highestDataRateType)
        {
            ERROR_ReportError(
                        "Specified PHY802.11-DATA-RATE-FOR-BROADCAST is not "
                        "in the data rate set");
        }

        this->txDataRateTypeForBC = i;
    }

    else
    {
        this->txDataRateTypeForBC =
                    MIN(m_parentData->highestDataRateType,
                        MAX(m_parentData->lowestDataRateType,
                            this->txDataRateTypeForBC));
    }
}


/// \brief Initializes Phy802_11a default parameters
///
/// This function initializes the default parameters of 802_11a
///
/// \param NodeInput Pointer to NodeInput
void Phy802_11a::initDefaultParams_a(const NodeInput* nodeInput)
{
    m_parentData->numDataRates = PHY802_11a_NUM_DATA_RATES;

    txDefaultPower_dBm[PHY802_11a__6M] =
                                   PHY802_11a_DEFAULT_TX_POWER__6M_dBm;
    txDefaultPower_dBm[PHY802_11a__9M] =
                                   PHY802_11a_DEFAULT_TX_POWER__9M_dBm;
    txDefaultPower_dBm[PHY802_11a_12M] =
                                   PHY802_11a_DEFAULT_TX_POWER_12M_dBm;
    txDefaultPower_dBm[PHY802_11a_18M] =
                                   PHY802_11a_DEFAULT_TX_POWER_18M_dBm;
    txDefaultPower_dBm[PHY802_11a_24M] =
                                   PHY802_11a_DEFAULT_TX_POWER_24M_dBm;
    txDefaultPower_dBm[PHY802_11a_36M] =
                                   PHY802_11a_DEFAULT_TX_POWER_36M_dBm;
    txDefaultPower_dBm[PHY802_11a_48M] =
                                   PHY802_11a_DEFAULT_TX_POWER_48M_dBm;
    txDefaultPower_dBm[PHY802_11a_54M] =
                                   PHY802_11a_DEFAULT_TX_POWER_54M_dBm;

    m_parentData->rxSensitivity_mW[PHY802_11a__6M] =
                    NON_DB(PHY802_11a_DEFAULT_RX_SENSITIVITY__6M_dBm);
    m_parentData->rxSensitivity_mW[PHY802_11a__9M] =
                    NON_DB(PHY802_11a_DEFAULT_RX_SENSITIVITY__9M_dBm);
    m_parentData->rxSensitivity_mW[PHY802_11a_12M] =
                    NON_DB(PHY802_11a_DEFAULT_RX_SENSITIVITY_12M_dBm);
    m_parentData->rxSensitivity_mW[PHY802_11a_18M] =
                    NON_DB(PHY802_11a_DEFAULT_RX_SENSITIVITY_18M_dBm);
    m_parentData->rxSensitivity_mW[PHY802_11a_24M] =
                    NON_DB(PHY802_11a_DEFAULT_RX_SENSITIVITY_24M_dBm);
    m_parentData->rxSensitivity_mW[PHY802_11a_36M] =
                    NON_DB(PHY802_11a_DEFAULT_RX_SENSITIVITY_36M_dBm);
    m_parentData->rxSensitivity_mW[PHY802_11a_48M] =
                    NON_DB(PHY802_11a_DEFAULT_RX_SENSITIVITY_48M_dBm);
    m_parentData->rxSensitivity_mW[PHY802_11a_54M] =
                    NON_DB(PHY802_11a_DEFAULT_RX_SENSITIVITY_54M_dBm);

    dataRate[PHY802_11a__6M] = PHY802_11a_DATA_RATE__6M;
    dataRate[PHY802_11a__9M] = PHY802_11a_DATA_RATE__9M;
    dataRate[PHY802_11a_12M] = PHY802_11a_DATA_RATE_12M;
    dataRate[PHY802_11a_18M] = PHY802_11a_DATA_RATE_18M;
    dataRate[PHY802_11a_24M] = PHY802_11a_DATA_RATE_24M;
    dataRate[PHY802_11a_36M] = PHY802_11a_DATA_RATE_36M;
    dataRate[PHY802_11a_48M] = PHY802_11a_DATA_RATE_48M;
    dataRate[PHY802_11a_54M] = PHY802_11a_DATA_RATE_54M;

    numDataBitsPerSymbol[PHY802_11a__6M] =
                               PHY802_11a_NUM_DATA_BITS_PER_SYMBOL__6M;
    numDataBitsPerSymbol[PHY802_11a__9M] =
                               PHY802_11a_NUM_DATA_BITS_PER_SYMBOL__9M;
    numDataBitsPerSymbol[PHY802_11a_12M] =
                               PHY802_11a_NUM_DATA_BITS_PER_SYMBOL_12M;
    numDataBitsPerSymbol[PHY802_11a_18M] =
                               PHY802_11a_NUM_DATA_BITS_PER_SYMBOL_18M;
    numDataBitsPerSymbol[PHY802_11a_24M] =
                               PHY802_11a_NUM_DATA_BITS_PER_SYMBOL_24M;
    numDataBitsPerSymbol[PHY802_11a_36M] =
                               PHY802_11a_NUM_DATA_BITS_PER_SYMBOL_36M;
    numDataBitsPerSymbol[PHY802_11a_48M] =
                               PHY802_11a_NUM_DATA_BITS_PER_SYMBOL_48M;
    numDataBitsPerSymbol[PHY802_11a_54M] =
                               PHY802_11a_NUM_DATA_BITS_PER_SYMBOL_54M;

    // Initialize num symbol for 802.11b interoperability
    numDBPS[PHY802_11b__1M] = PHY802_11b_NUM_DATA_BITS_PER_SYMBOL__1M;
    numDBPS[PHY802_11b__2M] = PHY802_11b_NUM_DATA_BITS_PER_SYMBOL__2M;
    numDBPS[PHY802_11b__5_5M] = PHY802_11b_NUM_DATA_BITS_PER_SYMBOL__5_5M;
    numDBPS[PHY802_11b_11M] = PHY802_11b_NUM_DATA_BITS_PER_SYMBOL_11M;

    m_parentData->lowestDataRateType = PHY802_11a_LOWEST_DATA_RATE_TYPE;
    m_parentData->highestDataRateType = PHY802_11a_HIGHEST_DATA_RATE_TYPE;
    txDataRateTypeForBC = PHY802_11a_DATA_RATE_TYPE_FOR_BC;

    m_parentData->channelBandwidth = PHY802_11a_CHANNEL_BANDWIDTH;
    m_parentData->rxTxTurnaroundTime = PHY802_11a_RX_TX_TURNAROUND_TIME;
}

/// \brief Set user configurable parameters of Phy802_11a
/// for backward compatibility.
///
/// \param NodeInput Pointer to NodeInput
void Phy802_11a::readCfgParamsForBackwardCompatibility_a(
    const NodeInput *nodeInput)
{
    Node* node = m_node;
    int phyIndex = m_parentData->thisPhy->phyIndex;
    double rxSensitivity_dBm;
    double txPower_dBm;
    BOOL wasFound;
    char buf[1024];

    const int k_types[] = { 6, 9, 12, 18, 24, 36, 48, 54 };

    //
    // Set PHY802_11a-TX-POWER's
    //
    for (int k1(0); k1 < PHY802_11a_NUM_DATA_RATES; k1++)
    {
        const int val = k_types[k1];

        if (val > 10)
        {
            (void)sprintf(buf, "PHY802.11a-TX-POWER-%dMBPS", val);
        }

        else
        {
            (void)sprintf(buf, "PHY802.11a-TX-POWER--%dMBPS", val);
        }

        IO_ReadDouble(node, node->nodeId,
            node->phyData[phyIndex]->macInterfaceIndex,
            nodeInput,
            buf,
            &wasFound,
            &txPower_dBm);

        if (wasFound)
        {
            txDefaultPower_dBm[k1] = (float)txPower_dBm;
        }
    }

    //
    // Set PHY802_11a-RX-SENSITIVITY's
    //
    for (int k1(0); k1 < PHY802_11a_NUM_DATA_RATES; k1++)
    {
        const int val = k_types[k1];

        if (val > 10)
        {
            (void)sprintf(buf, "PHY802.11a-RX-SENSITIVITY-%dMBPS", val);
        }

        else
        {
            (void)sprintf(buf, "PHY802.11a-RX-SENSITIVITY--%dMBPS", val);
        }

        IO_ReadDouble(node, node->nodeId,
            node->phyData[phyIndex]->macInterfaceIndex,
            nodeInput,
            buf,
            &wasFound,
            &rxSensitivity_dBm);

        if (wasFound)
        {
            m_parentData->rxSensitivity_mW[k1] = NON_DB(rxSensitivity_dBm);
        }
    }

}

/// \brief Set user configurable parameters of Phy802_11a
///
/// This function sets the user configurable parameters of 802_11a
///
/// \param NodeInput Pointer to NodeInput
void Phy802_11a::readCfgParams_a(const NodeInput *nodeInput)
{
    Node* node = m_node;
    int phyIndex = m_parentData->thisPhy->phyIndex;
    double rxSensitivity_dBm;
    double txPower_dBm;
    BOOL wasFound;
    char buf[1024];

    const int k_types[] = { 6, 9, 12, 18, 24, 36, 48, 54};

    //
    // Set PHY802_11a-TX-POWER's
    //
    for (int k1(0); k1 < PHY802_11a_NUM_DATA_RATES; k1++)
    {
        const int val = k_types[k1];
        (void)sprintf(buf, "PHY802.11-TX-POWER-%dMBPS", val);

        IO_ReadDouble(node, node->nodeId,
                      node->phyData[phyIndex]->macInterfaceIndex,
                      nodeInput,
                      buf,
                      &wasFound,
                      &txPower_dBm);

        if (wasFound)
        {
            txDefaultPower_dBm[k1] = (float)txPower_dBm;
        }
    }

    //
    // Set PHY802_11a-RX-SENSITIVITY's
    //
    for (int k1(0); k1 < PHY802_11a_NUM_DATA_RATES; k1++)
    {
        const int val = k_types[k1];
        (void)sprintf(buf, "PHY802.11-RX-SENSITIVITY-%dMBPS", val);

        IO_ReadDouble(node, node->nodeId,
                      node->phyData[phyIndex]->macInterfaceIndex,
                      nodeInput,
                      buf,
                      &wasFound,
                      &rxSensitivity_dBm);

        if (wasFound)
        {
            m_parentData->rxSensitivity_mW[k1] = NON_DB(rxSensitivity_dBm);
        }
    }
}


/// \brief Initializes Phy802_11b default parameters
///
/// This function initializes the default parameters of 802_11b
///
/// \param NodeInput Pointer to NodeInput
void Phy802_11a::initDefaultParams_b(const NodeInput *nodeInput)
{
    Node* node = m_node;
    int phyIndex = m_parentData->thisPhy->phyIndex;

    BOOL wasFound;

    useLegacy802_11b = TRUE;

    // Read parameter to enable/disable Legacy Phy802.11b calculations
    IO_ReadBool(node,
                node->nodeId,
                node->phyData[phyIndex]->macInterfaceIndex,
                nodeInput,
                "PHY802.11-USE-LEGACY-802.11b",
                &wasFound,
                &useLegacy802_11b);
    if (useLegacy802_11b)
    {
        char outputStr[MAX_STRING_LENGTH];
        sprintf(outputStr,
                "Legacy Phy802.11b is configured on interface"
                " [%d] of Node [%d]",
                node->phyData[phyIndex]->macInterfaceIndex,
                node->nodeId);
        ERROR_ReportWarning(outputStr);
    }

    m_parentData->numDataRates = PHY802_11b_NUM_DATA_RATES;

    txDefaultPower_dBm[PHY802_11b__1M] =
                                     PHY802_11b_DEFAULT_TX_POWER__1M_dBm;
    txDefaultPower_dBm[PHY802_11b__2M] =
                                     PHY802_11b_DEFAULT_TX_POWER__2M_dBm;
    txDefaultPower_dBm[PHY802_11b__5_5M] =
                                     PHY802_11b_DEFAULT_TX_POWER__5_5M_dBm;
    txDefaultPower_dBm[PHY802_11b_11M] =
                                     PHY802_11b_DEFAULT_TX_POWER_11M_dBm;

    if (useLegacy802_11b)
    {
        node->phyData[phyIndex]->noise_mW_hz /= 10;
        m_parentData->rxSensitivity_mW[PHY802_11b__1M] =
            NON_DB(LEGACY_PHY802_11b_DEFAULT_RX_SENSITIVITY__1M_dBm);
        m_parentData->rxSensitivity_mW[PHY802_11b__2M] =
            NON_DB(LEGACY_PHY802_11b_DEFAULT_RX_SENSITIVITY__2M_dBm);
        m_parentData->rxSensitivity_mW[PHY802_11b__5_5M] =
            NON_DB(LEGACY_PHY802_11b_DEFAULT_RX_SENSITIVITY__5_5M_dBm);
        m_parentData->rxSensitivity_mW[PHY802_11b_11M] =
            NON_DB(LEGACY_PHY802_11b_DEFAULT_RX_SENSITIVITY_11M_dBm);
    }
    else
    {
        m_parentData->rxSensitivity_mW[PHY802_11b__1M] =
            NON_DB(PHY802_11b_DEFAULT_RX_SENSITIVITY__1M_dBm);
        m_parentData->rxSensitivity_mW[PHY802_11b__2M] =
            NON_DB(PHY802_11b_DEFAULT_RX_SENSITIVITY__2M_dBm);
        m_parentData->rxSensitivity_mW[PHY802_11b__5_5M] =
            NON_DB(PHY802_11b_DEFAULT_RX_SENSITIVITY__5_5M_dBm);
        m_parentData->rxSensitivity_mW[PHY802_11b_11M] =
            NON_DB(PHY802_11b_DEFAULT_RX_SENSITIVITY_11M_dBm);
    }
    dataRate[PHY802_11b__1M] = PHY802_11b_DATA_RATE__1M;
    dataRate[PHY802_11b__2M] = PHY802_11b_DATA_RATE__2M;
    dataRate[PHY802_11b__5_5M] = PHY802_11b_DATA_RATE__5_5M;
    dataRate[PHY802_11b_11M] = PHY802_11b_DATA_RATE_11M;

    numDataBitsPerSymbol[PHY802_11b__1M] =
                                PHY802_11b_NUM_DATA_BITS_PER_SYMBOL__1M;
    numDataBitsPerSymbol[PHY802_11b__2M] =
                                PHY802_11b_NUM_DATA_BITS_PER_SYMBOL__2M;
    numDataBitsPerSymbol[PHY802_11b__5_5M] =
                                PHY802_11b_NUM_DATA_BITS_PER_SYMBOL__5_5M;
    numDataBitsPerSymbol[PHY802_11b_11M] =
                                PHY802_11b_NUM_DATA_BITS_PER_SYMBOL_11M;

    m_parentData->lowestDataRateType = PHY802_11b_LOWEST_DATA_RATE_TYPE;
    m_parentData->highestDataRateType = PHY802_11b_HIGHEST_DATA_RATE_TYPE;
    txDataRateTypeForBC = PHY802_11b_DATA_RATE_TYPE_FOR_BC;

    rxDataRateType = PHY802_11b_LOWEST_DATA_RATE_TYPE;

    m_parentData->channelBandwidth = PHY802_11b_CHANNEL_BANDWIDTH;
    m_parentData->rxTxTurnaroundTime = PHY802_11b_RX_TX_TURNAROUND_TIME;
}

/// \brief Set user configurable parameters of Phy802_11b
///
/// This function sets the user configurable parameters of 802_11b
/// for backward compatibility.
///
/// \param NodeInput Pointer to NodeInput
void Phy802_11a::readCfgParamsForBackwardCompatibility_b(
                                const NodeInput *nodeInput)
{
    Node* node = m_node;
    int phyIndex = m_parentData->thisPhy->phyIndex;
    double rxSensitivity_dBm;
    double txPower_dBm;
    BOOL wasFound;
    char buf[1024];

    const int k_types[] = { 1, 2, 6, 11 };

    //
    // Set PHY802_11b-TX-POWER's
    //
    for (int k1(0); k1 < PHY802_11b_NUM_DATA_RATES; k1++)
    {
        const int val = k_types[k1];

        if (val == 11)
        {
            (void)sprintf(buf, "PHY802.11b-TX-POWER-%dMBPS", val);
        }

        else
        {
            (void)sprintf(buf, "PHY802.11b-TX-POWER--%dMBPS", val);
        }

        IO_ReadDouble(node, node->nodeId,
            node->phyData[phyIndex]->macInterfaceIndex,
            nodeInput,
            buf,
            &wasFound,
            &txPower_dBm);

        if (wasFound)
        {
            txDefaultPower_dBm[k1] = (float)txPower_dBm;
        }
    }

    //
    // Set PHY802_11b-RX-SENSITIVITY's
    //
    for (int k1(0); k1 < PHY802_11b_NUM_DATA_RATES; k1++)
    {
        const int val = k_types[k1];

        if (val == 11)
        {
            (void)sprintf(buf, "PHY802.11b-RX-SENSITIVITY-%dMBPS", val);
        }

        else
        {
            (void)sprintf(buf, "PHY802.11b-RX-SENSITIVITY--%dMBPS", val);
        }

        IO_ReadDouble(node, node->nodeId,
            node->phyData[phyIndex]->macInterfaceIndex,
            nodeInput,
            buf,
            &wasFound,
            &rxSensitivity_dBm);

        if (wasFound)
        {
            m_parentData->rxSensitivity_mW[k1] = NON_DB(rxSensitivity_dBm);
        }
    }
}


/// \brief Set user configurable parameters of Phy802_11b
///
/// This function sets the user configurable parameters of 802_11b
///
/// \param NodeInput Pointer to NodeInput
void Phy802_11a::readCfgParams_b(const NodeInput *nodeInput)
{
    Node* node = m_node;
    int phyIndex = m_parentData->thisPhy->phyIndex;
    double rxSensitivity_dBm;
    double txPower_dBm;
    BOOL wasFound;
    char buf[1024];

    const char* k_types[] = { "1", "2", "5.5", "11"};

    //
    // Set PHY802_11b-TX-POWER's
    //
    for (int k1(0); k1 < PHY802_11b_NUM_DATA_RATES; k1++)
    {
        const char* val = k_types[k1];
        (void)sprintf(buf, "PHY802.11-TX-POWER-%sMBPS", val);

        IO_ReadDouble(node, node->nodeId,
                      node->phyData[phyIndex]->macInterfaceIndex,
                      nodeInput,
                      buf,
                      &wasFound,
                      &txPower_dBm);

        if (wasFound)
        {
            txDefaultPower_dBm[k1] = (float)txPower_dBm;
        }
    }

    //
    // Set PHY802_11b-RX-SENSITIVITY's
    //
    for (int k1(0); k1 < PHY802_11b_NUM_DATA_RATES; k1++)
    {
        const char* val = k_types[k1];
        (void)sprintf(buf, "PHY802.11-RX-SENSITIVITY-%sMBPS", val);

        IO_ReadDouble(node, node->nodeId,
                      node->phyData[phyIndex]->macInterfaceIndex,
                      nodeInput,
                      buf,
                      &wasFound,
                      &rxSensitivity_dBm);

        if (wasFound)
        {
            m_parentData->rxSensitivity_mW[k1] = NON_DB(rxSensitivity_dBm);
        }
    }
}


/// \brief Handles signal arrival event
///
/// This function decides to lock the signal or not based on spectral band,
/// phy mode, rssi etc.Phy carrier sensing is also done.
///
/// \param node         Pointer to Node
/// \param phyIndex     Phy index
/// \param channelIndex Channel Index
/// \param propRxInfo   Propagation Rx Info
void Phy802_11a::signalArrival(Node* node,
                               int phyIndex,
                               int channelIndex,
                               PropRxInfo *propRxInfo)
{
    PhyData802_11* phy802_11 = m_parentData;

    const spectralBand* band = node->getRadioBand(phyIndex);

    Phy802_11PlcpHeader* plcp
        = (Phy802_11PlcpHeader*)MESSAGE_ReturnPacket(propRxInfo->txMsg);
    ERROR_Assert(plcp != NULL,"Plcp header not found");

    double ni_dBm = node->sl().ni(propRxInfo, band, phyIndex);
    double s_dBm = node->sl().s(propRxInfo, phyIndex);

    switch (phy802_11->mode) {
        case PHY_RECEIVING:
        {
            PHY_NotificationOfPacketDrop(
                node,
                phyIndex,
                channelIndex,
                propRxInfo->txMsg,
                "PHY Busy in Receiving",
                pow(10.0, s_dBm/10.0),
                pow(10.0, ni_dBm/10.0),
                propRxInfo->pathloss_dB);

            DEBUG_PRINT("Signal Arrival in receiving Status: interference power: %lf \n",
                pow(10.0, ni_dBm/10.0));
            break;
        }

        //
        // If the phy is idle or sensing,
        // check if it can receive this signal.
        //
        case PHY_IDLE:
        case PHY_SENSING:
        {
            spectralBand* msb = MESSAGE_GetSpectralBand(propRxInfo->txMsg);
            ChBandwidth mchBwdth = getChBwdth(msb->getBandwidth());

            BOOL process = FALSE;
            if (mchBwdth <= getOperationChBwdth())
            {
                spectralBand* tsb = cc()->getSBand(mchBwdth);
                if (tsb && tsb->getFrequency() == msb->getFrequency()
                    && plcp->format == MODE_NON_HT)
                {
                    // 11a/b/g PHY.
                    // Process only those signals which are received on the
                    // band this PHY is tuned to.
                    // 11a/b/g device can only process packets with NON_HT
                    // format
                    process = TRUE;
                }
            }

            if (plcp->rate >= phy802_11->numDataRates)
            {
                process = FALSE;
            }

            if (!process)
            {
                // can not receive message due to frequency mismatch
                PHY_NotificationOfPacketDrop(node,
                                    phyIndex,
                                    channelIndex,
                                    propRxInfo->txMsg,
                                    "Signal below Rx Threshold",
                                    pow(10.0, s_dBm/10.0),
                                    pow(10.0, ni_dBm/10.0),
                                    propRxInfo->pathloss_dB);

                PhyStatusType newMode;
                if (carrierSensing(FALSE, phyIndex))
                {
                    newMode = PHY_SENSING;
                }
                else
                {
                    newMode = PHY_IDLE;
                }

                if (newMode != phy802_11->mode)
                {
                    Phy802_11ChangeState(node, phyIndex, newMode);
                    Phy802_11ReportStatusToMac(node, phyIndex, newMode);
                }
                return;
            }

            double rxSensitivity_dBm = IN_DB(phy802_11->rxSensitivity_mW[0]);
            if (s_dBm >= rxSensitivity_dBm)
            {
                PropTxInfo *propTxInfo
                    = (PropTxInfo*)MESSAGE_ReturnInfo(propRxInfo->txMsg);

                clocktype txDuration = propTxInfo->duration;

                Phy802_11LockSignal(
                    node,
                    phy802_11,
                    propRxInfo,
                    propRxInfo->txMsg,
                    pow(10.0, s_dBm/10.0),
                    (propRxInfo->rxStartTime + propRxInfo->duration),
                    channelIndex,
                    propRxInfo->txDOA,
                    propRxInfo->rxDOA);

#ifdef CYBER_LIB
                if (node->phyData[phyIndex]->jammerStatistics == TRUE)
                {
                    if (node->phyData[phyIndex]->jamInstances > 0)
                    {
                        phy802_11->stats.totalSignalsLockedDuringJam++;
                    }
                }
#endif

                Phy802_11ChangeState(node, phyIndex, PHY_RECEIVING);
                Phy802_11ReportExtendedStatusToMac(node,
                    phyIndex,
                    PHY_RECEIVING,
                    txDuration,
                    propRxInfo->txMsg);
            }
            else {
                //
                // Otherwise, check if the signal changes the phy status
                PHY_NotificationOfPacketDrop(node,
                    phyIndex,
                    channelIndex,
                    propRxInfo->txMsg,
                    "Signal below Rx Threshold",
                    pow(10.0, s_dBm/10.0),
                    pow(10.0, ni_dBm/10.0),
                    propRxInfo->pathloss_dB);

                PhyStatusType newMode;

                if (carrierSensing(FALSE, phyIndex))
                {
                    newMode = PHY_SENSING;
                }
                else
                {
                    newMode = PHY_IDLE;
                }

                if (newMode != phy802_11->mode)
                {
                    Phy802_11ChangeState(node, phyIndex, newMode);

                    Phy802_11ReportStatusToMac(node,
                        phyIndex,
                        newMode);
                }

            }
            break;
        }
        case PHY_TRX_OFF:
        {
            break;
        }

        default:
            abort();

    }//switch (phy802_11->mode)//
}


/// \brief Handles the signal end event
///
/// This function handles signal end event, function decides
/// whether to receive message and pass it to MAC or not.
/// Set phy state, update statistics.
///
/// \param node           Pointer to node
/// \param phyIndex       Phy Index
/// \param channelIndex   Channel Index
/// \param propRxInfo     Pointer to propagation Rx info
void Phy802_11a::signalEnd(Node* node,
                           int phyIndex,
                           int channelIndex,
                           PropRxInfo *propRxInfo)
{
    double sinr = -1.0;
    PhyData802_11* phy802_11 = m_parentData;

    ERROR_Assert(phy802_11->mode != PHY_TRANSMITTING,
                     "Phy should not be in transmitting mode at signal end");

    if (DEBUG)
    {
        char currTime[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(node->getNodeTime(), currTime);
        printf("\ntime %s: SignalEnd from %d at %d\n",
               currTime,
               propRxInfo->txMsg->originatingNodeId,
               node->nodeId);
    }

    bool noiseModified = false;

    if (phy802_11->thisPhy->phyModel == PHY802_11b
        && this->useLegacy802_11b)
    {
        if (this->rxDataRateType == PHY802_11b__5_5M
            || this->rxDataRateType == PHY802_11b_11M)
        {
            // Noise adjustment for supporting Legacy Phy802.11b
            // calculations
            node->phyData[phyIndex]->noise_mW_hz *= 11;
            noiseModified = true;
        }
    }

    spectralBand* msgBand = MESSAGE_GetSpectralBand(propRxInfo->txMsg);
    double s_dBm = node->sl().s(propRxInfo, phyIndex);
    double ni_dBm = node->sl().ni(propRxInfo, msgBand, phyIndex);
    double rssi_dBm = node->sl().rssi(msgBand, node->getNodeTime(), phyIndex);
    double noise_dBm = node->sl().n(phyIndex, msgBand);

    // revert back noise_mw_hz
    if (noiseModified)
    {
        node->phyData[phyIndex]->noise_mW_hz /= 11;
    }

    //
    // If the phy is still receiving this signal, forward the frame
    // to the MAC layer.
    //

    if ((phy802_11->mode == PHY_RECEIVING)
         && (phy802_11->rxMsg == propRxInfo->txMsg))
    {

#ifdef ADDON_DB
        Phy802_11UpdateEventsTable(node,
                                   phyIndex,
                                   channelIndex,
                                   propRxInfo,
                                   phy802_11->rxMsgPower_mW,
                                   phy802_11->rxMsg,
                                   "PhyReceiveSignal");
#endif
        Message* newMsg = NULL;

        BOOL inError = checkPacketError(&sinr,
                                        propRxInfo,
                                        phyIndex);

        int msgSize = -1;
        int overheadSize = -1;

        if (!phy802_11->rxMsg->isPacked)
        {
            msgSize = MESSAGE_ReturnPacketSize(phy802_11->rxMsg);
        }
        else
        {
            msgSize = MESSAGE_ReturnActualPacketSize(phy802_11->rxMsg);
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
            case PHY802_11pCCH:
            case PHY802_11pSCH:
            {
                overheadSize = PHY802_11p_CONTROL_OVERHEAD_SIZE;
                break;
            }
            default:
            {
                ERROR_ReportError("Invalid Phy Model");
            }
        }

        phy802_11->sController->updateStat("PhyReceived",
                                           msgSize,
                                           overheadSize,
                                           IN_DB(phy802_11->rxMsgPower_mW),
                                           sinr,
                                           rssi_dBm,
                                           ni_dBm,
                                           propRxInfo->pathloss_dB);

        Phy802_11UnlockSignal(phy802_11);


        if (carrierSensing(TRUE, phyIndex) == TRUE)
        {
            Phy802_11ChangeState(node,phyIndex, PHY_SENSING);
        }
        else
        {
            Phy802_11ChangeState(node,phyIndex, PHY_IDLE);
        }

        if (!inError)
        {
            PhySignalMeasurement sigMeasure;
            sigMeasure.rxBeginTime = propRxInfo->rxStartTime;
            sigMeasure.rss = s_dBm;
            sigMeasure.snr = IN_DB(NON_DB(s_dBm)/NON_DB(noise_dBm));
            sigMeasure.cinr = sinr;
            newMsg = MESSAGE_Duplicate(node, propRxInfo->txMsg);

            // Remove MIMO related information from the message
            if (newMsg->m_mimoData)
            {
                delete newMsg->m_mimoData;
                newMsg->m_mimoData = NULL;
            }
            MESSAGE_RemoveHeader(
                node, newMsg, sizeof(Phy802_11PlcpHeader), TRACE_802_11);

            MESSAGE_SetInstanceId(newMsg, (short) phyIndex);

            phy802_11->rxDOA = propRxInfo->rxDOA;

#ifdef ADDON_DB
            Phy802_11UpdateEventsTable(node,
                                       phyIndex,
                                       channelIndex,
                                       propRxInfo,
                                       pow(10.0, s_dBm/10.0),
                                       newMsg,
                                       "PhySendToUpper");
#endif
            PhySignalMeasurement* signalMeaInfo = NULL;
            MESSAGE_InfoAlloc(node,
                              newMsg,
                              sizeof(PhySignalMeasurement));
            signalMeaInfo = (PhySignalMeasurement*)
                            MESSAGE_ReturnInfo(newMsg);
            memcpy(signalMeaInfo,&sigMeasure,sizeof(PhySignalMeasurement));
            MAC_ReceivePacketFromPhy(
                node,
                node->phyData[phyIndex]->macInterfaceIndex,
                newMsg);

            // Get the path profile
            PropTxInfo* txInfo = (PropTxInfo*) MESSAGE_ReturnInfo(
                                                     propRxInfo->txMsg);

            // Compute the delay
            clocktype txDelay = propRxInfo->rxStartTime
                                - txInfo->txStartTime;

            if (phy802_11->thisPhy->phyStats)
            {
                phy802_11->thisPhy->stats->AddSignalToMacDataPoints(
                    node,
                    propRxInfo,
                    phy802_11->thisPhy,
                    channelIndex,
                    txDelay,
                    pow(10.0, ni_dBm/10.0),
                    propRxInfo->pathloss_dB,
                    pow(10.0, s_dBm/10.0));
            }

#ifdef CYBER_LIB
            if (node->phyData[phyIndex]->jammerStatistics == TRUE)
            {
                if (node->phyData[phyIndex]->jamInstances > 0)
                {
                    phy802_11->stats.totalRxSignalsToMacDuringJam++;
                }
            }
#endif
        }
        else {
            Phy802_11ReportStatusToMac(
                node,
                phyIndex,
                phy802_11->mode);


            PHY_NotificationOfPacketDrop(
                node,
                phyIndex,
                channelIndex,
                propRxInfo->txMsg,
                "Signal Received with Error",
                pow(10.0, s_dBm/10.0),
                pow(10.0, ni_dBm/10.0),
                propRxInfo->pathloss_dB);


            if (phy802_11->thisPhy->phyStats)
            {
                phy802_11->thisPhy->stats->AddSignalWithErrorsDataPoints(
                    node,
                    propRxInfo,
                    phy802_11->thisPhy,
                    channelIndex,
                    pow(10.0, ni_dBm/10.0),
                    pow(10.0, s_dBm/10.0));
            }

#ifdef CYBER_LIB
            if (node->phyData[phyIndex]->jammerStatistics == TRUE)
            {
                if (node->phyData[phyIndex]->jamInstances > 0)
                {
                    phy802_11->stats.totalSignalsWithErrorsDuringJam++;
                }
            }
#endif
        }//if//
        DEBUG_PRINT("Signal ends in receiving and it's the receiving signal,"
            "noise+interference power(dB): %lf \n", ni_dBm);
    }
    else {
        PhyStatusType newMode;
        if (phy802_11->mode != PHY_RECEIVING)
        {
           if (carrierSensing(TRUE, phyIndex) == TRUE)
           {
               newMode = PHY_SENSING;
           }
           else
           {
               newMode = PHY_IDLE;
           }//if//

           if (newMode != phy802_11->mode)
           {
                Phy802_11ChangeState(node,phyIndex, newMode);
                Phy802_11ReportStatusToMac(node,
                   phyIndex,
                   newMode);
           }//if//
        }//if//
        DEBUG_PRINT("Signal ends in other status,"
            "interference power: %lf \n", ni_dBm);
    }//if//
}


/// \brief Checks whether the phy is able to sense signal or not
///
/// Function compares receive signal strength with min rx sensitivity
/// at 20Mhz and takes decision accordingly.
///
/// \param isSigEnd   Whether signal has ended or not
/// \param phyIndex   Phy Index
///
/// \return TRUE if RSSI is more than or equal to sensitivity
///         FALSE otherwise
BOOL Phy802_11a::carrierSensing(BOOL isSigEnd, int phyIndex)
{
    PhyData802_11* phy802_11 = m_parentData;
    double rxSensitivity_dBm = IN_DB(m_parentData->rxSensitivity_mW[0]);
    spectralBand* sb = cc()->getSBand(cc()->getConfChBwdth());
    bool noiseModified = false;

    if (phy802_11->thisPhy->phyModel == PHY802_11b
        && this->useLegacy802_11b)
    {
        if (this->rxDataRateType == PHY802_11b__5_5M
            || this->rxDataRateType == PHY802_11b_11M)
        {
            // Noise adjustment for supporting Legacy Phy802.11b
            // calculations
            node()->phyData[phyIndex]->noise_mW_hz *= 11;
            noiseModified = true;
        }
    }

    clocktype now = node()->getNodeTime();

    double rssi_dBm = node()->sl().rssi(sb, now, phyIndex);

    if (DEBUG)
    {
        std::cout << "CarrierSensing rssi(dBm): " << rssi_dBm << " position: "
          << (isSigEnd ? "EOP" : "SOP") << std::endl;
    }

    // revert back noise_mw_hz
    if (noiseModified)
    {
        node()->phyData[phyIndex]->noise_mW_hz /= 11;
    }

    if (rssi_dBm >= rxSensitivity_dBm)
    {
        return TRUE;
    }

    return FALSE;
}


/// \brief checks whether to process the signal or not
///
/// \param phyIndex     Phy index
///
/// \return True if signal is to be processed, False otherwise
BOOL Phy802_11a::processSignal(int phyIndex)
{
    return carrierSensing(FALSE, phyIndex);
}


/// \brief checks whether the packet is received with error or not
///
/// \param sinrPtr    Pointer to signal to noise ratio
/// \param propRxInfo Propagation rx info
/// \param phyIndex   Phy index
///
/// \return True if packet is received with error, False otherwise
BOOL Phy802_11a::checkPacketError(double *sinrPtr,
                                  PropRxInfo* propRxInfo,
                                  int phyIndex)
{
    Node* node = m_node;
    PhyData802_11* phy802_11 = m_parentData;

    double BER = 0;
    double dataRate1 = 0;
    bool noiseModified = false;

    if (phy802_11->thisPhy->phyModel == PHY802_11b
        && this->useLegacy802_11b)
    {
        if (this->rxDataRateType == PHY802_11b__5_5M
            || this->rxDataRateType == PHY802_11b_11M)
        {
            // Noise adjustment for supporting Legacy Phy802.11b
            // calculations
            node->phyData[phyIndex]->noise_mW_hz *= 11;
            noiseModified = true;
        }
    }

    double snr_dB = node->sl().snr(propRxInfo, cc()->getSBand(CHBWDTH_20MHZ), phyIndex);
    double sinr = pow(10.0, snr_dB/10.0);

    if (DEBUG)
    {
        std::cout << "snr(dB): " << snr_dB << " sinr: " << sinr << std::endl;
    }

    *sinrPtr = sinr;

    assert(this->rxDataRateType >= 0 &&
           this->rxDataRateType < phy802_11->numDataRates);
    BER = getBer(sinr);

    dataRate1 = (double)this->dataRate[this->rxDataRateType];
    // revert back noise_mw_hz
    if (noiseModified)
    {
        node->phyData[phyIndex]->noise_mW_hz /= 11;
    }

    if (BER != 0.0)
    {
        double numBits = 0;
        numBits = ((propRxInfo->duration) * dataRate1 / (double)SECOND);

        double errorProbability = 1.0 - pow((1.0 - BER), numBits);
        double rand = RANDOM_erand(phy802_11->thisPhy->seed);

        // Fiddling with the lookahead time can cause timing imprecisions
        if (node->partitionData->looseSynchronization)
        {
            if (errorProbability < 0.0)
            {
                return false;
            }
            if (errorProbability > 1.0)
            {
                return true;
            }
        }
        else
        {
            assert((errorProbability >= 0.0) && (errorProbability <= 1.0));
        }

        if (errorProbability > rand)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/// \brief Function to initialize 802.11p default parameters
///
/// This function initializes 802.11p data structures with default
/// values
void Phy802_11a::initDefaultParams_p()
{
    Node* node = m_node;
    int phyIndex = m_parentData->thisPhy->phyIndex;
    PhyData802_11* phy802_11p =
        (PhyData802_11*)(node->phyData[phyIndex]->phyVar);

    phy802_11p->numDataRates = PHY802_11p_NUM_DATA_RATES;

    txDefaultPower_dBm[PHY802_11p__3M] =
                                    PHY802_11p_DEFAULT_TX_POWER__3M_dBm;
    txDefaultPower_dBm[PHY802_11p_4_5M] =
                                    PHY802_11p_DEFAULT_TX_POWER_4_5M_dBm;
    txDefaultPower_dBm[PHY802_11p__6M] =
                                    PHY802_11p_DEFAULT_TX_POWER__6M_dBm;
    txDefaultPower_dBm[PHY802_11p__9M] =
                                    PHY802_11p_DEFAULT_TX_POWER__9M_dBm;
    txDefaultPower_dBm[PHY802_11p_12M] =
                                    PHY802_11p_DEFAULT_TX_POWER_12M_dBm;
    txDefaultPower_dBm[PHY802_11p_18M] =
                                    PHY802_11p_DEFAULT_TX_POWER_18M_dBm;
    txDefaultPower_dBm[PHY802_11p_24M] =
                                    PHY802_11p_DEFAULT_TX_POWER_24M_dBm;
    txDefaultPower_dBm[PHY802_11p_27M] =
                                    PHY802_11p_DEFAULT_TX_POWER_27M_dBm;

    phy802_11p->rxSensitivity_mW[PHY802_11p__3M] =
                      NON_DB(PHY802_11p_DEFAULT_RX_SENSITIVITY__3M_dBm);
    phy802_11p->rxSensitivity_mW[PHY802_11p_4_5M] =
                      NON_DB(PHY802_11p_DEFAULT_RX_SENSITIVITY_4_5M_dBm);
    phy802_11p->rxSensitivity_mW[PHY802_11p__6M] =
                      NON_DB(PHY802_11p_DEFAULT_RX_SENSITIVITY__6M_dBm);
    phy802_11p->rxSensitivity_mW[PHY802_11p__9M] =
                      NON_DB(PHY802_11p_DEFAULT_RX_SENSITIVITY__9M_dBm);
    phy802_11p->rxSensitivity_mW[PHY802_11p_12M] =
                      NON_DB(PHY802_11p_DEFAULT_RX_SENSITIVITY_12M_dBm);
    phy802_11p->rxSensitivity_mW[PHY802_11p_18M] =
                      NON_DB(PHY802_11p_DEFAULT_RX_SENSITIVITY_18M_dBm);
    phy802_11p->rxSensitivity_mW[PHY802_11p_24M] =
                      NON_DB(PHY802_11p_DEFAULT_RX_SENSITIVITY_24M_dBm);
    phy802_11p->rxSensitivity_mW[PHY802_11p_27M] =
                      NON_DB(PHY802_11p_DEFAULT_RX_SENSITIVITY_27M_dBm);

    dataRate[PHY802_11p__3M] = PHY802_11p_DATA_RATE__3M;
    dataRate[PHY802_11p_4_5M] = PHY802_11p_DATA_RATE_4_5M;
    dataRate[PHY802_11p__6M] = PHY802_11p_DATA_RATE__6M;
    dataRate[PHY802_11p__9M] = PHY802_11p_DATA_RATE__9M;
    dataRate[PHY802_11p_12M] = PHY802_11p_DATA_RATE_12M;
    dataRate[PHY802_11p_18M] = PHY802_11p_DATA_RATE_18M;
    dataRate[PHY802_11p_24M] = PHY802_11p_DATA_RATE_24M;
    dataRate[PHY802_11p_27M] = PHY802_11p_DATA_RATE_27M;

    numDataBitsPerSymbol[PHY802_11p__3M] =
                                PHY802_11p_NUM_DATA_BITS_PER_SYMBOL__3M;
    numDataBitsPerSymbol[PHY802_11p_4_5M] =
                                PHY802_11p_NUM_DATA_BITS_PER_SYMBOL_4_5M;
    numDataBitsPerSymbol[PHY802_11p__6M] =
                                PHY802_11p_NUM_DATA_BITS_PER_SYMBOL__6M;
    numDataBitsPerSymbol[PHY802_11p__9M] =
                                PHY802_11p_NUM_DATA_BITS_PER_SYMBOL__9M;
    numDataBitsPerSymbol[PHY802_11p_12M] =
                                PHY802_11p_NUM_DATA_BITS_PER_SYMBOL_12M;
    numDataBitsPerSymbol[PHY802_11p_18M] =
                                PHY802_11p_NUM_DATA_BITS_PER_SYMBOL_18M;
    numDataBitsPerSymbol[PHY802_11p_24M] =
                                PHY802_11p_NUM_DATA_BITS_PER_SYMBOL_24M;
    numDataBitsPerSymbol[PHY802_11p_27M] =
                                PHY802_11p_NUM_DATA_BITS_PER_SYMBOL_27M;

    phy802_11p->lowestDataRateType = PHY802_11p_LOWEST_DATA_RATE_TYPE;
    phy802_11p->highestDataRateType = PHY802_11p_HIGHEST_DATA_RATE_TYPE;
    txDataRateTypeForBC = PHY802_11p_DATA_RATE_TYPE_FOR_BC;

    phy802_11p->channelBandwidth = PHY802_11p_CHANNEL_BANDWIDTH;
    phy802_11p->rxTxTurnaroundTime = PHY802_11p_RX_TX_TURNAROUND_TIME;
}

/// \brief Function to set user configurable parameters of 802.11p
///
/// \param nodeInput pointer to the node Input
void Phy802_11a::readCfgParams_p(const NodeInput *nodeInput)
{
    double rxSensitivity_dBm;
    double txPower_dBm;
    BOOL   wasFound;
    Node* node = m_node;
    int phyIndex = m_parentData->thisPhy->phyIndex;
    char buf[1024];

    PhyData802_11* phy802_11p =
        (PhyData802_11*)(node->phyData[phyIndex]->phyVar);

    const char* k_types[] = { "3", "4.5", "6", "9", "12", "18", "24", "27"};

    //
    // Set PHY802_11p TX POWER's
    //
    for (int k1(0); k1 < PHY802_11p_NUM_DATA_RATES; k1++)
    {
        const char* val = k_types[k1];
        (void)sprintf(buf, "PHY802.11-TX-POWER-%sMBPS", val);

        IO_ReadDouble(
            node->nodeId,
            node->phyData[phyIndex]->networkAddress,
            nodeInput,
            buf,
            &wasFound,
            &txPower_dBm);

        if (wasFound) {
            txDefaultPower_dBm[k1] = (float)txPower_dBm;
        }
    }

    for (int k1(0); k1 < PHY802_11p_NUM_DATA_RATES; k1++)
    {
        const char* val = k_types[k1];
        (void)sprintf(buf, "PHY802.11-RX-SENSITIVITY-%sMBPS", val);
        
        IO_ReadDouble(
            node->nodeId,
            node->phyData[phyIndex]->networkAddress,
            nodeInput,
            buf,
            &wasFound,
            &rxSensitivity_dBm);

        if (wasFound) {
            phy802_11p->rxSensitivity_mW[k1] =
                                       NON_DB(rxSensitivity_dBm);
        }
    }
}
