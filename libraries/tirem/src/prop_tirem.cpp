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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "api.h"
#include "prop_tirem.h"
#include "node.h"
#include "partition.h"
#include "product_info.h"

#ifndef WINDOWS_OS //linux
#  include <dlfcn.h>
#else
#  include <windows.h>
#  include <WinDef.h> 
#endif


#include "tirem3_activation.h"
#include "tirem3_dll.h"

const std::string tiremLibVer("340");

#ifndef WINDOWS_OS //linux
const std::string tiremLibPath("/addons/tirem/libtirem" + tiremLibVer +".so");
#else
const std::string tiremLibPath("\\addons\\tirem\\libtirem" + tiremLibVer + ".dll");
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Longley-Rice / TIREM related parameters:
//
//                               Climate Refractivity
// Equatorial                    1       360
// Continental Subtropical       2       320
// Maritime Tropical             3       370
// Desert                        4       280
// Continental Temperate         5       301
// Maritime Temperate, Over Land 6       320
// Maritime Temperate, Over Sea  7       350
//
//                Dielectric Constant Ground Conductivity
//                (Permittivity)
// Average Ground 15                  0.005
// Poor Ground     4                  0.001
// Good Ground    25                  0.02
// Fresh Water    81                  0.01
// Salt Water     81                  5.0


void TiremInitialize(
    PropChannel *propChannel,
    int channelIndex,
    const NodeInput *nodeInput)
{
    PropProfile* propProfile = propChannel->profile;
    BOOL wasFound;
    double elevationSamplingDistance;
    int climate;
    double refractivity;
    double conductivity;
    double permittivity;
    double humidity;
    BOOL horizontalPolarization;
    char polarization[MAX_STRING_LENGTH];
    char errorStr[MAX_STRING_LENGTH];
    char dllFilename[MAX_STRING_LENGTH];

    string homeEnv;
    if (!Product::GetProductHome(homeEnv))
    {
        std::string errStr("could not determine installation directory, try setting ");
        errStr += Product::GetHomeVariable();
        errStr += '\n';
        ERROR_ReportError(errStr.c_str());
    }
    string tiremLibName = homeEnv + tiremLibPath;
    // load the tirem library
#ifdef WINDOWS_OS //windows
    propProfile->tiremLibHandle = LoadLibrary( tiremLibName.c_str() );
#else //linux
    propProfile->tiremLibHandle = dlopen(tiremLibName.c_str(), RTLD_NOW | RTLD_GLOBAL );
#endif
    if (!propProfile->tiremLibHandle) {
        
        std::string errStr("Cannot Load TIREM library: ");
        errStr +=tiremLibName;
        errStr +="\n";
#ifdef WINDOWS_OS
        errStr += ERROR_GetErrorStdStr();
#else
        errStr += dlerror();
#endif
        ERROR_ReportError(errStr.c_str());
    }


    /* load the symbols from TIREM lib */

    //  The C symbol name is "RunTiremAnal".
    std:string librarySymbol = "RunTiremAnal";
    RunTiremAnalFunc  RunTiremAnal = NULL;

#ifdef WINDOWS_OS //windows
    RunTiremAnal = RunTiremAnalFunc( GetProcAddress( (HINSTANCE)propProfile->tiremLibHandle, librarySymbol.c_str() ) );
#else //linux
    RunTiremAnal = RunTiremAnalFunc(dlsym( propProfile->tiremLibHandle, librarySymbol.c_str()));
#endif

    if (!RunTiremAnal)
    {
#ifdef WINDOWS_OS //windows
        FreeLibrary( (HINSTANCE)propProfile->tiremLibHandle );
#else
        dlclose( propProfile->tiremLibHandle );
#endif //linux
        ERROR_ReportError("Unable to resolve symbol in TIREM lib, RunTiremAnal()\n");
    }


    IO_ReadDoubleInstance(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "PROPAGATION-SAMPLING-DISTANCE",
        channelIndex,
        (channelIndex == 0),
        &wasFound,
        &elevationSamplingDistance);

    if (wasFound) {
        propProfile->elevationSamplingDistance =
            (float)elevationSamplingDistance;
    }
    else {
        propProfile->elevationSamplingDistance =
            DEFAULT_SAMPLING_DISTANCE;
    }

    IO_ReadDoubleInstance(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "PROPAGATION-REFRACTIVITY",
        channelIndex,
        (channelIndex == 0),
        &wasFound,
        &refractivity);

    if (wasFound) {
        propProfile->refractivity = refractivity;
    }
    else {
        propProfile->refractivity = DEFAULT_REFRACTIVITY;
    }

    IO_ReadDoubleInstance(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "PROPAGATION-CONDUCTIVITY",
        channelIndex,
        (channelIndex == 0),
        &wasFound,
        &conductivity);

    if (wasFound) {
        propProfile->conductivity = conductivity;
    }
    else {
        propProfile->conductivity = DEFAULT_CONDUCTIVITY;
    }

    IO_ReadDoubleInstance(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "PROPAGATION-PERMITTIVITY",
        channelIndex,
        (channelIndex == 0),
        &wasFound,
        &permittivity);

    if (wasFound) {
        propProfile->permittivity = permittivity;
    }
    else {
        propProfile->permittivity = DEFAULT_PERMITTIVITY;
    }

    IO_ReadDoubleInstance(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "PROPAGATION-HUMIDITY",
        channelIndex,
        (channelIndex == 0),
        &wasFound,
        &humidity);

    if (wasFound) {
        propProfile->humidity = humidity;
    }
    else {
        propProfile->humidity = DEFAULT_HUMIDITY;
    }

    IO_ReadIntInstance(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "PROPAGATION-CLIMATE",
        channelIndex,
        (channelIndex == 0),
        &wasFound,
        &climate);

    if (wasFound) {
        assert(climate >= 1 && climate <= 7);
        propProfile->climate = climate;
    }
    else {
        propProfile->climate = DEFAULT_CLIMATE_NUM;
    }

    //
    // This must be an antenna property..
    //
    IO_ReadStringInstance(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "ANTENNA-POLARIZATION",
        channelIndex,
        (channelIndex == 0),
        &wasFound,
        polarization);

    if (wasFound) {
        if (strcmp(polarization, "HORIZONTAL")) {
            propProfile->polarization = HORIZONTAL;
            //propProfile->polarizationString = "H   ";
            strncpy(propProfile->polarizationString, "H   ", 4);
        }
        else if (strcmp(polarization, "VERTICAL")) {
            propProfile->polarization = VERTICAL;
            //propProfile->polarizationString = "V   ";
            strncpy(propProfile->polarizationString, "V   ", 4);
        }
        else {
            ERROR_ReportError("Unrecognized polarization\n");
        }
    }
    else {
        propProfile->polarization = VERTICAL;
        //propProfile->polarizationString = "V   ";
        strncpy(propProfile->polarizationString, "V   ", 4);
    }

    if ((propProfile->refractivity < 200) ||
        (propProfile->refractivity > 450))
    {
        sprintf(errorStr, "Surface refractivity out of range: %f\n",
                refractivity);
        ERROR_ReportError(errorStr);
    }

    if ((propProfile->conductivity < .00001) ||
        (propProfile->conductivity > 100))
    {
        sprintf(errorStr, "Conductivity out of range: %f\n", conductivity);
        ERROR_ReportError(errorStr);
    }

    return;
}

void TiremFinalize(Node *node, const int channelIndex)
{
    PropChannel* propChannel = node->partitionData->propChannel;
    PropProfile* propProfile = propChannel[channelIndex].profile;
    if (propProfile && propProfile->tiremLibHandle)
    {
         // so only executed once in each partition
#ifndef WINDOWS_OS
        dlclose(propProfile->tiremLibHandle);
#else
        FreeLibrary((HINSTANCE)propProfile->tiremLibHandle);
#endif
        propProfile->tiremLibHandle = NULL;
    }
}


double PathlossTirem(
    Node * node,
    const int channelIndex,
    long numSamples,
    float distance,
    double elevationArray[],
    float txAntennaHeight,
    float rxAntennaHeight,
    char polarizationString[],
    float humidity,
    float permittivity,
    float conductivity,
    float frequency,
    float refractivity)
{
    RunTiremAnalFunc  RunTiremAnal = NULL;
    float f_elevationArray[MAX_NUM_ELEVATION_SAMPLES];
    float distances[MAX_NUM_ELEVATION_SAMPLES];
    int j;
    long extnsn = 0;   // not a reused path
                       // NOTE:  DO NOT CHANGE THIS VALUE
    PropChannel* propChannel = node->partitionData->propChannel;
    PropProfile* propProfile = propChannel[channelIndex].profile;

    // TIREM outputs
    char version[TIREM_VRSN_LENGTH];    /* tirem version */
    char mode[TIREM_MODE_LENGTH];      /* operation mode */
    float path_loss = 0.0, freespace_loss = 0.0;
    char  actkey[TIREM_AKEY_LENGTH] = "";

    /*
     *
     * Input Parameter Ranges
     * These ranges were taken from Table A-1 in TIREM-SEM_Handbook.pdf
     *
     */

    // RX and TX antenna height
    ERROR_Assert( ((txAntennaHeight >= 0) && (txAntennaHeight <= 30000) &&
                   (rxAntennaHeight >= 0) && (rxAntennaHeight <= 30000)),
                  "Antenna height out of range (<0 or > 30000m)");

    // Frequency
    ERROR_Assert( ((frequency >= 1) && (frequency <= 20000)),
                  "Center frequency out of range (<1 or >20000)");

    // Conductivity
    ERROR_Assert( ((conductivity >= 0.00001) && (conductivity <= 100)),
                  "Conductivity out of range (<0.0001 or >100)");

    // Relative permitivity
    ERROR_Assert( ((permittivity >= 1) && (conductivity <= 100)),
                  "Conductivity out of range (<1 or >100)");

    // Humidity
    ERROR_Assert( ((humidity >= 0) && (humidity <= 13.25)),
                  "Conductivity out of range (<1 or >100)");


    for (j = 0; j < numSamples; j++)
    {
        f_elevationArray[j] = (float) elevationArray[j];
        distances[j] = distance * ((float) j / (numSamples - 1));
    }


    /* load the symbols from TIREM lib */

    //  The C symbol name is "RunTiremAnal".
    std:string librarySymbol = "RunTiremAnal";

#ifdef WINDOWS_OS //windows
    RunTiremAnal = RunTiremAnalFunc( GetProcAddress( (HINSTANCE)propProfile->tiremLibHandle, librarySymbol.c_str() ) );
#else //linux
    RunTiremAnal = RunTiremAnalFunc( dlsym( propProfile->tiremLibHandle, librarySymbol.c_str() ) );
#endif
    // it is better to save the RUnTiremAnal on node base, for now, get the handler each time calling the TIREM function.
    if (! RunTiremAnal )
    {
#ifdef WINDOWS_OS //windows
        FreeLibrary( (HINSTANCE)propProfile->tiremLibHandle );
#else
        dlclose( propProfile->tiremLibHandle );
#endif //linux
        ERROR_ReportError("Unable to resolve symbol in TIREM lib, RunTiremAnal()\n");
    }


    /****************************/
    //  Put in the correct key.
    /****************************/
    strncpy( actkey, tirem3_activation_key, tirem3_activation_key_length );

    /* additional input parameters to the tirem lib, which are not provided by JNE */

    int  numRainRates = 0;  
    float rainRateArray[5];
    float  rainHeight =0;  

    /* parameters for ouput from tirem lib */

    int  horztx, horzrx;
    float  clear, raloss[5], tottro, totdif, abloss, thet00,
           txang, rxang, alphae, betae,
           splen, tplen, rplen;


    clear = 0.0F;
    tottro = 0.0F;
    totdif = 0.0F;
    abloss = 0.0F;
    thet00 = 0.0F;
    txang = 0.0F;
    rxang = 0.0F;
    alphae = 0.0F;
    betae = 0.0F;
    horztx = 0;
    horzrx = 0;
    splen = 0.0F;
    tplen = 0.0F;
    rplen = 0.0F;


    RunTiremAnal( 
        txAntennaHeight, // Transmitter structural antenna height in meters. 
                         //Range: 0.0 to 30000.0 m
        rxAntennaHeight, // Receiver structural antenna height in meters. 
                         //Range: 0.0 to 30000.0 m
        frequency,      // Transmitter frequency in MHz. 
                        //Range: 1.0 to 40000.0 MHz 
        numSamples,          // Total number of profile points for the entire path. 
                        // Range: > 2
        f_elevationArray, // Array of profile terrain heights above mean sea level in meters. 
                        // Range: -450.0 to 9000.0 m
        distances,      //Array of great circle distances from the beginning of the profile 
                        //to each profile point in meters. Range: > 0.0
        extnsn,         // Profile indicator flag: 
                        // TRUE - Current profile is an extension of the last path along a radial 
                        // FALSE - New profile 
        refractivity,   // Surface refractivity in N-units. 
                        // Range: 200.0 to 450.0 N. 
        conductivity,   // Conductivity of earth surface Siemans per meter. 
                        // Range: 0.00001 to 100.0 S/m
        permittivity,   // Relative permittivity of earth surface. 
                        // Range: 1.0 to 100.0 
        humidity,       // Surface humidity at the transmitter site in grams per cubic meter. 
                        // Range: 0.0 to 110.0 in g/m^3 
        polarizationString,   // Transmitter antenna polarization: 
                        // "V " - Vertical, "H " - Horizontal 
                        // The char pointer must point to a 5 (TIREM_POLR_LENGTH) character buffer 
                        // including the null terminator. 
        numRainRates,   // The number of input rain rates.  
        rainRateArray,  // The array of requested rain rates. 
        rainHeight,     // Rain height above sea level in m.
        // output from tirem
        version,        // TIREM version number. 
                        // The char pointer must point to a 9 (TIREM_VRSN_LENGTH) character buffer 
                        // including the null terminator. 
        mode,           // Mode indicator: "LOS " - Line-of-sight, "DIF " - Diffraction, 
                        // "TRO " - Troposcatter, "INVL" - Invalid license (Activation key) 
                        // The char pointer must point to a 5 (TIREM_MODE_LENGTH) character buffer including the null terminator.  
        &clear,         // The ratio of the minimum clearance of the ray path to the first Fresnel zone radius. 
        &path_loss,     // Total path loss (Basic transmission loss) in dB. 
        &freespace_loss, // freespace_loss 
        raloss,         // The array of output rain losses. 
        &tottro,        // Troposcatter loss in dB. 
        &totdif,        // Diffraction loss in dB.
        &abloss,        // The absorption loss in dB. 
        &thet00,        // Troposcatter angle in radians 
        &txang,         // Elevation angle to the transmitter horizon in radians. 
        &rxang,         // Elevation angle to the receiver horizon in radians. 
        &alphae,        // Angle at the transmitter in the troposcatter triangle in radians. 
        &betae,         // Angle at the receiver in the troposcatter triangle in radians. 
        &horztx,        // Number of the profile point where the transmitter horizon occurs. 
        &horzrx,        // Number of the profile point where the receiver horizon occurs.
        &splen,         // Great circle path length at sea level. 
        &tplen,         // Great circle path length at transmitter antenna height.  
        &rplen,         // Great circle path length at receiver antenna height. 
        actkey );

    return (double) path_loss;
}

#ifdef __cplusplus
}
#endif
