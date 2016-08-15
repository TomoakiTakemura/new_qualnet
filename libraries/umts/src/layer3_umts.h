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

/// \defgroup Package_UMTS_LAYER3 UMTS LAYER3

/// \file
/// \ingroup Package_UMTS_LAYER3
/// Defines the high level data structures and APIs used in
/// UMTS layer 3.

#ifndef _LAYER3_UMTS_H_
#define _LAYER3_UMTS_H_

#include "umts_constants.h"
#include <list>
#include <vector>
#include <deque>
#include <map>

// Forward declarations
class STAT_NetStatistics;

/// overhead size for data packet transmitted in UMTS network
const unsigned int UMTS_DATA_IP_OVERHEAD_SIZE = 32;

/// Default value of min cell selection rx level (Qrxlevmin)
#define UMTS_DEFAULT_FDD_CELL_SELECTION_MIN_RX_LEVEL    (-95.0)

/// Minimum value of min cell selection rx level (Qrxlevmin)
#define UMTS_MIN_FDD_CELL_SELECTION_MIN_RX_LEVEL    (-115.0)

/// Maximum value of min cell selection rx level (Qrxlevmin)
#define UMTS_MAX_FDD_CELL_SELECTION_MIN_RX_LEVEL    (-80.0)

/// Default value of min cell selection qual level (Qualmin)
#define UMTS_DEFAULT_FDD_CELL_SELECTION_MIN_QUAL_LEVEL    (-20.0)


/// Default value of min cell selection rx level (Qrxlevmin)
#define UMTS_DEFAULT_TDD_CELL_SELECTION_MIN_RX_LEVEL    (-84.0)

/// Default value of the cell search threshold (Sintrasearch)
#define UMTS_DEFAULT_CELL_SEARCH_THRESHOLD    (-80.0)

/// Minimum value of the cell search threshold (Sintrasearch)
#define UMTS_MIN_CELL_SEARCH_THRESHOLD    (-105.0)

/// Maximum value of the cell search threshold (Sintrasearch)
#define UMTS_MAX_CELL_SEARCH_THRESHOLD    (-70.0)

/// Default value of the cell reselection hysteresis (Qhyst)
#define UMTS_DEFAULT_CELL_RESELECTION_HYSTERESIS    (4.0)

/// Default value of the cell reselection hysteresis (Qhyst)
#define UMTS_MIN_CELL_RESELECTION_HYSTERESIS    (0.0)

/// Default value of the cell reselection hysteresis (Qhyst)
#define UMTS_MAX_CELL_RESELECTION_HYSTERESIS    (40.0)

/// Default value of the cell reselection hysteresis (Qhyst)
#define UMTS_DEFAULT_CELL_EVALUATION_TIME    (3 * SECOND)

/// Skip indicator in the layer3 message header
const UInt8 UMTS_LAYER3_SKIP_INDICATOR = 0x0;

/// Spare half octet in the layer3 message IE
const UInt8 UMTS_LAYER3_SPARE_HALF_OCT = 0x0;

/// 30 MINUTES  0x1E
const UInt8 UMTS_LAYER3_DEFAULT_RA_UPDATE_INTERVAL = 30;  

/// layer3 Timer values
const clocktype UMTS_LAYER3_TIMER_HELLO_START = 1 *NANO_SECOND;
const clocktype UMTS_LAYER3_TIMER_HELLO_DELAY = 5*SECOND;
const clocktype UMTS_LAYER3_TIMER_T300_VAL = 1000*MILLI_SECOND;
const clocktype UMTS_LAYER3_TIMER_T301_VAL = 2000*MILLI_SECOND;
const clocktype UMTS_LAYER3_TIMER_T302_VAL = 4000*MILLI_SECOND;
const clocktype UMTS_LAYER3_TIMER_T304_VAL = 2000*MILLI_SECOND;
const clocktype UMTS_LAYER3_TIMER_T305_VAL = 30*MINUTE;
const clocktype UMTS_LAYER3_TIMER_T307_VAL = 30*SECOND;
const clocktype UMTS_LAYER3_TIMER_T308_VAL = 160*MILLI_SECOND;
const clocktype UMTS_LAYER3_TIMER_T309_VAL = 5*SECOND;
const clocktype UMTS_LAYER3_TIMER_T310_VAL = 160*MILLI_SECOND;
const clocktype UMTS_LAYER3_TIMER_T311_VAL = 1000*MILLI_SECOND;
const clocktype UMTS_LAYER3_TIMER_RRCCONNWAITTIME_VAL = 10*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3101_VAL = 500*MILLI_SECOND;
const clocktype UMTS_LAYER3_TIMEr_T3107_VAL = 500*MILLI_SECOND;
const clocktype UMTS_LAYER3_TIMER_T312_VAL = 1*SECOND;
const clocktype UMTS_LAYER3_TIMER_T313_VAL = 3*SECOND;
const clocktype UMTS_LAYER3_TIMER_T314_VAL = 12*SECOND;
const clocktype UMTS_LAYER3_TIMER_T315_VAL = 180*SECOND;
const clocktype UMTS_LAYER3_TIMER_T316_VAL = 30*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3210_VAL = 20*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3211_VAL = 15*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3212_VAL = 50*MINUTE;   
                            //Specification doesn't specify this value

const clocktype UMTS_LAYER3_TIMER_T3213_VAL =  4*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3214_VAL = 20*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3216_VAL = 15*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3218_VAL = 20*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3220_VAL = 5*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3230_VAL = 15*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3240_VAL = 10*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3241_VAL = 300*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3250_VAL = 12*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3255_VAL = 6*SECOND;   
                            //Specification doesn't specify this value

const clocktype UMTS_LAYER3_TIMER_T3260_VAL = 12*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3270_VAL = 12*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3310_VAL = 15*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3311_VAL = 15*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3316_VAL = 30*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3318_VAL = 20*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3320_VAL = 15*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3321_VAL = 15*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3330_VAL = 15*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3340_VAL = 10*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3302_VAL = 12*MINUTE;   //Default value
const clocktype UMTS_LAYER3_TIMER_T3312_VAL = 54*MINUTE;   //Default value
const clocktype UMTS_LAYER3_TIMER_T3314_VAL = 44*SECOND;   //Default value
const clocktype UMTS_LAYER3_TIMER_T3317_VAL = 15*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3319_VAL = 30*SECOND;   //Default value
const clocktype UMTS_LAYER3_TIMER_T3322_VAL = 6*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3350_VAL = 6*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3360_VAL = 6*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3370_VAL = 6*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3313_VAL = 10*SECOND;   
                            //Network dependent

const clocktype UMTS_LAYER3_TIMER_MOBILEREACHABLE_VAL = 
                       UMTS_LAYER3_TIMER_T3312_VAL + 4*MINUTE;

const clocktype UMTS_LAYER3_TIMER_T3380_VAL = 30*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3381_VAL = 8*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3390_VAL = 8*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3385_VAL = 8*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3386_VAL = 8*SECOND;
const clocktype UMTS_LAYER3_TIMER_T3395_VAL = 8*SECOND;
const clocktype UMTS_LAYER3_TIMER_CC_T303_MS_VAL = 30*SECOND;
const clocktype UMTS_LAYER3_TIMER_CC_T305_VAL = 30*SECOND;
const clocktype UMTS_LAYER3_TIMER_CC_T308_MS_VAL = 30*SECOND;
const clocktype UMTS_LAYER3_TIMER_CC_T310_MS_VAL = 30*SECOND;
const clocktype UMTS_LAYER3_TIMER_CC_T313_MS_VAL = 30*SECOND;
const clocktype UMTS_LAYER3_TIMER_CC_T323_VAL = 30*SECOND;
const clocktype UMTS_LAYER3_TIMER_CC_T324_VAL = 15*SECOND;
const clocktype UMTS_LAYER3_TIMER_CC_T332_VAL = 30*SECOND;
const clocktype UMTS_LAYER3_TIMER_CC_T335_VAL = 30*SECOND;
const clocktype UMTS_LAYER3_TIMER_CC_T336_VAL = 10*SECOND;
const clocktype UMTS_LAYER3_TIMER_CC_T337_VAL = 10*SECOND;
const clocktype UMTS_LAYER3_TIMER_CC_T301_VAL = 180*SECOND;  //minimum value

const clocktype UMTS_LAYER3_TIMER_CC_T303_NW_VAL = 30*SECOND;   
                            //Set by network operator

const clocktype UMTS_LAYER3_TIMER_CC_T306_VAL = 30*SECOND;

const clocktype UMTS_LAYER3_TIMER_CC_T308_NW_VAL = 30*SECOND;   
                            //Set by network operator

const clocktype UMTS_LAYER3_TIMER_CC_T310_NW_VAL = 30*SECOND;   
                            //Set by network operator

const clocktype UMTS_LAYER3_TIMER_CC_T313_NW_VAL = 30*SECOND;   
                            //Set by network operator

const clocktype UMTS_LAYER3_TIMER_CC_T331_VAL = 30*SECOND;   
                            //Set by network operator

const clocktype UMTS_LAYER3_TIMER_CC_T333_VAL = 30*SECOND;   
                            //Set by network operator

const clocktype UMTS_LAYER3_TIMER_CC_T334_VAL = 15*SECOND;   //minumum value

const clocktype UMTS_LAYER3_TIMER_CC_T338_VAL = 30*SECOND;   
                            //Set by network operator

const clocktype UMTS_LAYER3_TIMER_RABASSGT_VAL = 5*SECOND;
const clocktype UMTS_LAYER3_TIMER_HLR_CELL_UPDATE_VAL = 3*SECOND;
const clocktype UMTS_DEFAULT_FLOW_TIMEOUT_INTERVAL =  (10 * SECOND);
const clocktype UMTS_DEFAULT_FLOW_RESTART_INTERVAL =  (30 * SECOND);
const clocktype UMTS_DEFAULT_PS_FLOW_IDLE_TIME = (45 * SECOND);
const clocktype UMTS_LAYER3_TIMER_CPICHCHECK_VAL = 5*SECOND;
const clocktype UMTS_LAYER3_TIMER_MEASREPCHECK_VAL = 5*SECOND;

/// layer3 Timer counters
const unsigned char UMTS_LAYER3_TIMER_N300 = 3;
const unsigned char UMTS_LAYER3_TIMER_N301 = 2;
const unsigned char UMTS_LAYER3_TIMER_N302 = 3;
const unsigned char UMTS_LAYER3_TIMER_N308 = 3;
const unsigned char UMTS_LAYER3_TIMER_N310 = 4;
const unsigned char UMTS_LAYER3_TIMER_N312 = 1;
const unsigned char UMTS_LAYER3_TIMER_N313 = 20;
const unsigned char UMTS_LAYER3_TIMER_N315 = 1;

const unsigned char UMTS_LAYER3_TIMER_GMM_MAXCOUNTER = 4;
const unsigned char UMTS_LAYER3_TIMER_CC_MAXCOUNTER = 2;
const unsigned char UMTS_LAYER3_TIMER_SM_MAXCOUNTER = 4;

/// Maximum info size of Backbone msg
const unsigned int UMTS_BACKBONE_HDR_MAX_INFO_SIZE = 15;

/// The thresold between the current time and last
/// timestamp of CPICH measurement to remove a cell
/// from monitored set
const clocktype UMTS_CPICH_MEAS_OBSLT_TRH = 10*SECOND;

/// The thresold of maximum allowed interval between
/// two meausurement reports from a UE, used to
/// determine whether a UE is physically connected
const clocktype UMTS_MEASREP_EXP_TRH = 10*SECOND;

/// The list of sublayers in UMTS layer3
typedef enum
{
    UMTS_LAYER3_SUBLAYER_SM,
    UMTS_LAYER3_SUBLAYER_CC, // Connection Management (CM) sublayer
    UMTS_LAYER3_SUBLAYER_MM, // Mobility Management (MM) sublayer
    UMTS_LAYER3_SUBLAYER_GMM,
    UMTS_LAYER3_SUBLAYER_RRC, // Radio Resource Control (RRC) sublayer
} UmtsLayer3SublayerType;

/// CN domain identity
typedef enum
{
    UMTS_LAYER3_CN_DOMAIN_CS = 0,
    UMTS_LAYER3_CN_DOMAIN_PS
} UmtsLayer3CnDomainId;

/// Protocol Discriminator (PD) of UMTS layer3 protocols
typedef enum
{
    UMTS_LAYER3_PD_GCC  = 0,  // 0000: group call control
    UMTS_LAYER3_PD_BCC  = 1,  // 0001: broadcast call control
    UMTS_LAYER3_PD_SIM  = 2,  // 0011: Some packets for simulation purpose
    UMTS_LAYER3_PD_CC   = 3,  // 0011: call control, call related SS msgs
    UMTS_LAYER3_PD_GTP  = 4,  // 0100: GPRS Transparent Transport Protocol
    UMTS_LAYER3_PD_MM   = 5,  // 0101: mobility management messages
    UMTS_LAYER3_PD_RR   = 6,  // 0110: radio resource management messages
    UMTS_LAYER3_PD_GMM  = 8,  // 1000: GPRS mobility management messages
    UMTS_LAYER3_PD_SMS  = 9,  // 1001: SMS messages
    UMTS_LAYER3_PD_SM   = 10, // 1010: GPRS session management messages
    UMTS_LAYER3_PD_SS   = 11, // 1011: non call related SS messages
    UMTS_LAYER3_PD_LS   = 12, // 1100: location services
    UMTS_LAYER3_PD_EXT  = 14, // 1110: reserved for extend PD to 1octet long
    UMTS_LAYER3_PD_TEST = 15, // 1111: reserved for tests procedures
} UmtsLayer3PD;

/// Type of various UMTS layer3 timers
typedef enum
{
    //----------------------------------------------------------------------
    //  RRC timers - UE side
    //----------------------------------------------------------------------
    UMTS_LAYER3_TIMER_SIB,

    UMTS_LAYER3_TIMER_HELLO,    // timer msg for sending a hello
    UMTS_LAYER3_TIMER_POWERON,
    UMTS_LAYER3_TIMER_POWEROFF, // The node is powered on/off. This is only
                                // used for UE. Power on used for NodeB too.

    UMTS_LAYER3_TIMER_T300,     // Timer for rtx of RRC Connection Request
    UMTS_LAYER3_TIMER_T302,     // Timer for rtx of Cell Update
    UMTS_LAYER3_TIMER_T304,     // Timer for rtx of UE capability info.
    UMTS_LAYER3_TIMER_T305,     // Timer for rtx Cell Update
    UMTS_LAYER3_TIMER_T307,     // Timer for delay before going to idle mode
    UMTS_LAYER3_TIMER_T308,     // Timer for tx RRC Conn. Release Complete
    UMTS_LAYER3_TIMER_T309,     // Timer for changing cell
    UMTS_LAYER3_TIMER_T310,     // Timer for rtx of PUSCH Capacity Request
    UMTS_LAYER3_TIMER_T311,     // Timer for delay before ini PUSCH Cap. Req
    UMTS_LAYER3_TIMER_T312,     // Timer for detect phy. ch estab. failure
    UMTS_LAYER3_TIMER_T313,     // Timer for detect radio link failure
    UMTS_LAYER3_TIMER_T314,     // Timer for recovery from RL failure(CS)
    UMTS_LAYER3_TIMER_T315,     // Timer for recovery from RL failure(PS)
    UMTS_LAYER3_TIMER_T316,     // Timer for delay b4 transit to CELL_FACH
    UMTS_LAYER3_TIMER_T317,     // Timer when out of service & in CELL_FACH
    UMTS_LAYER3_TIMER_T318,     // Timer for waitting RRC conn. b4 go idle

    UMTS_LAYER3_TIMER_RRCCONNWAITTIME,  //Timer for wait for init. next RRC 
                                        //connection request after rejected.

    UMTS_LAYER3_TIMER_CPICHCHECK, //Timer for checking cell out of range
    UMTS_LAYER3_TIMER_MEASREPCHECK, //Timer for checking connected UEs 
                                    //lost of physical links

    //----------------------------------------------------------------------
    //  RRC timers - RNC side
    //----------------------------------------------------------------------
    UMTS_LAYER3_TIMER_T3101,    // Timer set after sending
                                // RRC CONNECTION SETUP

    UMTS_LAYER3_TIMER_T3107,    // Timer set after sending 
                                // RADIO BEARER SETUP/RECONFIGURATION

    //----------------------------------------------------------------------
    //  MM timers - MS side
    //----------------------------------------------------------------------
    UMTS_LAYER3_TIMER_T3210,    // Timer for waiting for 
                                // location updating response

    UMTS_LAYER3_TIMER_T3211,    // Timer for restarting 
                                // location update proc after rejected

    UMTS_LAYER3_TIMER_T3212,    // Timer for initiating 
                                // periodic location update proc

    UMTS_LAYER3_TIMER_T3213,    // Timer for initiate random attemp 
                                // after locate updating failure

    UMTS_LAYER3_TIMER_T3214,    // Timer for authentication purpose
    UMTS_LAYER3_TIMER_T3216,    // Timer for authentication purpose
    UMTS_LAYER3_TIMER_T3218,    // Timer for authentication purpose
    UMTS_LAYER3_TIMER_T3220,    // Timer for initiated after IMSI detach
    UMTS_LAYER3_TIMER_T3230,    // Timer for waiting for CM SERV req. resp
    UMTS_LAYER3_TIMER_T3240,    // Timer for aborting RR connection
    UMTS_LAYER3_TIMER_T3241,    // Timer for aborting RR connection

    //----------------------------------------------------------------------
    //  MM timers - Network side
    //----------------------------------------------------------------------
    UMTS_LAYER3_TIMER_T3250,    // Timer for waiting for TMSI-REALL-COM
    UMTS_LAYER3_TIMER_T3255,    // Timer for waiting for CM SERVICE REQUEST
    UMTS_LAYER3_TIMER_T3260,    // Timer for authentication pupose
    UMTS_LAYER3_TIMER_T3270,    // Timer for waiting IDENTITY RESPONSE

    //----------------------------------------------------------------------
    //  GMM timers - MS side
    //----------------------------------------------------------------------
    UMTS_LAYER3_TIMER_T3310,    // Timer for waiting for GMM attach response
    UMTS_LAYER3_TIMER_T3311,    // Timer for waiting for 
                                // next attempt of Attach or RAU

    UMTS_LAYER3_TIMER_T3316,    // Timer related to GMM security mode
    UMTS_LAYER3_TIMER_T3318,    // Timer related to GMM authentication
    UMTS_LAYER3_TIMER_T3320,    // Timer related to GMM authentication
    UMTS_LAYER3_TIMER_T3321,    // Timer for waiting for DETACH ACCEPT
    UMTS_LAYER3_TIMER_T3330,    // Timer for waiting for 
                                // routing ROUTING AREA UPDATE response

    UMTS_LAYER3_TIMER_T3340,    // Timer for waiting for 
                                // releasing PS signaling connection

    UMTS_LAYER3_TIMER_T3302,    // Timer for initiating new attempt of 
                                // attach after failure

    UMTS_LAYER3_TIMER_T3312,    // Timer for initiating periodic RAC proc
                                // after leaving PMM-CONNECTED

    UMTS_LAYER3_TIMER_T3314,    // READY Timer for A/Gb mode only
    UMTS_LAYER3_TIMER_T3317,    // Timer for initiating GMM-SERVICE-REQUESET
    UMTS_LAYER3_TIMER_T3319,    // Timer for repeating  GMM-SERVICE-REQUESET
                                // with service type "data"

    //--------------------------------------------------------------------
    //  GMM timers - network side
    //--------------------------------------------------------------------
    UMTS_LAYER3_TIMER_T3322,    // Timer for waiting for GMM DETACH REQ
    UMTS_LAYER3_TIMER_T3350,    // Timer for waiting for 
                                // ATTACH/RAU ACCEPT response from MS

    UMTS_LAYER3_TIMER_T3360,    // Timer for authentiating purpose
    UMTS_LAYER3_TIMER_T3370,    // Timer for identity purpose
    UMTS_LAYER3_TIMER_T3313,    // Timer for waiting for paging response
    UMTS_LAYER3_TIMER_MOBILEREACHABLE, //

    //---------------------------------------------------------------------
    //  SM timers - MS side
    //---------------------------------------------------------------------
    UMTS_LAYER3_TIMER_T3380,    // Timer for waiting for 
                                // ACTIVATE PDP CONTEXT response

    UMTS_LAYER3_TIMER_T3381,    // Timer for waiting for 
                                // MODIFY PDP CONTEXT response

    UMTS_LAYER3_TIMER_T3390,    // Timer for waiting for 
                                // DEACTIVATE PDP CONTEXT response

    //----------------------------------------------------------------------
    //  SM timers - Network side
    //----------------------------------------------------------------------
    UMTS_LAYER3_TIMER_T3385,    // Timer for waiting for 
                                // ACTIVATE PDP CONTEXT response

    UMTS_LAYER3_TIMER_T3386,    // Timer for waiting for 
                                // MODIFY PDP CONTEXT response

    UMTS_LAYER3_TIMER_T3395,    // Timer for waiting for 
                                // DEACTIVATE PDP CONTEXT response

    //----------------------------------------------------------------------
    //  CC timers
    //----------------------------------------------------------------------
    UMTS_LAYER3_TIMER_CC_T303,  // Timer for waiting for init. Call resp.
    UMTS_LAYER3_TIMER_CC_T305,  // Timer for waiting for disc. call resp.
    UMTS_LAYER3_TIMER_CC_T308,  // Timer for waiting for release call resp.

    UMTS_LAYER3_TIMER_CC_T310,  // Timer for waiting for 
                                // ALERT/CONN/DISC/PROG
    UMTS_LAYER3_TIMER_CC_T313,  // Timer for waiting for CONNect Ack
    UMTS_LAYER3_TIMER_CC_T323,  // Timer for waiting for MOD sent response
    UMTS_LAYER3_TIMER_CC_T324,  // Timer for waiting for MOD response 
                                // after MOD received

    UMTS_LAYER3_TIMER_CC_T332,  // Timer for waiting for CC-EST
    UMTS_LAYER3_TIMER_CC_T335,  // Timer for waiting for RECALL
    UMTS_LAYER3_TIMER_CC_T336,  // Timer for waiting for START DTMF response
    UMTS_LAYER3_TIMER_CC_T337,  // Timer for waiting for STOP  DTMT response

    UMTS_LAYER3_TIMER_CC_T301,  // Timer for waiting for CONN
    UMTS_LAYER3_TIMER_CC_T306,  // Timer for waiting for REL/DISC
    UMTS_LAYER3_TIMER_CC_T331,  // Timer for waiting for START CC
    UMTS_LAYER3_TIMER_CC_T333,  // Timer 4 waiting for CC-EST.CONF/REL COMP
    UMTS_LAYER3_TIMER_CC_T334,  // Timer for waiting for SETUP
    UMTS_LAYER3_TIMER_CC_T338,  // Timer for waiting for REL/DISC

    // misc
    UMTS_LAYER3_TIMER_RABASSGT,  //Timer 4 waiting 4 RAB ASSIGNMENT RESPONSE
    UMTS_LAYER3_TIMER_FLOW_TIMEOUT,
    //UMTS_LAYER3_TIMER_CELLSWITCH,
    UMTS_LAYER3_TIMER_HLR_CELL_UPDATE,

    //UMTS_LAYER3_TIMER_CELLSWITCH_FRAGMENT_DELAY,
    UMTS_LAYER3_TIMER_UPDATE_GUI
} UmtsLayer3TimerType;

// ENUM        :: UmtsBackboneMessageType
// DESCRIPTION :: Type of backbone message type
// REFERENCE   ::
typedef enum
{
    UMTS_BACKBONE_MSG_TYPE_HELLO,
    UMTS_BACKBONE_MSG_TYPE_IUB,     //For signalling and data on Interface
                                    //between NodeB and RNC 
                                    //(NBAP or NODEB forwarding)

    UMTS_BACKBONE_MSG_TYPE_IUR,     //For data on Interface between RNCs 
                                    //(SRNC relocation)

    UMTS_BACKBONE_MSG_TYPE_IU,      //For signalling on Interface between
                                    // SGSN and RNC (RANAP)

    UMTS_BACKBONE_MSG_TYPE_IU_CSDATA,
    //UMTS_BACKBONE_MSG_TYPE_GN,      //For signalling on Interface between
                                      // SGSN and GGSN

    UMTS_BACKBONE_MSG_TYPE_GR,      //For signalling on Interface between
                                    // SGSN and HLR

    UMTS_BACKBONE_MSG_TYPE_GC,      //For signalling on Interface between
                                    // GGSN and HLR

    UMTS_BACKBONE_MSG_TYPE_CSSIG,   //For CS domain signaling
    UMTS_BACKBONE_MSG_TYPE_CSDATA,  //CS domain data
    UMTS_BACKBONE_MSG_TYPE_GTP,     //PS domain signalling/data between
                                    // GGSN and SGSN

    UMTS_BACKBONE_MSG_TYPE_IUR_FORWARD    //Message type used to forward
                                          // IUR messages between SGSNs
} UmtsBackboneMessageType;

// ENUM        :: UmtsCsSigMsgType
// DESCRIPTION :: Type of various backbone CS domain signaling message
typedef enum
{
    UMTS_CSSIG_MSG_CALL_SETUP,
    UMTS_CSSIG_MSG_CALL_ALERTING,
    UMTS_CSSIG_MSG_CALL_CONNECT,
    UMTS_CSSIG_MSG_CALL_DISCONNECT
    //UMTS_CSSIG_MSG_CALL_REJECT,
}UmtsCsSigMsgType;

// ENUM        :: UmtsNbapMessageType
// DESCRIPTION :: Type of various NBAP signalling messages
typedef enum
{
    UMTS_NBAP_MSG_TYPE_RRC_SETUP_REQUEST,
    UMTS_NBAP_MSG_TYPE_RRC_SETUP_RESPONSE,
    UMTS_NBAP_MSG_TYPE_RRC_SETUP_FAILURE,
    UMTS_NBAP_MSG_TYPE_RRC_SETUP_RELEASE,
    UMTS_NBAP_MSG_TYPE_RRC_RELEASE_RESPONSE,
    UMTS_NBAP_MSG_TYPE_RADIO_BEARER_SETUP_REQUEST,
    UMTS_NBAP_MSG_TYPE_RADIO_BEARER_SETUP_RESPONSE,
    UMTS_NBAP_MSG_TYPE_RADIO_BEARER_RELEASE_REQUEST,
    UMTS_NBAP_MSG_TYPE_RADIO_BEARER_RELEASE_RESPONSE,
    UMTS_NBAP_MSG_TYPE_JOIN_ACTIVE_SET_REQUEST,
    UMTS_NBAP_MSG_TYPE_JOIN_ACTIVE_SET_RESPONSE,
    UMTS_NBAP_MSG_TYPE_LEAVE_ACTIVE_SET_REQUEST,
    UMTS_NBAP_MSG_TYPE_LEAVE_ACTIVE_SET_RESPONSE,
    UMTS_NBAP_MSG_TYPE_SWITCH_CELL_REQUEST,
    UMTS_NBAP_MSG_TYPE_SWITCH_CELL_RESPONSE,
    UMTS_NBAP_MSG_TYPE_REPORT_AMRLC_ERROR
} UmtsNbapMessageType;

// ENUM        :: UmtsRnsapMessageType
// DESCRIPTION :: Type of various RNSAP signalling messages
// REFERENCE   ::
typedef enum
{
    UMTS_RNSAP_MSG_TYPE_SRNC_UPDATE,
    UMTS_RNSAP_MSG_TYPE_UPLINK_TRANSFER,
    UMTS_RNSAP_MSG_TYPE_DOWNLINK_TRANSFER,
    UMTS_RNSAP_MSG_TYPE_SRB_RELEASE_REQUEST,
    UMTS_RNSAP_MSG_TYPE_RADIO_BEARER_SETUP_REQUEST,
    UMTS_RNSAP_MSG_TYPE_RADIO_BEARER_SETUP_RESPONSE,
    UMTS_RNSAP_MSG_TYPE_RADIO_BEARER_RELEASE_REQUEST,
    UMTS_RNSAP_MSG_TYPE_RADIO_BEARER_RELEASE_RESPONSE,
    UMTS_RNSAP_MSG_TYPE_JOIN_ACTIVE_SET_REQUEST,
    UMTS_RNSAP_MSG_TYPE_JOIN_ACTIVE_SET_RESPONSE,
    UMTS_RNSAP_MSG_TYPE_LEAVE_ACTIVE_SET_REQUEST,
    UMTS_RNSAP_MSG_TYPE_LEAVE_ACTIVE_SET_RESPONSE,
    UMTS_RNSAP_MSG_TYPE_SWITCH_CELL_REQUEST,
    UMTS_RNSAP_MSG_TYPE_SWITCH_CELL_RESPONSE,
    UMTS_RNSAP_MSG_TYPE_REPORT_AMRLC_ERROR
}UmtsRnsapMessageType;

// ENUM        :: UmtsRanapMessageType
// DESCRIPTION :: Type of various RANAP signalling messages
// REFERENCE   ::
typedef enum
{
    UMTS_RANAP_MSG_TYPE_CELL_LOOKUP,
    UMTS_RANAP_MSG_TYPE_CELL_LOOKUP_REPLY,
    UMTS_RANAP_MSG_TYPE_IUR_MESSAGE_FORWARD,
    UMTS_RANAP_MSG_TYPE_INITIAL_UE_MESSAGE,
    UMTS_RANAP_MSG_TYPE_DIRECT_TRANSFER,
    UMTS_RANAP_MSG_TYPE_PAGING,
    UMTS_RANAP_MSG_TYPE_RAB_ASSIGNMENT_REQUEST,
    UMTS_RANAP_MSG_TYPE_RAB_ASSIGNMENT_RESPONSE,
    UMTS_RANAP_MSG_TYPE_RAB_ASSIGNMENT_RELEASE,
    UMTS_RANAP_MSG_TYPE_IU_RELEASE_REQUEST,
    UMTS_RANAP_MSG_TYPE_IU_RELEASE_COMMAND,
    UMTS_RANAP_MSG_TYPE_IU_RELEASE_COMPLETE,
} UmtsRanapMessageType;

// ENUM        :: UmtsRRMessageType
// DESCRIPTION :: Type of various RR signalling messages
// REFERENCE   ::
typedef enum
{
    UMTS_RR_MSG_TYPE_ACTIVE_SET_UPDATE,
    UMTS_RR_MSG_TYPE_ACTIVE_SET_UPDATE_COMPLETE,
    UMTS_RR_MSG_TYPE_ACTIVE_SET_UPDATE_FAILURE,
    UMTS_RR_MSG_TYPE_ASSISTANCE_DATA_DELIVERY,
    UMTS_RR_MSG_TYPE_CELL_CHANGE_ORDER_FROM_UTRAN,
    UMTS_RR_MSG_TYPE_CELL_CHANGE_ORDER_FROM_UTRAN_FAILURE,
    UMTS_RR_MSG_TYPE_CELL_UPDATE,
    UMTS_RR_MSG_TYPE_CELL_UPDATE_CONFIRM_CCCH,
    UMTS_RR_MSG_TYPE_CELL_UPDATE_CONFIRM,
    UMTS_RR_MSG_TYPE_COUNTER_CHECK,
    UMTS_RR_MSG_TYPE_COUNTER_CHECK_RESPONSE,
    UMTS_RR_MSG_TYPE_DL_DIRECT_TRANSFER,
    UMTS_RR_MSG_TYPE_HANDOVER_TO_UTRAN_COMPLETE,
    UMTS_RR_MSG_TYPE_INITIAL_DIRECT_TRANSFER,
    UMTS_RR_MSG_TYPE_HANDOVER_FROM_UTRAN_COMMAND_GRRANIu,
    UMTS_RR_MSG_TYPE_HANDOVER_FROM_UTRAN_COMMAND_GSM,
    UMTS_RR_MSG_TYPE_HANDOVER_FROM_UTRAN_COMMAND_CDMA2000,
    UMTS_RR_MSG_TYPE_HANDOVER_FROM_UTRAN_FAILURE,
    // skip MBMS
    UMTS_RR_MSG_TYPE_MEASUREMENT_CONTROL,
    UMTS_RR_MSG_TYPE_MEASUREMENT_CONTROL_FAILURE,
    UMTS_RR_MSG_TYPE_MEASUREMENT_REPORT,
    UMTS_RR_MSG_TYPE_PAGING_TYPE1,
    UMTS_RR_MSG_TYPE_PAGING_TYPE2,
    UMTS_RR_MSG_TYPE_PHYSICAL_CHANNEL_RECONFIGURATION,
    UMTS_RR_MSG_TYPE_PHYSICAL_CHANNEL_RECONFIGURATION_COMPLETE,
    UMTS_RR_MSG_TYPE_PHYSICAL_CHANNEL_RECONFIGURATION_FAILURE,
    UMTS_RR_MSG_TYPE_PHYSICAL_SHARED_CHANNEL_ALLOCATION,
    UMTS_RR_MSG_TYPE_PUSCH_CAPACITY_REQUEST,
    UMTS_RR_MSG_TYPE_RADIO_BEARER_RECONFIGURATION,
    UMTS_RR_MSG_TYPE_RADIO_BEARER_RECONFIGURATION_COMPLETE,
    UMTS_RR_MSG_TYPE_RADIO_BEARER_RECONFIGURATION_FAILURE,
    UMTS_RR_MSG_TYPE_RADIO_BEARER_RELEASE,
    UMTS_RR_MSG_TYPE_RADIO_BEARER_RELEASE_COMPLETE,
    UMTS_RR_MSG_TYPE_RADIO_BEARER_RELEASE_FAILURE,
    UMTS_RR_MSG_TYPE_RADIO_BEARER_SETUP,
    UMTS_RR_MSG_TYPE_RADIO_BEARER_SETUP_COMPLETE,
    UMTS_RR_MSG_TYPE_RADIO_BEARER_SETUP_FAILURE,
    UMTS_RR_MSG_TYPE_RRC_CONNECTION_REJECT,
    UMTS_RR_MSG_TYPE_RRC_CONNECTION_RELEASE,
    UMTS_RR_MSG_TYPE_RRC_CONNECTION_RELEASE_COMPLETE,
    UMTS_RR_MSG_TYPE_RRC_CONNECTION_RELEASE_CCCH,
    UMTS_RR_MSG_TYPE_RRC_CONNECTION_REQUEST,
    UMTS_RR_MSG_TYPE_RRC_CONNECTION_SETUP,
    UMTS_RR_MSG_TYPE_RRC_CONNECTION_SETUP_COMPLETE,
    UMTS_RR_MSG_TYPE_RRC_STATUS,
    UMTS_RR_MSG_TYPE_SECURITY_MODE_COMMAND,
    UMTS_RR_MSG_TYPE_SECURITY_MODE_COMPLETE,
    UMTS_RR_MSG_TYPE_SECURITY_MODE_FAILURE,
    UMTS_RR_MSG_TYPE_SIGNALLING_CONNECTION_RELEASE,
    UMTS_RR_MSG_TYPE_SIGNALLING_CONNECTION_RELEASE_INDICATION,
    UMTS_RR_MSG_TYPE_SYSTEM_INFORMATION_BCH,
    UMTS_RR_MSG_TYPE_SYSTEM_INFORMATION_FACH,
    UMTS_RR_MSG_TYPE_SYSTEM_INFORMATION_CHANGE_INDICATION,
    UMTS_RR_MSG_TYPE_TRANSPORT_CHANNEL_RECONFIGURATION,
    UMTS_RR_MSG_TYPE_TRANSPORT_CHANNEL_RECONFIGURATION_COMPLETE,
    UMTS_RR_MSG_TYPE_TRANSPORT_CHANNEL_RECONFIGURATION_FAILURE,
    UMTS_RR_MSG_TYPE_TRANSPORT_FORMAT_COMBINATION_CONTROL,
    UMTS_RR_MSG_TYPE_TRANSPORT_FORMAT_COMBINATION_CONTROL_FAILURE,
    UMTS_RR_MSG_TYPE_UE_CAPABILITY_ENQUIRY,
    UMTS_RR_MSG_TYPE_UE_CAPABILITY_INFORMATION,
    UMTS_RR_MSG_TYPE_UE_CAPABILITY_INFORMATION_CONFIRM,
    UMTS_RR_MSG_TYPE_UL_DIRECT_TRANSFER,
    UMTS_RR_MSG_TYPE_UL_PHYSICAL_CHANNEL_CONTROL,
    UMTS_RR_MSG_TYPE_URA_UPDATE,
    UMTS_RR_MSG_TYPE_URA_UPDATA_CONFIRM,
    UMTS_RR_MSG_TYPE_UTRAN_MOBILITY_INFOMATION,
    UMTS_RR_MSG_TYPE_UTRAN_MOBILITY_INFORMATION_CONFIRM,
    UMTS_RR_MSG_TYPE_UTRAN_MOBILITY_INFORMATION_FAILURE,
} UmtsRRMessageType;

/// TYPE of voice call clearing
typedef enum
{
    UMTS_CALL_CLEAR_END,
    UMTS_CALL_CLEAR_DROP,
    UMTS_CALL_CLEAR_REJECT
} UmtsCallClearingType;

/// Type of System Information Block (SIB)
typedef enum
{
    UMTS_SIB_MASTER = 0,  // master information block
    UMTS_SIB_TYPE1,       // system information block type 1
    UMTS_SIB_TYPE2,       // system information block type 2
    UMTS_SIB_TYPE3,       // system information block type 3
    UMTS_SIB_TYPE4,       // system information block type 4
    UMTS_SIB_TYPE5,       // system information block type 5
    UMTS_SIB_TYPE6,       // system information block type 6
    UMTS_SIB_TYPE7,       // system information block type 7
    UMTS_SIB_DUMMY,       // not used
    UMTS_SIB_DUMMY2,      // not used
    UMTS_SIB_DUMMY3,      // not used
    UMTS_SIB_TYPE11,      // system information block type 11
    UMTS_SIB_TYPE12,      // system information block type 12
    UMTS_SIB_TYPE13,      // system information block type 13
    UMTS_SIB_TYPE13_1,    // system information block type 13-1
    UMTS_SIB_TYPE13_2,    // system information block type 13-2
    UMTS_SIB_TYPE13_3,    // system information block type 13-3
    UMTS_SIB_TYPE13_4,    // system information block type 13-4
    UMTS_SIB_TYPE14,      // system information block type 14
    UMTS_SIB_TYPE15,      // system information block type 15
    UMTS_SIB_TYPE15_1,    // system information block type 15-1
    UMTS_SIB_TYPE15_2,    // system information block type 15-2
    UMTS_SIB_TYPE15_3,    // system information block type 15-3
    UMTS_SIB_TYPE16,      // system information block type 16
    UMTS_SIB_TYPE17,      // system information block type 17
    UMTS_SIB_TYPE15_4,    // system information block type 15-4
    UMTS_SIB_TYPE18,      // system information block type 18
    UMTS_SIB_Schedule1,   // scheudling block 1
    UMTS_SIB_Schedule2,   // scheudling block 2
    UMTS_SIB_TYPE15_5,    // system information block type 15-5
    UMTS_SIB_TYPE5_bis,   // system information block type 5bis
    UMTS_SIB_EXTENSION    // extension type
} UmtsSibType;

/// RRC establishment cause
typedef enum
{
    UMTS_RRC_ESTCAUSE_CLEARED = 0,
    UMTS_RRC_ESTCAUSE_ORIGINATING_CONVERSATIONAL,
    UMTS_RRC_ESTCAUSE_ORIGINATING_STREAMING,
    UMTS_RRC_ESTCAUSE_ORIGINATING_INTERACTIVE,
    UMTS_RRC_ESTCAUSE_ORIGINATING_BACKGROUND,
    UMTS_RRC_ESTCAUSE_ORIGINATING_SUBSCRIBEDTRAFFIC,
    UMTS_RRC_ESTCAUSE_TERMINATING_CONVERSATIONAL,
    UMTS_RRC_ESTCAUSE_TERMINATING_STREAMING,
    UMTS_RRC_ESTCAUSE_TERMINATING_INTERACTIVE,
    UMTS_RRC_ESTCAUSE_TERMINATING_BACKGROUND,
    UMTS_RRC_ESTCAUSE_TERMINATING_SUBSCRIBEDTRAFFIC,
    UMTS_RRC_ESTCAUSE_REGISTRATION,
    UMTS_RRC_ESTCAUSE_DETACH,
    UMTS_RRC_ESTCAUSE_ORIGINATING_HIGHPRI_SIGNALLING,
    UMTS_RRC_ESTCAUSE_ORIGINATING_LOWPRI_SIGNALLING,
    UMTS_RRC_ESTCAUSE_CALLREESTABLISHMENT,
    UMTS_RRC_ESTCAUSE_TERMINATING_HIGHPRI_SIGNALLING,
    UMTS_RRC_ESTCAUSE_TERMINATING_LOWPRI_SIGNALLING,
    UMTS_RRC_ESTCAUSE_TERMINATING_UNKNOWN
} UmtsRrcEstCause;

/// RRC protocol error cause
typedef enum
{
    UMTS_RRC_PTCLERRCAUSE_ENCODING,
    UMTS_RRC_PTCLERRCAUSE_MSGTYPENONEXIST,
    UMTS_RRC_PTCLERRCAUSE_MSGCONFICTSTATE,
    UMTS_RRC_PTCLERRCAUSE_IENOTCOMPREHEND,
    UMTS_RRC_PTCLERRCAUSE_IEMISSING
} UmtsRrcPtclErrCause;

/// CN paging cause
typedef enum
{
    UMTS_PAGING_CAUSE_CONVERSATION_CALL,
    UMTS_PAGING_CAUSE_STREAMING_CALL,
    UMTS_PAGING_CAUSE_INTERACTIVE_CALL,
    UMTS_PAGING_CAUSE_BACKGROUND_CALL,
    UMTS_PAGING_CAUSE_HIGH_PRI_SIGNALLING,
    UMTS_PAGING_CAUSE_LOW_PRI_SIGNALLING,
    UMTS_PAGING_CAUSE_UNKNOWN
} UmtsPagingCause;

/// UE is in sevice area of not
typedef enum
{
    UMTS_UE_OUT_OF_SERVICE_AREA = 0,  // out of service area
    UMTS_UE_IN_SERVICE_AREA = 1       // in service area
} UmtsUeServiceAreaStatus;

/// RRC stats
typedef enum
{
    // idle section
    UMTS_RRC_SUBSTATE_NO_PLMN = 0,
    UMTS_RRC_SUBSTATE_PLMN_SELECTED,
    UMTS_RRC_SUBSTATE_ANY_CELL_SELECTION,
    UMTS_RRC_SUBSTATE_CAMPED_ANY_CELL,
    UMTS_RRC_SUBSTATE_SUITABLE_CELL_SELECTED,
    UMTS_RRC_SUBSTATE_CAMPED_NORMALLY,

    // connected setion
    UMTS_RRC_SUBSTATE_URA_PCH,            // UTRA URA DCH mode
    UMTS_RRC_SUBSTATE_CELL_PCH,           // UTRA CELL PCH mode
    UMTS_RRC_SUBSTATE_CELL_FACH,          // UTRA CELL FACH mode
    UMTS_RRC_SUBSTATE_CELL_DCH            // UTRA CELL DCH mode
} UmtsRrcSubState;

/// RRC stats
typedef enum
{
    UMTS_RRC_STATE_IDLE = 0,
    UMTS_RRC_STATE_CONNECTED,
} UmtsRrcState;

/// RAB states
typedef enum
{
    UMTS_RAB_STATE_NULL,
    UMTS_RAB_STATE_ESTREQQUEUED,
    UMTS_RAB_STATE_WAITFORNODEBSETUP,
    UMTS_RAB_STATE_WAITFORACTVSETRED,        //All NODEB responses has come
                                             //but need to remove some 
                                             //active set members

    //UMTS_RAB_STATE_WAITFORNODEBSETUP_AND_RELREQQUEUED,
    UMTS_RAB_STATE_WAITFORUESETUP,
    //UMTS_RAB_STATE_WAITFORUESETUP_AND_RELREQQUEUED,
    UMTS_RAB_STATE_RELREQQUEUED,
    UMTS_RAB_STATE_WAITFORUEREL,
    UMTS_RAB_STATE_ESTED
}UmtsRabState;

/// MM states for PS domain
typedef enum
{
    UMTS_PMM_DETACHED,
    UMTS_PMM_IDLE,
    UMTS_PMM_CONNECTED
} UmtsPmmState;

/// GMM Service Type
typedef enum
{
    UMTS_GMM_SERVICE_TYPE_SIGNALLING = 0,
    UMTS_GMM_SERVICE_TYPE_DATA,
    UMTS_GMM_SERVICE_TYPE_PAGINGRESP
} UmtsGmmServiceType;

/// Type of various MM signalling messages
typedef enum
{
    UMTS_MM_MSG_TYPE_IMSI_DETACH_INDICATION = 0x1,
    UMTS_MM_MSG_TYPE_LOCATION_UPDATING_ACCEPT = 0x2,
    UMTS_MM_MSG_TYPE_LOCATION_UPDATING_REJECT = 0x4,
    UMTS_MM_MSG_TYPE_LOCATION_UPDATING_REQUEST = 0x8,
    UMTS_MM_MSG_TYPE_CM_SERVICE_ACCEPT = 0x21,
    UMTS_MM_MSG_TYPE_CM_SERVICE_REJECT = 0x22,
    UMTS_MM_MSG_TYPE_CM_SERVICE_ABORT = 0x23,
    UMTS_MM_MSG_TYPE_CM_SERVICE_REQUEST = 0x24,
    UMTS_MM_MSG_TYPE_CM_SERVICE_PROMPT = 0x25,
    UMTS_MM_MSG_TYPE_CM_REESTABLISHMENT_REQUEST = 0x28,
    UMTS_MM_MSG_TYPE_ABORT = 0x29,
    UMTS_MM_MSG_TYPE_MM_NULL = 0x30,
    UMTS_MM_MSG_TYPE_MM_STATUS = 0x31,
    UMTS_MM_MSG_TYPE_MM_INFORMATION = 0x32,
    UMTS_MM_MSG_TYPE_PAGING_RESPONSE
} UmtsMmMessageType;

/// Definition of the types of location update
typedef enum
{
    UMTS_NORMAL_LOCATION_UPDATING = 0,
    UMTS_PERIODIC_LOCATION_UPDATING = 1,
    UMTS_IMSI_ATTACH = 2,
}UmtsLocUpdateType;

/// Definition of the types of location update
typedef enum
{
    UMTS_PERIOD_RA_UPDATE_UNIT_2SEC = 0,
    UMTS_PERIOD_RA_UPDATE_UNIT_MIN = 1,
    UMTS_PERIOD_RA_UPDATE_UNIT_DECIHOUR = 3,
    UMTS_PERIOD_RA_UPDATE_UNIT_DEACTIVATE = 7,
}UmtsPeriodLocUpUnitType;
/// Definition of the types of Attach Type
typedef enum
{
    UMTS_GPRS_ATTACH_ONLY_NO_FOLLOW_ON = 0x9,
    UMTS_COMBINED_ATTACH_NO_FOLLOW_ON = 0xb,
    UMTS_GPRS_ATTACH_ONLY_FOLLOW_ON = 0x1,
    UMTS_COMBINED_ATTACH_FOLLOW_ON = 0x3,
}UmtsAttachType;
/// Definition of the types of Attach result Type
typedef UmtsAttachType UmtsAttachResult;
/// Type of various GMM signalling messages
typedef enum
{
    UMTS_GMM_MSG_TYPE_ATTACH_REQUEST = 0x1,
    UMTS_GMM_MSG_TYPE_ATTACH_ACCEPT = 0x2,
    UMTS_GMM_MSG_TYPE_ATTACH_COMPLETE = 0x3,
    UMTS_GMM_MSG_TYPE_ATTACH_REJECT = 0x4,
    UMTS_GMM_MSG_TYPE_DETACH_REQUEST = 0x5,
    UMTS_GMM_MSG_TYPE_DETACH_ACCEPT = 0x6,
    UMTS_GMM_MSG_TYPE_ROUTING_AREA_UPDATE_REQUEST = 0x8,
    UMTS_GMM_MSG_TYPE_ROUTING_AREA_UPDATE_ACCEPT = 0x9,
    UMTS_GMM_MSG_TYPE_ROUTING_AREA_UPDATE_COMPLETE = 0xA,
    UMTS_GMM_MSG_TYPE_ROUTING_AREA_UPDATE_REJECT = 0xB,
    UMTS_GMM_MSG_TYPE_SERVICE_REQUEST = 0xC,
    UMTS_GMM_MSG_TYPE_SERVICE_ACCEPT = 0xD,
    UMTS_GMM_MSG_TYPE_SERVICE_REJECT = 0xE,
    UMTS_GMM_MSG_TYPE_GMM_STATUS = 0x20,
    UMTS_GMM_MSG_TYPE_GMM_INFORMATION = 0x21
} UmtsGMmMessageType;

/// event type of the MM state update
typedef enum
{
    UMTS_MM_UPDATE_PowerOn,
    UMTS_MM_UPDATE_NoCellSelected,
    UMTS_MM_UPDATE_CellSelected,
}UmtsMmStateUpdateEventType;

/// event type of the GMM state update
typedef enum
{
    UMTS_GMM_UPDATE_EnableGprsMode,
    UMTS_GMM_UPDATE_DisableGprsMode,
    UMTS_GMM_UPDATE_AttachRequested,
    UMTS_GMM_UPDATE_AttachAccepted,
    UMTS_GMM_UPDATE_AttachRejected,
    UMTS_GMM_UPDATE_NwInitDetachRequested,
    UMTS_GMM_UPDATE_LowerLayerFailure,
    UMTS_GMM_UPDATE_DetachRequestedPowerOff,
    UMTS_GMM_UPDATE_DetachAccepted,
    UMTS_GMM_UPDATE_RauRejected, // routing area update
    UMTS_GMM_UPDATE_ImplicitDetach,
    UMTS_GMM_UPDATE_DetachRequestedNotPowerOff,
    UMTS_GMM_UPDATE_RauAccepted,
    UMTS_GMM_UPDATE_RauRejectCause13,
    UMTS_GMM_UPDATE_RauRejectCause15,
    UMTS_GMM_UPDATE_RauRequested,
    UMTS_GMM_UPDATE_SrAccepted,
    UMTS_GMM_UPDATE_SrRequestAccepted,
    UMTS_GMM_UPDATE_SrFailure,

    UMTS_GMM_UPDATE_MsInitDetachRequested,
    UMTS_GMM_UPDATE_AttachProcSucc,
    UMTS_GMM_UPDATE_CommonProcRequested,
    UMTS_GMM_UPDATE_CommonProcFailed,
    UMTS_GMM_UPDATE_COmmonProcSucc,
}UmtsGMmStateUpdateEventType;

/// Type of various CC signalling messages
typedef enum
{
    UMTS_CC_MSG_TYPE_ALERTING = 0x1,
    UMTS_CC_MSG_TYPE_CALL_CONFIRMED = 0x8 ,
    UMTS_CC_MSG_TYPE_CALL_PROCEEDING = 0x2,
    UMTS_CC_MSG_TYPE_CONNECT = 0x7,
    UMTS_CC_MSG_TYPE_CONNECT_ACKNOWLEDGE = 0xF,
    UMTS_CC_MSG_TYPE_EMERGENCY_SETUP = 0xE ,
    UMTS_CC_MSG_TYPE_PROGRESS = 0x3,
    UMTS_CC_MSG_TYPE_CC_ESTABLISHMENT = 0x4,
    UMTS_CC_MSG_TYPE_CC_ESTABLISHMENT_CONFIRMED = 0x6,
    UMTS_CC_MSG_TYPE_RECALL = 0xB,
    UMTS_CC_MSG_TYPE_START_CC = 0x9,
    UMTS_CC_MSG_TYPE_SETUP = 0x5,
    UMTS_CC_MSG_TYPE_MODIFY = 0x17,
    UMTS_CC_MSG_TYPE_MODIFY_COMPLETE = 0x1F,
    UMTS_CC_MSG_TYPE_MODIFY_REJECT = 0x13,
    UMTS_CC_MSG_TYPE_HOLD = 0x18,
    UMTS_CC_MSG_TYPE_HOLD_ACKNOWLEGE = 0x19,
    UMTS_CC_MSG_TYPE_HOLD_REJECT = 0x1A,
    UMTS_CC_MSG_TYPE_RETRIEVE = 0x1C,
    UMTS_CC_MSG_TYPE_RETRIEVE_ACKNOWLEGE = 0x1D,
    UMTS_CC_MSG_TYPE_RETRIEVE_REJECT = 0x1E,
    UMTS_CC_MSG_TYPE_DISCONNECT = 0x25,
    UMTS_CC_MSG_TYPE_RELEASE = 0x2D,
    UMTS_CC_MSG_TYPE_RELEASE_COMPLETE = 0x2A,
    UMTS_CC_MSG_TYPE_NOTIFY = 0x3E,
    UMTS_CC_MSG_TYPE_STATUS = 0x3D
} UmtsCcMessageType;

/// Type of various SM signalling messages
typedef enum
{
    UMTS_SM_MSG_TYPE_ACTIVATE_PDP_REQUEST = 0x81,
    UMTS_SM_MSG_TYPE_ACTIVATE_PDP_ACCEPT = 0x82,
    UMTS_SM_MSG_TYPE_ACTIVATE_PDP_REJECT = 0x83,
    UMTS_SM_MSG_TYPE_REQUEST_PDP_ACTIVATION = 0x84,
    UMTS_SM_MSG_TYPE_REQUEST_PDP_ACTIVATION_REJ = 0x85,
    UMTS_SM_MSG_TYPE_DEACTIVATE_PDP_REQUEST = 0x86,
    UMTS_SM_MSG_TYPE_DEACTIVATE_PDP_ACCEPT = 0x87,
    UMTS_SM_MSG_TYPE_MODIFY_PDP_REQ_NWTOMS = 0x88,
    UMTS_SM_MSG_TYPE_MODIFY_PDP_ACCEPT_MSTONW = 0x89,
    UMTS_SM_MSG_TYPE_MODIFY_PDP_REQ_MSTONW = 0x8A,
    UMTS_SM_MSG_TYPE_MODIFY_PDP_ACCEPT_NWTOMS = 0x8B,
    UMTS_SM_MSG_TYPE_MODIFY_PDP_REJ = 0x8C,
    UMTS_SM_MSG_TYPE_ACTIVATE_SECONDARY_PDP_REQ = 0x8D,
    UMTS_SM_MSG_TYPE_ACTIVATE_SECONDARY_PDP_ACCEPT = 0x8E,
    UMTS_SM_MSG_TYPE_ACTIVATE_SECONDARY_PDP_REJECT = 0x8F,
    UMTS_SM_MSG_TYPE_SM_STATUS = 0x95,
    UMTS_SM_MSG_TYPE_REQUEST_SECONDARY_PDP_ACTIVATION = 0x9B,
    UMTS_SM_MSG_TYPE_REQUEST_SECONDARY_PDP_ACTIVATION_REJ = 0x9C
} UmtsSmMessageType;

/// causes for deactivation,rejection
typedef enum
{
    UMTS_SM_CAUSE_LLC_SNDCP_FAILURE = 25,
    UMTS_SM_CAUSE_INSUFFICINT_RESOURCES = 26,
    UMTS_SM_CAUSE_REGUALR_DEACTIVATION = 36,
    UMTS_SM_CAUSE_QOS_NOT_ACCEPTED = 37,

    // more
}UmtsSmCauseType;

/// UTRAN system information combination type
typedef enum
{
    UMTS_SIB_NO_SEG,
    UMTS_SIB_FIRST_SEG,
    UMTS_SIB_SUB_SEG,
    UMTS_SIB_LAST_SEG,
    UMTS_SIB_LAST_FIRST_SEG,
    UMTS_SIB_LAST_COMPLETE,
    UMTS_SIB_LAST_COMPLETE_FIRST,
    UMTS_SIB_COMPLETE,
    UMTS_SIB_COMPLETE_FIRST,
    UMTS_SIB_COMPLETE_215_226,
    UMTS_SIB_LAST_SEG_215_222
}UmtsSibCombinationType;

/// Appliaiton's Src Dest Type, UE init/ network init
typedef enum
{
    UMTS_FLOW_MOBILE_ORIGINATED,
    UMTS_FLOW_NETWORK_ORIGINATED,
}UmtsFlowSrcDestType;

/// transaction start index at UE
#define UMTS_LAYER3_UE_MOBILE_START    0

/// transaction start index at UE
#define UMTS_LAYER3_UE_MOBILE_END    6

/// transaction start index at UE
#define UMTS_LAYER3_NETWORK_MOBILE_START    8

/// transaction start index at UE
#define UMTS_LAYER3_NETWORK_MOBILE_END    15

/// Message Type of GTP
typedef enum
{
    // more
    UMTS_GTP_CREATE_PDP_CONTEXT_REQUEST = 16,
    UMTS_GTP_CREATE_PDP_CONTEXT_RESPONSE = 17,
    UMTS_GTP_UPDATE_PDP_CONTEXT_REQUEST = 18,
    UMTS_GTP_UPDATE_PDP_CONTEXT_RESPONSE = 19,
    UMTS_GTP_DELETE_PDP_CONTEXT_REQUEST = 20,
    UMTS_GTP_DELETE_PDP_CONTEXT_RESPONSE = 21,

    // more
    UMTS_GTP_PDU_NOTIFICATION_REQUEST = 27,
    UMTS_GTP_PDU_NOTIFICATION_RESPONSE = 28,
    UMTS_GTP_PDU_NOTIFICATION_REJECT_REQUEST = 29,
    UMTS_GTP_PDU_NOTIFICATION_REJECT_RESPONSE = 30,
   // more
    UMTS_GTP_G_PDU = 255,
}UmtsGtpMsgType;

/// casue Type of GTP
typedef enum
{
    UMTS_GTP_REQUEST_ACCEPTED,
    UMTS_GTP_REQUEST_REJECTED,
    UMTS_GTP_REQUEST_DROPPED,
}UmtsGtpCauseType;

//******************************************************************
// TLV enum
//******************************************************************

/// TLV type in system informatIon blOck type 3
typedef enum
{
    TLV_SIB_PRACH_InfoList,
    TLV_SIB_SCCPCH_InfoList,
    // more
}UmtsSib3TlvType;

/// TLV type in prach info list
typedef enum
{
    TLV_PRACH_Info,
    TLV_PRACH_RachTfs,
    TLV_PRACH_Rachtfcs,
    TLV_PRACH_AscSetting,
    TLV_PRACH_AcAscMapping,
    TLV_PRACH_AichInfo,

    // more
}UmtspPrachInfoTlvType;

/// TLV type in Aich info list
typedef enum
{
    TLV_AICH_ChannelCode,
    TLV_AICH_SttdInd,
    TLV_AICH_TxTiming
}UmtspAichInfoTlvType;

/// IE types in RRC CONNECTION SETUP
typedef enum
{
    TLV_RRCCONNSETUP_INITUEID = 1,
    TLV_RRCCONNSETUP_RRCSTATEINDI,
    TLV_RRCCONNSETUP_SPECMODE,
    TLV_RRCCONNSETUP_PRECONFIGMODE
} UmtsRrcConnSetupTlvType;

/// Specification mode in RRC CONNECTION REQUEST
typedef enum
{
    UMTS_RRCCONNSETUP_SPECMODE_PRECONFIG,
    UMTS_RRCCONNSETUP_SPECMODE_COMPLETE
} UmtsRrcConnSetupSpecMode;

/// Preconfiguration mode in RRC CONNECTION REQUEST
typedef enum
{
    UMTS_RRCCONNSETUP_PRECONFIGMODE_PREDEFINE,
    UMTS_RRCCONNSETUP_PRECONFIGMODE_DEFAULT
} UmtsRrcConnSetupPreConfigMode;

/// IE types in Radio Bearer Setup message
typedef enum
{
    TLV_RBSETUP_RAB_SETUP,
    TLV_RBSETUP_UL_TRCH_DELETED,
    TLV_RBSETUP_UL_TRCH_ADDRECONFIGED,
    TLV_RBSETUP_DL_TRCH_DELETED,
    TLV_RBSETUP_DL_TRCH_ADDRECONFIGED,
    TLV_RBSETUP_UL_DPCH_INFO,
    TLV_RBSETUP_DL_RL_COMMON,
    TLV_RBSETUP_DL_DPCH_INFO
} UmtsRbSetupTlvType;

/// IE types in RAB ASSIGNMENT RESPONSE message
typedef enum
{
    TLV_RABASSGTRES_SETUP,
    TLV_RABASSGTRES_RELEASE,
    TLV_RABASSGTRES_FAIL_SETUP,
    TLV_RABASSGTRES_FAIL_RELEASE
} UmtsRabAssgtResTlvType;

/// TLV type downlink DPCH info for each radio link
typedef enum
{
    TLV_DL_CHCODE_LIST
}UmtsDlDpchTlvType;

/// IE types in IE RB information to setup
typedef enum
{
    TLV_RBINFOSETUP_PDCPINFO,
    TLV_RBINFOSETUP_RLCINFO,
    TLV_RBINFOSETUP_RBMAPPINGINFO
} UmtsRbInfoSetupTlvType;

//*****************************************************************
// data structure for UMTS layer3 implementation
//*****************************************************************

/////////
// APP
/////////

/// Structure of the applicaiton classifier
typedef struct
{
    // domain info
    // whether it is a CS/PS applicaiton
    UmtsLayer3CnDomainId domainInfo;

    unsigned char ipProtocol; // IP protocol, eg. UDP/TCP/ICMP...

    Address srcAddr;    // Address of the source node (IPv4, IPv6 or ATM)
    Address dstAddr;    // Address of the dest node (IPv4, IPv6 or ATM)

    unsigned short srcPort;  // Port number at source node
    unsigned short dstPort;  // Port number at dest node
}UmtsLayer3FlowClassifier;

/// MM conection data structure
struct UmtsLayer3MmConn
{
    UmtsLayer3PD pd;    //protocol discriminator
    unsigned char ti;   //transction ID
    explicit UmtsLayer3MmConn(UmtsLayer3PD protDscm,
                              unsigned char transctId)
                              :pd(protDscm), ti(transctId)
    {
    }

    bool operator== (const UmtsLayer3MmConn& mmConn) const
    {
        return mmConn.pd == pd && mmConn.ti == ti;
    }
};

/// Containter containing active MM connections
typedef std::list<UmtsLayer3MmConn*> UmtsMmConnCont;

/// Queue containing MM connections to be requested
typedef std::deque<UmtsLayer3MmConn*> UmtsMmConnQueue;

/// Info of a NodeB
typedef struct umts_layer3_nodeb_info_str
{
    NodeId nodeId;      // node ID
    Address nodeAddr;   // node address
    int interfaceIndex; // interface to this node
} UmtsLayer3NodebInfo;

/// Info of a RNC node
typedef struct umts_layer3_rnc_info_str
{
    NodeId nodeId;      // node ID
    Address nodeAddr;   // node address
    int interfaceIndex; // interface to this node
} UmtsLayer3RncInfo;

/// Info of a GSN node
typedef struct umts_layer3_gsn_info_str
{
    NodeId nodeId;      // node ID
    Address nodeAddr;   // node address
    int interfaceIndex; // interface to this node
} UmtsLayer3GsnInfo;

/// Info of IP addresses in PLMN
typedef struct umts_layer3_address_info_str
{
    NodeAddress subnetAddress;
    int numHostBits;
} UmtsLayer3AddressInfo;

/// Scheduling information for system information block
typedef struct umts_sib_scheduling_info_str
{
    char sibType;  // type of the SIB, has values of UmtsSibType
    char segCount; // # of segments, from 1 to 16
    int sibRep;    // repetition period of the SIB in frames
    int sibPos;    // position of the first segment
    // list of SIB_OFFs for subsequent segments, use default value 2
} UmtsSibSchedulingInfo;

/// Structure of the Master Information Block
typedef struct umts_master_info_block_str
{
    unsigned char valueTag;  // MIB Value Tag
    UmtsPlmnType plmnType;   // suppoted PLMN types. Only GSM_MAP now
    UmtsPlmnIdentity plmnId; // PLMN type is GSM_MAP, identify PLMNs
    UInt32 cellId;       // canned for implementation purpose
    // rest part is a list of scheduling info of SIBs
} UmtsMasterInfoBlock;

/// Structure of System Info Block Type 1
typedef struct umts_sib_type1_str
{
    UInt32 cellId;       // canned for implementation purpose
} UmtsSibType1;

/// Structure of System Info Block Type 2
typedef struct umts_sib_type2_str
{
    // assume a cell only belongs to one URA
    UInt16 regAreaId;

} UmtsSibType2;

/// Structure of System Info Block Type 3
typedef struct umts_sib_type3_str
{
    char sib4Indicator;  // TRUE indicates that SIB4 is bcast in the cell
    int ulChIndex;

    // cell selection and reselection info
    char qualityMea;     // Cell selection and reselection quality measure
    int Qqualmin;        // Ec/N0, [dB]
    int Qrxlevmin;       // RSCP, [dBm]
    int Qhyst1;          // [dB]
    int Treselection;    // [s]
    int maxUlTxPower;    // Maximum allowed UL TX power
} UmtsSibType3;

/// Structure of System Info Block Type 5
typedef struct umts_sib_type5_str
{
    char sib6Indicator;  // TRUE indicates that SIB6 is bcast in the cell

    // PhyCh info
    char pichPowerOffset; // (-10 ~ 5)db
    char aichPowerOffset; // (-22 ~ 3)db

    // PRACH info
    // S-CCPCH info
} UmtsSibType5;

/// Header Structure of System Info Block
typedef struct umts_sib_blk_complete_header_str
{
    unsigned char sibType;
} UmtsSibBlkCompleteHeader;

/// Header Structure of System Info Block
typedef struct umts_sib_blk_non_complete_header_str
{
    unsigned char sibType;
    unsigned char segCountIndex;
}UmtsSibBlkNonCompleteHeader;

//*****************************************************************
// data structure for UMTS layer3 signalling information element
//*****************************************************************
/// Timer message for layer3
typedef struct umts_layer3_timer_str
{
    UmtsLayer3TimerType timerType;
} UmtsLayer3Timer;

/// RAB/GTP tunnel mapping info
struct UmtsRabTunnelMap
{
    Address ggsnAddr;
    int ulTeId;
    int dlTeId;
};

/// RAB/CS flow mapping info
struct UmtsRabFlowMap
{
    UmtsLayer3PD   pd;
    char           ti;
};

/// A union for both kinds of RAB mapping info
typedef union
{
    UmtsRabTunnelMap tunnelMap;
    UmtsRabFlowMap   flowMap;
} UmtsRabConnMap;

/// Radio Access Bearer info stored in RNC
struct  UmtsRrcRabInfo
{
    UmtsRabState            state;
    UmtsLayer3CnDomainId    cnDomainId;
    char                    rbIds[UMTS_MAX_RB_PER_RAB];
    UmtsRabConnMap          connMap;
    UmtsRABServiceInfo      dlRabPara;
    UmtsRABServiceInfo      ulRabPara;
};

/// Radio Access Bearer info stored in UE
struct  UmtsUeRabInfo
{
    UmtsLayer3CnDomainId    cnDomainId;
    char                    rbIds[UMTS_MAX_RB_PER_RAB];
};

/// Signalling Connection info in a UE
struct UmtsRrcSignalConn
{
    UmtsLayer3CnDomainId    domain;
    BOOL                    connected;
};

/// Configuring parameters for setuping SRB1-3
/// of an UE at the NodeB.
struct UmtsNodebSrbConfigPara
{
    UInt32 uePriSc;
    unsigned char dlDcchId[3];
    unsigned char dlDchId;
    unsigned char dlDpdchId;
    UmtsSpreadFactor sfFactor;
    unsigned int chCode;
};

/// Response info from Nodeb during Pending RAB setup state
struct  UmtsRrcNodebResInfo
{
    UInt32              cellId;         //Nodeb Cell ID;
    int                 numDlDpdch;     //Num of DL physical channel info
    UmtsSpreadFactor    sfFactor[UMTS_MAX_DLDPDCH_PER_RAB];
    unsigned int        chCode[UMTS_MAX_DLDPDCH_PER_RAB];
    UmtsRrcNodebResInfo()
    {
        memset(this, 0, sizeof(UmtsRrcNodebResInfo));
    }
};

// struct      :: UmtsNodebRbSetupPara
// description :: configuring parameters for setting up
/// a rb
struct UmtsNodebRbSetupPara
{
    NodeId srncId;
    UInt32 cellId;
    NodeId ueId;
    UInt32 uePriSc;
    char rbId;
    char numDpdch;
    unsigned char dlDpdchId[UMTS_MAX_DLDPDCH_PER_RAB];
    UmtsSpreadFactor sfFactor[UMTS_MAX_DLDPDCH_PER_RAB];
    unsigned int chCode[UMTS_MAX_DLDPDCH_PER_RAB];
    unsigned char dlDchId;
    unsigned char dlDtchId;
    char logPriority;
    UmtsRlcEntityType rlcType;
    UmtsRlcConfig     rlcConfig;

    // transport format
    // use single format
    UmtsTransportFormat transFormat;

    // UL BW alloc
    int ulBwAlloc;

    // DL BW alloc
    int dlBwAlloc;

    // HSDPA related
    BOOL hsdpaRb;

    // either add or delete a radio link
    BOOL rlSetupNeeded;  //

    UmtsNodebRbSetupPara()
    {
       memset(this, 0, sizeof(UmtsNodebRbSetupPara));
    }

    UmtsNodebRbSetupPara
        (NodeId srnc,
         NodeId ue,
         char rb,
         UmtsRlcEntityType  rlcTp,
         const UmtsRlcConfig&  rlcCfg,
         const UmtsTransportFormat& trFormat,
         int ulBw,
         int dlBw)
         : srncId(srnc), ueId(ue),
           rbId(rb), rlcType(rlcTp), rlcConfig(rlcCfg),
           transFormat(trFormat), ulBwAlloc(ulBw), dlBwAlloc(dlBw)
    {
        cellId = 0;
        numDpdch = 0;
        memset(dlDpdchId, 0, sizeof(char)*UMTS_MAX_DLDPDCH_PER_RAB);
        memset(sfFactor, 0,
            sizeof(UmtsSpreadFactor)*UMTS_MAX_DLDPDCH_PER_RAB);
        memset(chCode, 0, sizeof(unsigned int)*UMTS_MAX_DLDPDCH_PER_RAB);
        dlDchId = 0;
        dlDtchId = 0;
        logPriority = 0;
        rlSetupNeeded = FALSE;
        if (trFormat.trChType == UMTS_TRANSPORT_HSDSCH)
        {
            hsdpaRb = TRUE;
        }
        else
        {
            hsdpaRb = FALSE;
        }
    }

    UmtsNodebRbSetupPara
        (NodeId srnc,
         NodeId ue,
         char rb,
         UmtsRlcEntityType  rlcTp,
         const UmtsRlcConfig&  rlcCfg)
         : srncId(srnc), ueId(ue),
           rbId(rb), rlcType(rlcTp), rlcConfig(rlcCfg)
    {
        cellId = 0;
        numDpdch = 0;
        memset(dlDpdchId, 0, sizeof(char)*UMTS_MAX_DLDPDCH_PER_RAB);
        memset(sfFactor, 0,
            sizeof(UmtsSpreadFactor)*UMTS_MAX_DLDPDCH_PER_RAB);
        memset(chCode, 0, sizeof(unsigned int)*UMTS_MAX_DLDPDCH_PER_RAB);
        dlDchId = 0;
        dlDtchId = 0;
        logPriority = 0;
        rlSetupNeeded = FALSE;
        hsdpaRb = FALSE;
    }
};

// struct      :: UmtsUeRbSetupPara
// description :: configuring parameters for setting up
/// a rb
struct UmtsUeRbSetupPara
{
    char rabId;
    UmtsLayer3CnDomainId domain;
    char rbId;
    char logPriority;
    char ulDtchId;
    char ulDchId;
    char ulDpdchId;
    //UmtsSpreadFactor sfFactor;
    unsigned char numUlDPdch;

    UmtsSpreadFactor sfFactor;
    UmtsRlcEntityType rlcType;
    UmtsRlcConfig     rlcConfig;

    // transport format
    UmtsTransportFormat transFormat;

    // HSDPA RB
    BOOL hsdpaRb;
    BOOL rlSetupNeeded;

    UmtsUeRbSetupPara() { }

    UmtsUeRbSetupPara(char  rab,
                      UmtsLayer3CnDomainId domainId,
                      UmtsRlcEntityType    rlcTp,
                      UmtsRlcConfig        rlcCfg,
                      const UmtsTransportFormat& trFormat)
                      : rabId(rab), domain(domainId),
                        rlcType(rlcTp), rlcConfig(rlcCfg),
                        transFormat(trFormat)
    {
        rlSetupNeeded = FALSE;
        if (trFormat.trChType == UMTS_TRANSPORT_HSDSCH)
        {
            hsdpaRb = TRUE;
        }
        else
        {
            hsdpaRb = FALSE;
        }
    }
};

/// Cell Switch Info
struct UmtsCellSwitchInfo
{
    UInt64    contextBufSize;            //Cell switch context buffer size
};

/// Catched cell info for fast looking up
/// parent RNC ID of each cell
struct UmtsCellCacheInfo
{
    NodeId  nodebId;
    NodeId  rncId;
    BOOL    hlrUpdated;

    UmtsCellCacheInfo() { }

    UmtsCellCacheInfo(NodeId nodeb, NodeId rnc)
                    : nodebId(nodeb), rncId(rnc)
    {
        hlrUpdated = FALSE;
    }
};

/// Catched RNC info for fast looking up
/// parent SGSN info
struct UmtsRncCacheInfo
{
    NodeId  sgsnId;
    Address sgsnAddr;

    UmtsRncCacheInfo( ) { }

    UmtsRncCacheInfo(NodeId id,
                     const Address& addr)
                    : sgsnId(id), sgsnAddr(addr)
    { }

    UmtsRncCacheInfo(NodeId id)
                    : sgsnId(id)
    {
        memset(&sgsnAddr, 0, sizeof(sgsnAddr));
    }
};

/// Type containing the Maps between a cell ID
/// and its position info (RNC and SGSN)
typedef std::map<UInt32, UmtsCellCacheInfo*> UmtsCellCacheMap;

/// Type containing the Maps between a RNC ID
/// and its position info (SGSN)
typedef std::map<NodeId, UmtsRncCacheInfo*> UmtsRncCacheMap;

//*****************************************************************
// structure to store the information conveyed in system info block 5
//*****************************************************************

typedef struct
{
    unsigned char padding;
}UmtsRachTfs;

typedef struct
{
    unsigned char padding;
}UmtsRachTfcs;

typedef struct
{
    unsigned char accessClass; // 0 -15
    unsigned char accessSericeClass; // 0- 7
}UmtsAcAscMap;

// STRUCTURE   :: UmtsPrachPartition
/// the asc mapping defined in PRACH partitions
typedef struct
{
    unsigned char ascIndex;
    unsigned char sigStartIndex;
    unsigned char sigEndIndex;
    UInt16 assignedSubCh; // 3 repeats of 4 bits
    double persistFactor;
}UmtsPrachPartition;

typedef struct
{
    unsigned char prachIndex; // channel index
    unsigned char trChId; // transport channel identity

    // For FDD only
    UInt16 availSig; // bit to indicate the available signiture
    UmtsSpreadFactor minSpreadFactor;
    unsigned char preambleScCode; // preamble scrambling code
    double punctureLimit;
    UInt16 avilSubChannel; // bit to inidcate the available subchannel

    std::list<UmtsRachTfs*>* rachTfs;
    std::list<UmtsRachTfcs*>* rachTfcs;
    std::list<UmtsPrachPartition*>* prachPartition;
    std::list<UmtsAcAscMap*>* acAscMapping;

    int consVal; // -35 - -10 dbm
    // prachPowerOffset has two part as follows
    unsigned char powerRampStep; // 1 - 8 db
    unsigned char maxPreambleRetran; // 64

    unsigned char Mmax; // Maximum number of preamble cycles
    unsigned char NB01min; // Sets lower bound for random back-off
    unsigned char NB01max; // Sets upper bound for random back-off
}UmtsPrachInfo;

typedef struct
{
    unsigned char ChannelCode; // 256  fixed
    BOOL sttdInd; // STTD indication
    unsigned char TxTiming;
    char powerOffset; // -22 - +5 db
}UmtsAichInfo;

typedef struct
{
    char powerOffset; // -10 - +5 db
}UmtsPichInfo;

typedef  struct
{
    int priCpichTxPower; // -10 - 50 dbm
}UmtsCpichInfo;

typedef struct
{
    unsigned char padding;
}UmtsPccpchInfo;

typedef struct
{
    unsigned char padding;
}UmtsSccpchInfo;

typedef struct
{
    UmtsPrachInfo prachInfo;

    //UmtsAichInfo aichInfo;
}UmtsPrachSystemInfo;

//*****************************************************************
//  Some packet format used by UMTS layer3
//  Msgs between differnt types of nodes
//******************************************************************

/// Standard header of backbone messages
struct UmtsBackboneHeader
{
    char   msgType;
    char   infoSize;
};

#define UMTS_GTP_HEADER_FIRST_BYTE 0x09

/// Standard header of GTP-U messages
struct UmtsGtpHeader
{

    unsigned char version: 3, // 001
                  flagPT: 1, // 0 GTP'; 1 GTP
                  flagE : 1, // extension header indicatior
                  flagS : 1, // sequence number flag
                  flagNPDU : 1; // N-PDU flag
    unsigned char msgType; // GTP msg Type
    UInt16 length; // the lenght of the msg ecept the 8 mandatory header
    UInt32 teId; // tunneling id
};

/// the header of NBAP messages
struct UmtsNbapHeader
{
    UInt8   transctId;      //transaction ID
    UInt8   padding1;
    UInt8   padding2;
    UInt8   padding3;
    NodeId  ueId;           //UE Id
};

/// Standard header of Layer 3 messages
typedef struct umts_layer3_header_str
{
    unsigned char tiSpd : 4,  // TI or Sub PD
                  pd    : 4;  // Protocol Descriptor (PD)
    unsigned char msgType;    // type of the message
    UInt8         padding1;
    UInt8         padding2;
} UmtsLayer3Header;

/// The layer3 hello packet is used to discover connectivity
/// This is a special design used to simplify user config.
/// Users don't need to configure the connectivity. Each type
/// of node will discover their neighbor nodes
typedef struct umts_layer3_hello_packet_str
{
    CellularNodeType nodeType;
    NodeId nodeId;
} UmtsLayer3HelloPacket;

/////////
// GTP related strucute
////////

////////
// MM /GMM related
////////

/// Structure for routing update request
typedef struct umts_layer3_routing_update_request_str
{
    unsigned char updateType : 4,  // Update type
                  gprsCipherKeySeqNum : 4; // GPRS Ciphering Key Seq Number
    UmtsRoutingAreaId oldRai;      // Old routing area identification

    // next is MS Radio Access capability in LV format

    // next are optional fields in TV or TLV format
} UmtsLayer3RoutingUpdateRequest;

/// Structure for routing update request
typedef struct umts_layer3_routing_update_accept_str
{
    unsigned char forceStandby : 4,  // Update type
                  updateResult : 4;  // GPRS Ciphering Key Seq Number
    UmtsRoutingAreaId newRai;        // Old routing area identification
    unsigned char periodicRaUpTime;  // periodic RA  update timer
    NodeId        rncId;             // RNC ID
    // next is MS Radio Access capability in LV format

    // next are optional fields in TV or TLV format
} UmtsLayer3RoutingUpdateAccept;

/// Structure for GPRS attach request
typedef struct
{
    // MS network capability omit in this implementation
    unsigned char attachType : 4,  // Update type
                  gprsCipherKeySeqNum : 4; // GPRS Ciphering Key Seq Number
    // skip DRX
    // P-TMSI IMSI
    // only IMSI is used

    // old routing area id
    // MS random access capability

}UmtsLayer3AttachRequest;

/// Structure for GPRS attach accept
typedef struct
{
    // MS network capability omit in this implementation
    unsigned char attachResult : 4,  // Update type
                  gprsCipherKeySeqNum : 4; // GPRS Ciphering Key Seq Number
    // skip DRX
    // P-TMSI IMSI
    // only IMSI is used

    // new routing area id
    // MS random access capability

}UmtsLayer3AttachAccept;

//////
// CC/SM related
//////

/// Structure for SERIVCE REQUEST
typedef struct
{
    unsigned char serviceType : 4,  // service Type
                                    // 000 signalling;
                                    // 001 data;
                                    // 010 page response;
                                    // 011 MBMS service reception
                  gprsCipherKeySeqNum : 4; // GPRS Ciphering Key Seq Number
    // P-TMSI LV

}UmtsLayer3ServiceRequest;

/// no specific Structure for SERIVCE ACCEPT

/// Structure for activate PDP COntext request
typedef struct
{
    unsigned char spare1 : 4,
                  nsapi : 4;
    unsigned char spare2 : 4,
                  sapi : 4;
    //QoS info LV (default 13)
    unsigned char qosIeLen;
    unsigned char qosSpare1 : 2,
                  delayClasss : 3,
                  reliabilityClass : 3;
    unsigned char peakThroughput : 4,
                  qosSpare2 : 1,
                  precedenceClass : 3;
    unsigned char qosSpare3 : 3,
                  meanThroughput : 5;
    unsigned char trafficClass : 3,
                  deliverOrder : 2,
                  deliveryErrorSdu : 3;
    unsigned char maxSduSize;
    unsigned char ulMaxBitRate;
    unsigned char dlMaxBitRate;
    unsigned char residualBer : 4,
                  sduErrorRatio : 4;
    unsigned char transferDelay : 6,
                  trafficHandlingPrio : 2;
    unsigned char ulGuaranteedBitRate;
    unsigned char dlGuaranteedBitRate;
    unsigned char qosSpare4 : 3,
                  signalingInd : 1,
                  srcStatDesc : 4;
    // the extended maxDlBitrate and dlGuaranteedBitRate
    // will be added if necessary
    unsigned char pdpAddrLen;
    unsigned char addrSpare1 : 4,
                  pdpOrgType : 4;  // only support 00000001 IETF
    unsigned char pdpTypeNumber;   // 00100001 IPv4, 01010111 IPv6
    // IPv4 addreess 6 bytes will be added
    // IPv6 address 16 bytes will be added
    // the maxDlBitRate will be added after address
}UmtsLayer3ActivatePDPContextRequest;

/// Structure for activate PDP COntext Accept
typedef struct
{
    unsigned char spare1 : 4,
                  sapi : 4;
    //negotiated QoS info LV (default 13)
    unsigned char qosIeLen;
    unsigned char qosSpare1 : 2,
                  delayClasss : 3,
                  reliabilityClass : 3;
    unsigned char peakThroughput : 4,
                  qosSpare2 : 1,
                  precedenceClass : 3;
    unsigned char qosSpare3 : 3,
                  meanThroughput : 5;
    unsigned char trafficClass : 3,
                  deliverOrder : 2,
                  deliveryErrorSdu : 3;
    unsigned char maxSduSize;
    unsigned char ulMaxBitRate;
    unsigned char dlMaxBitRate;
    unsigned char residualBer : 4,
                  sduErrorRatio : 4;
    unsigned char transferDelay : 6,
                  trafficHandlingPrio : 2;
    unsigned char ulGuaranteedBitRate;
    unsigned char dlGuaranteedBitRate;
    unsigned char qosSpare4 : 3,
                  signalingInd : 1,
                  srcStatDesc : 4;
    // the extended maxDlBitrate and dlGuaranteedBitRate
    // will be added if necessary
    unsigned char radioPrio : 4,
                  spare2 : 4;
}UmtsLayer3ActivatePDPContextAccept;

/// Structure for activate PDP COntext Reject
typedef struct
{
    unsigned char smCause;
}UmtsLayer3ActivatePDPContextReject;

/// Structure for request PDP COntext activation
typedef struct
{
    unsigned char pdpAddrLen;
    unsigned char addrSpare1 : 4,
                  pdpOrgType : 4;  // only support 00000001 IETF
    unsigned char pdpTypeNumber;   // 00100001 IPv4, 01010111 IPv6
    // IPv4 addreess 6 bytes will be added
    // IPv6 address 16 bytes will be added
    unsigned char trafficClass; // canned for implementation
}UmtsLayer3RequestPDPContextActivation;

/// Structure for request PDP COntext activation reject
typedef UmtsLayer3ActivatePDPContextReject
        UmtsLayer3RequestPDPContextActivationReject;

/// Structure for deactivate PDP COntext request
typedef UmtsLayer3ActivatePDPContextReject
        UmtsLayer3DeactivatePDPContextRequest;

/// NO specific structure for
/// UmtsLayer3DeactivatePDPContextAccept
typedef struct
{
    unsigned char transId;
} UmtsLayer3DeactivatePDPContextAccept;

/////
// RRC related
/////

// DESCRIPTIOn :: structure for MEASUREMENT CONTROL msg
typedef struct
{
    unsigned char rrcTransId;
    unsigned char measId;
    unsigned char measCmd;
}UmtsLayer3MeasurementControl;

// DESCRIPTIOn :: structure for MEASUREMENT REPORT msg
typedef struct
{
    unsigned char measId;
    // measurement results
    // currently only intra-freq measurement results are inculded
    // and only CPICH RSCP is reported
    // the format of the mesurement results,
    // 1. UInt32 cellId
    // 2. singed char rscpVal; 0-91
    unsigned char numMeas; // cannned for implemetation
}UmtsLayer3MeasurementReport;

/////////
// GTP
/////////

// DESCRIPTIOn :: structure for CREATE PDPD CONtexT REQUEST msg
typedef struct
{
    UInt32 dlTeId;
    unsigned char spare1 : 4,
                  nsapi : 4;
    //QoS info LV (default 13)
    unsigned char qosIeLen;
    unsigned char qosSpare1 : 2,
                  delayClasss : 3,
                  reliabilityClass : 3;
    unsigned char peakThroughput : 4,
                  qosSpare2 : 1,
                  precedenceClass : 3;
    unsigned char qosSpare3 : 3,
                  meanThroughput : 5;
    unsigned char trafficClass : 3,
                  deliverOrder : 2,
                  deliveryErrorSdu : 3;
    unsigned char maxSduSize;
    unsigned char ulMaxBitRate;
    unsigned char dlMaxBitRate;
    unsigned char residualBer : 4,
                  sduErrorRatio : 4;
    unsigned char transferDelay : 4,
                  trafficHandlingPrio : 4;
    unsigned char ulGuaranteedBitRate;
    unsigned char dlGuaranteedBitRate;
    unsigned char qosSpare4 : 3,
                  signalingInd : 1,
                  srcStatDesc : 4;
}UmtsGtpCreatePDPContextRequest;

// DESCRIPTIOn :: structure for CREATE PDPD CONtexT RESPONSE msg
typedef struct
{
  unsigned char spare1 : 4,
                nsapi : 4;
  unsigned char gtpCause;
  UInt32 ulTeId;
}UmtsGtpCreatePDPContextResponse;

// DESCRIPTIOn :: structure for CREATE PDPD CONtexT REQUEST msg
typedef struct
{
    unsigned char spare1: 7,
                  teardownInd: 1;
    unsigned char spare2 : 4,
                  nsapi : 4;
}UmtsGtpDeletePDPContextRequest;

// DESCRIPTIOn :: structure for CREATE PDPD CONTEXT REQUEST msg
typedef struct
{
  unsigned char gtpCause;
}UmtsGtpDeletePDPContextResponse;

// DESCRIPTIOn :: structure for PDU Notification REQUEST msg
typedef struct
{
    UInt32 ulTeId;
    UInt32 ueId;

    //QoS info LV (default 13)
    unsigned char qosIeLen;
    unsigned char qosSpare1 : 2,
                  delayClasss : 3,
                  reliabilityClass : 3;
    unsigned char peakThroughput : 4,
                  qosSpare2 : 1,
                  precedenceClass : 3;
    unsigned char qosSpare3 : 3,
                  meanThroughput : 5;
    unsigned char trafficClass : 3,
                  deliverOrder : 2,
                  deliveryErrorSdu : 3;
    unsigned char maxSduSize;
    unsigned char ulMaxBitRate;
    unsigned char dlMaxBitRate;
    unsigned char residualBer : 4,
                  sduErrorRatio : 4;
    unsigned char transferDelay : 4,
                  trafficHandlingPrio : 4;
    unsigned char ulGuaranteedBitRate;
    unsigned char dlGuaranteedBitRate;
    unsigned char qosSpare4 : 3,
                  signalingInd : 1,
                  srcStatDesc : 4;

    unsigned char pdpAddrLen;
    unsigned char addrSpare1 : 4,
                  pdpOrgType : 4;  // only support 00000001 IETF
    unsigned char pdpTypeNumber;   // 00100001 IPv4, 01010111 IPv6
    // IPv4 addreess 6 bytes will be added
    // IPv6 address 16 bytes will be added
} UmtsGtpPduNotificationRequest;


// DESCRIPTIOn :: structure for PDU Notification Response msg
typedef struct
{
  unsigned char gtpCause;
} UmtsGtpPduNotificationResponse;

//*****************************************************************
// main data structure
//*****************************************************************

/// Structure of the layer 3 data for a cellualr UMTS node
/// This is the main data structure of umts layer3
typedef struct umts_layer3_data_str
{
    RandomSeed seed;

    UInt32 interfaceIndex;      // UMTS interface Index

    CellularNodeType nodeType;  // type of the node such as UE
    BOOL isVLR;                 // whether VLR is located on this device

    // pointers to node type specific data
    void *dataPtr;  // UE/NodeB/GSN specific data according to nodeType
    void *vlrData;  // data structure of VLR
    void *hlrData;  // data structure of HLR. We use a separate pointer
                    // for HLR is for co-locating of HLR with other node

    // whether collect and print statistics
    BOOL collectStatistics;
    BOOL printDetailedStat; // Whether to print some detailed statistics

    STAT_NetStatistics* newStats;
} UmtsLayer3Data;

//--------------------------------------------------------------------------
//  Utility API functions
//--------------------------------------------------------------------------

/// Check the domain of the application
class UmtsCheckFlowDomain
{
private:
    UmtsLayer3CnDomainId domain;
public:
    UmtsCheckFlowDomain(UmtsLayer3CnDomainId domainId)
                      : domain(domainId)
    {
    }

    template <typename T>
    bool operator() (const T* appInfo) const
    {
        if (appInfo->classifierInfo)
        {
            return appInfo->classifierInfo->domainInfo == domain;
        }
        else
        {
            return FALSE;
        }
    }
};

/// Configurate a specific Radio Link
///
/// \param node  Pointer to node.
/// \param umtsL3  Point to the umts layer3 data
/// \param rlInfo  pointer to the RL info
///
void UmtsLayer3ConfigRadioLink(
    Node *node,
    UmtsLayer3Data *umtsL3,
    UmtsCphyRadioLinkSetupReqInfo *rlInfo);

/// Configurate a specific Radio Link
///
/// \param node  Pointer to node.
/// \param umtsL3  Point to the umts layer3 data
/// \param rlInfo  pointer to the RL info
///
void UmtsLayer3ReleaseRadioLink(
    Node *node,
    UmtsLayer3Data *umtsL3,
    UmtsCphyRadioLinkRelReqInfo *rlInfo);

/// Configurate the common Transport Channels
///
/// \param node  Pointer to node.
/// \param umtsL3  Point to the umts layer3 data
/// \param trRelInfo  pointer to the RL info
///
void UmtsLayer3ReleaseTrCh(
    Node *node,
    UmtsLayer3Data *umtsL3,
    UmtsCphyTrChReleaseReqInfo *trRelInfo);

/// Configurate the common Transport Channels
///
/// \param node  Pointer to node.
/// \param umtsL3  Point to the umts layer3 data
/// \param trCfgInfo  Pointer to the TrCh config
///
void UmtsLayer3ConfigTrCh(
    Node *node,
    UmtsLayer3Data *umtsL3,
    UmtsCphyTrChConfigReqInfo *trCfgInfo);

/// Release the common radio bearer
///
/// \param node  Pointer to node.
/// \param umtsL3  Point to the umts layer3 data
/// \param rbInfo  released rb info
///
void UmtsLayer3ReleaseRadioBearer(
         Node* node,
         UmtsLayer3Data *umtsL3,
         UmtsCmacConfigReqRbInfo *rbInfo);

/// Configurate the common radio bearer
///
/// \param node  Pointer to node.
/// \param umtsL3  Point to the umts layer3 data
/// \param rbInfo  Pointer to the RB info
/// \param phType  physical channel type
/// \param phId  phId
/// \param numPhCh  number of physical channels
/// \param transFormat  Transport format to be set
///
void UmtsLayer3ConfigRadioBearer(
         Node* node,
         UmtsLayer3Data *umtsL3,
         const UmtsCmacConfigReqRbInfo *rbInfo,
         UmtsPhysicalChannelType* phType,
         int* phId,
         int numPhCh,
         UmtsTransportFormat* transFormat = NULL);

/// Rlease a RLC entity
///
/// \param node  Pointer to node.
/// \param umtsL3  Point to the umts layer3 data
/// \param rbId  RB ID
/// \param direction  Entity direction
/// \param ueId  UE ID, used at NodeB
///
void UmtsLayer3ReleaseRlcEntity(
    Node *node,
    UmtsLayer3Data *umtsL3,
    unsigned char   rbId,
    UmtsRlcEntityDirect direction = UMTS_RLC_ENTITY_BI,
    NodeId ueId = 0);

/// Create a RLC entity
///
/// \param node  Pointer to node.
/// \param umtsL3  Point to the umts layer3 data
/// \param rbId  RB ID
/// \param direction  Entity direction
/// \param entityType  entity type
/// \param entityConfig  entity specific configuration parameters
/// \param ueId  UE ID, used at NodeB
///
void UmtsLayer3ConfigRlcEntity(
    Node *node,
    UmtsLayer3Data *umtsL3,
    unsigned char   rbId,
    UmtsRlcEntityDirect direction,
    UmtsRlcEntityType entityType,
    void* entityConfig,
    NodeId ueId = 0);

/// Set a timer message. If a non-NULL message pointer is
/// passed in, this function will just send out that msg.
/// If NULL message is passed in, it will create a new
/// message and send it out. In either case, a pointer to
/// the message is returned, in case the caller wants to
/// cancel the message in the future.
///
/// \param node  Pointer to node
/// \param umtsL3  Pointer to UMTS Layer3 data
/// \param timerType  Type of the timer
/// \param delay  Delay of this timer
/// \param msg  If non-NULL, use this message
/// \param infoPtr  Additional info fields if needed
/// \param infoSize  Size of the additional info fields
///
/// \return Pointer to the timer message
Message* UmtsLayer3SetTimer(Node *node,
                            UmtsLayer3Data *umtsL3,
                            UmtsLayer3TimerType timerType,
                            clocktype delay,
                            Message* msg,
                            void* infoPtr = NULL,
                            int infoSize = 0);

/// A utility function to add the standard L3 message header
///
/// \param node  Pointer to node.
/// \param msg  Message for adding the header
/// \param pd  Value of the PD field of the header
/// \param tiSpd  Value of the TI or SPD field of the header
/// \param msgType  Value of the Message Type field of the header
///
void UmtsLayer3AddHeader(Node *node,
                         Message *msg,
                         char pd,
                         char tiSpd,
                         char msgType);

/// A utility function to remove the standard L3 message header
///
/// \param node  Pointer to node.
/// \param msg  Message for adding the header
/// \param pd  For returning PD of the header
/// \param tiSpd  For returning TI or SPD field of the header
/// \param msgType  For returning Message Type field of the header
///
void UmtsLayer3RemoveHeader(Node *node,
                            Message *msg,
                            char *pd,
                            char *tiSpd,
                            char *msgType);

/// A utility function to add the Backbone message header
///
/// \param node  Pointer to node.
/// \param msg  Message for adding the header
/// \param msgType  The backbone message type
/// \param info  The additional header info
/// \param infoSize  The additional info size
///
void UmtsAddBackboneHeader(Node *node,
                           Message *msg,
                           UmtsBackboneMessageType msgType,
                           const char* info,
                           int infoSize);

/// A utility function to add the backbone message header
/// at the Iub Interface
///
/// \param node  Pointer to node.
/// \param msg  Message for adding the header
/// \param ueId  Value of the ueId
/// \param rbIdOrMsgType:  Value of rbId or message type
///
void UmtsAddIubBackboneHeader(Node *node,
                              Message *msg,
                              NodeId ueId,
                              UInt8 rbIdOrMsgType);

/// A utility function to remove the backbone header
///
/// \param node  Pointer to node.
/// \param msg  Message for adding the header
///    + msgType   : UmtsBackboneMessageType: For returning of 
///    the backbone message type
/// \param info  For returnning of the additional header info
/// \param infoSize  The additional info size
///
void UmtsRemoveBackboneHeader(Node *node,
                              Message *msg,
                              UmtsBackboneMessageType* msgType,
                              char* info,
                              int& infoSize);

/// Add a CS data IU interface header
///
/// \param node  Pointer to node.
/// \param msg  Message containing the NBAP message
///    + ueId      : NodeId            : Node Id 
/// \param rabId  RAB ID to  be added
void UmtsLayer3AddIuCsDataHeader(
        Node *node,
        Message *msg,
        NodeId ueId,
        char rabId);

/// A utility function to add the backbone message header
/// at the Iu Interface
///
/// \param node  Pointer to node.
/// \param msg  Message for adding the header
/// \param ueId  Value of the ueId
/// \param msgType  RANAP message type
///
void UmtsAddIuBackboneHeader(Node *node,
                             Message *msg,
                             NodeId ueId,
                             UmtsRanapMessageType msgType);

/// A utility function to add the NBAP message header
///
/// \param node  Pointer to node.
/// \param msg  Message for adding the header
/// \param transctId  Transaction ID
/// \param ueId  UE ID
///
void UmtsAddNbapHeader(Node *node,
                       Message *msg,
                       unsigned char transctId,
                       NodeId ueId);

/// A utility function to add the NBAP message header
///
/// \param node  Pointer to node.
/// \param msg  Message for adding the header
/// \param transctId  For returning transaction ID
/// \param ueId  For returnning UE ID
///
void UmtsRemoveNbapHeader(Node *node,
                          Message *msg,
                          unsigned char* transctId,
                          NodeId* ueId);

/// A utility function to add theGTP message header
///
/// \param node  Pointer to node.
/// \param msg  Message for adding the header
/// \param firstByte  the first byte of the GTP header
/// \param msgType  The GTP message type
/// \param teId  TeId of the GTP
/// \param info  The additional header info
/// \param infoSize  The additional info size
///
void UmtsAddGtpHeader(Node *node,
                      Message *msg,
                      unsigned char firstByte,
                      UmtsGtpMsgType msgType,
                      UInt32 teId,
                      const char* info,
                      int infoSize);

/// A utility function to remove the GTP header
///
/// \param node  Pointer to node.
/// \param msg  Message for adding the header
/// \param gtpHeader  Mandatory header
/// \param info  For returnning of the additional header info
/// \param infoSize  The additional info size
///
void UmtsRemoveGtpHeader(Node *node,
                         Message *msg,
                         UmtsGtpHeader* gtpHeader,
                         char* info,
                         int* infoSize);

/// Send a RANAP message to one RNC
///
/// \param node  Pointer to node.
/// \param msg  Message containing the NBAP message
/// \param destAddt  destination address
///    + upLink    : BOOL         : Is it uplink 
/// \param ueId  UE Id of the GTP msg
void UmtsLayer3GsnSendGtpMsgOverBackbone(
                                   Node *node,
                                   Message *msg,
                                   Address destAddr,
                                   BOOL upLink = TRUE,
                                   NodeId ueId = 0xFFFFFFFF);

/// Send a CS signalling message over backbone
///
/// \param node  Pointer to node.
/// \param msg  Message containing the NBAP message
/// \param destAddt  destination address
///    + ti        : char         : transction Id  
/// \param ueId  UE Id of the GTP msg
/// \param msgType  CS signalling msg type
void UmtsLayer3GsnSendCsSigMsgOverBackbone(
        Node *node,
        Message *msg,
        Address destAddr,
        char ti,
        NodeId ueId,
        UmtsCsSigMsgType msgType);

/// Send a CS data packet over backbone
///
/// \param node  Pointer to node.
/// \param msg  Message containing the NBAP message
/// \param destAddt  destination address
///    + ti        : char         : transction Id  
/// \param ueId  UE Id of the GTP msg
void UmtsLayer3GsnSendCsPktOverBackbone(
        Node *node,
        Message *msg,
        Address destAddr,
        char ti,
        NodeId ueId);

/// A utility function for sending packets on the links
/// connecting NodeB, RNC, SGSN, GGSN, HLR etc.
///
/// \param node  Pointer to node.
/// \param msg  Message to be transmitted
/// \param destAddr  Destination address
/// \param interfaceIndex  Outgoing interface index
/// \param priority  Priority of the message
/// \param ttl  TTL value
///
void UmtsLayer3SendMsgOverBackbone(Node *node,
                                   Message *msg,
                                   Address destAddr,
                                   int interfaceIndex,
                                   TosType priority,
                                   unsigned ttl);

/// The fixed nodes such as NodeB, RNC, SGSN will broadcast
/// hello packets to discover the connectivity. This is used
/// to simplify user configurations.
///
/// \param node  Pointer to node.
/// \param umtsL3  Pointer to UMTS Layer3 data
/// \param int  Interface to send the hello. If value is
///    -1, will be send on all possible infs
/// \param additionalInfoSize  size of additional info
/// \param additionalInfo  additional info
///
void UmtsLayer3SendHelloPacket(Node *node,
                               UmtsLayer3Data *umtsL3,
                               int interfaceIndex,
                               int additionInfoSize = 0,
                               const char* additionInfo = NULL);

/// Fill in classifier info structure.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interfaceIndex
/// \param networkType  Type of network
/// \param msg  Packet from upper layer
/// \param tos  Priority of packet
///    + classifierInfo   : UmtsLayer3FlowClassifier* : 
///    classifier info of this flow
/// \param payload  Packet payload
///
void UmtsLayer3BuildFlowClassifierInfo(
        Node* node,
        int interfaceIndex,
        int networkType,
        Message* msg,
        TosType* tos,
        UmtsLayer3FlowClassifier* classifierInfo,
        const char** payload);

/// Check basic QoS parameters.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interfaceIndex
/// \param appType  trace protocol type
/// \param tos  Tos type
/// \param payload  Packet payload
///
BOOL UmtsLayer3CheckFlowQoSParam(
        Node* node,
        int interfaceIndex,
        TraceProtocolType appType,
        TosType tos,
        const char** payload);

/// Fill in classifier info structure.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interfaceIndex
/// \param appType  trace protocol type
/// \param tos  Tos type
/// \param payload  Packet payload
/// \param qosInfo  QoS info
///
BOOL UmtsLayer3GetFlowQoSInfo(
        Node* node,
        int interfaceIndex,
        TraceProtocolType appType,
        TosType tos,
        const char** payload,
        UmtsFlowQoSInfo** qosInfo);

/// convert traffic class into octet
///
/// \param trafficClass  class of this flow
/// \param tfCls  output Octet
/// \param handlingPrio  handling priority
///
void UmtsLayer3ConvertTrafficClassToOctet(
                  UmtsQoSTrafficClass trafficClass,
                  unsigned char* tfCls,
                  unsigned char* handlingPrio);

/// convert octet into traffic class
///
/// \param tfCls  output Octet
/// \param handlingPrio  handling priority
/// \param trafficClass  class of this flow
///
void UmtsLayer3ConvertOctetToTrafficClass(
                  unsigned char tfCls,
                  unsigned char handlingPrio,
                  UmtsQoSTrafficClass* trafficClass);

/// convert max rate into octet
///
/// \param maxRate  max rate
/// \param rate  Octet
///
void UmtsLayer3ConvertMaxRateToOctet(unsigned int maxRate,
                                     unsigned char* rate);

/// convert octet into max rate
///
/// \param rate  Octet
/// \param maxRate  max rate
///
void UmtsLayer3ConvertOctetToMaxRate(unsigned char rate,
                                     unsigned int* maxRate);

/// convert delay into octet
///
/// \param appLatency  delay
/// \param dealy  octet
///
void UmtsLayer3ConvertDelayToOctet(clocktype appLaterncy,
                                     unsigned char* delay);

/// convert octet into delay
///
/// \param dealy  octet
/// \param appLatency  delay
///
void UmtsLayer3ConvertOctetToDelay(unsigned char delay,
                                   clocktype* appLaterncy);

/// Print run time info UMTS layer2 at a given interface.
///
/// \param node  Pointer to node.
/// \param sublayerType  Pointer to node input.
///
void UmtsLayer3PrintRunTimeInfo(Node *node,
                                UmtsLayer3SublayerType sublayerType);

//--------------------------------------------------------------------------
//  API functions
//--------------------------------------------------------------------------

/// Get the pointer which points to the UMTS layer 3 data.
///
/// \param node  Pointer to node.
///
/// \return Pointer to UMTS layer3 data or NULL
UmtsLayer3Data* UmtsLayer3GetData(Node *node);

/// A utility function to add the backbone message header
/// at the Iu
///
/// \param node  Pointer to node.
/// \param msg  Message for adding the header
/// \param ueId  Value of the ueId
/// \param BOOL  Whether this message is a uplink message
///
void UmtsAddGtpBackboneHeader(Node *node,
                              Message *msg,
                              BOOL upLink,
                              NodeId ueId);

/// Called by RLC to report unrecoverable
/// error for AM RLC entity.
///
/// \param node  Pointer to node.
/// \param ueId  UE identifier
/// \param rbId  Radio bearer ID
///
void UmtsLayer3ReportAmRlcError(Node *node,
                                NodeId ueId,
                                char rbId);

/// Handle timers and layer messages.
///
/// \param node  Pointer to node.
/// \param msg  Message for node to interpret.
/// \param lastHopAddress  Address of the last hop
/// \param interfaceIndex  Interface from which
///    the packet is received
///
void UmtsLayer3ReceivePacketFromMacLayer(Node *node,
                                         Message *msg,
                                         NodeAddress lastHopAddress,
                                         int interfaceIndex);

/// Handle received packet from IP
///
/// \param node  Pointer to node.
/// \param msg  Message for node to interpret.
/// \param interfaceIndex  Interface from which packet was received
/// \param srcAddr  Address of the source of the packet
///
void UmtsLayer3ReceivePacketOverIp(Node* node,
                                   Message *msg,
                                   int interfaceIndex,
                                   Address srcAddr);

/// Handle pakcets from upper or outside PDN
///
/// \param node  Pointer to node.
///    + msg              : Message*          : Message to be sent onver 
///    the air interface
///    + interfaceIndex   : int               : Interface from which 
///    the packet is received
/// \param networkType  network Type, IPv4 or IPv6
///
void UmtsLayer3HandlePacketFromUpperOrOutside(
            Node* node,
            Message* msg,
            int interfaceIndex,
            int networkType);

// FUNCITON   :: UmtsLayer3HandleInterLayerCommand
/// Handle Interlayer command
///
/// \param node  Pointer to node.
/// \param cmdType  command type
/// \param interfaceIdnex  interface index of UMTS
/// \param cmd  cmd to handle
///
void UmtsLayer3HandleInterLayerCommand(
            Node* node,
            UInt32 interfaceIndex,
            UmtsInterlayerCommandType cmdType,
            void* cmd);

/// Pre-Initialize UMTS layer 3 data. It may trigger node type
/// specific initialization too.
///
/// \param node  Pointer to node.
/// \param nodeInput  Pointer to node input.
///
void UmtsLayer3PreInit(Node* node, const NodeInput* nodeInput);

/// Initialize UMTS layer 3 data. It may trigger node type
/// specific initialization too.
///
/// \param node  Pointer to node.
/// \param nodeInput  Pointer to node input.
///
void UmtsLayer3Init(Node* node, const NodeInput* nodeInput);

/// Handle timers and layer messages.
///
/// \param node  Pointer to node.
/// \param msg  Message for node to interpret.
///
void UmtsLayer3Layer(Node* node, Message* msg);

/// Print stats and clear protocol variables.
///
/// \param node  Pointer to node.
///
void UmtsLayer3Finalize(Node *node);
#endif /* _LAYER3_UMTS_H_ */
