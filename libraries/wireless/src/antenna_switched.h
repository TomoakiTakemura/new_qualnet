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

#ifndef ANTENNA_SWITCHED_H
#define ANTENNA_SWITCHED_H

#include "antenna.h"

#define SWITCHED_AZIMUTH_INDEX 0
#define SWITCHED_ELEVATION_INDEX 1


/// This structure contains the data pertaining to 
/// Switched beam Antenna. The structure is assigned to 
/// the phydata's (void*) antennaVar


typedef struct struct_Antenna_Switched_Beam {
    int     patternIndex;
    int     lastPatternIndex;
    float   antennaHeight;
    float   antennaGain_dB;
    BOOL antennaIsLocked;
    int  numPatterns;
    AntennaPattern*  antennaPatterns;
}AntennaSwitchedBeam;


/// Init the switchedbeam antennas from
/// antenna-model config file.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param nodeInput  Pointer to NodeInput structure
/// \param phyIndex  interface for which physical
///    to be initialized
/// \param antennaModel  pointer to Global
///    antenna model structure
///

void ANTENNA_SwitchedBeamInit(
     Node* node,
     const NodeInput* nodeInput,
     int phyIndex,
     const AntennaModelGlobal* antennaModel);


/// Initialize data structure.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical
///    to be initialized
/// \param nodeInput  Pointer to NodeInput structure
///

void ANTENNA_SwitchedBeamInitFromConfigFile(
     Node* node,
     int phyIndex,
     const NodeInput* nodeInput);


/// Is using omnidirectional pattern.
///
/// \param phyData  Pointer to PhyData structure
///
/// \return Return TRUE if patternIndex is
/// ANTENNA_OMNIDIRECTIONAL_PATTERN

BOOL AntennaSwitchedBeamOmnidirectionalPattern(
    PhyData* phyData);


/// Return current pattern index.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical
///    to be initialized
///
/// \return Return the patternIndex

int AntennaSwitchedBeamReturnPatternIndex(
    Node* node,
    int phyIndex);


/// Return gain for this direction.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical
///    to be initialized
/// \param DOA  Direction of arrival
///
/// \return Return the antennaGain

float AntennaSwitchedBeamGainForThisDirection(
      Node* node,
      int phyIndex,
      Orientation DOA);


/// Return gain for current pattern index.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical
///    to be initialized
/// \param patternIndex  Index of a specified antenna pattern
/// \param DOA  Direction of arrival
///
/// \return Return the antennaGain

float AntennaSwitchedBeamGainForThisDirectionWithPatternIndex(
      Node* node,
      int phyIndex,
      int patternIndex,
      Orientation DOA);


/// Set the current pattern.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical
///    to be initialized
/// \param patternIndex  Index of a specified antenna pattern
///

void AntennaSwitchedBeamSetPattern(
     Node* node,
     int phyIndex,
     int patternIndex);


/// Get the current pattern.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical
///    to be initialized
/// \param patternIndex  Index of a specified antenna pattern
///

void AntennaSwitchedBeamGetPattern(
     Node* node,
     int phyIndex,
     int* patternIndex);


/// Return heighest gain and pattern index
/// for a signal
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical
///    to be initialized
/// \param propRxInfo  Pointer to PropRxInfo structure
/// \param patternIndex  Index of a specified antenna pattern
/// \param gain_dBi  Store antenna gain
///

void AntennaSwitchedBeamMaxGainPatternForThisSignal(
     Node* node,
     int phyIndex,
     PropRxInfo* propRxInfo,
     int* patternIndex,
     float* gain_dBi);

/// Returns TRUE is antenna is locked in one direction  
/// and returns false otherwise 
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
///
/// \return BOOL
inline
BOOL AntennaSwitchedBeamIsLocked(
    Node* node,
    int phyIndex)
{
    PhyData* phyData = node->phyData[phyIndex];
    AntennaSwitchedBeam* switched =
        (AntennaSwitchedBeam* )phyData->antennaData->antennaVar;

    ERROR_Assert(phyData->antennaData->antennaModelType ==
        ANTENNA_SWITCHED_BEAM ,
            "antennaModelType is not switched beam.\n");

    return (switched->antennaIsLocked);
}

/// Locks the antenna in one direction 
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
inline
void AntennaSwitchedBeamLockAntennaDirection(
    Node* node,
    int phyIndex)
{
    PhyData* phyData = node->phyData[phyIndex];
    AntennaSwitchedBeam* switched =
        (AntennaSwitchedBeam* )phyData->antennaData->antennaVar;

    ERROR_Assert(phyData->antennaData->antennaModelType ==
        ANTENNA_SWITCHED_BEAM ,
            "antennaModelType is not switched beam.\n");

    switched->antennaIsLocked = TRUE;
}


/// UN-Locks the antenna 
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
inline
void AntennaSwitchedBeamUnlockAntennaDirection(
    Node* node,
    int phyIndex)
{
    PhyData* phyData = node->phyData[phyIndex];
    AntennaSwitchedBeam* switched =
        (AntennaSwitchedBeam* )phyData->antennaData->antennaVar;

    ERROR_Assert(phyData->antennaData->antennaModelType ==
        ANTENNA_SWITCHED_BEAM ,
            "antennaModelType is not switched beam.\n");

    switched->antennaIsLocked = FALSE;
}

/// Looks through the switched beam patterns and finds the 
/// one closest to the new value of azimuthal angle that is
/// received as an input parameter. Sets the beam to the value
/// found.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
///    + azimuthAngle : double : New value of the antenna's azimuthal Angle 
inline
void AntennaSwitchedBeamSetBestPatternForAzimuth(
    Node* node,
    int phyIndex,
    double azimuthAngle)
{
    PhyData* phyData = node->phyData[phyIndex];
    AntennaSwitchedBeam* switched =
        (AntennaSwitchedBeam* )phyData->antennaData->antennaVar;
    double azimuthDelta = 360.0;
    int i;

    ERROR_Assert(phyData->antennaData->antennaModelType ==
        ANTENNA_SWITCHED_BEAM ,
            "antennaModelType is not switched beam.\n");

    for (i = 0; i < switched->numPatterns; i++) {
        double boreSightAngle =
                    switched->antennaPatterns->boreSightAngle[i].azimuth;

        double delta =
           MIN(fabs(azimuthAngle - boreSightAngle),
               fabs(azimuthAngle - (boreSightAngle - 360.0)));

        if (azimuthDelta > delta) {
            switched->patternIndex = i;
            azimuthDelta = delta;
        }//if//
    }//for//

    //GuiStart
    if (node->guiOption) {
        GUI_SetPatternIndex(node,
                            phyData->macInterfaceIndex,
                            switched->patternIndex,
                            node->getNodeTime());
    }
    //GuiEnd

}

#endif /* ANTENNA_SWITCHED_H */

