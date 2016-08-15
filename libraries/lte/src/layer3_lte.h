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

#ifndef _LAYER3_LTE_H_
#define _LAYER3_LTE_H_

#include <map>

#include "lte_common.h"
#include "layer2_lte_mac.h"
#include "layer3_lte_measurement.h"
#include "layer2_lte_rlc.h"
#include "layer2_lte_pdcp.h"
#include "lte_rrc_config.h"

//--------------------------------------------------------------------------
// Define
//--------------------------------------------------------------------------
#define LTE_DEFAULT_BEARER_ID (0)

#define RRC_LTE_DEFAULT_WAIT_RRC_CONNECTED_TIME (10*MILLI_SECOND)
#define RRC_LTE_DEFAULT_WAIT_RRC_CONNECTED_RECONF_TIME (10*MILLI_SECOND)
#define RRC_LTE_DEFAULT_RELOC_PREP_TIME                  ( 200*MILLI_SECOND)
#define RRC_LTE_DEFAULT_X2_RELOC_OVERALL_TIME            (1000*MILLI_SECOND)
#define RRC_LTE_DEFAULT_X2_WAIT_SN_STATUS_TRANSFER_TIME  ( 500*MILLI_SECOND)
#define RRC_LTE_DEFAULT_WAIT_ATTCH_UE_BY_HO_TIME         ( 500*MILLI_SECOND)
#define RRC_LTE_DEFAULT_X2_WAIT_END_MARKER_TIME          ( 200*MILLI_SECOND)
#define RRC_LTE_DEFAULT_S1_WAIT_PATH_SWITCH_REQ_ACK_TIME ( 200*MILLI_SECOND)
#define RRC_LTE_DEFAULT_MEAS_FILTER_COEFFICIENT (40.0)
#define RRC_LTE_DEFAULT_HO_IGNORED_TIME (0*SECOND)

#define RRC_LTE_STRING_WAIT_RRC_CONNECTED_TIME \
    "RRC-LTE-WAIT-RRC-CONNECTED-TIME"
#define RRC_LTE_STRING_WAIT_RRC_CONNECTED_RECONF_TIME \
    "RRC-LTE-WAIT-RRC-CONNECTED-RECONF-TIME"
#define RRC_LTE_STRING_MEAS_FILTER_COEFFICIENT \
    "RRC-LTE-MEAS-FILTER-COEFFICIENT"
#define RRC_LTE_STRING_IGNORE_HO_DECISION_TIME \
    "RRC-LTE-IGNORE-HO-DECISION-TIME"

#ifdef LTE_LIB_USE_POWER_TIMER
#define RRC_LTE_STRING_NUM_POWER_ON "RRC-LTE-NUM-POWER-ON"
#define RRC_LTE_STRING_NUM_POWER_OFF "RRC-LTE-NUM-POWER-OFF"
#define RRC_LTE_STRING_POWER_ON_TIME "RRC-LTE-POWER-ON-TIME"
#define RRC_LTE_STRING_POWER_OFF_TIME "RRC-LTE-POWER-OFF-TIME"
#endif

#define RRC_LTE_MAX_MEAS_FILTER_COEFFICIENT (100.0)

#define LTE_LIB_STATION_TYPE_GUARD(node, interfaceIndex, type, typeString) \
    ERROR_Assert(LteLayer2GetStationType(node, interfaceIndex) == type, \
        "This function should be called from only " typeString ".")

//--------------------------------------------------------------------------
// Enumulation
//--------------------------------------------------------------------------
typedef enum {
    LAYER3_LTE_POWER_OFF,
    LAYER3_LTE_POWER_ON,
    LAYER3_LTE_RRC_IDLE,
    LAYER3_LTE_RRC_CONNECTED,
    LAYER3_LTE_STATUS_NUM
} Layer3LteState;

// -------------------------------------------------------------------------
// Basic data structs such as parameters, statistics, protocol data struc
//--------------------------------------------------------------------------

/// Statistics Data
typedef struct {
    int numRrcConnectionEstablishment;
    int averageRetryRrcConnectionEstablishment;
    clocktype averageTimeOfRrcConnectionEstablishment;
} LteLayer3StatData;

/// Statistics Data for establishment
typedef struct {
    clocktype lastStartTime;
    clocktype numPowerOn;
    clocktype numPowerOff;
} LteLayer3EstablishmentStat;

typedef struct struct_rrc_connection_reconfiguration{
    LteRrcConfig rrcConfig;
    struct_rrc_connection_reconfiguration()
    {}
    struct_rrc_connection_reconfiguration(
        const struct_rrc_connection_reconfiguration& other)
    {
        rrcConfig = other.rrcConfig;    // just memberwise copy
    }
} RrcConnectionReconfiguration;

typedef enum {
    LAYER3_LTE_CONNECTION_WAITING,
    LAYER3_LTE_CONNECTION_CONNECTED,
    LAYER3_LTE_CONNECTION_HANDOVER,
    LAYER3_LTE_CONNECTION_STATUS_NUM
} Layer3LteConnectionState;

typedef struct struct_lte_radio_bearer
{
    LtePdcpEntity pdcpEntity;
    LteRlcEntity rlcEntity;

} LteRadioBearer;

typedef std::map < int, LteRadioBearer > MapLteRadioBearer;
typedef std::pair < int, LteRadioBearer > PairLteRadioBearer;

/// Connected Information
typedef struct struct_connected_info
{
    LteBsrInfo bsrInfo; // using only eNB
    MapLteRadioBearer radioBearers; // Radio Bearers
    clocktype connectedTime; // connectedTime
    BOOL isSchedulable;
} LteConnectedInfo;

typedef std::map < LteRnti, LteConnectedInfo > MapLteConnectedInfo;
typedef std::pair < LteRnti, LteConnectedInfo > PairLteConnectedInfo;

typedef struct struct_connection_info_lte
{
    Layer3LteConnectionState state;
    LteConnectedInfo connectedInfo;
    LteMapMessage mapLayer3Timer;

    HandoverParticipator hoParticipator;    // used ony for handover
#ifdef LTE_LIB_HO_VALIDATION
    UInt64 tempRecvByte;
#endif

    struct_connection_info_lte(Layer3LteConnectionState _state)
        : state(_state)
#ifdef LTE_LIB_HO_VALIDATION
        , tempRecvByte(0)
#endif
    {}
} LteConnectionInfo;

typedef std::map < LteRnti, LteConnectionInfo > MapLteConnectionInfo;
typedef std::pair < LteRnti, LteConnectionInfo > PairLteConnectionInfo;



typedef struct struct_rrc_connection_reconfiguration_including_mobCtrlInfo{
    HandoverParticipator hoParticipator;
    RrcConnectionReconfiguration reconf;
} RrcConnReconfInclMoblityControlInfomation;

typedef std::map<LteRnti, RrcConnReconfInclMoblityControlInfomation>
    RrcConnReconfInclMoblityControlInfomationMap;

typedef struct
    struct_rrc_connection_reconfiguration_including_mobCtrlInfo_container{
    int num;
    RrcConnReconfInclMoblityControlInfomation reconfList[1];
} RrcConnReconfInclMoblityControlInfomationContainer;

/// Data structure of Lte model
typedef struct struct_layer3_lte_str
{
    Layer3LteState layer3LteState;

    MapLteConnectionInfo connectionInfoMap;

    LteLayer3StatData statData; // for statistics
    LteLayer3EstablishmentStat establishmentStatData; // for statistics
    clocktype waitRrcConnectedTime; // RRC-LTE-WAIT-RRC-CONNECTED-TIME
    clocktype waitRrcConnectedReconfTime; // RRC-LTE-WAIT-RRC-CONNECTED-RECONF-TIME
    clocktype ignoreHoDecisionTime; // RRC-LTE-IGNORE-HO-DECISION-TIME

    VarMeasConfig varMeasConfig;                        // measurement configuration
    MeasEventConditionTable measEventConditionTable;    // to manage event condition
    VarMeasReportList varMeasReportList;                // report information
    MapMeasPeriodicalTimer mapMeasPeriodicalTimer;      // map of MeasId and periodical timer

} Layer3DataLte;

//--------------------------------------------------------------------------
//  Utility functions
//--------------------------------------------------------------------------
/// Get Connection Info
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param rnti  RNTI
///
/// \return Connection Info
LteConnectionInfo* Layer3LteGetConnectionInfo(
    Node* node, int interfaceIndex, const LteRnti& rnti);

/// Get connection List
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param store  result
/// \param state  state
///
void Layer3LteGetConnectionList(
    Node* node, int interfaceIndex, ListLteRnti* store,
    Layer3LteConnectionState state);

/// Get connected List sorted by connected time
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param store  result
///
void Layer3LteGetSchedulableListSortedByConnectedTime(
    Node* node, int interfaceIndex, ListLteRnti* store);

/// get connection state
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param rnti  RNTI
///
/// \return state
Layer3LteConnectionState Layer3LteGetConnectionState(
    Node* node, int interfaceIndex, const LteRnti& rnti);

/// set connection state
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param rnti  RNTI
///
void Layer3LteSetConnectionState(
    Node* node, int interfaceIndex, const LteRnti& rnti,
    Layer3LteConnectionState state);

/// get participators of H.O.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param rnti  RNTI
///
/// \return participators of H.O.
HandoverParticipator Layer3LteGetHandoverParticipator(
    Node* node, int interfaceIndex, const LteRnti& rnti);

/// set participators of H.O.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param rnti  RNTI
/// \param hoParticipator  participators of H.O.
///
void Layer3LteSetHandoverParticipator(
    Node* node, int interfaceIndex, const LteRnti& rnti,
    const HandoverParticipator& hoParticipator);

/// change connection state
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param rnti  RNTI
/// \param newState  new state
///
void Layer3LteChangeConnectionState(
    Node* node, int interfaceIndex, const LteRnti& rnti,
    Layer3LteConnectionState newState,
    const HandoverParticipator& hoParticipator = 
    HandoverParticipator());



/// get handover info
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param rnti  RNTI
///
/// \return info
LteConnectionInfo* Layer3LteGetHandoverInfo(
    Node* node, int interfaceIndex, const LteRnti& rnti = LTE_INVALID_RNTI);



/// Get Connected eNB
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
///
/// \return Connected eNB
const LteRnti Layer3LteGetConnectedEnb(Node* node, int interfaceIndex);

/// Get waiting eNB
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
///
/// \return waiting eNB
const LteRnti Layer3LteGetWaitingEnb(Node* node, int interfaceIndex);

/// Get waiting eNB
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
///
/// \return waiting eNB
const LteRnti Layer3LteGetRandomAccessingEnb(Node* node, int interfaceIndex);

/// get timer message
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param rnti  RNTI
/// \param eventType  eventType in Message structure
///
/// \return timer message
Message* Layer3LteGetTimerForRrc(
    Node* node, int interfaceIndex, const LteRnti& rnti, int eventType);

/// add timer message
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param rnti  RNTI
/// \param eventType  eventType in Message structure
/// \param delay  delay before timer expired
/// \param timerMsg  timer added
///
void Layer3LteAddTimerForRrc(
    Node* node, int interfaceIndex, const LteRnti& rnti, int eventType,
    Message* timerMsg);

/// delete timer message
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param rnti  RNTI
/// \param eventType  eventType in Message structure
///
void Layer3LteDeleteTimerForRrc(
    Node* node, int interfaceIndex, const LteRnti& rnti, int eventType);

/// Set timer message
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param rnti  RNTI
/// \param eventType  eventType in Message structure
/// \param delay  delay before timer expired
///
void Layer3LteSetTimerForRrc(
    Node* node, int interfaceIndex, const LteRnti& rnti, int eventType,
    clocktype delay);

/// Cancel timer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param rnti  RNTI
/// \param eventType  eventType in Message structure
///
void Layer3LteCancelTimerForRrc(
    Node* node, int interfaceIndex, const LteRnti& rnti, int eventType);

/// Cancel all the timers
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param rnti  RNTI
///
void Layer3LteCancelAllTimerForRrc(
    Node* node, int interfaceIndex, const LteRnti& rnti);

/// Check timer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param rnti  RNTI
/// \param eventType  eventType in Message structure
///
/// \return Not Running
BOOL Layer3LteCheckTimerForRrc(
    Node* node, int interfaceIndex, const LteRnti& rnti, int eventType);

////////////////////////////////////////////////////////////////////////////
// eNB/UE - API for Common
////////////////////////////////////////////////////////////////////////////
/// Process Event.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param msg  Event message
///
void Layer3LteProcessEvent(Node* node, UInt32 interfaceIndex, Message* msg);

/// Add Connected RNTI to List & Create Radio Bearer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param rnti  RNTI
/// \param state  initial state
///
/// \return not exist
void Layer3LteAddConnectionInfo(
    Node* node, int interfaceIndex, const LteRnti& rnti,
    Layer3LteConnectionState state);

/// Remove Connected info
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param rnti  RNTI
///
/// \return not exist
BOOL Layer3LteRemoveConnectionInfo(
    Node* node, int interfaceIndex, const LteRnti& rnti);

/// Power ON
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
///
void Layer3LtePowerOn(Node* node, int interfaceIndex);

/// Power OFF
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
///
void Layer3LtePowerOff(Node* node, int interfaceIndex);



/// Restart to search cells for re-attach
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
///
void 
Layer3LteRestart(Node* node, int interfaceIndex);



/// Add Route
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param oppositeRnti  Oppsite RNTI
///
// NOTE       :: this function is not supported at Phase 1
void Layer3LteAddRoute(
    Node* node, int interfaceIndex, const LteRnti& oppositeRnti);



/// Delete Route
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param oppositeRnti  Oppsite RNTI
///
void Layer3LteDeleteRoute(
    Node* node, int interfaceIndex, const LteRnti& oppositeRnti);



////////////////////////////////////////////////////////////////////////////
// eNB - API for Common
////////////////////////////////////////////////////////////////////////////
/// Init RRC
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param nodeInput  Pointer to node input.
///
void Layer3LteInitialize(
    Node* node, int interfaceIndex, const NodeInput* nodeInput);

/// Finalize RRC
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
///
void Layer3LteFinalize(
    Node* node, int interfaceIndex);

/// SAP for BSR Notification from eNB's MAC LAYER
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param bsrInfo  BSR Info Structure
///
void Layer3LteNotifyBsrFromMac(
    Node* node, int interfaceIndex, const LteBsrInfo& bsrInfo);

////////////////////////////////////////////////////////////////////////////
// UE - API for Common
////////////////////////////////////////////////////////////////////////////
/// Process RRC_CONNECTED Timer expire event.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
///
void Layer3LteProcessRrcConnectedTimerExpired(
    Node* node, UInt32 interfaceIndex);



/// Process RRC_CONNECTED_RECONF Timer expire event.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param connectedInfo  connectedInfo used before H.O.
///
void Layer3LteProcessRrcConnectedReconfTimerExpired(
    Node* node, UInt32 interfaceIndex);

/// Detach UE
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param ueRnti  RNTI of detached UE
///
void Layer3LteDetachUE(
    Node* node, int interfaceIndex, const LteRnti& oppositeRnti);



////////////////////////////////////////////////////////////////////////////
// eNB/UE - API for PHY
////////////////////////////////////////////////////////////////////////////
/// Process of radio link failure
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param ueRnti  RNTI lost
///
void Layer3LteProcessRadioLinkFailure(
    Node* node, int interfaceIndex, LteRnti oppositeRnti);

/// Notification of radio link failure
/// 
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param ueRnti  RNTI for which radio
void Layer3LteNotifyRadioLinkFailure(
    Node* node, int interfaceIndex, LteRnti oppositeRnti);


/// Notify lost detection
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
///
void Layer3LteNotifyFailureRAToTargetEnbOnHandover(
    Node* node, int interfaceIndex);



/// Set Up Measurement Configuration
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param nodeInput  Pointer to node input.
///
void Layer3LteSetupMeasConfig(
    Node* node, int interfaceIndex, const NodeInput* nodeInput);

/// Setup Event Condition Observing Information
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
///
void Layer3LteSetupMeasEventConditionTable(Node *node,
                    UInt32 interfaceIndex);
/// initialize measurement
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
///
void Layer3LteInitMeasurement(
    Node* node, UInt32 interfaceIndex);

/// IF for PHY to notify about measurement result updated
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
///
void Layer3LteIFHPNotifyL3FilteringUpdated(
                    Node *node,
                    UInt32 interfaceIndex);

/// IF for PHY to notify Measurement Repot Received to RRC
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param ueRnti  the source of the report
/// \param measurementReport: std:  report
///
void Layer3LteIFHPNotifyMeasurementReportReceived(
                    Node *node,
                    UInt32 interfaceIndex,
                    const LteRnti& ueRnti,
                    std::list<MeasurementReport>* measurementReport);
/// handover decision
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param ueRnti  the source of the report
/// \param measurementReport: std:  report
/// \param targetRnti  target eNB (if INVALID_RNTI
///    is set, doesn't hand over)
///
void Layer3LteHandOverDecision(
                    Node *node,
                    UInt32 interfaceIndex,
                    const LteRnti& ueRnti,
                    std::list<MeasurementReport>* measurementReport,
                    LteRnti* targetRnti);
/// admission control
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///
/// \return result
BOOL Layer3LteAdmissionControl(
                    Node *node,
                    UInt32 interfaceIndex,
                    const HandoverParticipator& hoParticipator);
/// process after receive handover request
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///
void Layer3LteReceiveHoReq(
                    Node *node,
                    UInt32 interfaceIndex,
                    const HandoverParticipator& hoParticipator);
/// process after receive handover request ack
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
/// \param reconf  reconfiguration
///
void Layer3LteReceiveHoReqAck(
                    Node *node,
                    UInt32 interfaceIndex,
                    const HandoverParticipator& hoParticipator,
                    const RrcConnectionReconfiguration& reconf);

/// send SnStatusTransfer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///
void Layer3LteSendSnStatusTransfer(
    Node *node,
    UInt32 interfaceIndex,
    const HandoverParticipator& hoParticipator);

/// process after receive SnStatusTransfer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///    + snStatusTransferItem : std::map<int, PdcpLteSnStatusTransferItem>& : 
///
void Layer3LteReceiveSnStatusTransfer(
    Node *node,
    UInt32 interfaceIndex,
    const HandoverParticipator& hoParticipator,
    std::map<int, PdcpLteSnStatusTransferItem>& snStatusTransferItem);

/// send reconfiguration to handovering UE
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
/// \param reconf  reconfiguration
///
void Layer3LteSendRrcConnReconfInclMobilityControlInfomation(
                    Node *node,
                    UInt32 interfaceIndex,
                    const HandoverParticipator& hoParticipator,
                    const RrcConnectionReconfiguration& reconf);
/// notify reconfiguration received
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
/// \param reconf  reconfiguration
///
void Layer3LteNotifyRrcConnReconf(
                    Node *node,
                    UInt32 interfaceIndex,
                    const HandoverParticipator& hoParticipator,
                    const RrcConnectionReconfiguration& reconf);
/// clear information for handover execution
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///
void Layer3LtePrepareForHandoverExecution(Node* node, int interfaceIndex,
                    const HandoverParticipator& hoParticipator);

/// clear information for handover execution
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///
void Layer3LtePrepareDataForwarding(
    Node* node, int interfaceIndex,
    const HandoverParticipator& hoParticipator);

/// start data forwarding process
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param hoParticipator  participators of H.O.
///
void Layer3LteStartDataForwarding(Node* node, int interfaceIndex,
                              const HandoverParticipator& hoParticipator);

/// send forwarding data list
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param rnti  UE's RNTI
/// \param bearerId  bearer ID of the message list
/// \param forwardingMsg  : std:  message list
///
void Layer3LteSendForwardingDataList(Node* node, int interfaceIndex,
    const LteRnti& ueRnti, int bearerId, std::list<Message*>* forwardingMsg);

/// process after receive DataForwardin
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
/// \param forwardingMsg  message
///
void Layer3LteReceiveDataForwarding(
    Node* node,
    int interfaceIndex,
    const HandoverParticipator& hoParticipator,
    int bearerId,
    Message* forwardingMsg);

/// receive path switch request ack
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
/// \param result  path switch result
///
void Layer3LteReceivePathSwitchRequestAck(
    Node* node,
    int interfaceIndex,
    const HandoverParticipator& hoParticipator,
    BOOL result);

/// receive end marker
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///
void Layer3LteReceiveEndMarker(
    Node* node,
    int interfaceIndex,
    const HandoverParticipator& hoParticipator);

/// receive ue context release
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///
void Layer3LteReceiveUeContextRelease(
    Node* node,
    int interfaceIndex,
    const HandoverParticipator& hoParticipator);

/// release ue context
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///
void Layer3LteReleaseUeContext(
    Node* node,
    int interfaceIndex,
    const HandoverParticipator& hoParticipator);

/// receive ho preparation failure
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///
void Layer3LteReceiveHoPreparationFailure(
    Node* node,
    int interfaceIndex,
    const HandoverParticipator& hoParticipator);



#endif // _LAYER3_LTE_H_

