/// \file phy_802_11_manager.h
///
/// \brief Basic data structure for 802_11
///
/// This file contains Class Phy802_11Manager which serve as base class
/// for phy 802_11 family of protocols

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

#ifndef PHY_802_11_MANAGER
#define PHY_802_11_MANAGER

#include "mac_phy_802_11n.h"
#include "mac_dot11_channelmanager.h"

struct PhyData802_11;
struct Phy802_11PlcpHeader;

class Phy802_11Manager
{
protected:
    /*!
     * \brief Code rate supported in 802.11
     */
    enum CodeRate {
        CODERATE_1_2,
        CODERATE_2_3,
        CODERATE_3_4,
        CODERATE_5_6,
    };

    /*!
     * \brief Modulation supported in 802.11ac
     */
    enum Modulation {
        BPSK,
        QPSK,
        QAM16,
        QAM64,
        QAM256
    };

    /*!
     * \brief Receive signal information structure
     */
    struct  RxSignalInfo {
        MAC_PHY_TxRxVector  m_rxVector;
        int                 m_txNumAtnaElmts;
        double              m_txAtnaElmtSpace;
        CplxMiniMatrix      m_chnlEstMatrix;    //Channel estimation matrix
    };

    /*!
     * \brief Operation mode
     */
    Mode            m_Mode;

    /*!
     * \brief operation channel bandwidth
     */
    ChBandwidth     m_ChBwdth;

    /*!
     * \brief Number of antenna array elements
     */
    int             m_NumAtnaElmts;

    /*!
    * \brief Number of active antenna array elements
    */
    int             m_NumActiveAtnaElmts;

    /*!
     * \brief Space between antenna array elements
     */
    double          m_AtnaSpace;

    /*!
     * \brief Whether to support short GI
     */
    BOOL            m_ShortGiCapable;

    /*!
     * \brief Whether to support STBC
     */
    BOOL            m_StbcCapable;

    /*!
     * \brief Whether to enable short GI transmission
     */
    BOOL            m_ShortGiEnabled;


    /*!
     * \brief Transmission parameters for the outgoing packet
     */
    MAC_PHY_TxRxVector      m_TxVector;

    /*!
     * \brief Receiving signal information
     */
    RxSignalInfo    m_RxSignalInfo;

     /*!
     * \brief Previous Rx vector information
     */
    MAC_PHY_TxRxVector m_PrevRxVector;

     /*!
     * \brief Pointer to parent 802.11PHY data structure
     */
    PhyData802_11* m_parentData;

    /*!
     * \brief Pointer to channel controller
     */
    Dot11::Qos::ChannelController* m_chController;

    /*!
    * \brief Pointer to node
    */
    Node* m_node;

    /*!
    * \brief Interface index
    */
    int m_interfaceIndex;

public:
    /*!
     * \brief Tx data rate type for backward compatibility
     */
    int txDataRateTypeForBC;

    /*!
     * \brief Tx data rate type
     */
    int txDataRateType;

    /*!
     * \brief Default Tx power in dBm
     *
     * This is the common data structure for 802.11a/b default
     * tx power based on data rates.
     */
    float txDefaultPower_dBm[8];

    /*!
     * \brief Rx data rate type
     */
    int rxDataRateType;

    /*!
     * \brief Data rate
     */
    int dataRate[8];

    /*!
     * \brief Number of data bits per symbol
     */
    double numDataBitsPerSymbol[8];

    /*!
     * \brief Number of data bits per symbol for 802.11b
     */
    double numDBPS[4];

    /*!
     * \brief Modulation and coding parameters corresponding to each MCS index
     */
    struct MCSParam
    {
        //number of spatial streams
        unsigned int   m_nSpatialStream;

        //coding rate
        CodeRate        m_codeRate;

        // modulation
        Modulation      m_modulation;

        // number of data bits per OFDM symbol
        unsigned int    m_nDataBitsPerSymbol;

        // Number of BCC coder
        unsigned int    m_numEs;

        // data rate
        UInt64    m_dataRate[GI_NUMS];
    };

    /*!
     * \brief Default TxRxVector value
     */
    static const MAC_PHY_TxRxVector Def_TxRxVector;

    /*!
     * \brief Base channel bandwidth 20MHz
     */
    static const Int64 Channel_Bandwidth_Base = 20000000;

    /*!
     * \brief IDFT/DFT period
     */
    static const clocktype T_Dft    = 3200 * NANO_SECOND;

    /*!
     * \brief Guard interval
     */
    static const clocktype T_Gi     = 800 * NANO_SECOND;

    /*!
     * \brief Short guard interval
     */
    static const clocktype T_Gis    = 400 * NANO_SECOND;

    /*!
     * \brief Symbol interval
     */
    static const clocktype T_Sym = 4000 * NANO_SECOND;


    /*!
     * \brief short GI symbol interval
     */
    static const clocktype T_Syms = 3600 * NANO_SECOND;


    /*!
     * \brief Number of bits in the SERVICE field
     */
    static const size_t Ppdu_Service_Bits_Size  = 16;

    /*!
     * \brief Number of tail bits per BCC encoder
     */
    static const size_t Ppdu_Tail_Bits_Size     = 6;

    double    phy802_11aNdbps[8];
    double    phy802_11bNdbps[4];

    /*!
     * \brief Get tx vector
     */
    MAC_PHY_TxRxVector getTxVector() const { return m_TxVector; }

    /*!
     * \brief Initializes phy_802_11_manager
     */
    virtual void init(const NodeInput* nodeInput) = 0;

    /*!
     * \brief Read bandwidth parameters
     */
    virtual void readBwdthParams(const NodeInput* nodeInput) = 0;

    void initDefaultParam();

    PhyRxModel getRxModel(MAC_PHY_TxRxVector rxVector);

    /*!
     * \brief Set the lowest tx data rate type
     */
    void setLowestTxDataRateType();

    /*!
     * \brief Finalize function for phy_802_11_manager
     */
    virtual void finalize(Node* node, int phyIndex);


    /*!
     * \brief Get Number of antenna elements
     */
    int getNumAtnaElems() const { return m_NumAtnaElmts; }

    /*!
     * \brief Returns pointer to channel controller
     */
    Dot11::Qos::ChannelController* cc() { return m_chController;}

    /*!
     * \brief Returns Space between antenna array elements
     */
    double getAtnaElemSpace() const { return m_AtnaSpace; }

    /*!
     * \brief Returns reference of parent phy802.11 data structure
     */
    PhyData802_11* phy() { return m_parentData;}

    /*!
     * \brief Returns Whether short gi capable or not
     */
    BOOL shortGiCapable() const { return m_ShortGiCapable; }

    /*!
     * \brief Returns Whether stbc capable or not
     */
    BOOL stbcCapable() const { return m_StbcCapable; }

    /*!
     * \brief Sets Tx vector
     */
    void setTxVector(const MAC_PHY_TxRxVector& txVector);

    /*!
     * \brief Sets operation channel bandwidth
     */
    void setOperationChBwdth(ChBandwidth chBwdth)
    {
        m_ChBwdth = chBwdth;
        cc()->setChBwdth(chBwdth);
    }

    /*!
     * \brief Returns operational channel bandwidth
     */
    ChBandwidth getOperationChBwdth() const
    {
        return m_ChBwdth;
    }

    /*!
     * \brief Returns bandwidth in MHz
     */
    double getBwdth_MHz(ChBandwidth bwdth);

    /*!
     * \brief Checks whether to process signal or not - 802.11n/ac
     */
    virtual BOOL processSignal(int phyIndex,
                       PropRxInfo* propRxInfo,
                       double rxPower_dBm) = 0;

    /*!
     * \brief Checks whether to process signal or not - 802.11a/b
     */
    virtual BOOL processSignal(int phyIndex) = 0;

    /*!
     * \brief Returns whether short guard interval is enabled or not
     */
    BOOL isGiEnabled() const { return m_ShortGiEnabled; }

    /*!
     * \brief Enables short GI transmission
     */
    void enableGi(BOOL enable) { m_ShortGiEnabled = enable; }


    /*!
     * \brief Returns Rx vector of locked signal
     */
    void getRxVectorOfLockedSignal(MAC_PHY_TxRxVector& rxVector) const
    {
        rxVector = m_RxSignalInfo.m_rxVector;
    }

    /*!
     * \brief Returns previousRx vector
     */
    void getPreviousRxVector(MAC_PHY_TxRxVector& rxVector) const
    {
        rxVector = m_PrevRxVector;
    }

    /*!
     * \brief Checks whether phy is able to sense signal or not
     */
    virtual BOOL carrierSensing(BOOL isSigEnd,
                                int phyIndex) = 0;

    /*!
     * \brief Read configuration parameters
     */
    void readCfgParams(const NodeInput* nodeInput);

    /*!
     * \brief Get frame duration - 802.11n/ac
     */
    virtual clocktype getFrameDuration(const MAC_PHY_TxRxVector& txParam);

    /*!
     * \brief Get frame duration - 802.11a/b
     */
    clocktype getFrameDuration(int dataRateType, int size);

    /*!
     * \brief Returns minimum receiving sensitivity based on
     * channel bandwidth.
     */
    virtual double getMinSensitivity_dBm(ChBandwidth chWidth) = 0;

    /*!
     * \brief Returns receiver sensitivity based on
     * configured bandwidth.
     */
    virtual double getSensitivity_dBm() = 0;

    /*!
    * \brief Returns receiver sensitivity in dBm based on
    * configured bandwidth and mcs.
    */
    virtual double getRxSensitivity_dBm(ChBandwidth chWidth, int mcs)
    {
        return 0.0;
    }

    /*!
     * \brief Function for clear channel assessment
     */
    virtual ChBandwidth cca(ChBandwidth chBwdth) = 0;

    void terminateCurrentReceive(int phyIndex);

    void terminateCurrentTransmission(Node* node, int phyIndex);

    /*!
    * \brief Handles the signal arrival event from the kernel
    */
    virtual void signalArrival(Node* node,
                       int phyIndex,
                       int channelIndex,
                       PropRxInfo *propRxInfo) = 0;
    /*!
     * \brief Handles the signal end event from the kernel
     */
    virtual void signalEnd(Node* node,
                   int phyIndex,
                   int channelIndex,
                   PropRxInfo *propRxInfo) = 0;

    /*!
     * \brief Initializes phy_802_11_manager class
     */
    Phy802_11Manager(Mode mode, PhyData802_11* parentData,
            Dot11::Qos::ChannelController* p_Ch,
            Node* node,
            int InterfaceIndex,
            const NodeInput *nodeInput,
            AcMode phyMode);
    ~Phy802_11Manager()
    {
        delete m_chController;
    }

    /*!
     * \brief Fills plcp Header
     */
    void fillPlcpHdr(Phy802_11PlcpHeader* plcpHdr) const;

    /*!
     * \brief Locks Signal
     */
    void lockSignal(Node* node,
                    int channelIndex,
                    Phy802_11PlcpHeader* plcpHdr,
                    const Orientation& txDOA,
                    const Orientation& rxDOA,
                    int    txNumAtnaElmts,
                    double txAtnaElmtSpace);

    /*!
     * \brief Unlocks Signal
     */
    void unlockSignal();


    /*!
     * \brief Releases signal to channel
     */
    void releaseSignalToChannel(
        Node* node,
        Message* packet,
        int phyIndex,
        int channelIndex,
        float txPower_dBm,
        clocktype duration,
        clocktype delayUntilAirborne,
        double directionalAntennaGain_dB);

    /*!
     * \brief Returns pointer to node
     */
    Node* node()
    {
        return m_node;
    }

    /*!
     * \brief Returns interface index
     */
    int ifIdx()
    {
        return m_interfaceIndex;
    }

    /*!
     * \brief Returns default operational mode
     */
    virtual Mode getDefaultMode() = 0;

    /*!
     * \brief Returns channel bandwidth
     *
     * This is a mapping function between bandwidth in Hz and bandwidth enumerator
     */
    static ChBandwidth getChBwdth(double bw);

    /*!
     * \brief Returns non Ht bandwidth
     */
    ChBandwidth getNonHtBwdth(UInt8 count);

    /*!
    * \brief Sets number of active antenna elements
    */
    void setNumActiveAtnaElems(RfChainMode mode)
    {
        if (mode == k_Single_Rf_Chain)
        {
            m_NumActiveAtnaElmts = 1;
        }
        else if (mode == k_All_Rf_Chain)
        {
            m_NumActiveAtnaElmts = m_NumAtnaElmts;
        }
    }

    /*!
    * \brief Returns number of active antenna elements
    */
    int getNumActiveAtnaElems() const
    {
        return m_NumActiveAtnaElmts;
    }

    /*!
    * \brief Returns default channel estimation matrix
    */
    CplxMiniMatrix getDefaultChEstimationMatrix(Node* txNode, Node* rxNode);
};

double PHY_MIMOBER(PhyData *thisPhy,
                   double snr,
                   MAC_PHY_TxRxVector rxVector,
                   double dataRate,
                   double bandwidth,
                   int numAtnaElmts,
                   CplxMiniMatrix channelMatrix, 
                   PhyRxModel = RX_MODEL_NONE);

#endif

