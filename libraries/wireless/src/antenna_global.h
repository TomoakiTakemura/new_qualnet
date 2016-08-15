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

/// \defgroup Package_ANTENNA_GLOBAL ANTENNA_GLOBAL

/// \file
/// \ingroup Package_ANTENNA_GLOBAL
/// This file describes additional data structures and functions used by antenna models.

#ifndef ANTENNA_GLOBAL_H
#define ANTENNA_GLOBAL_H

/*
# ANTENNA-EFFICIENCY                        <antenna efficiency in dB>
# ANTENNA-MISMATCH-LOSS                     <antenna mismatch loss in dB>
# ANTENNA-CABLE-LOSS                        <antenna cable loss in dB>
# ANTENNA-CONNECTION-LOSS
*/

#ifdef __cplusplus
extern "C" {
#endif

/// Maximum number of models to allow.
#define  MAX_ANTENNA_MODELS     50

/// Maximum number of antenna patterns to allow.
#define  MAX_ANTENNA_PATTERNS   50


/// Different types of antenna models supported.
enum AntennaModelType {
    ANTENNA_OMNIDIRECTIONAL,
    ANTENNA_SWITCHED_BEAM,
    ANTENNA_STEERABLE,
    ANTENNA_PATTERNED
};


/// Different types of antenna pattern types supported.

enum AntennaPatternType {
    ANTENNA_PATTERN_TRADITIONAL,
    ANTENNA_PATTERN_ASCII2D,
    ANTENNA_PATTERN_ASCII3D,
    ANTENNA_PATTERN_NSMA,
    ANTENNA_PATTERN_EBE,
    ANTENNA_PATTERN_ASAPS
};

/// Different types of NSMA pattern versions supported.

enum NSMAPatternVersion {
    NSMA_TRADITIONAL,
    NSMA_REVISED,
};
/// Different types of antenna gain units supported.

enum AntennaGainUnit {
    ANTENNA_GAIN_UNITS_DBI,
    ANTENNA_GAIN_UNITS_VOLT_METERS
};


/// Different types of antenna pattern units supported.

enum AntennaPatternUnit {
    ANTENNA_PATTERN_UNITS_DEGREES_0_TO_90,
    ANTENNA_PATTERN_UNITS_DEGREES_0_TO_180,
    ANTENNA_PATTERN_UNITS_DEGREES_0_TO_360,
    ANTENNA_PATTERN_UNITS_DEGREES_MINUS_90_TO_90,
    ANTENNA_PATTERN_UNITS_DEGREES_MINUS_180_TO_180
};


typedef float AntennaElementGain;


/// Structure for antenna pattern elements

struct AntennaPatternElement {
    int                 numGainValues;  // Number of array items
    AntennaGainUnit     units;          // Units for gain

    AntennaElementGain  *gains;         // gain array pointer
};


/// Structure for antenna pattern

struct AntennaPattern {
    char                    antennaPatternName[2 * MAX_STRING_LENGTH];
    char                    antennaFileName[2 * MAX_STRING_LENGTH];
    AntennaPatternType      antennaPatternType; // Pattern Type

    int                     numOfPatterns;      // Number of patterns
    int                     patternSetRepeatAngle;//repeat angle for
                                                      //steerable antenna
    Orientation*             boreSightAngle;     // Orentation of bore
    float*                   boreSightGain_dBi;  // Gain at bore
    AntennaPatternUnit      azimuthUnits;       // Units of az
    AntennaPatternUnit      elevationUnits;     // Units of el
    int                 azimuthResolution;  // Resolution of az
                                                // units
    int                 elevationResolution;// Resolution of el
                                                // units
    int         maxAzimuthIndex;    // Maximum az
    int         maxElevationIndex;  // Maximum el
    bool        is3DGeometry;       // True if pattern is in 3D geometry
                                    // False if it is in pattern cut geometry
    bool        is3DArray;          // True if pattern is saved in 3D array
                                    // False if it is saved in 2D array
    void*                   antennaPatternElements; // Pattern array
};


/// Structure for antenna model

struct AntennaModelGlobal {
    char                antennaModelName[MAX_STRING_LENGTH];
    AntennaModelType    antennaModelType;   // Model type
    float                   height;         // Height in meters
    float                   antennaGain_dB; // Max antenna
                                                //gain
    double                  antennaEfficiency_dB;
    double                  antennaMismatchLoss_dB;
    double                  antennaConnectionLoss_dB;
    double                  antennaCableLoss_dB;
    AntennaPattern      *antennaPatterns;   // Array of antenna patterns
};


/// Preinitalize the global antenna structs.
///
/// \param partitionData  Pointer to partition data.
///

void ANTENNA_GlobalAntennaModelPreInitialize(
    PartitionData* partitionData);


/// Preinitalize the global antenna structs.
///
///    + partitionData : PartitionData*  : Pointer to partition data.
///

void ANTENNA_GlobalAntennaPatternPreInitialize(
    PartitionData* partitionData);


/// used to assign global radiation pattern for each
/// antenna.
///
/// \param node  node being used.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param antennaModelInput  structure containing
///    contents of input
///    file
///
/// \return Pointer to the global
/// antenna pattern structure.

AntennaPattern* ANTENNA_GlobalModelAssignPattern(
    Node* node,
    int phyIndex,
    const NodeInput* antennaModelInput);


/// Reads the antenna configuration parameters
/// into the global antenna model structure.
///
/// \param node  node being used.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param antennaModelInput  structure containing
///    contents of input
///    file
///    +antennaModelName  : const char * : contains the name of the antenna
///    model to be initialized.
///

void ANTENNA_GlobalAntennaModelInit(
    Node* node,
    int  phyIndex,
    const NodeInput* antennaModelInput,
    const char* antennaModelName);

/// Init the antenna pattern structure for pattern name for
/// the Old antenna model.
///
/// \param node  node being used.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param antennaPatternName  antenna pattern name to be
///    initialized from the main
///    configuration file.
/// \param steer  A boolean variable to differntiate which
///    init function called this function steerable
///    init or switched beam init.
///

void ANTENNA_GlobalAntennaPatternInitFromConfigFile(
    Node* node,
    int phyIndex,
    const char* antennaPatternName,
    BOOL steer);

/// Init the antenna pattern structure for pattern name.
///
/// \param node  node being used.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param antennaModelInput  structure containing
///    contents of input file.
/// \param antennaPatternName  antenna pattern name to be
///    initialized.
///

void ANTENNA_GlobalAntennaPatternInit(
    Node* node,
    int phyIndex,
    const NodeInput* antennaModelInput,
    const char* antennaPatternName);


/// Alloc a new model.
///
/// \param partitionData  Pointer to partition data.
///
/// \return Pointer to the global
/// antenna model structure.

AntennaModelGlobal* ANTENNA_GlobalAntennaModelAlloc(
    PartitionData* partitionData);


/// Return the model based on the name.
///
/// \param partitionData  Pointer to partition data.
/// \param antennaModelName  contains the name of the
///    antenna model.
///
/// \return Pointer to the global
/// antenna model structure.

AntennaModelGlobal* ANTENNA_GlobalAntennaModelGet(
    PartitionData* partitionData,
    const char* antennaModelName);


/// Return the antenna pattern based on the name.
///
/// \param partitionData  Pointer to partition data.
/// \param antennaPatternName  contains the name of the
///    antenna pattern.
///
/// \return Pointer to the global
/// antenna pattern structure.

AntennaPattern* ANTENNA_GlobalAntennaPatternGet(
    PartitionData* partitionData,
    const char* antennaPatternName);

/// Generate the Pattern name base on Pattern type.
///
/// \param node  node being used.
/// \param phyIndex  interface for which physical to be
///    initialized.
/// \param antennaModelInput  structure containing
///    contents of input
///    file
///    + patternType : char* patternType : Type of Pattern
/// \param antennaPatternName  antenna pattern name to be
///    initialized from the main
///    configuration file.
///


void ANTENNA_GeneratePatterName(Node* node,
                                int phyIndex,
                                const NodeInput* antennaModelInput,
                                const char* patternType,
                                char* antennaPatternName);

#ifdef __cplusplus
}
#endif

#endif // ANTENNA_GLOBAL_H
