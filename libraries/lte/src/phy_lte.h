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

/*
 */

#ifndef _PHY_LTE_H_
#define _PHY_LTE_H_

#include <list>
#include <map>
#include <set>

#include "types.h"
#include "lte_common.h"
#include "lte_transport_block_size_table.h"
#include "matrix_calc.h"
#include "layer2_lte.h"
#include "lte_rrc_config.h"

#ifdef LTE_LIB_LOG
#include "log_lte.h"
#endif // LTE_LIB_LOG
#include "layer3_lte_measurement.h"
#include "layer3_lte.h"

///////////////////////////////////////////////////////////////
// swtiches
///////////////////////////////////////////////////////////////
// #define ERROR_DETECTION_DISABLE
#define PHY_LTE_APPLY_SAME_FADING_FOR_DL_AND_UL (0)
#define PHY_LTE_USE_IDEAL_PATHLOSS_FOR_FILTERING (0)
#define PHY_LTE_APPLY_SENSITIVITY_FOR_CELL_SELECTION (1)
#define PHY_LTE_ENABLE_INTERFERENCE_FILTERING (1)
#define PHY_LTE_ENABLE_ONETIME_ERROR_EVALUATION (1) // Must be used with PHY_LTE_ENABLE_INTERFERENCE_FILTERING
#define PHY_LTE_INTERFERENCE_FILTERING_IN_DBM (0)

#if PHY_LTE_ENABLE_INTERFERENCE_FILTERING
class LteExponentialMean;
#endif

///////////////////////////////////////////////////////////////
// define
///////////////////////////////////////////////////////////////
#define PHY_LTE_MCS_INDEX_LEN           (32)
#define PHY_LTE_NUM_BER_TALBES          (64)
#define PHY_LTE_CQI_INDEX_LEN           (16)
#define PHY_LTE_PRACH_CONFIG_INDEX_LEN  (32)
#define PHY_LTE_MAX_NUM_AVAILABLE_SUBFRAME          (10)
#define PHY_LTE_PREAMBLE_FORMAT_NOT_AVAILABLE       (-1)
#define PHY_LTE_SUBFRAME_NUMBER_NOT_AVAILABLE       (-1)
#define PHY_LTE_MAX_NUM_RB \
        (PHY_LTE_CHANNEL_BANDWIDTH_20MHZ_NUM_RB) //100
#define PHY_LTE_NUM_OFDM_SYMBOLS               (7)
#define PHY_LTE_NUM_SUBCARRIER_PER_RB          (12)
#define PHY_LTE_NUM_SUBCARRIER_SPACING_HZ      (15)
#define PHY_LTE_NUM_RS_RESOURCE_ELEMENTS_IN_RB (2)
#define PHY_LTE_RX_TX_TURNAROUND_TIME   (0)
#define PHY_LTE_IMPLEMENTATION_LOSS     (0.0) //in dB
#define PHY_LTE_DELTAFREQUENCY_HZ       (15000)   // 15kHz
#define PHY_LTE_MIN_DL_CHANNEL_NO       (0)
#define PHY_LTE_MIN_UL_CHANNEL_NO       (0)
#define PHY_LTE_MIN_NUM_TX_ANTENNAS      (1)
#define PHY_LTE_MAX_NUM_TX_ANTENNAS      (2)
#define PHY_LTE_MAX_UE_NUM_TX_ANTENNAS   (1)
#define PHY_LTE_MIN_NUM_RX_ANTENNAS      (1)
#define PHY_LTE_MAX_NUM_RX_ANTENNAS      (2)
#define PHY_LTE_DL_CQI_SNR_TABLE_LEN    (16)
#define PHY_LTE_DL_CQI_SNR_TABLE_OFFSET (1)
#define PHY_LTE_MAX_MCS_INDEX_LEN       (31)
#define PHY_LTE_MAX_RX_SENSITIVITY_LEN  (1)
#define PHY_LTE_MIN_TPC_ALPHA   (0)
#define PHY_LTE_MAX_TPC_ALPHA   (1)
#define PHY_LTE_MIN_CQI_REPORTING_INTERVAL     (1)
#define PHY_LTE_MAX_CQI_REPORTING_INTERVAL     (INT_MAX)
#define PHY_LTE_MIN_CQI_REPORTING_OFFSET       (0)
#define PHY_LTE_MAX_CQI_REPORTING_OFFSET       (INT_MAX)
#define PHY_LTE_MIN_RI_REPORTING_INTERVAL      (1)
#define PHY_LTE_MAX_RI_REPORTING_INTERVAL      (INT_MAX)
#define PHY_LTE_MIN_RI_REPORTING_OFFSET        (0)
#define PHY_LTE_MAX_RI_REPORTING_OFFSET        (INT_MAX)
#define PHY_LTE_MIN_SRS_TRANSMISSION_INTERVAL   (1)
#define PHY_LTE_MAX_SRS_TRANSMISSION_INTERVAL   (INT_MAX)
#define PHY_LTE_MIN_SRS_TRANSMISSION_OFFSET     (0)
#define PHY_LTE_MAX_SRS_TRANSMISSION_OFFSET     (INT_MAX)
#define PHY_LTE_MIN_CHECKING_CONNECTION_INTERVAL (1)
#define PHY_LTE_MAX_CHECKING_CONNECTION_INTERVAL (CLOCKTYPE_MAX)

#define PHY_LTE_DCI_RECEPTION_DELAY (4)
#define PHY_LTE_RA_PRACH_FREQ_OFFSET_MAX (-6)
// CPLength[nsec]=(144 * (1 / (15000 * 2048))) * SECOND = 4687.5
#define PHY_LTE_CP_LENGTH_NS (4687)
#define PHY_LTE_SIGNAL_DURATION_REDUCTION (PHY_LTE_CP_LENGTH_NS) // > 0
// 24576*Ts = 24576*(1/(1500*2048))[sec]
#define PHY_LTE_PREAMBLE_DURATION (800000)
#define PHY_LTE_PRACH_RB_NUM        (6)
#define PHY_LTE_PSS_INTERVAL (10)
#define PHY_LTE_MIN_PUCCH_OVERHEAD       (0)
#define PHY_LTE_MIN_RA_PRACH_FREQ_OFFSET (0)

#define PHY_LTE_SNR_OFFSET_FOR_CQI_SELECTION   (2.0) // dB
//Constants for invalid values
#define PHY_LTE_INVALID_CQI (-1)
#define PHY_LTE_INVALID_RI (-1)
#define PHY_LTE_INVALID_MCS (255)

//default parameter
#define PHY_LTE_DEFAULT_CHANNEL_BANDWIDTH \
         (PHY_LTE_CHANNEL_BANDWIDTH_10MHZ)
#define PHY_LTE_DEFAULT_TX_POWER_DBM  (23.0)
#define PHY_LTE_DEFAULT_DL_CANNEL     (0)
#define PHY_LTE_DEFAULT_UL_CANNEL           (1)
#define PHY_LTE_DEFAULT_NUM_TX_ANTENNAS      (1)
#define PHY_LTE_DEFAULT_NUM_RX_ANTENNAS      (1)
#define PHY_LTE_DEFAULT_NUM_PUCCH_OVERHEAD  (0)
#define PHY_LTE_DEFAULT_TPC_P_O_PUSCH       (-90.0)
#define PHY_LTE_DEFAULT_TPC_ALPHA           (1)
#define PHY_LTE_DEFAULT_CQI_REPORTING_OFFSET    (0)
#define PHY_LTE_DEFAULT_CQI_REPORTING_INTERVAL  (10)
#define PHY_LTE_DEFAULT_CQI_REPORTING_INTERVAL_K  (10)
#define PHY_LTE_DEFAULT_RI_REPORTING_OFFSET     (1)
#define PHY_LTE_DEFAULT_RI_REPORTING_INTERVAL   (10)
#define PHY_LTE_DEFAULT_CELL_SELECTION_MONITORING_PERIOD \
        (30 * MILLI_SECOND)
#define PHY_LTE_DEFAULT_NON_SERVING_CELL_MEASUREMENT_FOR_HO_PERIOD \
        (1 * MILLI_SECOND)
#define PHY_LTE_DEFAULT_NON_SERVING_CELL_MEASUREMENT_PERIOD \
        (200 * MILLI_SECOND)
#define PHY_LTE_CELL_SELECTION_MIN_SERVING_DURATION (1 * SECOND)
#define PHY_LTE_DEFAULT_CELL_SELECTION_RXLEVEL_MIN_DBM        (-140.0)
#define PHY_LTE_DEFAULT_CELL_SELECTION_RXLEVEL_MIN_OFF_DBM    (0.0)
#define PHY_LTE_DEFAULT_CELL_SELECTION_QUALLEVEL_MIN_DB       (-19.5)
#define PHY_LTE_DEFAULT_CELL_SELECTION_QUALLEVEL_MINOFFSET_DB (0.0)
#define PHY_LTE_DEFAULT_RA_PRACH_FREQ_OFFSET        (0)
#define PHY_LTE_DEFAULT_RA_DETECTION_THRESHOLD_DBM  (-100.0)
#define PHY_LTE_DEFAULT_SRS_TRANSMISSION_INTERVAL   (10)
#define PHY_LTE_DEFAULT_SRS_TRANSMISSION_OFFSET     (0)
#define PHY_LTE_DEFAULT_BER_TABLE_LEN (64)
#define PHY_LTE_DEFAULT_RXSENSITIVITY_DBM LTE_NEGATIVE_INFINITY_POWER_dBm
#define PHY_LTE_DEFAULT_PATHLOSS_FILTER_COEFFICIENT (40.0)
#define PHY_LTE_DEFAULT_CHECKING_CONNECTION_INTERVAL (1 * SECOND)
#if PHY_LTE_ENABLE_INTERFERENCE_FILTERING
#define PHY_LTE_DEFAULT_INTERFERENCE_FILTER_COEFFICIENT (40.0)
#endif
#define PHY_LTE_DEFAULT_INTERVAL_SUBFRAME_NUM_INTRA_FREQ    200 // once/200ms
#define PHY_LTE_DEFAULT_OFFSET_SUBFRAME_INTRA_FREQ          0
#define PHY_LTE_DEFAULT_FILTER_COEF_RSRP                    4
#define PHY_LTE_DEFAULT_FILTER_COEF_RSRQ                    4

#define PHY_LTE_TARGET_BLER (0.1)

#define PHY_LTE_ENABLE_SUBBAND_CQI (0)
#define PHY_LTE_NUM_MAX_RETRANMISSION (3)

/// Default value for CQI-SNR table.
/// Threshold value of SINR satisfying PER=1.0e-2 under
/// the assumption of 10 RBs allocation.
/// Note : Could be modified in later release.
const double PHY_LTE_DEFAULT_CQI_SNR_TABLE[PHY_LTE_DL_CQI_SNR_TABLE_LEN] = {
        -5.00, -4.42, -3.40, -1.70, -0.19, 1.34, 2.64, 5.16, 6.71, 8.18,
        10.43, 11.84, 13.32, 15.53, 16.20, 22.38 };

/// Mapping table of CQI to MCS
const Int32 lteCqiToMcsMapping[PHY_LTE_DL_CQI_SNR_TABLE_LEN] = {
        -1,                    // Not defined
        0,  1,  2,  4,  6,  9, // QPSK
        12, 14, 16,            // 16QAM
        19, 21, 23, 25, 27, 28 // 64QAM
};

///////////////////////////////////////////////////////////////
// typedef
///////////////////////////////////////////////////////////////
typedef Int32 PhyLteRandomNumberSeedType;

///////////////////////////////////////////////////////////////
// typedef enum
///////////////////////////////////////////////////////////////

/// 
typedef enum {
    PHY_LTE_CHANNEL_BANDWIDTH_1400KHZ = 1400000,
    PHY_LTE_CHANNEL_BANDWIDTH_3MHZ    = 3000000,
    PHY_LTE_CHANNEL_BANDWIDTH_5MHZ    = 5000000,
    PHY_LTE_CHANNEL_BANDWIDTH_10MHZ   = 10000000,
    PHY_LTE_CHANNEL_BANDWIDTH_15MHZ   = 15000000,
    PHY_LTE_CHANNEL_BANDWIDTH_20MHZ   = 20000000
} LteChannelBandwidth;

/// 
typedef enum {
    PHY_LTE_CHANNEL_BANDWIDTH_1400KHZ_NUM_RB = 6,
    PHY_LTE_CHANNEL_BANDWIDTH_3MHZ_NUM_RB    = 15,
    PHY_LTE_CHANNEL_BANDWIDTH_5MHZ_NUM_RB    = 25,
    PHY_LTE_CHANNEL_BANDWIDTH_10MHZ_NUM_RB   = 50,
    PHY_LTE_CHANNEL_BANDWIDTH_15MHZ_NUM_RB   = 75,
    PHY_LTE_CHANNEL_BANDWIDTH_20MHZ_NUM_RB   = 100
} LteChannelBandwidthNumRb;

/// MCS Table for PD-SCH
typedef enum {
    PHY_LTE_PDSCH_MODULATION_ORDER,
    PHY_LTE_PDSCH_TBS_INDEX, PHY_LTE_PDSCH_LEN
} LteMcsTableForPdsch;

/// 
typedef enum {
    PHY_LTE_MOD_INVALID,
    PHY_LTE_MOD_QPSK,
    PHY_LTE_MOD_16QAM,
    PHY_LTE_MOD_64QAM
} LteModulation;

/// 
typedef enum {
    PHY_LTE_CQI_INDEX,
    PHY_LTE_MODULATION,
    PHY_LTE_CODE_RATELTE,
    PHY_LTE_EFFICIENCY,
    PHY_LTE_CQI_TABLE_LEN
} LteCqiTableIndex;

/// 
typedef enum {
    PHY_LTE_POWER_OFF,
    // Cell selection
    PHY_LTE_IDLE_CELL_SELECTION,
    PHY_LTE_CELL_SELECTION_MONITORING,
    // Random access
    PHY_LTE_IDLE_RANDOM_ACCESS,
    PHY_LTE_RA_PREAMBLE_TRANSMISSION_RESERVED,
    PHY_LTE_RA_PREAMBLE_TRANSMITTING,
    PHY_LTE_RA_GRANT_WAITING,
    // Stationary status
    PHY_LTE_DL_IDLE,
    PHY_LTE_UL_IDLE,
    // eNB status
    PHY_LTE_DL_FRAME_RECEIVING,
    PHY_LTE_UL_FRAME_TRANSMITTING,
    // UE status
    PHY_LTE_UL_FRAME_RECEIVING,
    PHY_LTE_DL_FRAME_TRANSMITTING,
    PHY_LTE_STATUS_TYPE_LEN
} PhyLteState;

/// monitoring state during network connected status
typedef enum {
    PHY_LTE_NON_SERVING_CELL_MONITORING,
    PHY_LTE_NON_SERVING_CELL_MONITORING_IDLE
} PhyLteMonitoringState;

/// 
typedef enum {
    PHY_LTE_SYSTEM_FRAME_NUMBER_EVEN,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_NOT_AVAILABLE
} PhyLteSystemFrameNumber;

///////////////////////////////////////////////////////////////
// Structure for phy statistics variables
///////////////////////////////////////////////////////////////

/// PhyLteCqiTableColumn
typedef struct struct_phy_cqi_table_column {
    UInt8 cqiIndex;
    LteModulation modulation;
    UInt32 coderate;
    double efficiency;
} PhyLteCqiTableColumn;

/// PhyLtePdschTableColumn
typedef struct struct_phy_pdsch_table_column {
    UInt8 modulationOrder;
    UInt8 tbsIndex;
} PhyLtePdschTableColumn;

/// PhyLtePuschTableColumn
typedef struct struct_phy_pusch_table_column {
    UInt8 modulationOrder;
    UInt8 tbsIndex;
    UInt8 version;
} PhyLtePuschTableColumn;

/// Stats
typedef struct phy_lte_stats_str {
    Int32 totalTxSignals;
    Int32 totalSignalsLocked;
    Int32 totalRxTbsToMac;
    Int32 totalRxTbsWithErrors;
    Int32 totalRxInterferenceSignals;
    UInt64 totalBitsToMac;
} PhyLteStats;

/// lteinfo
typedef struct phy_lte_tx_info {
    LteStationType txStationType;
    D_Float32 txPower_dBm; // Transmission power per RB
    UInt8 numTxAnntena;
} PhyLteTxInfo;

/// packaged Rx-massage info
typedef struct phy_lte_rx_pack_msg_info_str {
    Message* rxMsg;
    Message* propRxMsgAddress;
    double txPower_mW;
    PropTxInfo propTxInfo; // Register PropTxInfo at Signal Arrival Timing,
            // Used for fading calculation when interference signal arrives
    PhyLteTxInfo lteTxInfo; // Register LteTxInfo at Signal Arrival Timing,
            // Used for fading calculation when interference signal arrives
    double rxPower_mW;
    double geometry;
    clocktype rxArrivalTime;
} PhyLteRxMsgInfo;

/// information of transport block
typedef struct phy_lte_tbs_info_str {
    BOOL isError;
#if !PHY_LTE_ENABLE_ONETIME_ERROR_EVALUATION
    clocktype rxTimeEvaluated;
#endif
    int transportBlockSize;
#ifdef LTE_LIB_LOG
#ifdef LTE_LIB_VALIDATION_LOG
    double sinr;
    UInt8 mcs;
#endif
#endif
} PhyLteTbsInfo;

/// 
typedef struct phy_lte_control_phy_to_mac_info {
    BOOL isError;
    LteRnti srcRnti;
} PhyLtePhyToMacInfo;

/// 
typedef struct phy_lte_sinrs {
    double sinr0; // For spatial stream 0
    double sinr1; // For spatial stream 1
} PhyLteSinrs;

/// 
typedef struct phy_lte_cqi {
    int cqi0; // For spatial stream 0
    int cqi1; // For spatial stream 1
    int subbandIndex; // -1 for wideband CQI
} PhyLteCqi;

/// 
typedef struct phy_lte_cqi_report_info {
    PhyLteCqi wideBandCqiInfo;
#if PHY_LTE_ENABLE_SUBBAND_CQI
    std::vector<PhyLteCqi> cqiInfo;
#endif
    int riInfo;

    // Context
    void clear(
#if PHY_LTE_ENABLE_SUBBAND_CQI
            int numSubband
#else
            int /*numSubband*/
#endif
            )
    {
#if PHY_LTE_ENABLE_SUBBAND_CQI
        cqiInfo.resize(numSubband);

        for (int i = 0; i < cqiInfo.size(); ++i)
        {
            cqiInfo[i].cqi0 = PHY_LTE_INVALID_CQI;
            cqiInfo[i].cqi1 = PHY_LTE_INVALID_CQI;
            cqiInfo[i].subbandIndex = -2;
        }
#endif
        wideBandCqiInfo.cqi0 = PHY_LTE_INVALID_CQI;
        wideBandCqiInfo.cqi1 = PHY_LTE_INVALID_CQI;
        wideBandCqiInfo.subbandIndex = -2;

        riInfo = PHY_LTE_INVALID_RI;
    }
    bool complete()
    {
        if (riInfo == PHY_LTE_INVALID_RI) return false;
        if (wideBandCqiInfo.cqi0 == PHY_LTE_INVALID_CQI) return false;

#if PHY_LTE_ENABLE_SUBBAND_CQI
        for (int i = 0; i < cqiInfo.size(); ++i)
        {
            if (cqiInfo[i].cqi0 == PHY_LTE_INVALID_CQI) return false;
        }
#endif

        return true;
    }
} PhyLteCqiReportInfo;

/// Management CQI receiving
class CqiReceiver {
    Node* _node;
    int _phyIndex;

    int _numSubband;

    // Valid CQIs
    PhyLteCqiReportInfo _cqiReportInfo;

    // Newest CQIs
    PhyLteCqiReportInfo _receivingReportInfo;

public:
    CqiReceiver();
    ~CqiReceiver();

    void init(Node* node, int phyIndex, int numSubband);
public:

    void notifyCqiUpdate(const PhyLteCqi& cqis);
    void notifyRiUpdate(int ri);
    bool getCqiReportInfo(PhyLteCqiReportInfo& cqiReportInfo);

#ifdef LTE_LIB_LOG
    void Dump(const char* which);
#endif

};

/// 
typedef struct phy_lte_connected_ue_info_str {
    double maxTxPower_dBm;

    LteDciFormat0 dciFormat0;

    // CQI receiver
    CqiReceiver cqiReceiver;

    // UL instant pathloss measured by SRS
    std::vector < double > ulInstantPathloss_dB;

    // UE lost detection
    BOOL isDetecting;
    clocktype connectedTime;
    BOOL isFirstChecking;
} PhyLteConnectedUeInfo;

/// 
typedef struct phy_lte_rx_dci_for_ue_info_str {
    LteDciFormat0 dciFormat0;
    clocktype rxTime;
    UInt64 rxTtiNumber;
} PhyLteRxDciForUlInfo;

/// Information of the RA preamble
typedef struct phy_lte_ra_preamble_str {
    LteRnti raRnti;
    LteRnti targetEnbRnti;
    unsigned int prachResourceStart;
    unsigned int preambleIndex;
    double txPower_mW;
    BOOL isHandingOverRa;
} PhyLteRaPreamble;

/// Information of the RRC setup complete
typedef struct phy_lte_rrc_setup_complete_str {
    LteRnti enbRnti;
    double maxTxPower_dBm;
} PhyLteRrcSetupComplete;



/// Information of the RRC setup complete
typedef struct phy_lte_rrc_reconf_complete_str {
    LteRnti enbRnti;
    double maxTxPower_dBm;
} PhyLteRrcReconfComplete;



/// Information of the received broadcast
typedef struct phy_lte_received_broadcast_str {
    PhyLteRaPreamble raPreamble;
    clocktype duration;
    bool collisionFlag;
    double rxPower_mW;
} PhyLteReceivedBroadcast;

/// Information of the receiving preamble
typedef struct phy_lte_receiving_ra_preamble_str {
    PhyLteRaPreamble raPreamble;
    clocktype arrivalTime;
    clocktype duration;
    BOOL collisionFlag;
    double rxPower_mW;
    Message* txMsg;
} PhyLteReceivingRaPreamble;

/// Data about establishment
typedef struct phy_lte_establishment_data_str {
    // Cell selection
    std::map < LteRnti, LteRrcConfig >* rxRrcConfigList;
    // Random Access
    LteRnti selectedRntieNB;
    LteRrcConfig selectedRrcConfig;
    std::list < PhyLteReceivingRaPreamble >* rxPreambleList;

    // eNB lost detection
    BOOL isDetectingSelectedEnb;

    // measurement config for HO (intra-frequency)
    BOOL measureIntraFreq;
    int intervalSubframeNum_intraFreq;
    int offsetSubframe_intraFreq;
    // measurement config for HO (inter-frequency)
    BOOL measureInterFreq;
    // common
    int filterCoefRSRP;
    int filterCoefRSRQ;
} PhyLteEstablishmentData;

/// SRS Information
typedef struct phy_lte_srs_info_str {
    LteRnti dstRnti;
} PhyLteSrsInfo;

/// Sub-band CQI configuration
struct PhyLteSubbandCqiConf
{
    int nPRB;
    int k; // Number of PRBs in a subband
    int J; // Number of bandwidth parts
};

/// BER table identifier
typedef struct PhyLteBerTableId
{
    UInt8 direction;     // 0:DL, 1:UL
    UInt8 MCS;           // 0,...,28
    UInt8 nRB;         // 0,...,110(0:Not Used)
    UInt8 mod[PHY_LTE_NUM_MAX_RETRANMISSION]; // 0,2,4,6 (0:None, 2:QPSK, 4:16QAM, 6:64QAM)

    // UInt8 snr[MAX_RETX];  // Quantized SNR : Not used
} PhyLteBerTableId;

typedef struct struct_phy_lte_str {
    // ----------------------------------------------------------- //
    //                        Common parameters
    // ----------------------------------------------------------- //
    PhyData* thisPhy;

    const NodeInput* nodeInput;

    // PHY state
    PhyLteState txState;
    PhyLteState previousTxState;

    PhyLteState rxState;
    PhyLteState previousRxState;

    // Station type eNB or UE
    LteStationType stationType;

    // rx parameters
    int rxModel;
    int numRxSensitivity;
    double* rxSensitivity_mW;

    // Message holders
    Int32 numTxMsgs;
    std::list < Message* >* txEndMsgList; // list of Tx End msgs for cancel
    std::map < int, Message* >* eventMsgList;// list of event msgs for cancel
    std::list < PhyLteRxMsgInfo* >* rxPackMsgList; // list of desired signals

    // list of interference signals
    std::list < PhyLteRxMsgInfo* >* ifPackMsgList;

    // interference power on each RBs
    double* interferencePower_mW;

#if PHY_LTE_ENABLE_INTERFERENCE_FILTERING
    // Filtered interference power on each RBs
    LteExponentialMean* filteredInterferencePower;
#endif

    // Antenna parameters
    UInt8 numTxAntennas;
    UInt8 numRxAntennas;

    // Thermal noise power in RB unit
    double rbNoisePower_mW;

    // Maximum transmission power
    double maxTxPower_dBm;

    // eNB & UE Common
    UInt8 subframePerTti;

    // Bandwidth parameters
    Int32 channelBandwidth;
    UInt8 numResourceBlocks;

    // Channel parameters
    int dlChannelNo;
    int ulChannelNo;

    // Frame parameters
    clocktype rxTxTurnaroundTime;
    clocktype txDuration;
    clocktype propagationDelay;

    // Filter coefficient for pathloss measurement
    double pathlossFilterCoefficient;

#if PHY_LTE_ENABLE_INTERFERENCE_FILTERING
    // Filter coefficient for interference power
    double interferenceFilterCoefficient;
#endif

    // Statistics for LTE PHY layer
    PhyLteStats stats;

    // CheckingConnection timer interval
    clocktype checkingConnectionInterval;

    // Establishment data
    PhyLteEstablishmentData* establishmentData;

    // ----------------------------------------------------------- //
    //                            For eNB
    // ----------------------------------------------------------- //

    // Map of connected UE information
    std::map < LteRnti, PhyLteConnectedUeInfo >* ueInfoList;

    // TPC parameters
    double tpcPoPusch_dBmPerRb;
    UInt8 tpcAlpha;

    // Cell selection parameters
    double cellSelectionRxLevelMin_dBm;
    double cellSelectionRxLevelMinOff_dBm;
    double cellSelectionQualLevelMin_dB;
    double cellSelectionQualLevelMinOffset_dB;

    // Random access parameters
    UInt8 raPrachFreqOffset;
    double raDetectionThreshold_dBm;

    // PUCCH overhead in RB unit
    UInt32 numPucchOverhead;

    // RA grant send UEs
    std::set < LteRnti >* raGrantsToSend;

    // ----------------------------------------------------------- //
    //                            For UE
    // ----------------------------------------------------------- //

    // CQI informations which UE report to eNB next at feedback timing.
    PhyLteCqi nextReportedCqiInfo;
    int nextReportedRiInfo;

    // CQI feedback context
    std::vector< std::vector<int> >* bandwidthParts;
    int nextReportSubbandIndex;

    // CQI reporting parameters
    int cqiReportingOffset;
    int cqiReportingInterval;
    int riReportingOffset;
    int riReportingInterval;
    int cqiReportingIntervalK; // Parameter K ( H = K*J + 1 )

    // Selection time of current eNB
    clocktype phySelectedTime;

    // Cell selection parameters
    clocktype nonServingCellMeasurementInterval;
    clocktype nonServingCellMeasurementPeriod;
    clocktype nonServingCellMeasurementForHoPeriod;
    clocktype cellSelectionMinServingDuration;

    // SRS transmission parameters
    int srsTransmissionInterval;
    int srsTransmissionOffset;

    // Monitoring state
    PhyLteMonitoringState monitoringState;

    // Flag for RRC setup complete signal transmission
    BOOL rrcSetupCompleteFlag;
    BOOL rrcReconfigCompleteFlag;

    // Holder for DCI information
    std::list < PhyLteRxDciForUlInfo >* rxDciForUlInfo;
    std::list < MeasurementReport >* rrcMeasurementReportList;
    RrcConnReconfInclMoblityControlInfomationMap* rrcConnReconfMap;

    // ----------------------------------------------------------- //
    //                      Other parameters
    // ----------------------------------------------------------- //

#ifdef LTE_LIB_LOG
    lte::Aggregator* aggregator;
#endif // LTE_LIB_LOG

#ifdef ERROR_DETECTION_DISABLE
    double debugPPER;
#endif

#ifdef LTE_LIB_LOG
#ifdef LTE_LIB_VALIDATION_LOG
    std::map < LteRnti, lte::LogLteAverager >* avgReceiveSinr;
    std::map < LteRnti, lte::LogLteAverager >* avgEffectiveSinr;
    std::map < LteRnti, lte::LogLteAverager >* avgTxPower;
    std::map < LteRnti, lte::LogLteAverager >* avgMcs;
    std::map < LteRnti, lte::LogLteAverager >* bler;
    std::map < LteRnti, lte::LogLteAverager >* blerLookup;
    std::map < LteRnti, lte::LogLteAverager >* avgHarqRetx;
    std::map < LteRnti, lte::LogLteAverager >* totalReceivedBits;
#endif
#endif

} PhyDataLte;

//--------------------------------------------------------------------------
//  Constant array
//--------------------------------------------------------------------------

/// 
#ifdef __PHY_LTE_CPP__
const PhyLteCqiTableColumn PHY_LTE_CQI_TABLE[PHY_LTE_CQI_INDEX_LEN] =
{
    {   0, PHY_LTE_MOD_INVALID, 0, 0.0},
    {   1, PHY_LTE_MOD_QPSK, 78, 0.1523},
    {   2, PHY_LTE_MOD_QPSK, 120, 0.2344},
    {   3, PHY_LTE_MOD_QPSK, 193, 0.3770},
    {   4, PHY_LTE_MOD_QPSK, 308, 0.6016},
    {   5, PHY_LTE_MOD_QPSK, 449, 0.8770},
    {   6, PHY_LTE_MOD_QPSK, 602, 1.1758},
    {   7, PHY_LTE_MOD_16QAM, 378, 1.4766},
    {   8, PHY_LTE_MOD_16QAM, 490, 1.9141},
    {   9, PHY_LTE_MOD_16QAM, 616, 2.4063},
    {   10, PHY_LTE_MOD_64QAM, 466, 2.7305},
    {   11, PHY_LTE_MOD_64QAM, 567, 3.3223},
    {   12, PHY_LTE_MOD_64QAM, 666, 3.9023},
    {   13, PHY_LTE_MOD_64QAM, 772, 4.5234},
    {   14, PHY_LTE_MOD_64QAM, 873, 5.1152},
    {   15, PHY_LTE_MOD_64QAM, 948, 5.5547}
};
#else
extern PhyLteCqiTableColumn PHY_LTE_CQI_TABLE[PHY_LTE_CQI_INDEX_LEN];
#endif

/// 
const PhyLtePdschTableColumn PHY_LTE_PDSCH_TABLE[PHY_LTE_MCS_INDEX_LEN] = {
    { 2,  0 }, { 2,  1 }, { 2,  2 }, { 2,  3 }, { 2,  4 }, { 2,  5 },
    { 2,  6 }, { 2,  7 }, { 2,  8 }, { 2,  9 }, { 4,  9 }, { 4, 10 },
    { 4, 11 }, { 4, 12 }, { 4, 13 }, { 4, 14 }, { 4, 15 }, { 6, 15 },
    { 6, 16 }, { 6, 17 }, { 6, 18 }, { 6, 19 }, { 6, 20 }, { 6, 21 },
    { 6, 22 }, { 6, 23 }, { 6, 24 }, { 6, 25 }, { 6, 26 },
    { 2, 0 }, // Reserved
    { 4, 0 }, // Reserved
    { 6, 0 }  // Reserved
};

/// 
const PhyLtePuschTableColumn PHY_LTE_PUSCH_TABLE[PHY_LTE_MCS_INDEX_LEN] = {
    { 2,  0, 0 }, { 2,  1, 0 }, { 2,  2, 0 }, { 2,  3, 0 }, { 2, 4, 0 },
    { 2,  5, 0 }, { 2,  6, 0 }, { 2,  7, 0 }, { 2,  8, 0 }, { 2, 9, 0 },
    { 2, 10, 0 }, { 4, 10, 0 }, { 4, 11, 0 }, { 4, 12, 0 }, { 4, 13, 0 },
    { 4, 14, 0 }, { 4, 15, 0 }, { 4, 16, 0 }, { 4, 17, 0 }, { 4, 18, 0 },
    { 4, 19, 0 }, { 6, 19, 0 }, { 6, 20, 0 }, { 6, 21, 0 }, { 6, 22, 0 },
    { 6, 23, 0 }, { 6, 24, 0 }, { 6, 25, 0 }, { 6, 26, 0 },
    { 0, 0, 1 }, // Reserved
    { 0, 0, 2 }, // Reserved
    { 0, 0, 3 }  // Reserved
};

/// 
const int
PHY_LTE_PRACH_PREAMBLE_FORMAT_TABLE[PHY_LTE_PRACH_CONFIG_INDEX_LEN] = {
    0, // PRACH Configuration Index: 0
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, // PRACH Configuration Index: 10
    0, 0, 0, 0, 0, 1, 1, 1, 1,
    1, // PRACH Configuration Index: 20
    1, 1, 1, 1, 1, 1, 1, 1, 1,
    PHY_LTE_PREAMBLE_FORMAT_NOT_AVAILABLE, // PRACH Configuration Index: 30
    1 };

/// 
const PhyLteSystemFrameNumber
PHY_LTE_PRACH_SYSTEM_FRAME_NUMBER_TABLE[PHY_LTE_PRACH_CONFIG_INDEX_LEN] = {
    PHY_LTE_SYSTEM_FRAME_NUMBER_EVEN, // PRACH Config Index: 0
    PHY_LTE_SYSTEM_FRAME_NUMBER_EVEN,
    PHY_LTE_SYSTEM_FRAME_NUMBER_EVEN,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY, // PRACH Config Index: 10
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_EVEN,
    PHY_LTE_SYSTEM_FRAME_NUMBER_EVEN,
    PHY_LTE_SYSTEM_FRAME_NUMBER_EVEN,
    PHY_LTE_SYSTEM_FRAME_NUMBER_EVEN,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY, // PRACH Config Index: 20
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_ANY,
    PHY_LTE_SYSTEM_FRAME_NUMBER_NOT_AVAILABLE, // PRACH Config Index: 30
    PHY_LTE_SYSTEM_FRAME_NUMBER_EVEN, };

/// 
const int
PHY_LTE_PRACH_SUBFRAME_NUMBER_TABLE
    [PHY_LTE_PRACH_CONFIG_INDEX_LEN][PHY_LTE_MAX_NUM_AVAILABLE_SUBFRAME] =
{
    {  1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }, // PRACH Config Index: 0
    {  4, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  7, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  4, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  7, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  1,  6, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  2,  7, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  3,  8, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  1,  4,  7, -1, -1, -1, -1, -1, -1, -1 },
    {  2,  5,  8, -1, -1, -1, -1, -1, -1, -1 }, // PRACH Config Index: 10
    {  3,  6,  9, -1, -1, -1, -1, -1, -1, -1 },
    {  0,  2,  4,  6,  8, -1, -1, -1, -1, -1 },
    {  1,  3,  5,  7,  9, -1, -1, -1, -1, -1 },
    {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9 },
    {  9, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  4, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  7, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  4, -1, -1, -1, -1, -1, -1, -1, -1, -1 }, // PRACH Config Index: 20
    {  7, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  1,  6, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  2,  7, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  3,  8, -1, -1, -1, -1, -1, -1, -1, -1 },
    {  1,  4,  7, -1, -1, -1, -1, -1, -1, -1 },
    {  2,  5,  8, -1, -1, -1, -1, -1, -1, -1 },
    {  3,  6,  9, -1, -1, -1, -1, -1, -1, -1 },
    {  0,  2,  4,  6,  8, -1, -1, -1, -1, -1 },
    {  1,  3,  5,  7,  9, -1, -1, -1, -1, -1 },
    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }, // PRACH Config Index: 30
    {  9, -1, -1, -1, -1, -1, -1, -1, -1, -1 } };

/// 
const UInt8 PHY_LTE_SIZE_OF_RESOURCE_BLOCK_GROUP_TABLE[4] = { 1, 2, 3, 4 };

/// Sub-band CQI configuration
static const PhyLteSubbandCqiConf PHY_LTE_SUBBAND_CQI_CONF[] =
{
    {6, 7, 1}, // Note that in 36.213, Subband CQI is not permitted when
    {8, 4, 1}, // system bandwidth RBs is less than 8
    {11,4, 2},
    {27,6, 3},
    {64,8, 4}
};

//------------------------------------------------------------------------
//  API functions
//------------------------------------------------------------------------

/// Ceiling function for integer.
///
/// \param a  Numerator
/// \param b  Denominator
///
/// \return ceil(a/b)
inline int PhyLteCeilInt(int a, int b)
{
    return (a%b == 0 ? a/b : a/b+1);
}

/// Get the number of transmit antennas.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return Number of antennas
UInt8 PhyLteGetNumTxAntennas(Node* node, int phyIndex);

/// Get the number of receiver antennas.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return Number of antennas
UInt8 PhyLteGetNumRxAntennas(Node* node, int phyIndex);

/// Get the maximum transmit power.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return Maximum transmit power(dBm)
double PhyLteGetMaxTxPower_dBm(Node* node, int phyIndex);

/// Calculate and setup of transmission signal power
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param packet  Message to send
///
/// \return Transmission power per RB [ dBm ]
double PhyLteGetTxPower(Node* node, Int32 phyIndex, Message* packet);

/// Get the uplink pathloss.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param target  target node rnti.
/// \param pathloss : std:  pathloss.
///
/// \return propagation delay.
BOOL PhyLteGetUlFilteredPathloss(Node* node, int phyIndex, LteRnti target,
        std::vector < double >* pathloss_dB);

/// Get the uplink pathloss.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param target  target node rnti.
/// \param pathloss : std:  pathloss.
///
/// \return propagation delay.
BOOL PhyLteGetUlInstantPathloss(Node* node, int phyIndex, LteRnti target,
        std::vector < double >* pathloss_dB);

/// Get the number of transmit antennas.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return Number of antennas
UInt8 PhyLteGetNumTxAntennas(Node* node, int phyIndex);

/// Get the number of receiver antennas.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return Number of antennas
UInt8 PhyLteGetNumRxAntennas(Node* node, int phyIndex);

/// Get the maximum transmit power.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return Maximum transmit power(dBm)
double PhyLteGetMaxTxPower_dBm(Node* node, int phyIndex);

/// Get the interference power received.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///    + interferencePower_mW
///    : LteExponentialMean**
///    : Pointer to the buffer of the pointer to the filtered value
/// \param len  The length of the array of interference power.
///
void PhyLteGetUlRxIfPower(Node* node, int phyIndex,
        double** interferencePower_mW, int* len);

#if PHY_LTE_ENABLE_INTERFERENCE_FILTERING
/// Get the filtered interference power received.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///    + filteredInterferencePower_dBm
///    : LteExponentialMean**
///    : Pointer to the buffer of the pointer to the filtered value
/// \param len  The length of the array of interference power.
///
void PhyLteGetUlFilteredRxIfPower(Node* node, int phyIndex,
        LteExponentialMean** filteredInterferencePower_dBm, int* len);
#endif

/// Get the number of resource blocks.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return number of resource blocks.
UInt8 PhyLteGetNumResourceBlocks(Node* node, int phyIndex);

/// Get the uplink control channel overhead.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return PucchOverhead.
UInt32 PhyLteGetUlCtrlChannelOverhead(Node* node, int phyIndex);

/// Get the downlink channel frequency.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return frequency.
double PhyLteGetDlChannelFrequency(Node* node, int phyIndex);

/// Get the uplink channel frequency.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return frequency.
double PhyLteGetUlChannelFrequency(Node* node, int phyIndex);

/// Get the number of subcarriers.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return number of subcarriers.
UInt8 PhyLteGetNumSubcarriersPerRb(Node* node, int phyIndex);

/// Get the number of symbols.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return number of symbols.
UInt8 PhyLteGetSymbolsPerRb(Node* node, int phyIndex);

/// Get the subcarrier spacing.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return subcarrier spacing.
int PhyLteGetSubcarrierSpacing(Node* node, int phyIndex);

//UInt8 PhyLteGetRbsPerSubframe(Node* node, int phyIndex); //TODO: Remove

/// Get the downlink mcs table.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
/// \param len  Pointer to The length of the mcs table.
/// Pointer to downlink mcs table structure.
const PhyLtePdschTableColumn* PhyLteGetDlMcsTable(Node* node, int phyIndex,
        int* len);

/// Get the uplink mcs table.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
/// \param len  Pointer to The length of the mcs table.
/// Pointer to uplink mcs table structure.
const PhyLtePuschTableColumn* PhyLteGetUlMcsTable(Node* node, int phyIndex,
        int* len);

/// Get the precoding matrix list.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return precoding matrix list.
Cmat PhyLteGetPrecodingMatrixList(Node* node, int phyIndex);

/// Get the RB groups size.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param numRb  The number of resource blocks.
///
/// \return RB groups size.
UInt8 PhyLteGetRbGroupsSize(Node* node, int phyIndex, int numRb);

/// Get the downlink transmission block size.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param mcsIndex  Index of the MCS.
/// \param numRb  The number of resource blocks.
///
/// \return downlink transmission block size.
int
PhyLteGetDlTxBlockSize(Node* node, int phyIndex, int mcsIndex, int numRbs);

/// Get the uplink transmission block size.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param mcsIndex  Index of the MCS.
/// \param numRb  The number of resource blocks.
///
/// \return uplink transmission block size.
int
PhyLteGetUlTxBlockSize(Node* node, int phyIndex, int mcsIndex, int numRbs);

/// Check if msg has no transport block info.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param msg  Pointer to message.
///
/// \return TRUE found.
/// FALSE not found.
BOOL PhyLteIsMessageNoTransportBlock(Node* node, int phyIndex, Message* msg);

/// Get the ber table.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
///
/// \return Pointer to ber table.
const PhyBerTable* PhyLteGetBerTable(Node* node, int phyIndex);

/// Get the reception model.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
///
/// \return reception model.
PhyRxModel PhyLteGetReceptionModel(Node* node, int phyIndex);

/// Get the cqi info.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param target  target node rnti.
///
/// \return cqi report info.
bool PhyLteGetCqiInfoFedbackFromUe(Node* node, int phyIndex, LteRnti target,
        PhyLteCqiReportInfo* getValue);

/// Get the cqi snr table.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param cqiTable  Pointer to the cqi table.
/// \param len  Pointer to The length of the cqi table.
///
/// \return cqi report info.
void PhyLteGetCqiSnrTable(Node* node, int phyIndex, Float64** cqiTable,
        int* len);

/// Get the rs ofdm symbol in rb.
///
/// \param node  Pointer to node
/// \param phyIndex  Index of the PHY
///
/// \return RS OFDM symbol in RB.
int PhyLteGetRsResourceElementsInRb(Node* node, int phyIndex);

/// Get newest DCI for UE.
/// When pop-flag is true, update newest-data.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param ttiNumber  Current TTI number
/// \param pop  when true, popping target one and remove older ones
/// \param setValue  data pointer of DCI for UE
///
/// \return when true, successful of get data.
BOOL PhyLteGetDciForUl(Node* node, int phyIndex, UInt64 ttiNumber, BOOL pop,
        LteDciFormat0* setValue);

/// Get the propagation delay.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
///
/// \return propagation delay.
clocktype PhyLteGetPropagationDelay(Node* node, int phyIndex);

/// To change a given state.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param phyLte  Pointer to the LTE PHY structure
/// \param status  State transition.
///
void PhyLteChangeState(Node* node, int phyIndex, PhyDataLte* phyLte,
        PhyLteState status);

//**
// FUNCTION   :: PhyLteGetMessageTxInfo
// LAYER      :: PHY
// PURPOSE    :: Get the transmit info.
// PARAMETERS ::
// + node     : Node*   : Pointer to node.
// + phyIndex : int     : Index of the PHY.
// RETURN     :: PhyLteTxInfo* : Pointer to LTE PHY transmit info.
PhyLteTxInfo* PhyLteGetMessageTxInfo(Node* node, int phyIndex, Message* msg);

/// Notify PHY layer of RRC connected complete.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param handingover  whether handingover
///
void PhyLteRrcConnectedNotification(Node* node, int phyIndex,
                                    BOOL handingover = FALSE);



/// 
///
/// \param node  Pointer to seed.
/// \param phyIndex  hash
///
inline PhyLteRandomNumberSeedType PhyLteHashInputsToMakeSeed(
        const PhyLteRandomNumberSeedType seed, const int hashingInput)
{
    UInt64 hash1 = seed;
    UInt64 hash2 = hashingInput;
    return static_cast < PhyLteRandomNumberSeedType > ((9838572817LL * hash1
            + 77138234763LL * ~hash2) % INT_MAX);
}

/// 
///
/// \param node  seed.
/// \param phyIndex  hash
/// \param phyIndex  hash
///
inline PhyLteRandomNumberSeedType PhyLteHashInputsToMakeSeed(
        const PhyLteRandomNumberSeedType seed, const int hashingInput1,
        const int hashingInput2) {
    return PhyLteHashInputsToMakeSeed(PhyLteHashInputsToMakeSeed(seed,
            hashingInput1), hashingInput2);
}

/// 
///
/// \param node  seed.
/// \param phyIndex  hash
/// \param phyIndex  hash
/// \param phyIndex  hash
///
inline PhyLteRandomNumberSeedType PhyLteHashInputsToMakeSeed(
        const PhyLteRandomNumberSeedType seed, const int hashingInput1,
        const int hashingInput2, const int hashingInput3) {
    return PhyLteHashInputsToMakeSeed(PhyLteHashInputsToMakeSeed(seed,
            hashingInput1), hashingInput2, hashingInput3);
}

/// 
///
/// \param node  seed.
/// \param phyIndex  hash
/// \param phyIndex  hash
/// \param phyIndex  hash
/// \param phyIndex  hash
///
inline PhyLteRandomNumberSeedType PhyLteHashInputsToMakeSeed(
        const PhyLteRandomNumberSeedType seed, const int hashingInput1,
        const int hashingInput2, const int hashingInput3,
        const int hashingInput4) {
    return PhyLteHashInputsToMakeSeed(PhyLteHashInputsToMakeSeed(seed,
            hashingInput1), hashingInput2, hashingInput3, hashingInput4);
}

/// 
///
/// \param node  Pointer to seed.
/// \param phyIndex  hash
/// \param phyIndex  hash
/// \param phyIndex  hash
/// \param phyIndex  hash
/// \param phyIndex  hash
///
inline PhyLteRandomNumberSeedType PhyLteHashInputsToMakeSeed(
        const PhyLteRandomNumberSeedType seed, const Int32 hashingInput1,
        const Int32 hashingInput2, const Int32 hashingInput3,
        const Int32 hashingInput4, const Int32 hashingInput5) {
    return PhyLteHashInputsToMakeSeed(PhyLteHashInputsToMakeSeed(seed,
            hashingInput1), hashingInput2, hashingInput3, hashingInput4,
            hashingInput5);
}

/// Calculate channel matrix.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param phyLte  PHY data structure
/// \param propTxInfo  message of propTx.
/// \param lteTxInfo  LTE-info of propRx.
///    + matH         : Cmat& :
///    Reference to the buffer channel matrix is set to.
///
void PhyLteGetChannelMatrix(Node* node, int phyIndex, PhyDataLte* phyLte,
        PropTxInfo* propTxInfo, PhyLteTxInfo* lteTxInfo, Cmat& matH);

/// Calculate channel matrix including pathloss.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param phyLte  PHY data structure
/// \param propTxInfo  message of propTx.
/// \param lteTxInfo  LTE-info of propRx.
/// \param pathloss_dB  Pathloss of propRx.
///    + matH         : Cmat& :
///    Reference to the buffer channel matrix is set to.
///
void PhyLteGetChannelMatrix(Node* node, int phyIndex, PhyDataLte* phyLte,
        PropTxInfo* propTxInfo, PhyLteTxInfo* lteTxInfo, double pathloss_dB,
        Cmat& matHhat);

/// Apply pathloss for channel matrix.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param phyLte  PHY data structure
/// \param lteTxInfo  LTE-info of propRx.
/// \param pathloss_dB  Pathloss
/// \param matHhat  Channel matrix to apply pathloss_dB
///
void PhyLteChannelMatrixMultiplyPathloss(Node* node, int phyIndex,
        PhyDataLte* phyLte, PhyLteTxInfo* lteTxInfo, double pathloss_dB,
        Cmat& matHhat);

/// Set the transmit info.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param phyLte  Pointer to the LTE PHY structure.
/// \param txPower_dBm  Transmission power of RB in dBm.
///    + msg         : The massage address to append "info".
///
void PhyLteSetMessageTxInfo(Node* node, int phyIndex, PhyDataLte* phyLte,
        double txPower_dBm, Message* msg);

/// Set the Grant info.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param msg  The massage address to append RA grant info.
///
BOOL PhyLteSetGrantInfo(Node* node, int phyIndex, Message* msg);

/// Calculate SINR of each transport block.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param txScheme  Transmission scheme
/// \param phyLte  Pointer to LTE PHY structure
/// \param usedRB_list  RB allocation array
/// \param matHhat  Channel matrix
/// \param geometry  geometry
/// \param txPower_mW  Transmission power in mW
///    + useFilteredInterferencePower
///    : bool         : Whether to use filtered interference power
///    + isForCqi     : bool         :
///    Indicate SINR calculation is for CQI or not ( For log)
/// \param txRnti  RNTI of tx node. ( For log)
///
/// \return SINR
std::vector < double > PhyLteCalculateSinr(
        Node* node,
        int phyIndex,
        LteTxScheme txScheme,
        PhyDataLte* phyLte,
        UInt8 usedRB_list[PHY_LTE_MAX_NUM_RB],
        Cmat& matHhat,
        double geometry,
        double txPower_mW,
        bool useFilteredInterferencePower,
        std::vector< std::vector<double> >* sinrRBList,
        bool isForCqi, // for log
        LteRnti txRnti); // for log

/// Calculate SINR of each transport block.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param txScheme  Transmission scheme
/// \param phyLte  PHY data structure
/// \param geometry  geometry.
/// \param txPower_mW  Tx-signal power.
/// \param ifPower_mW  Interference signal power.
///
/// \return SINR
std::vector < double > PhyLteCalculateSinr(Node* node, int phyIndex,
        LteTxScheme txScheme, PhyDataLte* phyLte, Cmat& matHhat,
        double geometry, double txPower_mW, double ifPower_mW);

/// Create a unpacked received message structure.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param phyLte  Pointer to the LTE PHY structure.
/// \param propRxInfo  Information of the arrived signal.
/// \param rxpathloss_mW  Path loss.
/// \param txPower_mW  Transmit power.
///
/// \return Pointer to a structure created.
PhyLteRxMsgInfo* PhyLteCreateRxMsgInfo(Node* node, int phyIndex,
        PhyDataLte* phyLte, PropRxInfo* propRxInfo, double rxpathloss_mW,
        double txPower_mW);

/// Calculate propagation delay of each EU.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param propTxInfo  message of propTx.
///
void
PhyLteSetPropagationDelay(Node* node, int phyIndex, PropTxInfo* propTxInfo);

/// Terminate all ongoing transmissions
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteTerminateCurrentTransmissions(Node* node, int phyIndex);

/// get the list of RB.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param txScheme  Transmission scheme
/// \param numCodeWords  Number of *codewords*
/// \param tb2cwSwapFlag  Codeword to transport block swap flag
///    (Valid for Dic2aInfo)
/// \param usedRB_list  List of resource blocks allocated
/// \param numRb  Number of resource blocks allocated
/// \param mcsIndex  : std:  List of mcsIndex for
///    each *transport blocks*.
///
/// \return rfMsg->next if 2 transport blocks detected
/// rfMsg       otherwise.
Message* PhyLteGetResourceAllocationInfo(Node* node, int phyIndex,
        Message* rfMsg, LteTxScheme txScheme, int* numCodeWords,
        bool* tb2cwSwapFlag, UInt8* usedRB_list, int* numRb,
        std::vector < UInt8 >* mcsIndex);

/// Get the list of RB.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param rfMsg  received message
/// \param usedRB_list  Pointer to the buffer for list of RB
/// \param numRb  Pointer to the buffer for number of RBs
///
/// \return If DCI found
/// FALSE : otherwise
BOOL PhyLteGetResourceAllocationInfo(Node* node, int phyIndex,
        Message* rfMsg, UInt8* usedRB_list, int* numRb);

/// Initialize the LTE PHY
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param nodeInput  Pointer to the node input
///
void PhyLteInit(Node* node, const int phyIndex, const NodeInput* nodeInput);

/// Finalize the LTE PHY, print out statistics
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteFinalize(Node* node, const int phyIndex);

/// Notify PHY layer of power off.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteNotifyPowerOff(Node* node, const int phyIndex);

/// Notify PHY layer of power on.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteNotifyPowerOn(Node* node, const int phyIndex);

/// Add interference power
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param rxMsgInfo  PhyLteRxMsgInfo structure of interference signal
///
void PhyLteAddInterferencePower(Node* node,
        int phyIndex, PhyLteRxMsgInfo* rxMsgInfo);

/// Subtract interference power
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param rxMsgInfo  PhyLteRxMsgInfo structure of interference signal
///
void PhyLteSubtractInterferencePower(Node* node,
        int phyIndex, PhyLteRxMsgInfo* rxMsgInfo);

/// Handle signal arrival from the channel
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param channelIndex  Index of the channel receiving signal from
/// \param propRxInfo  Propagation information
///
void PhyLteSignalArrivalFromChannel(Node* node, int phyIndex,
        int channelIndex, PropRxInfo* propRxInfo);

/// Handle signal end from a chanel
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param channelIndex  Index of the channel receiving signal from
/// \param propRxInfo  Propagation information
///
void PhyLteSignalEndFromChannel(Node* node, int phyIndex, int channelIndex,
        PropRxInfo* propRxInfo);

/// Terminate all signals current under receiving.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteTerminateCurrentReceive(Node* node, int phyIndex);

/// Start transmitting a frame
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param packet  Frame to be transmitted
/// \param useMacLayerSpecifiedDelay  Use MAC specified
///    delay or calculate it
/// \param initDelayUntilAirborne  The MAC specified delay
///
void PhyLteStartTransmittingSignal(Node* node, int phyIndex, Message* packet,
        BOOL useMacLayerSpecifiedDelay, clocktype initDelayUntilAirborne);

/// End of the transmission
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param msg  The Tx End event
///
void PhyLteTransmissionEnd(Node* node, int phyIndex, Message* msg);

/// Response to the channel listening status changes when
/// MAC switches channels.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param channelIndex  channel that the node starts/stops listening to
/// \param startListening  TRUE if the node starts listening to the ch
///    FALSE if the node stops listening to the ch
///
void PhyLteChannelListeningSwitchNotification(Node* node, int phyIndex,
        int channelIndex, BOOL startListening);

/// Add connected-UE/Serving-eNB.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param ueRnti  RNTI of UE to add
/// \param phyLte  PHY data structure
///
/// \return registration of new UE is succeed,
/// FALSE : otherwise.
BOOL PhyLteAddConnectedUe(Node* node, int phyIndex, LteRnti ueRnti,
        PhyDataLte* phyLte);

/// Get the uplink Maximum transmit power(dBm).
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param target  target node rnti.
/// \param maxTxPower_dBm  Pointer to Maximum transmit power.
///
/// \return TRUE target found & get.
/// FALSE target not found.
BOOL PhyLteGetUlMaxTxPower_dBm(Node* node, int phyIndex, LteRnti target,
        double* maxTxPower_dBm);

/// Get TPC parameter P_O_PUSCH
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
/// \return TPC parameter P_O_PUSCH ( mW/RB)
double PhyLteGetTpcPoPusch(Node* node, int phyIndex);

/// Get TPC parameter alpha
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
/// \return TPC parameter alpha
double PhyLteGetTpcAlpha(Node* node, int phyIndex);

/// Get thermal noise
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
/// \return Thermal noise [ mW per RB ]
double PhyLteGetThermalNoise(Node* node, int phyIndex);

/// Evaluate transport block error
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param isDL  true if DL, otherwise false
/// \param buf  const HarqPhyLteReceiveBuffer* : Pointer to HARQ receiver buffer
/// \param bler  Storage for block error rate
/// \param refSinr_dB  Storage for reference SINR value to reference curve
///
/// \return TRUE if TB has error, otherwise FALSE
BOOL PhyLteEvaluateError(Node* node, int phyIndex,
        bool isDL, const HarqPhyLteReceiveBuffer* buf,
        double* bler, double* refSinr_dB);

/// Check the reception signals sensitivity.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param phyLte  PHY data structure
/// \param rxPower_mW  Rx-signals power
///
/// \return when true, Greater than or equal to receive sensitivity.
BOOL PhyLteLessRxSensitivity(Node* node,
                             int phyIndex,
                             PhyDataLte* phyLte,
                             double rxPower_mW);

/// Judge transmission scheme from the DCI information
/// on the message
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param msg  Message structure to be judged
///
/// \return Transmission scheme.
/// TX_SCHEME_INVALID if no tx scheme determined
LteTxScheme PhyLteJudgeTxScheme(Node* node, int phyIndex, Message* msg);

/// Determine transmission scheme from rankIndicator and
/// configured transmission mode.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param rankIndicator  Rank indicator
///    + numLayer  : int* :
///    Pointer to the buffer for number of layer.
///    + numTransportBlocksToSend : int* :
///    Pointer to buffer for number of transport blocks to send.
///
/// \return Transmission scheme
LteTxScheme PhyLteDetermineTransmissiomScheme(Node* node, int phyIndex,
        int rankIndicator, int* numLayer, int* numTransportBlocksToSend);

/// Calculate rank indicator
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param phyLte  Pointer to LTE PHY structure
/// \param propTxInfo  Pointer to propagation tx info
/// \param lteTxInfo  Pointer to LTE transmission info
/// \param pathloss_dB  Pathloss of received signal in dB
///
/// \return Rank indicator
int PhyLteCalculateRankIndicator(
    Node* node,
    int phyIndex,
    PhyDataLte* phyLte,
    PropTxInfo* propTxInfo, // Tx info of desired signal
    PhyLteTxInfo* lteTxInfo, // Tx info of desired signal
    double pathloss_dB);

/// Calculate CQI
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param phyLte  Pointer to LTE PHY structure
/// \param propTxInfo  Pointer to propagation tx info
/// \param lteTxInfo  Pointer to LTE transmission info
/// \param rankIndicator  Rank indicator
/// \param pathloss_dB  Pathloss of received signal in dB
/// \param forCqiFeedback  For CQI feedback or not
///    (For aggregation log)
///
/// \return Rank indicator
PhyLteCqi PhyLteCalculateCqi(
    Node* node,
    int phyIndex,
    PhyDataLte* phyLte,
    PropTxInfo* propTxInfo, // Tx info of desired signal
    PhyLteTxInfo* lteTxInfo, // Tx info of desired signal
    int subbandIndex,
    int rankIndicator,
    double pathloss_dB,
    bool forCqiFeedback); // for log

/// Get the downlink cqi snr table index.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param phyLte  Pointer to the LTE PHY structure.
/// \param sinr  Sinr.
///
int PhyLteGetDlCqiSnrTableIndex(Node* node, int phyIndex, PhyDataLte* phyLte,
        Float64 sinr);

/// Calculate pathloss excluding fading
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param lteTxInfo  Pointer to LTE transmission info.
/// \param propRxInfo  Pointer to propagation rx info.
///
/// \return Pathloss excluding fading in dB
double PhyLteGetPathloss_dB(Node* node,
                            int phyIndex,
                            PhyLteTxInfo* lteTxInfo,
                            PropRxInfo* propRxInfo);

#ifdef LTE_LIB_LOG
lte::Aggregator* PhyLteGetAggregator(Node* node, int phyIndex);
#endif // LTE_LIB_LOG
/// Set a CheckingConnection Timer
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteSetCheckingConnectionTimer(Node* node, int phyIndex);

/// Checking Connection
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param msg  CheckingConnection timer message
///
void PhyLteCheckingConnectionExpired(Node* node, int phyIndex, Message* msg);

/// Set a CheckingConnection Timer
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteSetInterferenceMeasurementTimer(Node* node, int phyIndex);

/// Timer for measuring interference power
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param msg  InterferenceMeasurement timer message
///
void PhyLteInterferenceMeasurementTimerExpired(Node* node, int phyIndex,
        Message* msg);

/// Indicate disconnection to UE
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param ueRnti  UE's RNTI
///
void PhyLteIndicateDisconnectToUe(Node* node, int phyIndex,
        const LteRnti& ueRnti);

/// Get the cqi table.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param len  Pointer to The length of the cqi table.
///
/// \return Pointer to the cqi table structure.
const PhyLteCqiTableColumn* PhyLteGetCqiTable(Node* node, int phyIndex,
        int* len);

#ifdef LTE_LIB_LOG

void PhyLteDebugOutputRxMsgInfoList(
        Node* node, int phyIndex, std::list < PhyLteRxMsgInfo* >* msgList);

void PhyLteDebugOutputRxMsgInfo(Node* node, int phyIndex,
        PhyLteRxMsgInfo* rxMsgInfo);
#endif

/// Process notification of RLC reset.
///    State variable like UL allocation information are deleted.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param rnti  opposite RNTI
void PhyLteNotifyRlcReset(Node* node, int phyIndex, const LteRnti& rnti);


/// clear information about opposite RNTI specified
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param rnti  opposite RNTI
///
void PhyLteClearInfo(Node* node, int phyIndex, const LteRnti& rnti);


/// reset to poweron state
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteResetToPowerOnState(Node* node, const int phyIndex);


/// Handle interference signal arrival from the chanel
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param channelIndex  Index of the channel receiving interference signal from
/// \param propRxInfo  Propagation information
/// \param sigPower_mW  The inband interference power in mW
///
void PhyLteInterferenceArrivalFromChannel(Node* node,
                                    int phyIndex,
                                    int channelIndex,
                                    PropRxInfo *propRxInfo,
                                    double sigPower_mW);


/// Handle interference signal end from a chanel
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param channelIndex  Index of the channel receiving interference signal from
/// \param propRxInfo  Propagation information
/// \param sigPower_mW  The inband interference power in mW
///
void PhyLteInterferenceEndFromChannel(Node* node,
                                int phyIndex,
                                int channelIndex,
                                PropRxInfo *propRxInfo,
                                double sigPower_mW);


#ifdef ADDON_DB
/// To get size of various control infos attached during packet
/// transmition in APIs PhyLteStartTransmittingSignal(...), and
/// PhyLteStartTransmittingSignalInEstablishment(...).
/// This API needs an update on adding new control infos in LTE.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param channelIndex  Index of the channel
/// \param msg  Pointer to the Message
///
/// \return Returns various control size attached.
Int32 PhyLteGetPhyControlInfoSize(Node* node,
                                  Int32 phyIndex,
                                  Int32 channelIndex,
                                  Message* msg);

/// Updates Stats-DB phy events table for the received messages
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param channelIndex  Index of the channel
/// \param propRxInfo  Pointer to propRxInfo
/// \param msgToMac  Pointer to the Message
/// \param eventStr       : std:  Receives eventType
///
void PhyLteUpdateEventsTable(Node* node,
                             Int32 phyIndex,
                             Int32 channelIndex,
                             PropRxInfo* propRxInfo,
                             Message* msgToMac,
                             const char* eventStr);

/// This API is used to Update event "PhyDrop" in Stats-DB phy
/// events table for those messages only are a part of the
/// Packed message received on PHY. For others places,
/// default API PHY_NotificationOfPacketDrop() is used.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param channelIndex  Index of the channel
/// \param propRxInfoTxMsg  Pointer to propRxInfo->txMsg
/// \param dropType             : std:  Receives drop reason
/// \param rxPower_dB  Rx power in dB
/// \param pathloss_dB  Pathloss in dB
/// \param msgInRxPackedMsg  Pointer to the message in received
///    packed message.
///
void PhyLteNotifyPacketDropForMsgInRxPackedMsg(
                                    Node* node,
                                    Int32 phyIndex,
                                    Int32 channelIndex,
                                    Message* propRxInfoTxMsg,
                                    const char* dropType,
                                    double rxPower_dB,
                                    double pathloss_dB,
                                    Message* msgInRxPackedMsg);
#endif // ADDON_DB

///Set the BER table for LTE
///
/// \param thisPhy  Pointer to PHY data.
void PhyLteSetBerTable(Node* node, PhyData* thisPhy);

/// Global initialization of LTE physical layer before creating partitions
void PhyLteGlobalInit();

/// Check if specified Message includes transport block
///
/// \param rxMsg  Received message
///
/// \return TRUE if specified Message has transport block,
///    otherwise FALSE.
BOOL PhyLteCheckDlTransportBlockInfo(Message* msg);

///Calculate transport block block error rate from received SNRs
///    considering HARQ retransmissions.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param isDL  TRUE if DL, otherwise FALSE
/// \param mcsIndex  MCS index of 1st transmission(0-28)
/// \param snrdB_list:  vector<vector<double>>
///    : History of SNRs of PRBs for each HARQ retransmission.
/// \param offset_dB  Offset value for looking up reference BLER curve.
///                   When to calculate estimated BLER for determining
///                   MCS/CQI, offset value greater than 0 can be
///                   specified as safe factor.
/// \param refSinr_dB  Storage for reference SNR used for looking up
///                    reference BLER curve.
/// \param cber  Storage for code block error rate
/// \param isForEstimation  true if this call is for link quality estimation
///
/// \return Transport block error rate [0.0, 1.0]
double PhyLteCalculateBlockErrorRate(Node* node, int phyIndex,
        bool isDL, int mcsIndex,
        std::vector< std::vector<double> >& snrdB_list,
        double offset_dB,
        double* refSinr_dB, double* cber, bool isForEstimation);

/// Get sub-band index of next feedback CQI
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param ttiCount  TTI number
///
/// \return  Sub-band index
int PhyLteGetNextCqiReportSubband(Node* node, int phyIndex, UInt64 ttiCount);

/// Get MCS index mapped to specified CQI index
///
/// \param node  pointer to node.
/// \param phyIndex  Index of the PHY.
/// \param nPRB  Number of PRBs
/// \param cqi  CQI index
///
/// \return Mapped MCS index
int PhyLteGetMCSFromCQI(Node* node, int phyIndex, int nPRB, int cqi);

/// Get number of sub-bands
///
/// \param systemBandWidthPRB  System bandwidth( Number of PRBs )
///
/// \return  Number of sub-bands
int PhyLteGetNumSubband(int systemBandWidthPRB);

/// Get number of PRBs in a sub-band
///
/// \param systemBandWidthPRB  System bandwidth( Number of PRBs )
///
/// \return  Number of PRBs in a sub-band
int PhyLteGetSubbandSize(int systemBandWidthPRB);

#endif /* _PHY_LTE_H_ */

