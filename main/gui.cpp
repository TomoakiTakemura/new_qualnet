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

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <algorithm>
#include <ctype.h>
#include <stdlib.h>
#include "ClientIDGenerator.h"

// receive buffer size
#define SOCK_MAX_BUFFER_SIZE  4098

#define HUMAN_IN_THE_LOOP_DEMO 1
#define HITL 1

#ifdef _WIN32
#include "qualnet_mutex.h"
#include "winsock.h"

// recv() is supposed to return an ssize_t, but on Windows (even
// 64-bit Windows) ssize_t is missing and recv() returns an int.
typedef int ssize_t;
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifndef s6_addr32
#define s6_addr32 s6_addr
#endif
#include <netdb.h>
#include "qualnet_mutex.h"
#endif


//#define SOCKET unsigned int

#include <sys/types.h>
#include <time.h>

#include "api.h"
#include "clock.h"
#include "qualnet_error.h"
#include "fileio.h"
#include "gui.h"
#include "partition.h"
#include "phy.h"
#include "external_util.h"
#include "scheduler.h"
#include "stats.h"
#include "WallClock.h"

#ifdef SOPSVOPS_INTERFACE
#include "SopsProperty.h"
#endif

#ifdef VRLINK_INTERFACE
#include "vrlink.h"
#endif /* VRLINK_INTERFACE */

#ifdef HLA_INTERFACE
#include "hla.h"
#endif /* HLA_INTERFACE */

#ifdef WIRELESS_LIB
#include "phy_cellular.h"
#include "phy_802_11.h"
#include "phy_abstract.h"
#include "antenna_global.h"
#include "antenna_switched.h"
#include "antenna_steerable.h"
#include "antenna_patterned.h"
#endif // WIRELESS_LIB

#ifdef CELLULAR_LIB
#include "phy_gsm.h"
#endif

#ifdef UMTS_LIB
#include "phy_umts.h"
#endif // UMTS_LIB


#ifdef ADVANCED_WIRELESS_LIB
#include "phy_dot16.h"
#endif

#ifdef SENSOR_NETWORKS_LIB
#include "phy_802_15_4.h"
#endif

#ifdef PAS_INTERFACE
#include "packet_analyzer.h"
#endif

#ifdef IPNE_INTERFACE
#include "ipnetworkemulator.h"
#endif

#ifdef AUTO_IPNE_INTERFACE
#include "auto-ipnetworkemulator.h"
#endif

#ifdef CYBER_LIB
#include "app_cbr.h"
#include "attack_sequence.h"
#include "app_jammer.h"
#include "firewall_model.h"
#include "application.h"
#include "app_dos.h"
#include "app_sigint.h"
#include "app_eaves.h"
#include "app_portscan.h"
#endif // CYBER_LIB

#include "app_util.h"

#ifdef MULTI_GUI_INTERFACE
#include "multi_gui_interface.h"
#endif // MULTI_GUI_INTERFACE

#include "network_ip.h"

using namespace std;

#define HASH_SIZE           128

#define DEBUG             0

//--------------------------------------------------------
// data structures for use in managing node data.
//--------------------------------------------------------
typedef struct nodeIdToNodeIndex * IdToNodeIndexMap;

struct nodeIdToNodeIndex
{
    NodeId           nodeID;
    int              index;
    IdToNodeIndexMap next;
};

//--------------------------------------------------------
// data structures for use in managing dynamic statistics
//--------------------------------------------------------
typedef struct {
    MetricData* metricDataPtr;
    BOOL* nodeEnabled;
} MetricFilter;

//--------------------------------------------------------
// Variables used by partition_private.c for controlling dynamic statistics.
//--------------------------------------------------------
clocktype GUI_statReportTime;
clocktype GUI_statInterval;

//--------------------------------------------------------
// These globals are safe-enough. If shared, only p0 will write to, in
// mpi these globals are in sep processes.
// Global variable set exclusively by command line arguments
BOOL         GUI_animationTrace = FALSE;
#ifdef MULTI_GUI_INTERFACE
BOOL         GUI_multigui = FALSE;
#endif // MULTI_GUI_INTERFACE
static int   g_statLayerEnabled[MAX_LAYERS];
//static int   metricsEnabledByLayer[MAX_LAYERS];


// Filter similar animations if they occur within the same frequency
// Default is to not filter (g_animationFilterFrequency is 0)
// The filter frequency is set by the GUI_SET_ANIMATION_FILTER_FREQUENCY command
double   g_animationFilterFrequency = 0;

// Struct for determining similar animations
struct GUI_AnimationFilterType
{
    GuiEvents animationType;
    int id1;
    int id2;

    bool operator< (const GUI_AnimationFilterType& rhs) const
    {
        if (animationType < rhs.animationType)
        {
            return true;
        }
        else if (animationType > rhs.animationType)
        {
            return false;
        }
        else if (id1 < rhs.id1)
        {
            return true;
        }
        else if (id1 > rhs.id1)
        {
            return false;
        }
        else if (id2 < rhs.id2)
        {
            return true;
        }
        else if (id2 > rhs.id2)
        {
            return false;
        }

        return false;
    }
};

std::map<GUI_AnimationFilterType, double> g_animationFilterMap;

// Check if an animation is the same as another animation that occurred
// recently.   This function call must be protected by a mutex.
static bool GUI_CheckSimilarAnimation(const GUI_AnimationFilterType& type)
{
    std::map<GUI_AnimationFilterType, double>::iterator it =
        g_animationFilterMap.find(type);
    double now = (double)time(NULL);
    // Do not filter if frequency is not specified
    if (g_animationFilterFrequency == 0)
    {
        return false;
    }

    if (it == g_animationFilterMap.end())
    {
        g_animationFilterMap[type] = now;
        return false;
    }
    else
    {
        // Filter if within the animation window
        bool filter = now < it->second + g_animationFilterFrequency;
        if (!filter)
        {
            it->second = now;
        }
        return filter;
    }
}

// Store of visualization objects which might need to be sent (or resent)
// to the GUI later.

// Unfortunately, we can't just store the GuiReply object, since if we
// need to resend it later, we need to send it with the current time in
// the event.  So instead we store enough information to reconstruct a
// GuiReply, except for the time which is provided to GUI_SetVisObjFilter.
struct GUI_VisObjStoreEntry {
    GuiVisObjCommands cmd;
    std::string text;
    NodeId detailsNodeId;
};
static std::multimap<std::string, GUI_VisObjStoreEntry> g_visObjStore;

// Set of filters which are currently active on the GUI side.
static std::set<std::string> g_activeVisObjFilters;

// Set of nodes which have a details dialog open in the GUI.
static std::set<NodeId> g_activeVisObjDetails;

// Determine whether the given filter is currently active.
static bool GUI_IsVisObjFilterActive(const std::string& filterName)
{
    // Handle "never" and "always" hierarchies specially
    if (filterName == "never" ||
        filterName.substr(0, strlen("never|")) == "never|")
    {
        return false;
    }
    if (filterName == "always" ||
        filterName.substr(0, strlen("always|")) == "always|")
    {
        return true;
    }

    std::set<std::string>::iterator it = g_activeVisObjFilters.find(filterName);
    if (it != g_activeVisObjFilters.end())
    {
        return true;
    }

    // If the full filter name isn't found in the set, check for parents
    // (from most general to most specific, since the GUI filters are
    // usually fairly general).
    for (std::string::size_type sepPos = filterName.find('|');
         sepPos != std::string::npos;
         sepPos = filterName.find(sepPos + 1, '|'))
    {
        if (sepPos > 0 &&
            g_activeVisObjFilters.find(filterName.substr(0, sepPos)) !=
                g_activeVisObjFilters.end())
        {
            return true;
        }
    }
    return false;
}

// Determine whether the given node has an open details dialog.
static bool GUI_IsVisObjDetailsActive(NodeId node)
{
    return (g_activeVisObjDetails.find(node) != g_activeVisObjDetails.end());
}

typedef struct GUI_InterfaceData {
    // How far the gui commands have stepped the gui simulation clock to
    clocktype               guiExternalTime;
    // Increment of simulation time that the horizon advances for each step.
    clocktype               guiStepSize;


    int                     maxNodeID;
    bool                    firstIteration;

    // Keep track of whether the GUI interface is currently paused
    bool                    guiPaused;
} GUI_InterfaceData;

//--------------------------------------------------------
// GUI uses several global variables.
// When running MPI simulations, these globals are fine.
// In shared-memory parallel, all of these variables need
// to be accessed synchronously with other partitions.
//--------------------------------------------------------
QNPartitionMutex       GUI_globalsMutex;
//--------------------------------------------------------
// These variables are used for animation and stats filtering
//--------------------------------------------------------
BOOL                    g_layerEnabled[MAX_LAYERS];

// Enable or disable simulator-side event filtering.  This array will be
// initialized to contain all TRUE values (all events are enabled).  Events
// may be enabled or disabled using the GUI_ENABLE_EVENT and
// GUI_DISABLE_EVENT commands.
BOOL                    g_eventEnabled[GUI_MAX_EVENTS];
int                     g_numberOfNodes;
IdToNodeIndexMap        g_idToIndexMap[HASH_SIZE];
int                     g_nodesInitialized = 0;
// These arrays have as many slots as nodes in the entire simulation
// As a result, commands to change the value in a slot must
// be sent to all partitions (in mpi)
BOOL*                   g_nodeEnabledForAnimation;
int*                    g_nodeEnabledForStatistics;

BOOL                    g_sendFinalizationStatus = TRUE;

//--------------------------------------------------------
// These variables are used for application hop by hop flow animation
//--------------------------------------------------------
map<string, string> g_appHopByHopFlowColors;
map<string, string> g_appHopByHopFlowFilters;
set<string>         g_appHopByHopFlowUsedColors;
bool                g_appHopByHopFlowEnabled = false;

//--------------------------------------------------------
// These are used internally to manage dynamic statistics.
//--------------------------------------------------------

#define METRIC_NOT_FOUND       -1
#define METRIC_ALLOCATION_UNIT 100

// These globals are used for dynamic states, and required synchronized
// access for shared-parallel simulations.
MetricLayerData        g_metricData[MAX_LAYERS];
static MetricFilter*   g_metricFilters;
static int             g_metricsAllocated = 0;
static int             g_numberOfMetrics  = 0;




static BOOL GUI_isConnected ();
BOOL GUI_isAnimate ();

/*------------------------------------------------------
 * We want to store node information in a simple array, but
 * node IDs are not necessarily contiguous, so we place the
 * node IDs into a hash along with the node's index into
 * the simple array.
 *------------------------------------------------------*/

/*------------------------------------------------------
 * HashNodeIndex takes a node ID and index and adds them to
 * the hash.
 *------------------------------------------------------*/
static void HashNodeIndex(NodeId nodeID,
                          int    index) {

    // We are called from GUI_InitNode (), which should only
    // be called during partition init, and after that no more.
    // So, other functions that access the g_idToIndexMap don't
    // have to lock.

    int              hash     = nodeID % HASH_SIZE;
    IdToNodeIndexMap mapEntry = g_idToIndexMap[hash];
    IdToNodeIndexMap newMapEntry;

    newMapEntry = (IdToNodeIndexMap)
        MEM_malloc(sizeof(struct nodeIdToNodeIndex));
    newMapEntry->nodeID = nodeID;
    newMapEntry->index  = index;
    newMapEntry->next   = mapEntry;
    g_idToIndexMap[hash]  = newMapEntry;
}

/*------------------------------------------------------
 * GetNodeIndexFromHash returns the array index for the given
 * node ID.
 *------------------------------------------------------*/
static int GetNodeIndexFromHash(NodeId nodeID) {

    int              hash     = nodeID % HASH_SIZE;
    IdToNodeIndexMap mapEntry = g_idToIndexMap[hash];

    while (mapEntry != NULL) {
        if (mapEntry->nodeID == nodeID) {
            return mapEntry->index;
        }
        mapEntry = mapEntry->next;
    }
    //printf("node ID is %d\n", nodeID); fflush(stdout);
    //assert(FALSE);
    return -1;
}

/*------------------------------------------------------------------------
 * We create two structures for keeping track of dynamic statistics.
 * First, there's an array, indexed by metric ID, of node filters.
 * Second, there's an array of metric names, organized by layer.
 * The first structure is used for filtering metrics, the latter for
 * assigning them IDs.  The following two functions allocate and
 * initialize these two data structures.
 *------------------------------------------------------------------------*/

/*------------------------------------------------------
 * This function allocates new entries in the filter
 * array.  It increases the array size by 100 at a time.
 * We should probably do something to avoid all the mallocs.
 *------------------------------------------------------*/
static void AllocateAMetricFilter(int metricID) {

    if (g_numberOfMetrics > g_metricsAllocated) {
        // reallocate metrics table
        int numberToAllocate = g_metricsAllocated + METRIC_ALLOCATION_UNIT;
        MetricFilter* oldFilters = g_metricFilters;
        g_metricFilters = (MetricFilter*) MEM_malloc(sizeof(MetricFilter) *
                                               numberToAllocate);
        assert(g_metricFilters != NULL);
        if (g_metricsAllocated > 0) {
            memcpy(g_metricFilters, oldFilters,
                   sizeof(MetricFilter) * g_metricsAllocated);
            MEM_free(oldFilters);
        }
        g_metricsAllocated = numberToAllocate;
    }

    // Create a filter for the metric and initialize it to false;
    g_metricFilters[metricID].nodeEnabled = (BOOL*) MEM_malloc(sizeof(BOOL) *
                                                         g_numberOfNodes);
    assert(g_metricFilters[metricID].nodeEnabled != NULL);
    memset(g_metricFilters[metricID].nodeEnabled, 0,
           sizeof(BOOL) * g_numberOfNodes);
}

/*------------------------------------------------------
 * This function takes the metric name and layer and returns
 * a unique identifier for the metric, allocating space for
 * it if necessary.
 *------------------------------------------------------*/
static int GetMetricID(const char*  metricName,
                       GuiLayers    layer,
                       GuiDataTypes metricDataType) {
    QNPartitionLock globalsLock(&GUI_globalsMutex);
    int metricID;

    if (g_metricData[layer].numberAllocated == 0) {
        // obviously a new metric
        metricID = g_numberOfMetrics;
        g_numberOfMetrics++;
        AllocateAMetricFilter(metricID);

        // allocate space for this
        g_metricData[layer].numberAllocated = METRIC_ALLOCATION_UNIT;
        g_metricData[layer].metricList =
            (MetricData*) MEM_malloc(sizeof(MetricData) * METRIC_ALLOCATION_UNIT);
        assert(g_metricData[layer].metricList != NULL);

        g_metricFilters[metricID].metricDataPtr
            = &g_metricData[layer].metricList[0];

        // assign the values
        g_metricData[layer].numberUsed = 1;
        g_metricData[layer].metricList[0].metricID = metricID;
        g_metricData[layer].metricList[0].metricLayerID = layer;
        g_metricData[layer].metricList[0].metricDataType = metricDataType;
        strcpy(g_metricData[layer].metricList[0].metricName, metricName);
    }
    else { // search for the metric
        int i;
        for (i = 0; i < g_metricData[layer].numberUsed; i++) {
            MetricData md = g_metricData[layer].metricList[i];
            if (!strcmp(metricName, md.metricName))
            {
                return md.metricID;
            }
        }
        // if we got here, it wasn't found, so we have to add it.
        metricID = g_numberOfMetrics;
        g_numberOfMetrics++;
        AllocateAMetricFilter(metricID);

        assert(i <= g_metricData[layer].numberAllocated);
        if (i == g_metricData[layer].numberAllocated) {
            int numberAllocated = g_metricData[layer].numberAllocated;
            int numberToAllocate = numberAllocated + METRIC_ALLOCATION_UNIT;
            MetricData* oldList  = g_metricData[layer].metricList;

            g_metricData[layer].numberAllocated += METRIC_ALLOCATION_UNIT;
            g_metricData[layer].metricList =
                (MetricData*) MEM_malloc(sizeof(MetricData) * numberToAllocate);
            assert(g_metricData[layer].metricList != NULL);
            memcpy(g_metricData[layer].metricList, oldList,
                   sizeof(MetricData) * numberAllocated);
        }
        // assign the values
        g_metricData[layer].numberUsed++;
        g_metricData[layer].metricList[i].metricID = metricID;
        g_metricData[layer].metricList[i].metricLayerID = layer;
        g_metricData[layer].metricList[i].metricDataType = metricDataType;
        strcpy(g_metricData[layer].metricList[i].metricName, metricName);

        g_metricFilters[metricID].metricDataPtr
            = &g_metricData[layer].metricList[i];
    }

    return metricID;
}

/*------------------------------------------------------
 * This function adds double-quotes at the beginning
 * and end of a string which is passed as an argument
 *------------------------------------------------------*/
static void EncloseStringWithinQuotes(char* str) {

    int i;
    int len = (int)strlen(str);

    for (i = len; i >= 0; i--)
    {
        str[i + 1] = str[i];
    }
    str[0] = '"';
    str[len + 1] = '"';
    str[len + 2] ='\0';
}


/*------------------------------------------------------
 * GUI_Initialize
 *
 * Initializes the GUI in order to start the animation.
 * This function is not really necessary during a live run, but
 * will be needed in a trace file.
 * Called for all partitions, and always called to setup global
 * variables for printing stats.
 *------------------------------------------------------*/
static void GUI_Initialize(PartitionData * partitionData,
                    GUI_InterfaceData * data,
                    NodeInput*  nodeInput,
                    int         coordinateSystem,
                    Coordinates origin,
                    Coordinates dimensions,
                    clocktype   maxClock) {
    GuiReply reply;
    char     replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    int      layer;
    int      node;
    char     experimentName[MAX_STRING_LENGTH];
    char     backgroundImage[MAX_STRING_LENGTH];
    BOOL     wasFound;
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (!GUI_isAnimateOrInteractive ())
    {
        return;
    }

    ctoa(maxClock, timeString);

    IO_ReadString(ANY_NODEID,
                  ANY_ADDRESS,
                  nodeInput,
                  "EXPERIMENT-NAME",
                  &wasFound,
                  experimentName);

    if (!wasFound) {
        strcpy(experimentName, "\"qualnet\"");
    }
    else {
        EncloseStringWithinQuotes(experimentName);
    }

    IO_ReadString(ANY_NODEID,
                  ANY_ADDRESS,
                  nodeInput,
                  "GUI-BACKGROUND-IMAGE-FILENAME",
                  &wasFound,
                  backgroundImage);

    if (!wasFound) {
        strcpy(backgroundImage, "\"\"");
    }
    else {
        EncloseStringWithinQuotes(backgroundImage);
    }

    reply.type = GUI_ANIMATION_COMMAND;
    /*
    // MS VS Express edition has a compilation bug.
    // This call to sprintf results in incorrect output -
    // should be:
    // 0 30 0 0.000000 0.000000 0.000000 1500.000000 1500.000000 0.000000 900000000000 default ""
    // but bad code produced by compiler results in
    // 0 0 30 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000 (null) 900000030720 "default"
    // Arguments _are_ correct. Possibly a bug in microsoft's optimizer.
    // It appears that adding explicit casts solves the problem.
    sprintf(reply.args, "%d %d %d %f %f %f %f %f %f %s %s %s",
            GUI_INITIALIZE, partitionData->numNodes, coordinateSystem,
            origin.common.c1, origin.common.c2, origin.common.c3,
            dimensions.common.c1, dimensions.common.c2, dimensions.common.c3,
            timeString, experimentName, backgroundImage);
            */
    {
        sprintf(replyBuff, "%d %d %d %f %f %f %f %f %f",
            (int) GUI_INITIALIZE, (int) partitionData->numNodes,
            (int) coordinateSystem, (double) origin.common.c1,
            (double) origin.common.c2, (double) origin.common.c3,
            (double) dimensions.common.c1, (double) dimensions.common.c2,
            (double) dimensions.common.c3);
        reply.args.append(replyBuff);
    }

    reply.args.append(" ");
    reply.args.append(timeString);
    reply.args.append(" ");
    reply.args.append(experimentName);
    reply.args.append(" ");
    reply.args.append(backgroundImage);

    // Add list of terrain files
    std::string* fileList = PARTITION_GetTerrainPtr(partitionData)->terrainFileList(nodeInput);
    if (fileList != NULL) {
        reply.args += " ";
        reply.args += *fileList;
        delete fileList;
    }

    // For mpi, all partitions will send GUI_INTIALIZE command.
    // For shared or 1 cpu, only one GUI_INITIALIZE
    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE

    // Initialize the GUI filtering and control variables.
    int numNodes = partitionData->numNodes;
    g_numberOfNodes = numNodes;
    data->maxNodeID     = numNodes - 1;
                                // fine if ordered sequentially, but
                                // will have to change if not.

    g_nodeEnabledForAnimation  = (BOOL*) MEM_malloc(numNodes * sizeof(BOOL));
    g_nodeEnabledForStatistics = (int*) MEM_malloc(numNodes * sizeof(BOOL));
    for (node = 0; node < g_numberOfNodes; node++) {
        g_nodeEnabledForAnimation[node] = TRUE;
        g_nodeEnabledForStatistics[node] = 0;
    }

    for (layer = 0; layer < MAX_LAYERS; layer++) {
        g_layerEnabled[layer] = TRUE;
        g_statLayerEnabled[layer]           = 0;
        g_metricData[layer].metricList      = NULL;
        g_metricData[layer].numberAllocated = 0;
        g_metricData[layer].numberUsed      = 0;
    }

    for (int event = 0; event < GUI_MAX_EVENTS; event++) {
        g_eventEnabled[event] = TRUE;
    }
}

/*------------------------------------------------------
 * GUI_SetEffect
 *
 * This function will allow the protocol designer to specify the
 * effect to use to display certain events.
 *------------------------------------------------------*/
void GUI_SetEffect(GuiEvents  event,
                   GuiLayers  layer,
                   int        type,
                   GuiEffects effect,
                   GuiColors  color) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];

    reply.type = GUI_SET_EFFECT;
    sprintf(replyBuff, "%d %d %d %d %d", event, layer, type, effect, color);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * InitNode
 *
 * Provides the initial location of the node, and also contains other
 * attributes such as transmission range, protocols, etc.
 *------------------------------------------------------*/
void GUI_InitNode(Node*      node,
                  NodeInput* nodeInput,
                  clocktype  time) {

    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    // This code calculates a reasonable default value for the radio
    // range, used by the GUI for displaying radio transmissions,
    // and otherwise initializes the GUI for this node.

    char name[MAX_STRING_LENGTH];
    char icon[MAX_STRING_LENGTH];
    BOOL wasFound;

   // Commented out Y Zhou 6-3-05
   /*
    IO_ReadString(node->nodeId, ANY_ADDRESS,
                  nodeInput, "USE-NODE-ICON", &wasFound, useIcon);
    if (wasFound && !strcmp(useIcon, "YES"))
    {
        IO_ReadString(node->nodeId, ANY_ADDRESS,
                      nodeInput, "NODE-ICON", &wasFound, icon);
        if (!wasFound) {
            strcpy(icon, GUI_DEFAULT_ICON);
        }
        else {
            EncloseStringWithinQuotes(icon);
        }
    }
    else
    {
        strcpy(icon, GUI_DEFAULT_ICON);
    }
   */

   /* Added Y Zhou 6-3-05 for USE-NODE-ICON removal */
   IO_ReadString(node->nodeId, ANY_ADDRESS,
        nodeInput, "GUI-NODE-2D-ICON", &wasFound, icon);
    if (!wasFound) {
        IO_ReadString(node->nodeId, ANY_ADDRESS,
            nodeInput, "NODE-ICON", &wasFound, icon);
    }
    if (!wasFound) {
            strcpy(icon, GUI_DEFAULT_ICON);
    }
    else {
            EncloseStringWithinQuotes(icon);
    }

    IO_ReadString(node->nodeId, ANY_ADDRESS,
                  nodeInput, "HOSTNAME", &wasFound, name);
    if (!wasFound) {
        sprintf(name, "\"host%u\"", node->nodeId);
    }
    else {
        EncloseStringWithinQuotes(name);
    }

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %.10f %.10f %.10f %d %d %d",
            GUI_INIT_NODE, node->nodeId,
            node->mobilityData->current->position.common.c1,
            node->mobilityData->current->position.common.c2,
            node->mobilityData->current->position.common.c3,
            0, // type
            node->mobilityData->current->orientation.azimuth,
            node->mobilityData->current->orientation.elevation);
    reply.args.append(replyBuff);
    reply.args.append(" ");
    reply.args.append(timeString);
    reply.args.append(" ");
    reply.args.append(icon); // icon file
    reply.args.append(" ");
    reply.args.append(name); // label

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE

    // enter the node into the hash
    HashNodeIndex(node->nodeId, g_nodesInitialized);
    g_nodesInitialized++;
}

/*------------------------------------------------------
 * InitWirelessInterface
 *
 * Sets up the wireless properties (power, sensitivity, range)
 * for a node's wireless interface.
 *------------------------------------------------------*/
void GUI_InitWirelessInterface(Node* node,
                               int   interfaceIndex) {
#ifdef WIRELESS_LIB

    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    NodeInput*  nodeInput = node->partitionData->nodeInput;

    float conversionParameter = 0.0f;

    char antennaFile[MAX_STRING_LENGTH];
    char antennaAziFile[MAX_STRING_LENGTH];
    char antennaElvFile[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    BOOL wasFound = FALSE;
    BOOL wasAziFound = FALSE;
    BOOL wasElvFound = FALSE;
    BOOL wasParamFound = FALSE;
    BOOL wasModelFound = FALSE;

    // Input variables
    PathlossModel pathlossModel = TWO_RAY;

    // assign reasonable defaults, in case variables are undefined.
    double channel0Frequency  = 2.4e9;
    double directionalTxPower = 0.0;
    double txPower            = 15.0;
    double rxThreshold        = -81.0;
    double txAntennaGain      = 0.0;
    double rxAntennaGain      = 0.0;
    double antennaHeight      = 0.0;
    double distance           = 0.0;
    double systemLoss         = 0.0;
    double shadowingMean      = 4.0;

    PropProfile*     propProfile = node->partitionData->propChannel[0].profile;

    PhyData*         thisRadio;
    PhyModel         phyModel = PHY_NONE;
    PhyData802_11*   phy802_11 = NULL;
    PhyDataAbstract* phyAbstract;
    AntennaOmnidirectional* omniDirectional;
    AntennaSwitchedBeam* switchedBeam;
    AntennaSteerable* steerable;
    AntennaPatterned* patterned;
#ifdef CELLULAR_LIB
    PhyDataGsm*      phyGsm;
#endif // CELLULAR_LIB
#ifdef ADVANCED_WIRELESS_LIB
    PhyDataDot16*    phyDot16 = NULL;
#endif
#ifdef SENSOR_NETWORKS_LIB
    PhyData802_15_4* phy802_15_4 = NULL;
#endif


    int phyIndex = node->macData[interfaceIndex]->phyNumber;

    thisRadio = node->phyData[phyIndex];
    phyModel  = node->phyData[phyIndex]->phyModel;

    switch (phyModel) {
        case PHY802_11pCCH:
        case PHY802_11pSCH:
        case PHY802_11a:
        case PHY802_11b:
            phy802_11          = (PhyData802_11*)thisRadio->phyVar;
            txPower            = (float)phy802_11->pDot11Base->
                                                    txDefaultPower_dBm[0];
            rxThreshold        = phy802_11->rxSensitivity_mW[0];
            directionalTxPower =
                txPower - phy802_11->directionalAntennaGain_dB;
            break;

        case PHY802_11n:
        case PHY802_11ac:
            phy802_11          = (PhyData802_11*)thisRadio->phyVar;
            txPower            = (float)phy802_11->txPower_dBm;
            rxThreshold        = phy802_11->pDot11Base->getSensitivity_dBm();
            directionalTxPower =
                txPower - phy802_11->directionalAntennaGain_dB;
            break;

        case PHY_ABSTRACT:
            phyAbstract = (PhyDataAbstract*)thisRadio->phyVar;
            txPower     = (float)phyAbstract->txPower_dBm;
            rxThreshold = phyAbstract->rxThreshold_mW;
            break;

#ifdef CELLULAR_LIB
        case PHY_GSM:
            phyGsm      = (PhyDataGsm*) thisRadio->phyVar;
            txPower     = (float)phyGsm->txPower_dBm;
            rxThreshold = phyGsm->rxThreshold_mW;
            break;
#endif

#ifdef WIRELESS_LIB
        case PHY_CELLULAR:
        {
            PhyCellularData *phyCellular = (PhyCellularData*)thisRadio->phyVar;
            CellularPhyProtocolType cpt = phyCellular->cellularPhyProtocolType;

            switch (cpt) {
#ifdef UMTS_LIB
            case Cellular_UMTS_Phy:
            {
                PhyUmtsData* phyUmts = (PhyUmtsData* )phyCellular->phyUmtsData;
                txPower     = (float)phyUmts->txPower_dBm;
                rxThreshold = phyUmts->rxSensitivity_mW[0];

            }
            break;
#endif
            }
        }
        break;
#endif // WIRELESS_LIB

#ifdef ADVANCED_WIRELESS_LIB
        case PHY802_16:
            phyDot16 = (PhyDataDot16*) thisRadio->phyVar;
            txPower            = (float)phyDot16->txPower_dBm;
            rxThreshold        = phyDot16->rxSensitivity_mW[0];

            break;
#endif
#ifdef SENSOR_NETWORKS_LIB
        case PHY802_15_4:
            phy802_15_4 = (PhyData802_15_4*) thisRadio->phyVar;
            txPower            = (float)phy802_15_4->txPower_dBm;
            rxThreshold        = phy802_15_4->rxSensitivity_mW;
            break;
#endif

        // need case for FCSC
        // need case for Link-16
        default:
            break; // use defaults
    }

    Int32 patternIndex = ANTENNA_OMNIDIRECTIONAL_PATTERN;
    switch (thisRadio->antennaData->antennaModelType) {
        case ANTENNA_OMNIDIRECTIONAL:
            omniDirectional = (AntennaOmnidirectional*)
                                thisRadio->antennaData->antennaVar;
            rxAntennaGain = omniDirectional->antennaGain_dB;
            antennaHeight = omniDirectional->antennaHeight;
            break;
        case ANTENNA_SWITCHED_BEAM:
            switchedBeam = (AntennaSwitchedBeam*)
                            thisRadio->antennaData->antennaVar;
            rxAntennaGain = switchedBeam->antennaGain_dB;
            antennaHeight = switchedBeam->antennaHeight;
            break;
        case ANTENNA_STEERABLE:
            steerable = (AntennaSteerable*)
                thisRadio->antennaData->antennaVar;
            rxAntennaGain = steerable->antennaGain_dB;
            antennaHeight = steerable->antennaHeight;
            break;
        case ANTENNA_PATTERNED:
            patterned = (AntennaPatterned*)
                thisRadio->antennaData->antennaVar;
            rxAntennaGain = patterned->antennaGain_dB;
            antennaHeight = patterned->antennaHeight;
            patternIndex = ANTENNA_DEFAULT_PATTERN;
            break;
    }

    rxThreshold       = IN_DB(rxThreshold);

    txAntennaGain     = rxAntennaGain;

    pathlossModel     = propProfile->pathlossModel;
    channel0Frequency = propProfile->frequency;
    shadowingMean     = propProfile->shadowingMean_dB;
    systemLoss        = thisRadio->systemLoss_dB;

    int numChannels   = PROP_NumberChannels(node);
    int k;
    int channelindex = 0;

    for (k = 0; k < numChannels; k++)
    {
        if (PHY_CanListenToChannel(node, phyIndex, k))
        {
            break;
        }

        channelindex++;
    }

    distance          = PHY_PropagationRange(node, node, phyIndex, phyIndex, channelindex, FALSE);

    IO_ReadString(
            node->nodeId,
            MAPPING_GetInterfaceAddressForInterface(node,
            node->nodeId, interfaceIndex),
            nodeInput,
            "ANTENNA-MODEL",
            &wasModelFound,
            buf);
    NodeInput* antennaModelInput = NULL;

    if (!wasModelFound || ((strcmp(buf, "OMNIDIRECTIONAL") == 0) ||
        (strcmp(buf, "SWITCHED-BEAM") == 0) ||
        (strcmp(buf, "STEERABLE") == 0) || (strcmp(buf, "PATTERNED") == 0)))
    {
        antennaModelInput = nodeInput;
    }

    else{

      antennaModelInput
            = ANTENNA_MakeAntennaModelInput(nodeInput,
                                            buf);
    }

        switch (thisRadio->antennaData->antennaPatternType) {

            case ANTENNA_PATTERN_TRADITIONAL:
            case ANTENNA_PATTERN_ASCII2D:
                 IO_ReadString(node->nodeId,
                              MAPPING_GetInterfaceAddressForInterface(node,
                              node->nodeId, interfaceIndex),
                              antennaModelInput,
                              "ANTENNA-AZIMUTH-PATTERN-FILE",
                              &wasAziFound,
                              antennaAziFile);

                 if (wasAziFound) {
                    EncloseStringWithinQuotes(antennaAziFile);
                 }

                 IO_ReadString(node->nodeId,
                              MAPPING_GetInterfaceAddressForInterface(node,
                              node->nodeId, interfaceIndex),
                              antennaModelInput,
                              "ANTENNA-ELEVATION-PATTERN-FILE",
                              &wasElvFound,
                              antennaElvFile);

                 if (wasElvFound) {
                    EncloseStringWithinQuotes(antennaElvFile);
                 }

                 break;

            case ANTENNA_PATTERN_ASCII3D:
            case ANTENNA_PATTERN_NSMA:
                 IO_ReadString(node->nodeId,
                              MAPPING_GetInterfaceAddressForInterface(node,
                              node->nodeId, interfaceIndex),
                              antennaModelInput,
                              "ANTENNA-PATTERN-PATTERN-FILE",
                              &wasFound,
                              antennaFile);

                 if (wasFound) {
                    EncloseStringWithinQuotes(antennaFile);
                 }

                 break;

            default:
            {
                break;
            }
        }

        IO_ReadFloat(node->nodeId,
                     MAPPING_GetInterfaceAddressForInterface(node,
                     node->nodeId, interfaceIndex),
                     antennaModelInput,
                     "ANTENNA-PATTERN-CONVERSION-PARAMETER",
                     &wasParamFound,
                     &conversionParameter);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff,"%d %u %d %d %d %d %.5lf %.5lf %.5lf %.5lf %.5lf "
            "%.5lf %.5lf %d %.5lf %d",
            GUI_INIT_WIRELESS, node->nodeId, interfaceIndex,
            (int) distance,
            patternIndex,
            thisRadio->antennaData->antennaModelType,
            txPower,
            rxThreshold,
            antennaHeight,
            channel0Frequency,
            directionalTxPower,
            systemLoss,
            shadowingMean,
            (int) pathlossModel,
            (wasParamFound) ? conversionParameter : 0.0,
            thisRadio->antennaData->antennaPatternType);
    reply.args.append(replyBuff);
    reply.args.append(" ");
    reply.args.append((wasAziFound) ? antennaAziFile : "");
    reply.args.append(" ");
    reply.args.append((wasElvFound) ? antennaElvFile : "");
    reply.args.append(" ");
    reply.args.append((wasFound) ? antennaFile : "");

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE

#else // WIRELESS_LIB
    ERROR_ReportWarning("Wireless library is required to initialize wireless interfaces");
#endif // WIRELESS_LIB
}

static
void GUI_ValidateNodeIdAndInterfaceIndex(
    char* variableNames,
    char* qualifier,
    NodeId* nodeId,
    int interfaceIndex,
    BOOL* isNodeId)
{
    if (!qualifier)
    {
        char err[MAX_STRING_LENGTH];

        sprintf(err, "%s must have a qualifier node id\n",
            variableNames);

        ERROR_ReportError(err);
    }

    IO_ParseNodeIdOrHostAddress(
        qualifier,
        nodeId,
        isNodeId);

    if (!(*isNodeId))
    {
        char err[MAX_STRING_LENGTH];

        sprintf(err, "Qualifier of %s must be a node id\n",
            variableNames);

        ERROR_ReportError(err);
    }

    if (interfaceIndex < 0)
    {
        char err[MAX_STRING_LENGTH];

        sprintf(err, "Variable %s requires a non-negative index, for example\n"
                "[%d] %s[1] value\n",
                variableNames, *nodeId, variableNames);

        ERROR_ReportError(err);
    }
}

/*------------------------------------------------------
 * GUI_InitializeInterfaces
 *
 * Sets the IP address for a node's interface.
 *------------------------------------------------------*/
void GUI_InitializeInterfaces(const NodeInput* nodeInput) {

    int i;

    // send over all the user-defined IP-ADDRESS,
    // IP-SUBNET-MASK and INTERFACE-NAME entries
    // presumably first two were validated in mapping.c
    for (i = 0; i < nodeInput->numLines; i++)
    {
        char* qualifier      = nodeInput->qualifiers[i];
        int   interfaceIndex = nodeInput->instanceIds[i];
        char* value          = nodeInput->values[i];
        BOOL  isNodeId       = FALSE;

        NodeId nodeId;

        // Change the IP address of a node's interface.
        if (!strcmp(nodeInput->variableNames[i], "IP-ADDRESS"))
        {
#ifndef ADDON_NGCNMS
            NodeAddress newInterfaceAddress;

            // Make sure that the qualifier is a node ID first.
            GUI_ValidateNodeIdAndInterfaceIndex(
                nodeInput->variableNames[i],
                qualifier,
                &nodeId,
                interfaceIndex,
                &isNodeId);

            // Get new interface address.
            IO_ParseNodeIdOrHostAddress(
                value,
                &newInterfaceAddress,
                &isNodeId);

            GUI_SetInterfaceAddress(
                nodeId,
                newInterfaceAddress,
                interfaceIndex,
                0);
#endif
        }
        // Change the Subnet mask of a node's interface.
        else if (!strcmp(nodeInput->variableNames[i], "IP-SUBNET-MASK"))
        {
            NodeAddress newSubnetMask;

            // Make sure that the qualifier is a node ID first.
            GUI_ValidateNodeIdAndInterfaceIndex(
                nodeInput->variableNames[i],
                qualifier,
                &nodeId,
                interfaceIndex,
                &isNodeId);

            // Get new subnet mask.
            IO_ParseNodeIdOrHostAddress(
                value,
                &newSubnetMask,
                &isNodeId);

            GUI_SetSubnetMask(
                nodeId,
                newSubnetMask,
                interfaceIndex,
                0);
        }
        // Change the Interface name of a node's interface.
        else if (!strcmp(nodeInput->variableNames[i], "INTERFACE-NAME"))
        {
            // Make sure that the qualifier is a node ID first.
            GUI_ValidateNodeIdAndInterfaceIndex(
                nodeInput->variableNames[i],
                qualifier,
                &nodeId,
                interfaceIndex,
                &isNodeId);

            GUI_SetInterfaceName(
                nodeId,
                value,
                interfaceIndex,
                0);
        }
    }
}


/*------------------------------------------------------
 * GUI_SetInterfaceAddress
 *
 * Sets the IP address for a node's interface.
 *------------------------------------------------------*/
void GUI_SetInterfaceAddress(NodeId      nodeID,
                             NodeAddress interfaceAddress,
                             int         interfaceIndex,
                             clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %d %u %s",
            GUI_SET_INTERFACE_ADDRESS, nodeID, interfaceIndex,
            interfaceAddress, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}


/*------------------------------------------------------
 * GUI_SetSubnetMask
 *
 * Sets the Subnet mask for a node's interface.
 *------------------------------------------------------*/
void GUI_SetSubnetMask(NodeId      nodeID,
                       NodeAddress subnetMask,
                       int         interfaceIndex,
                       clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %d %u %s",
            GUI_SET_SUBNET_MASK, nodeID, interfaceIndex,
            subnetMask, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}


/*------------------------------------------------------
 * GUI_SetInterfaceName
 *
 * Sets the Interface name for a node's interface.
 *------------------------------------------------------*/
void GUI_SetInterfaceName(NodeId      nodeID,
                          char*       interfaceName,
                          int         interfaceIndex,
                          clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %d",
            GUI_SET_INTERFACE_NAME, nodeID, interfaceIndex);
    reply.args.append(replyBuff);
    reply.args.append(" ");
    reply.args.append(interfaceName);
    reply.args.append(" ");
    reply.args.append(timeString);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * MoveNode
 *
 * Moves the node to a new position.
 *------------------------------------------------------*/
void GUI_MoveNode(NodeId      nodeID,
                  Coordinates position,
                  clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_MOVE_NODE])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %.15f %.15f %.15f %s",
            GUI_MOVE_NODE, nodeID,
            position.common.c1, position.common.c2,
            position.common.c3, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * SetNodeOrientation
 *
 * Changes the orientation of a node.
 *------------------------------------------------------*/
void GUI_SetNodeOrientation(NodeId      nodeID,
                            Orientation orientation,
                            clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_SET_ORIENTATION])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %d %d %s",
            GUI_SET_ORIENTATION,
            nodeID,
            orientation.azimuth,
            orientation.elevation,
            timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * SetNodeIcon
 *
 * Changes the icon associated with a node.
 *------------------------------------------------------*/
void GUI_SetNodeIcon(NodeId      nodeID,
                     const char* iconFile,
                     clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    char iconFileName[MAX_STRING_LENGTH];
    memset(iconFileName, 0, MAX_STRING_LENGTH);
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_SET_NODE_ICON])
        return;

    if (!strstr(iconFile, GUI_DEFAULT_ICON))
    {
        memcpy(iconFileName, iconFile, strlen(iconFile));
        EncloseStringWithinQuotes(iconFileName);
    }
    else
    {
        memcpy(iconFileName, GUI_DEFAULT_ICON, strlen(iconFile));
    }

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u",
            GUI_SET_NODE_ICON,
            nodeID);
    reply.args.append(replyBuff);
    reply.args.append(" ");
    reply.args.append(iconFileName); //iconFile
    reply.args.append(" ");
    reply.args.append(timeString);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * SetNodeLabel
 *
 * Changes the label (the node name) of a node.
 *------------------------------------------------------*/
void GUI_SetNodeLabel(NodeId      nodeID,
                      char*       label,
                      clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_SET_NODE_LABEL])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u",
            GUI_SET_NODE_LABEL,
            nodeID);
    reply.args.append(replyBuff);
    reply.args.append(" ");
    reply.args.append(label);
    reply.args.append(" ");
    reply.args.append(timeString);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * SetNodeRange
 *
 * Changes the range of a node
 *------------------------------------------------------*/
void GUI_SetNodeRange(NodeId      nodeID,
                      int         interfaceIndex,
                      double      range,
                      clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_SET_NODE_RANGE])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %d %d %s",
            GUI_SET_NODE_RANGE,
            nodeID,
            interfaceIndex,
            (int) range,
            timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}


/*------------------------------------------------------
 * SetNodeType
 *
 * Changes the type of a node
 *------------------------------------------------------*/
void GUI_SetNodeType(NodeId      nodeID,
                     int         type,
                     clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_SET_NODE_TYPE])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %d %s",
            GUI_SET_NODE_TYPE,
            nodeID,
            type,
            timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * SetPatternIndex
 *
 * Changes the antenna pattern index.
 *------------------------------------------------------*/
void GUI_SetPatternIndex(Node*      node,
                         int         interfaceIndex,
                         int         index,
                         clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    int nodeAzimuthAngle = 0;
    int nodeElevationAngle = 0;
    int mobility = 0;
    ctoa(time, timeString);

    if (node->mobilityData->mobilityType != NO_MOBILITY) {
        nodeAzimuthAngle =
            (Int32)node->mobilityData->current->orientation.azimuth;
        nodeElevationAngle =
            (Int32)node->mobilityData->current->orientation.elevation;
        mobility = 1;
    }

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %d %d %s %d %d %d",
            GUI_SET_PATTERN,
            node->nodeId,
            interfaceIndex,
            index,
            timeString,
            nodeAzimuthAngle,
            nodeElevationAngle,
            mobility);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * SetPatternAndAngle
 *
 * Changes the antenna pattern index and the angle of
 * orientation at which to display the pattern.
 *------------------------------------------------------*/
void GUI_SetPatternAndAngle(Node*      node,
                            int         interfaceIndex,
                            int         index,
                            int         angleInDegrees,
                            int         elevationAngle,
                            clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    int nodeAzimuthAngle = 0;
    int nodeElevationAngle = 0;
    int mobility = 0;
    ctoa(time, timeString);

    if (node->mobilityData->mobilityType != NO_MOBILITY) {
        nodeAzimuthAngle =
            (Int32)node->mobilityData->current->orientation.azimuth;
        nodeElevationAngle =
            (Int32)node->mobilityData->current->orientation.elevation;
        mobility = 1;
    }

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %d %d %d %d %s %d %d %d",
            GUI_SET_PATTERNANGLE,
            node->nodeId,
            interfaceIndex,
            index,
            angleInDegrees,
            elevationAngle,
            timeString,
            nodeAzimuthAngle,
            nodeElevationAngle,
            mobility);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * AddLink
 *
 * Adds a link between two nodes.  In a wired topology, this could be
 * a static link; in wireless, a dynamic one.
 *
 * For simplicity, only one "link" between two nodes will be shown (per
 * layer).  If addLink is called when a link already exists, the "type"
 * of the link will be updated.
 *------------------------------------------------------*/
void GUI_AddLink(NodeId      sourceID,
                 NodeId      destID,
                 GuiLayers   layer,
                 int         type,
                 NodeAddress subnetAddress,
                 int         numHostBits,
                 clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(sourceID) == -1) return;
    if (GetNodeIndexFromHash(destID) == -1) return;
    if (!g_layerEnabled[layer] ||
        (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(sourceID)] &&
         !g_nodeEnabledForAnimation[GetNodeIndexFromHash(destID)]))
        return;
    if (!g_eventEnabled[GUI_ADD_LINK])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %u %d %u %d %s",
            GUI_ADD_LINK, layer, sourceID, destID, type, subnetAddress,
            numHostBits, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * AddLink
 *
 * Adds a link between two nodes.  In a wired topology, this could be
 * a static link; in wireless, a dynamic one.
 *
 * For simplicity, only one "link" between two nodes will be shown (per
 * layer).  If addLink is called when a link already exists, the "type"
 * of the link will be updated.
 *------------------------------------------------------*/
void GUI_AddLink(NodeId      sourceID,
                 NodeId      destID,
                 GuiLayers   layer,
                 int         type,
                 int         tla,
                 int         nla,
                 int         sla,
                 clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(sourceID) == -1) return;
    if (GetNodeIndexFromHash(destID) == -1) return;
    if (!g_layerEnabled[layer] ||
        (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(sourceID)] &&
         !g_nodeEnabledForAnimation[GetNodeIndexFromHash(destID)]))
        return;
    if (!g_eventEnabled[GUI_ADD_LINK])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %u %d %d %d %d %s",
            GUI_ADD_LINK, layer, sourceID, destID, type, tla, nla,
            sla, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
     }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * AddLink
 *
 * Adds a link between two nodes.  In a wired topology, this could be
 * a static link; in wireless, a dynamic one.
 *
 * For simplicity, only one "link" between two nodes will be shown (per
 * layer).  If addLink is called when a link already exists, the "type"
 * of the link will be updated.
 *------------------------------------------------------*/
void GUI_AddLink(NodeId       sourceID,
                 NodeId       destID,
                 GuiLayers    layer,
                 int          type,
                 char*        ip6Addr,
                 unsigned int IPv6subnetPrefixLen,
                 clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    char     addrString[MAX_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(sourceID) == -1) return;
    if (GetNodeIndexFromHash(destID) == -1) return;
    if (!g_layerEnabled[layer] ||
        (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(sourceID)] &&
         !g_nodeEnabledForAnimation[GetNodeIndexFromHash(destID)]))
        return;
    if (!g_eventEnabled[GUI_ADD_LINK])
        return;

    ctoa(time, timeString);
    IO_ConvertIpAddressToString(ip6Addr, addrString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %u %d %s %d %s",
            GUI_ADD_LINK, layer, sourceID, destID, type,
            addrString, IPv6subnetPrefixLen,
            timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
     }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * DeleteLink
 *
 * Removes the aforementioned link.
 *------------------------------------------------------*/

// NEW - accepts link type.
void GUI_DeleteLink(NodeId      sourceID,
                    NodeId      destID,
                    GuiLayers   layer,
                    int         type,
                    clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (!g_layerEnabled[layer] ||
        (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(sourceID)] &&
         !g_nodeEnabledForAnimation[GetNodeIndexFromHash(destID)]))
        return;
    if (!g_eventEnabled[GUI_DELETE_LINK])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;

    sprintf(replyBuff, "%d %d %d %u %u %s",
            GUI_DELETE_LINK, layer, type, sourceID, destID, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

// for backwards compatibility.
void GUI_DeleteLink(NodeId      sourceID,
                    NodeId      destID,
                    GuiLayers   layer,
                    clocktype   time) {

    // default link type assumed.
    GUI_DeleteLink(sourceID,
                   destID,
                   layer,
                   GUI_DEFAULT_LINK_TYPE,
                   time);

}

/*------------------------------------------------------
 * Multicast
 *------------------------------------------------------*/
void GUI_Multicast(NodeId      nodeID,
                   GuiLayers   layer,
                   int         type,
                   int         interfaceIndex,
                   clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_layerEnabled[layer] ||
        !g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_MULTICAST])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %d %d %s",
            GUI_MULTICAST, layer, nodeID, type, interfaceIndex, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * Broadcast
 *
 * At lower layers, indicates a radio broadcast.  At network layer, can
 * be used to represent a multicast.
 *------------------------------------------------------*/
void GUI_Broadcast(NodeId      nodeID,
                   GuiLayers   layer,
                   int         type,
                   int         interfaceIndex,
                   clocktype   time) {

    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_layerEnabled[layer] ||
        !g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_BROADCAST])
        return;

    // Check if this is the same animation as a receont one, if so return
    GUI_AnimationFilterType filterType;
    filterType.animationType = GUI_BROADCAST;
    filterType.id1 = nodeID;
    filterType.id2 = 0;
    bool same = GUI_CheckSimilarAnimation(filterType);
    if (same)
    {
        return;
    }

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %d %d %s",
            GUI_BROADCAST, layer, nodeID, type, interfaceIndex, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE

}

/*------------------------------------------------------
 * EndBroadcast
 *
 * Signals the end of a wireless broadcast.
 *------------------------------------------------------*/
void GUI_EndBroadcast(NodeId      nodeID,
                      GuiLayers   layer,
                      int         type,
                      int         interfaceIndex,
                      clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_layerEnabled[layer] ||
        !g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_ENDBROADCAST])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %d %d %s",
            GUI_ENDBROADCAST, layer, nodeID, type, interfaceIndex,
            timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * Unicast
 *
 * Sends a packet/frame/signal to a destination.
 *------------------------------------------------------*/
void GUI_Unicast(NodeId      sourceID,
                 NodeId      destID,
                 GuiLayers   layer,
                 int         type,
                 int         sendingInterfaceIndex,
                 int         receivingInterfaceIndex,
                 clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(sourceID) == -1) return;
    if (GetNodeIndexFromHash(destID) == -1) return;
    if (!g_layerEnabled[layer] ||
        (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(sourceID)] &&
         !g_nodeEnabledForAnimation[GetNodeIndexFromHash(destID)]))
        return;
    if (!g_eventEnabled[GUI_UNICAST])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %u %d %d %d %s",
            GUI_UNICAST, layer, sourceID, destID, type, sendingInterfaceIndex,
            receivingInterfaceIndex, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * Receive
 *
 * Shows a packet/frame/signal being successfully received at
 * a destination.
 *------------------------------------------------------*/
void GUI_Receive(NodeId      sourceID,
                 NodeId      destID,
                 GuiLayers   layer,
                 int         type,
                 int         sendingInterfaceIndex,
                 int         receivingInterfaceIndex,
                 clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(sourceID) == -1) return;
    if (GetNodeIndexFromHash(destID) == -1) return;
    if (!g_layerEnabled[layer] ||
        !g_nodeEnabledForAnimation[GetNodeIndexFromHash(destID)])
        return;
    if (!g_eventEnabled[GUI_RECEIVE])
        return;

    // Possible for this call to be source less.
    if ((sourceID != 0) &&
        !g_nodeEnabledForAnimation[GetNodeIndexFromHash(sourceID)])
        return;


    // Check if this is the same animation as a receont one, if so return
    GUI_AnimationFilterType filterType;
    filterType.animationType = GUI_RECEIVE;
    filterType.id1 = sourceID;
    filterType.id2 = destID;
    bool same = GUI_CheckSimilarAnimation(filterType);
    if (same)
    {
        return;
    }
    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %u %d %d %d %s",
            GUI_RECEIVE, layer, sourceID, destID, type, sendingInterfaceIndex,
            receivingInterfaceIndex, timeString);
    reply.args.append(replyBuff);
    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE

}

/*------------------------------------------------------
 * Drop
 *
 * Shows a packet/frame/signal being dropped.
 *------------------------------------------------------*/
void GUI_Drop(NodeId      sourceID,
              NodeId      destID,
              GuiLayers   layer,
              int         type,
              int         sendingInterfaceIndex,
              int         receivingInterfaceIndex,
              clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(sourceID) == -1) return;
    if (GetNodeIndexFromHash(destID) == -1) return;
    if (!g_layerEnabled[layer] ||
        !g_nodeEnabledForAnimation[GetNodeIndexFromHash(destID)])
        return;
    if (!g_eventEnabled[GUI_DROP])
        return;

    // Possible for this call to be source less.
    if ((sourceID != 0) &&
        !g_nodeEnabledForAnimation[GetNodeIndexFromHash(sourceID)])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %u %d %d %d %s",
            GUI_DROP, layer, sourceID, destID, type, sendingInterfaceIndex,
            receivingInterfaceIndex, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * Collision
 *
 * Shows a collision.
 *------------------------------------------------------*/
void GUI_Collision(NodeId      nodeID,
                   GuiLayers   layer,
                   clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_layerEnabled[layer] ||
        !g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_COLLISION])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %s",
            GUI_COLLISION, GUI_CHANNEL_LAYER, nodeID, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * Create Subnet
 *
 * Creates a subnet.  Normally done at startup.
 *------------------------------------------------------*/
void GUI_CreateSubnet(GuiSubnetTypes type,
                      NodeAddress    subnetAddress,
                      int            numHostBits,
                      const char*    nodeList,
                      clocktype      time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %d",
            GUI_CREATE_SUBNET, type, subnetAddress,
            numHostBits);
    reply.args.append(replyBuff);
    reply.args.append(" ");
    reply.args.append(nodeList);
    reply.args.append(" ");
    reply.args.append(timeString);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * Create Subnet
 *
 * Creates a subnet.  Normally done at startup.
 *------------------------------------------------------*/
// This function is overloaded function for ipv6.

void GUI_CreateSubnet(GuiSubnetTypes type,
                      char*          ip6Addr,
                      unsigned int   IPv6subnetPrefixLen,
                      const char*    nodeList,
                      clocktype      time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    char addrString[MAX_STRING_LENGTH];

    ctoa(time, timeString);
    IO_ConvertIpAddressToString(ip6Addr, addrString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %s %d",
            GUI_CREATE_SUBNET, type,
            addrString, IPv6subnetPrefixLen);
    reply.args.append(replyBuff);
    reply.args.append(" ");
    reply.args.append(nodeList);
    reply.args.append(" ");
    reply.args.append(timeString);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}


/*------------------------------------------------------
 * Create Hierarchy
 *
 * Since the GUI supports hierarchical design, this function informs
 * the GUI of the contents of a hierarchical component.
 * This method is only called by partition 0.
 *------------------------------------------------------*/
void GUI_CreateHierarchy(int   componentID,
                         char* nodeList) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d",
            GUI_CREATE_HIERARCHY, componentID);
    reply.args.append(replyBuff);
    reply.args.append(" ");
    reply.args.append(nodeList);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * MoveHierarchy
 *
 * Moves the hierarchy to a new position.
 *------------------------------------------------------*/
void GUI_MoveHierarchy(int         hierarchyID,
                       Coordinates centerCoordinates,
                       Orientation orientation,
                       clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %.15f %.15f %.15f %d %d %s",
            GUI_MOVE_HIERARCHY, hierarchyID,
            centerCoordinates.common.c1, centerCoordinates.common.c2,
            centerCoordinates.common.c3, orientation.azimuth,
            orientation.elevation, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}


/*------------------------------------------------------
 * Create Weather
 *------------------------------------------------------*/
void GUI_CreateWeatherPattern(int   patternID,
                              char* inputLine) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d",
            GUI_CREATE_WEATHER_PATTERN, patternID);
    reply.args.append(replyBuff);
    reply.args.append(" ");
    reply.args.append(inputLine);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * MoveWeather
 *
 * Moves the hierarchy to a new position.
 *------------------------------------------------------*/
void GUI_MoveWeatherPattern(int         patternID,
                            Coordinates coordinates,
                            clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %s (%.15f,%.15f)",
            GUI_MOVE_WEATHER_PATTERN, patternID, timeString,
            coordinates.common.c1, coordinates.common.c2);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}


/*------------------------------------------------------
 * Add Application
 *
 * Shows label beside the client and the server as app link is setup.
 *------------------------------------------------------*/
void GUI_AddApplication(NodeId      sourceID,
                        NodeId      destID,
                        char*       appName,
                        int         uniqueId,
                        clocktype   time){
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(sourceID) == -1) return;
    if (GetNodeIndexFromHash(destID) == -1) return;
    if (!g_layerEnabled[GUI_APP_LAYER] ||
        (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(sourceID)] &&
         !g_nodeEnabledForAnimation[GetNodeIndexFromHash(destID)]))
        return;
    if (!g_eventEnabled[GUI_ADD_APP])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %u %s %d %s",
            GUI_ADD_APP, sourceID, destID, appName, uniqueId, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * Delete Application
 *
 * deletes the labels shown by AddApplication.
 *------------------------------------------------------*/
void GUI_DeleteApplication(NodeId      sourceID,
                           NodeId      destID,
                           char*       appName,
                           int         uniqueId,
                           clocktype   time){
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(sourceID) == -1) return;
    if (GetNodeIndexFromHash(destID) == -1) return;
    if (!g_layerEnabled[GUI_APP_LAYER] ||
        (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(sourceID)] &&
         !g_nodeEnabledForAnimation[GetNodeIndexFromHash(destID)]))
        return;
    if (!g_eventEnabled[GUI_DELETE_APP])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %u %s %d %s",
            GUI_DELETE_APP, sourceID, destID, appName,
            uniqueId, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * AddInterfaceQueue
 *
 * Creates a queue for a node, interface and priority
 *------------------------------------------------------*/
void GUI_AddInterfaceQueue(NodeId       nodeID,
                           GuiLayers    layer,
                           int          interfaceIndex,
                           unsigned int priority,
                           int          queueSize,
                           clocktype    currentTime)
{
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];

    // The queue creation cannot be filtered, or it can never
    // be enabled.

    ctoa(currentTime, timeString);
    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %d %u %d %s",
            GUI_QUEUE_CREATE, layer, nodeID, interfaceIndex,
            priority, queueSize, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * QueueInsertPacket
 *
 * Inserting one packet to a queue for a node, interface and priority
 *------------------------------------------------------*/
void GUI_QueueInsertPacket(NodeId       nodeID,
                           GuiLayers    layer,
                           int          interfaceIndex,
                           unsigned int priority,
                           int          packetSize,
                           clocktype    currentTime)
{
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_layerEnabled[layer] ||
        !g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_QUEUE_ADD])
        return;

    ctoa(currentTime, timeString);
    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %d %u %d %s",
            GUI_QUEUE_ADD, layer, nodeID, interfaceIndex,
            priority, packetSize, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * QueueDropPacket
 *
 * Dropping one packet from a queue for a node, interface and priority
 *------------------------------------------------------*/
void GUI_QueueDropPacket(NodeId       nodeID,
                         GuiLayers    layer,
                         int          interfaceIndex,
                         unsigned int priority,
                         clocktype    currentTime)
{
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_layerEnabled[layer] ||
        !g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_QUEUE_DROP])
        return;

    ctoa(currentTime, timeString);
    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %d %u %s",
            GUI_QUEUE_DROP, layer, nodeID, interfaceIndex,
            priority, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * QueueDequeuePacket
 *
 * Dequeuing one packet from a queue for a node, interface and priority
 *------------------------------------------------------*/
void GUI_QueueDequeuePacket(NodeId       nodeID,
                            GuiLayers    layer,
                            int          interfaceIndex,
                            unsigned int priority,
                            int          packetSize,
                            clocktype    currentTime)
{
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_layerEnabled[layer] ||
        !g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_QUEUE_REMOVE])
        return;

    ctoa(currentTime, timeString);
    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %d %u %d %u %d %s",
            GUI_QUEUE_REMOVE, layer, nodeID, interfaceIndex,
            priority, packetSize, timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        printf("%s\n", reply.args.c_str());
    }
#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*------------------------------------------------------
 * DefineMetric
 *
 * This function defines a metric by giving it a name and a
 * description.  The system will assign a number to this data
 * item.  Future references to the data should use the number
 * rather than the name.  The link ID will be used to associate
 * a metric with a particular application link, or MAC interface, etc.
 *------------------------------------------------------*/
int GUI_DefineMetric(const char*  name,
                     NodeId       nodeID,
                     GuiLayers    layer,
                     int          linkID,
                     GuiDataTypes datatype,
                     GuiMetrics   metrictype) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    int      metricID;

    metricID = GetMetricID(name, layer, datatype);

    reply.type = GUI_STATISTICS_COMMAND;
    sprintf(replyBuff, "%d %d \"%s\" %u %d %d %d %d",
            GUI_DEFINE_METRIC, metricID, name, nodeID, layer, linkID,
            (int) datatype, (int) metrictype);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }

#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE

    //printf("Metric (\"%s\") assigned ID %d\n", name, metricID);
    return metricID;
}

/*------------------------------------------------------
 * SendIntegerData
 *
 * Sends data for an integer metric.
 *------------------------------------------------------*/
void GUI_SendIntegerData(NodeId      nodeID,
                         int         metricID,
                         int         value,
                         clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (g_metricFilters[metricID].nodeEnabled[GetNodeIndexFromHash(nodeID)])
    {
        ctoa(time, timeString);

        reply.type = GUI_STATISTICS_COMMAND;
        sprintf(replyBuff, "%d %u %d %d %s",
                GUI_SEND_INTEGER, nodeID, metricID, value, timeString);
        reply.args.append(replyBuff);

        if (GUI_isConnected ()) {
            GUI_SendReply(GUI_guiSocket, reply);
        }

#ifdef MULTI_GUI_INTERFACE
        if (GUI_isMultigui())
        {
            MultiGUI_SendReply(reply);
        }
#endif // MULTI_GUI_INTERFACE

#ifdef HLA_INTERFACE
        if (HlaIsActive())
        {
            assert(metricID < g_numberOfMetrics);
            assert(g_metricFilters[metricID].metricDataPtr != NULL);

            HlaUpdateMetric(
                nodeID,
                *g_metricFilters[metricID].metricDataPtr,
                &value,
                time);
        }
#endif /* HLA_INTERFACE */
#ifdef VRLINK_INTERFACE
        if (VRLinkIsActive())
        {
            assert(metricID < g_numberOfMetrics);
            assert(g_metricFilters[metricID].metricDataPtr != NULL);

            VRLinkUpdateMetric(
                nodeID,
                *g_metricFilters[metricID].metricDataPtr,
                &value,
                time);
        }
#endif /* VRLINK_INTERFACE */
    }
}

/*------------------------------------------------------
 * SendUnsignedData
 *
 * Sends data for an unsigned metric.
 *------------------------------------------------------*/
void GUI_SendUnsignedData(NodeId      nodeID,
                          int         metricID,
                          unsigned    value,
                          clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (g_metricFilters[metricID].nodeEnabled[GetNodeIndexFromHash(nodeID)])
    {
        ctoa(time, timeString);

        reply.type = GUI_STATISTICS_COMMAND;
        sprintf(replyBuff, "%d %u %d %d %s",
                GUI_SEND_UNSIGNED, nodeID, metricID, value, timeString);
        reply.args.append(replyBuff);

        if (GUI_isConnected ()) {
            GUI_SendReply(GUI_guiSocket, reply);
        }

#ifdef MULTI_GUI_INTERFACE
        if (GUI_isMultigui())
        {
            MultiGUI_SendReply(reply);
        }
#endif // MULTI_GUI_INTERFACE

#ifdef HLA_INTERFACE
        if (HlaIsActive())
        {
            assert(metricID < g_numberOfMetrics);
            assert(g_metricFilters[metricID].metricDataPtr != NULL);

            HlaUpdateMetric(
                nodeID,
                *g_metricFilters[metricID].metricDataPtr,
                &value,
                time);
        }
#endif /* HLA_INTERFACE */
#ifdef VRLINK_INTERFACE
        if (VRLinkIsActive())
        {
            assert(metricID < g_numberOfMetrics);
            assert(g_metricFilters[metricID].metricDataPtr != NULL);

            VRLinkUpdateMetric(
                nodeID,
                *g_metricFilters[metricID].metricDataPtr,
                &value,
                time);
        }
#endif /* VRLINK_INTERFACE */
    }
}

/*------------------------------------------------------
 * SendRealData
 *
 * Sends data for a double metric.
 *------------------------------------------------------*/
void GUI_SendRealData(NodeId      nodeID,
                      int         metricID,
                      double      value,
                      clocktype   time) {
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (g_metricFilters[metricID].nodeEnabled[GetNodeIndexFromHash(nodeID)])
    {
        ctoa(time, timeString);

        reply.type = GUI_STATISTICS_COMMAND;
        sprintf(replyBuff, "%d %u %d %.15f %s",
                GUI_SEND_REAL, nodeID, metricID, value, timeString);
        reply.args.append(replyBuff);

        if (GUI_isConnected ()) {
            GUI_SendReply(GUI_guiSocket, reply);
        }

#ifdef MULTI_GUI_INTERFACE
        if (GUI_isMultigui())
        {
            MultiGUI_SendReply(reply);
        }
#endif // MULTI_GUI_INTERFACE

#ifdef HLA_INTERFACE
        if (HlaIsActive())
        {
            assert(metricID < g_numberOfMetrics);
            assert(g_metricFilters[metricID].metricDataPtr != NULL);

            HlaUpdateMetric(
                nodeID,
                *g_metricFilters[metricID].metricDataPtr,
                &value,
                time);
        }
#endif /* HLA_INTERFACE */
#ifdef VRLINK_INTERFACE
        if (VRLinkIsActive())
        {
            assert(metricID < g_numberOfMetrics);
            assert(g_metricFilters[metricID].metricDataPtr != NULL);

            VRLinkUpdateMetric(
                nodeID,
                *g_metricFilters[metricID].metricDataPtr,
                &value,
                time);
        }
#endif /* HLA_INTERFACE */
    }
}

/*------------------------------------------------------
 * MultiGUI_SendReply
 *------------------------------------------------------*/
#ifdef MULTI_GUI_INTERFACE
void MultiGUI_SendReply(GuiReply reply, Int64 connectionId)
{
    char buffer[GUI_MAX_COMMAND_LENGTH + 10] = {0};
    std::string outString;

    sprintf(buffer, "%d ", reply.type);
    outString.append(buffer);
    outString.append(reply.args);
    outString.append("\n");

    unsigned int size;
    GuiEvents guiEvent = GUI_MAX_EVENTS;

    if (reply.type == GUI_ANIMATION_COMMAND)
    {
      sscanf((reply.args).c_str() ,"%d",(int*)&guiEvent);
    }

    if (reply.type == GUI_STEPPED ||
        reply.type == GUI_FINISHED ||
        reply.type == GUI_HITL_RESPONSE ||
        (reply.type == GUI_ANIMATION_COMMAND &&
        (guiEvent == GUI_DEACTIVATE_INTERFACE ||
        guiEvent == GUI_ACTIVATE_INTERFACE ||
        guiEvent == GUI_MOVE_NODE||
        guiEvent == GUI_SET_ORIENTATION ||
            guiEvent == GUI_ADD_LINK ||
            guiEvent == GUI_DELETE_LINK ||
            guiEvent == GUI_MULTICAST ||
            guiEvent == GUI_RECEIVE ||
            guiEvent == GUI_RESET_EXTERNAL_NODE ||
            guiEvent == GUI_SET_EXTERNAL_NODE ||
            guiEvent == GUI_SET_EXTERNAL_NODE_MAPPING ||
            guiEvent == GUI_RESET_EXTERNAL_NODE_MAPPING ||
            guiEvent == GUI_NODE_SIDE_CHANGE)))
    {
        size = sprintf(buffer,"%s",outString.c_str());
        //EXTERNAL_ForwardGuiCommandToProxies(buffer,size);
        MultiGuiIntf_Send(buffer, size, connectionId);
    }
}
#endif // MULTI_GUI_INTERFACE

/*------------------------------------------------------
 * GUI_SetLayerFilter
 *------------------------------------------------------*/
void GUI_SetLayerFilter(const char* args,
                        BOOL  offOrOn) {
    int layer;
    sscanf(args, "%d", &layer);
    QNPartitionLock globalsLock(&GUI_globalsMutex);
    g_layerEnabled[layer] = offOrOn;
}

/*------------------------------------------------------
 * GUI_SetEventFilter
 *------------------------------------------------------*/
void GUI_SetEventFilter(const char* args,
                        BOOL  offOrOn) {
    int eventId;
    sscanf(args, "%d", &eventId);
    if (eventId >= 0 && eventId < GUI_MAX_EVENTS)
    {
        QNPartitionLock globalsLock(&GUI_globalsMutex);
        g_eventEnabled[eventId] = offOrOn;
    }
}

static void GUI_SetVisObjFilter(const char* args,
                                bool  filterOn,
                                clocktype currentTime)
{
    std::string filterId;
    size_t len = strlen(args);
    if (len >= 2 && args[0] == '"' && args[len - 1] == '"')
    {
        filterId.assign(args + 1, len - 2);
    }
    else
    {
        filterId.assign(args, len);
    }

    QNPartitionLock lock(&GUI_globalsMutex);
    std::set<std::string>::iterator it =
        g_activeVisObjFilters.find(filterId);
    if (filterOn && it == g_activeVisObjFilters.end())
    {
        g_activeVisObjFilters.insert(filterId);

        // When the user activates a filter in the GUI, update the GUI with
        // any objects it didn't get while the filter was deactivated.
        // We do this by sending out all objects in our visobj store matching
        // this filter.
        GuiReply reply;
        reply.type = GUI_ANIMATION_COMMAND;
        char buf[GUI_MAX_COMMAND_LENGTH];
        char clock[MAX_CLOCK_STRING_LENGTH];
        ctoa(currentTime, clock);

        // But first, send a synthetic GUI_DELETE_OBJECTS
        // reply to the GUI, in case of a situation like this:
        // 1. GUI disables a filter
        // 2. Before receiving the disable message, the simulator sends out
        //    a new visobj which comes under this filter.
        // 3. Before receiving the event from #2, the GUI re-enables the filter.
        // 4. The GUI now receives the visobj from #2 and registers it.
        // 5. The simulator receives the disable request from #1.
        // 6. The simulator has GUI_DeleteObjects called which removes
        //    the visobj from #2 from its store.  Nothing is sent to the GUI.
        // 7. The simulator receives the enable request from #3.
        //
        // In such a situation, without this GUI_DELETE_OBJECTS reply (which
        // would happen in #5 above), the GUI would end up with a spurious
        // copy of the visobj from #2, which would last until the simulator
        // next sends an appropriate GUI_DELETE_OBJECTS reply (and that might
        // be never if a model has just stabilized).
        sprintf(buf, "%d %d %s \"%s\"",
                GUI_VISUALIZATION_OBJECT,
                GUI_DELETE_OBJECTS,
                clock,
                filterId.c_str());
        reply.args = buf;
        GUI_SendReply(GUI_guiSocket, reply);

        // Now send the objects
        std::multimap<std::string, GUI_VisObjStoreEntry>::iterator objIt;
        for (objIt = g_visObjStore.lower_bound(filterId);
             objIt != g_visObjStore.end() && (*objIt).first == filterId;
             ++objIt)
        {
            sprintf(buf,
                    "%d %d %s %s",
                    GUI_VISUALIZATION_OBJECT,
                    (*objIt).second.cmd,
                    clock,
                    (*objIt).second.text.c_str());
            reply.args = buf;
            GUI_SendReply(GUI_guiSocket, reply);
        }

        // Also check for objects stored under subfilters of filterId.
        std::string filterPrefix = filterId + '|';
        for (objIt = g_visObjStore.lower_bound(filterPrefix);
             objIt != g_visObjStore.end() &&
               (*objIt).first.substr(0, filterPrefix.length()) == filterPrefix;
             ++objIt)
        {
            sprintf(buf,
                    "%d %d %s %s",
                    GUI_VISUALIZATION_OBJECT,
                    (*objIt).second.cmd,
                    clock,
                    (*objIt).second.text.c_str());
            reply.args = buf;
            GUI_SendReply(GUI_guiSocket, reply);
        }
    }
    else if (! filterOn && it != g_activeVisObjFilters.end())
    {
        g_activeVisObjFilters.erase(it);
    }
}

static void GUI_SetVisObjDetails(const char* args,
                                 bool  detailsOn,
                                 clocktype currentTime)
{
    NodeId node;
    if (sscanf(args, "%d", &node) != 1 || node == ANY_NODEID)
    {
        return;
    }

    QNPartitionLock lock(&GUI_globalsMutex);
    std::set<NodeId>::iterator it = g_activeVisObjDetails.find(node);
    if (detailsOn && it == g_activeVisObjDetails.end())
    {
        g_activeVisObjDetails.insert(node);

        GuiReply reply;
        reply.type = GUI_ANIMATION_COMMAND;
        char buf[GUI_MAX_COMMAND_LENGTH];
        char clock[MAX_CLOCK_STRING_LENGTH];
        ctoa(currentTime, clock);

        // For reasons similar to above, ask GUI to clear out any text
        // objects it has registered on this node.
        sprintf(buf,
                "%d %d %s %d",
                GUI_VISUALIZATION_OBJECT,
                GUI_CLEAR_DETAILS,
                clock,
                node);
        reply.args = buf;
        GUI_SendReply(GUI_guiSocket, reply);

        // Send any visobjs which are text objects on this node.
        for (std::multimap<std::string, GUI_VisObjStoreEntry>::iterator objIt =
                 g_visObjStore.begin();
             objIt != g_visObjStore.end();
             ++objIt)
        {
            if ((*objIt).second.detailsNodeId == node)
            {
                sprintf(buf,
                        "%d %d %s %s",
                        GUI_VISUALIZATION_OBJECT,
                        (*objIt).second.cmd,
                        clock,
                        (*objIt).second.text.c_str());
                reply.args = buf;
                GUI_SendReply(GUI_guiSocket, reply);
            }
        }
    }
    else if (! detailsOn && it != g_activeVisObjDetails.end())
    {
        g_activeVisObjDetails.erase(it);
    }
}

/*------------------------------------------------------
 * GUI_SetNodeFilter
 *------------------------------------------------------*/
void GUI_SetNodeFilter(PartitionData * partitionData,
                       const char* args,
                       BOOL  offOrOn) {
    QNPartitionLock globalsLock(&GUI_globalsMutex);
    //args should contain the nodeID
    NodeId      nodeID = atoi(args);
    if (nodeID == 0) {
        // set all nodes
        int index;
        for (index = 0; index < g_numberOfNodes; index++) {
            g_nodeEnabledForAnimation[index] = offOrOn;
        }
    }
    else {
        if (GetNodeIndexFromHash(nodeID) == -1) return;
        int index = GetNodeIndexFromHash(nodeID);

        g_nodeEnabledForAnimation[index] = offOrOn;
    }
}

/*------------------------------------------------------
 * GUI_SetAnimationFilterFrequency
 *------------------------------------------------------*/
void GUI_SetAnimationFilterFrequency(const char* args) {
    QNPartitionLock globalsLock(&GUI_globalsMutex);
    g_animationFilterFrequency = /*SECOND * */atof(args);
}

/*------------------------------------------------------
 * GUI_SetMetricFilter
 *------------------------------------------------------*/
void GUI_SetMetricFilter(const char* args,
                         BOOL  offOrOn) {
    QNPartitionLock globalsLock(&GUI_globalsMutex);
    //args should contain both metric ID and node or link ID and layer
    //we don't currently deal with link ID, so assume it's a node ID
    int metricID;
    int nodeID;
    int layer;
    sscanf(args, "%d %d %d", &metricID, &nodeID, &layer);
    if (nodeID == 0) {
        // all nodes
        int i;
        for (i = 0; i < g_numberOfNodes; i++) {
            g_metricFilters[metricID].nodeEnabled[i] = offOrOn;
            if (offOrOn) {
                g_nodeEnabledForStatistics[i]++;
                g_statLayerEnabled[layer]++;
            }
            else {
                g_nodeEnabledForStatistics[i]--;
                g_statLayerEnabled[layer]--;
            }
        }
    }
    else {
        if (GetNodeIndexFromHash(nodeID) == -1) return;
        int index = GetNodeIndexFromHash(nodeID);
        g_metricFilters[metricID].nodeEnabled[index] = offOrOn;
        if (offOrOn) {
            g_nodeEnabledForStatistics[index]++;
            g_statLayerEnabled[layer]++;
        }
        else {
            g_nodeEnabledForStatistics[index]--;
            g_statLayerEnabled[layer]--;
        }
    }
}

/*------------------------------------------------------
 * GUI_NodeIsEnabledForAnimation
 *------------------------------------------------------*/
BOOL GUI_NodeIsEnabledForAnimation(NodeId nodeID) {
    if (GetNodeIndexFromHash(nodeID) == -1) false;
    int index = GetNodeIndexFromHash(nodeID);
    return g_nodeEnabledForAnimation[index];
}

/*------------------------------------------------------
 * GUI_NodeIsEnabledForStatistics
 *------------------------------------------------------*/
BOOL GUI_NodeIsEnabledForStatistics(NodeId nodeID) {
    if (GetNodeIndexFromHash(nodeID) == -1) return false;
    int index = GetNodeIndexFromHash(nodeID);
    if (g_nodeEnabledForStatistics[index] > 0)
        return TRUE;
    else
        return FALSE;
}

/*------------------------------------------------------
 * GUI_LayerIsEnabledForStatistics
 *------------------------------------------------------*/
BOOL GUI_LayerIsEnabledForStatistics(GuiLayers layer) {
    return g_statLayerEnabled[layer];
}

// /*
//  *   This function is installed as an ErrorHandler so that when
//  *   ERORR_WriteMessage () reports an error this handler will
//  *   be triggered so that we can notify the GUI of the warning/error.
//  */
// MT Safe
void EXTERNAL_GUI_ErrorHandler (int type, const char * errorMessage)
{
    std::string error;

    // local copy so we can modify it.
    error = errorMessage;

    GuiReplies replyType = GUI_STEPPED;
    switch (type) {
        case ERROR_ASSERTION:
            replyType = GUI_ASSERTION;
            break;
        case ERROR_ERROR:
            replyType = GUI_ERROR;
            break;
        case ERROR_WARNING:
            replyType = GUI_WARNING;
            break;
        default:
            abort();
    }

    if (GUI_isConnected ()) {
        GuiCommand command;
        GUI_SendReply(GUI_guiSocket,
                      GUI_CreateReply(replyType, &error));
        if (type != ERROR_WARNING) {
            // The simulation is exiting via a call to abort ().
            // However, before terminating we wait for the IDE to
            // send us a final handshake sent so that we know it
            // got the message that the simulation is aborting.
            do {
                command = GUI_ReceiveCommand(GUI_guiSocket);
            } while (command.type != GUI_STOP);
            GUI_DisconnectFromGUI(GUI_guiSocket);
        }
    }
}

#ifdef D_LISTENING_ENABLED
GuiDynamicObjectCallback::GuiDynamicObjectCallback(
    EXTERNAL_Interface* iface,
    GuiEvents eventType)
    : m_Iface(iface), m_EventType(eventType)
{
    // empty
}

void GuiDynamicObjectCallback::operator () (const std::string& newValue)
{
    // Send gui command to add new object if connected
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    reply.type = GUI_DYNAMIC_COMMAND;
    if (m_EventType == GUI_DYNAMIC_AddObject
        || m_EventType == GUI_DYNAMIC_ObjectPermissions)
    {
        sprintf(replyBuff, "%d ", m_EventType);
        reply.args.append(replyBuff);
        reply.args.append(newValue);
        reply.args.append(" ");

        // Add rwx permissions
        if (m_Iface->partition->dynamicHierarchy.IsReadable(newValue))
        {
            reply.args.append("r");
        }
        if (m_Iface->partition->dynamicHierarchy.IsWriteable(newValue))
        {
            reply.args.append("w");
        }
        if (m_Iface->partition->dynamicHierarchy.IsExecutable(newValue))
        {
            reply.args.append("x");
        }
    }
    else
    {
        sprintf(replyBuff, "%d ", m_EventType);
        reply.args.append(replyBuff);
        reply.args.append(newValue);
        reply.args.append(" ");
    }
    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else {
        printf("%s\n", reply.args.c_str());
    }
}

GuiDynamicObjectValueCallback::GuiDynamicObjectValueCallback(
    EXTERNAL_Interface* iface,
    const std::string &path)
    : m_Iface(iface), m_Path(path)
{
    // empty
}

void GuiDynamicObjectValueCallback::operator () (const std::string& newValue)
{
    // Send gui command to add new object if connected
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    reply.type = GUI_DYNAMIC_COMMAND;
    sprintf(replyBuff, "%d ",
        GUI_DYNAMIC_ObjectValue);
    reply.args.append(replyBuff);
    reply.args.append(m_Path);
    reply.args.append(" ");
    reply.args.append(newValue);
    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else {
        printf("%s\n", reply.args.c_str());
    }
}
#endif

static BOOL GUI_isConnected ()
{
    return GUI_guiSocketOpened;
}
BOOL GUI_isAnimate ()
{
    return GUI_animationTrace;
}

#ifdef MULTI_GUI_INTERFACE
BOOL GUI_isMultigui ()
{
    return GUI_multigui;
}
#endif // MULTI_GUI_INTERFACE

bool GUI_isAnimateOrInteractive ()
{
    // Animation (.trc) or interactive behavior is true.
    if (GUI_isConnected () || GUI_isAnimate ()
#ifdef MULTI_GUI_INTERFACE
        || GUI_isMultigui ()
#endif // MULTI_GUI_INTERFACE
        )
        return true;
    return false;
}

// Main, before creating threads for each partition will
// call EXTERNAL_Bootstrap (), which will call this. In MPI
// this will be called on each process.
bool GUI_EXTERNAL_Bootstrap(int argc, char * argv [], NodeInput * nodeInput, int thisPartitionId)
{
    bool    active = false;
    bool    reportErrors = true;
    BOOL     wasFound;
    char     buf[MAX_STRING_LENGTH];

    // main does this for us.
    // PrintSimTimeInterval = maxClock / NUM_SIM_TIME_STATUS_PRINTS;

    //
    /* Set the statistics and status print intervals. */
    // These global variable have to be set here because
    // these global variables are used by main, however
    // GUI_Initialize () won't be called unless external interfaces
    // are active. We always know bootstrap is called, so set
    // these variable here.
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STAT_RUN_TIME_INTERVAL",
        &wasFound,
        buf);

    if (wasFound == FALSE) {
        GUI_statInterval = CLOCKTYPE_MAX;
    }
    else {
        GUI_statInterval = TIME_ConvertToClock(buf);
        if (GUI_statInterval <= 0) {
            ERROR_ReportError("Invalid entry for STAT_RUN_TIME_INTERVAL\n");
        }
    }
    GUI_statReportTime = GUI_statInterval;
    if (thisPartitionId != 0)
    {
        // We are MPI, and each process will examine the argc/argv.
        // Only p0 needs to complain about probelms with the cmd line
        // args.
        reportErrors = false;
    }

    int thisArg = 2;
    while (thisArg < argc) {
        /*
         * Set up socket connection to GUI
         */
        if (!strcmp(argv[thisArg], "-interactive")) {
            char hostname[64];
            int  portNumber;
            //
            // if thisarg is -interactive, then next two must be host port
            //
            if (argc < thisArg + 3) {
                if (reportErrors) {
                    ERROR_ReportError(
                        "Not enough arguments to -interactive.\n"
                        "Correct Usage:\t -interactive host port.\n");
                }
            }

            strcpy(hostname, argv[thisArg+1]);
            portNumber = atoi(argv[thisArg+2]);

            GUI_ConnectToGUI(hostname, (unsigned short) portNumber);
            SendUidToGuiClient();
#ifdef PARALLEL //Parallel
            if (GUI_guiSocketOpened) {
                PARALLEL_SetGreedy(false);
            }
#endif //endParallel
            // Add our handler to send error messages back to the gui.
            ERROR_InstallHandler (&EXTERNAL_GUI_ErrorHandler);
            // guiEnabled = true;
            active = true;
            thisArg += 3;
        }

        /*
         * Check whether animation is enabled.  If the GUI is interactive,
         * then animation is implied.
         */
        else if (!strcmp(argv[thisArg], "-animate")) {
            GUI_animationTrace = TRUE;
            active = true;
            thisArg++;
        }
#ifdef MULTI_GUI_INTERFACE
        else if (!strcmp(argv[thisArg], "-multigui")) {
            int  portNumber;
            //
            // if thisarg is -multigui, then next one must be multi gui port
            //
            if (argc < thisArg + 2) {
                if (reportErrors) {
                    ERROR_ReportError(
                        "Not enough arguments to -multigui.\n"
                        "Correct Usage:\t -multigui port.\n");
                }
            }
            portNumber = atoi(argv[thisArg+1]);
            setMultiguiPort(portNumber);
            GUI_multigui = TRUE;
            active = true;
            thisArg += 2;
        }
#endif // MULTI_GUI_INTERFACE
        else {
          thisArg++;
        }
    }
    return active;
}

void GUI_EXTERNAL_Finalize (EXTERNAL_Interface *iface) {
    // Close socket connection to GUI.
    if (GUI_isConnected ()) {
        if (DEBUG) {
            printf ("Finalizing - performing disconnect\n");
            fflush(stdout);
        }
        GUI_DisconnectFromGUI(GUI_guiSocket);
    }
}

#ifdef MULTI_GUI_INTERFACE
void MULTIGUI_EXTERNAL_Finalize (EXTERNAL_Interface *iface) {
    // Close socket connection to GUI.
    if (DEBUG) {
        printf ("Finalizing - performing disconnect\n");
        fflush(stdout);
    }

    std::string replyText = " ";
    MultiGUI_SendReply(GUI_CreateReply(GUI_FINISHED, &replyText));
}
#endif // MULTI_GUI_INTERFACE

/*--
 *  Return the simulation time that the IDE has advanced to
 */
clocktype GUI_EXTERNAL_QueryExternalTime (EXTERNAL_Interface *iface) {
    GUI_InterfaceData *     data;
    data = (GUI_InterfaceData*) iface->data;
    return data->guiExternalTime;
}

/*--
 *  Return the simulation time that the IDE will allow the simulation
 *  to execute out to.
 */
void GUI_EXTERNAL_SimulationHorizon (EXTERNAL_Interface *iface) {
    // Don't need to do anything, as the iface's horizon is
    // advanced as step commands arrive from the IDE.
    return;
}

void DeactivateNode(EXTERNAL_Interface* iface, const char* nodeId)
{
    NodeAddress id = atoi(nodeId);
    Node* node;
    BOOL success = FALSE;
    success = PARTITION_ReturnNodePointer(iface->partition, &node, id, TRUE);

    if (success == TRUE)
    {
        EXTERNAL_DeactivateNode(
            iface,
            node);
    }
}

void ActivateNode(EXTERNAL_Interface* iface, const char* nodeId)
{
    NodeAddress id = atoi(nodeId);
    Node* node;
    BOOL success = FALSE;
    success = PARTITION_ReturnNodePointer(iface->partition, &node, id, TRUE);

    if (success == TRUE)
    {
        EXTERNAL_ActivateNode(
            iface,
            node);
    }
}

#ifdef AUTO_IPNE_INTERFACE
void AutoIPNE_SetVirtualNode(
    PartitionData* partitionData,
    char* value1,
    char* value2,
    char* value3)
{
    Address physicalAddress;
    Address virtualAddress;
    int flag = atoi(value1);

    EXTERNAL_Interface *autoIPNE = NULL;

    autoIPNE = EXTERNAL_GetInterfaceByName(
        &partitionData->interfaceList,
        "IPNE");

    BOOL isNodeId = FALSE;
    IO_ParseNodeIdHostOrNetworkAddress(
        value2,
        &virtualAddress,
        &isNodeId);
    isNodeId = FALSE;
    IO_ParseNodeIdHostOrNetworkAddress(
        value3,
        &physicalAddress,
        &isNodeId);

    if (flag) //addition
    {
        AutoIPNE_AddVirtualNode(autoIPNE,
            physicalAddress,
            virtualAddress,
            FALSE);
    }
    else //deletion
    {
        AutoIPNE_RemoveVirtualNode(autoIPNE,
            physicalAddress,
            virtualAddress,
            FALSE);

    }
}

#endif

#ifdef PAS_INTERFACE
void PSI_BroadcastToPartitions(
    PartitionData *partitionData,
    int messageType,
    int data)
{
    Message *msg, *dupMsg;

    msg = MESSAGE_Alloc(partitionData->firstNode,
                    EXTERNAL_LAYER,
                    EXTERNAL_PAS,
                    messageType);

    MESSAGE_PacketAlloc(partitionData->firstNode, msg, sizeof(int), TRACE_IP);
    *(int *)MESSAGE_ReturnPacket(msg) = data;

    for (int i = 1; i < partitionData->getNumPartitions(); i++)
    {
        printf("Sending pas messagae %d from %d to %d\n", messageType, partitionData->partitionId, i);
            fflush(stdout);
        dupMsg = MESSAGE_Duplicate(partitionData->firstNode, msg);
        EXTERNAL_MESSAGE_RemoteSend(partitionData->interfaceTable[EXTERNAL_PAS],
                        i,
                        dupMsg,
                        0,
                        EXTERNAL_SCHEDULE_SAFE);
    }

    MESSAGE_Free(partitionData->firstNode, msg);

}

void PSI_EnablePSI(PartitionData* partitionData, char* nodeId)
{
    EXTERNAL_Interface *pas = NULL;
    PASData *pasData = NULL;

    NodeAddress id = atoi(nodeId);

    pas = EXTERNAL_GetInterfaceByName(
        &partitionData->interfaceList,
        "PAS");

    if (!pas)
        return;

    pasData = (PASData*) pas->data;

    if (id == -1) //disable PSI
    {
        if ((partitionData->partitionId == 0)
            && pasData->isPAS && (pasData->pasId != 0))
        {
            GUI_ResetExternalNode(pasData->pasId, 2,
                partitionData->getGlobalTime());
        }
        pasData->isPAS = FALSE;
    }
    else
    {
        // If PAS was enabled earlier, then ask gui to
        // stop showing the earlier highlight
        if ((partitionData->partitionId == 0) && pasData->isPAS &&
            (pasData->pasId != 0))
        {
            GUI_ResetExternalNode(pasData->pasId, 1,
                partitionData->getGlobalTime());
        }
        pasData->isPAS = TRUE;
        pasData->pasId = id;
        if ((partitionData->partitionId == 0) && id != 0)
        {
            if (SopsVopsInterfaceEnabled())
            {
                Node* node = NULL;
                BOOL isValidNodePointer = PARTITION_ReturnNodePointer(
                    partitionData,
                    &node,
                    pasData->pasId);

#ifdef SOPSVOPS_INTERFACE
                if (isValidNodePointer)
                {
                    char key[MAX_STRING_LENGTH];
                    char value[MAX_STRING_LENGTH];

                    sprintf(key, "/node/%d/exata/status", node->nodeId);
                    strcpy(value, "enabled");
                    SetSopsProperty(key, value);

                    sprintf(key, "/node/%d/interface/%d/externalHostAddress", node->nodeId, -1);
                    strcpy(value, "");
                    SetSopsProperty(key, value);


                    sprintf(key, "/node/%d/exata/isDefaultExternalNode", node->nodeId);
                    strcpy(value, "1");
                    SetSopsProperty(key, value);
                }
            }
#endif
            GUI_SetExternalNode(pasData->pasId, 1,
                partitionData->getGlobalTime());
        }
    }

    if ((partitionData->partitionId == 0) && (partitionData->isRunningInParallel()))
    {
        PSI_BroadcastToPartitions(partitionData,
                    EXTERNAL_MESSAGE_PAS_EnablePSI,
                    id);
    }
}
void PSI_EnableDot11(PartitionData* partitionData, BOOL flag)
{
    EXTERNAL_Interface *pas = NULL;
    PASData *pasData = NULL;

    pas = EXTERNAL_GetInterfaceByName(
        &partitionData->interfaceList,
        "PAS");

    if (!pas)
        return;

    pasData = (PASData*) pas->data;

    if (flag) //enable
        pasData->isDot11 = TRUE;
    else //disable
        pasData->isDot11 = FALSE;

    if ((partitionData->partitionId == 0) && (partitionData->isRunningInParallel()))
    {
        PSI_BroadcastToPartitions(partitionData,
                    EXTERNAL_MESSAGE_PAS_EnableDot11,
                    flag);
    }
}

void PSI_EnableApp(PartitionData* partitionData, BOOL flag)
{
    EXTERNAL_Interface *pas = NULL;
    PASData *pasData = NULL;

    pas = EXTERNAL_GetInterfaceByName(
        &partitionData->interfaceList,
        "PAS");

    if (!pas)
        return;

    pasData = (PASData*) pas->data;

    if (flag) //enable
        pasData->pasFilterBit |= PACKET_SNIFFER_ENABLE_APP;
    else //disable
        pasData->pasFilterBit &= ~PACKET_SNIFFER_ENABLE_APP;

    if ((partitionData->partitionId == 0) &&
        (partitionData->isRunningInParallel()))
    {
        PSI_BroadcastToPartitions(partitionData,
                    EXTERNAL_MESSAGE_PAS_EnableApp,
                    flag);
    }
}

void PSI_EnableUdp(PartitionData* partitionData, BOOL flag)
{
    EXTERNAL_Interface *pas = NULL;
    PASData *pasData = NULL;

    pas = EXTERNAL_GetInterfaceByName(
        &partitionData->interfaceList,
        "PAS");

    if (!pas)
        return;


    pasData = (PASData*) pas->data;

    if (flag) //enable
        pasData->pasFilterBit |= PACKET_SNIFFER_ENABLE_UDP;
    else //disable
        pasData->pasFilterBit &= ~PACKET_SNIFFER_ENABLE_UDP;


    if ((partitionData->partitionId == 0) && (partitionData->isRunningInParallel()))
    {
        PSI_BroadcastToPartitions(partitionData,
                    EXTERNAL_MESSAGE_PAS_EnableUdp,
                    flag);
    }
}

void PSI_EnableTcp(PartitionData* partitionData, BOOL flag)
{
    EXTERNAL_Interface *pas = NULL;
    PASData *pasData = NULL;

    pas = EXTERNAL_GetInterfaceByName(
        &partitionData->interfaceList,
        "PAS");

    if (!pas)
        return;

    pasData = (PASData*) pas->data;

    if (flag) //enable
        pasData->pasFilterBit |= PACKET_SNIFFER_ENABLE_TCP;
    else //disable
        pasData->pasFilterBit &= ~PACKET_SNIFFER_ENABLE_TCP;


    if ((partitionData->partitionId == 0) && (partitionData->isRunningInParallel()))
    {
        PSI_BroadcastToPartitions(partitionData,
                    EXTERNAL_MESSAGE_PAS_EnableTcp,
                    flag);
    }
}

void PSI_EnableRouting(PartitionData* partitionData, BOOL flag)
{
    EXTERNAL_Interface *pas = NULL;
    PASData *pasData = NULL;

    pas = EXTERNAL_GetInterfaceByName(
        &partitionData->interfaceList,
        "PAS");

    if (!pas)
        return;


    pasData = (PASData*) pas->data;

    if (flag) //enable
        pasData->pasFilterBit |= PACKET_SNIFFER_ENABLE_ROUTING;
    else //disable
        pasData->pasFilterBit &= ~PACKET_SNIFFER_ENABLE_ROUTING;

    if ((partitionData->partitionId == 0) && (partitionData->isRunningInParallel()))
    {
        PSI_BroadcastToPartitions(partitionData,
                    EXTERNAL_MESSAGE_PAS_EnableRouting,
                    flag);
    }
}

void PSI_EnableMac(PartitionData* partitionData, BOOL flag)
{
    EXTERNAL_Interface *pas = NULL;
    PASData *pasData = NULL;

    pas = EXTERNAL_GetInterfaceByName(
        &partitionData->interfaceList,
        "PAS");

    if (!pas)
        return;


    pasData = (PASData*) pas->data;

    if (flag) //enable
        pasData->pasFilterBit |= PACKET_SNIFFER_ENABLE_MAC;
    else //disable
        pasData->pasFilterBit &= ~PACKET_SNIFFER_ENABLE_MAC;


    if ((partitionData->partitionId == 0) && (partitionData->isRunningInParallel()))
    {
        PSI_BroadcastToPartitions(partitionData,
                    EXTERNAL_MESSAGE_PAS_EnableMac,
                    flag);
    }
}
#endif

#ifdef HITL
// These are ultra simple and primitive functions used to modify
// all nodes of the scenario during the simulation.
void SetCbrPrecedence(PartitionData* partitionData, const char* p)
{
    int precedence = *p - '0';
    int i;
    int j;
    int childrenA;
    int childrenB;
    //char* childNameA;
    //char* childNameB;
    std::string childNameA;
    std::string childNameB;
    //char path[MAX_STRING_LENGTH];
    std::string path;
    char value[MAX_STRING_LENGTH];

    if (precedence < 0 || precedence > 7)
    {
        return;
    }

    // Put in TOS form
    sprintf(value, "%d", precedence << 5);

    // NOTE: Here we should add * capability to paths
    try
    {
        // Loop over all nodes
        if (partitionData->dynamicHierarchy.IsEnabled () == false)
        {
            ERROR_ReportWarning("CBR precedence cannot be changed because DYNAMIC-ENABLED isn't set for this scenario");
            return;
        }
        childrenA = partitionData->dynamicHierarchy.GetNumChildren("/node");
        for (i = 0; i < childrenA; i++)
        {
            childNameA = (partitionData->dynamicHierarchy.GetChildName("/node", i)).c_str();

            //sprintf(path, "/node/%s/cbrClient", childNameA);
            path = "/node/"+childNameA+"/cbrClient";
            try
            {
                // Loop over all cbr apps
                childrenB = partitionData->dynamicHierarchy.GetNumChildren(path);
                for (j = 0; j < childrenB; j++)
                {
                    childNameB = (partitionData->dynamicHierarchy.GetChildName(path, j)).c_str();
                    //sprintf(
                    //    path,
                     //   "/node/%s/cbrClient/%s/tos",
                      //  childNameA,
                       // childNameB);
                    path = "/node/"+childNameA+"/cbrClient/"+childNameB+"/tos";

                    // If this is a valid path (no exception was thrown)
                    // then write the new value
                    partitionData->dynamicHierarchy.WriteAsString(path, value);
                }
            }
            catch (D_Exception&)
            {
                // Ignore -- path doesn't exist, not an error.
            }
        }
    }
    catch (D_Exception &e)
    {
        e.GetFullErrorString(path);
        printf("exception: %s\n", path.c_str());
    }

    partitionData->dynamicHierarchy.Print();
}

// for n command: change the tos for AppForward (IPNE)
void SetCbrPrecedenceAppForward(PartitionData* partitionData, const char* p)
{
    int precedence = *p - '0';
    int i;
    int j;
    int childrenA;
    int childrenB;
    //char* childNameA;
    //char* childNameB;
    //char path[MAX_STRING_LENGTH];
    std::string childNameA;
    std::string childNameB;
    std::string path;
    char value[MAX_STRING_LENGTH];

    if (precedence < 0 || precedence > 7)
    {
        return;
    }

    // Put in TOS form
    sprintf(value, "%d", precedence << 5);

    // NOTE: Here we should add * capability to paths
    try
    {
        // Loop over all nodes
        if (partitionData->dynamicHierarchy.IsEnabled () == false)
        {
            ERROR_ReportWarning("CBR precedence cannot be changed because DYNAMIC-ENABLED isn't set for this scenario");
            return;
        }
        childrenA = partitionData->dynamicHierarchy.GetNumChildren("/node");
        for (i = 0; i < childrenA; i++)
        {
            childNameA = partitionData->dynamicHierarchy.GetChildName("/node", i);
            //sprintf(path, "/node/%s/appForward", childNameA);
            path = "/node/"+childNameA+"/appForward";
            try
            {
                // Loop over all cbr apps
                childrenB = partitionData->dynamicHierarchy.GetNumChildren(path);
                for (j = 0; j < childrenB; j++)
                {
                    childNameB = partitionData->dynamicHierarchy.GetChildName(path, j);
                    /*
                    sprintf(
                        path,
                        "/node/%s/appForward/%s/tos",
                        childNameA,
                        childNameB);
                    */
                    path = "/node/"+childNameA+"/appForward/"+childNameB+"/tos";

                    // If this is a valid path (no exception was thrown)
                    // then write the new value
                    partitionData->dynamicHierarchy.WriteAsString(path, value);
                }
            }
            catch (D_Exception&)
            {
                // Ignore -- path doesn't exist, not an error.
            }
        }
    }

    catch (D_Exception &e)
    {
        e.GetFullErrorString(path);
        printf("exception: %s\n", path.c_str());
    }

    partitionData->dynamicHierarchy.Print();
}

// These are ultra simple and primitive functions used to modify
// all nodes of the scenario during the simulation.
// T command
void SetCbrTrafficRate(PartitionData* partitionData, const char* r)
{
    int i;
    int j;
    int childrenA;
    int childrenB;
    std::string childNameA;
    std::string childNameB;
    std::string path;
    //char value[MAX_STRING_LENGTH];
    std::string value;

    //parse input rate -> input is MS
    while (*r != 0 && isspace(*r)) {
        r++;
    }

    //sanity check
    char temp[10];
    strncpy(temp, r, strlen(r));
    for (i = 0; i < (int) strlen(r);i++)
    {
        if (!isdigit(temp[i]) && (temp[i]!= '.')) {
            printf("should be digit\n");
            return;
        }
    }
    if (!strncmp(r, "10", 2) && isspace(*(r+2)))
    {
        value = "10.0MS";
    }
    else
    {
        value.assign(r);
        value.append("MS");
    }

    // NOTE: Here we should add * capability to paths
    try
    {
        if (partitionData->dynamicHierarchy.IsEnabled () == false)
        {
            ERROR_ReportWarning("CBR traffic rate cannot be changed because DYNAMIC-ENABLED isn't set for this scenario");
            return;
        }
        // Loop over all nodes
        childrenA = partitionData->dynamicHierarchy.GetNumChildren("/node");
        for (i = 0; i < childrenA; i++)
        {
            childNameA = partitionData->dynamicHierarchy.GetChildName("/node", i);

            //sprintf(path, "/node/%s/cbrClient", childNameA);
            path = "/node/"+childNameA+"/cbrClient";
            try
            {
                // Loop over all cbr apps
                childrenB = partitionData->dynamicHierarchy.GetNumChildren(path);
                for (j = 0; j < childrenB; j++)
                {
                    childNameB = partitionData->dynamicHierarchy.GetChildName(path, j);
                    //sprintf(
                    //    path,
                    //    "/node/%s/cbrClient/%s/interval",
                    //    childNameA,
                    //    childNameB);
                    path = "/node/"+childNameA+"/cbrClient/"+childNameB+"/interval";

                    // If this is a valid path (no exception was thrown)
                    // then write the new value
                    partitionData->dynamicHierarchy.WriteAsString(path, value);
                }
            }
            catch (D_Exception&)
            {
                // Ignore -- path doesn't exist, not an error.
            }
        }
    }
    catch (D_Exception &e)
    {
        e.GetFullErrorString(path);
        printf("exception: %s\n", path.c_str());
    }
    partitionData->dynamicHierarchy.Print();
}

// modify the CBR interval for all nodes of the scenario
// L command
void SetCbrTrafficRateTimes(PartitionData* partitionData, const char* r)
{
    int i;
    int j;
    int childrenA;
    int childrenB;
    std::string childNameA;
    std::string childNameB;
    std::string path;
    std::string value;

    //parse input rate -> input is MS
    while (*r != 0 && isspace(*r)) {
        r++;
    }

    //sanity check
    char temp[10];
    strncpy(temp, r, strlen(r));
    for (i = 0; i < (int) strlen(r);i++)
    {
        if (!isdigit(temp[i]) && (temp[i]!= '.')) {
            printf("should be digit\n");
            return;
        }
    }

    // NOTE: Here we should add * capability to paths
    try
    {
        if (partitionData->dynamicHierarchy.IsEnabled () == false)
        {
            ERROR_ReportWarning("CBR traffic rate cannot be changed because DYNAMIC-ENABLED isn't set for this scenario");
            return;
        }
        // Loop over all nodes
        childrenA = partitionData->dynamicHierarchy.GetNumChildren("/node");
        for (i = 0; i < childrenA; i++)
        {
            childNameA = partitionData->dynamicHierarchy.GetChildName("/node", i);

            //sprintf(path, "/node/%s/cbrClient", childNameA);
            path = "/node/"+childNameA+"/cbrClient";
            try
            {
                // Loop over all cbr apps
                childrenB = partitionData->dynamicHierarchy.GetNumChildren(path);
                for (j = 0; j < childrenB; j++)
                {
                    childNameB = partitionData->dynamicHierarchy.GetChildName(path, j);
                    path = "/node/"+childNameA+"/cbrClient/"+childNameB+"/interval";

                    // If this is a valid path (no exception was thrown)
                    // read the value and multiply
                    partitionData->dynamicHierarchy.ReadAsString(path, value);
                    std::string newValue;
                    float tempValue = (float) strtod(value.c_str(),NULL);
                    float tempInput = (float) strtod(r,NULL);

                    std::stringstream ss;
                    ss << ((tempValue * tempInput)/1000000);
                    ss >> newValue;

                    newValue = newValue + "MS";
                    partitionData->dynamicHierarchy.WriteAsString(path, newValue);
                }
            }
            catch (D_Exception&)
            {
                // Ignore -- path doesn't exist, not an error.
            }
        }
    }
    catch (D_Exception &e)
    {
        e.GetFullErrorString(path);
        printf("exception: %s\n", path.c_str());
    }
    partitionData->dynamicHierarchy.Print();
}

#endif

void GUI_HandleHITLInput(const char* args, PartitionData* partition)
{
    // BEGIN HUMAN IN THE LOOP CODE

#ifdef HITL
    char* ch = NULL;
    char guiCommand[MAX_STRING_LENGTH] = "";

    if ((strcmp(args, " ") == 0) || (strcmp(args, "") == 0))
    {
        return;
    }
    if (strlen(args) > MAX_STRING_LENGTH)
    {
        ERROR_ReportWarning(
            "guiCommand is greater than MAX_STRING_LENGTH\n");
        return;
    }

    char* localArgs = (char*) MEM_malloc(strlen(args) + 1);
    strcpy(localArgs, args);

    // Trim leading and trailing spaces in the entered command
    IO_TrimLeft(localArgs);
    IO_TrimRight(localArgs);

    // Truncate multiple spaces to single space
    TruncateSpaces(localArgs);

    // firewall is a case sensitive command as iptables so in case of
    // firewall, the command should not be changed into upper case.
#ifdef CYBER_LIB
    if (strncmp(localArgs, "FIREWALL ", strlen("FIREWALL "))
         && strncmp(localArgs, "firewall ", strlen("firewall ")))
    {
        IO_ConvertStringToUpperCase(localArgs);
    }
#else
    IO_ConvertStringToUpperCase(localArgs);
#endif

#ifdef CYBER_LIB
    HITLApplication hitlApp;
    Node* firstNode = partition->firstNode;

    if (!strncmp(localArgs, "DOS ", strlen("DOS ")))
    {
        std::string cyberInput (localArgs);
        hitlApp.appType = APP_DOS_ATTACKER;

        std::size_t indexOfID = std::string::npos;
        std::string hitlID = GUI_GetHID(cyberInput, &indexOfID);

        APP_InitializeCyberApplication(
            firstNode, cyberInput, TRUE, hitlApp.apps, hitlID);

        if (indexOfID != std::string::npos)
        {
            if (hitlApp.apps.size() > 0)
            {
                if (partition->hitlApplicationMap.
                        insert(
                        std::pair<std::string, HITLApplication>
                        (hitlID, hitlApp)).second == false)
                {
                    ERROR_ReportError("Insertion failed in HITLApplication map.");
                }
            }
            else
            {
                GUI_CreateHITLResponse(hitlID, GUI_HITL_ERROR);
            }
        }
        return;
    }

    if (!strncmp(localArgs, "STOP DOS", strlen("STOP DOS")))
    {
        std::string cyberInput (localArgs);
        std::size_t indexOfID = std::string::npos;
        std::string hitlID = GUI_GetHID(cyberInput, &indexOfID);

        if (indexOfID != std::string::npos)
        {

            std::map<std::string, HITLApplication>::const_iterator it;
            it = partition->hitlApplicationMap.find(hitlID);

            if (it != partition->hitlApplicationMap.end())
            {
                HITLApplication dosApp = it->second;

                if (dosApp.appType != APP_DOS_ATTACKER)
                {
                    std::string errorStr =
                        "[STOP DOS] Passed HID does not"
                        " signify any active dos attack";
                    ERROR_ReportWarning(errorStr.c_str());
                    GUI_CreateHITLResponse(
                        hitlID, GUI_HITL_ERROR, errorStr);
                    return;
                }
                else
                {
                    std::vector<HITLInfo> attackersInfo = dosApp.apps;

                    if (dosApp.apps.size() > 0)
                    {
                        Node* victimNode = NULL;
                        int victimNodeId = dosApp.apps.at(0).victimNodeId;

                        BOOL success = FALSE;
                        success = PARTITION_ReturnNodePointer(
                                                partition,
                                                &victimNode,
                                                victimNodeId,
                                                TRUE);

                        if (success == TRUE)
                        {
                            //initialize at remote node by sending a message
                            Message* dosFinalizeMsg;
                            dosFinalizeMsg = MESSAGE_Alloc(victimNode,
                                    APP_LAYER,
                                    APP_DOS_VICTIM,
                                    MSG_DOS_StopVictim);

                            MESSAGE_InfoAlloc(
                                victimNode,
                                dosFinalizeMsg,
                                sizeof(AppDOSStopInfo));
                            AppDOSStopInfo* dosInfo =
                                    (AppDOSStopInfo*)
                                    MESSAGE_ReturnInfo(dosFinalizeMsg);
                            memset(dosInfo, 0, sizeof(AppDOSStopInfo));
                            strcpy(dosInfo->hid, hitlID.c_str());

                            dosFinalizeMsg->originatingNodeId =
                                                        victimNodeId;
                            EXTERNAL_MESSAGE_SendAnyNode(partition,
                                    victimNode,
                                    dosFinalizeMsg,
                                    0,
                                    EXTERNAL_SCHEDULE_SAFE);
                        }
                        else
                        {
                            char errorStr[MAX_STRING_LENGTH];
                            sprintf(errorStr,
                                "[STOP DOS] Cannot configure on node %d: "
                                    "this node id does not exist",
                                    victimNodeId);
                            ERROR_ReportWarning(errorStr);

                            GUI_CreateHITLResponse(
                                hitlID, GUI_HITL_ERROR, std::string(errorStr));
                        }

                        return;
                    }
                    else
                    {
                        std::string errorStr =
                            "[STOP DOS] DOS application has no HITL info";
                        ERROR_ReportWarning(errorStr.c_str());
                        GUI_CreateHITLResponse(hitlID, GUI_HITL_ERROR, errorStr);
                        return;
                    }
                }
            }
            else
            {
                std::string errorStr =
                    "[STOP DOS] Passed HID does not"
                    " signify any active Dos attack";
                ERROR_ReportWarning(errorStr.c_str());
                GUI_CreateHITLResponse(hitlID, GUI_HITL_ERROR, errorStr);
            }
            return;
        }

        char victimStr[MAX_STRING_LENGTH];
        NodeAddress victimNodeId;
        Node *victimNode;
        Address victimNodeAddr;
        sscanf(localArgs, "%*s %*s %s", victimStr);
        IO_AppParseSourceString(
                partition->firstNode,
                localArgs,
                victimStr,
                &victimNodeId,
                &victimNodeAddr);

        BOOL success = FALSE;
        success = PARTITION_ReturnNodePointer(partition,
                &victimNode, victimNodeId, TRUE);
        //get node pointer

        if (success == TRUE)
        {
            //initialize at remote node by sending a message
            Message* dosFinalizeMsg;
            dosFinalizeMsg = MESSAGE_Alloc(victimNode,
                    APP_LAYER,
                    APP_DOS_VICTIM,
                    MSG_DOS_StopVictim);

            dosFinalizeMsg->originatingNodeId = victimNodeId;
            EXTERNAL_MESSAGE_SendAnyNode(partition,
                    victimNode,
                    dosFinalizeMsg,
                    0,
                    EXTERNAL_SCHEDULE_SAFE);
        }
        else
        {
            char errorStr[MAX_STRING_LENGTH];
            sprintf(errorStr, "Cannot configure DOS on node %d: "
                    "this node id does not exist",
                    victimNodeId);
            ERROR_ReportWarning(errorStr);
        }
        return;
    }
    if (!strncmp(localArgs, "CHANGE DOS RATE", strlen("CHANGE DOS RATE")))
    {
        std::string cyberInput (localArgs);
        std::size_t indexOfID = std::string::npos;
        std::string hitlID = GUI_GetHID(cyberInput, &indexOfID);

        if (indexOfID == std::string::npos)
        {
            std::string errorStr =
                "[CHANGE DOS RATE]"
                " Usage: CHANGE DOS RATE <node-id> / <host-name> "
                " / <interface-address> <new-rate> HID <hid>";

            ERROR_ReportWarning(errorStr.c_str());

            return;
        }

        char newRateStr[MAX_STRING_LENGTH];

        double newRate = 0;

        if (sscanf(localArgs, "%*s %*s %*s %*s %s", newRateStr) != 1)
        {
            std::string errorStr =
                "[CHANGE DOS RATE]"
                " Usage: CHANGE DOS RATE <node-id> / <host-name> "
                " / <interface-address> <new-rate> HID <hid>";
            ERROR_ReportWarning(errorStr.c_str());
            GUI_CreateHITLResponse(hitlID, GUI_HITL_ERROR, errorStr);
            return;
        }

        newRate = strtod((const char*) newRateStr, NULL);

        if (newRate <= 0)
        {
            std::string errorStr =
                "[CHANGE DOS RATE]"
                " RATE cannot be less than or equal to 0";
            ERROR_ReportWarning(errorStr.c_str());
            GUI_CreateHITLResponse(hitlID, GUI_HITL_ERROR, errorStr);
            return;
        }

            std::map<std::string, HITLApplication>::iterator it;
            it = partition->hitlApplicationMap.find(hitlID);

            if (it != partition->hitlApplicationMap.end())
            {
                HITLApplication dosApp = it->second;

                if (dosApp.appType != APP_DOS_ATTACKER)
                {
                    std::string errorStr =
                        "[CHANGE DOS RATE] Passed HID does not"
                        " signify any active DOS attack";
                    ERROR_ReportWarning(errorStr.c_str());
                GUI_CreateHITLResponse(hitlID, GUI_HITL_ERROR, errorStr);
                }
                else
                {
                    std::vector<HITLInfo> attackersInfo = dosApp.apps;

                    for (size_t i = 0; i < attackersInfo.size(); i++)
                    {
                        Node* attackerNode =
                            ResolveNodeId(partition,
                                          attackersInfo.at(i).nodeId);

                        if (attackerNode)
                        {
                            Message* rateChangeMsg =
                                MESSAGE_Alloc(attackerNode,
                                              APP_LAYER,
                                              APP_DOS_ATTACKER,
                                              MSG_DOS_ChangeRate);

                            MESSAGE_AddInfo(attackerNode,
                                            rateChangeMsg,
                                            sizeof(AppDOSRateChangeInfo),
                                            INFO_TYPE_DOS_CHANGE_RATE);

                            AppDOSRateChangeInfo* info =
                                (AppDOSRateChangeInfo*)
                                MESSAGE_ReturnInfo(
                                    rateChangeMsg, INFO_TYPE_DOS_CHANGE_RATE);

                            int length = hitlID.length();
                            strncpy(info->hid, hitlID.c_str(), length);
                            info->hid[length] = '\0';


                            // Extracting the default address for the victimNodeId
                            // to be sent in the AppDOSRateChangeInfo
                            Node* victimNodePtr;
                            PARTITION_ReturnNodePointer(partition,
                                                &victimNodePtr,
                                                attackersInfo.at(i).victimNodeId,
                                                true);
                            NodeAddress addr;
                            if (victimNodePtr)
                            {
                                addr =
                                    MAPPING_GetDefaultInterfaceAddressFromNodeId(
                                                victimNodePtr,
                                                attackersInfo.at(i).victimNodeId);
                            }
                            else
                            {
                                return;
                            }
 
                            info->victimNode = addr;
                            info->newRate = newRate;

                            MESSAGE_RemoteSend(partition->firstNode,
                                attackerNode->nodeId,
                                rateChangeMsg,
                                0);
                        } // if
                    } // for
                } // else
            } // if
            else
            {
                std::string errorStr =
                    "[CHANGE DOS RATE] Passed HID does not"
                    " signify any active DOS attack";
                ERROR_ReportWarning(errorStr.c_str());
            GUI_CreateHITLResponse(hitlID, GUI_HITL_ERROR, errorStr);
            } // else

        return;
    }
    if (!strncmp(localArgs, "JAMMER ", strlen("JAMMER ")))
    {
        std::string cyberInput (localArgs);
        hitlApp.appType = APP_JAMMER;
        std::size_t indexOfID = std::string::npos;
        std::string hitlID = GUI_GetHID(cyberInput, &indexOfID);

        APP_InitializeCyberApplication(
            firstNode, cyberInput, TRUE, hitlApp.apps, hitlID);

        if (indexOfID != std::string::npos)
        {
            if (hitlApp.apps.size() > 0)
            {
                if (partition->hitlApplicationMap.
                        insert(
                        std::pair<std::string, HITLApplication>
                        (hitlID, hitlApp)).second == false)
                {
                    ERROR_ReportError("Insertion failed in HITLApplication map.");
                }
            }
            else
            {
                GUI_CreateHITLResponse(hitlID, GUI_HITL_ERROR);
            }
        }

        return;
    }

    if (!strncmp(localArgs, "STOP JAMMER", strlen("STOP JAMMER")))
    {
        std::string cyberInput (localArgs);
        std::size_t indexOfID = std::string::npos;
        std::string hitlID = GUI_GetHID(cyberInput, &indexOfID);

        if (indexOfID != std::string::npos)
        {
            std::map<std::string, HITLApplication>::iterator it;
            it = partition->hitlApplicationMap.find(hitlID);

            if (it != partition->hitlApplicationMap.end())
            {
                HITLApplication jammerApp = it->second;

                if (jammerApp.appType != APP_JAMMER)
                {
                    std::string errorStr =
                        "[STOP JAMMER] Passed HID does not"
                        " signify any active Jammer attack";
                    ERROR_ReportWarning(errorStr.c_str());
                    GUI_CreateHITLResponse(
                        hitlID, GUI_HITL_ERROR, errorStr);
                    return;
                }
                else
                {
                    std::vector<HITLInfo> attackersInfo = jammerApp.apps;

                    if (attackersInfo.size() > 0)
                    {
                        Node* attackerNode =
                            ResolveNodeId(partition,
                                          attackersInfo.at(0).nodeId);

                        if (attackerNode)
                        {
                            AppJammerStopJammingInstance(
                                attackerNode,
                                hitlID);
                            return;
                        }
                    }
                }
            }
            else
            {
                std::string errorStr =
                    "[STOP JAMMER] Passed HID does not"
                    " signify any active Jammer attack";
                ERROR_ReportWarning(errorStr.c_str());
                GUI_CreateHITLResponse(hitlID, GUI_HITL_ERROR, errorStr);
            }
            return;
        }

        char nodeIdHostName[MAX_STRING_LENGTH];
        Node* node = NULL;
        bool success = false;

        if (sscanf(localArgs, "%*s %*s %s", nodeIdHostName) != 1)
        {
            printf("[STOP JAMMER] Usage: stop jammer"
                   " <node-id> / <host-name> / <interface-address>\n");
            return;
        }

        // Try resolving node considering <host-name> is mentioned in
        // the HITL command
        node = ResolveHostName(partition, nodeIdHostName, &success);

        // If node pointer is not resolved, try doing the same considering
        // node id is mentioned in the HITL command
        if (!node)
        {
            success = PARTITION_ReturnNodePointer(partition,
                                                  &node,
                                                  atoi(nodeIdHostName),
                                                  TRUE) != FALSE; // warning suppression
        }

        if (!success || !node)
        {
            // Try resolving node considering interface address
            // is mentioned in HITL command
            NodeAddress jammingNodeId;
            NodeAddress jammingInterface;

            IO_AppParseSourceString(
                partition->firstNode,
                localArgs,
                nodeIdHostName,
                &jammingNodeId,
                &jammingInterface);

            success = PARTITION_ReturnNodePointer(partition,
                                                  &node,
                                                  jammingNodeId,
                                                  TRUE) != FALSE;

            if (!success)
            {
                printf("[STOP JAMMER] Cannot locate the node"
                        " with interface address = %s",
                        nodeIdHostName);
            }
            else
            {
                int inerfaceNum =
                    MAPPING_GetInterfaceIndexFromInterfaceAddress(
                        node,
                        jammingInterface);

                AppJammerStopJammingAtInterface(node, inerfaceNum);
            }
            return;
        }
        AppJammerStopAllJamming(node);
        return;
    }
    if (!strncmp(localArgs,
                 "CHANGE JAMMER POWER",
                 strlen("CHANGE JAMMER POWER")))
    {
        char power[MAX_STRING_LENGTH];

        std::string cyberInput (localArgs);
        std::size_t indexOfID = std::string::npos;
        std::string hitlID = GUI_GetHID(cyberInput, &indexOfID);

        if (indexOfID != std::string::npos)
        {
            std::map<std::string, HITLApplication>::iterator it;
            it = partition->hitlApplicationMap.find(hitlID);

            if (it != partition->hitlApplicationMap.end())
            {
                HITLApplication jammerApp = it->second;

                if (jammerApp.appType != APP_JAMMER)
                {
                    std::string errorStr =
                        "[CHANGE JAMMER POWER] Passed HID does not"
                        " signify any active Jammer attack";
                    ERROR_ReportWarning(errorStr.c_str());
                    GUI_CreateHITLResponse(
                        hitlID, GUI_HITL_ERROR, errorStr);
                }
                else
                {
                    std::vector<HITLInfo> attackersInfo = jammerApp.apps;

                    if (attackersInfo.size() > 0)
                    {
                        Node* attackerNode =
                            ResolveNodeId(partition,
                                          attackersInfo.at(0).nodeId);

                        if (attackerNode)
                        {
                            if (sscanf(localArgs,
                                       "%*s %*s %*s %*s %s",
                                       power)
                                != 1)
                            {
                                std::string errorStr =
                                    "[CHANGE JAMMER POWER] Usage:"
                                       " change jammer power <node-id> /"
                                    " <host-name> / <interface-address> <power>\n";
                                ERROR_ReportWarning(errorStr.c_str());
                                GUI_CreateHITLResponse(
                                    hitlID, GUI_HITL_ERROR, errorStr);
                                return;
                            }
                            else
                            {
                                AppJammerChangeJammingPower(
                                    attackerNode,
                                    hitlID,
                                    strtod((const char*) power, NULL));
                                return;
                            }
                        } // if
                    } // if
                } // else
            } // if
            else
            {
                std::string errorStr =
                    "[CHANGE JAMMER POWER] Passed HID does not"
                    " signify any active Jammer attack";
                ERROR_ReportWarning(errorStr.c_str());
                GUI_CreateHITLResponse(hitlID, GUI_HITL_ERROR, errorStr);
            }
        } // if

        else
        {
            char nodeIdHostName[MAX_STRING_LENGTH];
            Node* node = NULL;
            bool success = false;
            NodeAddress jammingNodeId;
            NodeAddress jammingInterface;

            if (sscanf(localArgs,
                       "%*s %*s %*s %s %s",
                       nodeIdHostName,
                       power)
                != 2)
            {
                printf("[CHANGE JAMMER POWER] Usage: change jammer power"
                       " <node-id> / <host-name> / <interface-address>"
                       " <power>\n");
                return;
            }

            // Try resolving node considering <host-name> is mentioned in
            // the command

            node = ResolveHostName(partition, nodeIdHostName, &success);

            if (node)
            {
                jammingInterface = GetDefaultIPv4InterfaceAddress(node);
            }
            else
            {
                IO_AppParseSourceString(
                    partition->firstNode,
                    localArgs,
                    nodeIdHostName,
                    &jammingNodeId,
                    &jammingInterface);

                success = PARTITION_ReturnNodePointer(partition,
                                                      &node,
                                                      jammingNodeId,
                                                      TRUE) != FALSE;
            }

            if (!success || !node)
            {
                printf("[CHANGE JAMMER] Cannot locate the node"
                       " for node id / hostname / interfaceAddress = %s",
                       nodeIdHostName);
                return;
            }

            int inerfaceNum =
                MAPPING_GetInterfaceIndexFromInterfaceAddress(
                    node,
                    jammingInterface);
            AppJammerChangeJammingPower(node,
                                        inerfaceNum,
                                        strtod((const char*) power, NULL));
            return;
        }
    }

    if (!strncmp(localArgs, "FIREWALL ", strlen("FIREWALL "))
        || !strncmp(localArgs, "firewall ", strlen("firewall ")))
    {
        int nodeId;
        sscanf(localArgs, "%*s %d", &nodeId);

        std::string tempHID = GUI_GetHID(localArgs);

        if (nodeId < 0)
        {
            char errorStr[MAX_STRING_LENGTH];
            sprintf(errorStr,
                "Invalid Node id mentioned in HITL command: %s",
                 args);
            ERROR_ReportWarning(errorStr);

            GUI_CreateHITLResponse(
                tempHID, GUI_HITL_ERROR, std::string(errorStr));
            return;
        }

        // get node pointer
        Node* node;
        BOOL success = FALSE;
        success = PARTITION_ReturnNodePointer(partition, &node, nodeId, TRUE);

        if (!success)
        {
            char errorStr[MAX_STRING_LENGTH];
            sprintf(errorStr,
                "Invalid Node id mentioned in HITL command: %s",
                args);
            ERROR_ReportWarning(errorStr);

            GUI_CreateHITLResponse(
                tempHID, GUI_HITL_ERROR, std::string(errorStr));
            return;
        }

        // get next string after nodeId
        char actionStr[MAX_STRING_LENGTH];
        sscanf(localArgs, "%*s %*s %s", actionStr);
        IO_ConvertStringToUpperCase(actionStr);

        if (!strcmp(actionStr, "OFF"))
        {
            // disabling firewall on a node
            if (partition->partitionId == node->partitionId)
            {
                if (!(node->firewallModel))
                {
                    // firewall model is NULL
                    char errorStr[MAX_STRING_LENGTH];
                    sprintf(
                        errorStr,
                        "Trying to disable unconfigured firewall"
                        " on Node %d",
                        nodeId);

                    ERROR_ReportWarning(errorStr);
                    GUI_CreateHITLResponse(
                        tempHID, GUI_HITL_ERROR, std::string(errorStr));
                }
                else if (!(node->firewallModel->isFirewallOn()))
                {
                    // firewall is already disabled
                    char errorStr[MAX_STRING_LENGTH];
                    sprintf(
                        errorStr,
                        "Trying to disable already disabled firewall"
                        " on Node %d",
                        nodeId);
                    ERROR_ReportWarning(errorStr);

                    GUI_CreateHITLResponse(
                        tempHID, GUI_HITL_ERROR, std::string(errorStr));
                }
                else
                {
                    // disabling firewall
                    node->firewallModel->turnFirewallOff();
                    GUI_CreateHITLResponse(tempHID, GUI_HITL_SUCCESS);
                }
            }
            else
            {
                Message* disableMsg = NULL;
                char* hid = NULL;

                disableMsg = 
                    MESSAGE_Alloc(node,
                        APP_LAYER,
                        APP_FIREWALL,
                        MSG_Firewall_Disable);

                disableMsg->originatingNodeId = node->nodeId;

                hid = MESSAGE_InfoAlloc(node,
                                        disableMsg,
                                        tempHID.size() + 1);

                memcpy(hid, tempHID.c_str(), tempHID.size() + 1);

                EXTERNAL_MESSAGE_RemoteSend(partition,
                    node->partitionId,
                    disableMsg,
                0,
                EXTERNAL_SCHEDULE_SAFE);
            }

        }
        else if (!strcmp(actionStr, "ON"))
        {
            // enabling firewall on a node
            if (partition->partitionId == node->partitionId)
            {
                if (!(node->firewallModel))
                {
                    // firewall model is NULL
                    char errorStr[MAX_STRING_LENGTH];
                    sprintf(
                        errorStr,
                        "No firewall rule is configured on Node %d. Enabling firewall"
                        " without any rule. Hence no packet will be filtered on the node.",
                        nodeId);

                    ERROR_ReportWarning(errorStr);
                    node->firewallModel = new FirewallModel(node);
                    node->firewallModel->configureFirewall();
                    GUI_CreateHITLResponse(tempHID, GUI_HITL_SUCCESS);
                }
                else if (node->firewallModel->isFirewallOn())
                {
                    // firewall is already enabled
                    char errorStr[MAX_STRING_LENGTH];
                    sprintf(
                        errorStr,
                        "Trying to enable already enabled firewall"
                        " on Node %d",
                        nodeId);
                    ERROR_ReportWarning(errorStr);

                    GUI_CreateHITLResponse(
                        tempHID, GUI_HITL_ERROR, std::string(errorStr));
                }
                else
                {
                    // enabling firewall
                    node->firewallModel->turnFirewallOn();
                    GUI_CreateHITLResponse(tempHID, GUI_HITL_SUCCESS);
                }
            }
            else
            {
                Message* enableMsg = NULL;
                char* hid = NULL;

                enableMsg = 
                    MESSAGE_Alloc(node,
                        APP_LAYER,
                        APP_FIREWALL,
                        MSG_Firewall_Enable);

                enableMsg->originatingNodeId = node->nodeId;

                hid = MESSAGE_InfoAlloc(node,
                                        enableMsg,
                                        tempHID.size() + 1);

                memcpy(hid, tempHID.c_str(), tempHID.size() + 1);

                EXTERNAL_MESSAGE_RemoteSend(partition,
                    node->partitionId,
                    enableMsg,
                    0,
                    EXTERNAL_SCHEDULE_SAFE);
            }

        }
        else
        {
            // configuring a new firewall on a node
            if (partition->partitionId == node->partitionId)
            {
                if (node->firewallModel == NULL)
                {
                    node->firewallModel = new FirewallModel(node);
                }
                node->firewallModel->configureFirewall();

                if (tempHID.size() > 0)
                {
                    GUI_StripHID(localArgs);
                }

                node->firewallModel->processCommand(localArgs, tempHID);
            }
            else
            {
                Message* firewallMsg = NULL;
                char* rule = NULL;

                firewallMsg =
                    MESSAGE_Alloc(node,
                        APP_LAYER,
                        APP_FIREWALL,
                        MSG_Firewall_Rule);

                firewallMsg->originatingNodeId = node->nodeId;

                rule =
                    MESSAGE_InfoAlloc(node,
                                      firewallMsg,
                                      strlen(localArgs) + 1);

                memcpy (rule, localArgs, strlen(localArgs)+ 1);

                EXTERNAL_MESSAGE_RemoteSend(partition,
                        node->partitionId,
                        firewallMsg,
                        0,
                        EXTERNAL_SCHEDULE_SAFE);
            }

        }
        return;
    }

    if (!strncmp(localArgs, "SIGINT ", strlen("SIGINT ")))
    {
        std::string tempHID = GUI_GetHID(localArgs);

        char sigintString[MAX_STRING_LENGTH];
        NodeAddress sigintNodeId;
        Node *sigintNode;
        Address sigintNodeAddr;
        sscanf(localArgs, "%*s %s", sigintString);
        IO_AppParseSourceString(
            partition->firstNode,
            localArgs,
            sigintString,
            &sigintNodeId,
            &sigintNodeAddr);

        BOOL success = FALSE;
        success = PARTITION_ReturnNodePointer(partition,
                &sigintNode, sigintNodeId, TRUE);
        //get node pointer

        if (success == TRUE)
        {
            Message *sigintMsg;
            char *sigInfo;
            sigintMsg = MESSAGE_Alloc(sigintNode,
                    APP_LAYER,
                    APP_SIGINT,
                    MSG_SIGINT_Start);
            sigintMsg->originatingNodeId = sigintNodeId;
            sigInfo =
                MESSAGE_InfoAlloc(sigintNode,
                                  sigintMsg,
                                  strlen(localArgs) + 1);
            memcpy (sigInfo, localArgs, strlen(localArgs) + 1);
            EXTERNAL_MESSAGE_SendAnyNode(
                    partition,
                    sigintNode,
                    sigintMsg,
                    0,
                    EXTERNAL_SCHEDULE_SAFE);
        }
        else
        {
            char errorStr[MAX_STRING_LENGTH];
            sprintf(errorStr, "Cannot configure SIGINT on node %d: "
                    "this node id does not exist",
                    sigintNodeId);
            ERROR_ReportWarning(errorStr);

            GUI_CreateHITLResponse(
                tempHID, GUI_HITL_ERROR, std::string(errorStr));
        }
        return;
    }

    if (!strncmp(localArgs, "STOP SIGINT", strlen("STOP SIGINT")))
    {
        std::string tempHID = GUI_GetHID(localArgs);

        char sigintString[MAX_STRING_LENGTH];
        NodeAddress sigintNodeId;
        Node *sigintNode;
        Address sigintNodeAddr;
        sscanf(localArgs, "%*s %*s %s", sigintString);
        IO_AppParseSourceString(
                partition->firstNode,
                localArgs,
                sigintString,
                &sigintNodeId,
                &sigintNodeAddr);
        BOOL success = FALSE;
        success = PARTITION_ReturnNodePointer(partition,
                &sigintNode, sigintNodeId, TRUE);
        //get node pointer

        if (success == TRUE)
        {
            Message *sigintMsg;

            sigintMsg = MESSAGE_Alloc(sigintNode,
                    APP_LAYER,
                    APP_SIGINT,
                    MSG_SIGINT_Stop);
            sigintMsg->originatingNodeId = sigintNodeId;
            MESSAGE_InfoAlloc(sigintNode,sigintMsg, sizeof(SigintInfo));
            SigintInfo* sigInfo =
                    (SigintInfo *) MESSAGE_ReturnInfo(sigintMsg);
            memset(sigInfo , 0, sizeof(SigintInfo));
            sigInfo->sigintAddress = sigintNodeAddr.interfaceAddr.ipv4;
            EXTERNAL_MESSAGE_SendAnyNode(
                    partition,
                    sigintNode,
                    sigintMsg,
                    0,
                    EXTERNAL_SCHEDULE_SAFE);
        }
        else
        {
            char errorStr[MAX_STRING_LENGTH];
            sprintf(errorStr, "Cannot stop SIGINT on node %d: "
                    "this node id does not exist",
                    sigintNodeId);
            ERROR_ReportWarning(errorStr);

            GUI_CreateHITLResponse(
                tempHID, GUI_HITL_ERROR, std::string(errorStr));
        }
        return;
    }

    if (!strncmp(localArgs, "EAVES ", strlen("EAVES ")))
    {
        std::string tempHID = GUI_GetHID(localArgs);

        NodeAddress eavesdropper = 0;
        Node *eavesNode;
        Address eavesNodeAddr;
        BOOL success = FALSE;
        char eavesString[MAX_STRING_LENGTH];
        char param1[MAX_STRING_LENGTH];
        char param2[MAX_STRING_LENGTH];
        BOOL doHla = FALSE;
        BOOL doSwitchHeader = FALSE;
        param1[0] = '\0';
        param2[0] = '\0';

        Int32 numValues =
            sscanf(localArgs, "%*s %s %s %s", eavesString, param1, param2);

        if (numValues < 1)
        {
            char errorStr[MAX_STRING_LENGTH];

            sprintf(errorStr,
                    "Incorrect usage: eaves "
                    "<eavesdropper node id / interface address>"
                    "[switchheader] [hla]\n");
            ERROR_ReportWarning(errorStr);

            GUI_CreateHITLResponse(
                tempHID, GUI_HITL_ERROR, std::string(errorStr));
            return;
        }
        if (!strcmp(param1, "SWITCHHEADER")
            || (!strcmp(param2, "SWITCHHEADER")))
        {
            doSwitchHeader = TRUE;
        }
        if (!strcmp(param1, "HLA") || (!strcmp(param2, "HLA")))
        {
            doHla = TRUE;
        }

        eavesNodeAddr.interfaceAddr.ipv4 = INVALID_ADDRESS;

        if (IO_IsStringNonNegativeInteger(eavesString))
        {
            eavesdropper = strtol(eavesString, 0, 10);
        }
        else
        {
            //the interface address is specified, so get both node id and interface address
            IO_AppParseSourceString(
                    partition->firstNode,
                    localArgs,
                    eavesString,
                    &eavesdropper,
                    &eavesNodeAddr);
        }

        //node = MAPPING_GetNodePtrFromHash(iface->partition->nodeIdHash, eavesdropper);
        success = PARTITION_ReturnNodePointer(partition,
                &eavesNode, eavesdropper, TRUE);

        if (!success)
        {
            char errorStr[MAX_STRING_LENGTH];

            sprintf(errorStr,
                    "Invalid attacker node id = %d\n",
                    eavesdropper);
            ERROR_ReportWarning(errorStr);

            GUI_CreateHITLResponse(
                tempHID, GUI_HITL_ERROR, std::string(errorStr));

            return;
        }
        else
        {
            //this is a node id, so enable eavesdrop on all interfaces
            Message *eavesMsg;

            eavesMsg = MESSAGE_Alloc(eavesNode,
                    APP_LAYER,
                    APP_EAVESDROP,
                    MSG_EAVES_Start);
            eavesMsg->originatingNodeId = eavesdropper;
            MESSAGE_InfoAlloc(eavesNode, eavesMsg, sizeof(EavesInfo));
            EavesInfo* eInfo =
                    (EavesInfo *) MESSAGE_ReturnInfo(eavesMsg);
            memset(eInfo , 0, sizeof(EavesInfo));
            eInfo->eavesAddress = eavesNodeAddr.interfaceAddr.ipv4;
            eInfo->switchHeader = doSwitchHeader;
            eInfo->hla = doHla;
            strcpy(eInfo->hid, tempHID.c_str());

            EXTERNAL_MESSAGE_SendAnyNode(
                    partition,
                    eavesNode,
                    eavesMsg,
                    0,
                    EXTERNAL_SCHEDULE_SAFE);
        }

        //node->eavesdrop = true;
        return;
    }
    if (!strncmp(localArgs, "STOP EAVES", strlen("STOP EAVES")))
    {
        std::string tempHID = GUI_GetHID(localArgs);

        char eavesString[MAX_STRING_LENGTH];
        NodeAddress eavesNodeId;
        Node *eavesNode;
        Address eavesNodeAddr;
        if (sscanf(localArgs, "%*s %*s %s", eavesString) != 1)
        {
            char errorStr[MAX_STRING_LENGTH];
            sprintf(errorStr,
                    "Incorrect usage: eaves "
                    "<eavesdropper node id/Interface Address>\n");

            ERROR_ReportWarning(errorStr);

            GUI_CreateHITLResponse(
                tempHID, GUI_HITL_ERROR, std::string(errorStr));

            return;
        }
        eavesNodeAddr.interfaceAddr.ipv4 = INVALID_ADDRESS;
        //BOOL isNodeId = FALSE;
        if (IO_IsStringNonNegativeInteger(eavesString))
        {
            //isNodeId = TRUE;
            eavesNodeId = atoi(eavesString);
        }
        else
        {
            //the interface address is specified, so get both node id and interface address
            IO_AppParseSourceString(
                    partition->firstNode,
                    localArgs,
                    eavesString,
                    &eavesNodeId,
                    &eavesNodeAddr);
        }

        BOOL success = FALSE;
        success = PARTITION_ReturnNodePointer(partition,
                &eavesNode, eavesNodeId, TRUE);
        //get node pointer
        if (!success)
        {
            char errorStr[MAX_STRING_LENGTH];

            sprintf(errorStr,
                "Invalid attacker node id = %d\n",
                eavesNodeId);

            ERROR_ReportWarning(errorStr);

            GUI_CreateHITLResponse(
                tempHID, GUI_HITL_ERROR, std::string(errorStr));

            return;
        }

        else
        {
            Message *eavesMsg;
            eavesMsg = MESSAGE_Alloc(eavesNode,
                    APP_LAYER,
                    APP_EAVESDROP,
                    MSG_EAVES_Stop);
            eavesMsg->originatingNodeId = eavesNodeId;
            MESSAGE_InfoAlloc(eavesNode,eavesMsg, sizeof(EavesInfo));
            EavesInfo* eInfo =
                    (EavesInfo *) MESSAGE_ReturnInfo(eavesMsg);
            memset(eInfo , 0, sizeof(EavesInfo));
            eInfo->eavesAddress = eavesNodeAddr.interfaceAddr.ipv4;
            eInfo->switchHeader = FALSE;
            eInfo->hla = FALSE;
            strcpy(eInfo->hid, tempHID.c_str());

            EXTERNAL_MESSAGE_SendAnyNode(
                    partition,
                    eavesNode,
                    eavesMsg,
                    0,
                    EXTERNAL_SCHEDULE_SAFE);
        }
        return;
    }
    if (!strncmp(localArgs, "PORTSCAN ", strlen("PORTSCAN ")))
    {
        std::string tempHID = GUI_GetHID(localArgs);

        char scannerInitString[MAX_STRING_LENGTH];
        NodeAddress scannerNodeId;
        Node* scannerNode = NULL;
        Address scannerNodeAddr;
        sscanf(localArgs, "%*s %s", scannerInitString);

        IO_AppParseSourceString(
            partition->firstNode,
            localArgs,
            scannerInitString,
            &scannerNodeId,
            &scannerNodeAddr);

        BOOL success = FALSE;

        success =
            PARTITION_ReturnNodePointer(
                partition,
                &scannerNode,
                scannerNodeId,
                TRUE);

        if (success == TRUE)
        {
            Message* portscanMsg;
            AppPortScannerInitInfo* initInfo = NULL;

            portscanMsg =
                MESSAGE_Alloc(
                    scannerNode,
                    APP_LAYER,
                    APP_PORTSCANNER,
                    APP_PORTSCANNER_INIT_PORT_SCANNING);

            portscanMsg->originatingNodeId = scannerNodeId;

            initInfo =
                (AppPortScannerInitInfo*)
                    MESSAGE_InfoAlloc(scannerNode,
                                      portscanMsg,
                                      sizeof(AppPortScannerInitInfo));

            strncpy(initInfo->initString, localArgs, sizeof(initInfo->initString));
            initInfo->initString[sizeof(initInfo->initString) - 1] = '\0';
            initInfo->fromGUI = TRUE;

            EXTERNAL_MESSAGE_SendAnyNode(
                partition,
                scannerNode,
                portscanMsg,
                0,
                EXTERNAL_SCHEDULE_SAFE);
        }
        else
        {
            char errorStr[MAX_STRING_LENGTH];
            sprintf(errorStr, "Cannot configure PORTSCAN on node %d: "
                    "this node id does not exist in the current scenario.",
                    scannerNodeId);
            ERROR_ReportWarning(errorStr);

            GUI_CreateHITLResponse(
                tempHID, GUI_HITL_ERROR, std::string(errorStr));
        }

        return;
    }

    if (!strncmp(localArgs, "NETSCAN ", strlen("NETSCAN ")))
    {
        std::string tempHID = GUI_GetHID(localArgs);

        char scannerInitString[MAX_STRING_LENGTH];
        NodeAddress scannerNodeId;
        Node* scannerNode = NULL;
        Address scannerNodeAddr;
        sscanf(localArgs, "%*s %s", scannerInitString);

        IO_AppParseSourceString(
            partition->firstNode,
            localArgs,
            scannerInitString,
            &scannerNodeId,
            &scannerNodeAddr);

        BOOL success = FALSE;

        success =
            PARTITION_ReturnNodePointer(
                partition,
                &scannerNode,
                scannerNodeId,
                TRUE);

        if (success == TRUE)
        {
            Message* netscanMsg;
            AppPortScannerInitInfo* initInfo = NULL;

            netscanMsg =
                MESSAGE_Alloc(
                    scannerNode,
                    APP_LAYER,
                    APP_PORTSCANNER,
                    APP_PORTSCANNER_INIT_NETWORK_SCANNING);

            netscanMsg->originatingNodeId = scannerNodeId;

            initInfo =
                (AppPortScannerInitInfo*)
                    MESSAGE_InfoAlloc(scannerNode,
                                      netscanMsg,
                                      sizeof(AppPortScannerInitInfo));

            strncpy(initInfo->initString, localArgs, sizeof(initInfo->initString));
            initInfo->initString[sizeof(initInfo->initString) - 1] = '\0';
            initInfo->fromGUI = TRUE;

            EXTERNAL_MESSAGE_SendAnyNode(
                partition,
                scannerNode,
                netscanMsg,
                0,
                EXTERNAL_SCHEDULE_SAFE);
        }
        else
        {
            char errorStr[MAX_STRING_LENGTH];
            sprintf(errorStr, "Cannot configure NETSCAN on node %d: "
                    "this node id does not exist in the current scenario.",
                    scannerNodeId);
            ERROR_ReportWarning(errorStr);

            GUI_CreateHITLResponse(
                tempHID, GUI_HITL_ERROR, std::string(errorStr));
        }

        return;
    }

    if (!strncmp(localArgs, "STOP PORTSCAN", strlen("STOP PORTSCAN")))
    {
        std::string tempHID = GUI_GetHID(localArgs);

        Node* node;
        bool success = false;
        char sourceString[MAX_STRING_LENGTH];
        NodeAddress sourceNodeId;
        Address sourceAddr;

        if (sscanf(localArgs, "%*s %*s %s", sourceString) != 1)
        {
            std::string errorStr =
                "[STOP PORTSCAN] Usage: stop portscan <src>";
            ERROR_ReportWarning(errorStr.c_str());

            GUI_CreateHITLResponse(tempHID, GUI_HITL_ERROR, errorStr);
            return;
        }

        IO_AppParseSourceString(
            partition->firstNode,
            localArgs,
            sourceString,
            &sourceNodeId,
            &sourceAddr);

        success = PARTITION_ReturnNodePointer(
                       partition->firstNode->partitionData,
                       &node,
                       sourceNodeId,
                       TRUE) != FALSE;

        if (!success)
        {
            char errorString[MAX_STRING_LENGTH];
            sprintf(errorString,
                    "[STOP PORTSCAN] Cannot locate node for %s",
                    sourceString);
            ERROR_ReportWarning(errorString);

            GUI_CreateHITLResponse(
                tempHID, GUI_HITL_ERROR, std::string(errorString));
            return;
        }

        AppPortScannerStopPortScanning(node);
        return;
    }

    if (!strncmp(localArgs,
                 "STOP NETSCAN",
                 strlen("STOP NETSCAN")))
    {
        std::string tempHID = GUI_GetHID(localArgs);

        Node* node;
        bool success = false;
        char sourceString[MAX_STRING_LENGTH];
        NodeAddress sourceNodeId;
        Address sourceAddr;

        if (sscanf(localArgs, "%*s %*s %s", sourceString) != 1)
        {
            std::string error("[STOP NETSCAN] Usage: stop netscan <src>");
            ERROR_ReportWarning(error.c_str());

            GUI_CreateHITLResponse(tempHID, GUI_HITL_ERROR, error);
            return;
        }

        IO_AppParseSourceString(
            partition->firstNode,
            localArgs,
            sourceString,
            &sourceNodeId,
            &sourceAddr);

        success = PARTITION_ReturnNodePointer(
                       partition->firstNode->partitionData,
                       &node,
                       sourceNodeId,
                       TRUE) != FALSE;

        if (!success)
        {
            char errorString[MAX_STRING_LENGTH];
            sprintf(errorString,
                    "[STOP NETSCAN] Cannot locate node for %s",
                    sourceString);
            ERROR_ReportWarning(errorString);

            GUI_CreateHITLResponse(
                tempHID, GUI_HITL_ERROR, std::string(errorString));

            return;
        }

        AppPortScannerStopNetworkScanning(node);
        return;
    }

    // Processing for all the attacks which may have HID
    // in their commands has been done above - the code written
    // below is only for the attacks for which HID is insignificant
    // as per current implementation; hence, stripping off HID.

    GUI_StripHID(localArgs);

    if (!strncmp(localArgs, "ATTACK ", strlen("ATTACK ")))
    {
        int attackerNodeId;
        Node* node = NULL;
        bool success = false;
        NodeAddress victimAddress;
        char victimAddrStr[MAX_STRING_LENGTH];

        if (sscanf(localArgs, "%*s %d %s", &attackerNodeId, victimAddrStr) != 2)
        {
            printf("Invalid syntax of attack command. "
                    "Expected: attack attackerId victimIPAddress\n");
            return;
        }
        victimAddress = inet_addr(victimAddrStr);
        victimAddress = ntohl(victimAddress);

        success = PARTITION_ReturnNodePointer(partition,
                                              &node,
                                              attackerNodeId,
                                              TRUE) != FALSE;

        if (!success)
        {
            printf("Invalid attacker node id = %d\n", attackerNodeId);
            return;
        }

        if (victimAddress == INVALID_ADDRESS)
        {
            printf("Invalid victim node: %s\n", victimAddrStr);
            return;
        }

        // initialize at remote node by sending a message
        Message* initiateAttackMsg;
        initiateAttackMsg = MESSAGE_Alloc(node,
                NETWORK_LAYER,
                NETWORK_PROTOCOL_ATTACK,
                MSG_ATTACK_InitiateAttack);

        MESSAGE_InfoAlloc(node, initiateAttackMsg, sizeof(NodeAddress));
        NodeAddress* header = (NodeAddress*)MESSAGE_ReturnInfo(initiateAttackMsg);
        *header = victimAddress;

        EXTERNAL_MESSAGE_SendAnyNode(partition,
                node,
                initiateAttackMsg,
                0,
                EXTERNAL_SCHEDULE_SAFE);

        return;
    }


#endif // CYBER_LIB

#ifdef PAS_INTERFACE
    //parsing GuiCommand
    char value1[MAX_STRING_LENGTH];
    char value2[MAX_STRING_LENGTH];
    char value3[MAX_STRING_LENGTH];
    sscanf(localArgs,
           "%s %s %s %s",
           guiCommand,
           value1,
           value2,
           value3);

    if ((!strncmp(guiCommand,
                  "EXATA-EXTERNAL-NODE",
                  strlen("EXATA-EXTERNAL-NODE"))))
    {
        AutoIPNE_SetVirtualNode(partition,
                                value1,
                                value2,
                                value3);
    }
    else if ((!strncmp(guiCommand,
                       "PACKET-SNIFFER-NODE",
                       strlen("PACKET-SNIFFER-NODE"))))
    {
        PSI_EnablePSI(partition, value1);
    }
    else if ((!strncmp(guiCommand,
                       "PACKET-SNIFFER-DOT11",
                       strlen("PACKET-SNIFFER-DOT11"))))
    {
        if ((!strncmp(value1, "YES", strlen("YES"))))
        {
            PSI_EnableDot11(partition, TRUE);
        }
        else if ((!strncmp(value1, "NO", strlen("NO"))))
        {
            PSI_EnableDot11(partition, FALSE);
        }
    }
    else if ((!strncmp(guiCommand,
                       "PACKET-SNIFFER-ENABLE-APP",
                       strlen("PACKET-SNIFFER-ENABLE-APP"))))
    {
        if ((!strncmp(value1, "YES", strlen("YES"))))
        {
            PSI_EnableApp(partition, TRUE);
        }
        else if ((!strncmp(value1, "NO", strlen("NO"))))
        {
            PSI_EnableApp(partition, FALSE);
        }
    }
    else if ((!strncmp(guiCommand,
                       "PACKET-SNIFFER-ENABLE-UDP",
                       strlen("PACKET-SNIFFER-ENABLE-UDP"))))
    {
        if ((!strncmp(value1, "YES", strlen("YES"))))
        {
            PSI_EnableUdp(partition, TRUE);
        }
        else if ((!strncmp(value1, "NO", strlen("NO"))))
        {
            PSI_EnableUdp(partition, FALSE);
        }
    }
    else if ((!strncmp(guiCommand,
                       "PACKET-SNIFFER-ENABLE-TCP",
                       strlen("PACKET-SNIFFER-ENABLE-TCP"))))
    {
        if ((!strncmp(value1, "YES", strlen("YES"))))
        {
            PSI_EnableTcp(partition, TRUE);
        }
        else if ((!strncmp(value1, "NO", strlen("NO"))))
        {
            PSI_EnableTcp(partition, FALSE);
        }
    }
    else if ((!strncmp(guiCommand,
                       "PACKET-SNIFFER-ENABLE-ROUTING",
                       strlen("PACKET-SNIFFER-ENABLE-ROUTING"))))
    {
        if ((!strncmp(value1, "YES", strlen("YES"))))
        {
            PSI_EnableRouting(partition, TRUE);
        }
        else if ((!strncmp(value1, "NO", strlen("NO"))))
        {
            PSI_EnableRouting(partition, FALSE);
        }
    }
    else if ((!strncmp(guiCommand,
                       "PACKET-SNIFFER-ENABLE-MAC",
                       strlen("PACKET-SNIFFER-ENABLE-MAC"))))
    {
        if ((!strncmp(value1, "YES", strlen("YES"))))
        {
            PSI_EnableMac(partition, TRUE);
        }
        else if ((!strncmp(value1, "NO", strlen("NO"))))
        {
            PSI_EnableMac(partition, FALSE);
        }
    }
    else
    {
#else
        sscanf(localArgs, "%s", guiCommand);
#endif // PAS_INTERFACE
        if (strlen(guiCommand) == 1)
        {
            // Advance to the first non-whitespace character
            ch = (char*) &localArgs[2];
            while (*ch != 0 && isspace(*ch))
            {
                ch++;
            }
        }
        else
        {
            string warning = "Invalid command";
            warning += " - ";
            warning += localArgs;
            ERROR_ReportWarning(warning.c_str());
            return;
        }
        EXTERNAL_Interface *iface = NULL;
        iface = partition->interfaceTable[EXTERNAL_QUALNET_GUI];

        switch (guiCommand[0]) {
        case 'p': case 'P':
        // This command will change the precedence of
            // all Voip applications.
            // If the first non-whitespace character is a
            // valid number then set the precedence
            if ((*ch >= '0') && (*ch <= '7')) {
                //printf("Setting precedence to %c\n", *ch);
                SetCbrPrecedence(partition, ch);
            }
            break;
        case 'n': case 'N':
            // This command will change the precedence of appForward
            // If the first non-whitespace character is a
            // valid number then set the precedence
            if ((*ch >= '0') && (*ch <= '7')) {
                //printf("Setting precedence to %c\n", *ch);
                SetCbrPrecedenceAppForward(partition, ch);
            }
            break;

        case 't': case 'T':
            // This command will change the traffic of all
            // Cbr applications
            SetCbrTrafficRate(partition,  ch);
            break;

        case 'l': case 'L':
            // This command will change the traffic of all
            // Cbr applications
            SetCbrTrafficRateTimes(partition,  ch);
            break;

        case 'd': case 'D':

            DeactivateNode(iface, ch);
            break;
        case 'a': case 'A':
            ActivateNode(iface, ch);
            break;

        default:
            //printf("Unknown user command\n");
            break;
        }//SWITCH

#ifdef PAS_INTERFACE
    } // PAS
#endif
    //END HUMAN IN THE LOOP CODE
#endif // HITL
}

/*--
 *
 *  Block waiting for more commands from the gui.
 */
void GUI_EXTERNAL_ReceiveCommands (EXTERNAL_Interface *iface) {
    char        timeString[MAX_CLOCK_STRING_LENGTH];
    GuiCommand  command;
    BOOL        waitingForStepCommand;
    PartitionData * partitionData = iface->partition;
    GUI_InterfaceData *     data;
    data = (GUI_InterfaceData*) iface->data;

    if (iface->horizon > partitionData->maxSimClock)
        return;

    // Time of the next event that the simulation will next execute.
    clocktype advanceTime = GetNextInternalEventTime (partitionData);

    if (data->firstIteration)
    {
        // For the first iteration:
        // Intialization is now complete. We now make a blocking
        // call to wait for the gui to send us the first
        // GUI_STEP command. Because GUI_ReceiveCommand
        // is going to block waiting for the user to hit play we
        // are in effect in a paused state.
        data->firstIteration = false;
        data->guiPaused = true;
        partitionData->wallClock->pause ();
    }
    if (advanceTime == CLOCKTYPE_MAX) {
        // In the case of IPNE, events will eventually come
        // from the external interface, but there might
        // not be any yet. So, we are going to
        // proceed as though everything is ready right now.
        // This will "heartbeat" between the simulation
        // and the gui.
        // advanceTime = EXTERNAL_QuerySimulationTime(iface);
        advanceTime = iface->horizon;
    }

    bool hasEvents = true;
    while (hasEvents && iface->horizon <= advanceTime) {
        // The simulation has reached the gui-horizion. So we will
        // now notify the gui we've reached the horizon
        // (GUI_STEPPED==STEP_REPLY) and block waiting for the next
        // STEP-forward message from the gui.

        // Tell the Animator we've reached the simulation time
        // allowed by the last step.
        ctoa (EXTERNAL_QuerySimulationTime(iface), timeString);
        GuiReply    reply;
        reply.type = GUI_STEPPED;
        reply.args.append(timeString);
        GUI_SendReply(GUI_guiSocket, reply);

        if (DEBUG) {
            printf ("Sent a GUI_STEPPED reply to gui '%s'\n", reply.args.c_str());
            fflush(stdout);
        }

        waitingForStepCommand = TRUE;
        while (hasEvents && waitingForStepCommand) {
            command = GUI_ReceiveCommand(GUI_guiSocket);
            switch (command.type) {
                case GUI_STEP:
                {
                    data->guiExternalTime = iface->horizon;
                    //guiExternalTimeHorizon = advanceTime + guiStepSize;
                    // Instead of just opening the horizion a fixed
                    // amount we will open up out to the next internal
                    // event time.
                    // iface->horizon += data->guiStepSize;
                    clocktype nextEventTime =
                        GetNextInternalEventTime (partitionData);
                    if (advanceTime > nextEventTime)
                    {
                        // A new earlier event has been scheduled
                        // while we have been looping here in our
                        // handshake while loop, so use the updated
                        // upper limit on how long we handshake for.
                        advanceTime = nextEventTime;
                    }
                    if (nextEventTime == CLOCKTYPE_MAX)
                    {
                        // No scheduled events. This sometimes happens when
                        // the simulation is driven by an external application.
                        // Set the hasEvents flag so we will leave this function
                        // and other interfaces can check for new events.
                        hasEvents = false;
                    }

                    if (partitionData->isRealTime)
                    {
                        // In real time mode, set horizon to close to current
                        // time in order to be able to accept GUI events at
                        // any time.
                        nextEventTime = partitionData->getGlobalTime();
                    }
                    else
                    {
                        // In simulation mode, treat other interfaces'
                        // horizons as synthetic events for the purpose of
                        // determining the GUI interface's horizon.  This way,
                        // the GUI interface will be able to accept further
                        // events if other interfaces inject events before the
                        // next currently scheduled internal event.
                        for (EXTERNAL_Interface* otherIface =
                                 partitionData->interfaceList.interfaces;
                             otherIface != NULL;
                             otherIface = iface->next)
                        {
                            if (otherIface != iface &&
                                otherIface->simulationHorizonFunction != NULL &&
                                otherIface->horizon < nextEventTime)
                            {
                                nextEventTime = otherIface->horizon;
                            }
                        }
                    }

                    if (nextEventTime < CLOCKTYPE_MAX - data->guiStepSize)
                    {
                        if (nextEventTime + data->guiStepSize >
                                iface->horizon)
                        {
                            iface->horizon =
                                nextEventTime + data->guiStepSize;
                        }
                    }
                    else
                    {
                        iface->horizon = CLOCKTYPE_MAX;
                    }
                    if (DEBUG) {
                        printf("set our gui horizon to %"
                                TYPES_64BITFMT "d \n",
                               iface->horizon);
                        fflush(stdout);
                    }

                    waitingForStepCommand = FALSE;
                    if (data->guiPaused)
                    {
                        data->guiPaused = false;
                        partitionData->wallClock->resume ();
                    }
                    break;
                }
                case GUI_DISABLE_LAYER:
                    GUI_SetLayerFilter(command.args.c_str(), FALSE);
                    break;
                case GUI_ENABLE_LAYER:
                    GUI_SetLayerFilter(command.args.c_str(), TRUE);
                    break;
                case GUI_DISABLE_EVENT:
                    GUI_SetEventFilter(command.args.c_str(), FALSE);
                    break;
                case GUI_ENABLE_EVENT:
                    GUI_SetEventFilter(command.args.c_str(), TRUE);
                    break;
                case GUI_DISABLE_VISOBJ_FILTER:
                    GUI_SetVisObjFilter(command.args.c_str(),
                                        false,
                                        partitionData->getGlobalTime());
                    break;
                case GUI_ENABLE_VISOBJ_FILTER:
                    GUI_SetVisObjFilter(command.args.c_str(),
                                        true,
                                        partitionData->getGlobalTime());
                    break;
                case GUI_DISABLE_VISOBJ_DETAILS:
                    GUI_SetVisObjDetails(command.args.c_str(),
                                         false,
                                         partitionData->getGlobalTime());
                    break;
                case GUI_ENABLE_VISOBJ_DETAILS:
                    GUI_SetVisObjDetails(command.args.c_str(),
                                         true,
                                         partitionData->getGlobalTime());
                    break;
                case GUI_SET_COMM_INTERVAL:
                    data->guiStepSize =
                        TIME_ConvertToClock(command.args.c_str());
                    break;
                case GUI_ENABLE_NODE:
                    GUI_SetNodeFilter(iface->partition,
                                      command.args.c_str(),
                                      TRUE);
                    break;
                case GUI_DISABLE_NODE:
                    GUI_SetNodeFilter(iface->partition,
                                      command.args.c_str(),
                                      FALSE);
                    break;
                case GUI_SET_STAT_INTERVAL:
                    GUI_statInterval =
                        TIME_ConvertToClock(command.args.c_str());
                    // guiExternalTime hasn't been advanced yet.
                    GUI_statReportTime = data->guiExternalTime;
                    break;
                case GUI_ENABLE_METRIC:
                    GUI_SetMetricFilter(command.args.c_str(), TRUE);
                    break;
                case GUI_DISABLE_METRIC:
                    GUI_SetMetricFilter(command.args.c_str(), FALSE);
                    break;
                case GUI_STOP:
                    // Shut the simulation down asap.
                    if (strcmp(command.args.c_str(),
                               "sendFinalizationCommand") == 0)
                    {
                        g_sendFinalizationStatus = TRUE;
                    }
                    else if (strcmp(command.args.c_str(),
                             "donotSendFinalizationCommand") == 0)
                    {
                        g_sendFinalizationStatus = FALSE;
                    }
                    waitingForStepCommand = FALSE;
                    iface->horizon = CLOCKTYPE_MAX;
                    EXTERNAL_SetSimulationEndTime (iface);
                    break;
                case GUI_PAUSE:
                    if (! data->guiPaused)
                    {
                        data->guiPaused = true;
                        partitionData->wallClock->pause ();
                    }
                    break;
                case GUI_RESUME:
                    if (data->guiPaused)
                    {
                        data->guiPaused = false;
                        partitionData->wallClock->resume ();
                    }
                    break;
                case GUI_SET_ANIMATION_FILTER_FREQUENCY:
                    GUI_SetAnimationFilterFrequency(command.args.c_str());
                    break;
                case GUI_DYNAMIC_ReadAsString:
                {
                    std::string value;
                    try
                    {
                        // Skip past first whitespace
                        const char* firstChar = command.args.c_str();
                        while (*firstChar != 0 && *firstChar == ' ')
                        {
                            firstChar++;
                        }

                        // Get value from hierarchy
                        partitionData->dynamicHierarchy.ReadAsString(
                            firstChar,
                            value);

                        // Send gui command for object value
                        GuiReply reply;
                        char resultStr[GUI_MAX_COMMAND_LENGTH];

                        reply.type = GUI_DYNAMIC_COMMAND;
                        sprintf(resultStr, "%d ", GUI_DYNAMIC_ObjectValue);
                        reply.args = resultStr;
                        reply.args += firstChar;
                        reply.args += " ";
                        reply.args += value;

                        GUI_SendReply(GUI_guiSocket, reply);
                    }
                    catch (D_Exception &e)
                    {
                        e.GetFullErrorString(value);
                        ERROR_ReportError((char*) value.c_str());
                    }
                    break;
                }
                case GUI_DYNAMIC_WriteAsString:
                {
                    try
                    {
                        char path[MAX_STRING_LENGTH];
                        char value[MAX_STRING_LENGTH];

                        sscanf(command.args.c_str(), "%s %s", path, value);
                        //printf("writing %s %s\n", path, value);

                        // Get value from hierarchy
                        partitionData->dynamicHierarchy.WriteAsString(
                            path,
                            value);
                    }
                    catch (D_Exception &e)
                    {
                        std::string err;
                        e.GetFullErrorString(err);
                        ERROR_ReportError((char*) err.c_str());
                    }
                    break;
                }
                case GUI_DYNAMIC_ExecuteAsString:
                {
                    std::string value;
                    try
                    {
                        char path[MAX_STRING_LENGTH];
                        char value[MAX_STRING_LENGTH];
                        std::string output;

                        // Skip past first whitespace
                        const char* firstChar = command.args.c_str();
                        while (*firstChar != 0 && *firstChar == ' ')
                        {
                            firstChar++;
                        }

                        sscanf(firstChar, "%s %s", path, value);

                        // Get value from hierarchy
                        partitionData->dynamicHierarchy.ExecuteAsString(
                            path,
                            value,
                            output);

                        // Send gui command for object value
                        GuiReply reply;
                        char resultStr[GUI_MAX_COMMAND_LENGTH];
                        std::string replyBuff;
                        sprintf(resultStr, "%d ", GUI_DYNAMIC_ExecuteResult);
                        replyBuff = resultStr;
                        replyBuff += firstChar;
                        replyBuff += " ";
                        replyBuff += output;
                        reply = GUI_CreateReply(
                            GUI_DYNAMIC_COMMAND,
                            &replyBuff);
                        GUI_SendReply(GUI_guiSocket, reply);
                    }
                    catch (D_Exception &e)
                    {
                        e.GetFullErrorString(value);
                        ERROR_ReportError((char*) value.c_str());
                    }
                    break;
                }
                case GUI_DYNAMIC_Listen:
                {
#ifdef D_LISTENING_ENABLED
                    try
                    {
                        // Skip past first whitespace
                        const char* firstChar = command.args.c_str();
                        while (*firstChar != 0 && *firstChar == ' ')
                        {
                            firstChar++;
                        }

                        // Get value from hierarchy
                        partitionData->dynamicHierarchy.AddListener(
                            firstChar,
                            "",
                            "",
                            "GUI",
                            new GuiDynamicObjectValueCallback(iface, firstChar));
                    }
                    catch (D_Exception &e)
                    {
                        std::string value;
                        e.GetFullErrorString(value);
                        ERROR_ReportError((char*) value.c_str());
                    }
#endif // D_LISTENING_ENABLED
                    break;
                }

                case GUI_DYNAMIC_Unlisten:
                {
#ifdef D_LISTENING_ENABLED
                    try
                    {
                        // Skip past first whitespace
                        const char* firstChar = command.args.c_str();
                        while (*firstChar != 0 && *firstChar == ' ')
                        {
                            firstChar++;
                        }

                        // Get value from hierarchy
                        partitionData->dynamicHierarchy.RemoveListeners(
                            firstChar,
                            "GUI");
                    }
                    catch (D_Exception &e)
                    {
                        std::string value;
                        e.GetFullErrorString(value);
                        ERROR_ReportError((char*) value.c_str());
                    }
#endif // D_LISTENING_ENABLED
                    break;
                }
                case GUI_USER_DEFINED: {
#ifdef HITL
                    GUI_HandleHITLInput(command.args.c_str(), iface->partition);
#endif
                    break;
                }
                case GUI_UNRECOGNIZED:
                    break; // this should be an error, but what to do?
                default:
                    break;
            }//switch
        }//waiting for step
    }
}

// Called for all partitions
void GUI_EXTERNAL_Registration(EXTERNAL_Interface* iface,
    NodeInput *nodeInput)
{
    PartitionData* partitionData = iface->partition;
    GUI_InterfaceData *     data;

    data = (GUI_InterfaceData*) MEM_malloc(sizeof(GUI_InterfaceData));
    memset(data, 0, sizeof(GUI_InterfaceData));
    iface->data = data;
    iface->horizon = 0;
    data->guiExternalTime = 0;
    data->guiStepSize = GUI_DEFAULT_STEP;
    data->firstIteration = true;
    data->guiPaused = false;

    //
    // Initialize GUI, all partitons do this for setup of
    // bookeeping data structs, and a few GUI specific globals that are
    // accessed by partition_private. Note that only p0 will actually
    // send a GUI_INIT message to the gui.
    //
    GUI_Initialize (partitionData,
                    data,
                    nodeInput,
                    PARTITION_GetTerrainPtr(partitionData)->getCoordinateSystem(),
                    PARTITION_GetTerrainPtr(partitionData)->getOrigin(),
                    PARTITION_GetTerrainPtr(partitionData)->getDimensions(),
                    partitionData->maxSimClock);

    if (GUI_isAnimateOrInteractive ())
    {
#ifdef D_LISTENING_ENABLED
        // Add listener for new levels if dynamic hierarchy is being used
        if (iface->partition->dynamicHierarchy.IsEnabled())
        {
            iface->partition->dynamicHierarchy.AddObjectListener(
                new GuiDynamicObjectCallback(iface, GUI_DYNAMIC_AddObject));
            iface->partition->dynamicHierarchy.AddObjectPermissionsListener(
                new GuiDynamicObjectCallback(iface, GUI_DYNAMIC_ObjectPermissions));
            iface->partition->dynamicHierarchy.AddLinkListener(
                new GuiDynamicObjectCallback(iface, GUI_DYNAMIC_AddLink));
            iface->partition->dynamicHierarchy.AddRemoveListener(
                new GuiDynamicObjectCallback(iface, GUI_DYNAMIC_RemoveObject));
    }
#endif

        //
        // Read and create hierarchy information (only partition 0 does this)
        //
        if (partitionData->partitionId == 0)
        {
            for (int i = 0; i < nodeInput->numLines; i++)
            {
                char* currentLine = nodeInput->values[i];

                if (strcmp(nodeInput->variableNames[i], "COMPONENT") == 0)
                {
                    int componentId = atoi(currentLine);

                    GUI_CreateHierarchy(componentId,
                                        strstr(currentLine, "{"));
                }
            }
        }

#ifndef USE_MPI
        if (partitionData->partitionId != 0) {
            // We are parallel and we are shared memory, so we are
            // done performing init steps.
            // In shared memory, only partition 0 creates an external
            // interface for waiting for the gui commands, while
            // the gui socket will be shared by all partitions to send
            // commands back to the UI, and the exteranl itnerface/step
            // controls will be all handled by partition 0.
            return;
        }
#endif
        if (GUI_isConnected ())
        {
            // Our partition has a connection for the control socket
            // Register Receive function - this reads the commands sent
            // from the gui into the running simulation.
            EXTERNAL_RegisterFunction(
                iface,
                EXTERNAL_RECEIVE,
                (EXTERNAL_Function) GUI_EXTERNAL_ReceiveCommands);
            EXTERNAL_RegisterFunction(
                iface,
                EXTERNAL_TIME,
                (EXTERNAL_Function) GUI_EXTERNAL_QueryExternalTime);
            EXTERNAL_RegisterFunction(
                iface,
                EXTERNAL_SIMULATION_HORIZON,
                (EXTERNAL_Function) GUI_EXTERNAL_SimulationHorizon);
            EXTERNAL_RegisterFunction(
                iface,
                EXTERNAL_FINALIZE,
                (EXTERNAL_Function) GUI_EXTERNAL_Finalize);
        }
    }
}

void GUI_SetExternalNode(
    NodeId nodeID,
    int type,
    clocktype time)
{
    GuiReply reply;
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);
    char replyBuff[GUI_MAX_COMMAND_LENGTH];

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_SET_EXTERNAL_NODE])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %d %s",
            GUI_SET_EXTERNAL_NODE,
            nodeID,
            type,
            timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        cout << reply.args << endl;
    }

#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

void GUI_ResetExternalNode(NodeId nodeID,
    int type,
    clocktype   time)
{
    GuiReply reply;
    char     timeString[MAX_CLOCK_STRING_LENGTH];
    QNPartitionLock globalsLock(&GUI_globalsMutex);
    char replyBuff[GUI_MAX_COMMAND_LENGTH];

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_RESET_EXTERNAL_NODE])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %d %s",
            GUI_RESET_EXTERNAL_NODE,
            nodeID,
            type,
            timeString);
    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        cout << reply.args << endl;
    }

#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}


void GUI_SetExternalNodeMapping(
    NodeId nodeID,
    char* virtualAddr,
    char* physicalAddr,
    clocktype time)
{
    GuiReply reply;
    char     timeString[MAX_CLOCK_STRING_LENGTH] = {0};
    QNPartitionLock globalsLock(&GUI_globalsMutex);
    char replyBuff[GUI_MAX_COMMAND_LENGTH] = {0};

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_SET_EXTERNAL_NODE_MAPPING])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %s %s %s",
            GUI_SET_EXTERNAL_NODE_MAPPING,
            nodeID,
            virtualAddr,
            physicalAddr,
            timeString);

    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        cout << reply.args << endl;
    }

#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

void GUI_ResetExternalNodeMapping(NodeId nodeID,
    char* virtualAddr,
    char* physicalAddr,
    clocktype   time)
{
    GuiReply reply;
    char     timeString[MAX_CLOCK_STRING_LENGTH] = {0};
    QNPartitionLock globalsLock(&GUI_globalsMutex);
    char replyBuff[GUI_MAX_COMMAND_LENGTH] = {0};

    if (GetNodeIndexFromHash(nodeID) == -1) return;
    if (!g_nodeEnabledForAnimation[GetNodeIndexFromHash(nodeID)])
        return;
    if (!g_eventEnabled[GUI_RESET_EXTERNAL_NODE_MAPPING])
        return;

    ctoa(time, timeString);

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %u %s %s %s",
            GUI_RESET_EXTERNAL_NODE_MAPPING,
            nodeID,
            virtualAddr,
            physicalAddr,
            timeString);

    reply.args.append(replyBuff);

    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        cout << reply.args << endl;
    }

#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}



/*
 * NAME:        GUI_SendInterfaceActivateDeactivateStatus
 * PURPOSE:     Sends activation/deactivation status of interface
 * PARAMETERS:  NodeId       nodeID: NodeId for which activation/deactivation
 *                                   status needs to be send
 *              GuiReplies   nodeStatus: whether activation or deactivation
 *                                   of node is done
 *              Int32        interfaceIndex: index of interface which needs
 *                                   to be activated/deactivated
 *                                   (default value = -1 when the node needs
 *                                   to be activated/deactivated
 *              char*        optionalMessage: Any optional message
 *                                            (default value = NULL)
 * RETURN:      none.
 */

void GUI_SendInterfaceActivateDeactivateStatus(NodeId       nodeID,
                                               GuiEvents    nodeStatus,
                                               Int32        interfaceIndex,
                                               char*        optionalMessage)
{
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];

    reply.type = GUI_ANIMATION_COMMAND;

    // Currently nodeId which needs to be activated/decativated
    // is passed as parameter.However, for future use an optional (NULL)
    // message is also passed to GUI

    if (optionalMessage)
    {
        char checkReplyBuffOverflow[GUI_MAX_COMMAND_LENGTH];
        sprintf(checkReplyBuffOverflow, "%d %d", nodeStatus, nodeID);
        size_t remainingLength =
                    GUI_MAX_COMMAND_LENGTH - strlen(checkReplyBuffOverflow);

        if (strlen(optionalMessage) > remainingLength)
        {
            ERROR_ReportWarning("Too long optional message parameter,"
                    "activate/deactivate command not sent to GUI.");
            return;
        }
    }

    sprintf(replyBuff, "%d %d %d %s", nodeStatus, nodeID , interfaceIndex,
                                      optionalMessage);
    reply.args.append(replyBuff);
    if (GUI_isConnected())
    {
        GUI_SendReply(GUI_guiSocket, reply);
    }

#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE

#ifdef SOPSVOPS_INTERFACE
    if (SopsVopsInterfaceEnabled())
    {
        char key[MAX_STRING_LENGTH];
        sprintf(key, "/node/%d/active", nodeID);
        if (nodeStatus == GUI_ACTIVATE_INTERFACE)
        {
            SetSopsProperty(key, "yes");
        }
        else if (nodeStatus == GUI_DEACTIVATE_INTERFACE)
        {
            SetSopsProperty(key, "no");
        }
    }
#endif
}

void GuiVisObjCmd(GuiVisObjCommands cmd,
                  clocktype time,
                  const char* text,
                  const char* filterId,
                  NodeId detailsNode)
{
    QNPartitionLock lock(&GUI_globalsMutex);
    if (!g_eventEnabled[GUI_VISUALIZATION_OBJECT])
    {
        return;
    }

    if (GUI_isConnected())
    {
        bool sendCmd = (cmd == GUI_ADD_FILTER ||
            cmd == GUI_ADD_SHAPE_LEGEND ||
            cmd == GUI_ADD_LINE_LEGEND ||
            GUI_IsVisObjFilterActive(filterId));

        // Also should send it if this is a text object and the GUI
        // has the node's details dialog open.
        if (! sendCmd &&
            detailsNode != ANY_NODEID &&
            GUI_IsVisObjDetailsActive(detailsNode))
        {
            sendCmd = true;
        }

        if (cmd == GUI_DELETE_OBJECTS && ! sendCmd)
        {
            // For GUI_DELETE_OBJECTS, we should also send the command along
            // if the GUI has a subfilter of filterId active
            std::string filterPrefix = std::string(filterId) + '|';
            std::set<std::string>::iterator it =
                g_activeVisObjFilters.lower_bound(filterPrefix);
            if (it != g_activeVisObjFilters.end() &&
                it->substr(0, filterPrefix.length()) == filterPrefix)
            {
                sendCmd = true;
            }

            if (! sendCmd)
            {
                // Also should send the GUI_DELETE_OBJECTS if any object being
                // deleted is a text object on an open details dialog.
                for (std::multimap<std::string, GUI_VisObjStoreEntry>::iterator objIt =
                         g_visObjStore.lower_bound(filterId);
                     objIt != g_visObjStore.end() && (*objIt).first == filterId;
                     ++objIt)
                {
                    if ((*objIt).second.detailsNodeId != ANY_NODEID &&
                        GUI_IsVisObjDetailsActive((*objIt).second.detailsNodeId))
                    {
                        sendCmd = true;
                        break;
                    }
                }
            }
            if (! sendCmd)
            {
                for (std::multimap<std::string, GUI_VisObjStoreEntry>::iterator objIt =
                         g_visObjStore.lower_bound(filterPrefix);
                     objIt != g_visObjStore.end() &&
                         (*objIt).first.substr(0, filterPrefix.length()) == filterPrefix;
                     ++objIt)
                {
                    if ((*objIt).second.detailsNodeId != ANY_NODEID &&
                        GUI_IsVisObjDetailsActive((*objIt).second.detailsNodeId))
                    {
                        sendCmd = true;
                        break;
                    }
                }
            }
        }

        if (sendCmd)
        {
            GuiReply        reply;
            char            buf[GUI_MAX_COMMAND_LENGTH];
            char            clock[MAX_CLOCK_STRING_LENGTH];

            ctoa(time, clock);
            sprintf(buf, "%d %d %s %s", GUI_VISUALIZATION_OBJECT, cmd, clock, text);

            reply.type = GUI_ANIMATION_COMMAND;
            reply.args.append(buf);
            GUI_SendReply(GUI_guiSocket, reply);
        }

        if (cmd == GUI_DELETE_OBJECTS)
        {
            // Remove all matching objects from our store
            g_visObjStore.erase(filterId);

            // Also remove objects with filters which are a subfilter of filterId
            std::multimap<std::string, GUI_VisObjStoreEntry>::iterator stIt =
                g_visObjStore.lower_bound(std::string(filterId) + '|');
            // The strict upper bound of the range to erase (in other words, the
            // first element which shouldn't be deleted) is the first element
            // with key >= filterId + '}', where '}' is the character after '|'.
            std::multimap<std::string, GUI_VisObjStoreEntry>::iterator enIt =
                g_visObjStore.lower_bound(std::string(filterId) + char('|' + 1));
            g_visObjStore.erase(stIt, enIt);
        }
        else if (cmd != GUI_DRAW_FLOW_LINE &&
                 cmd != GUI_ADD_FILTER &&
                 cmd != GUI_ADD_SHAPE_LEGEND &&
                 cmd != GUI_ADD_LINE_LEGEND)
        {
            // Add the new object to our store.
            GUI_VisObjStoreEntry newEntry;
            newEntry.cmd = cmd;
            newEntry.text = text;
            newEntry.detailsNodeId = detailsNode;
            g_visObjStore.insert(std::make_pair(filterId, newEntry));
        }
    }
    else  // if (GUI_IsConnected())
    {
        char            clock[MAX_CLOCK_STRING_LENGTH];

        ctoa(time, clock);
        printf("%d %d %s %s\n", GUI_VISUALIZATION_OBJECT, cmd, clock, text);
    }
}

void GUI_AddFilter(
    const char* filter,
    const char* toolTip,
    bool        active,
    clocktype   time)
{
    char buffer[GUI_MAX_COMMAND_LENGTH];
    sprintf(buffer, "\"%s\" %d \"%s\"", filter, active, toolTip);
    GuiVisObjCmd(GUI_ADD_FILTER, time, buffer, filter, ANY_NODEID);
    if (active)
    {
        QNPartitionLock lock(&GUI_globalsMutex);
        g_activeVisObjFilters.insert(filter);
    }
}

void GUI_DeleteObjects(const char* id, clocktype time)
{
    char buffer[GUI_MAX_COMMAND_LENGTH];
    sprintf(buffer, "\"%s\"", id);
    GuiVisObjCmd(GUI_DELETE_OBJECTS, time, buffer, id, ANY_NODEID);
}

void GUI_DrawLine(
    NodeId          src,
    NodeId          dst,
    const char*     color,
    const char*     id,
    const char*     label,
    float           thickness,
    unsigned short  pattern,
    int             factor,
    bool            srcArrow,
    bool            dstArrow,
    clocktype       time)
{
    char buffer[GUI_MAX_COMMAND_LENGTH];
    sprintf(buffer, "\"%s\" %u %u \"%s\" %f %d %u %d %d \"%s\"", id, src,
        dst, color, thickness, factor, pattern, srcArrow, dstArrow, label);
    GuiVisObjCmd(GUI_DRAW_LINE, time, buffer, id, ANY_NODEID);
}

void GUI_DrawFlowLine(
    NodeId      src,
    NodeId      dst,
    const char* color,
    const char* id,
    const char* label,
    float       thickness,
    int         pattern,
    int         factor,
    int         ttl,
    clocktype   time)
{
    char buffer[GUI_MAX_COMMAND_LENGTH];
    sprintf(buffer, "\"%s\" %u %u \"%s\" %f %d %d %d \"%s\"", id, src, dst,
        color, thickness, factor, pattern, ttl, label);
    GuiVisObjCmd(GUI_DRAW_FLOW_LINE, time, buffer, id, ANY_NODEID);
}

void GUI_DrawShape(
    GuiVisShapes    shape,
    NodeId          node,
    double          scale,
    const char*     color,
    const char*     id,
    clocktype       time)
{
    char buffer[GUI_MAX_COMMAND_LENGTH];
    sprintf(buffer, "\"%s\" %u %u %f %s", id, node, shape, scale, color);
    GuiVisObjCmd(GUI_DRAW_SHAPE, time, buffer, id, ANY_NODEID);
}

void GUI_DrawText(
    NodeId          node,
    const char*     text,
    const char*     id,
    int             interfaceIndex,
    int             order,
    clocktype       time)
{
    char buffer[GUI_MAX_COMMAND_LENGTH];
    sprintf(buffer, "\"%s\" %u %d %d \"%s\"", id, node, interfaceIndex,
        order, text);
    GuiVisObjCmd(GUI_DRAW_TEXT, time, buffer, id, node);
}

void GUI_AddShapeLegend(
    GuiVisShapes  shape,
    const char*   color,
    const char*   legendText,
    clocktype     time)
{
    char buffer[GUI_MAX_COMMAND_LENGTH];
    sprintf(buffer, "%d \"%s\" \"%s\"", shape, color, legendText);
    GuiVisObjCmd(GUI_ADD_SHAPE_LEGEND, time, buffer, "", ANY_NODEID);
}

void GUI_AddLineLegend(
    const char*     color,
    float           thickness,
    unsigned short  pattern,
    int             factor,
    bool            srcArrow,
    bool            dstArrow,
    const char*     legendText,
    clocktype       time)
{
    char buffer[GUI_MAX_COMMAND_LENGTH];
    sprintf(buffer, "\"%s\" %f %hu %d %d %d \"%s\"",
            color, (double) thickness, pattern, factor,
            srcArrow ? 1 : 0,
            dstArrow ? 1 : 0,
            legendText);
    GuiVisObjCmd(GUI_ADD_LINE_LEGEND, time, buffer, "", ANY_NODEID);
}

string GuiGetColor(
    const string&           index,
    map<string, string>&    colors,
    set<string>&            usedColors)
{
    srand(0);
    for (int i = 0; i < 10000; ++i)
    {
        char buffer[10];
        sprintf(buffer, "#%x", ((rand() % 256) << 16)
            + ((rand() % 256) << 8) + (rand() % 256));
        string color = buffer;

        // Don't allow white
        if (color != "#ffffff" &&
            usedColors.find(color) == usedColors.end())
        {
            usedColors.insert(color);
            colors[index] = color;
            return color;
        }
    }

    return "#ff0000"; //default to red
}
void GuiSelectAppHopByHopFlowColor(const string& index)
{
    map<string, string>::iterator it = g_appHopByHopFlowColors.find(index);

    if (it == g_appHopByHopFlowColors.end())
    {
        GuiGetColor(index, g_appHopByHopFlowColors,
            g_appHopByHopFlowUsedColors);
    }
}

void GUI_AppHopByHopFlow(Node* node, Message* msg, NodeAddress previousHop)
{
    STAT_Timing* timing =
        (STAT_Timing*) MESSAGE_ReturnInfo(msg, INFO_TYPE_StatsTiming);

    if (timing)
    {
        stringstream i;
        i << timing->uniqueSessionId << '_' << timing->sourceNodeId;//msg->originatingNodeId;

        map<string, string>::iterator it =
            g_appHopByHopFlowColors.find(i.str());

        if (it != g_appHopByHopFlowColors.end())
        {
            NodeId hop =
                MAPPING_GetNodeIdFromInterfaceAddress(node, previousHop);
            string filter = "Application Flow Filters|"
                + g_appHopByHopFlowFilters[i.str()];
            clocktype time = node->getNodeTime() + getSimStartTime(node);

            GUI_DrawFlowLine(hop, node->nodeId, it->second.c_str(),
                filter.c_str(), g_appHopByHopFlowFilters[i.str()].c_str(),
                3, 0xffff, 1, 10, time);
        }
    }
}

void GUI_CreateAppHopByHopFlowFilter(
    UInt32      sessionId,
    NodeId      source,
    const char* srcString,
    const char* destString,
    const char* appName)
{
    stringstream    text;
    string          filter;
    string          tooltip = "Click to Enable/Disable Application Flow "
                        "animation for ";

    // Select a unique color base on sessionId_source
    text << sessionId << '_' << source;
    filter = text.str();
    GuiSelectAppHopByHopFlowColor(filter);

    // Store the app flow filter
    map<string, string>::iterator i = g_appHopByHopFlowFilters.find(filter);
    text.str("");
    text << appName << " " << srcString << " " << destString;
    if (i == g_appHopByHopFlowFilters.end())
    {
        g_appHopByHopFlowFilters[filter] = text.str();
    }

    // Create Application Flow Filter
    GUI_AddFilter("Application Flow Filters", "Application Flow Filters",
        true);

    // Create All button
    GUI_AddFilter("Application Flow Filters|All",
        "Click to Enable/Disable All Application Flow Filters", false);

    // Create button for the app flow filter
    filter = "Application Flow Filters|" + text.str();
    tooltip.append(text.str());
    GUI_AddFilter(filter.c_str(), tooltip.c_str(), false);
}

bool GUI_IsAppHopByHopFlowEnabled()
{
    return g_appHopByHopFlowEnabled;
}

void GUI_ReadAppHopByHopFlowAnimationEnabledSetting(
    const NodeInput* nodeInput)
{
    BOOL found = FALSE;
    BOOL enabled = FALSE;

    IO_ReadBool(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "APP-HOP-BY-HOP-FLOW-ANIMATION-ENABLED",
        &found,
        &enabled);

    g_appHopByHopFlowEnabled = (enabled != FALSE); // silly warning-suppression expression
}

void GUI_RealtimeIndicator(const char* rtStatus)
{
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];

    reply.type = GUI_ANIMATION_COMMAND;
    sprintf(replyBuff, "%d %s",
            GUI_SET_REALTIME_INDICATOR_STATUS,
            rtStatus);
    reply.args.append(replyBuff);
    if (GUI_isConnected ()) {
        GUI_SendReply(GUI_guiSocket, reply);
    }
    else if (GUI_isAnimate ()) {
        cout << reply.args << endl;
    }

#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE
}

/*
 * NAME:        GUI_SendFinalizationStatus
 * PURPOSE:     Sends finalization status to GUI when GUI is waiting for
 *              simulation finish command
 * PARAMETERS:  NodeId       nodeID: NodeId for which finalization satus
 *                                   needs to be send
 *              GuiLayers    layer: Layer for which finalization satus
 *                                   needs to be send
 *                                   (default value = GUI_ANY_LAYER)
 *              Int32        modelName: Model for which finalization satus
 *                                       needs to be send (default value = 0)
 *              Int32        subCommand: Any sub-command (default value = 0)
 *              char*        optionalMessage: Any optional message
 *                                            (default value = NULL)
 * RETURN:      none.
 */

void GUI_SendFinalizationStatus(NodeId       nodeID,
                                GuiLayers    layer,
                                Int32        modelName,
                                Int32        subCommand,
                                char*        optionalMessage)
{
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    // return in case no finalization status message required
    if (!g_sendFinalizationStatus)
    {
        return;
    }
    reply.type = GUI_FINALIZATION_COMMAND;

    // Currrently modelName and subCommand are not used
    // However, as per review comment optionalMessage needs to be passed
    // on to GUI.
    if (optionalMessage)
    {
        char checkReplyBuffOverflow[GUI_MAX_COMMAND_LENGTH];
        sprintf(checkReplyBuffOverflow, "%d %d", layer, nodeID);
        size_t remainingLength =
                    GUI_MAX_COMMAND_LENGTH - strlen(checkReplyBuffOverflow);

        if (strlen(optionalMessage) > remainingLength)
        {
            ERROR_ReportWarning("Too long optional message parameter,"
                "finalization command not sent to GUI.");
            return;
        }
        else
        {
            sprintf(replyBuff, "%d %d %s", layer, nodeID, optionalMessage);
        }
    }
    else
    {
        sprintf(replyBuff, "%d %d", layer, nodeID);
    }
    reply.args.append(replyBuff);

    if (GUI_isConnected())
    {
        GUI_SendReply(GUI_guiSocket, reply);
    }

#ifdef MULTI_GUI_INTERFACE
    if (GUI_isMultigui())
    {
        MultiGUI_SendReply(reply);
    }
#endif // MULTI_GUI_INTERFACE

}

static int g_sopsPort = 0;
static SopsProtocol g_sopsProtocol = SOPS_PROTOCOL_default;

/*
 * NAME:        SopsVopsInterfaceEnabled
 * PURPOSE:     Return true if the Sops/Vops interface is enabled
 * PARAMETERS:  none.
 * RETURN:      Return true if the Sops/Vops interface is enabled
 */
bool SopsVopsInterfaceEnabled()
{
    return g_sopsPort > 0;
}


/*
 * NAME:        SopsPort
 * PURPOSE:     Returns the SOPS Server port
 * PARAMETERS:  none.
 * RETURN:      Returns the SOPS commuication port
 */
int SopsPort()
{
    return g_sopsPort;
}


/*
 * NAME:        EnableSopsVopsInterface
 * PURPOSE:     Indicate that the Sops/Vops interface is enabled
 * PARAMETERS:  port - the TCP port the GUI communicates over.
 * RETURN:      none.
 */
void EnableSopsVopsInterface(int port, SopsProtocol protocol)
{
    g_sopsPort = port;
    g_sopsProtocol = protocol;
}

/*
 * NAME:        GetSopsProtocol()
 * PURPOSE:     Returns the SOPS protocol
 * PARAMETERS:  none.
 * RETURN:      SopsProtocol
 *              If the protocol is sopsdefault, returns TCP.
 */
SopsProtocol GetSopsProtocol()
{
    if (g_sopsProtocol == SOPS_PROTOCOL_default)
    {
        return SOPS_PROTOCOL_TCP;
    }
    return g_sopsProtocol;
}

#ifdef SOPSVOPS_INTERFACE
/*
 * NAME:        SetSopsPropertyExataNetLink
 * PURPOSE:     Set Exata Net Link property.  This will update the
 *              simulation side data store (SOPS).
 * PARAMETERS:  Node*   node: destination node
 *              int     previousHopId: source node ID
 *              int     bytesRcvd: byte received
 * RETURN:      none.
 */
void SetSopsPropertyExataNetLink(
    Node*   node,
    int     previousHopId,
    int     bytesRcvd)
{
    if (SopsVopsInterfaceEnabled())
    {
        char key[MAX_STRING_LENGTH];
        char value[MAX_STRING_LENGTH];

        typedef std::map<NodeAddress, unsigned> ReceivedMap;
        static ReceivedMap packetsReceived;
        static ReceivedMap bytesReceived;
        ReceivedMap::iterator i = packetsReceived.find(node->nodeId);

        if (i != packetsReceived.end())
        {
            i->second += 1;
        }
        else
        {
            packetsReceived[node->nodeId] = 1;
        }

        i = bytesReceived.find(node->nodeId);
        if (i != bytesReceived.end())
        {
            i->second += bytesRcvd;
        }
        else
        {
            bytesReceived[node->nodeId] = bytesRcvd;
        }

        sprintf(key, "/node/%d/exata/netLink/%d/previousHopId",
            node->nodeId, previousHopId);
        sprintf(value, "%d", previousHopId);
        SetSopsProperty(key, value);

        sprintf(key, "/node/%d/exata/netLink/%d/packetsReceived",
            node->nodeId, previousHopId);
        sprintf(value, "%u", packetsReceived[node->nodeId]);
        SetSopsProperty(key, value);

        sprintf(key, "/node/%d/exata/netLink/%d/bytesReceived",
            node->nodeId, previousHopId);
        sprintf(value, "%u", bytesReceived[node->nodeId]);
        SetSopsProperty(key, value);
    }
}
#endif

#ifdef SOPSVOPS_INTERFACE
/*
 * NAME:        SetSopsPropertyExataAppLink
 * PURPOSE:     Set Exata App Link property.  This will update the
 *              simulation side data store (SOPS).
 * PARAMETERS:  Node*   node: destination node
 *              int     sourceId: source node ID
 *              int     bytesRcvd: bytes received
 * RETURN:      none.
 */
void SetSopsPropertyExataAppLink(Node* node, int sourceId, int bytesRcvd)
{
    if (SopsVopsInterfaceEnabled())
    {
        char key[MAX_STRING_LENGTH];
        char value[MAX_STRING_LENGTH];

        typedef std::map<NodeAddress, unsigned> ReceivedMap;
        static ReceivedMap packetsReceived;
        static ReceivedMap bytesReceived;
        ReceivedMap::iterator i = packetsReceived.find(node->nodeId);

        if (i != packetsReceived.end())
        {
            i->second += 1;
        }
        else
        {
            packetsReceived[node->nodeId] = 1;
        }

        i = bytesReceived.find(node->nodeId);
        if (i != bytesReceived.end())
        {
            i->second += bytesRcvd;
        }
        else
        {
            bytesReceived[node->nodeId] = bytesRcvd;
        }

        sprintf(key, "/node/%d/exata/appLink/%d/sourceId", node->nodeId,
            sourceId);
        sprintf(value, "%d", sourceId);
        SetSopsProperty(key, value);

        sprintf(key, "/node/%d/exata/appLink/%d/packetsReceived",
            node->nodeId, sourceId);
        sprintf(value, "%u", packetsReceived[node->nodeId]);
        SetSopsProperty(key, value);

        sprintf(key, "/node/%d/exata/appLink/%d/bytesReceived",
            node->nodeId, sourceId);
        sprintf(value, "%u", bytesReceived[node->nodeId]);
        SetSopsProperty(key, value);
    }
}
#endif

/*
 * NAME:        GUI_SendAddressChange
 * PURPOSE:     Sends address change event for an interface
 * PARAMETERS:  NodeId      nodeID        : NodeId for which address changes
 *              Int32       interfaceIndex: Interface index for which address
 *                                          changes
 *              Address     addressToSet  : address that needs to be set
 *              NetworkType networkType:  : network type for an interface
 * RETURN:      none.
 */
void GUI_SendAddressChange(
                        NodeId nodeID,
                        Int32 interfaceIndex,
                        Address addressToSet,
                        NetworkType networkType)
{
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];
    char addrStr[MAX_STRING_LENGTH];

    reply.type = GUI_ANIMATION_COMMAND;

    IO_ConvertIpAddressToString(&addressToSet, addrStr);

    sprintf(replyBuff, "%d %d %d %s %d",
            GUI_ADDRESS_CHANGE,
            nodeID,
            interfaceIndex,
            addrStr,
            networkType);

    reply.args.append(replyBuff);

    if (GUI_isConnected())
    {
        GUI_SendReply(GUI_guiSocket, reply);
    }

    if (SopsVopsInterfaceEnabled())
    {
        char key[MAX_STRING_LENGTH];
        if (networkType == NETWORK_IPV4)
        {
            sprintf(key, "/node/%d/interface/%d/address/IPV4", nodeID, interfaceIndex);
        }
        else if (networkType == NETWORK_IPV6)
        {
            sprintf(key, "/node/%d/interface/%d/address/IPV6", nodeID, interfaceIndex);
        }
        SetSopsProperty(key, addrStr);
    }
}

/// \brief send uid to GUI
void SendUidToGuiClient()
{
    std::string clientID = ClientIdGenerator::generateClientId(LEGACY_CLIENT);

    GuiReply reply;
    reply.type = GUI_UID;
    reply.args.append(clientID.c_str());

    if (GUI_isConnected())
    {
        GUI_SendReply(GUI_guiSocket, reply);
    }
}


/// \brief To create and send HITL command response to subscribing client(s)
///
/// \param hid HID of the command for which response is being sent
/// \param responseType Response value
void GUI_CreateHITLResponse(const std::string& hid,
                            GuiHITLResponse responseType,
                            const std::string& responseString)
{
    GuiReply reply;
    char replyBuff[GUI_MAX_COMMAND_LENGTH];

    if (hid.empty())
    {
        return;
    }

    std::size_t index = hid.find("_");
    if (index == string::npos)
    {
        char err[MAX_STRING_LENGTH];
        sprintf(err, "Invalid HID: %s, no cmd seq number found\n", hid.c_str());
        ERROR_ReportWarning(err);
         return;
    }

    reply.type = GUI_HITL_RESPONSE;
    sprintf(replyBuff, "%d ", responseType);
    reply.args.append(replyBuff);

    if ((responseType == GUI_HITL_ERROR || responseType == GUI_HITL_OUTPUT)
         && responseString.size() > 0)
    {
        reply.args.append(responseString);
        reply.args.append(" ");
    }

    reply.args.append("HID ");
    reply.args.append(hid);

    if (hid.find("G-", 0, 2) != string::npos)
    {
        // Send HITL status to legacy clients
        if (GUI_isConnected ())
        {
            GUI_SendReply(GUI_guiSocket, reply);
        }
#ifdef MULTI_GUI_INTERFACE
        if (GUI_isMultigui())
        {
            MultiGUI_SendReply(reply);
        }
#endif // MULTI_GUI_INTERFACE
    }
    else if (hid.find("V-", 0, 2) != string::npos)
    {
#ifdef SOPSVOPS_INTERFACE
        // Send HITL status to VOPS clients
        char key[MAX_STRING_LENGTH];
        char value[MAX_STRING_LENGTH];
        std::string clientID;
        std::string cmdSeqNum;

        clientID.append(hid, 0, index);
        cmdSeqNum.append(hid, index + 1, hid.length());

        sprintf(key, "/HITL/%s/status/%s", clientID.c_str(), cmdSeqNum.c_str());
        sprintf(value, "%d", responseType);
        SetSopsProperty(
            key,
            value);
#endif
    }
    else
    {
        char err[MAX_STRING_LENGTH];
        sprintf(err, "Invalid HID: %s\n", hid.c_str());
        ERROR_ReportWarning(err);
    }
}

/// \brief Strip HID from input HITL command
///
/// \param hitlCommand Input HITL command
void GUI_StripHID(char* hitlCommand)
{
    char* indexOfHID = strstr(hitlCommand, " HID ");
    if (indexOfHID == NULL)
    {
        // HITL commands are not converted to Upper case
        // in case of a command for FIREWALL attack and
        // the HID received from GUI Client is appended
        // as "hid" i.e. lower case; hence, need to
        // check for lower case "HID" too.
        indexOfHID = strstr(hitlCommand, " hid ");
    }

    if (indexOfHID != NULL)
    {
        *indexOfHID = '\0';
    }
}

// \brief Get HID from input HITL command
//
// \param hitlCommand Input HITL command
//
// \return If the input string contains " HID " or " hid ", the
// string following it is returned, else an empty string is returned.
std::string GUI_GetHID(const char* hitlCommand)
{
    std::size_t index = 0;
    std::string hitlCommandStr(hitlCommand);
    return GUI_GetHID(hitlCommandStr, &index);
}

// \brief Get HID and its index from input HITL command
//
// \param hitlCommand Input HITL command
// \param index Index of HID
//
// \return If the input string contains " HID " or " hid ", the
// string following it and its index is returned, else an empty string
// and std::string::npos is returned.
std::string GUI_GetHID(const std::string& hitlCommand, std::size_t* index)
{
    *index = hitlCommand.rfind(" HID ");

    if (*index == std::string::npos)
    {
        // HITL commands are not converted to Upper case
        // in case of a command for FIREWALL attack and
        // the HID received from GUI Client is appended
        // as "hid" i.e. lower case; hence, need to
        // check for lower case "HID" too.
        *index = hitlCommand.rfind(" hid ");
    }

    if (*index != std::string::npos)
    {
        std::string hitlID =
            hitlCommand.substr(*index + strlen(" HID "));

        if (hitlID.length() >= MAX_HID_LENGTH)
        {
            ERROR_ReportWarning(
                "HID length in HITL command exceeds MAX_HID_LENGTH.");

            *index = std::string::npos;
            return std::string("");
        }

        return hitlID;
    }
    else
    {
        *index = std::string::npos;
        return std::string("");
    }
}

#ifdef MULTI_GUI_INTERFACE

static int g_multiguiPort = 0;

int getMultiguiPort()
{
    return g_multiguiPort;
}

void setMultiguiPort(int multigui_port)
{
    g_multiguiPort = multigui_port;
}

void setMultiguiEnabled()
{
    GUI_multigui = TRUE;
}

#endif //MULTI_GUI_INTERFACE
