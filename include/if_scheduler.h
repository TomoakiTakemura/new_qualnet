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


// PROTOCOL :: Queue-Scheduler
//
// SUMMARY  :: General Scheduler file
/// 
// LAYER ::
/// 
// STATISTICS::
/// 
// CONFIG_PARAM ::
/// 
// VALIDATION ::
/// 
// IMPLEMENTED_FEATURES ::
/// 
// OMITTED_FEATURES ::
/// 
// ASSUMPTIONS ::
/// 
// STANDARD ::
/// 
// RELATED ::

/// \defgroup Package_SCHEDULERS SCHEDULERS

/// \file
/// \ingroup Package_SCHEDULERS
/// This file describes the member functions of the scheduler base class.


#ifndef IF_SCHEDULER_H
#define IF_SCHEDULER_H

#include "if_queue.h"

//--------------------------------------------------------------------------
// Scheduler API
//--------------------------------------------------------------------------

/// Default number of queue per interface

#define DEFAULT_QUEUE_COUNT 3


/// This structure contains pointers to queue structures,
/// default function behaviors, and statistics for the
/// scheduler

typedef struct queue_data_str
{
    Queue* queue;
    int priority;
    float weight;
    float rawWeight;
    char* infoField;
} QueueData;


//class SchedGraphStat;

/// Scheduler abstract base class

class Scheduler
{
    friend class STAT_StatisticsList;
#ifdef ADDON_DB
    friend class StatsQueueDB::QueueStatusTbBuilder;
#endif
protected:
    QueueData*    queueData;
    int           numQueues;
    int           maxQueues;
    int           infoFieldSize;
    int           packetsLostToOverflow;

    // CurrentPeriod statistics
    clocktype               currentStateStartTime;
    clocktype               utilizedTime;
    BOOL                    stateIsIdle;
    int                     bytesDequeuedInPeriod;
    int                     packetsDequeuedInPeriod;
    clocktype               currentPeriodStartTime;
    clocktype               queueDelaysDuringPeriod;

    // Scheduler statistic collection
    BOOL                    schedulerStatEnabled;
    void*         schedGraphStatPtr;
    // Utility function for packet retrieval from specified priority queue
    QueueData*    SelectSpecificPriorityQueue(int priority);


public:
    void setRawWeight(const int priority, double rawWeight);
    void normalizeWeight();

    virtual int numQueue();
    virtual int GetQueuePriority(int queueIndex);

    virtual void insert(Message* msg,
                        BOOL* QueueIsFull,
                        const int priority,
                        const void* infoField,
                        const clocktype currentTime) = 0;

    virtual void insert(Message* msg,
                        BOOL* QueueIsFull,
                        const int priority,
                        const void* infoField,
                        const clocktype currentTime,
                        TosType* tos) = 0;

    virtual void insert(Node* node,
                        int interfaceIndex,
                        Message* msg,
                        BOOL* QueueIsFull,
                        const int priority,
                        const void* infoField,
                        const clocktype currentTime) = 0;

    virtual void insert(Node* node,
                        int interfaceIndex,
                        Message* msg,
                        BOOL* QueueIsFull,
                        const int priority,
                        const void* infoField,
                        const clocktype currentTime,
                        TosType* tos) = 0;

    virtual BOOL retrieve(const int priority,
                          const int index,
                          Message** msg,
                          int* msgPriority,
                          const QueueOperation operation,
                          const clocktype currentTime) = 0;

    // Resource Management API
    virtual void setQueueBehavior(const int priority,
                                  QueueBehavior suspend = RESUME);
    virtual QueueBehavior getQueueBehavior(const int priority);

    virtual BOOL isEmpty(const int priority,
                         BOOL checkPredecessor = FALSE);

    virtual int bytesInQueue(const int priority,
                             BOOL checkPredecessor = FALSE);

    virtual int numberInQueue(const int priority,
                              BOOL checkPredecessor = FALSE);

    virtual int addQueue(Queue* queue,
                         const int priority = ALL_PRIORITIES,
                         const double weight = 1.0) = 0;

    virtual void removeQueue(const int priority) = 0;

    virtual void swapQueue(Queue* queue, const int priority) = 0;

    virtual void qosInformationUpdate(int queueIndex,
                                      int* qDelayVal,
                                      int* totalTransmissionVal,
                                      const clocktype currentTime,
                                      BOOL isResetTotalTransmissionVal = FALSE);

    // Scheduler current period statistic collection
    //virtual int bytesDequeuedInPeriod(const int priority);
    //virtual clocktype utilizationInPeriod(const int priority);
    //virtual clocktype averageTimeInQueueDuringPeriod(const int priority);
    //virtual int resetPeriod(const clocktype currentTime);
    //virtual clocktype periodStartTime();

    // Scheduler statistic collection for graph
    virtual void collectGraphData(int priority,
                                  int packetSize,
                                  const clocktype currentTime);

    virtual void invokeQueueFinalize(Node* node,
                                     const char* layer,
                                     const int interfaceIndex,
                                     const int instanceId,
                                     const char* invokingProtocol = "IP",
                                     const char* splStatStr = NULL);

    virtual void finalize(Node* node,
                          const char* layer,
                          const int interfaceIndex,
                          const char* invokingProtocol = "IP",
                          const char* splStatStr = NULL) = 0;

    virtual clocktype getTopPacketInsertTime()
    {
        return 0;
    }

    virtual clocktype getPacketInsertTime(int priority, int index)
    {
        int i = 0;

        if (priority == ALL_PRIORITIES)
        {
            ERROR_ReportError("Doesnt make sense to call this function with "
                              "priority = ALL_PRIORITIES");
        }
        else
        {
            for (i = 0; i < numQueues; i++)
            {
                if (priority == queueData[i].priority)
                {
                    return queueData[i].queue->getPacketInsertTime(index);
                }
            }
        }

        // Error: No Queue exists with such a priority value
        char errStr[MAX_STRING_LENGTH] = {0};
        sprintf(errStr, "Scheduler Error:"
                " No Queue exists with priority value %d", priority);
        ERROR_Assert(FALSE, errStr);
        return 0; // Unreachable
    }

    // Virtual Destructor for Scheduler Class
    virtual ~Scheduler(){};
};


// FUNCTION   :: SCHEDULER_Setup
// LAYER      ::
// PURPOSE    :: This function runs the generic and then algorithm-specific
//               scheduler initialization routine.
// PARAMETERS ::
// + scheduler : Scheduler** : Pointer of pointer to Scheduler class
// + schedulerTypeString[] : const char : Scheduler Type string
// + enableSchedulerStat : BOOL : Scheduler Statistics is set YES or NO
// + graphDataStr : const char* : Scheduler's graph statistics is set or not
// RETURN     :: void : Null


void SCHEDULER_Setup(
    Scheduler** scheduler,
    const char schedulerTypeString[],
    BOOL enableSchedulerStat = FALSE,
    const char* graphDataStr = "NA");


// FUNCTION   :: GenericPacketClassifier
// LAYER      ::
// PURPOSE    :: Classify a packet for a specific queue
// PARAMETERS ::
// + scheduler : Scheduler* : Pointer to a Scheduler class.
// + pktPriority : int : Incoming packet's priority
// RETURN     :: int : Integer.


int GenericPacketClassifier(Scheduler *scheduler,
                            int pktPriority);


// Comments from source file copied below to generate API Reference




/// Returns pointer to QueueData associated with the queue.
/// this is a Private
///
/// \param priority  Queue priority
///
/// \return Pointer of queue

/// Returns number of queues under this Scheduler
///
///
/// \return Number of queue.

/// Returns Priority for the queues under this Scheduler
///
/// \param queueIndex  Queue index
///
/// \return Return priority of a queue

/// Returns a Boolean value of TRUE if the array of stored
/// messages in each queue that the scheduler controls are
/// empty, and FALSE otherwise
///
/// \param priority  Priority of a queue
///
/// \return TRUE or FALSE

/// This function prototype returns the total number of bytes
/// stored in the array of either a specific queue, or all
/// queues that the scheduler controls.
///
/// \param priority  Priority of a queue
///
/// \return TRUE or FALSE

/// This function prototype returns the number of messages
/// stored in the array of either a specific queue, or all
/// queues that the scheduler controls.
///
/// \param priority  Priority of a queue
///
/// \return Bytes in queue is used.

/// This function enable Qos monitoring for all
/// queues that the scheduler controls.
///
/// \param queueIndex  Queue index
/// \param qDelayVal  Queue delay
/// \param totalTransmissionVal  Transmission value
/// \param currentTime  Current simulation time
/// \param isResetTotalTransmissionVal  Total Transmission is set or not
///

/// This function enable data collection for performance
/// study of schedulers.
///
/// \param priority  Priority of the queue
/// \param packetSize  Size of packet
/// \param currentTime  Current simulation time
///

/// This function invokes queue finalization.
///
/// \param node  Pointer to Node structure
/// \param layer  The layer string
/// \param interfaceIndex  Interface Index
/// \param instanceId  Instance Ids
/// \param invokingProtocol  The protocol string
/// \param splStatStr  Special string for stat print
///

/// This function runs the generic and then algorithm-specific
/// scheduler initialization routine.
///
/// \param scheduler  Pointer of pointer to Scheduler class
/// \param schedulerTypeString[]  Scheduler Type string
/// \param enableSchedulerStat  Scheduler Statistics is set YES or NO
/// \param graphDataStr  Scheduler's graph statistics is set or not
///

/// Classify a packet for a specific queue
///
/// \param scheduler  Pointer to a Scheduler class.
/// \param pktPriority  Incoming packet's priority
///
/// \return Integer.



#endif // IF_SCHEDULER_H

