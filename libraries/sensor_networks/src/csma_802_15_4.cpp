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

#include "csma_802_15_4.h"
#include "mydef.h"
#define DEBUG 0
#define KANDEB 0
//#define MODE15_4E 1

// Set a timer message.
//
// \param node  Pointer to node
//    + interfaceIndex ; Int32               : Interface index of device
// \param timerType  Type of the timer
// \param delay  Delay of this timer
//
// \return Message*
static
Message* Csma802_15_4SetTimer(Node* node,
                              Int32 interfaceIndex,
                              C802_15_4TimerType timerType,
                              clocktype delay)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #SetTimer#\n",node->getNodeTime(),node->nodeId);
}
    Message* timerMsg = NULL;
    C802_15_4Timer* timerInfo = NULL;

    // allocate the timer message and send out
    timerMsg = MESSAGE_Alloc(node,
                             MAC_LAYER,
                             MAC_PROTOCOL_802_15_4,
                             MSG_CSMA_802_15_4_TimerExpired);

    MESSAGE_SetInstanceId(timerMsg, (short)interfaceIndex);

    MESSAGE_InfoAlloc(node, timerMsg, sizeof(C802_15_4Timer));
    timerInfo = (C802_15_4Timer*) MESSAGE_ReturnInfo(timerMsg);

    timerInfo->timerType = timerType;

    MESSAGE_Send(node, timerMsg, delay);
    return timerMsg;
}



// Resets running CSMA algo.
//
// \param node  Pointer to node
//
void Csma802_15_4Reset(Node* node, Int32 interfaceIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #Reset#\n",node->getNodeTime(),node->nodeId);
}

    CsmaData802_15_4* csmaca = NULL;
    MacData802_15_4* mac = NULL;

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;
    csmaca = (CsmaData802_15_4*)mac->csma;

    if (csmaca->beaconEnabled)
    {
        csmaca->NB = 0;
        csmaca->CW = 2;
        csmaca->BE = mac->mpib.macMinBE;
        if ((mac->mpib.macBattLifeExt) && (csmaca->BE > 2))
        {
            csmaca->BE = 2;
        }
    }
    else
    {
        csmaca->NB = 0;
        csmaca->BE = mac->mpib.macMinBE;
    }
}

// find the beginning point of CAP and adjust the scheduled
// time, if it comes before CAP
//
// \param node  Pointer to node
// \param wtime  Time to be adjusted
//
// \return clocktype
clocktype Csma802_15_4AdjustTime(Node* node,
                                 Int32 interfaceIndex,
                                 clocktype wtime)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #AjustTime#\n",node->getNodeTime(),node->nodeId);
}

    CsmaData802_15_4* csmaca = NULL;
    MacData802_15_4* mac = NULL;
    clocktype neg = 0;
    clocktype tmpf = 0;

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;
    csmaca = (CsmaData802_15_4*)mac->csma;

    ERROR_Assert(csmaca->txPkt, "csmaca->txPkt should not be NULL");
    if (!Mac802_15_4ToParent(node, interfaceIndex, csmaca->txPkt))
    {
        if (mac->mpib.macBeaconOrder != 15)
        {
            tmpf = mac->beaconPeriods * csmaca->bPeriod ;
            tmpf = node->getNodeTime() - tmpf;
            tmpf += wtime;
            neg = tmpf - csmaca->bcnTxTime;

            if (neg < 0.0)
            {
                wtime -= neg;
            }
            return wtime;
        }
        else
        {
            return wtime;
        }
    }
    else
    {
        if (mac->macBeaconOrder2 != 15)
        {
            tmpf = mac->beaconPeriods2 * csmaca->bPeriod;
            tmpf = node->getNodeTime() - tmpf;
            tmpf += wtime;
            neg = tmpf - csmaca->bcnRxTime;

            if (neg < 0.0)
            {
                wtime -= neg;
            }
            return wtime;
        }
        else
        {
            return wtime;
        }
    }
}

// To handle back off timer
//
// \param node  Pointer to node
//
static
void Csma802_15_4BackoffHandler(Node* node, Int32 interfaceIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #BackoffHandler#\n",node->getNodeTime(),node->nodeId);
}

    CsmaData802_15_4* csmaca = NULL;
    MacData802_15_4* mac = NULL;

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;
    csmaca = (CsmaData802_15_4*)mac->csma;
    mac->taskP.RX_ON_csmaca_STEP = TRUE;
    mac->trx_state_req = RX_ON;
    Phy802_15_4PlmeSetTRX_StateRequest(node,
                                       interfaceIndex,
                                       RX_ON);

    // calling state_confirm - assuming trx was success
    Mac802_15_4PLME_SET_TRX_STATE_confirm(node,
                                          interfaceIndex,
                                          PHY_SUCCESS);
}

// Check if can proceed within the current superframe
//
// \param node  Pointer to node
// \param  wtime  Time to be adjusted
//    + afterCCA   : BOOL              :
//
// \return TRUE or FALSE
BOOL Csma802_15_4CanProceed(Node* node,
                            Int32 interfaceIndex,
                            clocktype wtime,
                            BOOL afterCCA)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #CanProceed#\n",node->getNodeTime(),node->nodeId);
}

    // check if can proceed within the current superframe
    // (in the case the node acts as both a coordinator and a device, both
    // the superframes from and to this node should be taken into account)
    CsmaData802_15_4* csmaca = NULL;
    MacData802_15_4* mac = NULL;
    BOOL ok = FALSE;
    UInt16 t_bPeriods = 0;
    UInt32 t_CAP = 0;
    clocktype t_fCAP = 0;
    clocktype t_CCATime = 0;
    clocktype t_IFS = 0;
    clocktype t_transacTime = 0;
    clocktype BI2 = 0;
    clocktype tmpf = 0;

	//takemura
	clocktype t_fsf1 = 0;
	clocktype t_fsf2 = 0;
	clocktype bcnTxTime = 0;
	clocktype bcnRxTime = 0;
	clocktype myBI1 = 0,myBI2 = 0;
	//

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;

	//takemura
	bcnTxTime = mac->macBcnTxTime;
	bcnRxTime = mac->macBcnRxTime;
	myBI1 = mac->sfSpec.BI *SECOND / Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]);
	myBI2 = mac->sfSpec2.BI *SECOND / Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]);
	//
    csmaca = (CsmaData802_15_4*)mac->csma;
    csmaca->waitNextBeacon = FALSE;
    wtime = Mac802_15_4LocateBoundary(
                node,
                interfaceIndex,
                Mac802_15_4ToParent(node, interfaceIndex, csmaca->txPkt),
                wtime);
    if (!Mac802_15_4ToParent(node, interfaceIndex, csmaca->txPkt))
    {
	//takemura
	//printf("takemuraDEBUG : not to parent\n");
	//

        if (mac->mpib.macBeaconOrder != 15)
        {
            if (mac->sfSpec.BLE)
            {
                t_CAP = (UInt16)Mac802_15_4GetBattLifeExtSlotNum(node,
                            interfaceIndex);
            }
            else
            {
                 t_CAP = (mac->sfSpec.FinCAP + 1) * (mac->sfSpec.sd /
                        aUnitBackoffPeriod) - mac->beaconPeriods;
            }

            tmpf = node->getNodeTime() + wtime;
            tmpf -= csmaca->bcnTxTime;
            tmpf /= csmaca->bPeriod;
            t_bPeriods = (UInt16)abs((Int32)(tmpf - mac->beaconPeriods));

            tmpf = node->getNodeTime() + wtime;
            tmpf -= csmaca->bcnTxTime;
            
            if ((tmpf % csmaca->bPeriod) > 0)
            {
                t_bPeriods++;
            }
            csmaca->bPeriodsLeft = t_bPeriods - t_CAP;
        }
        else
        {
            csmaca->bPeriodsLeft = -1;
        }
    }
    else
    {
	//takemura
	//printf("takemuraDEBUG : to parent\n");
	//
        if (mac->macBeaconOrder2 != 15)
        {
            BI2 = mac->sfSpec2.BI * SECOND
                / Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]);
            clocktype tmpf2 = 0;
            tmpf2 = (mac->sfSpec2.FinCAP + 1) * mac->sfSpec2.sd * SECOND
                  / Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]);
            tmpf2 += mac->macBcnRxTime;

            tmpf = aMaxLostBeacons * BI2;
            if (tmpf2 + tmpf < node->getNodeTime())
            {
	//takemura
	//printf("takemuraDEBUG : if 1\n");
	//

                csmaca->bPeriodsLeft = -1;
            }
            else
            {
                if (mac->sfSpec2.BLE)
                {
	//takemura
	//printf("takemuraDEBUG : if 2\n");
	//
                    t_CAP = (UInt16)Mac802_15_4GetBattLifeExtSlotNum(
                                                          node,
                                                          interfaceIndex);
                }
                else
                {
	//takemura
	//printf("takemuraDEBUG : else 1\n");
	//

                    t_CAP = (mac->sfSpec2.FinCAP + 1)
                            * (mac->sfSpec2.sd / aUnitBackoffPeriod)
                            - mac->beaconPeriods2;
                }

                tmpf = node->getNodeTime() + wtime; //tmpf : end backoff time(sim time)(1)
                tmpf -= csmaca->bcnRxTime; //(2)
                tmpf /= csmaca->bPeriod; //(3)
                t_bPeriods = (UInt16)abs((Int32)(tmpf - mac->beaconPeriods2));

                tmpf = node->getNodeTime() + wtime;
                tmpf -= csmaca->bcnRxTime;
                if (tmpf % csmaca->bPeriod > 0)
                {
	//takemura
	//printf("takemuraDEBUG : if 3\n");
	//
                    t_bPeriods++;
                }
                csmaca->bPeriodsLeft = t_bPeriods - t_CAP;
            }
        }
        else
        {
            csmaca->bPeriodsLeft = -1;
        }
    }

    ok = TRUE;
    if (csmaca->bPeriodsLeft > 0)
    {
	//takemura
	//printf("error-1\n");
	//
        ok = FALSE;
    }
    else if (csmaca->bPeriodsLeft == 0)
    {
        if ((!Mac802_15_4ToParent(node, interfaceIndex, csmaca->txPkt))
              &&  (!mac->sfSpec.BLE))
        {
	//takemura
		//printf("error0\n");
	//

            ok = FALSE;
        }
        else if (Mac802_15_4ToParent(node, interfaceIndex, csmaca->txPkt)
                  &&  (!mac->sfSpec2.BLE))
        {
		//takemura
		//printf("error1\n");
		//

            ok = FALSE;
        }
    }
    if (!ok)
    {
        csmaca->bPeriodsLeft = 0;
        csmaca->waitNextBeacon = TRUE;
        return FALSE;
    }

    // calculate the time needed to finish the transaction
     t_CCATime = 8 * SECOND
                  / Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]);

     if (MESSAGE_ReturnPacketSize(csmaca->txPkt) <= aMaxSIFSFrameSize)
    {
        t_IFS = aMinSIFSPeriod * SECOND;
    }
    else
    {
        t_IFS = aMinLIFSPeriod * SECOND;
    }

    t_IFS /= Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]);

    t_transacTime  = Mac802_15_4LocateBoundary(
                        node,
                        interfaceIndex,
                        Mac802_15_4ToParent(node, interfaceIndex,
                            csmaca->txPkt),
                        wtime) - wtime;            // boundary location time
    // -- should be 0 here, since we have already located the boundary
    if (!afterCCA)
    {
        t_transacTime += t_CCATime;        // first CCA time
        t_transacTime
            += Mac802_15_4LocateBoundary(
                    node,
                    interfaceIndex,
                    Mac802_15_4ToParent(node, interfaceIndex, csmaca->txPkt),
                    t_transacTime) - (t_transacTime); // boundary location time

        // for second CCA
        t_transacTime += t_CCATime;       // second CCA time
    }
    t_transacTime
        += Mac802_15_4LocateBoundary(
                node,
                interfaceIndex,
                Mac802_15_4ToParent(node, interfaceIndex, csmaca->txPkt),
                t_transacTime) - (t_transacTime); // boundary location time

    // packet transmission time
    t_transacTime += Mac802_15_4TrxTime(node, interfaceIndex, csmaca->txPkt);
    M802_15_4Header* wph = (M802_15_4Header*) (csmaca->txPkt)->packet;
    if (csmaca->ackReq)
    {
        t_transacTime += PHY_GetTransmissionDuration(
                                    node,
                                    interfaceIndex,
                                    (int)PHY_GetTxDataRate(node, interfaceIndex),
                                    numPsduOctetsInAckFrame);
        t_transacTime += max_pDelay;
    }
    
    if (wph->MHR_DstAddrInfo.addr_16 != M802_15_4_COORDSHORTADDRESS)
    {
        // One side propagation delay from source to destination for unicast
        // packets

        t_transacTime += max_pDelay;

        // IFS time -- not only ensure that the sender can finish the
        // transaction, but also  the receiver

        t_transacTime += t_IFS;
    }
    else
    {
        // Broadcast packet
        t_transacTime += max_pDelay;
        t_transacTime += 12 * SECOND
              / Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]);
    }
    
    // Currently Superframe overflow protection is not supported.
    // Mac802_15_4GetCAP(...) function returns the valid cap taking
    // into account any collision with timer controlled
    // control packets (beacons).
    BOOL toParent = Mac802_15_4ToParent(node,interfaceIndex,csmaca->txPkt);
    t_fCAP = Mac802_15_4GetCAP(node, interfaceIndex, toParent);
	tmpf = node->getNodeTime() + wtime; // tmpf = end backoff time

	//takemura
	if (DEBUG){
		printf("t_fCAP : %ld\nnow time : %ld\n",t_fCAP,node->getNodeTime());
		printf("beaconTX : %ld\nbeaconRX : %ld\n",mac->macBcnTxTime,mac->macBcnRxTime);
	}
	//takemura change
	//
	//tmpf += t_transacTime;
	//if (tmpf > t_fCAP)
    //{
    //    ok = FALSE;
    //}
    //else
    //{
    //    ok = TRUE;
    //}
	//
#ifndef MODE15_4E
		tmpf += t_transacTime;
		if (tmpf > t_fCAP)
    	{
        	ok = FALSE;
    	}
    	else
    	{
        	ok = TRUE;
    	}
#else
		if (tmpf > t_fCAP) //transmit start time is later than end CAP
    	{
			if(DEBUG){
				printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp transmit start is out of CAP\n",node->getNodeTime(),node->nodeId);
			}
        	ok = FALSE;
    	}
    	else //check whether transaction is be able to end in the SuperFrame
    	{
			if (mac->mpib.macBeaconOrder != 15){
        		tmpf += t_transacTime;
				while (bcnTxTime + myBI1 < node->getNodeTime()){
					bcnTxTime += myBI1;
				}
				bcnTxTime += myBI1; //next beaconRxTime
			}
			if (mac->macBeaconOrder2 != 15){
        		tmpf += t_transacTime;
				while (bcnRxTime + myBI2 < node->getNodeTime()){
					bcnRxTime += myBI2;
				}
				bcnRxTime += myBI2; //next beaconRxTime
			}
			if (bcnRxTime == 0 || bcnRxTime >= tmpf){
				if (bcnTxTime == 0 || bcnTxTime >= tmpf){
					ok = TRUE;
				}
				else{
					if(DEBUG){
						printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp transmit end is out of CAP\n",node->getNodeTime(),node->nodeId);
					}
					ok = FALSE;
				}
			}
			else {
				if(DEBUG){
					printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp transmit end is out of CAP\n",node->getNodeTime(),node->nodeId);
				}
				ok = FALSE;
			}
    	}

#endif //MODE15_4E
	//takemura change end
    
    // Timer for beaconotherhandler has been removed.
    if (!ok)
    {
        csmaca->bPeriodsLeft = 0;
        csmaca->waitNextBeacon = TRUE;
        return FALSE;
    }
    else
    {
        csmaca->bPeriodsLeft = -1;
        return TRUE;
    }
}

// Called by MAC each time a new beacon received or sent within
// the current PAN
//
// \param node  Pointer to node
//    + trx        : char              :
//
void Csma802_15_4NewBeacon(Node* node,
                           Int32 interfaceIndex,
                           char trx)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #NewBeacon#\n",node->getNodeTime(),node->nodeId);
}

    CsmaData802_15_4* csmaca = NULL;
    MacData802_15_4* mac = NULL;
    Int32 rate = 0;
    clocktype wtime = 0;

    if (DEBUG)
    {
        printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4CSMA :"
               "Check to see if any packet is pending to  be "
               "transmitted(Csma802_15_4NewBeacon)\n",
                node->getNodeTime(),
                node->nodeId);
    }

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;
    csmaca = (CsmaData802_15_4*)mac->csma;

    if (!mac->txAck)
    {
        mac->trx_state_req = RX_ON;
        Phy802_15_4PlmeSetTRX_StateRequest(node,
                                           interfaceIndex,
                                           RX_ON);
    }

    if (csmaca->bcnOtherT != NULL)
    {
        MESSAGE_CancelSelfMsg(node, csmaca->bcnOtherT);
        csmaca->bcnOtherT = NULL;
    }

    // update values
    csmaca->beaconEnabled = ((mac->mpib.macBeaconOrder !=15)
                                || (mac->macBeaconOrder2 != 15));
    csmaca->beaconOther = (mac->macBeaconOrder3 != 15);
    Csma802_15_4Reset(node, interfaceIndex);

    rate = Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]);

    csmaca->bcnTxTime = mac->macBcnTxTime;
    csmaca->bcnRxTime = mac->macBcnRxTime;
    csmaca->bPeriod = aUnitBackoffPeriod * SECOND / rate;

    if (csmaca->waitNextBeacon)
    {
        if ((csmaca->txPkt)
             && (csmaca->backoffT == NULL)
             && (csmaca->txPkt->packet))
        {
            ERROR_Assert(csmaca->bPeriodsLeft >= 0, "Beacon period left"
                " should be >=0");
            if (csmaca->bPeriodsLeft == 0)
            {
                wtime = Csma802_15_4AdjustTime(node, interfaceIndex, 0);
                if (Csma802_15_4CanProceed(node, interfaceIndex, wtime))
                {
                    // no need to resume backoff
                    if (DEBUG)
                    {
                       printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4CSMA"
                              ": CSMA Go-ahead via CSMANewBeacon\n",
                               node->getNodeTime(), node->nodeId);
                    }
                    Csma802_15_4BackoffHandler(node, interfaceIndex);
                }
            }
            else
            {
                wtime = Csma802_15_4AdjustTime(node, interfaceIndex, 0);
                wtime += csmaca->bPeriodsLeft * csmaca->bPeriod;
                if (Csma802_15_4CanProceed(node, interfaceIndex, wtime))
                {
                    csmaca->backoffT = Csma802_15_4SetTimer(
                                                    node,
                                                    interfaceIndex,
                                                    C802_15_4BACKOFFTIMER,
                                                    wtime);
                }
            }
        }
    }
}

// Called by MAC to start CSMA algo to send pkt
//
// \param node  Pointer to node
// \param firsttime  Whether sending for the first time
// \param pkt  Packet to be sent
// \param ackreq  Whether Ack requested
//
void Csma802_15_4Start(Node* node,
                       Int32 interfaceIndex,
                       BOOL firsttime,
                       Message* pkt,
                       BOOL ackreq)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #Start#\n",node->getNodeTime(),node->nodeId);
}

    CsmaData802_15_4* csmaca = NULL;
    MacData802_15_4* mac = NULL;
    BOOL backoff = FALSE;
    Int32 rate = 0;
    clocktype wtime = 0;
    clocktype BI2 = 0;

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;
    csmaca = (CsmaData802_15_4*)mac->csma;

    if (mac->txAck)
    {
        mac->backoffStatus = BACKOFF_RESET;
        csmaca->txPkt = NULL;
        return;
    }

    ERROR_Assert(csmaca->backoffT == NULL, "Backoff timer should be NULL");
    if (firsttime)
    {
        csmaca->beaconEnabled = ((mac->mpib.macBeaconOrder != 15)
                                    || (mac->macBeaconOrder2 != 15));
        csmaca->beaconOther = (mac->macBeaconOrder3 != 15);
        Csma802_15_4Reset(node, interfaceIndex);
        ERROR_Assert(csmaca->txPkt == NULL, "csmaca->txPkt should be NULL");
        csmaca->txPkt = pkt;
        csmaca->ackReq = ackreq;

         rate = Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]);

         csmaca->bPeriod = aUnitBackoffPeriod * SECOND / rate;
        if (csmaca->beaconEnabled)
        {
            csmaca->bcnTxTime = mac->macBcnTxTime;
            csmaca->bcnRxTime = mac->macBcnRxTime;

            BI2 = (mac->sfSpec2.BI * SECOND
                 / Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]));

            if (mac->macBeaconOrder2 != 15)
            {
                // it's possible we missed some beacons
                while (csmaca->bcnRxTime + BI2 < node->getNodeTime())
                {
                    csmaca->bcnRxTime += BI2;
                }
            }
        }
    }

    wtime = (RANDOM_nrand(csmaca->seed) % (1 << csmaca->BE))
                * csmaca->bPeriod;
    wtime = Csma802_15_4AdjustTime(node, interfaceIndex, wtime);
    backoff = TRUE;
    if (csmaca->beaconEnabled || csmaca->beaconOther)
    {
        if (csmaca->beaconEnabled)
        {
            if (firsttime)
            {
                wtime = Mac802_15_4LocateBoundary(
                                node,
                                interfaceIndex,
                                Mac802_15_4ToParent(node, interfaceIndex,
                                        csmaca->txPkt),
                                wtime);
            }
        }
        if (!Csma802_15_4CanProceed(node, interfaceIndex, wtime))
        {
            if (DEBUG)
            {
                printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4CSMA :"
                       "Cannot proceed with CSMA-CA\n",
                       node->getNodeTime(), node->nodeId);
            }
            backoff = FALSE;
        }
    }
    if (backoff)
    {
        if (DEBUG)
        {
            printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4CSMA :"
                   "Backing off for %" TYPES_64BITFMT "d\n",
                   node->getNodeTime(), node->nodeId, wtime);
        }
        csmaca->backoffT = Csma802_15_4SetTimer(node,
                                                interfaceIndex,
                                                C802_15_4BACKOFFTIMER,
                                                wtime);
    }
}

// To cancel CSMA algo
//
// \param node  Pointer to node
//
void Csma802_15_4Cancel(Node* node, Int32 interfaceIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #Cancel#\n",node->getNodeTime(),node->nodeId);
}

    CsmaData802_15_4* csmaca = NULL;
    MacData802_15_4* mac = NULL;

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;
    csmaca = (CsmaData802_15_4*)mac->csma;

    if (csmaca->bcnOtherT != NULL)
    {
        MESSAGE_CancelSelfMsg(node, csmaca->bcnOtherT);
        csmaca->bcnOtherT = NULL;
    }
    else if (csmaca->backoffT != NULL)
    {
        MESSAGE_CancelSelfMsg(node, csmaca->backoffT);
        csmaca->backoffT = NULL;
    }
    else if (csmaca->deferCCAT != NULL)
    {
        MESSAGE_CancelSelfMsg(node, csmaca->deferCCAT);
        csmaca->deferCCAT = NULL;
    }
    else
    {
        mac->taskP.CCA_csmaca_STEP = 0;
    }
    csmaca->txPkt = NULL;
}

// Retruns receive ON status
//
// \param node  Pointer to node
// \param status  RxON status
//
void Csma802_15_4RX_ON_confirm(Node* node,
                               Int32 interfaceIndex,
                               PLMEsetTrxState status)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #RX_ON_confirm#\n",node->getNodeTime(),node->nodeId);
}

    CsmaData802_15_4* csmaca = NULL;
    MacData802_15_4* mac = NULL;
    clocktype now = 0;
    clocktype wtime = 0;

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;
    csmaca = (CsmaData802_15_4*)mac->csma;

    if (status != RX_ON)
    {
        if (status == TX_ON)
        {
            mac->taskP.RX_ON_csmaca_STEP = 1;
        }
        else
        {
            Csma802_15_4BackoffHandler(node, interfaceIndex);
        }
        return;
    }

    // locate backoff boundary if needed
    now = node->getNodeTime();
    if (csmaca->beaconEnabled)
    {
        wtime = Mac802_15_4LocateBoundary(
                    node,
                    interfaceIndex,
                   Mac802_15_4ToParent(node, interfaceIndex, csmaca->txPkt),
                    0);
    }
    else
    {
        wtime = 0;
    }

    if (wtime == 0)
    {
        mac->taskP.CCA_csmaca_STEP = 1;
        Phy802_15_4PLME_CCA_request(node, interfaceIndex);
    }
    else
    {
        csmaca->deferCCAT = Csma802_15_4SetTimer(node,
                                                 interfaceIndex,
                                                 C802_15_4DEFERCCATIMER,
                                                 wtime);
    }
}

// Fn to handle Beacon Other timer
//
// \param node  Pointer to node
//
void Csma802_15_4BcnOtherHandler(Node* node, Int32 interfaceIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #BcnOtherHandler#\n",node->getNodeTime(),node->nodeId);
}

    Csma802_15_4NewBeacon(node, interfaceIndex, 'R');
}

// Fn to handle Defer CCA timer
//
// \param node  Pointer to node
//
void Csma802_15_4DeferCCAHandler(Node* node, Int32 interfaceIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #DeferCCAHandler#\n",node->getNodeTime(),node->nodeId);
}

    CsmaData802_15_4* csmaca = NULL;
    MacData802_15_4* mac = NULL;

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;
    csmaca = (CsmaData802_15_4*)mac->csma;

    if (csmaca->txPkt == NULL || csmaca->txPkt->packet == NULL)
    {
        // nothing to send
        return;
    }
    mac->taskP.CCA_csmaca_STEP = 1;
    Phy802_15_4PLME_CCA_request(node, interfaceIndex);
}

// Returns channel clear access status
//
// \param node  Pointer to node
// \param status  CCA status
//
void Csma802_15_4CCA_confirm(Node* node,
                             Int32 interfaceIndex,
                             PhyStatusType status)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #CCA_confirm#\n",node->getNodeTime(),node->nodeId);
}

    // This function should be called when mac receiving CCA_confirm.
    CsmaData802_15_4* csmaca = NULL;
    MacData802_15_4* mac = NULL;
    BOOL idle = FALSE;

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;
    csmaca = (CsmaData802_15_4*)mac->csma;

    if (status == PHY_IDLE)
    {
        idle = TRUE;
    }
    else
    {
        idle = FALSE;
    }
    if (idle)
    {
        if ((!csmaca->beaconEnabled) && (!csmaca->beaconOther))
        {
            csmaca->txPkt = 0;
            Mac802_15_4CsmacaCallBack(node,interfaceIndex, PHY_IDLE);
        }
        else
        {
            if (csmaca->beaconEnabled)
            {
                csmaca->CW--;
            }
            else
            {
                csmaca->CW = 0;
            }
            if (csmaca->CW == 0)
            {
                // timing condition may not still hold -- check again
                if (Csma802_15_4CanProceed(node, interfaceIndex, 0, TRUE))
                {
                    csmaca->txPkt = 0;
                    Mac802_15_4CsmacaCallBack(node, interfaceIndex,
                        PHY_IDLE);
                }
                else    // postpone until next beacon sent or received
                {
                    if (csmaca->beaconEnabled)
                    {
                        csmaca->CW = 2;
                    }
                    csmaca->bPeriodsLeft = 0;
                }
            }
            else    // perform CCA again
            {
                Csma802_15_4BackoffHandler(node, interfaceIndex);
            }
        }
    }
    else    // busy
    {
        if (DEBUG)
        {
            printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4CSMA :"
                   "Channel busy\n",
                    node->getNodeTime(), node->nodeId);
        }
        if (csmaca->beaconEnabled)
        {
            csmaca->CW = 2;
        }
        csmaca->NB++;
        if (csmaca->NB > mac->mpib.macMaxCSMABackoffs)
        {
            csmaca->txPkt = NULL;
            Mac802_15_4CsmacaCallBack(node, interfaceIndex, PHY_BUSY_TX);
        }
        else    // backoff again
        {
            csmaca->BE++;
            if (csmaca->BE > aMaxBE)
            {
                csmaca->BE = aMaxBE;
            }
            Csma802_15_4Start(node, interfaceIndex, FALSE);
        }
    }
}

// FUNCTION     Csma802_15_4Init
// PURPOSE      Initialization function for CSMA-CA
// PARAMETERS   Node* node
// Node being initialized.
// NodeInput* nodeInput
// Structure containing contents of input file.
// Int32 interfaceIndex
// Interface index.
// RETURN       None
// NOTES        None
void Csma802_15_4Init(Node* node,
                      const NodeInput* nodeInput,
                      Int32 interfaceIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #Init#\n",node->getNodeTime(),node->nodeId);
}

    CsmaData802_15_4* csmaca = NULL;
    MacData802_15_4* mac = NULL;

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;

    csmaca = (CsmaData802_15_4*)MEM_malloc(sizeof(CsmaData802_15_4));
    ERROR_Assert(csmaca != NULL,
                 "802.15.4: Unable to allocate memory for CSMACA data"
                    " struc.");
    memset(csmaca, 0, sizeof(CsmaData802_15_4));

    mac->csma = csmaca;
    RANDOM_SetSeed(csmaca->seed,
                   node->globalSeed,
                   node->nodeId,
                   MAC_PROTOCOL_802_15_4,
                   interfaceIndex);
}

// FUNCTION     Csma802_15_4Layer
// PURPOSE      To handle timer events at CSMA level
// PARAMETERS   Node *node
// Node which received the message.
// Int32 interfaceIndex
// Interface index.
// Message* msg
// Message received by the layer.
// RETURN       None
// NOTES        None
void Csma802_15_4Layer(Node* node, Int32 interfaceIndex, Message* msg)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #Layer#\n",node->getNodeTime(),node->nodeId);
}

    CsmaData802_15_4* csmaca = NULL;
    MacData802_15_4* mac = NULL;
    C802_15_4Timer* timerInfo = NULL;

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;
    csmaca = (CsmaData802_15_4*)mac->csma;
    timerInfo = (C802_15_4Timer*) MESSAGE_ReturnInfo(msg);

    switch(timerInfo->timerType)
    {
        case C802_15_4BACKOFFTIMER:
        {
            csmaca->backoffT = NULL;
            Csma802_15_4BackoffHandler(node, interfaceIndex);
            break;
        }
        case C802_15_4BEACONOTHERTIMER:
        {
            Csma802_15_4BcnOtherHandler(node, interfaceIndex);
            csmaca->bcnOtherT = NULL;
            break;
        }
        case C802_15_4DEFERCCATIMER:
        {
            Csma802_15_4DeferCCAHandler(node, interfaceIndex);
            csmaca->deferCCAT = NULL;
            break;
        }
        default:
        {
            break;
        }
    }
}

// FUNCTION     Csma802_15_4Finalize
// PURPOSE      Called at the end of simulation to clean CSMA
// PARAMETERS   Node* node
// Node which received the message.
// Int32 interfaceIndex
// Interface index.
// RETURN       None
// NOTES        None
void Csma802_15_4Finalize(Node* node, Int32 interfaceIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: csma.cpp in the #Finalize#\n",node->getNodeTime(),node->nodeId);
}

    CsmaData802_15_4* csmaca = NULL;
    MacData802_15_4* mac = NULL;

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;
    csmaca = (CsmaData802_15_4*)mac->csma;
}
