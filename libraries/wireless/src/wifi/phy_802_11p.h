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
// use in compliance with the license agreement as part of the EXata
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.


//--------------------------------------------------------------------------
//                             WAVE 802.11p Qualnet Extension
//--------------------------------------------------------------------------
//
//These files extend Qualnet 4.0 to include the physical and link layers
//of the WAVE 802.11p protocols.  You are welcome to use this code for
//research, education, or commercial purposes.If you use this code as part
//of a research project, please cite the following paper in your manuscripts.
//(NOTE THAT THIS CITATION MAY CHANGE AFTER A MORE APPROPRIATE ONE IS
// PUBLISHED):
//Blum, J., Neiswender, A.5, and Eskandarian, A.1, "Denial of Service Attacks
//on Inter-Vehicle Networks,"
//IEEE Intelligent Transportation Systems Conference, pp. 797-802, Oct 2008.
//--------------------------------------------------------------------------

//Scalable Network Technologies has ported the code from QualNet 4.0. 
//We have resolved the following issues in the code while
//porting:

//1)Before start of a transmission, sender should verify if there is
//adequate time to transmit before the start of guard interval. If not,
//then transmission is deferred.
//2) Transmission initiation is stopped in the gaurd interval.
//3) Recpetion of packet is stopped in the gaurd interval.


#ifndef _WAVE_PHY11_P_H_
#define _WAVE_PHY11_P_H_

/*
 * 802.11p parameters OFDM PHY
 */

// PLCP_SIZE's for 802.11p microseconds

// Source:  DSRC MAC and PHY Standard (2003) p. 8
#define PHY802_11p_SHORT_TRAINING_SIZE  16 // 10 short symbols

#define PHY802_11p_LONG_TRAINING_SIZE   16 // T[GI2] + 2 T[FFT]

#define PHY802_11p_SIGNAL_SIZE          8 // T[GI] + T[FFT]

#define PHY802_11p_OFDM_SYMBOL_DURATION 8 // T[GI] + T[FFT]

// Source:  DSRC MAC and PHY Standard (2003) p. 11 (Note that control channel
//will be channel 178 and we assume that service channel will be channel 184
#define PHY802_11p_CHANNEL_BANDWIDTH    10000000 //    10 MHz

#define PHY802_11p_SERVICE_BITS_SIZE    0
#define PHY802_11p_TAIL_BITS_SIZE       0

// Source:  DSRC MAC and PHY Standard (2003) p. 8 (unchanged)
#define PHY802_11p_PREAMBLE_AND_SIGNAL \
        (PHY802_11p_SHORT_TRAINING_SIZE + \
        PHY802_11p_LONG_TRAINING_SIZE + \
        PHY802_11p_SIGNAL_SIZE)

#define PHY802_11p_SYNCHRONIZATION_TIME \
        (PHY802_11p_PREAMBLE_AND_SIGNAL * MICRO_SECOND)


// Source:  DSRC MAC and PHY Standard (2003) p. 17 (unchanged)
#define PHY802_11p_RX_TX_TURNAROUND_TIME  (2 * MICRO_SECOND)
    #define PHY802_11p_PHY_DELAY PHY802_11p_RX_TX_TURNAROUND_TIME


// Source: DSRC MAC and PHY Standard (2003) p8(only changes are to data rate)
/*
 * Table 3  Rate-dependent parameters
 *
 * Data rate|Mod Coding rate|Coded bits per sym|Data bits per OFDM sym(Ndbps)
 *      3   | BPSK     1/2  |   1              |  24
 *      4.5 | BPSK     3/4  |   1              |  36
 *      6   | QPSK     1/2  |   2              |  48
 *      9   | QPSK     3/4  |   2              |  72
 *      12  | 16-QAM   1/2  |   4              |  96
 *      18  | 16-QAM   3/4  |   4              |  144
 *      24  | 64-QAM   2/3  |   6              |  192
 *      27  | 64-QAM   3/4  |   6              |  216
 */
#define PHY802_11p__3M  0
#define PHY802_11p_4_5M  1
#define PHY802_11p__6M  2
#define PHY802_11p__9M  3
#define PHY802_11p_12M  4
#define PHY802_11p_18M  5
#define PHY802_11p_24M  6
#define PHY802_11p_27M  7

#define PHY802_11p_LOWEST_DATA_RATE_TYPE  PHY802_11p__3M
#define PHY802_11p_HIGHEST_DATA_RATE_TYPE PHY802_11p_27M
#define PHY802_11p_DATA_RATE_TYPE_FOR_BC  PHY802_11p__3M

#define PHY802_11p_NUM_DATA_RATES 8
#define PHY802_11p_NUM_BER_TABLES  8

#define PHY802_11p_DATA_RATE__3M   3000000
#define PHY802_11p_DATA_RATE_4_5M   4500000
#define PHY802_11p_DATA_RATE__6M   6000000
#define PHY802_11p_DATA_RATE__9M   9000000
#define PHY802_11p_DATA_RATE_12M  12000000
#define PHY802_11p_DATA_RATE_18M  18000000
#define PHY802_11p_DATA_RATE_24M  24000000
#define PHY802_11p_DATA_RATE_27M  27000000

#define PHY802_11p_NUM_DATA_BITS_PER_SYMBOL__3M   24
#define PHY802_11p_NUM_DATA_BITS_PER_SYMBOL_4_5M   36
#define PHY802_11p_NUM_DATA_BITS_PER_SYMBOL__6M   48
#define PHY802_11p_NUM_DATA_BITS_PER_SYMBOL__9M   72
#define PHY802_11p_NUM_DATA_BITS_PER_SYMBOL_12M   96
#define PHY802_11p_NUM_DATA_BITS_PER_SYMBOL_18M  144
#define PHY802_11p_NUM_DATA_BITS_PER_SYMBOL_24M  192
#define PHY802_11p_NUM_DATA_BITS_PER_SYMBOL_27M  216

// From DSRC MAC and PHY Standard (2003) p. 11 (for channels 178 and 184)
#define PHY802_11p_DEFAULT_TX_POWER__3M_dBm  28.8
#define PHY802_11p_DEFAULT_TX_POWER_4_5M_dBm  28.8
#define PHY802_11p_DEFAULT_TX_POWER__6M_dBm  28.8
#define PHY802_11p_DEFAULT_TX_POWER__9M_dBm  28.8
#define PHY802_11p_DEFAULT_TX_POWER_12M_dBm  28.8
#define PHY802_11p_DEFAULT_TX_POWER_18M_dBm  28.8
#define PHY802_11p_DEFAULT_TX_POWER_24M_dBm  28.8
#define PHY802_11p_DEFAULT_TX_POWER_27M_dBm  28.8

// From DSRC MAC and PHY Standard (2003) p. 14 (using minimum sensitivities)
#define PHY802_11p_DEFAULT_RX_SENSITIVITY__3M_dBm  -85.0
#define PHY802_11p_DEFAULT_RX_SENSITIVITY_4_5M_dBm  -84.0
#define PHY802_11p_DEFAULT_RX_SENSITIVITY__6M_dBm  -82.0
#define PHY802_11p_DEFAULT_RX_SENSITIVITY__9M_dBm  -80.0
#define PHY802_11p_DEFAULT_RX_SENSITIVITY_12M_dBm  -77.0
#define PHY802_11p_DEFAULT_RX_SENSITIVITY_18M_dBm  -70.0
#define PHY802_11p_DEFAULT_RX_SENSITIVITY_24M_dBm  -69.0
#define PHY802_11p_DEFAULT_RX_SENSITIVITY_27M_dBm  -67.0

// Control overhead for PHY 802.11a and 802.11b
#define PHY802_11p_CONTROL_OVERHEAD_SIZE \
    ((PHY802_11p_SYNCHRONIZATION_TIME \
                     * PHY802_11p_DATA_RATE__3M / SECOND) \
    + PHY802_11p_SERVICE_BITS_SIZE               \
    + PHY802_11p_TAIL_BITS_SIZE)

#endif
