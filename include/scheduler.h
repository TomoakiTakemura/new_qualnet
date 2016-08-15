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

#ifndef SCHEDULER_H
#define SCHEDULER_H

//------------------------------------------------------------------
//-- New global system scheduler calls (replaces existing SCHED_) --
//------------------------------------------------------------------

// -- inlines --

/// Insert a message into the node's message queue
///
/// \param node  Pointer to the node to insert into
/// \param msg  Pointer to the message to insert
/// \param time  time to delay
inline
void SCHED_InsertMessage(
    Node *node,
    Message *msg,
    clocktype delay)
{
    node->partitionData->schedulerInfo->InsertMessage(node, msg, delay);
}


/// Remove the first message from the node's message queue
///
/// \param partitionData  Pointer to the partition data
/// \param node  Pointer to the node
///    to be extracted
///
/// \return First message from queue
inline
Message* SCHED_ExtractFirstMessage(
    PartitionData *partitionData,
    Node *node)
{
    return node->partitionData->schedulerInfo->ExtractFirstMessage(partitionData, node);
}


/// Peek at the first message from the node's message queue
/// NOT REMOVED FROM QUEUE
///
/// \param node  Pointer to the node
///    to be extracted
///
/// \return Pointer to message
inline
Message* SCHED_PeekFirstMessage(
    Node *node)
{
    return node->partitionData->schedulerInfo->PeekFirstMessage(node);
}


/// Delete a message from the nodes's message queue
///
/// \param node  Pointer to the node
///    to delete message from
inline
void SCHED_DeleteMessage(
    Node *node,
    Message *msg)
{
    node->partitionData->schedulerInfo->DeleteMessage(node, msg);
}


/// Insert a node into the partition's scheduler queue
///
/// \param partitionData  Pointer to the partition
///    to be inserted
/// \param node  Pointer to the node
inline
void SCHED_InsertNode(
    PartitionData *partitionData,
    Node* node)
{
    partitionData->schedulerInfo->InsertNode(partitionData, node);
}


/// Peek at next node to process in the partition's scheduler queue
/// NOT REMOVED FROM QUEUE
///
/// \param partitionData  Pointer to the node
///    to be inserted
///
/// \return next node to process
inline
Node* SCHED_PeekNextNode(
    PartitionData *partitionData)
{
    return partitionData->schedulerInfo->PeekNextNode(partitionData);
}


/// Delete a node from the partition's scheduler queue
///
/// \param partitionData  Pointer to the node
///    to be inserted
/// \param node  Pointer to the node
inline
void SCHED_DeleteNode(
    PartitionData *partitionData,
    Node *node)
{
    partitionData->schedulerInfo->DeleteNode(partitionData, node);
}


/// Current node from the partition's scheduler queue
///
/// \param partitionData  Pointer to the node
///    to be inserted
inline
Node* SCHED_CurrentNode(
    const PartitionData *partitionData)
{
    return partitionData->schedulerInfo->CurrentNode(partitionData);
}


/// Check for nodes in the scheduler's node queue.
///
/// \param partitionData  Pointer to the partition data
///    
///
/// \return scheduler has nodes in it's queue
inline
BOOL SCHED_HasNodes(
    const PartitionData *partitionData)
{
    return partitionData->schedulerInfo->HasNodes(partitionData);
}


/// Get the next event for this partition
///
/// \param partitionData  Pointer to the partition data
/// \param node  Pointer to the node
///
/// \return next event
inline
Message* SCHED_NextEvent(
    PartitionData *partitionData,
    Node *node)
{
    return partitionData->schedulerInfo->NextEvent(partitionData, node);
}

/// Get the next event time for this partition
///
/// \param partitionData  Pointer to the partition data
///
/// \return time of next event
inline
clocktype SCHED_NextEventTime(
    const PartitionData *partitionData)
{
    return partitionData->schedulerInfo->NextEventTime(partitionData);
}


/// Initalize Scheduler
///
/// \param partitionData  Pointer to the partition data
/// \param NodeInput  Pointer to the node input data
void SCHED_Initialize(
    PartitionData *partitionData,
    const NodeInput *nodeInput);


/// Finalize Scheduler
///
/// \param partitionData  Pointer to the partition data
void SCHED_Finalize(
    PartitionData *partitionData);

//---------------------------------------------------------------------

#endif /* SCHEDULER_H */
