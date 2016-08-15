#ifndef LAYER3_LTE_MEASUREMENT_H
#define LAYER3_LTE_MEASUREMENT_H

#include <map>
#include <list>
#include <string>
#include <sstream>

#include "clock.h"
#include "layer3_lte_filtering.h"

// parameter names in config file
#define RRC_LTE_STRING_MEAS_OBSERVING_EVENT_MASK_RSRP           \
    "RRC-LTE-MEAS-OBSERVING-EVENT-MASK-RSRP"
#define RRC_LTE_STRING_MEAS_OBSERVING_EVENT_MASK_RSRQ           \
    "RRC-LTE-MEAS-OBSERVING-EVENT-MASK-RSRQ"
#define RRC_LTE_STRING_MEAS_EVENT_A1_RSRP_TH                    \
    "RRC-LTE-MEAS-EVENT-A1-RSRP-TH"
#define RRC_LTE_STRING_MEAS_EVENT_A1_RSRP_HYS                   \
    "RRC-LTE-MEAS-EVENT-A1-RSRP-HYS"
#define RRC_LTE_STRING_MEAS_EVENT_A1_RSRQ_TH                    \
    "RRC-LTE-MEAS-EVENT-A1-RSRQ-TH"
#define RRC_LTE_STRING_MEAS_EVENT_A1_RSRQ_HYS                   \
    "RRC-LTE-MEAS-EVENT-A1-RSRQ-HYS"
#define RRC_LTE_STRING_MEAS_EVENT_A2_RSRP_TH                    \
    "RRC-LTE-MEAS-EVENT-A2-RSRP-TH"
#define RRC_LTE_STRING_MEAS_EVENT_A2_RSRP_HYS                   \
    "RRC-LTE-MEAS-EVENT-A2-RSRP-HYS"
#define RRC_LTE_STRING_MEAS_EVENT_A2_RSRQ_TH                    \
    "RRC-LTE-MEAS-EVENT-A2-RSRQ-TH"
#define RRC_LTE_STRING_MEAS_EVENT_A2_RSRQ_HYS                   \
    "RRC-LTE-MEAS-EVENT-A2-RSRQ-HYS"
#define RRC_LTE_STRING_MEAS_EVENT_A3_RSRP_OFF                   \
    "RRC-LTE-MEAS-EVENT-A3-RSRP-OFF"
#define RRC_LTE_STRING_MEAS_EVENT_A3_RSRP_HYS                   \
    "RRC-LTE-MEAS-EVENT-A3-RSRP-HYS"
#define RRC_LTE_STRING_MEAS_EVENT_A3_RSRQ_OFF                   \
    "RRC-LTE-MEAS-EVENT-A3-RSRQ-OFF"
#define RRC_LTE_STRING_MEAS_EVENT_A3_RSRQ_HYS                   \
    "RRC-LTE-MEAS-EVENT-A3-RSRQ-HYS"
#define RRC_LTE_STRING_MEAS_EVENT_A4_RSRP_TH                    \
    "RRC-LTE-MEAS-EVENT-A4-RSRP-TH"
#define RRC_LTE_STRING_MEAS_EVENT_A4_RSRP_HYS                   \
    "RRC-LTE-MEAS-EVENT-A4-RSRP-HYS"
#define RRC_LTE_STRING_MEAS_EVENT_A4_RSRQ_TH                    \
    "RRC-LTE-MEAS-EVENT-A4-RSRQ-TH"
#define RRC_LTE_STRING_MEAS_EVENT_A4_RSRQ_HYS                   \
    "RRC-LTE-MEAS-EVENT-A4-RSRQ-HYS"
#define RRC_LTE_STRING_MEAS_EVENT_A5_RSRP_TH1                   \
    "RRC-LTE-MEAS-EVENT-A5-RSRP-TH1"
#define RRC_LTE_STRING_MEAS_EVENT_A5_RSRP_TH2                   \
    "RRC-LTE-MEAS-EVENT-A5-RSRP-TH2"
#define RRC_LTE_STRING_MEAS_EVENT_A5_RSRP_HYS                   \
    "RRC-LTE-MEAS-EVENT-A5-RSRP-HYS"
#define RRC_LTE_STRING_MEAS_EVENT_A5_RSRQ_TH1                   \
    "RRC-LTE-MEAS-EVENT-A5-RSRQ-TH1"
#define RRC_LTE_STRING_MEAS_EVENT_A5_RSRQ_TH2                   \
    "RRC-LTE-MEAS-EVENT-A5-RSRQ-TH2"
#define RRC_LTE_STRING_MEAS_EVENT_A5_RSRQ_HYS                   \
    "RRC-LTE-MEAS-EVENT-A5-RSRQ-HYS"
#define RRC_LTE_STRING_MEAS_TIME_TO_TRIGGER                     \
    "RRC-LTE-MEAS-TIME-TO-TRIGGER"
#define RRC_LTE_STRING_MEAS_REPORT_INTERVAL                     \
    "RRC-LTE-MEAS-REPORT-INTERVAL"
#define RRC_LTE_STRING_MEAS_REPORT_AMOUNT                       \
    "RRC-LTE-MEAS-REPORT-AMOUNT"
#define RRC_LTE_STRING_MEAS_QUANTITY_CONFIG_FILTER_COEF_RSRP    \
    "RRC-LTE-MEAS-QUANTITY-CONFIG-FILTER-COEF-RSRP"
#define RRC_LTE_STRING_MEAS_QUANTITY_CONFIG_FILTER_COEF_RSRQ    \
    "RRC-LTE-MEAS-QUANTITY-CONFIG-FILTER-COEF-RSRQ"
#define RRC_LTE_STRING_MEAS_GAP_CONFIG_TYPE                     \
    "RRC-LTE-MEAS-GAP-CONFIG-TYPE"
#define RRC_LTE_STRING_MEAS_GAP_CONFIG_OFFSET                   \
    "RRC-LTE-MEAS-GAP-CONFIG-OFFSET"

// default values
#define LAYER3_LTE_MEAS_DEFAULT_FILTER_COEF_RSRP        4
#define LAYER3_LTE_MEAS_DEFAULT_FILTER_COEF_RSRQ        4
#define LAYER3_LTE_MEAS_DEFAULT_GAP_TYPE                MEAS_GAP_TYPE0
#define LAYER3_LTE_MEAS_DEFAULT_GAP_OFFSET              0
#define LAYER3_LTE_MEAS_DEFAULT_TIME_TO_TRIGGER         400000000    // 400MS
#define LAYER3_LTE_MEAS_DEFAULT_REPORT_INTERVAL         120000000    // 120MS
#define LAYER3_LTE_MEAS_DEFAULT_REPORT_AMOUNT           1
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A1_RSRP_TH        0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A1_RSRP_HYS       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A1_RSRQ_TH        0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A1_RSRQ_HYS       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A2_RSRP_TH        0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A2_RSRP_HYS       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A2_RSRQ_TH        0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A2_RSRQ_HYS       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A3_RSRP_OFF       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A3_RSRP_HYS       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A3_RSRQ_OFF       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A3_RSRQ_HYS       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A4_RSRP_TH        0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A4_RSRP_HYS       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A4_RSRQ_TH        0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A4_RSRQ_HYS       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A5_RSRP_TH1       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A5_RSRP_TH2       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A5_RSRP_HYS       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A5_RSRQ_TH1       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A5_RSRQ_TH2       0
#define LAYER3_LTE_MEAS_DEFAULT_EVENT_A5_RSRQ_HYS       0
#define LAYER3_LTE_MEAS_DEFAULT_INTRA_FREQ_INTERVAL     200
#define LAYER3_LTE_MEAS_DEFAULT_INTRA_FREQ_OFFSET       0

#define LAYER3_LTE_MEAS_EVENT_A_NUM                     5
#define LAYER3_LTE_MEAS_MEASUREMENT_TYPE_NUM            2
//#define LAYER3_LTE_MEAS_CAT_SETUP_MEAS_CONFIG         "SetupMeasConfig"
#define LAYER3_LTE_MEAS_CAT_MEASUREMENT                 "Measurement"
#define LAYER3_LTE_MEAS_CAT_HANDOVER                    "HandOver"


#define LAYER3_LTE_MEAS_MIN_TIME_TO_TRIGGER             0
#define LAYER3_LTE_MEAS_MIN_REPORT_INTERVAL             0
#define LAYER3_LTE_MEAS_MIN_REPORT_AMOUNT               1

#define LAYER3_LTE_MEAS_NEGATIVE_INFINITY_EVENT_DB_PARAMETER -1000.0
#define LAYER3_LTE_MEAS_INFINITY_EVENT_DB_PARAMETER 1000.0

#define LAYER3_LTE_MEAS_CHECK_EVENT_DB_PARAMETER(paramName, param) \
    if ((param < LAYER3_LTE_MEAS_NEGATIVE_INFINITY_EVENT_DB_PARAMETER) || \
        (LAYER3_LTE_MEAS_INFINITY_EVENT_DB_PARAMETER < param)) { \
        char errorStr[MAX_STRING_LENGTH]; \
        sprintf(errorStr, \
            "Measurement: Invalid %s : %f. " \
            "It should be in the range [%f, %f]", \
            paramName, \
            param, \
            LAYER3_LTE_MEAS_NEGATIVE_INFINITY_EVENT_DB_PARAMETER, \
            LAYER3_LTE_MEAS_INFINITY_EVENT_DB_PARAMETER); \
        ERROR_Assert(FALSE, errorStr); \
    }




//--------------------------------------------------------------------------
//  Measurement for Handover
//--------------------------------------------------------------------------
/// measurement event type
typedef enum enum_measurement_event_type
{
    MEAS_EVENT_A1,
    MEAS_EVENT_A2,
    MEAS_EVENT_A3,
    MEAS_EVENT_A4,
    MEAS_EVENT_A5,
    MEAS_EVENT_TYPE_NUM,
} MeasEventType;

/// measurement gap type
typedef enum enum_measurement_gap_type
{
    MEAS_GAP_TYPE0,        // gp0 (period = 40)
    MEAS_GAP_TYPE1,        // gp1 (period = 80)
    MEAS_GAP_TYPE_NUM,
} MeasGapType;

// DESCRIPTION:: measurement ID
typedef int MeasId;

// DESCRIPTION:: filter coefficient for measurement
typedef int MeasFilterCoefType;

// DESCRIPTION:: RSRP value type
typedef double MeasRSRPType;

// DESCRIPTION:: RSRQ value type
typedef double MeasRSRQType;

// DESCRIPTION:: Hysteresis value type
typedef double MeasHysteresis;

//// UNION:: MeasQuantityVar
//// DESCRIPTION:: measurement quantity
//typedef union union_measurement_quantity_variable
//{
//    MeasRSRPType rsrp_dBm;    // RSRP [dBm] value is stored
//    MeasRSRQType rsrq_dB;    // RSRQ [dB] value is stored
//} MeasQuantityVar;

/// event configuration variable(s)
typedef union union_measurement_event_variable
{
    // eventA1
    struct
    {
        double threshold;
    } eventA1;
    // eventA2
    struct
    {
        double threshold;
    } eventA2;
    // eventA3
    struct
    {
        double offset;
    } eventA3;
    // eventA4
    struct
    {
        double threshold;
    } eventA4;
    // eventA5
    struct
    {
        double threshold1;
        double threshold2;
    } eventA5;
        
    std::string ToString(MeasEventType eventType)
    {
        std::stringstream ss;
        switch (eventType)
        {
        case MEAS_EVENT_A1:
            ss << "{threshold=" << eventA1.threshold << "}";
            break;
        case MEAS_EVENT_A2:
            ss << "{threshold=" << eventA2.threshold << "}";
            break;
        case MEAS_EVENT_A3:
            ss << "{offset=" << eventA3.offset << "}";
            break;
        case MEAS_EVENT_A4:
            ss << "{threshold=" << eventA4.threshold << "}";
            break;
        case MEAS_EVENT_A5:
            ss << "{threshold1=" << eventA5.threshold1
                << ",threshold2=" << eventA5.threshold2
                << "}";
            break;
        case MEAS_EVENT_TYPE_NUM:
        default:
            ERROR_ReportErrorArgs("Invalid measurement event type : %d", (int)eventType);
            break;
        }
        return ss.str();
    }
} MeasEventVar;

/// measurement object
typedef struct struct_meas_object
{
    const int id;     // measurement object ID
    int channelNo;    // corresponding to the carrier frequency

    struct_meas_object(int _id, int _channelNo)
        : id(_id), channelNo(_channelNo)
    { }

    std::string ToString()
    {
        std::stringstream ss;
        ss << "{id=" << id << ",channelNo=" << channelNo << "}";
        return ss.str();
    }

    struct struct_meas_object& operator=(
        const struct struct_meas_object& /*another*/)
    {
        // For supressing warning C4512
        ERROR_Assert(FALSE,"struct_meas_object::operator= cannot be called");
        return *this;
    }
} MeasObject;

/// measurement event
typedef struct struct_meas_event
{
    MeasEventType eventType;            // event type
    LteMeasurementType quantityType;    // quantity type
    MeasEventVar eventVar;              // event configuration variable
    MeasHysteresis hysteresis;          // hysteresis
    clocktype timeToTrigger;            // time to trigger this event
    
    std::string ToString()
    {
        std::stringstream ss;
        ss << "{eventType=" << eventType
            << ",quantityType=" << quantityType
            << ",eventVar=" << eventVar.ToString(eventType)
            << ",hysteresis=" << hysteresis
            << ",timeToTrigger=" << timeToTrigger
            << "}";
        return ss.str();
    }
} MeasEvent;

/// report configuration
typedef struct struct_report_config
{
    int id;                                // report configuration ID
    MeasEvent measEvent;                   // measurment event
    //LteMeasurementType triggerQuantity;  // trigger quantity
    clocktype reportInterval;              // report interval
    int reportAmount;                      // report amount
    
    std::string ToString()
    {
        std::stringstream ss;
        ss << "{id=" << id
            << ",measEvent=" << measEvent.ToString()
            << ",reportInterval=" << reportInterval
            << ",reportAmount=" << reportAmount
            << "}";
        return ss.str();
    }
} ReportConfig;

/// quantity config
typedef struct struct_quantity_config
{
    MeasFilterCoefType filterCoefRSRP;    // filter coefficient for RSRP
    MeasFilterCoefType filterCoefRSRQ;    // filter coefficient for RSRQ

    std::string ToString()
    {
        std::stringstream ss;
        ss << "{filterCoefRSRP=" << filterCoefRSRP
            << ",filterCoefRSRQ=" << filterCoefRSRQ << "}";
        return ss.str();
    }
} QuantityConfig;

/// measurement gap configuration
typedef struct struct_meas_gap_config
{
    MeasGapType type;    // measurement gap type
    int gapOffset;    

    std::string ToString()
    {
        std::stringstream ss;
        ss << "{type=" << type << ",gapOffset=" << gapOffset << "}";
        return ss.str();
    }        // 
} MeasGapConfig;

/// measurement information
typedef struct struct_meas_info
{
    MeasId id;                    // measurment ID
    MeasObject* measObject;       // measurement object
    ReportConfig* reportConfig;   // report configuration
    
    std::string ToString()
    {
        std::stringstream ss;
        ss << "{id=" << id
            << ",measObject(id)=" << measObject->id
            << ",reportConfig(id)=" << reportConfig->id
            << "}";
        return ss.str();
    }
} MeasInfo;

typedef std::list<MeasInfo> ListMeasInfo; 
typedef std::list<MeasObject> ListMeasObject;
typedef std::list<ReportConfig> ListReportConfig;

/// variables of measurment configuration
typedef struct struct_var_meas_config
{ 
    ListMeasInfo measInfoList;            // list of measurement information
    ListMeasObject measObjectList;        // list of measurement objects
    ListReportConfig reportConfigList;    // list of report configurations
    QuantityConfig quantityConfig;        // quantity configuration
    MeasGapConfig measGapConfig;          // measuremnt gap configuration

    // member functions
    void Clear()
    {
        measInfoList.clear();
        measObjectList.clear();
        reportConfigList.clear();
        quantityConfig.filterCoefRSRP   =
            LAYER3_LTE_MEAS_DEFAULT_FILTER_COEF_RSRP;
        quantityConfig.filterCoefRSRQ   =
            LAYER3_LTE_MEAS_DEFAULT_FILTER_COEF_RSRQ;
        measGapConfig.type              =
            LAYER3_LTE_MEAS_DEFAULT_GAP_TYPE;
        measGapConfig.gapOffset         =
            LAYER3_LTE_MEAS_DEFAULT_GAP_OFFSET;
    }
} VarMeasConfig;

typedef enum enum_measurement_state
{
    MEAS_STATE_NONE,
    MEAS_STATE_ENTERING,
    MEAS_STATE_REPORT,
    MEAS_STATE_LEAVING,
    MEAS_STATE_NUM
} MeasState;

typedef struct struct_measurement_event_condition_information
{
    MeasState    state;
    clocktype    changedTimeStamp;

    struct_measurement_event_condition_information()
        : state(MEAS_STATE_NONE),
        changedTimeStamp(-1)
    {
    }

    std::string ToString()
    {
        std::stringstream ss;
        ss << "{state=" << state
            << ",changedTimeStamp=" << changedTimeStamp
            << "}";
        return ss.str();
    }
} MeasEventConditionInfo;

typedef std::map<LteRnti, MeasEventConditionInfo>
    MapRntiMeasEventConditionInfo;

typedef std::map<MeasId, MapRntiMeasEventConditionInfo>
    MeasEventConditionTable;


// report data structure

typedef struct struct_meas_result_info
{
    LteRnti rnti;
    MeasRSRPType rsrpResult;
    MeasRSRQType rsrqResult;
} MeasResultInfo;

typedef std::list<MeasResultInfo> ListMeasResultInfo;

typedef struct struct_meas_results_serialized
{
    MeasId measId;
    MeasResultInfo measResultServCell;
    int neighCellsNum;
    MeasResultInfo measResultNeighCells[1];
} MeasResultsSerialized;

typedef struct struct_meas_results
{
    MeasId measId;
    MeasResultInfo measResultServCell;
    ListMeasResultInfo measResultNeighCells;

    struct_meas_results()
        : measId(0)
    { }

    // deserialize
    struct_meas_results(char* store)
    {
        MeasResultsSerialized* input = (MeasResultsSerialized*)store;
        measId = input->measId;
        measResultServCell = input->measResultServCell;
        for (int neighIndex = 0; neighIndex <
            input->neighCellsNum; neighIndex++)
        {
            measResultNeighCells.push_back(
                input->measResultNeighCells[neighIndex]);
        }
    }

    void Serialize(char* store)
    {
        MeasResultsSerialized* result = (MeasResultsSerialized*)store;
        result->measId = measId;
        result->measResultServCell = measResultServCell;
        result->neighCellsNum = (int) measResultNeighCells.size();
        int neighIndex = 0;
        for (ListMeasResultInfo::iterator it = measResultNeighCells.begin();
            it != measResultNeighCells.end(); it++)
        {
            result->measResultNeighCells[neighIndex] = *it;
            neighIndex++;
        }
    }

    int Size()
    {
        return (int) (sizeof(MeasResultsSerialized) +
            measResultNeighCells.size() * sizeof(MeasResultInfo));
    }

} MeasResults;

typedef struct struct_measurement_report
{
    MeasResults measResults;
    struct_measurement_report()
    {}
    struct_measurement_report(MeasResults& _measResults)
        : measResults(_measResults)
    {}

    std::string ToString()
    {
        std::stringstream ss;
        ss << "measId," << measResults.measId
            << ",serving,[" << measResults.measResultServCell.rnti.nodeId
            << " " << measResults.measResultServCell.rnti.interfaceIndex
            << "]";
        ss << ",neighbor";
        for (ListMeasResultInfo::iterator it = 
            measResults.measResultNeighCells.begin();
            it != measResults.measResultNeighCells.end(); it++)
        {
            ss << ",[" << it->rnti.nodeId
            << " " << it->rnti.interfaceIndex
            << "]";
        }
        return ss.str();
    }
} MeasurementReport;

typedef std::set<LteRnti> CellsTriggeredList;

typedef struct struct_var_meas_report
{
    MeasId measId;
    CellsTriggeredList cellsTriggeredList;
    int numberOfReportsSent;
} VarMeasReport;

typedef std::list<VarMeasReport> VarMeasReportList;

typedef std::map<MeasId, Message*> MapMeasPeriodicalTimer;

// used as MeasReport container
typedef struct struct_meas_report_container
{
    LteRnti dstEbnRnti;
    LteRnti srcUeRnti;
    int reportNum;
    char data[1];
} MeasReportContainer;


// functions

// void Layer3LteMeasuClearVarMeasConfig(VarMeasConfig* varMeasConfig);

/// Process Event.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param msg  Event message
///
void Layer3LteMeasProcessEvent(
    Node* node, UInt32 interfaceIndex, Message* msg);

/// cancel all the measurement timers
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param msg  Event message
///
void Layer3LteMeasCancelAllTimer(
    Node* node, UInt32 interfaceIndex);

/// get measurement event variable
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param nodeInput  Pointer to node input.
/// \param measEventType  Measurement Event Type
/// \param measType  Measurement Type
/// \param eventVar  Pointer to return value
/// \param hysteresis  Pointer to return value
///
void Layer3LteMeasGetMeasEventVar(Node* node,
                                  int interfaceIndex,
                                  const NodeInput* nodeInput,
                                  MeasEventType measEventType,
                                  LteMeasurementType measType,
                                  MeasEventVar* eventVar,
                                  MeasHysteresis* hysteresis);

/// Check Event To Report
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param measInfo  Measurement information
///
void Layer3LteMeasCheckEventToReport(Node* node,
                                     int interfaceIndex,
                                     MeasInfo* measInfo);

/// Send Measurement Report
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param measId  Measurement Id
///
void Layer3LteMeasSendMeasReport(Node* node,
                                 int interfaceIndex,
                                 MeasId measId);

/// process report
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param measInfo  Measurement information
/// \param cellsTriggeredList  cells triggered list
///
void Layer3LteMeasProcessReport(Node* node,
                                int interfaceIndex,
                                MeasInfo* measInfo,
                                CellsTriggeredList& relatedCells);

/// generate measurement report
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param measId  Measurement Id
/// \param cellsTriggeredList  cells triggered list
/// \param newReport  Generated report
///
void Layer3LteMeasGenMeasurementReport(Node* node,
                                   int interfaceIndex,
                                   MeasInfo* measInfo,
                                   CellsTriggeredList* cellsTriggeredList,
                                   MeasurementReport* newReport);

/// get corresponding condition entry
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param measInfo  Measurement information
/// \param cond  corresponding entry
/// \param createdNewEntry  new entry was created or not
///
void Layer3LteMeasGetEventConditionInfo(
    Node* node,
   int interfaceIndex,
   const LteRnti& keyRnti,
   MeasInfo* measInfo,
   MeasEventConditionInfo** cond,
   BOOL* createdNewEntry);

/// update entry of condition table
///
/// \param node  Pointer to node.
/// \param cond  corresponding entry
/// \param enterCondition  meet enter condition or not
/// \param leaveCondition  meet leave condition or not
/// \param timeToTrigger  Time to trigger value
///
void Layer3LteMeasUpdateConditionInfoEntry(
    Node* node,
   MeasEventConditionInfo* cond,
   BOOL enterCondition,
   BOOL leaveCondition,
   clocktype timeToTrigger);

/// update condition table
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param measInfo  Measurement information
///
void Layer3LteMeasUpdateConditionTable(Node* node,
                                       int interfaceIndex,
                                       MeasInfo* measInfo);

/// Stop Periodical Timer if neccesary
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param measId  Measurement Id
///
void Layer3LteMeasStopPeriodicalTimer(Node* node,
                                      int interfaceIndex,
                                      MeasId measId);

/// Set periodical report timer message
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param delay  delay before timer expired
/// \param measId  measurement id
///
void Layer3LteMeasSetPeriodicalReportTimer(Node* node,
                                           int interfaceIndex,
                                           clocktype delay,
                                           MeasId measId);


// utilities

/// get report info
///
/// \param measReportList  report list
/// \param measId  Measurement ID
///
/// \return iterator of retrieved info
VarMeasReportList::iterator Layer3LteMeasGetReportInfo(
    VarMeasReportList& measReportList,
    MeasId measId);

/// compare CellsTriggeredLists
///
/// \param measReportList  report list
/// \param list1  listNew
/// \param list2  listOld
///
/// \return TRUE
/// otherwise                     : FALSE
BOOL Layer3LteMeasContainedInOldList(
    CellsTriggeredList& listNew, CellsTriggeredList& listOld);

/// get measurement information
///
/// \param measInfoList  measurement information list
/// \param measId  Measurement ID
///
/// \return iterator of retrieved info
ListMeasInfo::iterator Layer3LteMeasGetMeasInfo(ListMeasInfo& measInfoList,
                                                MeasId measId);

/// get measurement information
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
///
void Layer3LteMeasResetMeasurementValues(Node* node, int interfaceIndex);

#endif    // LAYER3_LTE_MEASUREMENT_H

