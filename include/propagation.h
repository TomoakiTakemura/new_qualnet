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

/// \defgroup Package_PROPAGATION PROPAGATION

/// \file
/// \ingroup Package_PROPAGATION
/// This file describes data structures and functions used by propagation models.

#ifndef PROPAGATION_H
#define PROPAGATION_H

#include "main.h"
#include "coordinates.h"
#include "fileio.h"
#include "random.h"
#include "dynamic.h"
#include "terrain.h"
#include "prop_mimo.h"

#include <map>
#include <string>
#include <vector>

#include <algorithm>
using namespace::std;

/// Boltzmann constant
#define BOLTZMANN_CONSTANT 1.379e-23

/// Path loss in dB (used as an invalid value)
#define NEGATIVE_PATHLOSS_dB -1.0

/// Defines the value of speed of light
#define SPEED_OF_LIGHT                3.0e8

/// Default value for propagation limit.
#define PROP_DEFAULT_PROPAGATION_LIMIT_dBm -111.0

/// Default mean value for shadowing in dB
#define PROP_DEFAULT_SHADOWING_MEAN_dB 4.0
//PL_OPAR_PROP
#define PROP_DEFAULT_INTER_CITY_OBSTRUCTION_DENSITY_FACTOR 0.04
#define PROP_DEFAULT_INTRA_CITY_OBSTRUCTION_DENSITY_FACTOR 0.02
#define PROP_DEFAULT_INTER_CITY_FOLIAGE_OBSTRUCTION_DENSITY_FACTOR 0.4
#define PROP_DEFAULT_INTRA_CITY_FOLIAGE_OBSTRUCTION_DENSITY_FACTOR 0.2

/// Maximum number of sample would be taken.
#define MAX_NUM_ELEVATION_SAMPLES 16384

/// The bandwidth factor that is used to get the half sum bandwidth.
#define PROP_DEFAULT_BANDWIDTH_FACTOR 2.0

/// Get the number of channel.
#define PROP_NumberChannels(node) ((node)->numberChannels)

/// Get wavelength of channel having index channelIndex
#define PROP_ChannelWavelength(node, channelIndex) \
            ((node)->partitionData->propChannel[(channelIndex)].profile->wavelength)


#define HORIZONTAL 0
#define VERTICAL 1

/// Different type of path loss.
enum PathlossModel {
    FREE_SPACE = 0,
    TWO_RAY,
    PL_MATRIX,
    OPAR,
    ITM,
    TIREM,
    OKUMURA_HATA,
    ASAPS,
    RFPS,
    COST231_HATA,
    COST231_WALFISH_IKEGAMI,
    ITU_R,
    URBAN_MODEL_AUTOSELECT,
    STREET_MICROCELL,
    STREET_M_TO_M,
    INDOOR,
    SUBURBAN_FOLIAGE,
    PL_OPAR,
    PL_OPAR_PROP,
    FLAT_BINNING
};

/// Different type of shadowing used.
enum ShadowingModel {
    CONSTANT = 0,
    LOGNORMAL
};

/// Different type of fading used.
enum FadingModel {
    NONE = 0,
    RICEAN
};

/// Different type of propagation environment.
enum PropagationEnvironment {
    OPEN_RURAL = 0,
    QUASI_OPEN_RURAL,
    SUBURBAN,
    URBAN,
    METROPOLITAN  // building heights greater than 15 m
};

/// Indicated if the path is Line of sight OR non-Line of sight
enum LoSIndicator {
    LOS = 0,
    NLOS
};


/// Terrain types for Suburban-foliage model

enum SuburbanTerrainType{
    HILLY_TERRAIN_WITH_MOD_TO_HEAVY_TREE_DENSITY = 0,
    FLAT_TERRAIN_WITH_MOD_TO_HEAVY_TREE_DENSITY,
    FLAT_TERRAIN_WITH_LIGHT_TREE_DENSITY
};


/// Link types for Indoor model

enum IndoorLinkType {
    RR = 0, //Room-to-Room
    CC,     //Corridor-to-Corridor
    OO,     //Open area-to-Open area
    RC,     //Room-to-Corridor
    CO,     //Corridor-to-Open area
    RO      //Room-to-Open area
};

enum ObstructionType {
    OBSTRUCTION_BUILDING = 0,
    OBSTRUCTION_FOLIAGE
};

/// Link types for model
enum LinkType {
    OUTDOOR_ONLY = 0,
    INDOOR_ONLY,
    HETEROGENEOUS
};

/// Structure that keeps track of all propertice of a path.
struct PropPathProfile {
    clocktype   propDelay;
    double      distance;
    Orientation txDOA;
    Orientation rxDOA;
    double      rxPower_dBm;        // rx power at receiver
    double      pathloss_dB;        // includes shadowing
    double      shadowing_dB;       // for freespace and two-ray
    double      fading_dB;
    double      channelReal;        // for cooperative comm
    double      channelImag;        // for cooperative comm
    double      rxFrequency;        // frequency at receiver side
    Coordinates fromPosition;
    Coordinates toPosition;
    int         sequenceNum;
    double      weatherPathloss_dB; // pathloss due to weather
    int         weatherSequenceNum;
};

struct PropGridMatrix {
    std::map<int, PropPathProfile> gridMatrix;
};

/// structure of a channel.
struct PropChannel {
    int       numNodes;
    Node**    nodeList;

    int       numNodesWithLI; // LI: Limited Interference
    Node**    nodeListWithLI;

    int       profileIndex;
    std::string name;

    int maxNumNodes;
    PropProfile* profile;
};

//PL_OPAR_PROP
struct Obstruction {
    ObstructionType obstructiontype;
    Coordinates southwestOrLowerLeft;
    Coordinates northeastOrUpperRight;
    double intraCityObstructionDensityFactor;
    double interCityObstructionDensityFactor;
};

//PL_OPAR_PROP
struct PathlossArea {
    PathlossModel pathlossModel;
    PathlossModel pathlossModelPrimary;
    Coordinates southwestOrLowerLeft;
    Coordinates northeastOrUpperRight;
};

struct pathLossMatrixValue{
    clocktype simTime;
    string values;
};
/// Main structure of propagation profile
struct PropProfile {
    int       profileIndex;

    double    propLimit_dB;
    D_Float64    propMaxDistance;
    //double    propCommunicationProximity;
    //double    propProfileUpdateRatio;
    D_Float64 propCommunicationProximity;
    D_Float64 propProfileUpdateRatio;
    double    frequency;
    double    antennaHeight;
    double    wavelength;

    PathlossModel pathlossModel;

    float elevationSamplingDistance;
    int   climate;
    double refractivity;
    double conductivity;
    double permittivity;
    double humidity;
    int   polarization;
    char polarizationString[5];

    ShadowingModel shadowingModel;
    double shadowingMean_dB;

    FadingModel fadingModel;
    double kFactor;
    double dopplerFrequency;

    double  baseDopplerFrequency;
    D_Int32     samplingRate;
    int     numGaussianComponents;
    double* gaussianComponent1;
    double* gaussianComponent2;

    int       numChannelsInMatrix;
    int*      channelIndexArray;
    int       numNodesInMatrix;
    vector<pathLossMatrixValue>      matrixList;

    void *propGlobalVar;

    // high frequency
    double monthofyear;
    double dayofmonth;
    double timeofday;
    double Tindex;
    double mintoa;
    double hfTxpower;
    double reqSnr;
    double manMadenoise;
    double hfbandwidth;
    double reqPercentageDay;
    void   *TxantennaData;
    void   *RxantennaData;

    // ASAPS-sepcific
    char asapsPath[MAX_STRING_LENGTH];

    // motion
    BOOL motionEffectsEnabled;

    // OKUMURA_HATA, COST231_Hata, COST231_Walfish_Ikegami,ITU-R
    PropagationEnvironment propagationEnvironment;

    //COST231_Walfish_Ikegami
    double roofHeight;
    double streetWidth;
    double buildingSeparation;

    //Added for URBAN-AUTOSELECT-MODEL
    double RelativeNodeOrientation;
    double MaxRoofHeight;
    double MinRoofHeight;

    //Street_Microcell model, Indoor
    LoSIndicator losIndicator;

    //Street_M_to_M model
    int Num_builings_in_path;

    //Suburban-Foliage Model
    SuburbanTerrainType suburbanTerrainType;

    // PL_OPAR
    PathlossModel pathlossModelPrimary;

    // PL_OPAR_PROP
    Obstruction* obstructions;
    int numObstructions;

    PathlossArea* pathlossArea;
    int numPathlossAreas;

    // MIMO channel profile
    MIMO_ChannelProfile mimoProfile;

    TERRAIN::ConstructionMaterials  constructionMaterials;
    BOOL enableChannelOverlapCheck;

    // TIREM library
    void * tiremLibHandle;
};

/// Main structure of propagation data.
struct PropData {
private:
    int   numPhysListenable;
    int   numPhysListening;
    bool* phyListenable;
    bool* phyListening;

    friend void PHY_StartListeningToChannel(Node* node, int phyIdx, int channelIdx);
    friend void PHY_StopListeningToChannel(Node* node, int phyIdx, int channelIdx);
    friend bool PHY_IsListeningToChannel(Node* node, int phyIdx, int channelIdx);

    friend void PHY_AllowListeningToChannel(Node* node, int phyIdx, int channelIdx);
    friend void PHY_DisallowListeningToChannel(Node* node, int phyIdx, int channelIdx);
    friend bool PHY_CanListenToChannel(Node* node, int phyIdx, int channelIdx);

    friend void PROP_Init(Node* node, int channelIdx, NodeInput* input);
    friend Node* NODE_CreateNode(PartitionData*, NodeAddress, int, int);
public:
    int getNumPhysListenable() {return numPhysListenable;}
    int getNumPhysListening() {return numPhysListening;}

    BOOL  limitedInterference;

    RandomDistribution<double> shadowingDistribution;
    int  nodeListId;

    int  numSignals;

    PropRxInfo* rxSignalList;

    double fadingStretchingFactor;

    //PropPathProfile* pathProfile;

    void *propVar;
    int numPathLossCalculation;

    // These functions are not needed if there is only one PropPathProfile object per PropData, if we 
    // need to change back to having an array of numNodes elements then these functions are already in
    // the correct place.  Otherwise the stubs will keep the compiler quiet.
    void newPathProfiles(int) { }
    void deletePathProfiles() { }

    PropPathProfile* getPathProfile(int) {return initPathProfile(&thePropPathProfile);}
private:
    PropPathProfile* initPathProfile(PropPathProfile* p) {
        p->propDelay = 0;
        p->distance = 0.0;
        p->txDOA.azimuth = 0;
        p->txDOA.elevation = 0;
        p->rxDOA.azimuth = 0;
        p->rxDOA.elevation = 0;
        p->pathloss_dB = NEGATIVE_PATHLOSS_dB;
        // this will force a recalcuation of all the fields inside
        // PROP_CalculateRxPowerAndPropagationDelay()
        p->sequenceNum = -1;
        return p;
}
    PropPathProfile thePropPathProfile;
};

/// This structure is used for fields related to channel layer
/// information that need to be sent with a message.
struct PropTxInfo {
    clocktype txStartTime; // signal airborne time
    clocktype duration;    // signal duration
    double  txPower_dBm;   // transmit power in dBm
    int     dataRate;      // data rate
    short   phyIndex;      // transmitter index
    short   numReferenced; // number of receivers referencing this message
    int     patternIndex;  // antenna pattern
    int     sequenceNum;   // needed for checking if the path profile is up to date
    int     txNodeId;      // required for distributed memory parallel
    Node*   txNode;
    Coordinates position;
    Coordinates lastPosition;  // temporary substitute for velocity
    double      speed;         // ditto
    Orientation orientation;
    unsigned char txPhyModel;  // The PhyModel of the transmitter

    //AntennaModelType antennaModelType; // used by kernel
    Orientation      steeringAngle;    // used by kernel

    // MIMO related parameters
    int           txNumAtnaElmts;    //Number of transmit antenna elements
    double        txAtnaElmtSpace;   //Transmit antenna element space

};

/// This structure is used for fields related to channel layer
/// information that need to be received with a message.
struct PropRxInfo {
    NodeAddress txNodeId;
    short       txPhyIndex;
    clocktype   rxStartTime; // signal arrival time
    clocktype   duration;    // signal duration
    BOOL        distorted;
    double      rxPower_dBm;   // signal power before adding receiver antenna gain
    double      pathloss_dB;
    double      fading_dB;
    int         channelIndex;
    double      channelReal;   // for cooperative comm
    double      channelImag;   // for cooperative comm
    double      frequency;     // frequency at receiver side
    double      bandwidth;
    Orientation txDOA;
    Orientation rxDOA;

    // MIMO related parameters
    int         txNumAtnaElmts;    //Number of transmit antenna elements
    double      txAtnaElmtSpace;   //Transmit antenna element space

    Message*    txMsg;
    PropRxInfo* prev;
    PropRxInfo* next;
};

/// Initialization function for propagation
/// This function is called from each partition,
/// not from each node
///
/// \param partitionData  structure shared among nodes
/// \param nodeInput  structure containing contents of input file
void PROP_GlobalInit(PartitionData *partitionData, NodeInput *nodeInput);

/// Initialize some partition specific data structures.
/// This function is called from each partition, not from each node
/// This function is only called for non-MPI
///
/// \param partitionData  structure shared among nodes
/// \param nodeInput  structure containing contents of input file
void PROP_PartitionInit(PartitionData *partitionData, NodeInput *nodeInput);

/// Initialization function for propagation functions.
/// This function is called from each node.
///
/// \param node  node being initialized.
/// \param channelIndex  channel being initialized.
/// \param nodeInput  structure containing contents of input file
void PROP_Init(Node *node, int channelIndex, NodeInput *nodeInput);

/// To receive message.
///
/// \param node  Node that is
///    being instantiated in
/// \param msg  message received by the layer
void PROP_ProcessEvent(Node *node, Message *msg);

/// To collect various result.
///
/// \param node  node for which results are to be collected
void PROP_Finalize(Node *node);

/// Calculates pathloss using free space model.
///
/// \param distance  distance (meters) between two nodes
/// \param wavelength  wavelength used for propagation.
///
/// \return pathloss in db
double PROP_PathlossFreeSpace(double distance,
                              double waveLength);

/// To calculate path loss of a channel.
///
/// \param distance  distance (meters) between two nodes
/// \param wavelength  wavelength used for propagation.
/// \param txAntennaHeight  tranmitting antenna hight.
/// \param rxAntennaHeight  receiving antenna hight.
///
/// \return pathloss in db
double PROP_PathlossTwoRay(double distance,
                           double waveLength,
                           float txAntennaHeight,
                           float rxAntennaHeight);

/// Calculates extra path attenuation using opar model.
///
/// \param distance  distance (meters) between two nodes
/// \param OverlappingDistance  overlapping distance
/// \param frequency  frequency used for propagation.
/// \param obstructiontype  obstruction type
///
/// \return extra path attenuation in db


double PROP_PathlossOpar(double distance,
                         double OverlappingDistance,
                         double frequency,
                         ObstructionType obstructiontype);

/// To calculate path loss of a channel.
///
/// \param node  Node that is
///    being instantiated in
/// \param txNodeId  including for debugging
/// \param rxNodeId  including for debugging
/// \param channelIndex  channel number.
/// \param wavelength  wavelength used for propagation.
/// \param txAntennaHeight  tranmitting antenna hight.
/// \param rxAntennaHeight  receiving antenna hight.
/// \param pathProfile  characteristics of path.
/// \param forBinning  disables some features to support
///    flat binning
void PROP_CalculatePathloss(
    Node* node,
    NodeId txNodeId,
    NodeId rxNodeId,
    int channelIndex,
    double wavelength,
    float txAntennaHeight,
    float rxAntennaHeight,
    PropPathProfile* pathProfile,
    double* pathloss_dB,
    bool forBinning = false);

/// To calculate fading between two node.
///
/// \param propTxInfo  Information about the transmitter
/// \param node2  receiver
/// \param channelIndex  channel number
/// \param currentTime  current simulation time
/// \param fading_dB  calculated fading store here.
/// \param channelReal  for cooperative comm
/// \param channelImag  for cooperative comm
void PROP_CalculateFading(
    Message* signalMsg,
    PropTxInfo* propTxInfo,
    Node* node2,
    int channelIndex,
    clocktype currentTime,
    float* fading_dB,
    double* channelReal,
    double* channelImag);

/// Determines when shadowing applies to pathloss.
/// It applies when the model is FREE_SPACE or TWO_RAY, or 
/// with PL_OPAR or PL_OPAR_PROP models if the primary pathloss
/// model is FREE_SPACE or TWO_RAY.
///
/// \param node  Receiving node
/// \param channelIndex  Channel that the signal is propagated
///
/// \return shadowing was calculated
bool PROP_ShadowingApplies(
    Node* node,
    int channelIndex);


/// Calculates shadowing when it applies to the configured
/// pathloss model and returns true. If shadowing does not
/// apply, sets shadowing to zero and returns false. 
/// It applies when the model is FREE_SPACE or TWO_RAY, or 
/// with PL_OPAR or PL_OPAR_PROP models if the primary pathloss
/// model is FREE_SPACE or TWO_RAY.
///
/// \param node  Receiving node
/// \param channelIndex  Channel that the signal is propagated
/// \param shadowing_dB  address to store shadowing
///
/// \return shadowing was calculated
bool PROP_CalculateShadowing(
    Node* node,
    int channelIndex,
    double* shadowing_dB);

/// This function will be called by QualNet wireless
/// propagation code to calculate rxPower and prop delay
/// for a specific signal from a specific tx node to
/// a specific rx node.
///
/// \param msg  Signal to be propagated
/// \param channelIndex  Channel that the signal is propagated
/// \param propChannel  Info of the propagation channel
/// \param propTxInfo  Transmission parameers of the tx node
/// \param txNode  Point to the Tx node
/// \param rxNode  Point to the Rx node
/// \param pathProfile  For returning results
///
/// \return If FALSE, indicate the two nodes cannot comm
/// TRUE means two nodes can communicate
BOOL PROP_CalculateRxPowerAndPropagationDelay(
         Message* msg,
         int channelIndex,
         PropChannel* propChannel,
         PropTxInfo* propTxInfo,
         Node* txNode,
         Node* rxNode,
         PropPathProfile* pathProfile);

/// This function will be called by QualNet wireless
/// propagation code to calculate rxPower and prop delay
/// for a specific signal from a specific tx node to
/// a specific rx node.
///
/// \param msg  Signal to be propagated
/// \param channelIndex  Channel that the signal is propagated
/// \param propChannel  Info of the propagation channel
/// \param propTxInfo  Transmission parameers of the tx node
/// \param txNode  Point to the Tx node
/// \param rxNode  Point to the Rx node
/// \param pathProfile  For returning results
///
/// \return If FALSE, indicate the two nodes cannot comm
/// TRUE means two nodes can communicate
BOOL PROP_DefaultCalculateRxPowerAndPropagationDelay(
         Message* msg,
         int channelIndex,
         PropChannel* propChannel,
         PropTxInfo* propTxInfo,
         Node* txNode,
         Node* rxNode,
         PropPathProfile* pathProfile);

/// Get a stretching factor for fast moving objects.
///
/// \param propTxInfo  Transmitter information
/// \param receiver  Receiver node.
/// \param channelIndex  channel number
void PROP_MotionObtainfadingStretchingFactor(
    PropTxInfo* propTxInfo,
    Node* receiver,
    int   channelIndex);

// API              :: PROP_AddToChannelList
// PURPOSE          :: Add the node to the channel list
// PARAMETERS       ::
// + node            : Node*    : Node that is
//                                being instantiated in
// + channelIndex    : int      : channel number
// RETURN           :: void :
void PROP_AddToChannelList(Node *node, int channelIndex);

/// remove node from propChannel nodeList
///
/// \param node  the node
/// \param channelIndex  channel index
void PROP_RemoveFromChannelList(Node* node, int channelIndex);

/// UpdatePathProfiles
///
/// \param node  Node that is
///    being instantiated in
void PROP_UpdatePathProfiles(Node* node);

/// Release (transmit) the signal
///
/// \param node  Node that is
///    being instantiated in
/// \param msg  Signal to be transmitted
/// \param phyIndex  PHY data index
/// \param channelIndex  chanel index
/// \param txPower_dBm  transmitting power
/// \param duration  transmission duration
/// \param delayUntilAirborne  delay until airborne
#ifdef PHY_SYNC
void PROP_ReleaseSignal(
    Node *node,
    Message *msg,
    int phyIndex,
    int channelIndex,
    float txPower_dBm,
    clocktype duration,
        clocktype delayUntilAirborne,
        PhyModulation modulation = PHY_MODULATION_DEFAULT,
        PhyEncoding   encoding = PHY_ENCODING_DEFAULT,
        PhyCodingParameter codingParameter = 1);
#else
    void PROP_ReleaseSignal(
        Node* node,
        Message* msg,
        int phyIndex,
        int channelIndex,
        float txPower_dBm,
        clocktype duration,
        clocktype delayUntilAirborne,
        int     txNumAtnaElmts = 1,
        double  txAtnaElmtSpace = 0.0);

#endif // PHY_SYNC

/// Unreference a signal (internal use)
///
/// \param node  Node that is
///    being instantiated in
/// \param msg  Signal to be unreferenced
void PROP_UnreferenceSignal(Node *node, Message *msg);

/// Calculate inter-node pathloss, distance values
/// between all the nodes on a given  channel
/// 
///
/// \param node  any valid node
/// \param channelIndex  selected channel instance
/// \param numNodesOnChannel  number of nodes using this channel
/// \param nodeIdList  list of (numNodesOnChannel) nodeIds
/// \param pathloss_dB  2D pathloss array for nodes in
///    nodeIdList
/// \param distance  2D array of inter-node distances
// COMMENTS         :: To access values at location [i][j]:
/// (pathloss_dB + numNodes * i + j).
/// 
/// Memory of nodeIdList, pathloss_dB and distance
/// 'should be freed' by users after usage.
/// 
/// The pathloss value of a node to itself will be 0.0
/// For example: If nodes 1, 3, 4 can use channel 1,
/// then the pathloss values in pathloss_dB are:
/// NodeIdList: 1     3       4
/// ------------------------------
/// 1       0.0    97.49   94.33
/// 3      97.49    0.0    91.43
/// 4      94.33   91.43    0.0
/// *//
/*
void PROP_CalculateInterNodePathLossOnChannel(
        Node        *node,
        int         channelIndex,
        int         *numNodesOnChannel,
        NodeAddress **nodeIdList,
        float       **pathloss_dB,
        float       **distance);
        */



//
// API              :: PROP_IsLineOfSight
//
// PURPOSE          :: Check if the path is line of sight
//
// PARAMETERS       ::
// + numSamples             : int           : terrain data sample number
// + sampleDistance         : double        : terrain data sample distance
// + terrainProfile         : double*       : terrain profile sample data
// + txHeight               : double        : Tx node height
// + rxHeight               : double        : Rx node height
// + surfaceRefractivity    : double        : earth surface refractivity
// RETURN               :: BOOL :
BOOL PROP_IsLineOfSight (int     numSamples,
                         double  sampleDistance,
                         double* terrainProfile,
                         double  txHeight,
                         double  rxHeight,
                         double  surfaceRefractivity);

/// Calculate the wireless propagation delay for the given distance and 
/// propagation speed.
///
/// \param distance  Propagation distance
/// \param propSpeed  Propagation speed (per second)
inline
clocktype PROP_CalculatePropagationDelay(double distance, double propSpeed)
{
    return (clocktype) (distance * SECOND / propSpeed + 0.5);
}


// Prototype required in case Wireless library is missing.
double PathlossMatrix(
    Node* node,
    NodeAddress nodeId1,
    NodeAddress nodeId2,
    int channelIndex,
    clocktype currentTime);


// API              :: PROP_ChangeSamplingRate
//
// PURPOSE          :: Change sampling rate and fading stretching factor
//
// PARAMETERS       ::
// + node            : Node   pointer for the updated node
// + channelIndex    : int    channel index
// RETURN           :: NONE :

void PROP_ChangeSamplingRate(Node *node, int channelIndex);

double PROP_ReturnRxPowerForNodePair(
    Node* node,
    Node* txNode,
    Node* rxNode,
    int channelIndex,
    clocktype futureTime,
    BOOL isPredictive,
    double* noise,
    int* bandwidth,
    double* power_dBm,
    double* propLimit_db,
    double* threshold,
    double* phySnrThreshold);

void PROP_CollectPathlossSample(Node *node);
#ifdef ADDON_DB
void PROP_CollectConnectSample(Node *node);
void PROP_CollectConnectSampleParallel(Node* node);
void PROP_FinalizePathlossMatrixOutput(PartitionData* partitionData);
void PROP_OutputConnectTable(PartitionData* partitionData);

void PROP_GetPOPStatus(PartitionData* partitionData, FILE* fp);

int PROP_ChannelNodesListenOn(Node *node1, Node *node2);
BOOL PROP_CheckChannelForNodes(Node *node1, Node *node2, int channelIndex);
int PROP_ReturnNodeChannelIndex(Node* node, int channelIndex);
// map node id
int MNI(PartitionData* partitionData, int nodeId);

void PROP_FindShorestPath(PartitionData* partitionData, Node* node, int** resultTable);
#endif

/// Reset previous channel
/// remove/add node to propChannel for signal delivery,
/// in propagation_private.
///
/// \param node  Node that is being instantiated in
/// \param phyIndex  interface index
/// \param newChannelListenable  new channel
void PROP_Reset(Node* node, int phyIndex, char* newChannelListenable);

void PROP_RecordSignalRelease(
    Node *node,
    Message *msg,
    int phyIndex,
    int channelIndex,
    float txPower_dBm);

/// Get channel frequency from profile for
/// PropChannel.
///
/// \param node  the node
/// \param channelIndex  channel index
///
/// \return channel frequency
double PROP_GetChannelFrequency(Node* node, int channelIndex);

/// Set channel frequency from profile for
/// PropChannel.
///
/// \param node  the node
/// \param channelIndex  channel index
/// \param channelFrequency  new channel frequency
void PROP_SetChannelFrequency(Node* node,
                              int channelIndex,
                              double channelFrequency);

/// Get channel wavelength from profile for
/// PropChannel.
///
/// \param node  the node
/// \param channelIndex  channel index
///
/// \return channel wavelength
double PROP_GetChannelWavelength(Node* node, int channelIndex);

/// Set channel wavelength from profile for
/// PropChannel.
///
/// \param node  the node
/// \param channelIndex  channel index
/// \param channelWavelength  new channel wavelength
void PROP_SetChannelWavelength(Node* node,
                               int channelIndex,
                               double channelWavelength);

/// Get channel doppler freq  from profile for
/// PropChannel.
///
/// \param node  the node
/// \param channelIndex  channel index
///
/// \return channel doppler freq
double PROP_GetChannelDopplerFrequency(Node* node, int channelIndex);

/// Set channel doppler freq from profile for
/// PropChannel.
///
/// \param node  the node
/// \param channelIndex  channel index
/// \param channelDopplerFrequency  new channel doppler freq
void PROP_SetChannelDopplerFrequency(Node* node,
                                     int channelIndex,
                                     double channelDopplerFrequency);

/// Check if there is frequency overlap between signal and receiver node.
/// 
///
/// \param txNode  the Tx node
/// \param rxNode  the Rx node
/// \param txChannelIndex  the Tx channel index
/// \param rxChannelIndex  the Rx channel index
/// \param txPhyIndex  the PHY index for the Tx node.
/// \param rxPhyIndex  the PHY index for the Rx node.
///
/// \return if there is frequency overlap

BOOL PROP_FrequencyOverlap(
   Node *txNode,
   Node *rxNode,
   int txChannelIndex,
   int rxChanelIndex,
   int txPhyIndex,
   int rxPhyIndex );

#endif /* PROPAGATION_H */
