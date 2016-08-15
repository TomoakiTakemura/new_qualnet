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

/// \defgroup Package_PHYSICAL_LAYER PHYSICAL LAYER

/// \file
/// \ingroup Package_PHYSICAL_LAYER
/// This file describes data structures and functions used by the Physical Layer.
/// Most of this functionality is enabled/used in the Wireless library.

#include <map>
#include <vector>

#ifndef PHY_H
#define PHY_H

#include "propagation.h"
#include "dynamic.h"
#include "energy_model.h"
#include "antenna_global.h"

#ifdef WIRELESS_LIB
#include "antenna_global.h"
#include "energy_model.h"
#endif // WIRELESS_LIB

#include "prop_mimo.h"

#ifdef ADDON_DB
struct PhyOneHopNeighborData;
#endif

class STAT_PhyStatistics;

#ifdef ADDON_NGCNMS
struct PowerControlData;
#endif


/// Default noise factor in physical medium
#define PHY_DEFAULT_NOISE_FACTOR     10.0

/// Default temperature of physical medium.
#define PHY_DEFAULT_TEMPERATURE      290.0

/// Default minimum pcom value threshold
#define PHY_DEFAULT_MIN_PCOM_VALUE      0.0

/// Default minimum pcom value threshold
#define PHY_DEFAULT_SYNC_COLLISION_WINDOW    1 * MILLI_SECOND

//**
// CONSTANT    :: PHY_NEGATIVE_BANDWIDTH  : -1.0
// DESCRIPTION :: Bandwidth in Hz (used as an invalid value)
#define PHY_NEGATIVE_BANDWIDTH -1.0

/// Different phy types supported.

enum PhyModel {
    PHY802_11a = 0,
    PHY802_11b = 1,
    PHY_ABSTRACT = 2,
    PHY_GSM = 3,
    PHY_FCSC_PROTOTYPE = 4,
    PHY_SATELLITE_RSV = 7,
    PHY802_16 = 8,
    PHY_CELLULAR = 9,
    PHY_JAMMING = 11, // #10 was removed, set to 11 to maintain random seeds
    PHY802_15_4 = 12,

    // Stats DB
    PHY_STATSDB_AGGREGATE = 13,
    PHY_STATSDB_SUMMARY = 14,

    PHY_ABSTRACT_LAYER = 15,
    PHY_LTE = 17,
    PHY802_11n = 19,
    PHY802_11ac = 20,
    PHY802_11pCCH = 21,
    PHY802_11pSCH = 22,
    PHY_GENERIC_OFDM = 23,
    PHY_NONE
};

struct PhyFamily {
  enum ModulationFamily {
    kLinearModulation = 1,
    kDirectSequence = 2,
    kFrequencyHopping = 4,
    kMultiCarrier = 8,
    kVirtual = 32767
  } ;

  static bool isMulticarrier(PhyModel x) {
    switch (x) {
        // This is not a comprehensive list, use presently for spectral band support && OFDM
        case PHY802_11a: case PHY802_11b: case PHY802_11n: 
        case PHY802_11ac: case PHY802_11pCCH: case PHY802_11pSCH:
        case PHY_GENERIC_OFDM: return true;
    }
    return false;
  } 
} ;

/// Different types of packet reception model

enum PhyRxModel {
    RX_802_11a = 1,
    RX_802_11b,
    RX_802_11n,
    RX_802_16,
    RX_UMTS,
    RX_802_15_4,
    SNR_THRESHOLD_BASED,
    BER_BASED,
    PCOM_BASED,
    PER_BASED,
    SER_BASED,
    RX_LTE,
    RX_802_11ac,
    RX_802_11pCCH,
    RX_802_11pSCH,
    RX_MODEL_NONE
};

//**
// ENUM        :: PhyStatusType
// DESCRIPTION :: Status of physical layer
enum PhyStatusType {
  PHY_IDLE,
  PHY_SENSING,
  PHY_RECEIVING,
  PHY_TRANSMITTING,

  PHY_BUSY_TX,
  PHY_BUSY_RX,
  PHY_SUCCESS,
  PHY_TRX_OFF
};

/// SNR/BER curve entry
struct PhyBerEntry {
    double snr;
    double ber;
};

/// Bit Error Rate table.
struct PhyBerTable {
    char         fileName[MAX_STRING_LENGTH];
    int          numEntries;

    // When the snr values all have the same interval
    // we can lookup entires by snr directly instead
    // of performing a binary search.
    bool         isFixedInterval;
    double       interval;
    double       snrStart;
    double       snrEnd;
    PhyBerEntry* entries;

    PhyBerTable()
    : numEntries(0), isFixedInterval(false), interval(0.0), 
      snrStart(0.0), snrEnd(0.0), entries(NULL)
    { memset(fileName, 0, MAX_STRING_LENGTH); }

    // weakish copy
    PhyBerTable& operator=(const PhyBerTable& b)
    {
      memcpy(fileName, b.fileName, MAX_STRING_LENGTH);
      numEntries = b.numEntries;
      isFixedInterval = b.isFixedInterval;
      interval = b.interval;
      snrStart = b.snrStart;
      snrEnd = b.snrEnd;
      entries = b.entries;
      return *this;
    }
};

void
PHY_BerTablesPrepare (std::vector<PhyBerTable>& berTables, const int tableCount);

/// SNR/PER curve entry
struct PhyPerEntry {
    double snr;
    double per;
};

/// Packet Error Rate table.
struct PhyPerTable {
    char         fileName[MAX_STRING_LENGTH];
    int          numEntries;

    // When the snr values all have the same interval
    // we can lookup entires by snr directly instead
    // of performing a binary search.
    bool         isFixedInterval;
    double       interval;
    double       snrStart;
    double       snrEnd;
    PhyPerEntry* entries;
};

void
PHY_PerTablesPrepare (PhyPerTable * perTables, const int tableCount);

PhyPerTable *
PHY_PerTablesAlloc (const int tableCount);

/// SNR/PER curve entry
struct PhySerEntry {
    double snr;
    double ser;
};

/// Symbol Error Rate table.
struct PhySerTable {
    char         fileName[MAX_STRING_LENGTH];
    int          numEntries;

    // When the snr values all have the same interval
    // we can lookup entires by snr directly instead
    // of performing a binary search.
    bool         isFixedInterval;
    double       interval;
    double       snrStart;
    double       snrEnd;
    PhySerEntry* entries;
};

void
PHY_SerTablesPrepare (PhySerTable * serTables, const int tableCount);

PhySerTable *
PHY_SerTablesAlloc (const int tableCount);


/// Measurement of the signal of received pkt
struct PhySignalMeasurement {
    clocktype rxBeginTime;  // signal arrival time
    double snr;  // signal to noise ratio
    double rss;  // receving signal strength in mW
    double cinr; // signal to interference noise ratio

    // only used by PHY802.16
    unsigned char fecCodeModuType; // coding modulation type for the burst
};

/// Structure for classifying different types of antennas.

struct AntennaModel {
#ifdef WIRELESS_LIB
    AntennaModelType   antennaModelType;
    int                numModels;
    AntennaPatternType antennaPatternType;
    void               *antennaVar;
#endif // WIRELESS_LIB
};


/// Structure for an omnidirectional antenna.

struct AntennaOmnidirectional {
    float antennaHeight;
    float antennaGain_dB;
    double  systemLoss_dB;
    char antennaModelName[2 * MAX_STRING_LENGTH];
};

/// Used by Phy layer to store PCOM values
struct PhyPcomItem {
    clocktype startTime;
    clocktype endTime;
    double pcom;
    PhyPcomItem* next;
};


struct AbstractPhyStru;

/// Structure for phy layer
struct PhyData {
    int         phyIndex;
    int         macInterfaceIndex;
    Address*    networkAddress;
    BOOL        phyStats;
    int         channelIndexForTransmission;

    PhyModel       phyModel;
#ifdef CYBER_LIB
    BOOL      jammerStatistics;
    int       jamInstances;
    clocktype jamDuration;
#endif



    PhyRxModel     phyRxModel;
    double         phyRxSnrThreshold;
    double         noise_mW_hz;

    // int            numBerTables;
    // PhyBerTable*   snrBerTables;

    std::map<PhyRxModel, std::vector<PhyBerTable> > d_extSnrBerTables;

    int            numPerTables;
    PhyPerTable*   snrPerTables;
    int            numSerTables;
    PhySerTable*   snrSerTables;
    RandomSeed     seed;
    void*          phyVar;

    double         systemLoss_dB;
    Orientation   antennaMountingAngle;
    AntennaModel*  antennaData;
    float* antennaHeight;

    // indicate whether the contention free propagation is enabled
    BOOL           contentionFreeProp;

    void            *nodeLinkLossList;
    void            *nodeLinkDelayList;

    // battery model
    BOOL energyStats;
    EnergyModelType eType;
    PowerCosts*    powerConsmpTable;//added for energy modeling
    LoadProfile*  curLoad;
    EnergyModelGeneric genericEnergyModelParameters;

    double         noiseFactor;  //added for 802.16

    std::vector<MIMO_Data> mimoData;
    int mimoElementCount;

#ifdef ADDON_DB
    // For phy summary table
    std::vector<PhyOneHopNeighborData>* oneHopData;
#endif
    STAT_PhyStatistics* stats;
#ifdef CYBER_LIB
    BOOL isSigintInterface;
#endif

    PhyData() 
    : phyIndex(0), macInterfaceIndex(0), networkAddress(NULL), phyStats(FALSE), channelIndexForTransmission(0), 
      phyModel(PHY_NONE), phyRxModel(RX_MODEL_NONE), phyRxSnrThreshold(0.0), noise_mW_hz(0.0), 
      /* numBerTables(0), snrBerTables(NULL),  */
      d_extSnrBerTables(), numPerTables(0), snrPerTables(NULL),
      numSerTables(0), snrSerTables(NULL),  /* seed,  */ phyVar(NULL), 
      systemLoss_dB(0.0), antennaMountingAngle(), antennaData(NULL), antennaHeight(NULL),
      contentionFreeProp(FALSE), nodeLinkLossList(NULL), nodeLinkDelayList(NULL),
      energyStats(FALSE), eType(NO_ENERGY_MODEL), powerConsmpTable(NULL), curLoad(NULL), 
      genericEnergyModelParameters(), noiseFactor(0.0), mimoData(), mimoElementCount(0), stats(NULL)

#ifdef ADDON_DB
      , 
      oneHopData()
#endif

#ifdef CYBER_LIB
     ,
     jammerStatistics(FALSE), jamInstances(0), jamDuration(0),
     isSigintInterface(FALSE)
#endif

    { ; }
      
};


/// Used by Phy layer to report channel status to mac layer
struct PacketPhyStatus {
    PhyStatusType status;
    clocktype receiveDuration;
    const Message* thePacketIfItGetsThrough;
};

/// Pre-load all the BER files.
///
/// \param nodeInput  structure containing contents of input file
void PHY_GlobalBerInit(NodeInput * nodeInput);

/// Get a pointer to a specific BER table.
///
/// \param tableName  name of the BER file
PhyBerTable* PHY_GetSnrBerTableByName(char* tableName);

/// Get a index of BER table used by PHY.
///
/// \param node  Node pointer
/// \param phyIndex  interface Index
///
/// \return int

int PHY_GetSnrBerTableIndex(Node* node, int phyIndex );

/// Set index of BER table to be used by PHY.
///
/// \param node  Node pointer
/// \param phyIndex  interface Index
///
/// \return int

void PHY_SetSnrBerTableIndex(Node* node, int phyIndex, int index );
/// Pre-load all the PER files.
///
/// \param nodeInput  structure containing contents of input file
void PHY_GlobalPerInit(NodeInput * nodeInput);


/// Get a pointer to a specific PER table.
///
/// \param tableName  name of the PER file
PhyPerTable* PHY_GetSnrPerTableByName(char* tableName);


// SER
/// Pre-load all the SER files.
///
/// \param nodeInput  structure containing contents of input file
void PHY_GlobalSerInit(NodeInput * nodeInput);


/// Get a pointer to a specific SER table.
///
/// \param tableName  name of the SER file
PhySerTable* PHY_GetSnrSerTableByName(char* tableName);

/// Initialize physical layer
///
/// \param node  node being initialized
/// \param nodeInput  structure containing contents of input file
void PHY_Init(
    Node *node,
    const NodeInput *nodeInput);

/// Initialization function for the phy layer
///
/// \param node  node being initialized
/// \param nodeInput  structure containing contents of input file
/// \param interfaceIndex  interface being initialized.
/// \param networkAddress  address of the interface.
/// \param phyModel  Which phisical model is used.
/// \param phyNumber  returned value to be used as phyIndex
void PHY_CreateAPhyForMac(
    Node *node,
    const NodeInput *nodeInput,
    int interfaceIndex,
    Address *networkAddress,
    PhyModel phyModel,
    int* phyNumber);

/// Called at the end of simulation to collect the results of
/// the simulation of the Phy Layer.
///
/// \param node  node for which results are to be collected
void PHY_Finalize(Node *node);

/// Models the behaviour of the Phy Layer on receiving the
/// message encapsulated in msgHdr
///
/// \param node  node which received the message
/// \param msg  message received by the layer
void PHY_ProcessEvent(Node *node, Message *msg);

/// Retrieves the Phy's current status
///
/// \param node  node for which stats are to be collected
/// \param phyNum  interface for which stats are to be collected
///
/// \return status of interface.
PhyStatusType PHY_GetStatus(Node* node, int phyNum);

/// Sets the Radio's transmit power in mW
///
/// \param node  node for which transmit power is to be set
/// \param phyIndex  interface for which transmit power is to be set
/// \param newTxPower_mW  transmit power(mW)
void PHY_SetTransmitPower(
    Node *node,
    int phyIndex,
    double newTxPower_mW);



/// Sets the Radio's Rx SNR Threshold
///
/// \param node  node for which transmit power is to be set
/// \param phyIndex  interface for which transmit power is to be set
/// \param snr  threshold value to be set
void PHY_SetRxSNRThreshold(
    Node *node,
    int phyIndex,
    double snr);

/// Sets the Radio's Data Rate for both Tx and Rx
///
/// \param node  node for which transmit power is to be set
/// \param phyIndex  interface for which transmit power is to be set
/// \param dataRate  dataRate value to be set
void PHY_SetDataRate(
    Node *node,
    int phyIndex,
    Int64 dataRate);

/// For radios that support different Tx and Rx data rates,
/// this will set the Rx Data Rate.  For others, it will
/// call PHY_SetDataRate.
///
/// \param node  node for which transmit power is to be set
/// \param phyIndex  interface for which transmit power is to be set
/// \param dataRate  dataRate value to be set
void PHY_SetTxDataRate(
    Node *node,
    int phyIndex,
    Int64 dataRate);

/// For radios that support different Tx and Rx data rates,
/// this will set the Rx Data Rate.  For others, it will
/// call PHY_SetDataRate.
///
/// \param node  node for which transmit power is to be set
/// \param phyIndex  interface for which transmit power is to be set
/// \param dataRate  dataRate value to be set
void PHY_SetRxDataRate(
    Node *node,
    int phyIndex,
    Int64 dataRate);

/// Gets the pointer to StatsController
///
/// \param node  Pointer to node
/// \param phyIndex  Interface index
/// \return Pointer to StatsController
void* PHY_GetStatsController(Node* node,
                            int phyIndex);

/// Gets the Radio's transmit power in mW.
///
/// \param node  Node that is
///    being instantiated in.
/// \param phyIndex  interface index.
/// \param txPower_mW  transmit power(mW)
void PHY_GetTransmitPower(
    Node *node,
    int phyIndex,
    double *txPower_mW);

/// Get transmission delay
/// based on the first (usually lowest) data rate
/// WARNING: This function call is to be replaced with
/// PHY_GetTransmissionDuration() with an appropriate data rate
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
/// \param size  size of the frame in bytes
///
/// \return transmission delay.
clocktype PHY_GetTransmissionDelay(
    Node *node,
    int phyIndex,
    int size);

/// Get transmission duration of a structured signal fragment.
///
/// \param node  node pointer to node
/// \param phyIndex  interface index.
/// \param dataRateIndex  data rate.
/// \param size  size of frame in bytes.
///
/// \return transmission duration
clocktype PHY_GetTransmissionDuration(
    Node *node,
    int phyIndex,
    int dataRateIndex,
    int size);

/// Get Physical Model
///
/// \param node  node pointer to node
/// \param phyNum  interface index
///
/// \return Physical Model
PhyModel PHY_GetModel(
    Node* node,
    int phyNum);

/// Get Antenna Model type
///
/// \param node  node pointer to node
/// \param phyNum  interface index
///
/// \return Physical Model
AntennaModelType PHY_GetAntennaModelType(
    Node* node,
    int phyNum);

/// Starts transmitting a packet.
///
/// \param node  node pointer to node
/// \param phyNum  interface index
/// \param msg  packet to be sent
/// \param useMacLayerSpecifiedDelay  use delay specified by MAC
/// \param delayUntilAirborne  delay until airborne
void PHY_StartTransmittingSignal(
   Node* node,
   int phyNum,
   Message* msg,
   BOOL useMacLayerSpecifiedDelay,
   clocktype delayUntilAirborne,
   NodeAddress destAddr = ANY_DEST);

/// Starts transmitting a packet.
/// Function is being overloaded
///
/// \param node  node pointer to node
/// \param phyNum  interface index
/// \param msg  packet to be sent
/// \param duration  specified transmission delay
/// \param useMacLayerSpecifiedDelay  use delay specified by MAC
/// \param delayUntilAirborne  delay until airborne
void PHY_StartTransmittingSignal(
   Node* node,
   int phyNum,
   Message* msg,
   clocktype duration,
   BOOL useMacLayerSpecifiedDelay,
   clocktype delayUntilAirborne,
   NodeAddress destAddr = ANY_DEST);

/// Starts transmitting a packet.
///
/// \param node  node pointer to node
/// \param phyNum  interface index
/// \param msg  packet to be sent
/// \param bitSize  specified size of the packet in bits
/// \param useMacLayerSpecifiedDelay  use delay specified by MAC
/// \param delayUntilAirborne  delay until airborne
void PHY_StartTransmittingSignal(
   Node* node,
   int phyNum,
   Message* msg,
   int bitSize,
   BOOL useMacLayerSpecifiedDelay,
   clocktype delayUntilAirborne,
   NodeAddress destAddr = ANY_DEST);

/// Called when a new signal arrives
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
/// \param channelIndex  channel index
/// \param propRxInfo  information on the arrived signal
void PHY_SignalArrivalFromChannel(
   Node* node,
   int phyIndex,
   int channelIndex,
   PropRxInfo *propRxInfo);

/// Called when the current signal ends
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
/// \param channelIndex  channel index
/// \param propRxInfo  information on the arrived signal
void PHY_SignalEndFromChannel(
   Node* node,
   int phyIndex,
   int channelIndex,
   PropRxInfo *propRxInfo);

void PHY_ChannelListeningSwitchNotification(
   Node* node,
   int phyIndex,
   int channelIndex,
   BOOL startListening);

/// Get transmission data rate
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
Int64 PHY_GetTxDataRate(Node *node, int phyIndex);

/// Get reception data rate
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
Int64 PHY_GetRxDataRate(Node *node, int phyIndex);

/// Get transmission data rate type
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
int PHY_GetTxDataRateType(Node *node, int phyIndex);

/// Get transmission data rate type
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
/// \param phyType Rx Phy Type
int PHY_GetRxDataRateType(Node *node, int phyIndex, unsigned char& phyType);

/// Get reception data rate type
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
int PHY_GetRxDataRateType(Node *node, int phyIndex);

/// Set transmission data rate type
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
/// \param  dataRateType  rate of data
/// \param phyType Tx Phy Type
void PHY_SetTxDataRateType(Node *node,
                           int phyIndex,
                           int dataRateType,
                           unsigned char phyType = PHY802_11b);


/// Get the lowest transmission data rate type
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
/// \param dataRateType  rate of data
void PHY_GetLowestTxDataRateType(Node *node, int phyIndex,
                                 int* dataRateType);

/// Set the lowest transmission data rate type
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
void PHY_SetLowestTxDataRateType(Node *node, int phyIndex);

/// Get the highest transmission data rate type
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
/// \param dataRateType  rate of data
void PHY_GetHighestTxDataRateType(Node *node, int phyIndex,
                                  int* dataRateType);

/// Set the highest transmission data rate type
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
void PHY_SetHighestTxDataRateType(Node *node, int phyIndex);

/// Get the highest transmission data rate type for broadcast
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
/// \param dataRateType  rate of data
void PHY_GetHighestTxDataRateTypeForBC(
    Node *node, int phyIndex, int* dataRateType);

/// Set the highest transmission data rate type for broadcast
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
void PHY_SetHighestTxDataRateTypeForBC(
    Node *node, int phyIndex);

/// Compute SINR
///
/// \param phyData  PHY layer data
/// \param signalPower_mW  Signal power
/// \param interferencePower_mW  Interference power
/// \param bandwidth  Bandwidth
///
/// \return Signal to Interference and Noise Ratio
double PHY_ComputeSINR(
    PhyData *phyData,
    double signalPower_mW,
    double interferencePower_mW,
    int bandwidth);

/// Compute Power from the desired signal
/// and interference
///
/// \param node  Node that is being
///    instantiated in
/// \param phyIndex  interface number
/// \param channelIndex  channel index
/// \param msg  message including desired signal
/// \param signalPower_mW  power from the desired signal
/// \param interferencePower_mW  power from interfering signals
void PHY_SignalInterference(
    Node *node,
    int phyIndex,
    int channelIndex,
    Message *msg,
    double *signalPower_mW,
    double *interferencePower_mW,
    int     subChannelIndex = -1);

/// Get BER
///
/// \param phyData  PHY layer data
/// \param berTableIndex  index for BER tables
/// \param sinr  Signal to Interference and Noise Ratio
///
/// \return Bit Error Rate
double PHY_BER(
    PhyData *phyData,
    int berTableIndex,
    double sinr, 
    PhyRxModel rxModel = RX_MODEL_NONE);

// API            :: PHY_PER
// LAYER          :: Physical
// PURPOSE        :: Get PER
///
/// \param phyData  PHY layer data
/// \param perTableIndex  index for PER tables
/// \param sinr  Signal to Interference and Noise Ratio
///
/// \return Packet Error Rate
double PHY_PER(
    PhyData *phyData,
    int perTableIndex,
    double sinr);

/// Get SER
///
/// \param phyData  PHY layer data
/// \param perTableIndex  index for SER tables
/// \param sinr  Signal to Interference and Noise Ratio
///
/// \return Packet Error Rate
double PHY_SER(
    PhyData *phyData,
    int perTableIndex,
    double sinr);


/// Check if it can listen to the channel
///
/// \param node  Node that is being
///    instantiated in
/// \param phyIndex  interface number
/// \param channelIndex  channel index
bool PHY_CanListenToChannel(Node *node, int phyIndex, int channelIndex);

// API           :: PHY_AllowListeningToChannel
// LAYER         :: Physical
// PURPOSE       :: Allow the PHY to listen to specified channel
// PARAMETERS    ::
// + node         : Node*  : Node that is being
//                           instantiated in
// + phyIndex     : int    : interface number
// + channelIndex : int    : channel index
// RETURN        :: void :
void PHY_AllowListeningToChannel(Node* node, int phyIndex, int channelIndex);


// API           :: PHY_DisallowListeningToChannel
// LAYER         :: Physical
// PURPOSE       :: Prevent the PHY from listening to specified channel
//                  this is the default state.
// PARAMETERS    ::
// + node         : Node*  : Node that is being
//                           instantiated in
// + phyIndex     : int    : interface number
// + channelIndex : int    : channel index
// RETURN        :: void :
void PHY_DisallowListeningToChannel(Node* node, int phyIndex, int channelIndex);

// API           :: PHY_StartListeningToChannel
// LAYER         :: Physical
// PURPOSE       :: Start listening to the specified channel
// PARAMETERS    ::
// + node         : Node*  : Node that is being
//                           instantiated in
// + phyIndex     : int    : interface number
// + channelIndex : int    : channel index
// RETURN        :: void :
void PHY_StartListeningToChannel(Node *node, int phyIndex, int channelIndex);

///
///
/// \param node  Node that is being
///    instantiated in
/// \param phyIndex  interface number
/// \param channelIndex  channel index
void PHY_StopListeningToChannel(Node *node, int phyIndex, int channelIndex);

/// Check if it is listening to the channel
///
/// \param node  Node that is being
///    instantiated in
/// \param phyIndex  interface number
/// \param channelIndex  channel index
bool PHY_IsListeningToChannel(Node *node, int phyIndex, int channelIndex);

/// Set the channel index used for transmission
///
/// \param node  Node that is being
///    instantiated in
/// \param phyIndex  interface number
/// \param channelIndex  channel index
void PHY_SetTransmissionChannel(Node *node, int phyIndex, int channelIndex);

/// Get the channel index used for transmission
///
/// \param node  Node that is being
///    instantiated in
/// \param phyIndex  interface number
/// \param channelIndex  channel index
void PHY_GetTransmissionChannel(Node *node, int phyIndex, int *channelIndex);

/// Check if the medium is idle
///
/// \param node  Node that is being
///    instantiated in
/// \param phyNum  interface number
BOOL PHY_MediumIsIdle(Node* node, int phyNum);

/// Check if the medium is idle if sensed directionally
///
/// \param node  Node that is being
///    instantiated in
/// \param phyNum  interface number
/// \param azimuth  azimuth (in degrees)
BOOL PHY_MediumIsIdleInDirection(Node* node, int phyNum, double azimuth);

/// Set the sensing direction
///
/// \param node  Node that is being
///    instantiated in
/// \param phyNum  interface number
/// \param azimuth  azimuth (in degrees)
void PHY_SetSensingDirection(Node* node, int phyNum, double azimuth);

/// Start transmitting a signal directionally
///
/// \param node  Node that is
///    being instantiated in
/// \param phyNum  interface number
/// \param msg  signal to transmit
/// \param useMacLayerSpecifiedDelay  use delay specified by MAC
/// \param delayUntilAirborne  delay until airborne
/// \param directionAzimuth  azimuth to transmit the signal
void PHY_StartTransmittingSignalDirectionally(
   Node* node,
   int phyNum,
   Message* msg,
   BOOL useMacLayerSpecifiedDelay,
   clocktype delayUntilAirborne,
   double directionAzimuth);

/// Lock the direction of antenna
///
/// \param node  Node that is being
///    instantiated in
/// \param phyNum  interface number
void PHY_LockAntennaDirection(
   Node* node,
   int phyNum);

/// Unlock the direction of antenna
///
/// \param node  Node that is being
///    instantiated in
/// \param phyNum  interface number
void PHY_UnlockAntennaDirection(
   Node* node,
   int phyNum);

/// Get the AOA of the last signal
///
/// \param node  Node that is being
///    instantiated in
/// \param phyNum  interface number
///
/// \return AOA
double PHY_GetLastSignalsAngleOfArrival(Node* node, int phyNum);


/// Terminate the current signal reception
///
/// \param node  Node pointer that the
///    protocol is being
///    instantiated in
/// \param phyNum  interface number
/// \param terminateOnlyOnReceiveError  terminate only when
///    the error happened
/// \param receiveError  if error happened
/// \param endSignalTime  end of signal
void PHY_TerminateCurrentReceive(
   Node* node, int phyNum, const BOOL terminateOnlyOnReceiveError,
   BOOL*   receiveError, clocktype* endSignalTime);

/// Calculates an estimated radio range for the PHY.
/// Supports only TWO-RAY and FREE-SPACE.
///
/// \param txnode  the Tx node of interest
/// \param node  the Rx node of interest
/// \param txInterfaceIndex  the interface for the TX node
/// \param interfaceIndex  the interface for the Rx node
/// \param channnelIndex  the index of the channel
/// \param printAll  if TRUE, prints the range for all data
///    rates, otherwise returns the longest.
///
/// \return the range in meters
double PHY_PropagationRange(Node* txnode,
                            Node* node,
                            int   txInterfaceIndex,
                            int   interfaceIndex,
                            int   channnelIndex,
                            BOOL  printAllDataRates);

/// This function declares energy model variables and initializes them.
/// Moreover, the function read energy model specifications and configures
/// the parameters which are configurable.
///
/// \param node  the node of interest.
/// \param phyIndex  the PHY index.
/// \param nodeInput  the node input.
void
ENERGY_Init(Node *node,
            const int phyIndex,
            const NodeInput *nodeInput);

/// To print the statistic of Energy Model.
///
/// \param node  the node of interest.
/// \param phyIndex  the PHY index.
void
ENERGY_PrintStats(Node *node,
                  const int phyIndex);

/// This function should be called whenever a state transition occurs
/// in any place in PHY layer. As input parameters, the function reads the current
/// state and the new state of PHY layer and based on the new sates calculates the cost
/// of the load that should be taken off the battery.
/// The function then interacts with battery model and updates the charge of battery.
///
/// \param node  the node of interest.
/// \param phyIndex  the PHY index.
/// \param prevStatus  the the previous status.
/// \param newStatus  the the new status.
void
Phy_ReportStatusToEnergyModel(Node* node,
                            const int phyIndex,
                            PhyStatusType prevStatus,
                            PhyStatusType newStatus);

/// To update the current load of generic energy model.
///
/// \param node  the node of interest.
/// \param phyIndex  the PHY index.
void
Generic_UpdateCurrentLoad(Node* node, const int phyIndex);

// helper function for PHY connectivity table.
BOOL PHY_ProcessSignal(Node* node,
                       int phyIndex,
                       PropRxInfo* propRxInfo,
                       double rxPower_dBm);

double PHY_GetTxPower(Node* node, int phyIndex);

/// To Notify the StatsDB module and other modules of the packet dropping event.
///
/// \param node  the node of interest.
/// \param phyIndex  the PHY index.
///    + channelIndex : int : the channelIndex
/// \param msg  The dropped message
/// \param dropType  the reason for the drop
/// \param rxPower_mW  receving power of the signal
/// \param interferencePower_mW  interference power of the signal
///    + passloss_dB : double : pathloss value of the signal
void PHY_NotificationOfPacketDrop(Node* node,
                                  int phyIndex,
                                  int channelIndex,
                                  const Message* msg,
                                  const char* dropType,
                                  double rxPower_mW,
                                  double interferencePower_mW,
                                  double pathloss_dB);

/// To Notify the StatsDB module and other modules of the signal received event .
///
/// \param node  the node of interest.
/// \param phyIndex  the PHY index.
///    + channelIndex : int : the channelIndex
/// \param msg  The dropped message
/// \param rxPower_mW  receving power of the signal
/// \param interferencePower_mW  interference power of the signal
/// \param passloss_dB  pathloss value of the signal
///    + controlSize : int : size of control header
void PHY_NotificationOfSignalReceived(Node* node,
                                      int phyIndex,
                                      int channelIndex,
                                      const Message* msg,
                                      double rxPower_mW,
                                      double interferencePower_mW,
                                      double pathloss_dB,
                                      int controlSize);

void PHY_Reset(Node* node, int interfaceIndex);

/// Gets the current steering angle for a directional antenna
/// from PHY models that support this.
///
/// \param node  node being used
/// \param phyIndex  physical to be initialized
Orientation PHY_GetSteeringAngle(Node* node,
                                 int   phyIndex);

/// To get the bandwidth for the given PHY model.
///
///
/// \param node  The node of interest.
/// \param phyIndex  The PHY index.
///
/// \return The bandwidth
double PHY_GetBandwidth(
                Node *node,
                int phyIndex);


/// To get the frequency for the given signal.
///
///
/// \param node  The node of interest.
/// \param channelIndex  Index of the propagation channel
///
/// \return The frequency
double PHY_GetFrequency(
                Node *node,
                int channelIndex);


/// To get the PhyModel for the node.
///
///
/// \param node  The node of interest.
/// \param phyIndex  The PHY index.
///
/// \return The PhyModel
PhyModel  PHY_GetPhyModel(
                Node *node,
                int phyIndex);


/// To get the name of a phy model
///
///
/// \param node  The node of interest.
/// \param phyIndex  The PHY index.
///
/// \return The name of the phy model
std::string& PHY_GetPhyName(
                Node *node,
                int phyIndex);

/// To check if the signal feature matches the receiver's phyModel.
///
///
/// \param node  The node of interest.
/// \param phyIndex  The PHY index.
///
/// \return if the signal feature matches the receiver's phyModel

BOOL PHY_isSignalFeatureMatchReceiverPhyModel(
                    Node *node,              //receiver node
                    int phyIndex,            // receiver phy index
                    PhyModel signalPhyModel); // signal’s PHY model

/// To estimate the inband signal power for given signal and receiver parameters.
///
///
/// \param signalPower_mW  The signal power in mW.
/// \param signalFrequency  The signal frequency in Hz.
/// \param signalBandwidth  The signal bandwidth in Hz
/// \param rxFrequency  The receiver frequency in Hz
/// \param rxBandwidth  The receiver bandwidth in Hz
///
/// \return The inband signal power in mW

double PHY_ComputeInbandPower(
    double signalPower_mW,  // signal power
    double signalFrequency, // signal frequency
    double signalBandwidth, // signal bandwidth
    double rxFrequency,     // receiver frequency
    double rxBandwidth);     // receiver bandwidth


/// Called when a interference signal arrives
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
/// \param channelIndex  channel index
/// \param propRxInfo  information on the arrived signal
/// \param sigPower_mW  The inband interference power in mW

void PHY_InterferenceArrivalFromChannel(
   Node* node,
   int phyIndex,
   int channelIndex,
   PropRxInfo *propRxInfo,
   double sigPower_mW);

/// Called when a interference signal ends
///
/// \param node  node pointer to node
/// \param phyIndex  interface index
/// \param channelIndex  channel index
/// \param propRxInfo  information on the arrived signal
/// \param sigPower_mW  The inband interference power in mW

void PHY_InterferenceEndFromChannel(
   Node* node,
   int phyIndex,
   int channelIndex,
   PropRxInfo *propRxInfo,
   double sigPower_mW);

/*********HT START*************************************************/

struct MAC_PHY_TxRxVector;

// /** PHY_SetTxVector()
// * Setting the txVector for outgoing packet
void PHY_SetTxVector(Node* node,
                     int phyIndex,
                     MAC_PHY_TxRxVector& txVector);

// /** PHY_GetRxVector()
// * Getting the rxVector of the incoming packet
void PHY_GetRxVector(Node* node,
                     int phyIndex,
                     MAC_PHY_TxRxVector& rxVector);

// /** PHY_GetTxVector()
// * Getting the last set txVector
void PHY_GetTxVector(Node* node,
                     int phyIndex,
                     MAC_PHY_TxRxVector& txVector);

// /** PHY_GetTxVectorForBC()
// * Getting the txVector for broadcast messages
void PHY_GetTxVectorForBC(Node* node,
                     int phyIndex,
                     MAC_PHY_TxRxVector& txVector);

// /** PHY_GetTxVectorForHighestDataRate()
// * Getting the txVector for highest data rate supported at this PHY
void PHY_GetTxVectorForHighestDataRate(Node* node,
                     int phyIndex,
                     MAC_PHY_TxRxVector& txVector);

// /** PHY_GetTransmissionDuration()
// * Getting the transmission duration based on txVector supplied
clocktype PHY_GetTransmissionDuration(Node* node,
                     int phyIndex,
                     MAC_PHY_TxRxVector& txVector);

// /** PHY_GetTxVectorForLowestDataRate()
// * Getting the txVector for lowest data rate supported at this PHY
void PHY_GetTxVectorForLowestDataRate(Node* node,
                     int phyIndex,
                     MAC_PHY_TxRxVector& txVector);

// /** PHY_IsPhyHTEnabled()
// * Getting the HT capability of this PHY
BOOL PHY_IsPhyHTEnabled(Node* node,
                        int phyIndex);
BOOL PHY_IsPhyVHTEnabled(Node *node,
                        int phyIndex);
/// To get the name of the channel.
///
///
/// \param node  The node of interest.
/// \param channelIndex  Index of the propagation channel
///
/// \return The name of the channel.
std::string& PHY_GetChannelName(Node *node, int channelIndex);

/*********HT END****************************************************/

/// To get the channel index.
/// 
///
/// \param node           :  The node of interest.
/// \param channelName    :: std:  The name of the channel.
///
/// \return Channel index.
Int32 PHY_GetChannelIndexForChannelName(Node* node, const char* channelName);

/// To check whether channelName exist or not.
/// 
///
/// \param node           :  The node of interest.
///    +channelName     :: std::string  : The name of the channel.
///
/// \return TRUE if channelName is valid
/// False if channel name is invalid
BOOL PHY_ChannelNameExists(Node* node, const char*  channelName);

/// \brief To get the number of configured antenna elements
///
/// Return the number of configured antennas at the physical layer
///
/// \param node           :  The node of interest.
/// \param phyIndex       :  Physical layer index
///
/// \return number of configured antenna elements
int PHY_GetNumConfigAntennas(Node* node, int phyIndex);


/// \brief To get the number of active antenna elements
///
/// Return the number of active antennas at the physical layer
///
/// \param node           :  The node of interest.
/// \param phyIndex       :  Physical layer index
///
/// \return number of active antenna elements
int PHY_GetNumActiveAntennas(Node* node, int phyIndex);

/// \brief To tune the radio of wifi phy models to the channel
/// specified by radioOverlayId
///
/// \param node           :  The node of interest.
/// \param phyIndex       :  Physical layer index.
void PHY_TuneRadio(Node* node, int phyIndex);

/// \brief To get frequency for Radio Range utility
///
/// \param node           :  The node of interest.
/// \param chIndex        :  Channel index.
/// \param phyIndex       :  Physical layer index.
double PHY_GetFrequencyForRadioRange(Node* node, int chIndex, int phyIndex);

#endif /* PHY_H */
