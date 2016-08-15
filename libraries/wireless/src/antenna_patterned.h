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

#ifndef ANTENNA_PATTERNED_H
#define ANTENNA_PATTERNED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "antenna_global.h"

/// Const for azimuth index of Patterned antenna

#define PATTERNED_AZIMUTH_INDEX 0


/// Const for elevation index of Patterned antenna

#define PATTERNED_ELEVATION_INDEX 1


/// Structure for Patterned antenna

typedef struct struct_Antenna_Patterned {
    int                     modelIndex;
    int                     numPatterns;
    int                     patternIndex;
    float             antennaHeight;
    float             antennaGain_dB;
    AntennaPattern          *pattern;
} AntennaPatterned;

/// Init the Patterned antennas from
/// configuration file.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical
///    to be initialized
/// \param nodeInput  Pointer to NodeInput structure
///

void ANTENNA_PatternedInitFromConfigFile(
     Node* node,
     int phyIndex,
     const NodeInput* nodeInput);

/// initialization of patterned antenna.
///
/// \param node  Pointer to Node.
/// \param nodeInput  Pointer to NodeInput structure
/// \param phyIndex  interface for which physical to be initialized
/// \param antennaModel  pointer to global
///    antenna model
///

void ANTENNA_PatternedInit(
     Node* node,
     const NodeInput* nodeInput,
     int phyIndex,
     const AntennaModelGlobal* antennaModel);


/// Is using default pattern.
///
/// \param phyData  pointer to the struct PhyData
///
/// \return Return TRUE if antenna pattern index
/// is ANTENNA_DEFAULT_PATTERN

BOOL AntennaPatternedUsingDefaultPattern(
     PhyData* phyData);


/// Set the current pattern.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param patternIndex  index of the pattern which is set
///

void AntennaPatternedSetPattern(
     Node *node,
     int phyIndex,
     int patternIndex);


/// Set the current pattern.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param patternIndex  index of the pattern which is set
///

void AntennaPatternedGetPattern(
     Node *node,
     int phyIndex,
     int *patternIndex);


/// Return the current pattern Index.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
///
/// \return Return the patternIndex

int AntennaPatternedReturnPatternIndex(
    Node* node,
    int phyIndex);


/// Return gain for this direction.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param DOA  dirrection of arrival of current
///    signal
///
/// \return Return the antennaGain

float AntennaPatternedGainForThisDirection(
      Node *node,
      int phyIndex,
      Orientation DOA);


/// Return gain for current pattern index.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param patternIndex  index of the pattern which is set
/// \param DOA  dirrection of arrival of current
///    signal
///
/// \return Return the antennaGain

float AntennaPatternedGainForThisDirectionWithPatternIndex(
      Node *node,
      int phyIndex,
      int patternIndex,
      Orientation DOA);


/// Return gain for pattern index.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param propRxInfo  pointer to PropRxInfo
/// \param patternIndex  index of the pattern
/// \param gain_dBi  contains the gain value.
///

void AntennaPatternedMaxGainPatternForThisSignal(
    Node* node,
    int phyIndex,
    PropRxInfo* propRxInfo,
    int* patternIndex,
    float* gain_dBi);


/// Set best pattern for azimith.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param azimuthAngle  azimuth angle
///

void AntennaPatternedSetBestPatternForAzimuth(
     Node *node,
     int phyIndex,
     double azimuthAngle);


/// Set best pattern for azimith.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param azimuthAngle  azimuth angle
///

void ANTENNA_PatternedSetBestPatternForAzimuth(
     Node* node,
     int phyIndex,
     double azimuthAngle);


/// Return gain for a particular direction.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param DOA  azimuth & elvation angle
///
/// \return Return antennaGain

float ANTENNA_PatternedGainForThisDirection(
      Node* node,
      int phyIndex,
      Orientation DOA);


// inlines //

/// Lock antenna dirrection.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
inline
void AntennaPatternedLockAntennaDirection(
     Node *node,
     int phyIndex)
{
    PhyData *phyData = node->phyData[phyIndex];
    ERROR_Assert(phyData->antennaData->antennaModelType ==
        ANTENNA_PATTERNED ,
            "antennaModelType not patterned.\n");
}


/// UnLock antenna dirrection.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
inline
void AntennaPatternedUnlockAntennaDirection(
     Node *node,
     int phyIndex)
{
    PhyData *phyData = node->phyData[phyIndex];
    ERROR_Assert(phyData->antennaData->antennaModelType ==
        ANTENNA_PATTERNED ,
            "antennaModelType not patterned.\n");
}


/// Lock antenna.
///
/// \param node  node being initialized.
/// \param phyIndex  interface for which physical to be
///    initialized.
///
/// \return Return TRUE if antenna is locked
inline
BOOL AntennaPatternedIsLocked(
     Node *node,
     int phyIndex)
{
    PhyData *phyData = node->phyData[phyIndex];
    ERROR_Assert(phyData->antennaData->antennaModelType ==
        ANTENNA_PATTERNED ,
            "antennaModelType not patterned.\n");

    return FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /* ANTENNA_PATTERNED_H */

