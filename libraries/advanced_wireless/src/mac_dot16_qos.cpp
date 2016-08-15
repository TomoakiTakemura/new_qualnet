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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "api.h"
#include "network_ip.h"
#include "transport_udp.h"
#include "transport_tcp_hdr.h"
#include "app_cbr.h"
#include "app_vbr.h"
#include "app_superapplication.h"
#include "mac.h"

#include "ipv6.h"

#include "mac_dot16.h"
#include "mac_dot16_bs.h"
#include "mac_dot16_ss.h"
#include "mac_dot16_cs.h"
#include "mac_dot16_tc.h"
#include "mac_dot16_qos.h"

#define DEBUG                   0
#define DEBUG_APP_SERVICE       0
#define DEBUG_SERVICE_CLASS     0

#define DOT16_USE_SUPERAPP      0


/////////////////////////////////////////////////////////////////////
//
//  Start of Common Service functions
//
//////////////////////////////////////////////////////////////////////


// Add a new service class record
//
// \param node  Pointer to node.
// \param dot16  Pointer to DOT16 structure
// \param serviceClassId  service class id
// \param pktInfo  classifer info of this flow
//
// \return Pointer to the added record
static
unsigned int MacDot16QosServiceClassAddRecord(
        Node* node,
        MacDataDot16* dot16,
        const char* stationClassName,
        MacDot16StationClass stationClass,
        MacDot16QoSParameter* params)
{
    MacDot16Qos* dot16Qos = (MacDot16Qos*) dot16->qosData;
    MacDot16QosServiceClassRecord* recordPtr;

    if (DEBUG_SERVICE_CLASS)
    {
        printf("Node%d QOS is adding a new station class record.\n",
               node->nodeId);
        printf("    service class type = %d\n", stationClass);
    }

    // construct the classifier record
    recordPtr = (MacDot16QosServiceClassRecord*)
                MEM_malloc(sizeof(MacDot16QosServiceClassRecord));
    ERROR_Assert(recordPtr != NULL, "MAC 802.16: Out of memory!");

    strcpy(&recordPtr->stationClassName[0], stationClassName);
    recordPtr->stationClass = stationClass;
    recordPtr->serviceClassId = dot16Qos->serviceClassIds++;

    MacDot16QoSParameter* parameters = (MacDot16QoSParameter*)
             MEM_malloc(sizeof(MacDot16QoSParameter));
    ERROR_Assert(parameters != NULL, "MAC 802.16: Out of memory!");
    memset(parameters, 0, sizeof(MacDot16QoSParameter));

    memcpy((char*) parameters,
           (char*) params,
           sizeof(MacDot16QoSParameter));

    recordPtr->parameters = parameters;

    // add into the record array
    if (dot16Qos->numServiceClassRecords ==
        dot16Qos->allocServiceClassRecords)
    { // no more space to hold new record, expand it
        MacDot16QosServiceClassRecord** newPtr;

        dot16Qos->allocServiceClassRecords +=
                DOT16_QOS_NUM_SERVICE_CLASS_RECORD;

        newPtr = (MacDot16QosServiceClassRecord**)
                 MEM_malloc(sizeof(MacDot16QosServiceClassRecord*) *
                            dot16Qos->allocServiceClassRecords);
        ERROR_Assert(newPtr != NULL, "MAC 802.16: Out of memory!");

        // copy over existing records
        memcpy((char*) newPtr,
               (char*) dot16Qos->serviceClassRecords,
               dot16Qos->numServiceClassRecords *
               sizeof(MacDot16QosServiceClassRecord*));

        // free old memory
        MEM_free(dot16Qos->serviceClassRecords);

        dot16Qos->serviceClassRecords = newPtr;
    }

    dot16Qos->serviceClassRecords[dot16Qos->numServiceClassRecords]
        = recordPtr;
    dot16Qos->numServiceClassRecords++;

    return dot16Qos->numServiceClassRecords;

}

//////////////////////////////////////////////////////////////////////
//
//  Start of Common QOS functions
//
//////////////////////////////////////////////////////////////////////

// Get QoS parameters for a service class.
// 
//
// \param node  Pointer to node.
// \param node  Stations class.
// \param qosInfo  Pointer qos info structure.
//
static
void MacDot16QosGetStationClassParameters(
        Node* node,
        MacDataDot16* dot16,
        MacDot16StationClass stationClass,
        MacDot16QoSParameter* qosInfo)
{
    int ret = -1;
    MacDot16QoSParameter classInfo;
    memset(&classInfo, 0, sizeof(MacDot16QoSParameter));

    ERROR_Assert(qosInfo != NULL,
                 "MAC 802.16: NULL QOS parameter structure!");


    ret = MacDot16QosGetStationClassInfo(
                node,
                dot16,
                stationClass,
                &classInfo);

    if (ret > -1)
    {
        // copy class parameters to qos structure
        qosInfo->priority = classInfo.priority;
        qosInfo->minPktSize = classInfo.minPktSize;
        qosInfo->maxPktSize = classInfo.maxPktSize;
        qosInfo->maxSustainedRate = classInfo.maxSustainedRate;
        qosInfo->minReservedRate = classInfo.minReservedRate;
        qosInfo->maxLatency = classInfo.maxLatency;
        qosInfo->toleratedJitter = classInfo.toleratedJitter;

    }

}


// Get qos parameters for an application.
// 
//
// \param node  Pointer to node.
// \param appId  Application type.
// \param payload  pointer to message payload
// \param stationClass  classification of the station
// \param classifierInfo  classifer info of this flow
// \param qosInfo  To store returned QoS
//    parameter
//
// \return TRUE - App info was found
// FALSE -  App info was not found
static
BOOL MacDot16QosGetAppParameters(
        Node* node,
        MacDataDot16* dot16,
        TraceProtocolType appType,
        char** payload,
        MacDot16StationClass* stationClass,
        MacDot16CsClassifier* classifierInfo,
        MacDot16QoSParameter* qosInfo,
        int recPacketSize)
{
    clocktype temp;
    switch(appType)
    {
        case TRACE_CBR:
        {
             // this is a CBR packet
            CbrData* dataPtr;
            dataPtr = (CbrData* )MEM_malloc(sizeof(CbrData));
            memcpy((char*)dataPtr, (*payload),sizeof(CbrData));
            qosInfo->minPktSize = recPacketSize;
            qosInfo->maxPktSize = recPacketSize;
            temp = recPacketSize * 8 * SECOND;
            qosInfo->maxSustainedRate = (int) (temp / dataPtr->interval);

            qosInfo->minReservedRate = qosInfo->maxSustainedRate;
            qosInfo->maxLatency = dataPtr->interval;
            qosInfo->toleratedJitter = 0;
            // set the correct source port number
            *stationClass = DOT16_STATION_CLASS_APP_SPECIFIED;
            MEM_free(dataPtr);
            return TRUE;
            break;
        }

        case TRACE_VBR:
        {
             // this is a VBR packet
            VbrMsgData* dataPtr;
            dataPtr = (VbrMsgData*)MEM_malloc(sizeof(VbrMsgData));
            memcpy((char*)dataPtr, (*payload),sizeof(VbrMsgData));
            qosInfo->minPktSize = recPacketSize;
            qosInfo->maxPktSize = recPacketSize;
            temp = recPacketSize * 8 * SECOND;
            qosInfo->maxSustainedRate = (int)(temp / dataPtr->meanInterval);

            qosInfo->minReservedRate = qosInfo->maxSustainedRate;
            qosInfo->maxLatency = dataPtr->meanInterval;
            qosInfo->toleratedJitter = 0;

            // set the correct source port number
            classifierInfo->srcPort = dataPtr->sourcePort;

            *stationClass = DOT16_STATION_CLASS_APP_SPECIFIED;
            MEM_free(dataPtr);
            return TRUE;
            break;
        }

        case TRACE_TCP:
        {

#if DOT16_USE_SUPERAPP

            SuperApplicationTCPDataPacket* tcpDataPtr;
            tcpDataPtr = (SuperApplicationTCPDataPacket* )
                MEM_malloc(sizeof(SuperApplicationTCPDataPacket));
            memcpy((char*)tcpDataPtr, (*payload),
                sizeof(SuperApplicationTCPDataPacket));
            if (tcpDataPtr->traceType == TRACE_SUPERAPPLICATION)
            {
                qosInfo->minPktSize = tcpDataPtr->minPktSize;
                qosInfo->maxPktSize = tcpDataPtr->maxPktSize;
                qosInfo->maxSustainedRate = tcpDataPtr->maxSustainedRate;
                qosInfo->minReservedRate = tcpDataPtr->maxSustainedRate;
                qosInfo->maxLatency = tcpDataPtr->maxLatency;
                qosInfo->toleratedJitter = tcpDataPtr->toleratedJitter;

                // set the correct source port number
                classifierInfo->srcPort = tcpDataPtr->sourcePort;
                *stationClass = DOT16_STATION_CLASS_APP_SPECIFIED;
                MEM_free(tcpDataPtr);
                return TRUE;
            }
            else
            {
                MEM_free(tcpDataPtr);
                return FALSE;
            }
#endif
            break;
        }

    default:
        {
            // app info not avaliable
            return FALSE;
        }

    }

    // app info found
    return FALSE;

}


// Retrieve QoS parameters of a service flow.
//
// \param node  Pointer to node.
// \param dot16  Pointer to DOT16 structure
// \param serviceType  Data service type of this
//    flow
// \param classifierInfo  classifer info of this flow
// \param payload  Packet payload
// \param serviceType  Assigned service type
// \param qosInfo  To store returned QoS
//    parameter
//
void MacDot16QosUpdateParameters(
        Node* node,
        MacDataDot16* dot16,
        TosType tos,
        TraceProtocolType appType,
        MacDot16CsClassifier* classifierInfo,
        char** payload,
        MacDot16StationClass* stationClass,
        MacDot16QoSParameter* qosInfo,
        int recPacketSize)
{

    MacDot16Qos* dot16Qos = (MacDot16Qos*) dot16->qosData;
    BOOL found = FALSE;

    *stationClass = dot16Qos->stationClass;

    found = MacDot16QosGetAppParameters(
        node,
        dot16,
        appType,
        payload,
        stationClass,
        classifierInfo,
        qosInfo,
        recPacketSize);

    if (!found)
    {
        MacDot16QosGetStationClassParameters(
            node,
            dot16,
            *stationClass,
            qosInfo);
    }

}


// Get service class info.
// 
//
// \param node  pointer to node.
// \param dot16  pointer to DOT16 structure
// \param serviceClass  station class type
// \param qosInfo  to store returned
//    QoS parameter
//
// \return Service class record id
int MacDot16QosGetStationClassInfo(
         Node* node,
         MacDataDot16* dot16,
         MacDot16StationClass stationClass,
         MacDot16QoSParameter* qosInfo)
{
    MacDot16Qos* dot16Qos = (MacDot16Qos*) dot16->qosData;
    MacDot16QosServiceClassRecord* recordPtr = NULL;

    int i = 0;

    if (DEBUG_SERVICE_CLASS)
    {
        MacDot16PrintRunTimeInfo(node, dot16);
        printf("Node%d QOS is serching for service class record.\n",
               node->nodeId);
    }

    while (i < dot16Qos->numServiceClassRecords)
    {
        recordPtr = dot16Qos->serviceClassRecords[i];

        if (recordPtr->stationClass == stationClass)
        {

            if (recordPtr->parameters != NULL)
            {

                if (DEBUG_SERVICE_CLASS)
                {
                    printf("    Record found.\n");
                }

                qosInfo->priority = recordPtr->parameters->priority;
                qosInfo->maxLatency = recordPtr->parameters->maxLatency;
                qosInfo->maxPktSize = recordPtr->parameters->maxPktSize;
                qosInfo->maxSustainedRate =
                        recordPtr->parameters->maxSustainedRate;
                qosInfo->minPktSize = recordPtr->parameters->minPktSize;
                qosInfo->minReservedRate=
                        recordPtr->parameters->minReservedRate;
                qosInfo->toleratedJitter=
                        recordPtr->parameters->toleratedJitter;

                return recordPtr->serviceClassId;
            }
        }
        else
            i++;
    }

    if (DEBUG_SERVICE_CLASS)
    {
        printf("    Record not found!\n");
    }

    return -1;

}


// Called from app to add a service
// 
//
// \param node  the node pointer
// \param interfaceIndex  interface  index of  DOT16
// \param stationClassName  station class name
// \param stationClass  station class
// \param minPktSize  smallest packet size
// \param maxPktSize  largest packet size
// \param maxSustainedRate  maximum sustained traffic,
//    per second
// \param minReservedRate  minimum reserved traffic
//    rate, per second
// \param maxLatency  maximum latency
// \param toleratedJitter  tolerated jitter
//
// \return serviceClassId
unsigned int Dot16QosAddStationClass(
                   Node* node,
                   unsigned int interfaceIndex,
                   const char* stationClassName,
                   MacDot16StationClass stationClass,
                   int minPktSize,
                   int maxPktSize,
                   int maxSustainedRate,
                   int minReservedRate,
                   clocktype maxLatency,
                   clocktype toleratedJitter)
{
    MacDataDot16* dot16 = (MacDataDot16*)
                          node->macData[interfaceIndex]->macVar;
    unsigned int serviceClassId = 0;

    // fill in qos info for new classifier
    MacDot16QoSParameter qosInfo;
    memset(&qosInfo, 0, sizeof(MacDot16QoSParameter));

    qosInfo.minPktSize = minPktSize;
    qosInfo.maxPktSize = maxPktSize;
    qosInfo.maxSustainedRate = maxSustainedRate;
    qosInfo.minReservedRate = minReservedRate;
    qosInfo.maxLatency = maxLatency;
    qosInfo.toleratedJitter = toleratedJitter;

    // add the app service record and get service id
    serviceClassId = MacDot16QosServiceClassAddRecord(
                        node,
                        dot16,
                        stationClassName,
                        stationClass,
                        &qosInfo);

    // return service class Id
    return serviceClassId;

}
#if 0
// Called add a service class
// 
//
// \param node  the node pointer
// \param interfaceIndex  interface  index
// \param stationClassName  station class name
// \param stationClass  station class
// \param minPktSize  smallest packet size
// \param maxPktSize  largest packet size
// \param maxSustainedRate  maximum sustained traffic,
//    per second
// \param minReservedRate  minimum reserved traffic
//    rate, per second
// \param maxLatency  maximum latency
// \param toleratedJitter  tolerated jitter
//
// \return serviceClassId

static
unsigned int Dot16QosAddServiceClass(
                   Node* node,
                   unsigned int interfaceIndex,
                   char* stationClassName,
                   MacDot16StationClass stationClass,
                   int minPktSize,
                   int maxPktSize,
                   int maxSustainedRate,
                   int minReservedRate,
                   clocktype maxLatency,
                   clocktype toleratedJitter)
{
    MacDataDot16* dot16 = (MacDataDot16*)
                          node->macData[interfaceIndex]->macVar;

    unsigned int serviceClassId = 0;

    // fill in qos info for new classifier
    MacDot16QoSParameter qosInfo;
    memset(&qosInfo, 0, sizeof(MacDot16QoSParameter));

    qosInfo.minPktSize = minPktSize;
    qosInfo.maxPktSize = maxPktSize;
    qosInfo.maxSustainedRate = maxSustainedRate;
    qosInfo.minReservedRate = minReservedRate;
    qosInfo.maxLatency = maxLatency;
    qosInfo.toleratedJitter = toleratedJitter;

    // add the app service record and get service id
    serviceClassId = MacDot16QosServiceClassAddRecord(
                        node,
                        dot16,
                        stationClassName,
                        stationClass,
                        &qosInfo);

    // return service class Id
    return serviceClassId;

}
#endif
// Initialize QOS classes.
//
// \param node  Pointer to node.
// \param interfaceIndex  Interface index
// \param nodeInput  Pointer to node input.
//
void MacDot16QosInitClasses(
        Node* node,
        MacDataDot16* dot16,
        int interfaceIndex,
        const NodeInput* nodeInput)
{
    unsigned ret = 0;

    // Add Gold class
    ret = Dot16QosAddStationClass(node,
                interfaceIndex,
                "GOLD",
                DOT16_STATION_CLASS_GOLD,
                1024,
                1024,
                500000,
                200000,
                30 * MILLI_SECOND,
                100 * MICRO_SECOND);

    ERROR_Assert(ret != 0,
                "MAC 802.16: GOLD QOS class assignment error!");

    // Add Silver class
    ret = Dot16QosAddStationClass(node,
                interfaceIndex,
                "SILVER",
                DOT16_STATION_CLASS_SILVER,
                1024,
                512,
                200000,
                500000,
                100 * MILLI_SECOND,
                300 * MICRO_SECOND);

    ERROR_Assert(ret != 0,
                "MAC 802.16: SILVER QOS class assignment error!");

    // Add Bronze class
    ret = Dot16QosAddStationClass(node,
                interfaceIndex,
                "BRONZE",
                DOT16_STATION_CLASS_BRONZE,
                256,
                256,
                800,
                800,
                100 * MILLI_SECOND,
                100 * MICRO_SECOND);

    ERROR_Assert(ret != 0,
                "MAC 802.16: BRONZE QOS class assignment error!");

    // Add default class
    ret = Dot16QosAddStationClass(node,
                interfaceIndex,
                "DEFAULT",
                DOT16_STATION_CLASS_DEFAULT,
                1024,
                512,
                100000,
                100000,
                150 * MILLI_SECOND,
                1000 * MICRO_SECOND);
 
    ERROR_Assert(ret != 0,
             "MAC 802.16: DEFAULT QOS class assignment error!");

}


// Initialize DOT16 QOS.
//
// \param node  Pointer to node.
// \param interfaceIndex  Interface index
// \param nodeInput  Pointer to node input.
//
void MacDot16QosInit(
        Node* node,
        int interfaceIndex,
        const NodeInput* nodeInput)
{

    //Address interfaceAddress;

    //NetworkGetInterfaceInfo(node, interfaceIndex, &interfaceAddress);

    BOOL wasFound = false;
    char stringVal[MAX_STRING_LENGTH];

    MacDataDot16* dot16 = (MacDataDot16*)
                          node->macData[interfaceIndex]->macVar;

    MacDot16Qos* dot16Qos;

    dot16Qos = (MacDot16Qos*) MEM_malloc(sizeof(MacDot16Qos));
    ERROR_Assert(dot16Qos != NULL,
                 "MAC 802.16: Out of memory!");

    // using memset to initialize the whole QOS data strucutre
    memset((char*)dot16Qos, 0, sizeof(MacDot16Qos));
    dot16->qosData = (void*) dot16Qos;

    // alloc initial Service Class array
    dot16Qos->numServiceClassRecords = 0;
    dot16Qos->serviceClassRecords = (MacDot16QosServiceClassRecord**)
            MEM_malloc(sizeof(MacDot16QosServiceClassRecord*) *
                       DOT16_QOS_NUM_SERVICE_CLASS_RECORD);

    dot16Qos->allocServiceClassRecords =
                DOT16_QOS_NUM_SERVICE_CLASS_RECORD;

    MacDot16QosInitClasses(
            node,
            dot16,
            interfaceIndex,
            nodeInput);

    // read the duplex mode
    IO_ReadString(node,
                  node->nodeId,
                  interfaceIndex,
                  nodeInput,
                  "MAC-802.16-STATION-CLASS",
                  &wasFound,
                  stringVal);

    if (wasFound)
    {
        if (strcmp(stringVal, "GOLD") == 0)
        {
            // gold - service class
            dot16Qos->stationClass = DOT16_STATION_CLASS_GOLD;
            strcpy((char*)dot16Qos->stationClassName, stringVal);
        }
        else
        if (strcmp(stringVal, "SILVER") == 0)
        {
            // silver - service class
            dot16Qos->stationClass = DOT16_STATION_CLASS_SILVER;
            strcpy((char*)dot16Qos->stationClassName, stringVal);
        }
        else
        if (strcmp(stringVal, "BRONZE") == 0)
        {
            // bronze - service class
            dot16Qos->stationClass = DOT16_STATION_CLASS_BRONZE;
            strcpy((char*)dot16Qos->stationClassName, stringVal);
        }
        else
        {
            ERROR_ReportWarning(
                "MAC802.16: Wrong value for MAC-802.16-SERVICE-CLASS,"
                " should be GOLD, SILVER or BRONZE. Using default"
                " value as BRONZE.");

            // by default, a node uses DEFAULT
            dot16Qos->stationClass = DOT16_STATION_CLASS_BRONZE;
            strcpy((char*)dot16Qos->stationClassName, "BRONZE");
        }
    }
    else
    {
        // not configured. Assume DEFAULT by default
        dot16Qos->stationClass = DOT16_STATION_CLASS_BRONZE;
        strcpy((char*)dot16Qos->stationClassName, "BRONZE");

    }

    dot16Qos->qosAggTable =
        (MacDot16QosAggTable*) MEM_malloc(sizeof(MacDot16QosAggTable));
    ERROR_Assert(dot16Qos->qosAggTable != NULL,
                 "MAC 802.16: Out of memory!");

    memset((char*)dot16Qos->qosAggTable, 0, sizeof(MacDot16QosAggTable));

}


// Add a new service record
//
// \param node  Pointer to node.
// \param dot16  Pointer to DOT16 structure
// \param serviceType  app service type
// \param params  classifer info of this flow
//
// \return Index to the added record
static
unsigned int MacDot16QosAppServiceAddRecord(
        Node* node,
        MacDataDot16* dot16,
        MacDot16ServiceType serviceType,
        MacDot16QoSParameter* params)
{
    MacDot16Qos* dot16Qos = (MacDot16Qos*) dot16->qosData;
    MacDot16QosAppServiceRecord* recordPtr;

    if (DEBUG)
    {
        printf("Node%d QOS is adding a new application service record.\n",
               node->nodeId);
        printf("    service type = %d\n", serviceType);
    }

    // construct the classifier record
    recordPtr = (MacDot16QosAppServiceRecord*)
                MEM_malloc(sizeof(MacDot16QosAppServiceRecord));
    ERROR_Assert(recordPtr != NULL, "MAC 802.16: Out of memory!");

    recordPtr->appServiceId = dot16Qos->appServiceIds++;

    MacDot16QoSParameter* parameters = (MacDot16QoSParameter*)
             MEM_malloc(sizeof(MacDot16QoSParameter));
    ERROR_Assert(parameters != NULL, "MAC 802.16: Out of memory!");

    memcpy((char*) parameters,
           (char*) params,
           sizeof(MacDot16QoSParameter));

    recordPtr->parameters = parameters;

    // add into the record array
    if (dot16Qos->numAppServiceRecords == dot16Qos->allocAppServiceRecords)
    { // no more space to hold new record, expand it
        MacDot16QosAppServiceRecord** newPtr;

        dot16Qos->allocAppServiceRecords +=
                    DOT16_QOS_NUM_APP_SERVICE_RECORD;

        newPtr = (MacDot16QosAppServiceRecord**)
                 MEM_malloc(sizeof(MacDot16QosAppServiceRecord*) *
                            dot16Qos->allocAppServiceRecords);
        ERROR_Assert(newPtr != NULL, "MAC 802.16: Out of memory!");

        // copy over existing records
        memcpy((char*) newPtr,
               (char*) dot16Qos->appServiceRecords,
               dot16Qos->numAppServiceRecords *
               sizeof(MacDot16QosAppServiceRecord*));

        // free old memory
        MEM_free(dot16Qos->appServiceRecords);

        dot16Qos->appServiceRecords = newPtr;
    }

    dot16Qos->appServiceRecords[dot16Qos->numAppServiceRecords] = recordPtr;
    dot16Qos->numAppServiceRecords++;

    return recordPtr->appServiceId;

}

#if 0
// Remove invalidated service record.
// Removed record will be freed
//
// \param node  Pointer to node.
// \param dot16  Pointer to DOT16 structure
// \param serviceId  service record to remove
//
static
void MacDot16QosAppServiceRemoveRecord(
        Node* node,
        MacDataDot16* dot16,
        unsigned int appServiceId)
{
    MacDot16Qos* dot16Qos = (MacDot16Qos*) dot16->qosData;
    MacDot16QosAppServiceRecord* recordPtr = NULL;
    int i = 0;

    if (DEBUG)
    {
        printf("Node%d QOS is removing application service record.\n",
               node->nodeId);
    }

    while (i < dot16Qos->numAppServiceRecords)
    {
        recordPtr = dot16Qos->appServiceRecords[i];

        if (recordPtr->appServiceId == appServiceId )
        {
            // need to remove this one
            if (i < (dot16Qos->numAppServiceRecords - 1))
            {
                // not the last one
                memmove((char*) &(dot16Qos->appServiceRecords[i]),
                        (char*) &(dot16Qos->appServiceRecords[i + 1]),
                        (dot16Qos->numAppServiceRecords - i - 1) *
                        sizeof(MacDot16QosAppServiceRecord*));
            }

            if (recordPtr->parameters != NULL)
            {
                MEM_free(recordPtr->parameters);
            }
            MEM_free(recordPtr);
            recordPtr = NULL;

            dot16Qos->numAppServiceRecords--;
        }
        else
        {
            i ++;
        }
    }

}
#endif

// Called from app to add a service
// 
//
// \param node  the node pointer
// \param interfaceIndex  interface index
// \param priority  priority TOS of the flow
// \param sourceAddr  sourceAddr host address
// \param sourcePort  sourcePort host port
// \param destAddr  destAddr address
// \param destPort  destPort port
// \param qosInfo  qos structure
//
// \return serviceId  (-1 if failed)
int MacDot16QosAppAddService(
                   Node* node,
                   unsigned int interfaceIndex,
                   TosType tos,
                   NodeAddress sourceAddr,
                   short sourcePort,
                   NodeAddress destAddr,
                   short destPort,
                   MacDot16QoSParameter* qosInfo)
{

    MacDataDot16* dot16 = (MacDataDot16*)
                          node->macData[interfaceIndex]->macVar;

    MacDot16CsClassifierRecord* recordPtr = NULL;
    unsigned int serviceId = 0;

    // fill in classifier struct
    MacDot16CsClassifier classifierInfo;
    memset(&classifierInfo,0,sizeof(MacDot16CsClassifier));

    SetIPv4AddressInfo( &classifierInfo.srcAddr, sourceAddr);
    SetIPv4AddressInfo( &classifierInfo.dstAddr, destAddr);
    classifierInfo.srcPort = sourcePort;
    classifierInfo.dstPort = destPort;
    classifierInfo.ipProtocol = IPPROTO_UDP;
    classifierInfo.type = CLASSIFIER_TYPE_1;

    // get the service type
    MacDot16ServiceType serviceType = DOT16_SERVICE_BE;

    // look for existing classifier
    recordPtr = MacDot16CsGetClassifierInfo(
                        node,
                        dot16,
                        &classifierInfo);


    // need to add new classifier
    if (recordPtr == NULL)
    {

        // get the service class for the packet
        serviceType = MacDot16TcGetServiceClass(
                    node,
                    dot16,
                    tos);

        // add the new classifier
        recordPtr = MacDot16CsNewClassifier(
                            node,
                            dot16,
                            serviceType,
                            NULL,
                            dot16->macAddress,
                            qosInfo,
                            &classifierInfo);

    } else {

        recordPtr->refCount++;
    }

    // add the app service record and get service id
    serviceId = MacDot16QosAppServiceAddRecord(
                        node,
                        dot16,
                        serviceType,
                        qosInfo);

    // return service Id
    return serviceId;

}


// Called from SuperApp to update a services qos parameters
// 
//
// \param node  the node pointer
// \param serviceId  id of the service to update
// \param qosInfo  updated QOS info for flow
//
// \return serviceId  (-1 if failed)
int MacDot16QosAppUpdateService(
                   Node* node,
                   unsigned int serviceId,
                   MacDot16QoSParameter* qosInfo)
{
    return -1;
}

#if 0
// Called from App to terminate service flow
// 
//
// \param node  the node pointer
// \param interfaceIndex  interface index
// \param serviceId  id of the service to update
//
// \return TRUE - Removed
// FALSE - Failed

void MacDot16QosAppDone(Node* node,
                        unsigned int interfaceIndex,
                        unsigned int serviceId)
{

    // Do not use!
    return;

    MacDataDot16* dot16 = (MacDataDot16*)
                          node->macData[interfaceIndex]->macVar;

    // Find flow and invalidate it

    // remove service record
   MacDot16QosAppServiceRemoveRecord(node, dot16, serviceId);
}
#endif


//////////////////////////////////////////////////////////////////////
//
//  End of Common QOS functions
//
//////////////////////////////////////////////////////////////////////
