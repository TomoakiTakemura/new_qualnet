#ifndef _LAYER2_LTE_MAC_H_
#define _LAYER2_LTE_MAC_H_

#include "lte_common.h"
#include "parallel.h"
#include "lte_harq.h"
#ifdef LTE_LIB_LOG
#include "log_lte.h"
#endif

////////////////////////////////////////////////////////////////////////////
// Define
////////////////////////////////////////////////////////////////////////////
#define MAC_LTE_RA_PREAMBLE_INDEX_MAX (64)

#define MAC_LTE_DEFAULT_RA_DELTA_PREAMBLE (0)
#define MAC_LTE_DEFAULT_RA_PRACH_MASK_INDEX (0)

#define MAC_LTE_RRELCIDFL_WITH_7BIT_SUBHEADER_SIZE (2)
#define MAC_LTE_RRELCIDFL_WITH_15BIT_SUBHEADER_SIZE (3)
#define MAC_LTE_RRELCID_SUBHEADER_SIZE (1)
#define MAC_LTE_DEFAULT_F_FIELD (1)
#define MAC_LTE_DEFAULT_LCID_FIELD (0)
#define MAC_LTE_DEFAULT_E_FIELD (0)

// default parameter
#define MAC_LTE_DEFAULT_RA_BACKOFF_TIME (10*MILLI_SECOND)
#define MAC_LTE_DEFAULT_RA_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER (-90.0)
#define MAC_LTE_DEFAULT_RA_POWER_RAMPING_STEP (2.0)
#define MAC_LTE_DEFAULT_RA_PREAMBLE_TRANS_MAX (4)
#define MAC_LTE_DEFAULT_RA_RESPONSE_WINDOW_SIZE (10)
#define MAC_LTE_DEFAULT_RA_PRACH_CONFIG_INDEX (14)
#define MAC_LTE_DEFAULT_PERIODIC_BSR_TTI (1)
#define MAC_LTE_DEFAULT_ENB_SCHEDULER_TYPE (LTE_SCH_ENB_RR)
#define MAC_LTE_DEFAULT_UE_SCHEDULER_TYPE (LTE_SCH_UE_SIMPLE)
#define MAC_LTE_DEFAULT_TRANSMISSION_MODE (1)
#define MAC_LTE_DEFAULT_MAX_HARQ_TX (4)

// parameter strings
#define MAC_LTE_STRING_RA_BACKOFF_TIME       "MAC-LTE-RA-BACKOFF-TIME"
#define MAC_LTE_STRING_RA_PREAMBLE_INITIAL_RECEIVED_TARGET_POWER \
    "MAC-LTE-RA-PREAMBLE-INITIAL-RECEIVED-TARGET-POWER"
#define MAC_LTE_STRING_RA_POWER_RAMPING_STEP "MAC-LTE-RA-POWER-RAMPING-STEP"
#define MAC_LTE_STRING_RA_PREAMBLE_TRANS_MAX "MAC-LTE-RA-PREAMBLE-TRANS-MAX"
#define MAC_LTE_STRING_RA_RESPONSE_WINDOW_SIZE \
    "MAC-LTE-RA-RESPONSE-WINDOW-SIZE"
#define MAC_LTE_STRING_RA_PRACH_CONFIG_INDEX  "MAC-LTE-RA-PRACH-CONFIG-INDEX"
#define MAC_LTE_STRING_NUM_SF_PER_TTI         "MAC-LTE-NUM-SF-PER-TTI"
#define MAC_LTE_STRING_PERIODIC_BSR_TTI       "MAC-LTE-PERIODIC-BSR-TTI"
#define MAC_LTE_STRING_ENB_SCHEDULER_TYPE     "MAC-LTE-ENB-SCHEDULER-TYPE"
#define MAC_LTE_STRING_UE_SCHEDULER_TYPE      "MAC-LTE-UE-SCHEDULER-TYPE"
#define MAC_LTE_STRING_TRANSMISSION_MODE      "MAC-LTE-TRANSMISSION-MODE"
#define MAC_LTE_STRING_MAX_HARQ_TX            "MAC-LTE-MAX-HARQ-TX"

////////////////////////////////////////////////////////////////////////////
// Enumlation
////////////////////////////////////////////////////////////////////////////
typedef enum
{
    LTE_SCH_UE_SIMPLE, // SIMPLE-SCHEDULER
    LTE_SCH_ENB_RR, // ROUND-ROBIN
    LTE_SCH_ENB_PF, // PROPORTIONAL-FAIRNESS
    LTE_SCH_NUM
} LTE_SCHEDULER_TYPE;

typedef enum {
    MAC_LTE_POWER_OFF,
    MAC_LTE_IDEL,
    MAC_LTE_RA_GRANT_WAITING,
    MAC_LTE_RA_BACKOFF_WAITING,
    MAC_LTE_DEFAULT_STATUS,
    MAC_LTE_STATUS_NUM
} MacLteState;
////////////////////////////////////////////////////////////////////////////
// Structure
////////////////////////////////////////////////////////////////////////////
/// R/R/E/LCID/F/L sub-header with 7-bit L field
// NOTE        :: This and actual size are diferent.
typedef struct {
    UInt8 e;
    UInt8 lcid;
    UInt8 f;
    UInt8 l;
} LteMacRrelcidflWith7bitSubHeader;

/// R/R/E/LCID/F/L sub-header with 15-bit L field
// NOTE        :: This and actual size are diferent.
typedef struct {
    UInt8 e;
    UInt8 lcid;
    UInt8 f;
    UInt16 l;
} LteMacRrelcidflWith15bitSubHeader;

/// R/R/E/LCID sub-header
// NOTE        :: This and actual size are diferent.
typedef struct {
    UInt8 e;
    UInt8 lcid;
} LteMacRrelcidSubHeader;

typedef struct {
    // UE   Number of sending Random Access Preamble
    UInt32 numberOfSendingRaPreamble;
    // ENB  Number of receiving Random Access Preamble
    UInt32 numberOfRecievingRaPreamble;
    // ENB  Number of sending Random Access Grant
    UInt32 numberOfSendingRaGrant;
    // UE   Number of receiving Random Access Grant
    UInt32 numberOfRecievingRaGrant;
    // UE   Number of establishment
    UInt32 numberOfEstablishment;

    UInt32 numberOfSduFromUpperLayer;
    UInt32 numberOfPduToLowerLayer;
    UInt32 numberOfPduFromLowerLayer;
    UInt32 numberOfSduToUpperLayer;

    UInt32 numberOfReceiveTb;
    UInt32 numberOfFirstReceiveTb;
    UInt32 numberOfSendTb;
    UInt32 numberOfFirstSendTb;
} LteMacStatData;

/// MAC Sublayer's data
typedef struct {

    // configurable parameter
    LTE_SCHEDULER_TYPE enbSchType;    //MAC-LTE-ENB-SCHEDULER-TYPE
    LTE_SCHEDULER_TYPE ueSchType;    //MAC-LTE-UE-SCHEDULER-TYPE

    // random seed
    RandomSeed seed;

    // State Variable
    UInt64 ttiNumber; // TTI Number
    int preambleTransmissionCounter; // PREAMBLE_TRANSMISSION_COUNTER

    // Stat data
    LteMacStatData statData;

    // State
    MacLteState macState;

    // Timer
    LteMapMessage mapMacTimer;

    // Last propagation delay
    clocktype lastPropDelay;

#ifdef LTE_LIB_LOG
#ifdef LTE_LIB_VALIDATION_LOG
    std::map < LteRnti, lte::LogLteAverager >* avgEstimatedSinrDl;
    std::map < LteRnti, lte::LogLteAverager >* avgEstimatedSinrUl;
    std::map < LteRnti, lte::LogLteAverager >* avgNumAllocRbDl;
    std::map < LteRnti, lte::LogLteAverager >* avgNumAllocRbUl;
#endif
#endif
#ifdef PARALLEL
    LookaheadHandle lookaheadHandle;
#endif

    // HARQ
    MacLteDlHarqRxEntity* dlHarqRxEntity; // DL Rx
    MacLteUlHarqTxEntity* ulHarqTxEntity; // UL Tx
    std::map<LteRnti, MacLteDlHarqTxEntity*>* dlHarqTxEntity; // DL Tx
    std::map<LteRnti, MacLteUlHarqRxEntity*>* ulHarqRxEntity; // UL Rx


} LteMacData;

/// Buffer Status Reporting Info
typedef struct {
    LteRnti ueRnti; // UE's RNTI
    int bufferSizeLevel; // [0-63]
    UInt32 bufferSizeByte; // Sendable size in TX and/or RE-TX Buffer in RLC
} LteBsrInfo;

/// DL Information sending to each TTI.
typedef struct {
    UInt64 ttiNumber;
} DlTtiInfo;

/// Tx Information for MAC.
typedef struct {
    UInt32 numResourceBlocks;
} MacLteTxInfo;

////////////////////////////////////////////////////////////////////////////
// Function for MAC Layer
////////////////////////////////////////////////////////////////////////////
/// Initialize LTE MAC Layer.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param nodeInput  Pointer to node input.
///
void MacLteInit(Node* node,
                UInt32 interfaceIndex,
                const NodeInput* nodeInput);

/// Print stats and clear protocol variables.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
///
void MacLteFinalize(Node* node, UInt32 interfaceIndex);

/// Process Event.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param msg  Event message
///
void MacLteProcessEvent(Node* node, UInt32 interfaceIndex, Message* msg);

/// Process TTI Timer expire event.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param msg  Event message
///
void MacLteProcessTtiTimerExpired(
    Node* node, UInt32 interfaceIndex, Message* msg);

/// Start point for eNB scheduling.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param msg  Event message
///
void MacLteStartSchedulingAtENB(
    Node* node, UInt32 interfaceIndex, Message* msg);

/// Start point for UE scheduling.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param msg  Event message
///
void MacLteStartSchedulingAtUE(
    Node* node, UInt32 interfaceIndex, Message* msg);

/// Add Destination Info
///
/// \param node               : IN  Pointer to node.
/// \param interfaceIndex     : IN  Interface index
/// \param msg                : IN  MAC PDU Message Structure
/// \param oppositeRnti       : IN  Oposite RNTI
///
void MacLteAddDestinationInfo(Node* node,
                              Int32 interfaceIndex,
                              Message* msg,
                              const LteRnti& oppositeRnti);

/// Get BSR Level from size[byte].
///
///    + size             : BSR size[byte].
///
/// \return BSR Level
int MacLteGetBsrLevel(UInt32 size);

/// Set next TTI timer
///
/// \param node               : IN  Pointer to node.
/// \param interfaceIndex     : IN  Interface index
///
void MacLteSetNextTtiTimer(Node* node, int interfaceIndex);

/// Set timer message
///
/// \param node            : IN  Pointer to node.
/// \param interfaceIndex  : IN  Interface index
/// \param eventType       : IN  eventType in Message structure
/// \param delay           : IN  delay before timer expired
///
void MacLteSetTimerForMac(
    Node* node, int interfaceIndex, int eventType, clocktype delay);

/// Get timer message
///
/// \param node            : IN  Pointer to node.
/// \param interfaceIndex  : IN  Interface index
/// \param eventType       : IN  eventType in Message structure
///
/// \return Timer Message
Message* MacLteGetTimerForMac(
    Node* node, int interfaceIndex, int eventType);

/// Cancel timer
///
/// \param node            : IN  Pointer to node.
/// \param interfaceIndex  : IN  Interface index
/// \param eventType       : IN  eventType in Message structure
/// \param timerMsg        : IN  Pointer to timer message
///
void MacLteCancelTimerForMac(
    Node* node, int interfaceIndex, int eventType, Message* timerMsg);

/// Set state
///
/// \param node               : IN  Pointer to node.
/// \param interfaceIndex     : IN  Interface index
/// \param state              : IN  State
///
void MacLteSetState(Node* node, int interfaceIndex, MacLteState state);

////////////////////////////////////////////////////////////////////////////
// eNB/UE - API for Common
////////////////////////////////////////////////////////////////////////////
/// Get TTI Number
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
///
/// \return TTI Number
UInt64 MacLteGetTtiNumber(Node* node, int interfaceIndex);

/// Set TTI Number
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param ttiNumber  TTI Number
///
void MacLteSetTtiNumber(Node* node, int interfaceIndex, UInt64 ttiNumber);

/// Get number of subframe per TTI
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
///
/// \return Number of subframe per TTI
int MacLteGetNumSubframePerTti(
    Node* node, int interfaceIndex);

/// Get TTI length [nsec]
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
///
/// \return TTI length
clocktype MacLteGetTtiLength(
    Node* node, int interfaceIndex);

////////////////////////////////////////////////////////////////////////////
// eNB/UE - API for PHY
////////////////////////////////////////////////////////////////////////////
/// Receive a Transport Block from PHY Layer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param srcRnti  Source Node's RNTI
/// \param transportBlockMsg  one Transport Block
/// \param isErr             : BOOL     : TRUE  With ERROR
///
void MacLteReceiveTransportBlockFromPhy(
    Node* node, int interfaceIndex, LteRnti srcRnti,
    Message* transportBlockMsg);

////////////////////////////////////////////////////////////////////////////
// eNB - API for PHY
////////////////////////////////////////////////////////////////////////////
/// SAP for BSR Notification from eNB's PHY LAYER
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param bsrInfo  BSR Info Structure
///
void MacLteNotifyBsrFromPhy(
    Node* node, int interfaceIndex, const LteBsrInfo& bsrInfo);


////////////////////////////////////////////////////////////////////////////
// eNB/UE - API for RRC
////////////////////////////////////////////////////////////////////////////
/// SAP for Power ON Notification from RRC Layer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
///
void MacLteNotifyPowerOn(Node* node, int interfaceIndex);

/// SAP for Power ON Notification from RRC Layer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
///
void MacLteNotifyPowerOff(Node* node, int interfaceIndex);


void MacLteNotifyRlcReset(Node* node, int interfaceIndex, const LteRnti& oppositeRnti);

////////////////////////////////////////////////////////////////////////////
// eNB/UE - API for Scheduler
////////////////////////////////////////////////////////////////////////////
/// Check MAC PDU acutual size without padding size
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param dstRnti  Destination RNTI
/// \param bearerId  Bearer ID
///
int LteMacCheckMacPduSizeWithoutPadding(
    Node* node,
    int interfaceIndex,
    const LteRnti& dstRnti,
    const int bearerId,
    int size);

void MacLteClearInfo(Node* node, int interfaceIndex, const LteRnti& rnti);

// /**
// FUNCTION   :: MacLteGetNumberOfRbForDl
// LAYER      :: MAC
// PURPOSE    :: Get number of Resource Blocks for DL
// PARAMETERS ::
// + dlSchedulingResult : IN : LteDlSchedulingResultInfo* : DL scheduling
//                                                          result
// RETURN     :: int : number of RBs
// **/
int MacLteGetNumberOfRbForDl(
    LteDlSchedulingResultInfo* dlSchedulingResult);

// /**
// FUNCTION   :: MacLteIsFeedbackMessageInfo
// LAYER      :: MAC
// PURPOSE    :: Check the feedback message info.
// PARAMETERS ::
// + node     : Node*    : Pointer to node.
// + phyIndex : int      : Index of the PHY.
// + msg      : Message* : Pointer to message.
// RETURN     :: BOOL : TRUE found.
//                      FALSE not found.
// **/
BOOL MacLteIsDlFeedbackMessageInfo(Node* node, int phyIndex, Message* msg);

#endif // _LAYER2_LTE_MAC_H_
