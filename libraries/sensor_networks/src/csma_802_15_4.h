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

#ifndef CSMA_802_15_4_H
#define CSMA_802_15_4_H

#include "api.h"
#include "mac_802_15_4.h"
#include "phy_802_15_4.h"
// --------------------------------------------------------------------------
// #define's
// --------------------------------------------------------------------------


// --------------------------------------------------------------------------
// typedef's enums
// --------------------------------------------------------------------------

/// Timers used by CSMA-CA
typedef enum
{
    C802_15_4BACKOFFTIMER,
    C802_15_4BEACONOTHERTIMER,
    C802_15_4DEFERCCATIMER
}C802_15_4TimerType;

// --------------------------------------------------------------------------
// typedef's struct
// --------------------------------------------------------------------------

/// Timer type for CSMACA 802.15.4
typedef struct csma_802_15_4_timer_str
{
    C802_15_4TimerType timerType;
}C802_15_4Timer;

/// Layer structure of CSMA
typedef struct csma_802_15_4_str
{
    UInt8 NB;	//the Number of Backoffs 	バックオフの数
    UInt8 CW;	//the Contention Window		
    UInt8 BE;	//the Backoff Exponent   

    BOOL ackReq;
    BOOL beaconEnabled;
    BOOL beaconOther;
    BOOL waitNextBeacon;
    clocktype bcnTxTime;
    clocktype bcnRxTime;
    clocktype bPeriod;             // backoff periods
    Int32 bPeriodsLeft;           // backoff periods left for next superframe
    Message* txPkt;

    // Timers
    Message* backoffT;
    Message* bcnOtherT;
    Message* deferCCAT;

    RandomSeed seed;
}CsmaData802_15_4;

// --------------------------------------------------------------------------
// FUNCTION DECLARATIONS
// --------------------------------------------------------------------------

/// Resets running CSMA algo.
///
/// \param node  Pointer to node
///
void Csma802_15_4Reset(Node* node, Int32 interfaceIndex);

/// find the beginning point of CAP and adjust the scheduled
/// time, if it comes before CAP
///
/// \param node  Pointer to node
/// \param  wtime  Time to be adjusted
///
/// \return clocktype
clocktype Csma802_15_4AdjustTime(Node* node, Int32 interfaceIndex, clocktype wtime);

/// Check if can proceed within the current superframe
///
/// \param node  Pointer to node
/// \param  wtime  Time to be adjusted
///    + afterCCA   : BOOL              :
///
/// \return TRUE or FALSE
BOOL Csma802_15_4CanProceed(Node* node,
                            Int32 interfaceIndex,
                            clocktype wtime,
                            BOOL afterCCA = FALSE);

/// Called by MAC each time a new beacon received or sent within
/// the current PAN
///
/// \param node  Pointer to node
///    + trx        : char              :
///
void Csma802_15_4NewBeacon(Node* node, Int32 interfaceIndex, char trx);

/// Called by MAC to start CSMA algo to send pkt
///
/// \param node  Pointer to node
/// \param firsttime  Whether sending for the first time
/// \param pkt  Packet to be sent
/// \param ackreq  Whether Ack requested
///
void Csma802_15_4Start(Node* node,
                       Int32 interfaceIndex,
                       BOOL firsttime,
                       Message* pkt = NULL,
                       BOOL ackreq = FALSE);

/// To cancel CSMA algo
///
/// \param node  Pointer to node
///
void Csma802_15_4Cancel(Node* node, Int32 interfaceIndex);

/// Retruns receive ON status
///
/// \param node  Pointer to node
/// \param status  RxON status
///
void Csma802_15_4RX_ON_confirm(
        Node* node,
        Int32 interfaceIndex,
        PLMEsetTrxState status);

/// Fn to handle Beacon Other timer
///
/// \param node  Pointer to node
///
void Csma802_15_4BcnOtherHandler(Node* node, Int32 interfaceIndex);

/// Fn to handle Defer CCA timer
///
/// \param node  Pointer to node
///
void Csma802_15_4DeferCCAHandler(Node* node, Int32 interfaceIndex);

/// Returns channel clear access status
///
/// \param node  Pointer to node
/// \param status  CCA status
///
void Csma802_15_4CCA_confirm(Node* node,
                             Int32 interfaceIndex,
                             PhyStatusType status);

// FUNCTION     Csma802_15_4Init
// PURPOSE      Initialization function for CSMA-CA
/// PARAMETERS   Node* node
/// Node being initialized.
/// NodeInput* nodeInput
/// Structure containing contents of input file.
/// Int32 interfaceIndex
/// Interface index.
/// RETURN       None
/// NOTES        None
void Csma802_15_4Init(Node* node,
                      const NodeInput* nodeInput,
                      Int32 interfaceIndex);

// FUNCTION     Csma802_15_4Layer
// PURPOSE      To handle timer events at CSMA level
/// PARAMETERS   Node *node
/// Node which received the message.
/// Int32 interfaceIndex
/// Interface index.
/// Message* msg
/// Message received by the layer.
/// RETURN       None
/// NOTES        None
void Csma802_15_4Layer(Node* node, Int32 interfaceIndex, Message* msg);


// FUNCTION     Csma802_15_4Finalize
// PURPOSE      Called at the end of simulation to clean CSMA
/// PARAMETERS   Node* node
/// Node which received the message.
/// Int32 interfaceIndex
/// Interface index.
/// RETURN       None
/// NOTES        None
void Csma802_15_4Finalize(Node* node, Int32 interfaceIndex);

// --------------------------------------------------------------------------
// STATIC FUNCTIONS
// --------------------------------------------------------------------------

#endif  /*CSMA_802_15_4*/
