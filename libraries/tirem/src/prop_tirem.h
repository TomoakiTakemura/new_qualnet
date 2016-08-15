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


#ifndef TIREM_LIB_H
#define TIREM_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#define HORIZONTAL 0
#define VERTICAL 1

#define DEFAULT_SAMPLING_DISTANCE 100.0

#define DEFAULT_CLIMATE_NUM  1
#define DEFAULT_REFRACTIVITY 360
#define DEFAULT_PERMITTIVITY 15
#define DEFAULT_CONDUCTIVITY 0.005
#define DEFAULT_HUMIDITY     10

void TiremInitialize(
    PropChannel *propChannel,
    int channelIndex,
    const NodeInput *nodeInput);

void TiremFinalize(
    Node *node, 
    const int channelIndex);

double PathlossTirem(
    Node *node,
    const int channelIndex,
    long numSamples,
    float distanceBetwSamples,
    double elevationArray[],
    float txAntennaHeight,
    float rxAntennaHeight,
    char polarizationString[],
    float humidity,
    float permittivity,
    float conductivity,
    float frequency,
    float refractivity);


#ifdef __cplusplus
}
#endif

#endif /*TIREM_LIB_H*/
