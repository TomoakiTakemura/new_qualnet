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

/// \defgroup Package_PARTITION PARTITION

/// \file
/// \ingroup Package_PARTITION
/// This file contains declarations of some functions for partition threads.

#ifndef PARTITION_H
#define PARTITION_H

#include <stdio.h>

#include "clock.h"
#include "coordinates.h"
#include "main.h"
#include "mapping.h"
#include "message.h"
#include "terrain.h"
#include "mobility.h"
#include "splaytree.h"
#include "weather.h"
#include "dynamic.h"
#include <map>
#include <vector>
#include <list>
#include <string>
#include "external.h"
#include "simplesplay.h"
#include "sched_std_library.h"
#include "prop_mimo.h"

#include "prop_flat_binning.h"
#include "UrbanCache.h"

#ifdef PARALLEL //Parallel
#include "parallel.h"
#endif //endParallel
#include "spectrum.h"

namespace Proc
{
    namespace DB
    {
        class StatsDBController;
    }
}

///
/// This enumeration contains indexes into the PartitionGlobalData array
/// used for module data. Currently not used for any QualNet protocols.
enum PartitionGlobalDataIndex
{
    PartitionGlobalDataCount = 4 // leave some room for additional data entries
};

/// The number of percentage complete statements to print
#define NUM_SIM_TIME_STATUS_PRINTS 100

/// A default unitialized communication ID.
#define COMMUNICATION_ID_INVALID  0

/// A value to indicate real time interpartition communication
#define COMMUNICATION_DELAY_REAL_TIME -1

///
/*union MessageListCell {
    Message messageCell;
    union MessageListCell* next;
};*/

///
union MessagePayloadListCell {
    char payloadMemory[MAX_CACHED_PAYLOAD_SIZE];
    union MessagePayloadListCell* next;
};

///
union MessageInfoListCell {
    char infoMemory[SMALL_INFO_SPACE_SIZE];
    union MessageInfoListCell* next;
};

///
union SplayNodeListCell {
    SplayNode splayNodeCell;
    union SplayNodeListCell* next;
};


typedef Int64 EventCounter;

typedef int CommunicatorId;
typedef void (*PartitionCommunicationHandler) (PartitionData *, Message *);

typedef std::map <std::string, int >             PartitionCommunicatorMap;
typedef std::map <std::string, int >::iterator   PartitionCommunicatorMapIter;

typedef std::vector <Node *>                     NodePointerCollection;
typedef std::vector <Node *>::iterator           NodePointerCollectionIter;

typedef std::map <std::string, void *>           ClientStateDictionary;
typedef std::map <std::string, void *>::iterator ClientStateDictionaryIter;

// SendMT
class QNThreadMutex;

// SendMT
class QNThreadMutex;
extern clocktype PrintSimTimeInterval;

struct StatsDb;

class MessageSendRemoteInfo;

#include <memory> // std::allocator

//PARALLEL satcom stuff...
/// Data structure containing interfaceIndex and
/// Node* for a node in a single subnet
struct SubnetMemberData
{
    Node*   node;
    NodeId  nodeId;
    int     interfaceIndex;
    Address address;
    int     partitionIndex;
};


/// Data structure containing member data info
/// for all nodes in a single subnet
struct PartitionSubnetList
{
    SubnetMemberData *memberList;
    int numMembers;
};


/// Data structure containing subnet member data
/// for all subnets
struct PartitionSubnetData
{
    PartitionSubnetList* subnetList;
    int numSubnets;
};

struct PartitionPathLossSample
{
    clocktype sampleTime;
    int nodeId1;
    int nodeId2;
    double* pathloss;
};

// Forward declaration
class STAT_StatisticsList;
class PHY_CONN_NodePositionData;
class RecordReplayInterface;

/// \class HITLInfo
/// \brief  Handles the info for one HITL command.
/// \Info includes the node ID, the instance ID of the app, and the victim node ID.
class HITLInfo
{
public:
    NodeAddress nodeId;

    NodeAddress victimNodeId;

    HITLInfo(NodeAddress nodeId,
             NodeAddress victimNodeId) : nodeId(nodeId),
                                         victimNodeId(victimNodeId)
    {}

    HITLInfo(NodeAddress nodeId) : nodeId(nodeId),
                                  victimNodeId(0)
    {}

};

/// \class HITLApplication
/// \brief handle application type and info
class HITLApplication
{
public:
    /// application type
    AppType appType;

    /// HITLInfo vector
    std::vector<HITLInfo> apps;
};

/// Contains global information for this partition.
struct PartitionData {
private:
    clocktype   m_theCurrentTime;
    int m_numPartitions;
    int m_numThreadsPerPartition;

public:
    PartitionData(int thePartitionId, int numPartitions, int numOPHosts);

    int partitionId;        // Identifier for this partition
    D_Int32 numNodes;       // Number of nodes in the entire simulation.
    int seedVal;

    NodePositions *nodePositions;

    TerrainData* terrainData;

    AddressMapType *addressMapPtr;
    IdToNodePtrMap        nodeIdHash[32];

    clocktype   safeTime;
    clocktype   nextInternalEvent;
    clocktype   externalInterfaceHorizon;
    D_Clocktype theCurrentTimeDynamic; // dynamic copy of theCurrentTime
    D_Clocktype maxSimClock;
    double      startRealTime;
    clocktype    mimoUpdateInterval;  // how often to recalculate the random part of the mimo propagation
    const MIMO_TGn_cluster* mimo_Model;  // which tgn model to use

    BOOL guiOption;

    NodeInput* nodeInput;
    Node**     nodeData;      /* Information about all nodes */
    void* globalData[PartitionGlobalDataCount];  // Global Data

    int          numChannels;
    int          numFixedChannels;
    PropChannel* propChannel;

    // Pathloss Matrix value
    // moved from PropProfile to here as PropProfile is shared by all partitions
    // This is an array with length equal to numChannels;
    map <pair<NodeId, NodeId>, double>** pathLossMatrix;
    int plCurrentIndex;
    clocktype plNextLoadTime;

    int          numProfiles;

    /*
     * This is a pointer to a node in this partition. A node keeps pointers
     * to other nodes in the same partition.
     * If this partitcular node moves out of this partition, this variable
     * will also have to be updated.
     */
    Node       *firstNode;
    // In MPI only, this is a list of remote nodes. Remote nodes
    // are "shadow" nodes and have a very small subset of capabilities.
    // Note, in shared memory all nodes will remain in the normal list.
    Node        *firstRemoteNode;
    // This container holds references to all nodes, in order, for
    // both local and remote nodes.
    NodePointerCollection * allNodes;

    int                     msgFreeListNum;
    Message                *msgFreeList;
    int                     msgPayloadFreeListNum;
    MessagePayloadListCell *msgPayloadFreeList;
    int                     msgInfoFreeListNum;
    MessageInfoListCell    *msgInfoFreeList;
    int                     splayNodeFreeListNum;
    SplayNodeListCell      *splayNodeFreeList;

    // This number is used to break times when ordering events.
    // This sequence number will wrap, but that is ok as we
    // don't anticipate ever having 2^32-1 events scheduled on the same
    // node for the same simulation time.
    unsigned int            eventSequence;  // for natural ordering
    /*
     * When SCHEDULER is SPLAYTREE
     * Each node keeps a splay tree of all its future messages.
     * The partition keeps a heap of all the nodes in the partition,
     * so that we can easily retrieve the earliest message in this
     * particular partition.
     */
    HeapSplayTree heapSplayTree;
    /*
     * When SCHEDULER is STDLIB
     */
    StlHeap *    heapStdlib;

    MobilityHeap mobilityHeap;
    StlHeap *       looseEvsHeap;

    // Generic event queue, for events not assigned to a particular node
    SimpleSplayTree genericEventTree;

    // these need to be processed last after all other partition and node events
    // ex: statsDB timer events,
    SimpleSplayTree processLastEventTree;

    EventCounter numberOfEvents;
    EventCounter numberOfMobilityEvents;

    /*
     * Weather related variables.
     */
    WeatherPattern** weatherPatterns;
    int              numberOfWeatherPatterns;
    int              weatherMobilitySequenceNumber;
    clocktype        weatherMovementInterval;

    FILE    *statFd;       /* file descriptor used for statistics */

    BOOL    traceEnabled;
    FILE    *traceFd;      /* file descriptor used for packet tracing */

    Node    *activeNode;

    BOOL realTimeLogEnabled;
    FILE *realTimeFd;   /* file descriptor for real time log */
    EXTERNAL_InterfaceList interfaceList;
    D_Hierarchy dynamicHierarchy;
    STAT_StatisticsList* stats;

    EXTERNAL_Interface *    interfaceTable [EXTERNAL_TYPE_MAX];
    CommunicatorId          externalForwardCommunicator;
    CommunicatorId          externalSimulationDurationCommunicator;


    SchedulerInfo   *schedulerInfo;     // Pointer to the info struct for the schedular
    // to be used with this partition

    int                   numAntennaModels;
    AntennaModelGlobal    *antennaModels;  // Global Model list for partition
    int                   numAntennaPatterns;
    AntennaPattern        *antennaPatterns;//Global pattern list for partition

    WallClock *           wallClock;

    // Distribution Stuff
    UserProfileDataMapping *userProfileDataMapping;
    TrafficPatternDataMapping *trafficPatternMapping;

    // RegisteredCommunicators
    // Map of registered communicator names and their corresponding array indexes
    // std::map <const char *, int, CharComparitor>         communicators;
    PartitionCommunicatorMap *          communicators;
    PartitionCommunicationHandler *     handlerArray;
    int                                 nextAvailableCommunicator;
    int                                 communicatorArraySize;
    // mutex       communicatorsLock;
    bool                                communicatorsFrozen;

    // Dictionary of Client state
    ClientStateDictionary *             clientState;

#ifdef PARALLEL //Parallel
    // list of "shadow" node pointers for other partitions
    IdToNodePtrMap        remoteNodeIdHash[32];
    LookaheadCalculator*  lookaheadCalculator;
    clocktype             reportedEOT;
    bool                  looseSynchronization;

    // MAPPING relate partition communicator IDs.
    CommunicatorId          mappingAddrChgCmtr;
#endif // PARALLEL

    bool                  isRealTime;

#ifdef ADDON_NGCNMS
    // Used for main flat binning grid
    GridInfo* gridInfo;
    clocktype* gridUpdateTime;
    BOOL* gridAutoBuild;

    // Used for grid based pathloss matrix
    GridInfo* pathlossMatrixGrid;
    int numUsedMatrix;
    int numMatrixErased;
    int numMatrixAdded;

    char ngcHistFilename[MAX_STRING_LENGTH];
    char ngcStatFilename[MAX_STRING_LENGTH];
    clocktype ngcHistUpdateInterval;
    BOOL isLazy;
    BOOL isGreedy;
    double errorBound;
#endif

    // Subnet list array
    PartitionSubnetData subnetData;

    // for pathloss matrix
    PartitionPathLossSample **pathlossSampleArray;
    BOOL isCreatePathLossMatrix;
    clocktype pathlossSampleTimeInterval;
    int pathlossSampleIndex;
    char matrixFilename[MAX_STRING_LENGTH];

    BOOL isCreateConnectFile;
    clocktype connectSampleTimeInterval;
    char connectFilename[MAX_STRING_LENGTH];

    int **ConnectionTable;
    int *NodeMappingArray;
    int ***ConnectionTablePerChannel;
    int ***ConnectionTablePerPhyIndex;

#ifdef ADDON_DB
    // STATS DB CODE
    StatsDb* statsDb;
#endif

    PHY_CONN_NodePositionData* nodePositionData;


    //Address of forwarding node for cross-partition forward messages
    NodeAddress EXTERNAL_lastIdToInvokeForward;

    // Whether interface to AGI STK is enabled.
    BOOL isAgiInterfaceEnabled;

    BOOL isEmulationMode;
    BOOL isAutoIpne;
    BOOL isRtIndicator;
    int masterPartitionForCluster;
    int clusterId;
    BOOL partitionOnSameCluster[32];
    clocktype delayExt;
    double dropProbExt;
    std::map<int, int> *virtualLanGateways;
    RecordReplayInterface *rrInterface;
    int m_numOHAvailable;

    // The UrbanCache may be created by the urban terrain processing to retain
    // the path data for stationary objects. It is set byt the specific urban
    // terrain processing. It is kept in the partition for thread safety.
    UrbanCache* urbanCache;

    int getNumPartitions() { return m_numPartitions; }
    void setNumPartitions(int numPartitions) { m_numPartitions = numPartitions; }

    bool canAddOH() { return m_numOHAvailable > 0; }
    bool addOpHost();
    void removeOpHost();
    int getOH() { return m_numOHAvailable; }

    // Return TRUE if the simulation is running in parallel
    BOOL isRunningInParallel() { return m_numPartitions + m_numThreadsPerPartition > 2; }

    /// Returns the simulation time at a global level
    /// For the current time of a node, use Node::getNodeTime().
    /// PartitionData::getGlobalTime() should only be used for
    /// timing at the partition or global level.
    ///
    /// \return The current global simulation time
    clocktype getGlobalTime() const { return m_theCurrentTime; }

    void setTime(clocktype t) { m_theCurrentTime = t; }





    PARALLEL_PropDelay_NodePositionData *dynamicPropDelayNodePositionData;

    // SendMT
    QNThreadMutex*         sendMTListMutex;
    std::list <Message*>*  sendMTList;
    Proc::DB::StatsDBController* dbController;

#ifdef CYBER_LIB
    std::map<std::string, HITLApplication> hitlApplicationMap;
#endif // CYBER_LIB
    spectrum theSpectrum;
    // Users should not modify anything above this line.
};

/// Global properties of the simulation for all partitions.
struct SimulationProperties
{
    clocktype   maxSimClock;          // max clock used in simulation
    clocktype   startSimClock;        // start clock used in simulation
    std::string qualnetHomePath;
    std::string configFileName;
    BOOL        noMiniParser; // TRUE if we should not use the mini parser

    SimulationProperties() : maxSimClock(0), startSimClock(0),
        noMiniParser(false)
    {}
};

/// Inline function used to get terrainData pointer.
///
/// \param partitionData  pointer to partitionData
inline
TerrainData* PARTITION_GetTerrainPtr(PartitionData* partitionData)
{
    return partitionData->terrainData;
}

/// Function used to allocate and perform inititlaization of
/// of an empty partition data structure.
///
/// \param partitionId  the partition ID, used for parallel
/// \param numPartitions  for parallel
PartitionData* PARTITION_CreateEmptyPartition(
    int             partitionId,
    int             numPartitions,
    int             numOPHosts = 0);

/// Function used to initialize a partition.
///
/// \param partitionData  an empty partition data structure
/// \param terrainData  dimensions, terrain database, etc.
/// \param maxSimClock  length of the scenario
/// \param startRealTime  for synchronizing with the realtime
/// \param numNodes  number of nodes in the simulation
/// \param traceEnabled  is packet tracing enabled?
/// \param addressMapPtr  contains Node ID <--> IP address mappings
/// \param nodePositions  initial node locations and partition assignments
/// \param nodeInput  contains all the input parameters
/// \param seedVal  the global random seed
/// \param nodePlacementTypeCounts  gives information about node placemt
/// \param experimentPrefix  the experiment name
/// \param startSimClock  the simulation starting time
void PARTITION_InitializePartition(
    PartitionData * partitionData,
    TerrainData*    terrainData,
    clocktype       maxSimClock,
    double          startRealTime,
    int             numNodes,
    BOOL            traceEnabled,
    AddressMapType* addressMapPtr,
    NodePositions*  nodePositions,
    NodeInput*      nodeInput,
    int             seedVal,
    int*            nodePlacementTypeCounts,
    char*           experimentPrefix,
    clocktype       startSimClock);

/// Function used to allocate and initialize the nodes on a
/// partition.
///
/// \param partitionData  an pre-initialized partition data structure
void PARTITION_InitializeNodes(PartitionData* partitionData);


/// Finalizes the nodes on the partition.
///
/// \param partitionData  an pre-initialized partition data structure
void PARTITION_Finalize(PartitionData* partitionData);


/// Creates and initializes the nodes, then processes
/// events on this partition.
///
/// \param partitionData  an pre-initialized partition data structure
void* PARTITION_ProcessPartition(PartitionData* partitionData);

/// Messages sent by worker threads outside of the main
/// simulation event loop MUST call MESSAGE_SendMT ().
/// This funciton then is the other half - where the multi-thread
/// messages are properly added to the event list.
///
/// \param partitionData  an pre-initialized partition data structure
void
PARTITION_ProcessSendMT (PartitionData * partitionData);

/// Returns a pointer to the node or NULL if the node is not
/// on this partition.  If remoteOK is TRUE, returns a pointer
/// to this partition's proxy for a remote node if the node
/// does not belong to this partition.  This feature should
/// be used with great care, as the proxy is incomplete.
/// Returns TRUE if the node was successfully found.
///
/// \param partitionData  an pre-initialized partition data structure
/// \param node  for returning the node pointer
/// \param nodeId  the node's ID
/// \param remoteOK  is it ok to return a pointer to proxy node?
///
/// \return returns TRUE if the node was succesfully found
BOOL PARTITION_ReturnNodePointer(PartitionData* partitionData,
                                 Node**         node,
                                 NodeId         nodeId,
                                 BOOL           remoteOK = FALSE);


/// Determines whether the node ID exists in the scenario.
/// Must follow node creation.
///
/// \param partitionData  an pre-initialized partition data structure
/// \param nodeId  the node's ID
BOOL PARTITION_NodeExists(PartitionData* partitionData,
                          NodeId         nodeId);


/// If dynamic statistics reporting is enabled,
/// generates statistics for enabled layers.
///
/// \param partitionData  an pre-initialized partition data structure
void PARTITION_PrintRunTimeStats(PartitionData* partitionData);

/// Schedules a generic partition-level event.
///
/// \param partitionData  an pre-initialized partition data structure
/// \param msg  an event
/// \param eventTime  the time the event should occur
/// \param scheduleBeforeNodes  process event before or after node events
void PARTITION_SchedulePartitionEvent(PartitionData* partitionData,
                                      Message*       msg,
                                      clocktype      eventTime,
                                      bool  scheduleBeforeNodes = true);

/// An empty function for protocols to use that need to
/// schedule and handle partition-level events.
///
/// \param partitionData  an pre-initialized partition data structure
/// \param msg  an event
void PARTITION_HandlePartitionEvent(PartitionData* partitionData,
                                    Message*       msg);

/// Sets or replaces a pointer to client-state, identifed by name,
/// in the indicated partition.
/// Allows client code, like external iterfaces, to store
/// their own data in the partition. The client's state pointer
/// is set and found by name. If the caller passes a name for
/// client state that is already being stored, the state pointer
/// replaces what was already there.
///
/// \param partitionData  an pre-initialized partition data structure
/// \param stateName  Name used to locate this client state
///    information
/// \param clientState  Pointer to whatever data-structure the
///    client wishes to store.
void PARTITION_ClientStateSet(PartitionData* partitionData,
                              const char*    stateName,
                              void*          clientState);

/// Looks up the requested client-state by name. Returns NULL
/// if the state isn't present.
///
/// \param partitionData  an pre-initialized partition data structure
/// \param stateName  Name used to locate this client state
///    information
///
/// \return returns the client state
void* PARTITION_ClientStateFind(PartitionData* partitionData,
                                const char*    stateName);

/// Allocates a message id and registers the handler
/// that will be invoked to receive callbacks
/// when messages are with the id are sent.
///
/// \param partitionData  an pre-initialized partition data structure
/// \param name  Your name for this type of message.
///    Must be unique in the simulation.
/// \param handler  Function
///    to call for processing this type of message.
///
/// \return used to later when calling MESSAGE_Alloc().
CommunicatorId PARTITION_COMMUNICATION_RegisterCommunicator (
    PartitionData*                partitionData,
    const char*                   name,
    PartitionCommunicationHandler handler);

/// Locate an already registered commincator.
///
/// \param partitionData  an pre-initialized partition data structure
/// \param name          : std:  Your name for this type of message.
///
/// \return found communicator Id or COMMUNICATION_ID_INVALID
/// if not found.
CommunicatorId PARTITION_COMMUNICATION_FindCommunicator (
    PartitionData* partitionData,
    std::string    name);


/// Transmit a message to a partition.
///
/// \param partitionData  an pre-initialized partition data structure
/// \param partitionId  partition to send the message to
/// \param msg  Message to send. You are required to follow
///    several rules in regard to the message's contents.
///    The contents must not contain pointers. The message
///    should no longer be modified or freed after calling this
///    function.
/// \param delay  When the message should execute. Special delay
///    value of COMMUNICATION_DELAY_REAL_TIME which means that
///    the receiving partition will process the message
///    as soon as possible. Note, that if you want repetable
///    and consistent simulation results you can only use this delay
///    value for processing that won't affect simulation event
///    ordering. e.g. your msg is to trigger a call to an external
///    program.
void PARTITION_COMMUNICATION_SendToPartition(
    PartitionData* partitionData,
    int            partitionId,
    Message*       msg,
    clocktype      delay);

/// Transmit a message to all partitions.
///
/// \param partitionData  an pre-initialized partition data structure
/// \param msg  Message to send. You are required to follow
///    several rules in regard to the message's contents.
///    The contents must not contain pointers. The message
///    should no longer be modified or freed after calling this
///    function.
/// \param delay  When the message should execute. Special delay
///    value of COMMUNICATION_DELAY_REAL_TIME which means that
///    the receiving partition will process the message
///    as soon as possible. Note, that if you want repetable
///    and consistent simulation results you can only use this delay
///    value for processing that won't affect simulation event
///    ordering. e.g. your msg is to trigger a call to an external
///    program.
void PARTITION_COMMUNICATION_SendToAllPartitions(
    PartitionData* partitionData,
    Message*       msg,
    clocktype      delay);
/*
 * FUNCTION     PARTITION_GetRealTimeMode ()
 * PURPOSE      Returns true if the simulation should execute
 *              keeping up, but not faster than,
 *              real time. e.g. IPNE or HLA time managed.
 *
 * Parameters
 *      partitionData: a pre-initialized partition data structure
 *
 */
bool
PARTITION_GetRealTimeMode (PartitionData * partitionData);

/*
 * function     PARTITION_SetRealTimeMode ()
 * purpose      Simulation should execute keeping up, but not faster than,
 *              real time. Examples are IPNE or HLA time managed.
 *
 * parameters
 *      partitionData: a pre-initialized partition data structure
 *      runAsRealtime: true to indicate real time execution
 */
void
PARTITION_SetRealTimeMode (PartitionData * partition, bool runAsRealtime);

/*
 * FUNCTION     UpdateNextInternalEventTime
 * PURPOSE      Calculates the time of the next internal event and puts it in
 *              partitionData->nextInternalEvent;
 */
void UpdateNextInternalEventTime(PartitionData* partitionData);

/*
 * FUNCTION     PARTITION_GlobalInit
 * PURPOSE      Initializes process variables before partitions are
 *              created
 */
void
PARTITION_GlobalInit(NodeInput* nodeInput,
                     int numberOfProcessors,
                     char* experimentPrefix);

// The following functions may belong in non-partition header and cfile
// And are used in the licencing
/// This will return in a string the current directory
/// qualnet is executing from
///
///
/// \return string containing current qualnet directory
std::string IO_ReturnBaseDirectory();


/// This will return a boolean true if file exists, and false if not
///
///
/// \return boolean true/false if file exists
BOOL IO_SourceFileExists(std::string fileToTest);


/// This will return in a string the formatted
/// yes/no line for whether the fingerprint file exists for given
/// library
///
///
/// \return string containing list of libraries
std::string IO_CheckSourceLibrary(std::string filePath);


/// This will return in a string a list of libraries
/// currently compiled into product as well as those which
/// have source code available.
///
///    std::string licensePath : path of licence file including file itself
///    BOOL onlyForGUI : Print additional info for GUI.
///    BOOL validTS : A valid license exists in Trusted Storage or not
///    int *expirationDates : NULL it means that we just want to get
///                           the type of license.
///
/// \return string containing list of libraries
std::string IO_ReturnSourceAndCompiledLibraries(std::string licensePath,
                                                BOOL onlyForGUI,
                                                BOOL validTS = TRUE,
                                                int *expirationDates = NULL);

/// This will return in a string the status message for the library
///
///    std::string library: library to be tested for
///    std::string licensePath: path of licene file including file itself
///    BOOL validTS : A valid license may exist in Trusted Storage if true
///    int *expirationDates : If NULL it means that we just want to get
///                           the type of license.
///
/// \return string containing the status message for the library
std::string IO_ReturnLibraryStatus(std::string library,
                                    std::string licensePath,
                                    BOOL validTS = TRUE,
                                    int *expirationDates = NULL);

/// This will return true if it is a node-locked license.
///
///    std::string licensePath : path of licence file including file itself
///
/// \return true if it is a node-locked license
bool IO_IsNodeLocked(std::string licensePath);

/// This will return in a string a list of libraries
/// currently compiled into product as well as those which
/// have source code available.
///
///    std::string featureName: name of feature to look for
///
/// \return string containing expiration date for this feature
std::string IO_ReturnExpirationDateFromLicenseFeature(vector<std::string> *licenseLines, std::string featureName);

/// This will return in a string the expiration date of the library
///
///    :: BOOL onlyForGUI - additional information printed for gui
///
/// \return string containing expiration date for this feature
std::string IO_ReturnExpirationDateFromNumericalDate(int numericalExpiration,BOOL onlyForGUI);

/// This will return in a string the expiration date of the library
///
///    :: BOOL onlyForGUI - additional information printed for gui
///
/// \return string containing expiration date for this feature
std::string IO_ReturnExpirationDateFromNumericalDate(int numericalExpiration,BOOL onlyForGUI);

/// Parse a FlexLM date in a platform safe way
///
///
/// \return UInt64 containing the date
UInt64 IO_ParseFlexLMDate(const char* date);

/// This will return in a string the status message for the library
/// used with the -print_libraries option
///
/// \param expDate: Date when the license for this
///                 library expires
/// \param binaryStatus: true/false if library compiled in
/// \param fileName: Additional file to be checked for
///                  existence in order for the status to be ok
/// \return string containing status message for library
std::string IO_ReturnStatusMessageFromLibraryInfo(std::string expDate, BOOL binaryStatus, std::string fileName);

/// This will return in a string the library name from its index
//           :: because flexlm won't allow std strucsts in main.cpp
//           :: but main.cpp is the only place flex structs are allowed
///
///
/// \return string containing library name
std::string IO_ReturnLibraryNameFromAbsoluteIndex(int index);

/// Print standard QualNet progress log
///
///    char* message : The message to print (eg, Completed 5%)
///    bool printSimTIme : Whether to print sim time and real time
///    bool printDateTime : Whether to print the current date and
///    time in addition to the progress
void PARTITION_ShowProgress(
    PartitionData* partitionData,
    const char* message,
    bool printSimTime = true,
    bool printDateTime = false);

#ifdef GPROF
#ifndef USE_MPI
#include <sys/time.h>
extern struct itimerval g_itimer;
#endif
#endif

/* FUNCTION     PARTITION_SetSimulationEndTime
 * PURPOSE      To end the simulation in middle of execution
 *                Typically called by external interfaces, or upon
 *                external interrupts.
 *
 * Parameters
 *    partitionData: ParitionData *: pointer to partitionData
 *    duration : clocktype : interval after which the simulation must end
 */
void PARTITION_SetSimulationEndTime(
    PartitionData *partitionData,
    clocktype duration);

/*
 * API :: PARTITION_RequestEndSimulation
 * PURPOSE:: Request the simulation to end now
 */
void PARTITION_RequestEndSimulation();


/* FUNCTION     PARTITION_PrintUsage
 * PURPOSE      Prints the command-line usage of the application.
 *
 * Parameters
 *    commandName: const char*: The command name of the application.
 */
void PARTITION_PrintUsage( const char* commandName );


/* FUNCTION     PARTITION_ParseArgv
 * PURPOSE      Reads arguments from command line.  Prints usage if arugment
 *              processing fails for any reason.  Returns FALSE if execution
 *              should stop due to the processed arguments, returns TRUE
 *              otherwise.
 *
 * Parameters
 *    argc: int: Number of arguments to the command line
 *    argv: char **: Array of arguments to the command line
 *    seedVal: int: Used to set seedVal in the kernel
 *    seedValSet: BOOL: Test if seedVal is set in the kernel
 *    simProps: SimulationProperties: Used to set simProps in kernel
 *    onlyVerifyLicense: BOOL: Used to set onlyVerifyLicense in kernel
 *    onlyPrintLibraries: BOOL: Used to set onlyPrintLibraries in kernel
 *    onlyForGUI: BOOL: Used to set onlyForGUI in kernel
 *    sopsvopsInterface: BOOL: If the sopsvops interface should be enabled
 *    sopsPort: int: Used to set sopsPort in the kernel
 *    isEmulationMode: BOOL: Used to set isEmulationMode in kernel
 *    dbRegression: BOOL: If Stats DB regression should be performed
 *    numberOfPartitions int: Used to set numberOfPartitions in kernel
 *    numberOfThreadsPerPartition int: Used to set numberOfThreadsPerPartition in kernel
 *    experimentPrefix: char *: Used to set experimentPrefix in kernel
 *    statFileName: char *: Used to set statFileName in kernel
 *    traceFileName: char *: Used to set traceFileName in kernel
 *    product_info: std::string: The product info printed if -version is an argument
 *    g_looseSynchronization: bool: Used to set g_looseSynchronization in kernel
 *    g_forceEot: bool: Force lookahead calculator to use eot mode
 *    g_allowLateDeliveries: bool: Used to set g_allowLateDeliveries in kernel
 *    g_useDynamicPropDelay: bool: Used to set g_useDynamicPropDelay in kernel
 *    g_useRealTimeThread: bool: Used to set g_useRealTimeThread in kernel
 *    g_looseLookahead: clocktype Used to set g_looseLookahead in kernel
 *    g_syncAlgorithm: SynchronizationAlgorithm: Used to set g_syncAlgorithm in kernel
 *    sopsProtocol: SopsProtocol: Protocol for SOPS RPC
 */
BOOL PARTITION_ParseArgv(
    int argc,
    char **argv,
    int &seedVal,
    BOOL &seedValSet,
    SimulationProperties &simProps,
    BOOL &onlyVerifyLicense,
    BOOL &onlyPrintLibraries,
    BOOL &onlyForGUI,
    BOOL &sopsvopsInterface,
    int &sopsPort,
    BOOL &isEmulationMode,
    BOOL &dbRegression,
    int &numberOfPartitions,
    int &numberOfThreadsPerPartition,
    char *experimentPrefix,
    char *statFileName,
    char *traceFileName,
    const std::string &product_info,
#ifdef PARALLEL //Parallel
    bool &g_looseSynchronization,
    bool &g_forceEot,
    bool &g_allowLateDeliveries,
    bool &g_useDynamicPropDelay,
    bool &g_useRealTimeThread,
    clocktype &g_looseLookahead,
    SynchronizationAlgorithm &g_syncAlgorithm,
#endif //endParallel
    SopsProtocol &sopsProtocol
);

/* FUNCTION     PARTITION_RunStatsDbRegression
 * PURPOSE      Run database regression if DB is enabled
 *
 * Parameters
 *    prefix: char*: The name of the experiment
 */
void PARTITION_RunStatsDbRegression(char* prefix);

bool PARTITION_IsMilitaryLibraryEnabled();


/// \brief Keeps track of virtual interfaces.
/// This information is used when processing HITL commands.
/// \param node Pointer to the node
/// \param interfaceIndex index of the virtual interface
void PARTITION_DeclareVirtualInterface(Node *node, int interfaceIndex);

/// \brief Identifies virtual interfaces.
/// This information is used when processing HITL commands.
/// \param node Pointer to the node
/// \param interfaceIndex index of the virtual interface
/// \return true if the interface is virtual
bool PARTITION_IsVirtualInterface(Node *node, int interfaceIndex);

#endif /* _PARTITION_H_ */

