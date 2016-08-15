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

#ifndef MAC_DOT16_QOS_H
#define MAC_DOT16_QOS_H

//
// This is the header file of the implementation of QOS of IEEE 802.16 MAC
//

/// Initial size of the array for keeping the list
/// of established service class records
#define DOT16_QOS_NUM_SERVICE_CLASS_RECORD    5

/// Initial size of the array for keeping the list
/// of established service class records
#define DOT16_QOS_NUM_APP_SERVICE_RECORD    20

/// Data structure to hold information for aggregate table
typedef struct struct_dot16_qos_table_entry {
    int totalBytes;
    float usedTput;
    clocktype avgExpDelay;
} MacDot16QosTableEntry;

/// Table containing information from aggregation of flows
typedef struct struct_dot16_qos_agg_table {
    MacDot16QosTableEntry usg;
    MacDot16QosTableEntry rtps;
    MacDot16QosTableEntry nrtps;
    MacDot16QosTableEntry be;
    float availableTput;
} MacDot16QosAggTable;

/// Data structure contains the QOS service and related info
typedef struct mac_dot16_qos_service_record_str
{
    MacDot16ServiceClass classType;     // indicate service class type
    unsigned int appServiceId;          // to uniquely and easily identify
                                        // a app service record

    MacDot16QoSParameter* parameters; // pointer to the actual QOS service
                                      // parameters structure

    AppType appType;        // Application protocol types

    clocktype invalidTime;  // The time stamp when the record is marked
                            // as invalidated
} MacDot16QosAppServiceRecord;

/// Data structure contains the defined QOS service class info
typedef struct mac_dot16_qos_service_class_record_str
{
    unsigned int serviceClassId;                    // record class id
    char stationClassName[20];                      // station class name
    MacDot16StationClass stationClass;              // indicate stations
                                                    // service class type
    MacDot16QoSParameter* parameters;               // pointer to the actual
                                                    // QOS service
                                                    // parameters structure
} MacDot16QosServiceClassRecord;

/// Data structure of Dot16 QOS
typedef struct struct_mac_dot16_qos_str
{
    int appServiceIds; // counter used to assign ID to app services

    // list of possable service classes
    MacDot16QosAppServiceRecord** appServiceRecords;
    int numAppServiceRecords;
    int allocAppServiceRecords;

    int serviceClassIds; // counter used to assign ID to service class

    // list of possable service classes
    MacDot16QosServiceClassRecord** serviceClassRecords;
    int numServiceClassRecords;
    int allocServiceClassRecords;

    MacDot16QosAggTable* qosAggTable;

    MacDot16StationClass stationClass;
    char* stationClassName[20];

} MacDot16Qos;


//--------------------------------------------------------------------------
//  API functions
//--------------------------------------------------------------------------
/// Retrieve QoS parameters of a service flow.
/// 
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param serviceType  Data service type of this
///    flow
/// \param classifierInfo  classifer info of this flow
/// \param payload  Packet payload
/// \param serviceClass  Assigned station class
/// \param qosInfo  To store returned QoS
///    parameter
///
void MacDot16QosUpdateParameters(
         Node* node,
         MacDataDot16* dot16,
         TosType priority,
         TraceProtocolType appType,
         MacDot16CsClassifier* classifierInfo,
         char** payload,
         MacDot16StationClass* stationClass,
         MacDot16QoSParameter* qosInfo,
         int recPacketSize);

/// Add a service for an application.
/// 
///
/// \param node  Pointer to node.
///
void MacDot16QosAddService(
         Node* node);

/// Remove service for an application.
/// 
///
/// \param node  Pointer to node.
///
void MacDot16QosRemoveService(
         Node* node);

/// Set service for an application.
/// 
///
/// \param node  Pointer to node.
/// \param qosInfo  To store returned QoS parameter
///
void MacDot16QosSetServiceParameters(
         Node* node,
         MacDot16QoSParameter* qosInfo);

/// Get class parameters for a service.
/// 
///
/// \param node  pointer to node.
/// \param serviceId  service class id
/// \param qosInfo  to store returned QoS parameter
///
void MacDot16QosGetServiceParameters(
         Node* node,
         unsigned int serviceId,
         MacDot16QoSParameter* qosInfo);

/// Get service class info.
/// 
///
/// \param node  pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param serviceClass  stations class
/// \param qosInfo  to store returned QoS
///    parameter
///
/// \return recode id
int MacDot16QosGetStationClassInfo(
         Node* node,
         MacDataDot16* dot16,
         MacDot16StationClass stationClass,
         MacDot16QoSParameter* qosInfo);


/// Called from app to add a service
/// 
///
/// \param node  the node pointer
/// \param interfaceIndex  interface index of DOT16
/// \param serviceClassType  the service class
/// \param minPktSize  smallest packet size
/// \param maxPktSize  largest packet size
/// \param maxSustainedRate  maximum sustained traffic,
///    per second
/// \param minReservedRate  minimum reserved traffic rate,
///    per second
/// \param maxLatency  maximum latency
/// \param toleratedJitter  tolerated jitter
///
/// \return serviceClassId
unsigned int Dot16QosAddServiceClass(
                   Node* node,
                   unsigned int interfaceIndex,
                   MacDot16ServiceClass serviceClassType,
                   int minPktSize,
                   int maxPktSize,
                   int maxSustainedRate,
                   int minReservedRate,
                   clocktype maxLatency,
                   clocktype toleratedJitter);

/// Initialize DOT16 QOS.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param nodeInput  Pointer to node input.
///
void MacDot16QosInit(
        Node* node,
        int interfaceIndex,
        const NodeInput* nodeInput);



/// Called from app to add a service
/// 
///
/// \param node  the node pointer
/// \param interfaceIndex  Interface index of DOT16
/// \param priority  priority TOS of the flow
/// \param sourceAddr  sourceAddr host address
/// \param sourcePort  sourcePort host port
/// \param destAddr  destAddr address
/// \param destPort  destPort port
/// \param qosInfo  QOS info for new flow
///
/// \return the serviceId   ( -1 if failed)
int MacDot16QosAppAddService(
                   Node* node,
                   unsigned int interfaceIndex,
                   TosType priority,
                   NodeAddress sourceAddr,
                   short sourcePort,
                   NodeAddress destAddr,
                   short destPort,
                   MacDot16QoSParameter* qosInfo);


/// Called from app to update a service's parameters
/// 
///
/// \param node  the node pointer
/// \param serviceId  id of the service to update
/// \param qosInfo  updated QOS info for flow
///
/// \return the serviceId   ( -1 if failed)
int MacDot16QosAppUpdateService(
                   Node* node,
                   unsigned int serviceId,
                   MacDot16QoSParameter* qosInfo);


/// Called from app to drop a service flow
/// 
///
/// \param node  the node pointer
/// \param scId  the service's id to drop
///
/// \return serviceId  (-1 if failed)
int MacDot16QosAppDropService(
                   Node* node,
                   unsigned int serviceId);

#if 0

// FUNCTION   :: MacDot16QosAppDone
// LAYER      :: MAC
// PURPOSE    :: Called from app to terminate service flow and cleanup
//
// PARAMETERS ::
// + node           : Node*                 : the node pointer
// + interfaceIndex : unsigned int          : interface index of DOT16
// + serviceId      :unsigned int           : id of the service to update
// RETURN     :: BOOL : TRUE - Removed
//                      FALSE - Failed
void MacDot16QosAppDone(
                     Node* node,
                     unsigned int interfaceIndex,
                     unsigned int serviceId);
#endif

#endif // MAC_DOT16_QOS_H
