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

/// \defgroup Package_ANTENNA ANTENNA

/// \file
/// \ingroup Package_ANTENNA
/// This file describes data structures and functions used by antenna models.

// PROTOCOL     :: Antenna
// SUMMARY      :: This protocol deals with the Antenna signalling and
// communication
// LAYER        :: Physical Layer
// STATISTICS   :: None
// CONFIG_PARAM :: ANTENNA-MODEL:  OMNIDIRECTIONAL
// SWITCHED-BEAM
// STEERABLE
// PATTERNED
//
// ANTENNA-AZIMUTH-PATTERN-FILE ./default.antenna-azimuth
// ANTENNA-ELEVATION-PATTERN-FILE ./default.antenna-elevation
// ANTENNA-MODEL-CONFIG-FILE      ./default.antenna-models
// ANTENNA-GAIN             0.0
// ANTENNA-EFFICIENCY       0.8
// ANTENNA-MISMATCH-LOSS    0.3
// ANTENNA-CABLE-LOSS       0.0
// ANTENNA-CONNECTION-LOSS  0.2
// ANTENNA-HEIGHT           1.5
// PHY802.11-ESTIMATED-DIRECTIONAL-ANTENNA-GAIN 15.0
//
// VALIDATION   :: None
// IMPLEMENTED_FEATURES :: omnidirectional, switched beam , steerable
// antenna and patterned
// OMITTED_FEATURES :: None
// ASSUMPTIONS  :: pre-generated antenna pattern files are required.
// STANDARD     :: None
// CONFIG_PARAM ::
// MAC-802.11-DIRECTIONAL-ANTENNA-MODE                 YES |
// NO
// MAC-802.11-DIRECTION-CACHE-EXPIRATION-TIME          2S
// MAC-802.11-DIRECTIONAL-NAV-AOA-DELTA-ANGLE          37.0
// MAC-802.11-DIRECTIONAL-SHORT-PACKET-TRANSMIT-LIMIT  4
//


#ifndef ANTENNA_H
#define ANTENNA_H

#include "antenna_global.h"

/// Default height of the antenna
#define ANTENNA_DEFAULT_HEIGHT                           1.5

/// Default gain of the antenna
#define ANTENNA_DEFAULT_GAIN_dBi                         0.0

/// Default efficiency of the antenna
#define ANTENNA_DEFAULT_EFFICIENCY                       0.8

/// Default mismatch loss of the antenna
#define ANTENNA_DEFAULT_MISMATCH_LOSS_dB                 0.3

/// Default connection loss of the antenna
#define ANTENNA_DEFAULT_CONNECTION_LOSS_dB               0.2

/// Default cable loss of the antenna
#define ANTENNA_DEFAULT_CABLE_LOSS_dB                    0.0

/// Default minimum gain of the antenna
#define ANTENNA_LOWEST_GAIN_dBi                         -10000.0

/// Default Pattern
#define ANTENNA_DEFAULT_PATTERN                 0

/// OMNIDIRECTIONAL PATTERN
#define ANTENNA_OMNIDIRECTIONAL_PATTERN                 -1

/// Const for Pattern of antenna not set
#define ANTENNA_PATTERN_NOT_SET                         -2

/// Const for azimuth index of antenna Pattern
#define AZIMUTH_INDEX                   0

/// Const for elevation index of antenna Pattern
#define ELEVATION_INDEX                 1

/// Const for the line number in the antennaModelInput
#define MAX_ANTENNA_NUM_LINES           30

/// Const for the memory allocation of azimuth and elevation
/// gain array.
#define AZIMUTH_ELEVATION_INDEX         2

/// Const represents the basic pattern
/// starting point in NSMA file
#define NSMA_PATTERN_START_LINE_NUMBER        10

/// Const represents the Revised pattern
/// max line number where the revised NSMA 
/// pattern can start.
#define NSMA_MAX_STARTLINE                   41


/// Initialize antennas.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param nodeInput  structure containing contents of input
///    file.
///

void ANTENNA_Init(
     Node* node,
     int   phyIndex,
     const NodeInput* nodeInput);


/// Read in the azimuth pattern file.
///
/// \param node  node being used.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param antennaInput  structure containing contents of
///    input file.
/// \param numPatterns  contains the number of patterns
///    in the pattern file.
/// \param steerablePatternSetRepeatSectorAngle  contains
///    PatternSetRepeatSectorAngle
///    for steerable antenna.
/// \param pattern_dB  array used to store the gain values
///    of the pattern file.
/// \param azimuthPlane  shows whether the file is azimuth
///    file or elevation file.
///

void ANTENNA_ReadPatterns(
     Node* node,
     int phyIndex,
     const NodeInput* antennaInput,
     int* numPatterns,
     int* steerablePatternSetRepeatSectorAngle,
     float*** pattern_dB,
     BOOL azimuthPlane);


/// Read in the NSMA pattern file.
///
/// \param node  node being used.
/// \param phyIndex  interface for which physical
///    to be initialized.
/// \param antennaInput  structure containing contents of
///    input file.
/// \param numPatterns  number of patterns in the file.
/// \param azimuthPattern_dB  pattern_dB array for
///    azimuth gains.
/// \param azimuthResolution  azimuth resolution
///    and azimuth range.
/// \param elevationPattern_dB  pattern_dB array
///    for elevation gains.
/// \param elevationResolution  elevation resolution
///    and elevation range.
/// \param version  version of NSMA pattern
///

void ANTENNA_ReadNsmaPatterns(
     Node* node,
     int phyIndex,
     const NodeInput* antennaInput,
     int numPatterns,
     float*** azimuthPattern_dB,
     int* azimuthResolution,
     float*** elevationPattern_dB,
     int* elevationResolution,
     NSMAPatternVersion* version);

/// Read in the Revised NSMA pattern file.
///
/// \param node  node being used.
/// \param phyIndex  interface for which physical
///    to be initialized.
/// \param antennaInput  structure containing contents of
///    input file.
/// \param numPatterns  number of patterns in the file.
/// \param azimuthPattern_dB  pattern_dB array for
///    azimuth gains.
/// \param aziRatio  the ratio of azimuth resolution
///    and azimuth range.
/// \param elevationPattern_dB  pattern_dB array
///    for elevation gains.
/// \param elvRatio  the ratio of elevation resolution
///    and elevation range.
///

void ANTENNA_ReadRevisedNsmaPatterns(
     Node* node,
     int phyIndex,
     const NodeInput* antennaInput,
     int numPatterns,
     float*** azimuthPattern_dB,
     float aziRatio,
     float*** elevationPattern_dB,
    float elvRatio);


/// Used to read ASCII 3D pattern file.
///
/// \param node  node being used.
/// \param phyIndex  interface for which physical
///    to be initialized.
/// \param antennaInput  structure containing contents of
///    input file.
/// \param antennaPatterns  Pointer to Global Antenna Pattern
///    structure.

void ANTENNA_Read3DAsciiPatterns(
     Node* node,
     int phyIndex,
     const NodeInput* antennaInput,
     AntennaPattern* antennaPatterns);


/// Used to read ASCII 2D pattern file.
///
/// \param node  node being used.
/// \param phyIndex  interface for which physical
///    to be initialized.
/// \param antennaInput  structure containing contents of
///    input file.
/// \param antennaPatterns  Pointer to Global Antenna Pattern
///    structure.
/// \param azimuthPlane  A boolean variable to differentiate the file
///    azimuth or elevation.
/// \param conversionParameter  conversion parameter to change
///    the dB values in dBi.

void ANTENNA_Read2DAsciiPatterns(
     Node* node,
     int phyIndex,
     const NodeInput* antennaInput,
     AntennaPattern* antennaPatterns,
     BOOL azimuthPlane,
     const float conversionParameter);


/// Initialize omnidirectional antenna
/// from the antenna model file.
///
/// \param node  node being initialized.
/// \param nodeInput  pointer to node input
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param antennaModel  pointer to AntennaModelGlobal
///    structure.
///

void ANTENNA_OmniDirectionalInit(
     Node* node,
     const NodeInput* nodeInput,
     int phyIndex,
     const  AntennaModelGlobal* antennaModel);


// PURPOSE : Initialize omnidirectional antenna
/// from the default.config file.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param nodeInput  structure containing contents of input
///    file.
///

void ANTENNA_OmniDirectionalInitFromConfigFile(
     Node* node,
     int phyIndex,
     const NodeInput* nodeInput);



// PURPOSE : Initialize antenna from the default.config file.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param nodeInput  structure containing contents of input
///    file.
///

void ANTENNA_InitFromConfigFile(
     Node* node,
     int phyIndex,
     const NodeInput* nodeInput);


/// Is antenna in omnidirectional mode.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be use
///
/// \return returns TRUE if antenna is in
/// omnidirectional mode

BOOL ANTENNA_IsInOmnidirectionalMode(
    Node *node,
    int phyIndex);

/// Return nodes current pattern index.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to use
///
/// \return returns pattern index

int ANTENNA_ReturnPatternIndex(
    Node* node,
    int phyIndex);

/// Return nodes antenna height.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
///
/// \return height in meters

float* ANTENNA_ReturnHeightPtr(Node* node, int phyIndex);
#ifdef NODE_H
inline float ANTENNA_ReturnHeight(
     Node* node,
     int phyIndex) {
  PhyData* p = node->phyData[phyIndex];
  if (p->antennaHeight == NULL) p->antennaHeight = ANTENNA_ReturnHeightPtr(node, phyIndex);
  return *(p->antennaHeight);
}
#endif

/// Return systen loss in dB.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
///
/// \return loss in dB

double ANTENNA_ReturnSystemLossIndB(
       Node* node,
       int phyIndex);

/// Return gain for this direction in dB.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
/// \param DOA  direction of antenna
///
/// \return gain in dB

float ANTENNA_GainForThisDirection(
      Node* node,
      int phyIndex,
      Orientation DOA);


/// Return gain for this direction for the specified pattern in
/// dB.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
/// \param patternIndex  pattern index to use
/// \param DOA  direction of antenna
///
/// \return gain in dB

float ANTENNA_GainForThisDirectionWithPatternIndex(
      Node* node,
      int phyIndex,
      int patternIndex,
      Orientation DOA);


/// Return gain in dB.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
/// \param propRxInfo  receiver propagation info
///
/// \return gain in dB

float ANTENNA_GainForThisSignal(
      Node* node,
      int phyIndex,
      PropRxInfo* propRxInfo);


/// Return default gain in dB.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
/// \param propRxInfo  receiver propagation info
///
/// \return gain in dB

float ANTENNA_DefaultGainForThisSignal(
      Node* node,
      int phyIndex,
      PropRxInfo *propRxInfo);


/// Lock antenna to current direction.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
///

void ANTENNA_LockAntennaDirection(
     Node* node,
     int phyIndex);


/// Unlock antenna.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
///

void ANTENNA_UnlockAntennaDirection(
     Node* node,
     int phyIndex);


/// Return if direction antenna is locked.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
///
/// \return returns TRUE if the antenna direction is
/// locked

BOOL ANTENNA_DirectionIsLocked(
    Node *node,
    int phyIndex);

/// Return if antenna is locked.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
///
/// \return Returns TRUE if antenna is locked.

BOOL ANTENNA_IsLocked(
    Node *node,
    int phyIndex);

/// Set default antenna mode (usally omni).
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
///

void ANTENNA_SetToDefaultMode(
     Node* node,
     int phyIndex);


/// Set antenna for best gain using the Rx info.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
/// \param propRxInfo  receiver propagation info
///

void ANTENNA_SetToBestGainConfigurationForThisSignal(
     Node* node,
     int phyIndex,
     PropRxInfo* propRxInfo);


/// Set antenna for best gain using the azimuth.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
/// \param azimuth  the azimuth
///

void ANTENNA_SetBestConfigurationForAzimuth(
     Node* node,
     int phyIndex,
     double azimuth);

/// Get steering angle of the antenna.
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
/// \param angle  For returning the angle
///

void ANTENNA_GetSteeringAngle(
     Node* node,
     int phyIndex,
     Orientation* angle);

/// Set the steering angle of the antenna
///
/// \param node  node being used
/// \param phyIndex  interface for which physical to be used
/// \param angle  Steering angle to be
///

void ANTENNA_SetSteeringAngle(
     Node* node,
     int phyIndex,
     Orientation angle);

/// Read in the ASCII pattern .
///
/// \param node  node being used
/// \param phyIndex  interface for which physical
///    to be initialized
/// \param antennaModelInput  structure containing
///    contents of input
///    file
/// \param antennaPatterns  Pointer to the global
///    antenna pattern structure.
///

void ANTENNA_ReturnAsciiPatternFile(
     Node* node,
     int phyIndex,
     const NodeInput* antennaModelInput,
     AntennaPattern*  antennaPatterns);


/// Read in the NSMA pattern .
///
/// \param node  node being used
/// \param phyIndex  interface for which
///    physical to be initialized
/// \param antennaModelInput  structure containing
///    contents of input
///    file
/// \param antennaPatterns  Pointer to
///    the global antenna
///    pattern structure.
///

void ANTENNA_ReturnNsmaPatternFile(
     Node* node,
     int  phyIndex,
     const NodeInput* antennaModelInput,
     AntennaPattern*  antennaPatterns);


/// Used to read Qualnet Traditional pattern file
///
/// \param node  node being used
/// \param phyIndex  interface for which
///    physical to be initialized
/// \param antennaModelInput  structure containing
///    contents of input
///    file
/// \param antennaPatterns  Pointer to
///    the global antenna
///    pattern structure.
///

void ANTENNA_ReturnTraditionalPatternFile(
     Node* node,
     int phyIndex,
     const NodeInput* antennaModelInput,
     AntennaPattern* antennaPatterns);


/// Reads the antenna configuration parameters into
/// the NodeInput structure.
///
/// \param node  node being used
///    physical to be initialized
/// \param buf  Path to input file.
///
/// \return pointer to nodeInput structure

NodeInput* ANTENNA_MakeAntennaModelInput(
           const NodeInput* nodeInput,
           char* buf);

#endif
