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

/// \defgroup Package_PHY_CELLULAR PHY_CELLULAR

/// \file
/// \ingroup Package_PHY_CELLULAR
/// Defines the data structures used in the CELLULAR Model
/// Most of the time the data structures and functions start
/// with CellularPhy**

#ifndef _PHY_CELLULAR_H_
#define _PHY_CELLULAR_H_

#include "cellular.h"

#ifdef UMTS_LIB
//UMTS
#include "cellular_umts.h"
#include "phy_umts.h"
#endif


/// Enlisted different Cellular Protocols
typedef enum
{
    Cellular_ABSTRACT_Phy = 0,
    Cellular_GSM_Phy,
    Cellular_GPRS_Phy,
    Cellular_UMTS_Phy,
    Cellular_CDMA2000_Phy,
    Cellular_WIMAX_Phy
}CellularPhyProtocolType;

/// Cellular PHY Data
typedef struct
{
    // Must be set at initialization for every Cellular node
    PhyData* thisPhy;
    RandomSeed randSeed;
    CellularNodeType nodeType;
    CellularPhyProtocolType cellularPhyProtocolType;

#ifdef UMTS_LIB
    // UMTS
    PhyUmtsData* phyUmtsData;
#endif


    BOOL collectStatistics;
} PhyCellularData;

//--------------------------------------------------------------------------
//  Key API functions of the CELLULAR PHY
//--------------------------------------------------------------------------

/// Initialize the Cellular PHY
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param nodeInput  Pointer to the node input
///
void PhyCellularInit(Node *node,
                     const int phyIndex,
                     const NodeInput *nodeInput);

/// Finalize the CELLULAR PHY, print out statistics
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyCellularFinalize(Node *node, const int phyIndex);

/// Start transmitting a frame
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param packet  Frame to be transmitted
/// \param clocktype  Duration of the transmission
///    + useMacLayerSpecifiedDelay : Use MAC specified delay or calculate it
/// \param initDelayUntilAirborne  The MAC specified delay
///
void PhyCellularStartTransmittingSignal(
         Node* node,
         int phyIndex,
         Message* packet,
         clocktype duration,
         BOOL useMacLayerSpecifiedDelay,
         clocktype initDelayUntilAirborne);

/// End of the transmission
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyCellularTransmissionEnd(
         Node* node,
         int phyIndex);

/// Handle signal arrival from the chanel
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param channelIndex  Index of the channel receiving signal from
/// \param propRxInfo  Propagation information
///
void PhyCellularSignalArrivalFromChannel(
         Node* node,
         int phyIndex,
         int channelIndex,
         PropRxInfo* propRxInfo);

/// Handle signal end from a chanel
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param channelIndex  Index of the channel receiving signal from
/// \param propRxInfo  Propagation information
///
void PhyCellularSignalEndFromChannel(
         Node* node,
         int phyIndex,
         int channelIndex,
         PropRxInfo* propRxInfo);

/// Get the status of the PHY.
///
/// \param node  Pointer to node.
/// \param phyNum  Index of the PHY
///
/// \return Status of the PHY
PhyStatusType
PhyCellularGetStatus(
    Node* node,
    int phyNum);

/// Set the transmit power of the PHY
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param txPower_mW  Transmit power in mW unit
///
void PhyCellularSetTransmitPower(
         Node* node,
         PhyData* thisPhy,
         double newTxPower_mW);

/// Get the transmit power of the PHY
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param txPower_mW  Transmit power in mW unit to be return
///
void PhyCellularGetTransmitPower(
         Node* node,
         PhyData* thisPhy,
         double* txPower_mW);

/// Get the raw data rate
///
/// \param node  Pointer to node.
/// \param thisPhy  Pointer to PHY structure
///
/// \return Data rate in bps
int PhyCellularGetDataRate(
        Node* node,
        PhyData* thisPhy);
/// Get the Rx level
///
/// \param node  Pointer to node.
/// \param thisPhy  Pointer to PHY structure
///
/// \return Rx level
float PhyCellularGetRxLevel(
          Node* node,
          int phyIndex);

/// Get the raw data rate
///
/// \param node  Pointer to node.
/// \param thisPhy  Pointer to PHY structure
///
/// \return Data rate in bps
double PhyCellularGetRxQuality(
           Node* node,
           int phyIndex);

/// Handle signal arrival from the chanel
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param channelIndex  Index of the channel receiving signal from
/// \param propRxInfo  Propagation information
/// \param sigPower_mW  The inband interference signal power
///
void PhyCellularInterferenceArrivalFromChannel(
         Node* node,
         int phyIndex,
         int channelIndex,
         PropRxInfo* propRxInfo,
         double sigPower_mW);


// FUNCTION   :: PhyCellularInterferenceEndFromChannel
// LAYER      :: PHY
// PURPOSE    :: Handle signal end from a chanel
// PARAMETERS ::
// + node         : Node* : Pointer to node.
// + phyIndex     : int   : Index of the PHY
// + channelIndex : int   : Index of the channel receiving signal from
// + propRxInfo   : PropRxInfo* : Propagation information
// RETURN     :: void : NULL
void PhyCellularInterferenceEndFromChannel(
         Node* node,
         int phyIndex,
         int channelIndex,
         PropRxInfo* propRxInfo);

#endif /* _PHY_CELLULAR_H_ */
