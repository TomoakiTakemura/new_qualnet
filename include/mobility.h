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

/// \defgroup Package_MOBILITY MOBILITY

/// \file
/// \ingroup Package_MOBILITY
/// 
/// This file describes data structures and functions used by mobility models.

#ifndef MOBILITY_H
#define MOBILITY_H

#include "propagation.h"

typedef Coordinates Velocity;
/// Defines the default distance granurality
#define DEFAULT_DISTANCE_GRANULARITY 1

/// Defines the number of node placement schemes
#define NUM_NODE_PLACEMENT_TYPES 8

/// Defines the number of mobility models
#define NUM_MOBILITY_TYPES       5

/// Specifies different node placement schemes
enum NodePlacementType {
    RANDOM_PLACEMENT = 0,
    UNIFORM_PLACEMENT,
    GRID_PLACEMENT,
    FILE_BASED_PLACEMENT,
    GROUP_PLACEMENT,
    GAUSSIAN_PLACEMENT,
    EXTERNAL_PLACEMENT
};

/// Specifies different mobility models
enum MobilityType {
    NO_MOBILITY = 0,
    RANDOM_WAYPOINT_MOBILITY,
    FILE_BASED_MOBILITY,
    GROUP_MOBILITY
};

/// A Heap that determines the earliest time
struct MobilityHeap {
    clocktype minTime;
    Node **heapNodePtr;
    int heapSize;
    int length;
};


/// Inserts an event.
///
/// \param heapPtr  A pointer of type MobilityHeap.
/// \param node  A pointer to node.
void MOBILITY_InsertEvent(MobilityHeap *heapPtr, Node *node);


/// Deletes an event.
///
/// \param heapPtr  A pointer of type MobilityHeap.
/// \param node  A pointer to node.
void MOBILITY_DeleteEvent(MobilityHeap *heapPtr, Node *node);


/// Inserts an event and sort out the heap downwards
///
/// \param heapPtr  A pointer of type MobilityHeap.
/// \param i  index
void MOBILITY_HeapFixDownEvent(MobilityHeap *heapPtr, int i);


/// 
/// Defines all the element of mobility model.
struct MobilityElement {
    int               sequenceNum;
    clocktype         time;
    Coordinates       position;
    Orientation       orientation;
    double            speed;
    double            zValue;
    BOOL              movingToGround;
};

/// 
/// A structure that defines the next states of the elements of
/// mobility model.
struct MobilityRemainder {
    clocktype   nextMoveTime;
    Coordinates nextPosition;
    Orientation nextOrientation;
    double      speed;
    int         numMovesToNextDest;
    int         destCounter;
    clocktype   moveInterval;
    Coordinates delta;
    BOOL        movingToGround;
};

/// Number of past mobility models stored
#define NUM_PAST_MOBILITY_EVENTS 2


/// 
/// This structure keeps the data related to mobility model.
/// It also holds the variables which are
/// static and variable during the simulation. Buffer caches future
/// position updates as well.
struct MobilityData {
    // static during the simulation
    MobilityType mobilityType;
    D_Float32 distanceGranularity;
    D_BOOL groundNode;

    // variable during the simulation
    RandomSeed        seed;

#ifdef ADDON_NGCNMS
    RandomSeed        groupSeed;
    int               groupIndex;
    Coordinates groupTerrainOrigin;
    Coordinates groupTerrainDimensions;

    // Mobility parameters
    double groupMaxSpeed;
    double groupMinSpeed;
    double internalMaxSpeed;
    double internalMinSpeed;
    clocktype internalMobilityPause;
#endif /* ADDON_NGCNMS */

    int               sequenceNum;
    MobilityElement*  next;
    MobilityElement*  current;
    MobilityElement*  past[NUM_PAST_MOBILITY_EVENTS];

    int               numDests;
    MobilityElement*  destArray;

    MobilityRemainder remainder;

    Coordinates       lastExternalCoordinates;
    clocktype         lastExternalTrueMobilityTime;
    clocktype         lastExternalMobilityTime;
    Velocity          lastExternalVelocity;
    double            lastExternalSpeed;

    //Urban
    bool              indoors;
    //endUrban

    void *mobilityVar;

    int latestExternalMobilityCmdNum;
};

/// Allocates memory for nodePositions and mobilityData
/// Note: This function is called before NODE_CreateNode().
/// It cannot access Node structure
///
/// \param numNodes  number of nodes
/// \param nodeIdArray  array of nodeId
/// \param nodePositions  pointer to the array
///    to be allocated.
/// \param nodePlacementTypeCounts  array of placement type counts
/// \param nodeInput  configuration input
/// \param seedVal  seed for random number seeds
void MOBILITY_AllocateNodePositions(
    int numNodes,
    NodeAddress* nodeIdArray,
    NodePositions** nodePositionsPtr,
    int** nodePlacementTypeCountsPtr,
    NodeInput* nodeInput,
    int seedVal);

/// Initializes most variables in mobilityData.
/// (Node positions are set in MOBILITY_SetNodePositions().)
/// Note: This function is called before NODE_CreateNode().
/// It cannot access Node structure
///
/// \param nodeId  nodeId
/// \param mobilityData  mobilityData to be initialized
/// \param nodeInput  configuration input
/// \param seedVal  seed for random number seeds
void MOBILITY_PreInitialize(
    NodeAddress nodeId,
    MobilityData* mobilityData,
    NodeInput* nodeInput,
    int seedVal);

/// Initializes variables in mobilityData not initialized by
/// MOBILITY_PreInitialize().
///
/// \param node  node being initialized
/// \param nodeInput  structure containing contents of input file
void MOBILITY_PostInitialize(Node *node, NodeInput *nodeInput);

/// Updates the path profiles.
///
/// \param pathProfileHeap  MobilityHeap structure.
/// \param nextEventTime  Next event time.
/// \param upperBoundTime  Upper bound time.
void MOBILITY_UpdatePathProfiles(
    MobilityHeap *pathProfileHeap,
    clocktype nextEventTime,
    clocktype *upperBoundTime);


/// Called at the end of simulation to collect the results of
/// the simulation of the mobility data.
///
/// \param node  Node for which results are to be collected.
void MOBILITY_Finalize(Node* node);


/// Models the behaviour of the mobility models on receiving
/// a message.
///
/// \param node  Node which received the message
void MOBILITY_ProcessEvent(Node* node);


/// Adds a new destination.
///
/// \param mobilityData  MobilityData of the node
/// \param arrivalTime  Arrival time
/// \param dest  Destination
/// \param orientation  Orientation
/// \param zValue  original zValue
void MOBILITY_AddANewDestination(
    MobilityData *mobilityData,
    clocktype arrivalTime,
    Coordinates dest,
    Orientation orientation,
    double zValue = 0.0);

/// Update next node position for static mobility models
///
/// \param node  Node to be updated
/// \param element  next mobility update
BOOL MOBILITY_NextPosition(
    Node* node,
    MobilityElement* element);

/// Determines the time of next movement.
///
/// \param node  Pointer to node.
///
/// \return Next time of movement.
clocktype MOBILITY_NextMoveTime(const Node* node);


/// Used to get the mobility element.
///
/// \param node  Pointer to node.
/// \param sequenceNum  Sequence number.
MobilityElement* MOBILITY_ReturnMobilityElement(
    Node* node,
    int sequenceNum);


/// Inserts a new event.
///
/// \param node  Pointer to node.
/// \param nextMoveTime  Time of next movement.
/// \param position  Position of the node.
/// \param orientation  Node orientation.
/// \param speed  Speed of the node.
void MOBILITY_InsertANewEvent(
    Node* node,
    clocktype nextMoveTime,
    Coordinates position,
    Orientation orientation,
    double speed);


/// Returns whether the node is indoors.
///
/// \param node  Pointer to node.
///
/// \return returns true if indoors.
bool MOBILITY_NodeIsIndoors(const Node* node);


/// Sets the node's indoor variable.
///
/// \param node  Pointer to node.
/// \param indoors  true if the node is indoors.
void MOBILITY_SetIndoors(Node* node, bool indoors = true);


/// Returns the coordinate.
///
/// \param node  Pointer to node.
/// \param position  Position of the node.
void MOBILITY_ReturnCoordinates(
    const Node* node,
    Coordinates* position);


/// Returns the node orientation.
///
/// \param node  Pointer to node.
/// \param orientation  Pointer to Orientation.
void MOBILITY_ReturnOrientation(
    const Node *node,
    Orientation *orientation);


/// Returns instantaneous speed of a node.
///
/// \param node  Pointer to node.
/// \param speed  Speed of the node, double pointer.
void MOBILITY_ReturnInstantaneousSpeed(
    const Node *node,
    double *speed);


/// Returns a sequence number for the current position.
///
/// \param node  Pointer to node.
/// \param sequenceNum  Sequence number.
void MOBILITY_ReturnSequenceNum(
    const Node* node,
    int* sequenceNum);


/// Set positions of nodes
///
/// \param numNodes  Defines the number of nodes to be distributed.
/// \param nodePositions  Pointer to NodePositionInfo. States
///    the node position information.
/// \param nodePlacementTypeCounts  Array of placement type counts
/// \param terrainData  Terrain data.
/// \param nodeInput  Pointer to NodeInput, defines the
///    node input structure.
/// \param seed  Stores the seed value.
/// \param maxSimTime  Maximum simulation time.
void MOBILITY_SetNodePositions(
    int numNodes,
    NodePositions* nodePositions,
    int* nodePlacementTypeCounts,
    TerrainData* terrainData,
    NodeInput* nodeInput,
    RandomSeed seed,
    clocktype maxSimTime,
    clocktype startSimTime);


/// Initialization of mobility models that most be done
/// after partition is created; MOBILITY_SetNodePositions
/// would be too early
///
/// \param partitionData  Pointer to the partition data
void MOBILITY_PostInitializePartition(PartitionData* partitionData);


/// Finalize mobility models
///
/// \param partitionData  Pointer to the partition data
void MOBILITY_NodePlacementFinalize(PartitionData* partitionData);

/// Change GroundNode value..
///
/// \param node  Pointer to node being initialized.
/// \param before  Orginal value for Ground-Node variable
/// \param after  new value for Ground-Node variable.
void MOBILITY_ChangeGroundNode(Node* node, BOOL before, BOOL after);

/// Change Mobility-Position-Granularity  value..
///
/// \param node  Pointer to node being initialized.
void MOBILITY_ChangePositionGranularity(Node* node);

/*
 * FUNCTION    MOBILITY_BuildMinHeap
 * PURPOSE     Rebuild the current Heap to maintain Min-heap properties by
 *             Call Min-Heapify from bottoms up.
 *
*/
void MOBILITY_BuildMinHeap(Node* node, MobilityHeap* heapPtr);

#endif /*MOBILITY_H*/
