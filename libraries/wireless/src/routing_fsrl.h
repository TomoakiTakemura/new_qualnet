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


// PROTOCOL     :: LANMAR
//
// SUMMARY      :: This is the implementation of Landmark Ad Hoc
/// Routing (LANMAR)
/// 
// LAYER        :: Network
/// 
// STATICSTICS  ::
/// + intraScopeUpdate : Number of local scope routing updates
/// + LMUpdate : Number of landmark updates
/// + controlOH : Total routing control overhead in number of bytes
/// + controlPkts : Total number of routing packets
/// + dropInScope : Number of packets dropped due to no local route
/// + dropNoLM : Number of packets dropped due to no landmark info
/// + dropNoDF : Number of packets dropped due to no drifter info
/// + dropNoRoute : Number of packets dropped due to no route
/// All drops other above three reasons
/// 
// CONFIG_PARAM ::
/// + ROUTING-STATISTICS : BOOL : Determins if LANMAR will collect stats
/// + LANMAR-MIN-MEMBER-THRESHOLD : int : Minimum number of members in
/// scope for being a landmark
/// + LANMAR-FISHEYE-SCOPE : int : The scope limit for local routing
/// + LANMAR-FISHEYE-UPDATE-INTERVAL : clocktype : Frequency for local
/// Fisheye routing
/// + LANMAR-LANDMARK-UPDATE-INTERVAL : clocktype : Frequency ofr landmark
/// updates
/// + LANMAR-NEIGHBOR-TIMEOUT-INTERVAL : clocktype : Timeout duration of
/// neighbor list
/// + LANMAR-FISHEYE-MAX-AGE : clocktype : Lifetime of an entry in local
/// topology table
/// + LANMAR-LANDMARK-MAX-AGE : clocktype : Lifetime of an entry in
/// landmark table
/// + LANMAR-DRIFTER-MAX-AGE : clocktype : Lifetime of an entry in
/// drifter table
/// 
// VALIDATION   ::
/// 
// IMPLEMENTED FEATURES ::
/// + Local Scope Routing : Fisheye state routing
/// + Landmark Updates : Distance vector routing
/// 
// OMITTED FEATURES ::
/// 
// ASSUMPTIONS  ::
/// + Nodes are dived into landmark groups as different subnets
/// + Nodes in the same landmark group move as group mobility
/// + Group mobility model is configured
/// 
// STANDARD     :: IETF draft <draft-ietf-manet-lanmar-04.txt>

#ifndef FSRL_H
#define FSRL_H

#include "list.h"

/// Infinite distance in terms of number of hops
#define FISHEYE_INFINITY                        100

/// Default value of the alpha parameter of LANMAR election
#define FISHEYE_DEFAULT_ALPHA                   1.3

/// Default value of the scope for local routing
#define FISHEYE_DEFAULT_SCOPE                   2

/// Default value of the minimal member threshold of
/// LANMAR election algorithm
#define LANMAR_DEFAULT_MIN_MEMBER_THRESHOLD     8

/// Used for avoiding synchronization effect
#define FISHEYE_RANDOM_JITTER                   (100 * MILLI_SECOND)

/// Used for increment the sequence number of a node's
/// own entries in the routing table.
#define FISHEYE_SEQUENCE_NO_INCREMENT           2

/// The maximum size of the control packet
#define FISHEYE_MAX_TOPOLOGY_TABLE  (MAX_NW_PKT_SIZE - sizeof(IpHeaderType))

/// The timeout duration of neighbor list
#define LANMAR_DEFAULT_NEIGHBOR_TIMEOUT_INTERVAL    (6 * SECOND)

/// Routing table update frequency within the fisheye scope
#define LANMAR_DEFAULT_FISHEYE_UPDATE_INTERVAL      (2 * SECOND)

/// Frequency of landmark updates
#define LANMAR_DEFAULT_LANDMARK_UPDATE_INTERVAL     (4 * SECOND)

/// Maximum age of fisheye entries,
/// i.e, lifetime of an entry in local topology table
#define LANMAR_DEFAULT_FISHEYE_MAX_AGE              (6 * SECOND)

/// Maximum age of landmark entries,
/// i.e, lifetime of an entry in landmark table
#define LANMAR_DEFAULT_LANDMARK_MAX_AGE             (12 * SECOND)

/// Maximum age for drifter entries,
/// i.e, lifetime of an entry in drifter table
#define LANMAR_DEFAULT_DRIFTER_MAX_AGE              (12 * SECOND)

/// Type of control packet
typedef enum
{
    FSRL_UPDATE_DV,
    FSRL_UPDATE_TOPOLOGY_TABLE
} FsrlPacketType;


/// Header of the Fisheye routing packet
typedef struct
{
    // packetType needs to be declared first to determine packet type.
    unsigned char packetType;
    unsigned char reservedBits;
    unsigned short packetSize;
} FsrlFisheyeHeaderField;

/// Data unit format of Fisheye routing packet
typedef struct
{
    NodeAddress destAddr;
    UInt32 dataField;//sequenceNumber:24,
                            //numNeighbor:8;
} FsrlFisheyeDataField;

/// Set the value of sequenceNumber for FsrlFisheyeDataField
///
/// \param dataField  The variable containing the value of
///    sequenceNumber and numNeighbor
/// \param sequenceNumber  Input value for set operation
///
static void FsrlFisheyeDataFieldSetSeqNum(UInt32* dataField,
                                          UInt32 sequenceNumber)
{
    //masks sequenceNumber within boundry range
    sequenceNumber = sequenceNumber & maskInt(9, 32);

    //clears the first 24 bits
    *dataField = *dataField & maskInt(25, 32);

    //setting the value of sequence number in data field
    *dataField = *dataField | LshiftInt(sequenceNumber, 24);
}


/// Set the value of numNeighbour for FsrlFisheyeDataField
///
/// \param dataField  The variable containing the value of
///    sequenceNumber and numNeighbor
/// \param numNeighbor  Input value for set operation
///
static void FsrlFisheyeDataFieldSetNumNeighbour(UInt32* dataField,
                                                UInt32 numNeighbor)
{
    //masks numNeighbor within boundry range
    numNeighbor = numNeighbor & maskInt(25, 32);

    //clears the last 8 bits
    *dataField = *dataField & maskInt(1, 24);

    //setting the value of numneighbor in data field
    *dataField = *dataField | numNeighbor;
}


/// Returns the value of sequenceNumber for
/// FsrlFisheyeDataField
///
/// \param dataField  The variable containing the value of
///    sequenceNumber and numNeighbor
///
/// \return UInt32
static UInt32 FsrlFisheyeDataFieldGetSeqNum(UInt32 dataField)
{
    UInt32 sequenceNumber = dataField;

    //clears the last 8 bits
    sequenceNumber = sequenceNumber & maskInt(1, 24);

    //right shifts 8 bits
    sequenceNumber = RshiftInt(sequenceNumber, 24);

    return sequenceNumber;
}


/// Returns the value of numNeighbour for
/// FsrlFisheyeDataField
///
/// \param dataField  The variable containing the value of
///    sequenceNumber and numNeighbor
///
/// \return UInt32
static unsigned char FsrlFisheyeDataFieldGetNumNeighbour(UInt32 dataField)
{
    UInt32 numNeighbour = dataField;

    //clears the first 24 bits
    numNeighbour  = numNeighbour & maskInt(25, 32);

    return (unsigned char) numNeighbour;
}


/// Header of the landmark update packet
typedef struct
{
    // packetType needs to be declared first to determine packet type.
    // This takes the place of 'reserve' field for the time being.
    unsigned char packetType;

    // Not needed.  LANMAR author will remove this field in the next draft.
    unsigned char landmarkFlag;

    unsigned char numLandmark;
    unsigned char numDrifter;
} FsrlLanmarUpdateHeaderField;

/// Data unit format of landmark update packet
typedef struct
{
    NodeAddress landmarkAddr;
    NodeAddress nextHopAddr;
    unsigned char distance;
    unsigned char numMember;
    unsigned short sequenceNumber;
} FsrlLanmarUpdateLandmarkField;

/// Data unit format of drifter
typedef struct
{
    NodeAddress drifterAddr;
    NodeAddress nextHopAddr;
    unsigned short sequenceNumber;
    unsigned char distance;
    unsigned char reservedBits;
} FsrlLanmarUpdateDrifterField;

/// Data structure used for shorest-path calculation
typedef struct fsrl_list_to_array_item_str
{
    NodeAddress nodeAddr;
    int index;
    struct fsrl_list_to_array_item_str* prev;
    struct fsrl_list_to_array_item_str* next;
} FsrlListToArrayItem;


/// Data structure used for shorest-path calculation
typedef struct
{
    int size;
    FsrlListToArrayItem* first;
} FsrlListToArray;


/// information of one neighbor node
typedef struct fsrl_neighbor_list_str
{
    // Used during shortest path calculation.
    int* index;

    NodeAddress nodeAddr;

    clocktype timeLastHeard;

    int incomingInterface;

    struct fsrl_neighbor_list_str* next;
} FsrlNeighborListItem;

/// description of neighboring information
typedef struct
{
    /* sequence number of the neigboring list */
    int sequenceNumber;

    /* neighbor number of the destionation node */
    unsigned char numNeighbor;

    FsrlNeighborListItem* list;

} FisheyeNeighborInfo;

/// One entry of the local routing table
typedef struct
{
    NodeAddress destAddr;
    NodeAddress nextHop;
    NodeAddress prevHop;
    int distance;

    int outgoingInterface;
} FisheyeRoutingTableRow;


/// One entry of the local fisheye topology table
/// The topology table is a matrix of link connectivity.
typedef struct fisheye_tt_row_str
{
    NodeAddress linkStateNode;

    // Used during shortest path calculation.
    // Maps link state node to sorted array.
    int* index;

    int distance;

    // Direct neighbors of this link state node.
    FisheyeNeighborInfo* neighborInfo;

    // Needed to age out stale entries.
    clocktype timestamp;
    clocktype removalTimestamp;

    // the incoming interface index of this packet
    // this interface will be the outgoing interface
    // for routing packet later (assuming symmetric
    // wireless links.)
    int incomingInterface;

    // Used only for debugging purposes.
    NodeAddress fromAddr;

    struct fisheye_tt_row_str* next;
} FisheyeTopologyTableRow;


/// Structure of local Fisheye topology table
typedef struct
{
    // start pointer to the TopologyTable
    FisheyeTopologyTableRow* first;
} FisheyeTopologyTable;


/// Structure of local routing table
typedef struct
{
    int rtSize;
    BOOL changed;
    FisheyeRoutingTableRow* row;
} FisheyeRoutingTable;


/// One entry of the landmark table
typedef struct fisheye_lmdv_row_str
{
    NodeAddress destAddr;
    NodeAddress nextHop;
    int numLandmarkMember;
    int distance;
    int sequenceNumber;

    // Needed to age out stale entries.
    clocktype timestamp;

    // For debugging purposes only
    NodeAddress fromAddr;

    int outgoingInterface;

    struct fisheye_lmdv_row_str* prev;
    struct fisheye_lmdv_row_str* next;
} FsrlLandmarkTableRow;


/// Structure of the landmark table
typedef struct
{
    int size;
    int ownSequenceNumber;
    FsrlLandmarkTableRow* row;
} FsrlLandmarkTable;


/// One entry of the drifter table
typedef struct fisheye_dfdv_row_str
{
    NodeAddress destAddr;

    // Next hop from drifter to landmark
    NodeAddress nextHop;

    int distance;
    int sequenceNumber;
    clocktype lastModified;

    int outgoingInterface;

    struct fisheye_dfdv_row_str* next;
} FsrlDrifterTableRow;


/// Structure the drifter table
typedef struct
{
    int size;
    int ownSequenceNumber;
    FsrlDrifterTableRow* row;
} FsrlDrifterTable;


/// Statistics of LANMAR routing
typedef struct
{

    // Total number of TopologyTable updates In the scope
    int intraScopeUpdate;

    // Total number of LM updates
    int LMUpdate;

    // Total Control OH in bytes
    int controlOH;

    // Total Control Pkts
    int controlPkts;

    // drop within the scope
    int dropInScope;

    // drop no LM matching
    int dropNoLM;

    // drop no drifter matching
    int dropNoDF;

    // no route drop
    int dropNoRoute;

} FsrlStats;

/// Prameters of LANMAR routing
typedef struct
{
    // Fisheye scope
    short scope;

    // neighbor time out interval
    clocktype neighborTOInterval;

    // update hop < Scope interval
    clocktype fisheyeUpdateInterval;

    // landmark update interval
    clocktype landmarkUpdateInterval;

    // threshold for landmark election
    int minMemberThreshold;

    // factor for landmark election
    double alpha;

    // Used to expire entries in various tables.
    clocktype fisheyeMaxAge;
    clocktype landmarkMaxAge;
    clocktype drifterMaxAge;
} FsrlParameter;


/// Data structure of LANMAR routing
typedef struct
{
    // keep track of topology table for every node
    FisheyeTopologyTable topologyTable;

    // keep track of routing decisions
    FisheyeRoutingTable routingTable;

    // keep track of landmark distance vector
    FsrlLandmarkTable landmarkTable;

    // keep track of drifters
    FsrlDrifterTable drifterTable;


    int numLandmarkMember[MAX_NUM_PHYS];
    BOOL isLandmark[MAX_NUM_PHYS];

    // keep track of different routing statistics
    FsrlStats stats;

    // Keep track of the parameters of LANMAR protocol
    FsrlParameter parameter;

    NodeAddress ipAddress;
    int numHostBits;

    BOOL statsPrinted;

    RandomSeed seed;

} FsrlData;


//--------------------------------------------------------------------------
// PROTOTYPES FOR FUNCTIONS WITH EXTERNAL LINKAGE
//--------------------------------------------------------------------------

/// Initialization Function for LANMAR Protocol
///
/// \param node  Node pointer that the protocol is being instantiated in
/// \param fsrl  Address of data structure pointer to be filled in
/// \param nodeInput  Pointer to NodeInput
/// \param interfaceIndex  Interface index
///
void FsrlInit(Node* node,
              FsrlData** fsrl,
              const NodeInput* nodeInput,
              int interfaceIndex);

/// Finalization function, print out statistics
///
/// \param node  Node pointer that the protocol is being instantiated in
///
void FsrlFinalize(Node* node);

/// Handle self-scheduled messages such as timers
///
/// \param node  Node pointer that the protocol is being instantiated in
/// \param msg  Pointer to message structure of the event
///
void FsrlHandleProtocolEvent(Node* node,Message* msg);

/// Handle routing packets
///
/// \param node  Node pointer that the protocol is being instantiated in
/// \param msg  Pointer to message structure of the packet
/// \param srcAddr  Source node of the packet
/// \param destAddr  Destination node of the packet
/// \param incomingInterface  From which interface the packet is received
/// \param ttl  TTL value in the IP header
///
void FsrlHandleProtocolPacket(Node* node,
                              Message* msg,
                              NodeAddress srcAddr,
                              NodeAddress destAddr,
                              int incomingInterface,
                              int ttl);

/// Is called when network layer needs to route a packet
///
/// \param node  Node pointer that the protocol is being instantiated in
/// \param msg  Pointer to message structure of the packet
/// \param destAddr  Destination node of the packet
/// \param previousHopAddress  Previous hop node
/// \param PacketWasRouted  For indicating whehter the packet is routed
///    in this function
///
void FsrlRouterFunction(Node* node,
                        Message* msg,
                        NodeAddress destAddr,
                        NodeAddress previousHopAddress,
                        BOOL* PacketWasRouted);

#endif  // FSRL_H
