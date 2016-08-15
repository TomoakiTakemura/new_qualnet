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

/// \defgroup Package_NODE NODE

/// \file
/// \ingroup Package_NODE
///
/// This file defines the Node data structure and some generic
/// operations on nodes.

#ifndef NODE_H
#define NODE_H

#include "clock.h"
#include "main.h"

#ifdef WIRELESS_LIB
#include "battery_model.h"
#endif //WIRELESS_LIB
#include "prop_mimo.h"


#include "user.h"
#include "application.h"
#include "coordinates.h"
#include "gui.h"
#include "mac.h"
#include "message.h"
#include "terrain.h"
#include "mobility.h"
#include "network.h"
#include "phy.h"
#include "propagation.h"
#include "splaytree.h"
#include "trace.h"
#include "transport.h"
#include "external.h"
#include "scheduler_types.h"
#include "atm_layer2.h"
#include "adaptation.h"
#include "spectrum.h"
#include "interference.h"

#ifdef CELLULAR_LIB
#include "cellular_gsm.h"
#endif // CELLULAR_LIB

#if defined(SATELLITE_LIB)
#include <string>
#include <map>
#include <set>
#endif // SATELLITE_LIB

#ifdef CYBER_LIB
#include "os_resource_manager.h"
#include "firewall_model.h"
#include "app_eaves.h"
#endif // CYBER_LIB

#ifdef EXATA
#include "socketlayer.h"
#endif

#ifdef AGI_INTERFACE
#include "agi_interface.h"
#endif
#ifdef NETSNMP_INTERFACE
class NetSnmpAgent;
#endif

#ifdef LTE_LIB
typedef struct struct_epc_data EpcData;
#endif // LTE_LIB

#include "jlm.h"


/// This enumeration contains indexes into the nodeGlobal array
/// used for module data. Currently not used by any QualNet protocols.
enum GlobalDataIndex
{
    GlobalData_Count = 4 // leave some room for additional data entries
};

/// This enumeration contains the mimo correlation matrix type
enum
{
    k_Gq,
    k_Identity
};

/// This struct includes all the information for a particular node.
/// State information for each layer can be accessed from this structure.

struct Node {
public:
    Node();
    ~Node();

    // Information about other nodes in the same partition.
    Node      *prevNodeData;
    Node      *nextNodeData;

    /// nodeIndex will store a value from 0 to (the number of nodes - 1);
    /// each node has a unique nodeIndex (even across multiple partitions).
    /// A node keeps the same nodeIndex even if it becomes handled by
    /// another partition.  nodeIndex should not be used by the protocol
    /// code at any layer.
    unsigned    nodeIndex;

    NodeAddress nodeId;    ///< the user-specified node identifier
    char       hostname[MAX_STRING_LENGTH];  ///< hostname (Default: "hostN" where N is nodeId).

    Int32       globalSeed;
    Int32       numNodes;  ///< number of nodes in the simulation

    SplayTree splayTree;
    clocktype timeValue;

    clocktype* startTime;

    BOOL          packetTrace;
    unsigned      packetTraceSeqno;

    BOOL          guiOption;

    PartitionData* partitionData;
    int            partitionId;

    int*          lookaheadCalculatorIndices;

    int           numberChannels;
    int           numberPhys;
    int           numberInterfaces;

    MobilityData* mobilityData;

    // The radio band concerns setting the phy to having interest in a particular
    // spectralBand (tuning the radio). In 802.11n, 802.11ac and others in the
    // future both the bandwidth and the center frequency change, and bands
    // can overlap eachother.
    const spectralBand* getRadioBand(int phyIndex, int channelIndex = -1);
    void setRadioBand(int phyIndex, const spectralBand* b);
    void setRadioBand(int phyIndex, double frequency, double bandwidth);
    // turn on or off the radio's abillity to listen to the band set above.
    void setRadioListen(int phyIndex, bool on);
    bool getRadioListen(int phyIndex);
    // treat this as private
    const spectralBand** radios;


    void initMIMO();
    bool initMIMO_done;
    complex<double> correlationFn(double D, double AoA, double AS);
    MIMO_Data& getMIMO_Data(int phyIndex);
    void setMIMO_Data(int phyIndex, int elementCount, double elementSpace, const spectralBand* band);
    const MIMO_Matrix_t& getMIMO_Hw();
    void MIMO_getEigenValues(PropRxInfo* propRxInfo, int phyIndex, 
        Orientation& txDoA, Orientation& rxDoA, MIMO_Matrix_t& mimoLambda);

     MIMO_Matrix_t mimo_Hw;
     RandomDistribution<double> mimo_Rand;
     clocktype mimoHw_interval;
     clocktype mimoHw_nextRebuild;
     const MIMO_TGn_cluster* mimo_Model; // the model will be an array of clusters, ending with weight<=0; sum(weight)=1
     // now to support Gaussian quadrature on [0,PI]
     static const int mimo_GQ_order = 20;
    typedef Eigen::Matrix< double, Eigen::Dynamic, Eigen::Dynamic> mimo_GQ_coef_t;
    typedef Eigen::Matrix< double, Eigen::Dynamic, 1> mimo_GQ_vector_t;
     mimo_GQ_vector_t mimo_GQ_weights;
     mimo_GQ_vector_t  mimo_GQ_roots;
     mimo_GQ_coef_t mimo_GQ_coef;
     void mimo_GQ_get_coefs(mimo_GQ_coef_t& coef, int order);
     double mimo_GQ_eval(mimo_GQ_coef_t& coef, int n, double x);
     double mimo_GQ_diff(mimo_GQ_coef_t& coef, int n, double x);
     void mimo_GQ_get_roots(mimo_GQ_coef_t& coef, int order);
     void MIMO_getLOSMatrix(
          double freq,
          double txES,
          double rxES,
          int txEC,
          int rxEC,
          Orientation AOA,
          double distance,
          MIMO_Matrix_t& los);
     void MIMO_getLOSMatrix(double delta, MIMO_Matrix_t& los);

    //
    // End QualNet kernel context
    //

    // Users should not modify anything above this line.

    // Layer-specific information for the  node.
    PropChannel*    propChannel;

    PropInterference m_sl;               // interference/noise data
    PropInterference& sl() { return m_sl; }
    /// \brief convenience call to obtain local JLM
    /// \return reference to local JLM for node
    Jlm& jlm() { return d_jlm; }
    Jlm d_jlm;

    PropData*       propData;
    PhyData**       phyData;             // phy layer
    MacData**       macData;             // MAC layer
    MacSwitch*      switchData;          // MAC switch

    NetworkData     networkData;         // network layer
    TransportData   transportData;       // transport layer
    AppData         appData;             // application layer
    TraceData*      traceData;           // tracing
    SchedulerInfo*  schedulerInfo;       // Pointer to the info struct for the
                                         // scheduler to be used with this node
    UserData*       userData;            // User Data
    void* globalData[GlobalData_Count];  // Global Data

    int             numAtmInterfaces;    // Number of atm interfaces
    AtmLayer2Data** atmLayer2Data;       // ATM LAYER2
    AdaptationData  adaptationData;      // ADAPTATION Layer

#ifdef AGI_INTERFACE
    // AGI STK interface
    AgiData         agiData;
#endif

    int currentInterface;
    static const int InterfaceNone = -1;

    void enterInterface(int intf);

    void exitInterface();

    int ifidx();

    /// Get current time at the node.  When processing events,
    /// nodes should always use this function.  The
    /// PartitionData::getGlobalTime() should only be used for
    /// timing at the partition or global level.
    ///
    /// \return The current time
    clocktype getNodeTime() const;

#ifdef WIRELESS_LIB
    Battery*        battery;
    float*          hwTable;
#endif //WIRELESS_LIB


#ifdef EXATA
    SLData*     slData;

#ifdef IPNE_INTERFACE
    BOOL isIpneNode;                        // is IPNE node?
#endif

#ifdef GATEWAY_INTERFACE
    NodeAddress internetGateway;     // which node is ipv4 internet gateway
    Address ipv6InternetGateway;     // which node is ipv6 internet gateway
#endif

#endif  // EXATA



#ifdef CELLULAR_LIB
    GSMNodeParameters *gsmNodeParameters;
#endif // CELLULAR_LIB

#if defined(SATELLITE_LIB)
    std::map<std::string,void*> localMap;
#endif // SATELLITE_LIB

    clocktype* lastGridPositionUpdate;
    int* currentGridBoxNumber;

    // STATS DB CODE
#ifdef ADDON_DB
    MetaDataStruct* meta_data;
#endif

#ifdef CYBER_LIB
    BOOL eavesdrop;
    OSResourceManager* resourceManager;
    FirewallModel* firewallModel;
    EavesdroppingInfo* eavesInfo;
#endif

#ifdef ADDON_ABSTRACT
    google::dense_hash_map<NodeAddress, clocktype>* oneHopNeighbors;
    clocktype lastNeighborUpdate;
    clocktype neighborUpdateInterval;
    int neighborNumInterfaces;
#endif


#ifdef NETSNMP_INTERFACE
    BOOL isSnmpEnabled;
    char *snmpdConfigFilePath;
    BOOL generateTrap;
    BOOL generateInform;
    int notification_para;
    NodeAddress managerAddress;
    int snmpVersion;
    int SNMP_TRAP_LINKDOWN_counter;
    NetSnmpAgent *netSnmpAgent;
#endif

#ifdef LTE_LIB
    EpcData* epcData;
#endif // LTE_LIB
    BOOL isMsdpEnabled;
    int  mimoCorrMatType;
    double mimoRicean_kFactor;     // In non-dB
    clocktype mimoUpdateInterval;
};


/// Contains information about the initial positions of nodes.
struct NodePositions {
    NodeAddress       nodeId;
    int               partitionId;
    NodePlacementType nodePlacementType;
    MobilityData*     mobilityData;
};


/// Function used to allocate and initialize a node.
///
/// \param partitionData  the partition that owns the node
/// \param nodeId  the node's ID
/// \param index  the node's index within the partition
///    since nodeID is non-contiguous

Node* NODE_CreateNode(PartitionData* partitionData,
                      NodeId         nodeId,
                      int            partitionId,
                      int            index);

/// Function used to call the appropriate layer to execute
/// instructions for the message
///
/// \param node  node for which message is to be delivered
/// \param msg  message for which instructions are to be executed
void NODE_ProcessEvent(Node* node, Message* msg);

/// Prints the node's three dimensional coordinates.
///
/// \param node  the node
/// \param coordinateSystemType  Cartesian or LatLonAlt
void NODE_PrintLocation(Node* node,
                        int   coordinateSystemType);

/// Get terrainData pointer.
///
/// \param node  the node
///
/// \return TerrainData pointer
TerrainData* NODE_GetTerrainPtr(Node* node);

// Legacy functions -- remove in next release
// Using these will give deprecated warnings

/// \deprecated Use Node::getNodeTime
DEPRECATED_MSG("Use Node::getNodeTime") clocktype getSimTime(Node* node);

/// \deprecated Use Node::getNodeTime
DEPRECATED_MSG("Use Node::getNodeTime") clocktype TIME_getSimTime(Node* node);

inline bool PHY_IsListeningToChannel(Node* node, int phyIndex, int channelIndex) {
    return node->propData[channelIndex].phyListening[phyIndex];
}
#endif // NODE_H
