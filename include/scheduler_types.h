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
 * PURPOSE: System Queue/Scheduler Structures, Function Pointers
 */

#ifndef SCHEDULER_TYPES_H
#define SCHEDULER_TYPES_H

//#include "calendar.h"
#include "splaytree.h"

//-------- Function Pointer definitions for individual queues ---------

/// Insert a message into the node's message queue
///
/// \param node  Pointer to the node to insert into
/// \param msg  Pointer to the message to insert
/// \param time  time to delay
typedef void (*SchedulerQInsertMessageFunction) (Node *node, Message *msg, clocktype delay);


/// Remove the first message from the node's message queue
///
/// \param partitionData  Pointer to the node
/// \param node  Pointer to the node
///    to be extracted
///
/// \return First message from queue
typedef Message* (*SchedulerQExtractFirstMessageFunction) (PartitionData *partitionData, Node *node);


/// Peek at the first message from the node's message queue
/// NOT REMOVED FROM QUEUE
///
/// \param node  Pointer to the node
///    to be extracted
///
/// \return Pointer to message
typedef Message* (*SchedulerQPeekFirstMessageFunction) (Node *node);


/// Delete a message from the nodes's message queue
///
/// \param node  Pointer to the node
///    to delete message from
typedef void (*SchedulerQDeleteMessageFunction) (Node *node, Message *msg);


/// Insert a node into the partition's scheduler queue
///
/// \param partitionData  Pointer to the partition
///    to be inserted
/// \param node  Pointer to the node
typedef void (*SchedulerQInsertNodeFunction) (PartitionData *partitionData, Node *node);


/// Peek at next node to process in the partition's scheduler queue
///
/// \param partitionData  Pointer to the node
///    to be inserted
///
/// \return next node to process
typedef Node* (*SchedulerQPeekNextNodeFunction) (PartitionData *partitionData);


/// Delete a node from the partition's scheduler queue
///
/// \param partitionData  Pointer to the node
///    to be inserted
/// \param node  Pointer to the node
typedef void (*SchedulerQDeleteNodeFunction)  (PartitionData *partitionData, Node *node);

/// Current node from the partition's scheduler queue
///
/// \param partitionData  Pointer to the node
///    to be inserted
typedef Node* (*SchedulerQCurrentNodeFunction) (const PartitionData *partitionData);

/// Check for nodes in the scheduler's node queue.
///
/// \param partitionData  Pointer to the partition data
///    
///
/// \return scheduler has nodes in it's queue
typedef BOOL (*SchedulerQHasNodesFunction) (const PartitionData *partitionData);

/// Get the next event time for this partition
///
/// \param partitionData  Pointer to the partition data
/// \param node  Pointer to the node
///
/// \return next event
typedef Message* (*SchedulerQNextEventFunction) (PartitionData *partitionData, Node* node);

/// Get the next event time for this partition
///
/// \param partitionData  Pointer to the partition data
///
/// \return time of next event
typedef clocktype (*SchedulerQNextEventTimeFunction) (const PartitionData *partitionData);

/// Initalize Scheduler queue
///
/// \param partitionData  Pointer to the partition data
typedef void (*SchedulerQInitalizeFunction)(PartitionData *partitionData);

/// Finalize Scheduler Queue
///
/// \param partitionData  Pointer to the partition data
typedef void (*SchedulerQFinalizeFunction) (PartitionData *partitionData);

//-------------------------------------------------------------------

// -------------------- Queue Types available ---------------------
typedef enum {
    SPLAYTREE_QUEUE,
    // CALENDAR_QUEUE,     // NO LONGER USED
    LADDER_QUEUE,
    STDLIB_HEAP,
    CALENDAR_QUEUE2
} SchedulerQueueType;

class CalendarQ;

//------------ Declaration of Scheduler Queue Structure -------------
typedef struct scheduler_queue_str {

    SchedulerQueueType          schedQueueType;
    SplayTree                   splayTree;
    CalendarQ *                 calendarQ;

    BOOL                        isCollectingStats;

    // Standard Function Pointers for queue calls
    SchedulerQInsertMessageFunction         InsertMessage;
    SchedulerQExtractFirstMessageFunction   ExtractFirstMessage;
    SchedulerQPeekFirstMessageFunction      PeekFirstMessage;
    SchedulerQDeleteMessageFunction         DeleteMessage;
    SchedulerQInsertNodeFunction            InsertNode;
    SchedulerQPeekNextNodeFunction          PeekNextNode;
    SchedulerQDeleteNodeFunction            DeleteNode;
    SchedulerQCurrentNodeFunction           CurrentNode;
    SchedulerQHasNodesFunction              HasNodes;
    SchedulerQNextEventFunction             NextEvent;
    SchedulerQNextEventTimeFunction         NextEventTime;
    SchedulerQInitalizeFunction             Initalize;
    SchedulerQFinalizeFunction              Finalize;

} SchedulerInfo;

//--------------------------------------------------------------------

#endif /* SCHEDULER_TYPES_H */
