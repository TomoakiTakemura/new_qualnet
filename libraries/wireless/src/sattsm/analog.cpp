// Copyright (c) 2001-2015, SCALABLE Network Technologies, Inc.  All Rights Reserved.
// All Rights Reserved.
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
//
#define __SATTSM_ANALOG_C__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <stdarg.h>

#include "sattsm.h"

#include "util_constants.h"
// #include "util_memory.h"

#include "analog.h"

#define DEBUG 0

using namespace PhySattsm;

// \defgroup Package_SATTSM SATTSM

// \file
// \ingroup Package_SATTSM
// 
// 
// SATTSM is the Satellite Technology Support Module.
// 
// SATTSM provides advanced satellite features for the analysis
// satellite systems.  
// 
// The package is highly modular so functionality can be added
// easily by the customer.  Each module is initialized and 
// most of the code that requires modification is limited to
// the OnSend and OnReceive methods.
// 
// For assistance with this module, please send support
// requests to:
// 
// support@scalable-networks.com
// 

// /*
// PROTOCOL:: ANALOG
// LAYER:: PHYSICAL
// REFERENCES:: 
// COMMENTS:: 
//    This file contains routines that are useful in modifying an
//    analog signal as represented by the PhySattsmAnalogHeader
//    object.  This object is pre-pended to all frames passing into
//    the satellite channel and contains all state required to 
//    determine the latency and completion status of the frame.

// Amplify an analog signal
//
//    PhySattsmAnalogHeader& h: an analog signal
//    double gain_dB: the gain, in dB
//

void PhySattsmAnalogAmplify(AnalogHeader& h, 
                         double gain_dB)
{
    // 
    // In a gain, all signal components are amplified.
    // 
    
    h.signalPower += gain_dB;
    h.noisePower += gain_dB;
    h.interferencePower += gain_dB;
}

// This injects noise power (in dBW) into the transmission
//
//    PhySattsmAnalogHeader& h: an analog signal
//    double pwd_dB: the noise power, in dB
//

void
PhySattsmAnalogInjectNoisePower(AnalogHeader& h, 
                                double pwr_dB)
{
    // 
    // Add the absolute powers and store as a dBW value.
    // 
    h.noisePower = UTIL::dbAddAbs(h.noisePower, 
                                  pwr_dB);
}

// Injects noise with the parameter given as a NF
//
//    SPhyattsmAnalogHeader& h: an analog signal
//    double nf, the noise figure
//

void
PhySattsmAnalogInjectNoiseByNF(AnalogHeader& h, 
                               double nf)
{
    double n0 = UTIL::abs2db(UTIL::nf2t(nf)) + UTIL::Constants::LogKb;

    // 
    // Create a No value given the NF and Boltzmann's constant.
    // Inject the noise power given the value of No
    // 
    PhySattsmAnalogInjectNoiseByNo(h, n0);
}

//
// Inject noise power given a noise temperature of the source.
//
//    PhySattsmAnalogHeader& h: an analog signal
//    double t: the noise temperature (in K)
//

void
PhySattsmAnalogInjectNoiseByTemp(AnalogHeader& h, 
                                 double t)
{
    double n0 = UTIL::abs2db(t) + UTIL::Constants::LogKb;

    // 
    // Create a No value given a noise temperature and Boltzmann's
    // constant.
    // Inject the noise power given No
    // 
    
    PhySattsmAnalogInjectNoiseByNo(h, n0);
}

//
// To inject noise power given a value for No 
//
//    PhySattsmAnalogHeader& h: an analog signal
//    double n0: SSB noise power/Hz
//

void
PhySattsmAnalogInjectNoiseByNo(AnalogHeader& h, 
                               double n0)
{
    // 
    // The noise power is equivalent to kTB, kT is already given as No.
    // 
    
    PhySattsmAnalogInjectNoisePower(h, 
                                    n0 + UTIL::abs2db(h.bandwidth));
}

// Determine the overall SNR of an analog header 
//
//    PhySattsmAnalogHeader& h: an analog signal
//
// \return The SNR (C/N+1) in dB

double
PhySattsmAnalogCalculateSNR(AnalogHeader& h)
{
    // 
    // nip is the N+I component in dB
    // snr = signalPower - nip in dB
    // 
    
    double nip = UTIL::dbAddAbs(h.noisePower, 
                                h.interferencePower);
    
    double snr = h.signalPower - nip;

    if (DEBUG)
    {
       printf("RESULT SNR %0.3f = pwr: %0.3f -"
              " noise:%0.3f + interference:%0.3f\n", 
              snr, 
              h.signalPower, 
              h.noisePower,
              h.interferencePower);
    }

    return snr;
}

