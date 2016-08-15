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

/// \defgroup Package_PARALLEL PARALLEL

/// \file
/// \ingroup Package_PARALLEL
/// This file describes data structures and functions used for parallel programming.

#ifndef PARALLEL_H
#define PARALLEL_H

#ifdef PARALLEL //Parallel

#include <stdio.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <set>

#include "clock.h"
#include "main.h"
#include "mac_link.h"

using namespace std;

// for sending a message to all other partitions
#define PARALLEL_SEND_ALL_OTHERS -1

/// Possible algorithms to use in the parallel runtime.
/// Synchronous is used by default.
enum SynchronizationAlgorithm {
    SYNCHRONOUS,
    REAL_TIME,  // synchronized against the real time clock
    BEST_EFFORT // unsynchronized (currently unimplemented)
};

/// The maximum number of processes that can be used in
/// parallel QualNet.  Customers do not receive parallel.cpp,
/// so cannot effectively change this value.
#define MAX_THREADS 512

/// Type of barrier for synchronization integrity checking.
/// 
/// There should be a unique value for each location in the 
/// code that calls the parallel processing barrier, either
/// by a call to PARALLEL_SynchronizePartitions or to
/// PARALLEL_GetRemoteMessagesAndBarrier. 
/// 
/// When adding a new barrier call, add a new enum value here
/// to use. 
enum BarrierType {
    BARRIER_TYPE_CalculateSafeTimeUsingECOT,
    BARRIER_TYPE_Debug,
    BARRIER_TYPE_EndSimulation,
    BARRIER_TYPE_GlobalReady,
    BARRIER_TYPE_HandleStatsDBMalsrAggregateInsertion,
    BARRIER_TYPE_HandleStatsDBMalsrLoop,
    BARRIER_TYPE_HandleStatsDBOspfEnd,
    BARRIER_TYPE_HandleStatsDBOspfStart,
    BARRIER_TYPE_InitializeNodes,
    BARRIER_TYPE_InitializeSopsVops,
    BARRIER_TYPE_InitModemParams,
    BARRIER_TYPE_NodesCreated,
    BARRIER_TYPE_PhyConSend802_3,
    BARRIER_TYPE_PhyConSend802_3End,
    BARRIER_TYPE_PhyConSendSatCom,
    BARRIER_TYPE_PhyConSendSatComEnd,
    BARRIER_TYPE_PhyLinkConnectivity,
    BARRIER_TYPE_PhyLinkConnectivityEnd,
    BARRIER_TYPE_ProcessPartition,
    BARRIER_TYPE_PropDelaySend_After,
    BARRIER_TYPE_PropDelaySend_Before,
    BARRIER_TYPE_RunPartition,
    BARRIER_TYPE_StatAggregate,
    BARRIER_TYPE_StatAggregateEnd,
    BARRIER_TYPE_StatisticsList,
    BARRIER_TYPE_StatsDbParallelBarrier,
    BARRIER_TYPE_Summarize,
    BARRIER_TYPE_SummarizeBegin,
    BARRIER_TYPE_SummarizeEnd,
    BARRIER_TYPE_MAX
};

/** Lookahead **/

/*
 * Users of Lookahead/EOT should consider LookaheadHandle's to be opaque.
 * Internally they are just integers - for the array slot value
 * inside the vector of LookaheadLocators held in the LookaheadCalculator.
 */
typedef int LookaheadHandle;

/// This struct is allows us to be able to remove from the
/// LookaheadCalculator's heap. This way lookahead handles can
/// request they be removed. Internally, as the heap re-heapifies
/// these locators are updated.
struct LookaheadLocator {
    int     eotHeapIndex;
};

/// Basic data structure for simplifying lookahead calculation.
struct EotHeapElement {

    clocktype eot;
    LookaheadHandle    lookaheadLocatorIndex;
};

/// Stores a heap of EOT elements to calculate lookahead.
struct LookaheadCalculator {
    EotHeapElement*             eotHeap;
    int                         numberElementsInHeap;
    int                         maxElementsInHeap;
    clocktype                   maxLookahead;
    clocktype                   minLookahead;
    bool                        usingEOT;
    bool                        pastInitPhase;
    std::vector <LookaheadLocator>    *lookaheadLocators;

    LookaheadCalculator();

    // Verify that we have EOT elements when running in EOT mode
    void verifyEot();
};

class MessageSendRemoteInfo
{
public:
    MessageSendRemoteInfo (Message * msg, int destPartitionId) :
        m_msg (msg), m_destPartitionId (destPartitionId), m_isOobRequest (false)
    {
    }
    void setOobRequest () { m_isOobRequest = true; }
    bool isOobRequest () { return m_isOobRequest; }

    Message *   m_msg;
    int         m_destPartitionId;        // PARALLEL_SEND_ALL_OTHERS is valid here.
    bool        m_isOobRequest;

};

#define PARALLEL_PROP_DELAY_DEFAULT_INTERVAL (6 * SECOND)
#define PARALLEL_PROP_DELAY_DELTA (0)
//(0.001)

class PARALLEL_PropDelay_NodePositionData
{
public:
     enum MsgType
    {
        NODE_POSITION_INFO
    } ;

    PARALLEL_PropDelay_NodePositionData () {
        dynamicPropDelayEnabled = FALSE;
        dynamicPropDelayInterval = PARALLEL_PROP_DELAY_DEFAULT_INTERVAL;
    }
    typedef std::map<int, Coordinates> NODE_POSITION_DATABASE ;
    typedef NODE_POSITION_DATABASE::const_iterator _C_ITER;
    typedef NODE_POSITION_DATABASE::iterator _ITER;

    bool dynamicPropDelayEnabled ;
    clocktype dynamicPropDelayInterval ;

    Coordinates ReturnNodePosition(int);

    // for all nodes in the system
    vector<NODE_POSITION_DATABASE* > dynamicPropDelayDistanceTb ;

    // for nodes in my partition who moved in the interval
    NODE_POSITION_DATABASE nodesChangedPositionTb ;

    std::map<int, double> shortestDistancePartition ;

#ifdef USE_MPI
    vector<Message*> msgList ;
    vector<Message*> snd_msgList ;
#endif

} ;

void PARALLEL_PropDelayObtainNodesOnOtherPartitionPositions(
    PartitionData* partitionData,
    int            numNodes,
    NodeInput*     nodeInput,
    NodeId*        nodeIdArray,
    NodePositions* nodePositionsPtr) ;

void PARALLEL_PropDelay_SendTimerMessage(
    PartitionData * partitionData);

void PARALLEL_PropDelay_CollectConnectSample(
    Node*    node,
    Message* msg);

clocktype PARALLEL_PropDelay_ReturnPropDelay(
    PartitionData* partition);

//* Parameters related to parallel execution.
extern bool                     g_looseSynchronization;
extern bool                     g_forceEot;
extern clocktype                g_looseLookahead;
extern SynchronizationAlgorithm g_syncAlgorithm;
extern bool                     g_useDynamicPropDelay;
extern int                      g_numberOfSynchronizations;
extern bool                     g_allowLateDeliveries;
extern bool                     g_useRealTimeThread;
extern bool                     g_trySomethingNew;

#ifdef USE_MPI
extern clocktype g_mpiLookahead;
extern int g_pipelineFactor;
extern bool g_usePipeline;
extern bool g_optimizeThroughput;
extern bool g_multiMachine;
#endif

/// Obtains a new lookahead handle that allows a protocol
/// to indicate minimum delay values for output. This minimum
/// delay is called EOT - earliest output time.
///
/// \param node  the active node
///
/// \return Returns a reference to the node's lookahead data.
LookaheadHandle PARALLEL_AllocateLookaheadHandle(Node* node);

/// Adds a new LookaheadHandle to the lookahead calculator.
///
/// \param node  the active node
/// \param lookaheadHandle  the node's lookahead handle
/// \param eotOfNode  the node's EOT
void PARALLEL_AddLookaheadHandleToLookaheadCalculator(
    Node*                node,
    LookaheadHandle      lookaheadHandle,
    clocktype            eotOfNode);

/// Protocols that use EOT will make use of this function more than
/// any other to update the earliest output time as the simulation
/// progresses. Use of EOT is an all-or-nothing option. If your
/// protocol uses EOT, it _must_ use EOT pervasively.
///
/// \param node  the active node
/// \param lookaheadHandle  the node's lookahead handle
/// \param eot  the node's current EOT
void PARALLEL_SetLookaheadHandleEOT(Node*           node,
                                    LookaheadHandle lookaheadHandle,
                                    clocktype       eot);

/// Removes a LookaheadHandle from the lookahead calculator.
///
/// \param node  the active node
/// \param lookaheadHandle  the node's lookahead handle
/// \param eotOfNode  the node's current EOT
void PARALLEL_RemoveLookaheadHandleFromLookaheadCalculator(
    Node*                node,
    LookaheadHandle      lookaheadHandle,
    clocktype*           currentEOT);

/// Sets a minimum delay for messages going out on this interface.
/// This is typically set by the protocol running on that interface.
///
/// \param node  the active node
/// \param minLookahead  the protocol's minimum lookahead
void PARALLEL_SetMinimumLookaheadForInterface(Node*     node,
                                              clocktype minLookahead);

/// Initializes lookahead calculation.  For kernel use only.
///
/// \param lookaheadCalculator  the lookahead calculator
void PARALLEL_InitLookaheadCalculator(
    LookaheadCalculator* lookaheadCalculator);


/// Using their positions or other information, assigns
/// each node to a partition. For kernel use only.
///
/// \param numNodes  the number of nodes
/// \param numberOfPartitions  the number of partitions
/// \param nodeInput  the input configuration file
/// \param nodePos  the node positions
/// \param map  node ID <--> IP address mappings
///
/// \return the number of partitions used
int PARALLEL_AssignNodesToPartitions(int                   numNodes,
                                     int                   numberOfPartitions,
                                     NodeInput*            nodeInput,
                                     NodePositions*        nodePos,
                                     const AddressMapType* map);
/// Allows parallel code to determine to what partition a
/// node is assigned.  If a Node* is available, it's much
/// quicker to just look it up directly
///
/// \param nodeId  the node's ID
///
/// \return the partition to which the node is assigned
int  PARALLEL_GetPartitionForNode(NodeId nodeId);

/// Sets global variables and stuff. For kernel use only.
///
/// \param numberOfThreads  the number of processors to use.
void PARALLEL_InitializeParallelRuntime(int numberOfThreads);

/// Creates the threads for parallel execution and starts
/// them running. For kernel use only.
///
/// \param numberOfThreads  the number of threads to create.
/// \param nodeInput  the input configuration
/// \param partitionArray  an array containing the partition data
///    structures to give to each thread.
void PARALLEL_CreatePartitionThreads(int            numberOfThreads,
                                     NodeInput*     nodeInput,
                                     PartitionData* partitionArray[]);

/* -----------------------------------------------------------------
 * FUNCTION    PARALLEL_SynchronizePartitions 
 * PURPOSE     Public api that allow partition creation and others
 //            to synchronize their steps across all partitions.
// PARAMETERS   ::
// + partitionData : PartitionData* : a pointer to the partition
// + barrierType   : BarrierType    : unique ident for verification
 * -----------------------------------------------------------------*/
void
PARALLEL_SynchronizePartitions(
    PartitionData* partitionData, BarrierType barrierType);

/// Synchronize the realtime start time of each partition
/// Requires the wallclocks of each partition to be synced.
/// For kernel use only.
///
/// \param partitionData  a pointer to the partition
void
PARALLEL_SyncSimStartTime(
    PartitionData* partitionData);

/// Collects all the messages received from other partitions.
/// For kernel use only.
///
/// \param partitionData  a pointer to the partition
void PARALLEL_GetRemoteMessages(PartitionData* partitionData);

/// Collects all the messages received from other partitions.
/// This function also acts as a barrier. For kernel use only.
///
/// \param partitionData  a pointer to the partition
/// \param barrierType  unique ident for verification
void PARALLEL_GetRemoteMessagesAndBarrier(
    PartitionData* partitionData, BarrierType barrierType);

/// Collects all the messages received from other partitions.
/// This function also acts as a pipeline sync. For kernel use only.
///
/// \param partitionData  a pointer to the partition
/// \param pipelineTime  pipeline sync time
void PARALLEL_GetRemoteMessagesAndPipeline(
    PartitionData* partitionData, clocktype pipelineTime);

/// This function calculates the current safetime, updating it if 
/// possible. For kernel use only.
///
/// \param partitionData  a pointer to the partition
/// \param lookahead      the lookahead
void PARALLEL_CalculateSafeTimeUsingPipeline(
    PartitionData* partitionData, clocktype lookahead);

/// Sends one or more messages to a remote partition.
/// For kernel use only.
///
/// \param msgList  a linked list of Messages
/// \param partitionData  a pointer to the partition
/// \param partitionId  the partition's ID
///
/// \return :
void PARALLEL_SendRemoteMessages(Message*       msgList,
                                 PartitionData* partitionData,
                                 int            partitionId);

/// Delivers cached messages to all remote partitions.
/// For kernel use only.
///
/// \param partitionData  a pointer to the partition
///
/// \return :
void PARALLEL_DeliverRemoteMessages(PartitionData* partitionData);

/// Sends one or more messages to a remote partition.
/// These messages are oob messages and will be
/// processed immediately.
/// For kernel use only.
///
/// \param msgList  a linked list of Messages
/// \param partitionData  a pointer to the partition
/// \param partitionId  the partition's ID
/// \param isResponse  if it's a response to an OOB message
void PARALLEL_SendRemoteMessagesOob(Message*       msgList,
                                    PartitionData* partitionData,
                                    int            partitionId,
                                    bool           isResponse);
/// Sends a message to all remote partitions, but not the current
/// one. By default, duplicates will be sent to all remote
/// partitions and the original freed, but if freeMsg is false,
/// the original message will not be freed.
///
/// \param msg  the message(s) to send
/// \param partitionData  the sending partition
/// \param freeMsg  whether or not to free the original
///    message.
void PARALLEL_SendMessageToAllPartitions(Message*       msg,
                                         PartitionData* partitionData,
                                         bool           freeMsg = true);

/// Sends one LINK message to a remote partition.
///
/// \param node  the sending node
/// \param msg  the message to be sent
/// \param link  info about the link
/// \param txDelay  the transmission delay, not including propagation
///
/// \return :
void PARALLEL_SendRemoteLinkMessage(Node*     node,
                                    Message*  msg,
                                    LinkData* link,
                                    clocktype txDelay,
                                    WirelessLinkSiteParameters* params);

/// A generic function for calculating the window of safe events
/// For kernel use only.
///
/// \param partitionData  a pointer to the partition
void PARALLEL_UpdateSafeTime(PartitionData* partitionData);

/// Returns the earliest global event time.  Required for
/// interfacing to time-sensitive external programs.
/// For kernel use only.
///
/// \param partitionData  a pointer to the partition
///
/// \return the time of the earliest event across all
/// partitions
clocktype PARALLEL_ReturnEarliestGlobalEventTime(PartitionData* partitionData);

/// Exits from the parallel system, killing threads, etc.
/// For kernel use only.
///
/// \param partitionData  a pointer to the partition
void PARALLEL_Exit(PartitionData* partitionData);

/// Currently, EOT can only be used if supported by all
/// protocols running in the scenario.  If any protocol
/// is not capable, only the minimum lookahead is used.
///
/// \param node  the node's data
void PARALLEL_SetProtocolIsNotEOTCapable(Node* node);

/// Forces the runtime to consider mobility events when calculating
/// EOT/ECOT.  Mobility events are ignored by default.
/// This function should be called during the initialization of
/// models where changes in position or direction of one node may affect
/// the behavior of other nodes.
///
void PARALLEL_EnableDynamicMobility();

/// Tells the kernel to use spin locks on barriers if true, or
/// to use blocking barriers otherwise.  In greedy mode, the
/// Simulator needs a dedicated CPU per partition.
/// 
///
/// \param greedy  should it be greedy or not?
void PARALLEL_SetGreedy(bool isItGreedy = true);

/// Checks whether SetGreedy has been called.
/// 
///
///
/// \return true if greedy mode is enabled.
bool PARALLEL_IsGreedy();

/// Initializes parallel operation.
/// 
///
/// \param partitionData  the partition to initialize.
void PARALLEL_PreFlight(PartitionData* partitionData);

/// Takes a list of messages or an OOB message and schedules
/// them for execution on the current partition.  Typically
/// these messages have arrived from a remote partition.
/// 
///
/// \param partitionData  the partition.
/// \param msgList  a list of normal simulation messages.
/// \param oobMessage  an out of bounds message.
/// \param gotOobMessage  returns true if Oob response is received
/// \param isMT  is this called from a worker thread
void PARALLEL_ScheduleMessagesOnPartition(
    PartitionData* partitionData,
    Message*       msgList,
    Message**      oobMessage = NULL,
    bool*          gotOobMessage = NULL,
    bool           isMT = false);

/// Shuts down the parallel engine, including whatever
/// synchronization is required.
/// 
///
/// \param partitionData  the partition to terminate.
void PARALLEL_EndSimulation(PartitionData* partitionData);

/// Builds the final stat file when running in parallel node.
/// Should only be called once from partition 0.
///
/// \param numPartitions  number of partitions
/// \param statFileName  name of stat file
/// \param experimentPrefix  experiment prefix
void PARALLEL_BuildStatFile(int numPartitions,
                            char* statFileName,
                            char* experimentPrefix);

/// Return the number of synchronizations performed per partition
///
int PARALLEL_NumberOfSynchronizations();

/// Tells the kernel to use an independent thread to constantly
/// update realtime.
///
/// \param partitionData  a pointer to the partition
void PARALLEL_StartRealTimeThread(PartitionData* partitionData);

#endif //endParallel

#endif /* _PARALLEL_H_ */
