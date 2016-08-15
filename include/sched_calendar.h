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
 * PURPOSE: System Queue/Scheduler Structures, Function Pointers, Util Functions
 */

#ifndef SCHED_CALENDAR_H
#define SCHED_CALENDAR_H

#include "scheduler_types.h"
#include "scheduler.h"
#include "calendar.h"


//------------ Declaration of Scheduler Queue Structure -------------
typedef struct scheduler_calender_str {

    SchedulerQueueType              schedQueueType;
    CalendarQInfo                   *schedQueue;

} SchedulerCalendar;


//------------------------------------------------------------------

/// Insert a message into the node's message queue
///
/// \param node  Pointer to the node to insert into
/// \param msg  Pointer to the message to insert
/// \param time  time to delay
void SCHED_CALENDAR_InsertMessage(
    Node *node,
    Message *msg,
    clocktype delay);


/// Remove the first message from the node's message queue
///
/// \param partitionData  Pointer to the partition data
/// \param node  Pointer to the node
///    to be extracted
///
/// \return First message from queue
Message* SCHED_CALENDAR_ExtractFirstMessage(
    PartitionData *partitionData,
    Node *node);


/// Peek at the first message from the node's message queue
/// NOT REMOVED FROM QUEUE
///
/// \param node  Pointer to the node
///    to be extracted
///
/// \return Pointer to message
Message* SCHED_CALENDAR_PeekFirstMessage(
    Node *node);


/// Delete a message from the nodes's message queue
///
/// \param node  Pointer to the node
///    to delete message from
void SCHED_CALENDAR_DeleteMessage(
    Node *node,
    Message *msg);


/// Insert a node into the partition's scheduler queue
///
/// \param partitionData  Pointer to the partition
///    to be inserted
/// \param node  Pointer to the node
void SCHED_CALENDAR_InsertNode(
    PartitionData *partitionData,
    Node *node);


/// Peek at next node to process in the partition's scheduler queue
///
/// \param partitionData  Pointer to the node
///    to be inserted
///
/// \return next node to process
Node* SCHED_CALENDAR_PeekNextNode(
    PartitionData *partitionData);


/// Delete a node from the partition's scheduler queue
///
/// \param partitionData  Pointer to the node
///    to be inserted
/// \param node  Pointer to the node
void SCHED_CALENDAR_DeleteNode(
    PartitionData *partitionData,
    Node *node);


/// Current node from the partition's scheduler queue
///
/// \param partitionData  Pointer to the node
///    to be inserted
Node* SCHED_CALENDAR_CurrentNode(
    const PartitionData *partitionData);


/// Check for nodes in the scheduler's node queue.
///
/// \param partitionData  Pointer to the partition data
///    
///
/// \return scheduler has nodes in it's queue
BOOL SCHED_CALENDAR_HasNodes(
    const PartitionData *partitionData);


/// Get the next event time for this partition
///
/// \param partitionData  Pointer to the partition data
/// \param node  Pointer to the node
///
/// \return next event
Message* SCHED_CALENDAR_NextEvent(
    PartitionData *partitionData,
    Node* node);


/// Get the next event time for this partition
///
/// \param partitionData  Pointer to the partition data
///
/// \return time of next event
clocktype SCHED_CALENDAR_NextEventTime(
    const PartitionData *partitionData);


/// Initalize Scheduler
///
/// \param partitionData  Pointer to the partition data
void SCHED_CALENDAR_Initialize(
    PartitionData *partitionData);


/// Finalize Scheduler
///
/// \param partitionData  Pointer to the partition data
void SCHED_CALENDAR_Finalize(
    PartitionData *partitionData);

//----------------------------------------------------------------------

#endif /* SCHED_CALENDAR_H */
