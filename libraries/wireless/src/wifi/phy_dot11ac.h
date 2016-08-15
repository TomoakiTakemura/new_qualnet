/// \file phy_dot11ac.h
///
/// \brief Dot11ac PHY data structures
///
/// This file contains data structures for dot11ac PHY model

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

#ifndef __PHY_DOT11_AC_H__
#define __PHY_DOT11_AC_H__

#include "mac_dot11_channelmanager.h"
#include "phy_802_11.h"
#include "phy_802_11_manager.h"

namespace Dot11
{

namespace Qos
{


/*!
 * \brief 802.11ac PHY class
 */
class Phy802_11ac : public Phy802_11Manager
{
public:

    /*!
    * \brief Number of MCS values supported in 802.11ac
    */
    static const size_t MCS_NUMS_SINGLE_SS = 10;

    /*!
     * \brief Returns whether two spectral bands are overlapping or not
     */
    BOOL overlap(spectralBand* tsb, spectralBand* msb);

    /*!
     * \brief Get Data rate from txRxVector
     */
    static double getDataRate(const MAC_PHY_TxRxVector& txRxVector);

    /*!
     * \brief Returns non Ht bandwidth
     */
    ChBandwidth getNonHtBwdth(UInt8 count);

    /*!
     * \brief Returns whether the packet is received with errors or not
     */
    BOOL inError(PropRxInfo* rxInfo, BOOL isDup,
            double isAmpdu, double BER, double dataRate);

    /*!
     * \brief Get bit error rate in the received signal
     */
    double getBer(double snr, const MAC_PHY_TxRxVector& rxVector);

    /*!
     * \brief Function for Clear channel assessment
     */
    ChBandwidth cca(ChBandwidth chBwdth);

    /*!
     * \brief Maximum EQM MCS index
    */
    static const int Max_EQM_MCS = 80;

    /*!
     * \brief Modulation and coding parameters corresponding to each MCS
     * index for dot11ac
    */
    static const MCSParam MCS_Params[NUM_CH_BW_802_11AC][Max_EQM_MCS];

    /*!
     * \brief Modulation and coding parameters corresponding to each MCS
     * index for dot11n
    */
    static const MCSParam MCS_Params_N[NUM_CH_BW_802_11N][32];

   /*!
     * \brief Non-HT Short Training field duration
    */
    static const clocktype T_L_STF  = 8 * MICRO_SECOND;

    /*!
     * \brief Non-HT Long Training field duration
    */
    static const clocktype T_L_LTF  = 8 * MICRO_SECOND;

    /*!
     * \brief Non-HT SIGNAL field duration
    */
    static const clocktype T_L_SIG = 4 * MICRO_SECOND;

    /*!
     * \brief VHT Signal A field duration
    */
    static const clocktype T_VHT_SIG_A = 8 * MICRO_SECOND;

    /*!
     * \brief VHT Short Training field duration
    */
    static const clocktype T_VHT_STF = 4 * MICRO_SECOND;

    /*!
     * \brief Duration of each VHTLTF symbol
    */
    static const clocktype T_VHT_LTF = 4 * MICRO_SECOND;

    /*!
     * \brief VHT Signal B field duration
    */
    static const clocktype T_VHT_SIG_B = 4 * MICRO_SECOND;

    /*!
     * \brief HT subsequent long training field duration
    */
    static const clocktype T_Ht_Ltfs= 4 * MICRO_SECOND;

    /*!
     * \brief HT signal field duration
    */
    static const clocktype T_Ht_Sig = 8 * MICRO_SECOND;
    
    /*!
     * \brief HT short training field duration
    */
    static const clocktype T_Ht_Stf = 4 * MICRO_SECOND;

    /*!
     * \brief HT first long training field duration
    */
    static const clocktype T_Ht_Ltf1= 4 * MICRO_SECOND;

    /*!
     * \brief Function to get minimum sensitivity value based
     * on channel bandwidth
     */
    double getMinSensitivity_dBm(ChBandwidth chWidth)
    {
        return m_RxSensitivity_dBm[chWidth - 1][0];
    }

    /*!
     * \brief Function to get sensitivity value based
     * on configured channel bandwidth
     */
    double getSensitivity_dBm()
    {
        return m_RxSensitivity_dBm[getOperationChBwdth() - 1][0];
    }

    /*!
     * \brief Returns energy threshold value in dBm
    */
    double energyT_dBm(Int32 sChannel)
    {
        return m_energyThreshold_dBm[sChannel];
    }

    /*!
     * \brief Registers the signal arrival event with the kernel
     */
    void registerSigArrivalFunc();

    /*!
     * \brief Handles the signal arrival event from the kernel
     */
    void signalArrival(Node* node,
                       int phyIndex,
                       int channelIndex,
                       PropRxInfo *propRxInfo);
    /*!
     * \brief Handles the signal end event from the kernel
     */
    void signalEnd(Node* node,
                   int phyIndex,
                   int channelIndex,
                   PropRxInfo *propRxInfo);

    /*!
     * \brief Finalize function of 802.11ac Physical layer
     */
    void finalize();

    /*!
     * \brief Run time statistics
     */
    void runTimeStats();

    /*!
     * \brief Get the preamble duration at VHT mode
     */
    static clocktype preambDur_VHT(unsigned char numSts)
    {
        int numVHTLTF;
        if (numSts < 3)
        {
            numVHTLTF = numSts;
        }
        else if (numSts < 5)
        {
            numVHTLTF = 4;
        }
        else if (numSts < 7)
        {
            numVHTLTF = 6;
        }
        else
        {
            numVHTLTF = 8;
        }
        clocktype preamDuration = T_L_STF + T_L_LTF + T_L_SIG
                                  + T_VHT_SIG_A + T_VHT_STF
                                  + (numVHTLTF * T_VHT_LTF) + T_VHT_SIG_B;
        return preamDuration;
    }

    /*!
     * \brief Get the frame duration at VHT mode
     */
    clocktype getFrameDuration(const MAC_PHY_TxRxVector& txParam);

    /*!
     * \brief Initialization function for 802_11ac
     */
    void init(const NodeInput* nodeInput);

    /*!
     * \brief Read 802_11ac sensitivity parameters
     */
    void readSensitivityParams(const NodeInput* nodeInput);

    /*!
     * \brief Initializes 802_11ac class
     */
    Phy802_11ac(PhyData802_11* parentData,
                Node* node,
                int interfaceIndex,
                const NodeInput *nodeInput)
                                          :Phy802_11Manager(MODE_VHT,
                                            parentData,
                                            new ChannelController,
                                            node,
                                            interfaceIndex,
                                            nodeInput,
                                            k_Mode_Dot11ac)
    {
        for (int i = 0; i < NUM_CH_BW_802_11AC; i++) {
                for (unsigned int j = 0; j < MCS_NUMS_SINGLE_SS; j++) {
                    m_RxSensitivity_dBm[i][j] = m_minSensitivity_dBm[i][j];
                }
            }

        // Setup additional parameters used in parentData
        m_parentData->channelBandwidth = (m_ChBwdth)*Channel_Bandwidth_Base;
        m_parentData->txPower_dBm = PHY802_11a_DEFAULT_TX_POWER__6M_dBm;
        m_parentData->rxTxTurnaroundTime = PHY802_11a_RX_TX_TURNAROUND_TIME;
        m_parentData->numDataRates = MCS_NUMS_SINGLE_SS;
        m_parentData->lowestDataRateType = 0;
        m_parentData->highestDataRateType = MCS_NUMS_SINGLE_SS - 1;
        init(nodeInput);
        this->initDefaultParam();
    }

    /*!
     * \brief Initialization function for PHY 802_11ab
     */
    void readBwdthParams(const NodeInput* nodeInput);

    /*!
     * \brief Initializes 802_11ac class with channelController
     * and parentData
     */
    BOOL checkPacketError(double *sinrPtr, PropRxInfo* propRxInfo, int phyIndex);

    BOOL processSignal(int phyIndex,
                       PropRxInfo* propRxInfo,
                       double rxPower_dBm);

    BOOL processSignal(int phyIndex)
    {
        ERROR_ReportError("Invalid API call");
        return 0;
    }

    /*!
     * \brief Checks whether phy is able to sense signal present
     * in medium or not
     */
    BOOL carrierSensing(BOOL isSignalEnd,
                        int phyIndex);
    /*!
     * \brief Returns default operational mode
     */
    Mode getDefaultMode()
    {
        return MODE_VHT;
    }

    /*!
     * \brief Function to get the preamble duration at non-HT mode
     */
    static clocktype preambDur_NonHt() {
        return T_L_STF + T_L_LTF + T_Sym;
    }

    /*!
     * \brief Function to get the preamble duration at HT mixed mode
     */
    static clocktype preambDur_HtMixed(unsigned char numSts,
                                       unsigned char numEss)
    {
        unsigned char numHtDltf = (numSts == 3 ? 4 : numSts);
        unsigned char numHtEltf = (numEss == 3 ? 4 : numEss);
        return preambDur_NonHt() + T_Ht_Sig
            + T_Ht_Stf + T_Ht_Ltf1 + (numHtDltf  - 1 + numHtEltf)*T_Ht_Ltfs;
    }

    /*!
     * \brief Returns the control overhead
     */
    int getControlOverhead();

    /*!
    * \brief Returns receiver sensitivity in dBm based on
    * configured bandwidth and mcs.
    */
    double getRxSensitivity_dBm(ChBandwidth chWidth, int mcs)
    {
        return m_RxSensitivity_dBm[chWidth - 1][mcs];
    }

    ~Phy802_11ac(){}

private:
    /*!
    * \brief Minimum sensitivity values
    */
    static const double m_minSensitivity_dBm[NUM_CH_BW_802_11AC][MCS_NUMS_SINGLE_SS];

    /*!
    * \brief Minimum energy threshold value in dBm
    */
    static const double m_energyThreshold_dBm[NUM_CH_BW_802_11AC - 1];

    /*!
    * \brief Minimum reception sensitivity in MW
    */
    double  m_RxSensitivity_dBm[NUM_CH_BW_802_11AC][MCS_NUMS_SINGLE_SS];
};

}
}

void Phy802_11acAddBerTable(PhyData* thisPhy);

#endif
