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


#ifndef PHY_802_11N_H
#define PHY_802_11N_H

#include "mac_dot11_channelmanager.h"
#include "phy_802_11.h"
#include "phy_802_11_manager.h"

namespace Dot11
{

namespace Qos
{
/*!
 * \brief 802.11n PHY class
 */
class Phy802_11n : public Phy802_11Manager
{
public:
    /*!
     * \brief 802.11n PHY Statistics
     */
    struct Stats {
        unsigned int totalTxGfSignals;              //Greenfield signals
        unsigned int totalRxGfSignalsToMac;
        unsigned int totalGfSignalsLocked;
        unsigned int totalGfSignalsWithErrors;
        unsigned int totalTxMfSignals;              //Mixed mode signals
        unsigned int totalRxMfSignalsToMac;
        unsigned int totalMfSignalsLocked;
        unsigned int totalMfSignalsWithErrors;
    };

    /*!
     * \brief Maximum EQM MCS index
    */
    static const int Max_EQM_MCS            = 32;

    /*!
     * \brief coding parameters corresponding to each MCS index
    */
    static const MCSParam MCS_Params[NUM_CH_BW_802_11N][Max_EQM_MCS];

    /*!
     * \brief Non-HT short traning symbol period
    */
    static const clocktype T_L_Stf  = 8 * MICRO_SECOND;
    
    /*!
     * \brief Non-HT long training symbol period
    */
    static const clocktype T_L_Ltf  = 8 * MICRO_SECOND;
    
    /*!
     * \brief Greenfield short training period.
    */
    static const clocktype T_Gf_Stf = 8 * MICRO_SECOND;
    
    /*!
     * \brief HT signal field duration
    */
    static const clocktype T_Ht_Sig = 8 * MICRO_SECOND;
    
    /*!
     * \brief HT short training field duration
    */
    static const clocktype T_Ht_Stf = 4 * MICRO_SECOND;

    /*!
     * \brief HT Greenfield first long training field duration
    */
    static const clocktype T_Gf_Ltf1= 8 * MICRO_SECOND;
    
    /*!
     * \brief HT first long training field duration
    */
    static const clocktype T_Ht_Ltf1= 4 * MICRO_SECOND;

    /*!
     * \brief HT subsequent long training field duration
    */
    static const clocktype T_Ht_Ltfs= 4 * MICRO_SECOND;

    /*!
     * \brief Number of MCS for one spatial stream
    */
    static const size_t MCS_NUMS_SINGLE_SS  = 8;

    /*!
     * \brief Minimum reception sensitivity
    */
    static const double Min_Rx_Senstivity[NUM_CH_BW_802_11N][MCS_NUMS_SINGLE_SS];

    /*!
     * \brief Minimum reception sensitivity in dBm
    */
    double  m_RxSensitivity_dBm[NUM_CH_BW_802_11AC][MCS_NUMS_SINGLE_SS];

    /*!
     * \brief Minimum sensitivity values
     */
    static const double m_minSensitivity_dBm[NUM_CH_BW_802_11N][MCS_NUMS_SINGLE_SS];

    /*!
     * \brief Function to get STBC field from number of space-time
     * streams and number of spatial streams
     */
    static unsigned char getStbc(unsigned char numSts, unsigned char numSs) {
        return numSts - numSs;
    }

    /*!
     * \brief Function to get Get number of space-time streams from STBC and
     * number of spatial stream
     */
    static unsigned char getNSts(unsigned char stbc, unsigned char numSs) {
        return stbc + numSs;
    }

    /*!
     * \brief Function to get the preamble duration at HT greenfield mode
     */
    static clocktype preambDur_HtGf(unsigned char numSts,
                                    unsigned char numEss)
    {
        unsigned char numHtDltf = (numSts == 3 ? 4 : numSts);
        unsigned char numHtEltf = (numEss == 3 ? 4 : numEss);
        return T_Gf_Stf + T_Gf_Ltf1 + T_Ht_Sig + (numHtDltf - 1 + numHtEltf)*T_Ht_Ltfs;
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
     * \brief Function to get the preamble duration at non-HT mode
     */
    static clocktype preambDur_NonHt() {
        return T_L_Stf + T_L_Ltf + T_Sym;
    }

    /*!
     * \brief Function to get the frame duration
     */
    clocktype getFrameDuration(const MAC_PHY_TxRxVector& txParam);

    /*!
     * \brief Function to Get datarate of a signal giving
     * its associated TxRxVector
     */
    static double getDataRate(const MAC_PHY_TxRxVector& txRxVector);

    /*!
     * \brief Function to get number of spatial stream of a signal
     * giving its associated TxRxVector
     */
    static size_t getNumSs(const MAC_PHY_TxRxVector& txRxVector);

    /*!
     * \brief Initializes 802_11n class
     */
    Phy802_11n(PhyData802_11* parentData,
         Node* node,
         int interfaceIndex,
         const NodeInput *nodeInput)
                                :Phy802_11Manager(MODE_HT_GF,
                                   parentData,
                                   new Dot11::Qos::ChannelController,
                                   node,
                                   interfaceIndex,
                                   nodeInput,
                                   k_Mode_Dot11n)
    {
        for (int i = 0; i < NUM_CH_BW_802_11N; i++) {
                for (unsigned int j = 0; j < MCS_NUMS_SINGLE_SS; j++) {
                    m_RxSensitivity_dBm[i][j] = m_minSensitivity_dBm[i][j];
                }
            }

        // Setup additional parameters used in parentData
        m_parentData->channelBandwidth = (m_ChBwdth + 1)*Channel_Bandwidth_Base;
        m_parentData->txPower_dBm = PHY802_11a_DEFAULT_TX_POWER__6M_dBm;
        m_parentData->rxTxTurnaroundTime = PHY802_11a_RX_TX_TURNAROUND_TIME;
        m_parentData->numDataRates = MCS_NUMS_SINGLE_SS;
        m_parentData->lowestDataRateType = 0;
        m_parentData->highestDataRateType = MCS_NUMS_SINGLE_SS - 1;
        init(nodeInput);
        this->initDefaultParam();
    }
    ~Phy802_11n() { }

    /*!
     * \brief Initialization function for PHY 802_11n
     */
    void init(const NodeInput* nodeInput);

    /*!
     * \brief Initialization function for PHY 802_11ab
     */
    void readBwdthParams(const NodeInput* nodeInput);

    /*!
     * \brief Finalization function for PHY 802_11n
     */
    void finalize(Node* node, int phyIndex);

    int  getNumAtnaElems() const { return m_NumAtnaElmts; }

    /*!
     * \brief Function to set operational channel bandwidth
     */
    void setOperationChBwdth(ChBandwidth chBwdth)
    {
        m_ChBwdth = chBwdth;
    }

    /*!
     * \brief Checks whether to process signal or not
     */
    BOOL processSignal(int phyIndex,
                       PropRxInfo* propRxInfo,
                       double rxPower_dBm);

    BOOL processSignal(int phyIndex)
    {
        ERROR_ReportError("Invalid API call");
        return 0;
    }

    /*!
     * \brief Function to Data rate of received singal
     */
    double getDataRateOfRxSignal() const
    {
        return getDataRate(m_RxSignalInfo.m_rxVector);
    }

    /*!
     * \brief Funtion to get receive sensitivity in mW
     */
    double getSensitivity()
    {
        return m_RxSensitivity_mW[getOperationChBwdth() - 1][0];
    }

    /*!
     * \brief Checks whether phy is able to sense signal present
     * in medium or not
     */
    BOOL carrierSensing(BOOL isSigEnd,
                        int phyIndex);

    /*!
     * \brief Function for Clear channel assessment
     */
    ChBandwidth cca(ChBandwidth chBwdth)
    {
        return CHBWDTH_20MHZ;
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
     * \brief Function to get minimum sensitivity value based
     * on channel bandwidth
     */
    double getMinSensitivity_dBm(ChBandwidth chWidth)
    {
        return m_RxSensitivity_dBm[chWidth - 1][0];
    }

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
     * \brief Returns the control overhead
     */
    int getControlOverhead();

    /*!
     * \brief Returns default operational mode
     */
    Mode getDefaultMode()
    {
        return MODE_HT_GF;
    }

    /*!
    * \brief Returns receiver sensitivity in dBm based on
    * configured bandwidth and mcs.
    */
    double getRxSensitivity_dBm(ChBandwidth chWidth, int mcs)
    {
        return m_RxSensitivity_dBm[chWidth - 1][mcs];
    }

protected:
    /*!
     * \brief Minimum reception senstivity in MW.
    */
    double m_RxSensitivity_mW[NUM_CH_BW_802_11N][MCS_NUMS_SINGLE_SS];

    /*!
     * \brief Read 802_11n sensitivity parameters
     */
    void readSensitivityParams(const NodeInput* nodeInput);

    /*!
     * \brief Initializes 802_11ac class with channelController
     * and parentData
     */
    BOOL checkPacketError(double *sinrPtr,
                            PropRxInfo* propRxInfo,
                            int phyIndex);

    /*!
     * \brief Get bit error rate in the received signal
     */
    double getBer(double snr, const MAC_PHY_TxRxVector& rxVector);

    /*!
     * \brief Returns whether the packet is received with errors or not
     */
    BOOL inError(PropRxInfo* rxInfo, BOOL isDup,
            double isAmpdu, double BER, double dataRate);

    /*!
     * \brief Function to read configuration parameters for backward
     * compatibility.
     */
    void readParamsForBackwardCompatibility(const NodeInput* nodeInput);

};
}
}
void Phy802_11nAddBerTable(PhyData* thisPhy);

#endif //PHY_802_11N_H

