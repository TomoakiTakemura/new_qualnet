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
#include "phy_802_11n.h"
#include "partition.h"
#include "propagation.h"
#include "mac_phy_802_11n.h"

#include "mimo-mapper.h"

#define DEBUG 0
#define DEBUG_CHANMATRIX  0

using namespace Dot11;
using namespace Qos;

#define MCS_STRUCT MCS_Params[MAC_PHY_TxRxVector.chBwdth][MAC_PHY_TxRxVector.mcs]

const Phy802_11n::MCSParam
  Phy802_11n::MCS_Params[NUM_CH_BW_802_11N][Phy802_11n::Max_EQM_MCS] =
    { { { 1, CODERATE_1_2, BPSK, 26, 1, { TYPES_ToUInt64(6500000), TYPES_ToUInt64(7200000) } },
        { 1, CODERATE_1_2, QPSK, 52, 1, { TYPES_ToUInt64(13000000), TYPES_ToUInt64(14400000) } },
        { 1, CODERATE_3_4, QPSK, 78, 1, { TYPES_ToUInt64(19500000), TYPES_ToUInt64(21700000) } },
        { 1, CODERATE_1_2, QAM16, 104, 1, { TYPES_ToUInt64(26000000), TYPES_ToUInt64(28900000) } },
        { 1, CODERATE_3_4, QAM16, 156, 1, { TYPES_ToUInt64(39000000), TYPES_ToUInt64(43300000) } },
        { 1, CODERATE_2_3, QAM64, 208, 1, { TYPES_ToUInt64(52000000), TYPES_ToUInt64(57800000) } },
        { 1, CODERATE_3_4, QAM64, 234, 1, { TYPES_ToUInt64(58500000), TYPES_ToUInt64(65000000) } },
        { 1, CODERATE_5_6, QAM64, 260, 1, { TYPES_ToUInt64(65000000), TYPES_ToUInt64(72200000) } },
        

        { 2, CODERATE_1_2, BPSK, 52, 1, { TYPES_ToUInt64(13000000), TYPES_ToUInt64(14400000) } },
        { 2, CODERATE_1_2, QPSK, 104, 1, { TYPES_ToUInt64(26000000), TYPES_ToUInt64(28900000) } },
        { 2, CODERATE_3_4, QPSK, 156, 1, { TYPES_ToUInt64(39000000), TYPES_ToUInt64(43300000) } },
        { 2, CODERATE_1_2, QAM16, 208, 1, { TYPES_ToUInt64(52000000), TYPES_ToUInt64(57800000) } },
        { 2, CODERATE_3_4, QAM16, 312, 1, { TYPES_ToUInt64(78000000), TYPES_ToUInt64(86700000) } },
        { 2, CODERATE_2_3, QAM64, 416, 1, { TYPES_ToUInt64(104000000), TYPES_ToUInt64(115600000) } },
        { 2, CODERATE_3_4, QAM64, 468, 1, { TYPES_ToUInt64(117000000), TYPES_ToUInt64(130000000) } },
        { 2, CODERATE_5_6, QAM64, 520, 1, { TYPES_ToUInt64(130000000), TYPES_ToUInt64(144400000) } },
        

        { 3, CODERATE_1_2, BPSK, 78, 1, { TYPES_ToUInt64(19500000), TYPES_ToUInt64(21700000) } },
        { 3, CODERATE_1_2, QPSK, 156, 1, { TYPES_ToUInt64(39000000), TYPES_ToUInt64(43300000) } },
        { 3, CODERATE_3_4, QPSK, 234, 1, { TYPES_ToUInt64(58500000), TYPES_ToUInt64(65000000) } },
        { 3, CODERATE_1_2, QAM16, 312, 1, { TYPES_ToUInt64(78000000), TYPES_ToUInt64(86700000) } },
        { 3, CODERATE_3_4, QAM16, 468, 1, { TYPES_ToUInt64(117000000), TYPES_ToUInt64(130000000) } },
        { 3, CODERATE_2_3, QAM64, 624, 1, { TYPES_ToUInt64(156000000), TYPES_ToUInt64(173300000) } },
        { 3, CODERATE_3_4, QAM64, 702, 1, { TYPES_ToUInt64(175500000), TYPES_ToUInt64(195000000) } },
        { 3, CODERATE_5_6, QAM64, 780, 1, { TYPES_ToUInt64(195000000), TYPES_ToUInt64(216700000) } },
        

        { 4, CODERATE_1_2, BPSK, 104, 1, { TYPES_ToUInt64(26000000), TYPES_ToUInt64(28900000) } },
        { 4, CODERATE_1_2, QPSK, 208, 1, { TYPES_ToUInt64(52000000), TYPES_ToUInt64(57800000) } },
        { 4, CODERATE_3_4, QPSK, 312, 1, { TYPES_ToUInt64(78000000), TYPES_ToUInt64(86700000) } },
        { 4, CODERATE_1_2, QAM16, 416, 1, { TYPES_ToUInt64(104000000), TYPES_ToUInt64(115600000) } },
        { 4, CODERATE_3_4, QAM16, 624, 1, { TYPES_ToUInt64(156000000), TYPES_ToUInt64(173300000) } },
        { 4, CODERATE_2_3, QAM64, 832, 1, { TYPES_ToUInt64(208000000), TYPES_ToUInt64(231100000) } },
        { 4, CODERATE_3_4, QAM64, 936, 1, { TYPES_ToUInt64(234000000), TYPES_ToUInt64(260000000) } },
        { 4, CODERATE_5_6, QAM64, 1040, 1, { TYPES_ToUInt64(260000000), TYPES_ToUInt64(288900000) } } },
     
      { { 1, CODERATE_1_2, BPSK, 54, 1, { TYPES_ToUInt64(13500000), TYPES_ToUInt64(15000000) } },
        { 1, CODERATE_1_2, QPSK, 108, 1, { TYPES_ToUInt64(27000000), TYPES_ToUInt64(30000000) } },
        { 1, CODERATE_3_4, QPSK, 162, 1, { TYPES_ToUInt64(40500000), TYPES_ToUInt64(45000000) } },
        { 1, CODERATE_1_2, QAM16, 216, 1, { TYPES_ToUInt64(54000000), TYPES_ToUInt64(60000000) } },
        { 1, CODERATE_3_4, QAM16, 324, 1, { TYPES_ToUInt64(81000000), TYPES_ToUInt64(90000000) } },
        { 1, CODERATE_2_3, QAM64, 432, 1, { TYPES_ToUInt64(108000000), TYPES_ToUInt64(120000000) } },
        { 1, CODERATE_3_4, QAM64, 486, 1, { TYPES_ToUInt64(121500000), TYPES_ToUInt64(135000000) } },
        { 1, CODERATE_5_6, QAM64, 540, 1, { TYPES_ToUInt64(135000000), TYPES_ToUInt64(150000000) } },

        { 2, CODERATE_1_2, BPSK, 108, 1, { TYPES_ToUInt64(27000000), TYPES_ToUInt64(30000000) } },
        { 2, CODERATE_1_2, QPSK, 216, 1, { TYPES_ToUInt64(54000000), TYPES_ToUInt64(60000000) } },
        { 2, CODERATE_3_4, QPSK, 324, 1, { TYPES_ToUInt64(81000000), TYPES_ToUInt64(90000000) } },
        { 2, CODERATE_1_2, QAM16, 432, 1, { TYPES_ToUInt64(108000000), TYPES_ToUInt64(120000000) } },
        { 2, CODERATE_3_4, QAM16, 648, 1, { TYPES_ToUInt64(162000000), TYPES_ToUInt64(180000000) } },
        { 2, CODERATE_2_3, QAM64, 864, 1, { TYPES_ToUInt64(216000000), TYPES_ToUInt64(240000000) } },
        { 2, CODERATE_3_4, QAM64, 972, 1, { TYPES_ToUInt64(243000000), TYPES_ToUInt64(270000000) } },
        { 2, CODERATE_5_6, QAM64, 1080, 1, { TYPES_ToUInt64(270000000), TYPES_ToUInt64(300000000) } },

        { 3, CODERATE_1_2, BPSK, 162, 1, { TYPES_ToUInt64(40500000), TYPES_ToUInt64(45000000) } },
        { 3, CODERATE_1_2, QPSK, 324, 1, { TYPES_ToUInt64(81000000), TYPES_ToUInt64(90000000) } },
        { 3, CODERATE_3_4, QPSK, 486, 1, { TYPES_ToUInt64(121500000), TYPES_ToUInt64(135000000) } },
        { 3, CODERATE_1_2, QAM16, 648, 1, { TYPES_ToUInt64(162000000), TYPES_ToUInt64(180000000) } },
        { 3, CODERATE_3_4, QAM16, 972, 1, { TYPES_ToUInt64(243000000), TYPES_ToUInt64(270000000) } },
        { 3, CODERATE_2_3, QAM64, 1296, 1, { TYPES_ToUInt64(324000000), TYPES_ToUInt64(360000000) } },
        { 3, CODERATE_3_4, QAM64, 1458, 1, { TYPES_ToUInt64(364500000), TYPES_ToUInt64(405000000) } },
        { 3, CODERATE_5_6, QAM64, 1620, 1, { TYPES_ToUInt64(405000000), TYPES_ToUInt64(450000000) } },

        { 4, CODERATE_1_2, BPSK, 216, 1, { TYPES_ToUInt64(54000000), TYPES_ToUInt64(60000000) } },
        { 4, CODERATE_1_2, QPSK, 432, 1, { TYPES_ToUInt64(108000000), TYPES_ToUInt64(120000000) } },
        { 4, CODERATE_3_4, QPSK, 648, 1, { TYPES_ToUInt64(162000000), TYPES_ToUInt64(180000000) } },
        { 4, CODERATE_1_2, QAM16, 864, 1, { TYPES_ToUInt64(216000000), TYPES_ToUInt64(240000000) } },
        { 4, CODERATE_3_4, QAM16, 1296, 1, { TYPES_ToUInt64(324000000), TYPES_ToUInt64(360000000) } },
        { 4, CODERATE_2_3, QAM64, 1728, 1, { TYPES_ToUInt64(432000000), TYPES_ToUInt64(480000000) } },
        { 4, CODERATE_3_4, QAM64, 1944, 1, { TYPES_ToUInt64(486000000), TYPES_ToUInt64(540000000) } },
        { 4, CODERATE_5_6, QAM64, 2160, 1, { TYPES_ToUInt64(540000000), TYPES_ToUInt64(600000000) } } } };

const double Phy802_11n::m_minSensitivity_dBm[NUM_CH_BW_802_11N]
                                          [Phy802_11n::MCS_NUMS_SINGLE_SS] =
    { { -82.0, -79.0, -77.0, -74.0, -70.0, -66.0, -65.0, -64.0 },
      { -79.0, -76.0, -74.0, -71.0, -67.0, -63.0, -62.0, -61.0 } };




/// \brief Function to get frame duration
///
/// This function calculates frame duration based on transmission
/// vector.
///
/// \param txParam     Reference to txvector
///
/// \return Frame duration
clocktype Phy802_11n::getFrameDuration(const MAC_PHY_TxRxVector& txParam)
{
    // Reference: IEEE 802.11 2009 20.4.3
    size_t dataBits;
    size_t numSymbols;
    clocktype frameDur = 0;
    clocktype symbolDur = (txParam.gi == GI_LONG ? (clocktype)T_Sym : (clocktype)T_Syms);

    // Number of BCC coder
    unsigned int numEs = 1;
    
    // Number of spatial streams
    unsigned int numSts;
    unsigned int mStbc = 1;

    // section 20.3.11.3
    if (MCS_Params[txParam.chBwdth - 1][txParam.mcs].m_dataRate[txParam.gi]
        > 300000000u)
    {
        numEs = 2;
    }

    ChBandwidth chBwdth = CHBWDTH_20MHZ;
    if (txParam.nonHTMod != NON_HT_DUP_OFDM)
    {
        chBwdth = txParam.chBwdth;
    }

    switch (txParam.format) {
        case MODE_HT_GF:
        {
            assert(txParam.phyType == k_Dot11n);
            numSts = MCS_Params[chBwdth - 1][txParam.mcs].m_nSpatialStream;
            dataBits = txParam.length*8
                       + Ppdu_Service_Bits_Size
                       + Ppdu_Tail_Bits_Size*numEs;
            numSymbols = mStbc * (size_t)ceil((double)dataBits/
                   (MCS_Params[chBwdth - 1][txParam.mcs]
                                              .m_nDataBitsPerSymbol*mStbc));

            frameDur = preambDur_HtGf(numSts, txParam.numEss)
                            + (numSymbols * symbolDur);
            break;
        }
        case MODE_HT_MF:
        {
            numSts = MCS_Params[chBwdth - 1][txParam.mcs].m_nSpatialStream;
            dataBits = txParam.length*8
                       + Ppdu_Service_Bits_Size
                       + Ppdu_Tail_Bits_Size*numEs;
            numSymbols = mStbc * (size_t)ceil((double)dataBits/
                   (MCS_Params[chBwdth - 1][txParam.mcs]
                                              .m_nDataBitsPerSymbol*mStbc));
            if (txParam.gi == GI_LONG)
            {
                frameDur = preambDur_HtMixed(numSts, txParam.numEss)
                             + numSymbols * T_Sym;
            }
            else
            {
                frameDur = preambDur_HtMixed(numSts, txParam.numEss)
                             + T_Sym * ((size_t)ceil(
                                        (double)numSymbols * T_Syms/T_Sym));
            }
            break;
        }
        case MODE_NON_HT:
        {
            if (txParam.phyType == k_Dot11a)
            {
                dataBits = txParam.length*8
                       + Ppdu_Service_Bits_Size
                       + Ppdu_Tail_Bits_Size;

                numSymbols = (size_t)ceil((double)dataBits/
                    this->phy802_11aNdbps[txParam.mcs]);

                frameDur = PHY802_11a_SYNCHRONIZATION_TIME
                           + (numSymbols * PHY802_11a_OFDM_SYMBOL_DURATION)
                              * MICRO_SECOND;
            }
            else if (txParam.phyType == k_Dot11b)
            {
                dataBits = txParam.length*8;
                numSymbols = (size_t)ceil((double)dataBits/
                    this->phy802_11bNdbps[txParam.mcs]);
                frameDur = PHY802_11b_SYNCHRONIZATION_TIME + numSymbols
                                                    * MICRO_SECOND;
            }
            else
            {
                dataBits = txParam.length*8
                       + Ppdu_Service_Bits_Size
                       + Ppdu_Tail_Bits_Size;
                numSymbols = (size_t)ceil((double)dataBits/
                   MCS_Params[chBwdth - 1][txParam.mcs].m_nDataBitsPerSymbol);
                frameDur = preambDur_NonHt() + (numSymbols * symbolDur);
            }
            break;
        }
        default:
            ERROR_ReportError("Invalid Tx Parameter Format");
            break;
    }

    {
        char durStr[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(frameDur, durStr);
        DEBUG_PRINT("Phy802_11n::GetFrameDuration: frame length(bytes): %"
            TYPES_SIZEOFMFT "d, "
            "frame MCS: %u, frame duration %s \n",
            txParam.length, txParam.mcs, durStr);
    }
    return frameDur;
}


/// \brief Function to get Data Rate from txvector
///
/// \param txRxVector  Reference to txvector
///
/// \return Data Rate
double Phy802_11n::getDataRate(const MAC_PHY_TxRxVector& txRxVector)
{
    return (double)(MCS_Params[txRxVector.chBwdth - 1][txRxVector.mcs])
                                                 .m_dataRate[txRxVector.gi];
}

/// \brief Function to number of spatial stream
///
/// \param txRxVector  Reference to txvector
///
/// \return Number of spatial streams
size_t Phy802_11n::getNumSs(const MAC_PHY_TxRxVector& txRxVector)
{
    return  MCS_Params[txRxVector.chBwdth - 1][txRxVector.mcs].m_nSpatialStream;
}


///\brief Function to read Phy802.11n sensitivity parameters
///
/// This function reads 802_11n sensitivity parameters
/// from configuration file.
///
/// \param nodeInput Pointer to node input
void Phy802_11n::readSensitivityParams(const NodeInput* nodeInput)
{
    Node* node = m_node;
    int phyIndex = m_parentData->thisPhy->phyIndex;

    const char* k_types[] = { "20MHz", "40MHz"};
    const ChBandwidth k_idxs[] = { CHBWDTH_20MHZ, CHBWDTH_40MHZ};
    BOOL wasFound(FALSE);

    for (int k1(0); k1 < 2; k1++)
    {
      const char* bw_str = k_types[k1];
      const ChBandwidth bw_idx = k_idxs[k1];

      for (unsigned int k2 = 0; k2 < MCS_NUMS_SINGLE_SS; k2++)
      {
        char buf[1024];
        (void)sprintf(buf, "PHY802.11-RX-SENSITIVITY-%s-MCS%d", bw_str, k2);
        double rxSensitivity_dBm = 0;

        IO_ReadDouble(node, node->nodeId,
                      node->phyData[phyIndex]->macInterfaceIndex,
                      nodeInput,
                      buf, 
                      &wasFound,
                      &rxSensitivity_dBm);

        if (wasFound) 
        {
          m_RxSensitivity_dBm[bw_idx - 1][k2] = rxSensitivity_dBm;
        }
      }
    }
}

/// \brief Function to read parameters for backward compatibility
///
/// This function reads Phy802.11n configuration parameters for backward
/// compatibility.
///
/// \param nodeInput Pointer to node input
void Phy802_11n::readParamsForBackwardCompatibility(const NodeInput* nodeInput)
{
    BOOL    wasFound;
    char    errStr[MAX_STRING_LENGTH];
    double  txPower_dBm = 0;
    Node* node = m_node;
    int phyIndex = m_parentData->thisPhy->phyIndex;

    //
    // Number of attenna elements
    //
    IO_ReadInt(
        node,
        node->nodeId,
        node->phyData[phyIndex]->macInterfaceIndex,
        nodeInput,
        "PHY802.11n-NUM-ANTENNA-ELEMENTS",
        &wasFound,
        &m_NumAtnaElmts);

    if (m_NumAtnaElmts <= 0 || m_NumAtnaElmts > 4)
    {
        sprintf(errStr, "The range of PHY802.11n-NUM-ANTENNA-ELEMENTS"
            "is between 1 and 4!");
        ERROR_ReportError(errStr);
    }
    m_NumActiveAtnaElmts = m_NumAtnaElmts;

    //
    // inter-attenna-element space
    //
    IO_ReadDouble(
        node,
        node->nodeId,
        node->phyData[phyIndex]->macInterfaceIndex,
        nodeInput,
        "PHY802.11n-ANTENNA-ELEMENT-SPACE",
        &wasFound,
        &m_AtnaSpace);

    if (m_AtnaSpace <= 0)
    {
        sprintf(errStr, "The range of PHY802.11n-ANTENNA-ELEMENT-SPACE"
            "is larger than 0");
        ERROR_ReportError(errStr);
    }

    //
    // Whether short GI is supported
    //
    IO_ReadBool(
        node,
        node->nodeId,
        node->phyData[phyIndex]->macInterfaceIndex,
        nodeInput,
        "PHY802.11n-SHORT-GI-CAPABLE",
        &wasFound,
        &m_ShortGiCapable);

    //
    // Whether STBC is supported
    //
    IO_ReadBool(
        node,
        node->nodeId,
        node->phyData[phyIndex]->macInterfaceIndex,
        nodeInput,
        "PHY802.11n-STBC-CAPABLE",
        &wasFound,
        &m_StbcCapable);

    //
    // Set PHY802_11n-TX-POWER
    //
    IO_ReadDouble(
        node,
        node->nodeId,
        node->phyData[phyIndex]->macInterfaceIndex,
        nodeInput,
        "PHY802.11n-TX-POWER",
        &wasFound,
        &txPower_dBm);

    if (wasFound) {
        m_parentData->txPower_dBm = (float)txPower_dBm;
    }

    const char* k_types[] = { "20MHz", "40MHz"};
    const ChBandwidth k_idxs[] = { CHBWDTH_20MHZ, CHBWDTH_40MHZ};

    for (int k1(0); k1 < 2; k1++)
    {
        const char* bw_str = k_types[k1];
        const ChBandwidth bw_idx = k_idxs[k1];

        for (unsigned int k2 = 0; k2 < MCS_NUMS_SINGLE_SS; k2++)
        {
            char buf[1024];
            (void)sprintf(buf,
                          "PHY802.11n-RX-SENSITIVITY-%s-MCS%d",
                          bw_str,
                          k2);

            BOOL wasFound(FALSE);
            double rxSensitivity_dBm = 0;

            IO_ReadDouble(node, node->nodeId,
                      node->phyData[phyIndex]->macInterfaceIndex,
                      nodeInput,
                      buf, 
                      &wasFound,
                      &rxSensitivity_dBm);

            if (wasFound) 
            {
                m_RxSensitivity_dBm[bw_idx - 1][k2] = rxSensitivity_dBm;
            }
        }
    }
}

/// \brief Initializes Phy802_11n
///
/// This function initializes 802_11 n by calling initialization
/// function for channel controller and reading configuration parameters
///
/// \param nodeInput Pointer to node input
void Phy802_11n::init(const NodeInput* nodeInput)
{
    readParamsForBackwardCompatibility(nodeInput);
    readCfgParams(nodeInput);
    readSensitivityParams(nodeInput);
}


void Phy802_11n::readBwdthParams(const NodeInput* nodeInput)
{
    m_chController->readCfgParamsFor20MHz(nodeInput);
    m_chController->readCfgParamsFor40MHz(nodeInput);
    spectralBand* sp = m_chController->getSBand(
                m_chController->getConfChBwdth());
    m_node->setRadioBand(
                m_chController->phyManager()->phy()->thisPhy->phyIndex, sp);
}

/// \brief Finalize function for Phy802_11n
///
/// For future use. Finalization and statistics printing is done in
/// Phy802_11Finalize() function.
///
/// \param node     Pointer to Node
/// \param phyIndex Phy Index
void Phy802_11n::finalize(Node* node, int phyIndex)
{
}

/// \brief Handles signal arrival event
///
/// This function decides to lock the signal or not based on spectral band,
/// phy mode, rssi etc.Phy carrier sensing is also done.
///
/// \param node_ptr     Pointer to node
/// \param phyIndex     Phy index
/// \param channelIndex Channel Index
/// \param propRxInfo   Propagation Rx Info
void Phy802_11n::signalArrival(Node* node_ptr,
                       int phyIndex,
                       int channelIndex,
                       PropRxInfo *propRxInfo)
{
    PhyData802_11* phy802_11 = m_parentData;
    Message* msg = propRxInfo->txMsg;

    Node& node = *node_ptr;
    const spectralBand* band = node.getRadioBand(phyIndex);
    double ni_dBm = node.sl().ni(propRxInfo, band, phyIndex);
    double s_dBm = node.sl().s(propRxInfo, phyIndex);

    switch (phy802_11->mode) {
        case PHY_RECEIVING:
        {
            PHY_NotificationOfPacketDrop(
                node_ptr,
                phyIndex,
                channelIndex,
                propRxInfo->txMsg,
                "PHY Busy in Receiving",
                pow(10.0, s_dBm/10.0),
                pow(10.0, ni_dBm/10.0),
                propRxInfo->pathloss_dB);
            break;
        }

        case PHY_IDLE:
        case PHY_SENSING:
        {
            Phy802_11PlcpHeader* plcp
              = (Phy802_11PlcpHeader*)MESSAGE_ReturnPacket(propRxInfo->txMsg);
            BOOL isDupMsg
                = plcp->nonHTMod == NON_HT_DUP_OFDM? TRUE:FALSE;
            spectralBand* msb = MESSAGE_GetSpectralBand(msg);
            ChBandwidth mchBwdth = getChBwdth(msb->getBandwidth());
            BOOL process = FALSE;
            if (isDupMsg)
            {
                spectralBand* cf= cc()->getSBand(CHBWDTH_20MHZ);
                if (cf->overlaps(*msb)
                    && plcp->format != MODE_VHT
                    && propRxInfo->txMsg->m_mimoData->m_elementCount
                        <= getNumActiveAtnaElems())
                {
                    process = TRUE;
                }
            }
            else
            {
                if (mchBwdth <= getOperationChBwdth())
                {
                    spectralBand* tsb = cc()->getSBand(mchBwdth);
                    if (tsb
                        && tsb->getFrequency() == msb->getFrequency()
                        && plcp->format != MODE_VHT
                        && propRxInfo->txMsg->m_mimoData->m_elementCount
                            <= getNumActiveAtnaElems())
                    {
                        // 11n PHY.
                        // Process only those signals which are received on the
                        // band this PHY is tuned to.
                        // An 11n device doesn't support VHT format defined in
                        // 11ac RFC
                        process = TRUE;
                    }
                }
            }
            if (!process)
            {
                // can not receive message due to frequency mismatch
                PHY_NotificationOfPacketDrop(node_ptr,
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
                    Phy802_11ChangeState(node_ptr, phyIndex, newMode);
                    Phy802_11ReportStatusToMac(node_ptr, phyIndex, newMode);
                }

                return;
            }

            double rxSensitivity_dBm(0.0);

            if (!isDupMsg)
            {
                rxSensitivity_dBm = getMinSensitivity_dBm(mchBwdth);
            }
            else
            {
                rxSensitivity_dBm = getMinSensitivity_dBm(CHBWDTH_20MHZ);
            }

            if (s_dBm >= rxSensitivity_dBm)
            {
                PropTxInfo *propTxInfo
                    = (PropTxInfo *)MESSAGE_ReturnInfo(propRxInfo->txMsg);

                clocktype txDuration = propTxInfo->duration;

                Phy802_11LockSignal(
                    node_ptr,
                    phy802_11,
                    propRxInfo,
                    propRxInfo->txMsg,
                    pow(10.0, s_dBm/10.0), // should be RSSI
                    (propRxInfo->rxStartTime + propRxInfo->duration),
                    channelIndex,
                    propRxInfo->txDOA,
                    propRxInfo->rxDOA);

#ifdef CYBER_LIB
                if (node.phyData[phyIndex]->jammerStatistics == TRUE)
                {
                    if (node.phyData[phyIndex]->jamInstances > 0)
                    {
                        phy802_11->stats.totalSignalsLockedDuringJam++;
                    }
                }
#endif

                Phy802_11ChangeState(node_ptr, phyIndex, PHY_RECEIVING);
                Phy802_11ReportExtendedStatusToMac(node_ptr,
                    phyIndex,
                    PHY_RECEIVING,
                    txDuration,
                    propRxInfo->txMsg);
            }
            else {
                // Otherwise, check if the signal changes the phy status
                PHY_NotificationOfPacketDrop(node_ptr,
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
                    Phy802_11ChangeState(node_ptr, phyIndex, newMode);

                    Phy802_11ReportStatusToMac(node_ptr,
                        phyIndex,
                        newMode);
                }

            }
            break;
        }
        default:
            ERROR_ReportError("Invalid PHY Mode");
            break;
    }
}

/// \brief Handles the signal end event
///
/// This function handles signal end event, function decides
/// whether to receive message and pass it to MAC or not.
/// Set phy state, update statistics.
///
/// \param node     Pointer to node
/// \param phyIndex Phy Index
/// \channelIndex   Channel Index
/// \propRxInfo     Pointer to propagation Rx info
void Phy802_11n::signalEnd(Node* node,
                   int phyIndex,
                   int channelIndex,
                   PropRxInfo *propRxInfo)
{
    PhyData802_11* phy802_11 = m_parentData;

    ERROR_Assert(phy802_11->mode != PHY_TRANSMITTING,
                 "Phy should not be in transmitting model at signal end");

    spectralBand* msgBand = MESSAGE_GetSpectralBand(propRxInfo->txMsg);

    double s_dBm = node->sl().s(propRxInfo, phyIndex);
    double ni_dBm = node->sl().ni(propRxInfo, msgBand, phyIndex);
    double rssi_dBm = node->sl().rssi(msgBand, node->getNodeTime(), phyIndex);
    double noise_dBm = node->sl().n(phyIndex, msgBand);

    double sinr = -1.0;

    if ((phy802_11->mode == PHY_RECEIVING)
        && (phy802_11->rxMsg == propRxInfo->txMsg))
    {
#ifdef ADDON_DB
        Phy802_11UpdateEventsTable(node,
                                   phyIndex,
                                   channelIndex,
                                   propRxInfo,
                                   pow(10.0, s_dBm/10.0),
                                   phy802_11->rxMsg,
                                   "PhyReceiveSignal");
#endif

        node->setMIMO_Data(phyIndex, propRxInfo->txMsg->m_mimoData->m_elementCount,
            getAtnaElemSpace(), msgBand);

        Message* newMsg = NULL;
        BOOL inError = checkPacketError(&sinr,
                                        propRxInfo,
                                        phyIndex);

        int msgSize = 0;
        int overheadSize = 0;

        if (!phy802_11->rxMsg->isPacked)
        {
            msgSize = MESSAGE_ReturnPacketSize(phy802_11->rxMsg);
        }
        else
        {
            msgSize = MESSAGE_ReturnActualPacketSize(phy802_11->rxMsg);
        }

        MAC_PHY_TxRxVector rxVector;
        this->getRxVectorOfLockedSignal(rxVector);
        int numSts
            = MCS_Params[rxVector.chBwdth - 1][rxVector.mcs].m_nSpatialStream;
        clocktype dataRate =
           Phy802_11n::MCS_Params[CHBWDTH_20MHZ - 1][0].m_dataRate[0];
        clocktype preamble = preambDur_HtGf(numSts, rxVector.numEss);

        overheadSize = (int)((preamble * dataRate) / SECOND
                       + Ppdu_Service_Bits_Size
                       + Ppdu_Tail_Bits_Size);
        overheadSize /= 8;

        phy802_11->sController->updateStat(
            "PhyReceived",
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
            //Perform Signal measurement
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
            MAC_ReceivePacketFromPhy(node,
                node->phyData[phyIndex]->macInterfaceIndex,
                newMsg);

            PropTxInfo* txInfo = (PropTxInfo*) MESSAGE_ReturnInfo(
                                                 propRxInfo->txMsg);
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
        else
        {
            Phy802_11ReportStatusToMac(node, phyIndex, phy802_11->mode);

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
           }

           if (newMode != phy802_11->mode)
           {
                Phy802_11ChangeState(node,phyIndex, newMode);
                Phy802_11ReportStatusToMac(
                   node,
                   phyIndex,
                   newMode);
           }
        }

        DEBUG_PRINT("Signal ends in other status,"
            "interference power(dBm): %lf \n", ni_dBm);
    }
}

/// \brief Checks whether the phy is able to sense signal or not
///
/// Function compares receive signal strength with min rx sensitivity
/// at 20Mhz and takes decision accordingly.
///
/// \param isSigEnd Whether this is signal end or not
/// \param phyIndex Phy Index
///
/// \return TRUE if RSSI is more than or equal to sensitivity
///         FALSE otherwise
BOOL Phy802_11n::carrierSensing(BOOL isSigEnd,
                                int phyIndex)
{
    double rxSensitivity_dBm = getMinSensitivity_dBm(CHBWDTH_20MHZ);
    spectralBand* sb = cc()->getSBand(CHBWDTH_20MHZ);

    clocktype now = node()->getNodeTime();

    double rssi_dBm = node()->sl().rssi(sb, now, phyIndex);

    if (DEBUG)
    {
        std::cout << "CarrierSensing rssi(dBm): " << rssi_dBm << " position: "
          << (isSigEnd ? "EOP" : "SOP") << std::endl;
    }

    if (rssi_dBm >= rxSensitivity_dBm)
    {
        return TRUE;
    }

    return FALSE;
}

/// \brief checks whether the packet is received with error or not
///
/// \param sinrPtr Pointer to signal to noise ration
/// \param propRxInfo Propagation rx info
/// \param phyIndex Phy index
///
/// \return True if packet is received with error, False other wise
BOOL Phy802_11n::checkPacketError(double *sinrPtr,
                                  PropRxInfo* propRxInfo,
                                  int phyIndex)
{
    Phy802_11PlcpHeader* plcp
        = (Phy802_11PlcpHeader*)MESSAGE_ReturnPacket(propRxInfo->txMsg);

    Message* msg = propRxInfo->txMsg;
    BOOL isDupMsg = plcp->nonHTMod == NON_HT_DUP_OFDM? TRUE:FALSE;

    spectralBand* msb = MESSAGE_GetSpectralBand(msg); 
    ChBandwidth mchBwdth = getChBwdth(msb->getBandwidth());

    MIMO_Matrix_t lambda_k = MIMO_Matrix_t::Zero(
        propRxInfo->txMsg->m_mimoData->m_elementCount,
        propRxInfo->txMsg->m_mimoData->m_elementCount);
    m_node->MIMO_getEigenValues(propRxInfo, phyIndex,
            propRxInfo->txDOA, propRxInfo->rxDOA, lambda_k);

    if (isDupMsg)
    {
        *sinrPtr = -1;
        spectralBand* sb = cc()->getSBand(CHBWDTH_20MHZ);

        if (sb->overlaps(*msb))
        {
            double branch_snr_dB = node()->sl().snr(propRxInfo, sb, phyIndex);
            for (int k(0); k < lambda_k.size(); k++)
            {
                double lr = std::real(lambda_k(k));
                if (DEBUG)
                {
                    std::cout << "lr(" << k << "): " << lr << std::endl;
                }
            }

            int ntx = propRxInfo->txMsg->m_mimoData->m_elementCount;

            // The sender sends data to his neighbor based on neighbor's
            // capability. If sender sends with ntx antenna elements. Receiver
            // could have num antennas (nrx) either more or equal to sender
            // antenna elements (nrx >= ntx).
            ERROR_AssertArgs(ntx <= getNumActiveAtnaElems(),
                "Incorrect antenna configuration");
            int nrx = ntx;

            MIMO::Demapper demapper(ntx, nrx, branch_snr_dB, lambda_k, true);
            for (int k(0); k < ntx; k++)
            {
              demapper(k, MIMO::Stbc::Uncoded());
            }

            double mimo_snr_dB = demapper();
            double sinr = pow(10, mimo_snr_dB / 10.0);

            if (DEBUG)
            {
                std::cout << "snr(dB): " << mimo_snr_dB << " sinr: " << sinr << std::endl;
            }

            MAC_PHY_TxRxVector rxVector = m_RxSignalInfo.m_rxVector;
            rxVector.chBwdth = CHBWDTH_20MHZ;
            double dataRate = getDataRate(rxVector);
            double ber = getBer(sinr, rxVector);

            if (inError(propRxInfo, TRUE, FALSE, ber, dataRate))
            {
                return TRUE;
            }
            return FALSE;
        }
        else
        {
            ERROR_ReportError(" Bands don't overlap.");
            return TRUE;
        }
    }
    else
    {
        spectralBand* tsb = cc()->getSBand(mchBwdth);
        double branch_snr_dB = node()->sl().snr(propRxInfo, tsb, phyIndex);
        for (int k(0); k < lambda_k.size(); k++)
        {
            double lr = std::real(lambda_k(k));
            if (DEBUG)
            {
                std::cout << "lr(" << k << "): " << lr << std::endl;
            }
        }

        int ntx = propRxInfo->txMsg->m_mimoData->m_elementCount;

        // The sender sends data to his neighbor based on neighbor's
        // capability. If sender sends with ntx antenna elements. Receiver
        // could have num antennas (nrx) either more or equal to sender
        // antenna elements (nrx >= ntx).
        ERROR_AssertArgs(ntx <= getNumActiveAtnaElems(),
            "Incorrect antenna configuration");
        int nrx = ntx;

        MIMO::Demapper demapper(ntx, nrx, branch_snr_dB, lambda_k, true);
        for (int k(0); k < ntx; k++)
        {
          demapper(k, MIMO::Stbc::Uncoded());
        }

        double mimo_snr_dB = demapper();
        double sinr = pow(10, mimo_snr_dB / 10.0);

        if (DEBUG)
        {
            std::cout << "snr(dB): " << mimo_snr_dB << " sinr: " << sinr << std::endl;
        }

        *sinrPtr = sinr;

        BOOL containAMPDU = FALSE;
        double ber = getBer(sinr, m_RxSignalInfo.m_rxVector);
        double dataRate = getDataRate(m_RxSignalInfo.m_rxVector);

        if (plcp->containAMPDU)
        {
            containAMPDU = TRUE;

            void* info_ptr = (void*)MESSAGE_AddInfo(node(),
                m_parentData->rxMsg, sizeof(double), INFO_TYPE_Dot11nBER);

            memcpy(info_ptr, (void*)&ber, sizeof(double));
        }

        return inError(propRxInfo, FALSE, containAMPDU, ber, dataRate);
    }
}


/// \brief Calculates bit error rate
///
/// \param sinr Signal to noise ratio
/// \param rxVector Rx vector
///
/// \return Bit Error rate
double Phy802_11n::getBer(double sinr, const MAC_PHY_TxRxVector& rxVector)
{
    double ber = 0;
    double bandwidth = getBwdth_MHz(rxVector.chBwdth);
    double dataRate = getDataRate(rxVector);

    ber = PHY_MIMOBER(m_parentData->thisPhy,
                  sinr,
                  rxVector,
                  dataRate,
                  bandwidth,
                  std::min(m_RxSignalInfo.m_txNumAtnaElmts, m_NumAtnaElmts),
                  m_RxSignalInfo.m_chnlEstMatrix,
                  getRxModel(rxVector));
    return ber;
}


/// \brief Check whether received packet has error or not.
///
/// \param propRxInfo Pointer to Propagation rx info
/// \param isDup      Is Dup message or not
/// \param isAmpdu    Is Ampdu or not
/// \param BER        Bit error rate
/// \dataRate         Data rate
///
/// \return True if packet has error, false otherwise
BOOL Phy802_11n::inError(PropRxInfo* propRxInfo, BOOL isDup,
        double isAmpdu, double BER, double dataRate)
{
    BOOL inError = FALSE;
    if (BER != 0.0)
    {
        double numBits = 0;

        if (!isAmpdu)
        {
            numBits = ((propRxInfo->duration) * dataRate/(double)SECOND);
        }
        else
        {
            ERROR_Assert(!isDup, "Invalid frame");
            numBits = sizeof(Phy802_11PlcpHeader) * 8;
        }

        double errorProbability = 1.0 - pow((1.0 - BER), numBits);
        double rand = RANDOM_erand(m_parentData->thisPhy->seed);

        ERROR_Assert((errorProbability >= 0.0) && (errorProbability <= 1.0),
            "Error Probability should be b/w 0-1");

        if (errorProbability >= rand)
        {
            inError = TRUE;
        }
    }

    return inError;
}

/// \brief Checks if the signal can be processed
///
/// \param phyIndex Index of this phy
/// \param propRxInfo Pointer to Propagation rx info
/// \param rxPower_dBm Signal power in dBm
///
/// \return True if signal can be processed. False otherwise.
BOOL Phy802_11n::processSignal(int phyIndex,
                       PropRxInfo* propRxInfo,
                       double rxPower_dBm)
{
    Message* msg = propRxInfo->txMsg;

    spectralBand* msb = MESSAGE_GetSpectralBand(msg);

    Phy802_11PlcpHeader* plcp
                  = (Phy802_11PlcpHeader*)MESSAGE_ReturnPacket(propRxInfo->txMsg);
    BOOL isDupMsg = plcp->nonHTMod == NON_HT_DUP_OFDM? TRUE:FALSE;
    ChBandwidth mchBwdth = getChBwdth(msb->getBandwidth());
    BOOL process = FALSE;
    if (isDupMsg)
    {
        spectralBand* cf= cc()->getSBand(CHBWDTH_20MHZ);
        if (cf->overlaps(*msb))
        {
            process = TRUE;
        }
    }
    else
    {
        if (mchBwdth <= getOperationChBwdth())
        {
            spectralBand* tsb = cc()->getSBand(mchBwdth);
            if (tsb->getFrequency() == msb->getFrequency())
            {
                process = TRUE;
            }
        }
    }
    return process;
}

/// \brief Calculates the control overhead
///
/// \return Control overhead
int Phy802_11n::getControlOverhead()
{
    int overheadSize = 0;
    MAC_PHY_TxRxVector rxVector;
    getRxVectorOfLockedSignal(rxVector);
    int numSts
        = MCS_Params[rxVector.chBwdth - 1][rxVector.mcs].m_nSpatialStream;
    clocktype dataRate =
        Phy802_11n::MCS_Params[CHBWDTH_20MHZ - 1][0].m_dataRate[0];
    clocktype preamble = preambDur_HtGf(numSts, rxVector.numEss);

    overheadSize = (int)((preamble * dataRate) / SECOND
                    + Ppdu_Service_Bits_Size
                    + Ppdu_Tail_Bits_Size);
    overheadSize /= 8;
    return overheadSize;
}

