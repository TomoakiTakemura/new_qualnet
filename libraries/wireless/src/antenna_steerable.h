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

#ifndef ANTENNA_STEERABLE_H
#define ANTENNA_STEERABLE_H

#define STEERABLE_AZIMUTH_INDEX 0
#define STEERABLE_ELEVATION_INDEX 1

/// This structure contains the data pertaining to 
/// Steerable Antenna. The structure is assigned to 
/// the phydata's (void*) antennaVar


typedef struct struct_Antenna_Steerable {
    int     patternSetRepeatAngle;
    int     patternIndex;
    float   antennaHeight;
    float   antennaGain_dB;            // Max antenna gain
    Orientation steeringAngle;
    BOOL antennaIsLocked;
    int  numPatterns;
    AntennaPattern* antennaPatterns;
} AntennaSteerable;



/// Init the steerable antennas from
/// antenna-model config file.
/// PARAMETERS
/// + node : Node* : Node pointer that the antenna is being
/// instantiated in
/// + nodeInput : const NodeInput* : Pointer to NodeInput structure
/// + phyIndex : int : interface for which physical layer is
/// to be initialized
/// + antennaModel : const AntennaModelGlobal* : pointer to Global
/// antenna model structure

void ANTENNA_SteerableInit(
    Node* node,
    const NodeInput* nodeInput,
    int phyIndex,
    const AntennaModelGlobal* antennaModel);


/// Init the steerable antennas from
/// configuration file.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
/// \param nodeInput  Pointer to NodeInput structure
///

void ANTENNA_SteerableInitFromConfigFile(
     Node* node,
     int phyIndex,
     const NodeInput* nodeInput);

/// Is using omnidirectional pattern.
///
/// \param phyData  Pointer to PhyData structure
///
/// \return Return TRUE if patternIndex is
/// ANTENNA_OMNIDIRECTIONAL_PATTERN


BOOL AntennaSteerableOmnidirectionalPattern(PhyData* phyData);


/// Return current pattern index.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
///
/// \return returns pattern index

int AntennaSteerableReturnPatternIndex(Node* node, int phyIndex);


/// Return gain for this direction.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
/// \param DOA  Direction of arrival
///
/// \return return gain in dBi

float AntennaSteerableGainForThisDirection(
      Node* node,
      int phyIndex,
      Orientation DOA);


/// Return gain for current pattern index.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
/// \param patternIndex  Index of a specified antenna pattern
/// \param DOA  Direction of arrival
///
/// \return return gain in dBi

float AntennaSteerableGainForThisDirectionWithPatternIndex(
      Node* node,
      int phyIndex,
      int patternIndex,
      Orientation DOA);


/// Set the current pattern.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
/// \param patternIndex  Index of a specified antenna pattern
///

void AntennaSteerableSetPattern(
     Node* node,
     int phyIndex,
     int patternIndex);


/// Get the current pattern.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
/// \param patternIndex  Index of a specified antenna pattern
///

void AntennaSteerableGetPattern(
     Node* node,
     int phyIndex,
     int* patternIndex);


/// Set the steering angle.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
/// \param DOA  Direction of Arrival
///

void AntennaSteerableSetMaxGainSteeringAngle(
    Node* node,
    int phyIndex,
    Orientation DOA);


/// Get the steering angle.
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
/// \param steeringAngle  Steering angle
///

void AntennaSteerableGetSteeringAngle(
     Node* node,
     int phyIndex,
     Orientation* steeringAngle);


/// Return heighest gain and pattern index
/// for a signal
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
/// \param propRxInfo  Pointer to PropRxInfo structure
/// \param patternIndex  Index of a specified antenna pattern
/// \param gain_dBi  Store antenna gain
///

void AntennaSteerableMaxGainPatternForThisSignal(
    Node* node,
    int phyIndex,
    PropRxInfo* propRxInfo,
    int* patternIndex,
    float* gain_dBi);


/// Return max gain and steering angle
/// for a signal
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
/// \param DOA  Direction of Arrival
/// \param steeringAngle  Steering angle
///

void AntennaSteerableMaxGainSteeringAngleForThisSignal(
    Node* node,
    int phyIndex,
    Orientation DOA,
    Orientation* steeringAngle);

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
BOOL AntennaSteerableIsLocked(
    Node* node,
    int phyIndex)
{
    PhyData* phyData = node->phyData[phyIndex];
    AntennaSteerable* steerable =
        (AntennaSteerable* )phyData->antennaData->antennaVar;

    ERROR_Assert(phyData->antennaData->antennaModelType ==
        ANTENNA_STEERABLE ,
            "antennaModelType not Steerable.\n");
    return (steerable->antennaIsLocked);
}

/// Locks the antenna in one direction 
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
///
inline
void AntennaSteerableLockAntennaDirection(
    Node* node,
    int phyIndex)
{
    PhyData* phyData = node->phyData[phyIndex];
    AntennaSteerable* steerable =
        (AntennaSteerable* )phyData->antennaData->antennaVar;

    ERROR_Assert(phyData->antennaData->antennaModelType ==
        ANTENNA_STEERABLE ,
            "antennaModelType not Steerable.\n");

    steerable->antennaIsLocked = TRUE;
}

/// UN-Locks the antenna 
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
inline
void AntennaSteerableUnlockAntennaDirection(
    Node* node,
    int phyIndex)
{
    PhyData* phyData = node->phyData[phyIndex];
    AntennaSteerable* steerable =
        (AntennaSteerable* )phyData->antennaData->antennaVar;

    ERROR_Assert(phyData->antennaData->antennaModelType ==
        ANTENNA_STEERABLE ,
            "antennaModelType not Steerable.\n");

    steerable->antennaIsLocked = FALSE;
}

/// Sets the current antenna-azimuth the the new value that is
/// received as an input parameter
///
/// \param node  Node pointer that the antenna is being
///    instantiated in
/// \param phyIndex  interface for which physical layer is
///    to be initialized
///    + azimuth : double : New value of the antenna's azimuthal Angle 
inline
void AntennaSteerableSetBestConfigurationForAzimuth(
    Node* node,
    int phyIndex,
    double azimuth)
{
    Orientation anOrientation;

    anOrientation.azimuth = (OrientationType)azimuth;
    anOrientation.elevation = 0;
    AntennaSteerableSetMaxGainSteeringAngle(node, phyIndex, anOrientation);
}

#endif /*ANTENNA_STEERABLE_H*/


