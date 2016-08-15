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
//We have resolved the following issues in the code while porting:

//1)Before start of a transmission, sender should verify if there is
//adequate time to transmit before the start of guard interval. If not,
//then transmission is deferred.
//2) Transmission initiation is stopped in the guard interval.
//3) Recpetion of packet is stopped in the guard interval.


#ifndef _WAVE_DOT11_P_H_
#define _WAVE_DOT11_P_H_

//SOURCE:  IEEE Trial Use Standard for Wireless Access in Vehicular
//         Environments - MultiChannel Operation (2006), Annex B, p. 45
#define M802_11p_CW_MIN          511
#define M802_11p_CW_MAX          511

//SOURCE:  DSRC Standard for MAC and PHY (2003), p. 10
//  "The slot time for the OFDM PHY shall be 16 µs, which is the sum
//   of the RX-to-TX turnaround time, MAC processing delay, and CCA
//   (clear channel assessment) detect time (<8 µs). The propagation
//   delay shall be regarded as being included in the CCA detect time."
#define M802_11p_SLOT_TIME       (16 * MICRO_SECOND)

//SOURCE:  DSRC Standard for MAC and PHY (2003), p. 17
#define M802_11p_SIFS            (32 * MICRO_SECOND)
#define M802_11p_DELAY_UNTIL_SIGNAL_AIRBORN  (2 * MICRO_SECOND)

// SOURCE: WAVE-Multichannel Operation Standard (2006)
//   p. 13, "The value of the guard interval is dot4SyncTolerance + dot4MaxChSwitchTime."
//   p. 50, Default value of dot4SyncTolerance is 2 Milliseconds
//   p. 50-51, Default value of dot4MaxChSwitchTime is 2 Milliseconds
#define M802_11p_GUARD_INTERVAL_DURATION    (4 * MILLI_SECOND)

// SOURCE: WAVE-Multichannel Operation Standard (2006)
//   p. 49, Default value of dot4CchInterval is 50 Milliseconds
#define M802_11p_CCH_INTERVAL_DURATION        (50 * MILLI_SECOND)

// SOURCE: WAVE-Multichannel Operation Standard (2006)
//   p. 50, Default value of dot4SchInterval is 50 Milliseconds
#define M802_11p_SCH_INTERVAL_DURATION        (50 * MILLI_SECOND)


// SOURCE:  IEEE TRIAL-USE STANDARD FOR WIRELESS ACCESS IN VEHICULAR
//          ENVIRONMENTS (WAVE)—MULTI-CHANNEL OPERATION (2006), p. 12

// Access Category related symbolic  constants operating on PHY 802.11p

// DESCRIPTION :: Background traffic Arbitary InterFrame Sequence
#define DOT11e_802_11p_CCH_AC_BK_AIFSN      9
#define DOT11e_802_11p_SCH_AC_BK_AIFSN      7

// DESCRIPTION :: Best effort traffic Arbitary InterFrame Sequence
#define DOT11e_802_11p_CCH_AC_BE_AIFSN      6
#define DOT11e_802_11p_SCH_AC_BE_AIFSN      3

// DESCRIPTION :: Video traffic Arbitary InterFrame Sequence
#define DOT11e_802_11p_CCH_AC_VI_AIFSN      3
#define DOT11e_802_11p_SCH_AC_VI_AIFSN      2

// DESCRIPTION :: Voice traffic Arbitary InterFrame Sequence
#define DOT11e_802_11p_CCH_AC_VO_AIFSN      2
#define DOT11e_802_11p_SCH_AC_VO_AIFSN      2


//Transmission Opportunity for each AC operating on PHY 802.11p

// DESCRIPTION :: Background traffic Transmission Opportunity Limit
#define DOT11e_802_11p_CCH_AC_BK_TXOPLimit  0
#define DOT11e_802_11p_SCH_AC_BK_TXOPLimit  0

// DESCRIPTION :: Best effort Transmission Opportunity Limit
#define DOT11e_802_11p_CCH_AC_BE_TXOPLimit  0
#define DOT11e_802_11p_SCH_AC_BE_TXOPLimit  0

// DESCRIPTION :: Video traffic Transmission Opportunity Limit
#define DOT11e_802_11p_CCH_AC_VI_TXOPLimit  0
#define DOT11e_802_11p_SCH_AC_VI_TXOPLimit  0

// DESCRIPTION :: Voice traffic Transmission Opportunity Limit
#define DOT11e_802_11p_CCH_AC_VO_TXOPLimit  0
#define DOT11e_802_11p_SCH_AC_VO_TXOPLimit  0

#define DOT11p_CCH_MAC_STATS_LABEL           "802.11pCCH"
#define DOT11p_SCH_MAC_STATS_LABEL           "802.11pSCH"

/*!
 * \brief Initialization function for 802.11P
 */
void Mac802_11p_Init(Node *node,
                     unsigned interfaceIndex,
                     const NodeInput* nodeInput,
                     PhyModel phyModel,
                     MacDataDot11* dot11);


/*!
 * \brief EDCA parameter initialization function for 802.11P
 */
void Mac802_11p_AcInit(Node* node,
                       MacDataDot11* dot11,
                       PhyModel phyModel);

/*!
 * \brief Function to manage the syncInterval that coordinates
 * multi-channel access
 */
void Mac802_11p_SyncInterval(Node* node, MacDataDot11* dot11, Message* msg);


/*!
 * \brief function to check if transmission can finish
 *before the guard interval start time
 */
BOOL Mac802_11p_CanStartTransmitting(Node* node,
                                MacDataDot11* dot11,
                                int dataRateType,
                                DOT11_MacFrameType frameType = DOT11_RTS,
                                clocktype hdrDelay = 0);
#endif /* _WAVE_DOT11_P_H_ */

