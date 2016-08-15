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


// PROTOCOL     :: Queue-Scheduler
// REFERENCES   ::
// + "EMPIRICAL STUDY OF BUFFER MANAGEMENT SCHEMES FOR
// DIFFSERV ASSURED FORWARDING PHB"
// www.sce.carleton.ca/faculty/lambadaris/recent-papers/162.pdf
// COMMENTS     :: None

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "api.h"
#include "network_ip.h"

#include "if_queue.h"
#include "queue_red.h"
#include "queue_red_ecn.h"
#include "queue_wred_ecn.h"
#include "queue_rio_ecn.h"

//--------------------------------------------------------------------------
// Start: Utility Functions
//--------------------------------------------------------------------------

// Returns average queue size and counter for Generalized Rio
// Coupled Virtual Queue to the function
// GenericRedMarkThePacket() As two color marking specified,
// the average queue size of green color(DP0) packets will be
// calculated using green color packets only. For yellow
// color(DP1) packets, the average queue will be calculated
// using red and yellow packets.In RIO-C with two color
// marking, there is an inherent assumption that drops DP1
// packets more aggressively than DP0 packets. Services based
// on AF implementations utilizing RIO-C will be subject to
// this restriction.
// 
//
// \param avgQueueSize  The Average Queue size pointer
// \param packetCountPtr  Returning the counter
// \param dscp  The dscp of IP TOS field of the packet
// \param currentTime  Current simulation time
//

void EcnRioQueue::RioCoupledTCReturnAvgQSizePktCountPtr(
    double *avgQueueSize,
    int** packetCountPtr,
    const int dscp,
    const clocktype currentTime)
{
    // At each packet arrival Update average queue size for the red queue.
    // As for average queue size calculation for yellow profile packet will
    // consider both green and yellow profile packet. So general
    // average queue size will give the average queue size for yellow
    // profile packets

    UpdateAverageQueueSize(
            isEmpty(),
            packetsInQueue(),
            rio->redEcnParams->queueWeight,
            rio->redEcnParams->typicalSmallPacketTransmissionTime,
            startIdleTime,
            &averageQueueSize,
            currentTime);

    // Updating average queue size for Green packet
    UpdateAverageQueueSize(
        numGreenPackets == 0 ? TRUE : FALSE,
        numGreenPackets,
        rio->redEcnParams->queueWeight,
        rio->redEcnParams->typicalSmallPacketTransmissionTime,
        startGreenIdleTime,
        &averageGreenQueueSize,
        currentTime);

    if (MredReturnPacketProfile(dscp) == GREEN_PROFILE)
    {
        // Returning average queue size for Green packet
        *packetCountPtr = &packetGreenCount;
        *avgQueueSize = averageGreenQueueSize;
    }
    else
    {
        // Returning average queue size for yellow packet
        *packetCountPtr = &packetCount;
        *avgQueueSize = averageQueueSize;
    }
}


// If a green packet is enqueued, update the variable number
// of Green Packets.
//
// \param dscp  The dscp of IP TOS field of the packet
//

void EcnRioQueue::RioCoupledTCUpdIntVarForEnqueue(const int dscp)
{

    if (MredReturnPacketProfile(dscp) == GREEN_PROFILE)
    {
         // This packet is an green one, update number of green packets
         // & number of yellow packets for RIO_COUPLED

         numGreenPackets++;
    }
}


// If a packet is dequeued, update Number of Green or Red or
// Yellow packets currently in the queue
//
// \param dscp  The dscp or IP TOS field of the packet
// \param currentTime  Current simulation time
//

void EcnRioQueue::RioCoupledTCUpdIntVarForDequeue(
    const int dscp,
    const clocktype currentTime)
{
    if (MredReturnPacketProfile(dscp) == GREEN_PROFILE)
    {
         // This packet is an green one, update number of Green Packets
         // & numYellowPackets for RIO_COUPLED

         numGreenPackets--;

         if (numGreenPackets == 0)
         {
             startGreenIdleTime = currentTime;
         }
    }
}


// Returns average queue size and counter for Generalized Rio
// Coupled Virtual Queue to the function
// GenericRedMarkThePacket() The average queue size of green
// color(DP0) packets will be calculated using green color
// packets only. For yellow color (DP1) packets, the average
// queue will be calculated using green and yellow packets.
// For red(DP2) packets, the average queue will be calculated
// using red, yellow and green packets. RIO-C derives its name
// using from the coupled relationship of the average queue
// calculation. Thus, there is an inherent assumption that
// it is better to drop DP1 and DP2 packets for DP0 packets
// Similarly, for DP1 packets, it is assumed better to drop
// DP2 packets. Services based on AF implementations utilizing
// RIO-C will be subject to this restriction.
//PARAMETERS  ::
// + avgQueueSize : double* : average queue size
// + packetCountPtr : int** : Returning the counter
// + dscp : const int : The dscp or IP TOS field of the packet
// + currentTime : const clocktype : Current simulation time

void EcnRioQueue::RioCoupledThCReturnAvgQSizePktCountPtr(
    double *avgQueueSize,
    int** packetCountPtr,
    const int dscp,
    const clocktype currentTime)
{
    // At each packet arrival Update average queue size for the red queue
    // As for average queue size calculation for red profile packet will
    // consider both green and yellow profile packet as well, so general
    // average queue size will give the average queue size for red profile
    // packets

    UpdateAverageQueueSize(
            isEmpty(),
            packetsInQueue(),
            rio->redEcnParams->queueWeight,
            rio->redEcnParams->typicalSmallPacketTransmissionTime,
            startIdleTime,
            &averageQueueSize,
            currentTime);

    // Updating average queue size for Green packet
    UpdateAverageQueueSize(
        numGreenPackets == 0 ? TRUE : FALSE,
        numGreenPackets,
        rio->redEcnParams->queueWeight,
        rio->redEcnParams->typicalSmallPacketTransmissionTime,
        startGreenIdleTime,
        &averageGreenQueueSize,
        currentTime);

    // Updating average queue size for Yellow packet
    UpdateAverageQueueSize(
        numYellowPackets == 0 ? TRUE : FALSE,
        numYellowPackets,
        rio->redEcnParams->queueWeight,
        rio->redEcnParams->typicalSmallPacketTransmissionTime,
        startYellowIdleTime,
        &averageYellowQueueSize,
        currentTime);

    if (MredReturnPacketProfile(dscp) == GREEN_PROFILE)
    {
        // Returning average queue size for Green packet
        *packetCountPtr = &packetGreenCount;
        *avgQueueSize = averageGreenQueueSize;
    }
    else if (MredReturnPacketProfile(dscp) == YELLOW_PROFILE)
    {
        // Returning average queue size for Yellow packet
        *packetCountPtr = &packetYellowCount;
        *avgQueueSize = averageYellowQueueSize;
    }
    else
    {
        // Returning average queue size for Red packet
        *packetCountPtr = &packetCount;
        *avgQueueSize = averageQueueSize;
    }
}


// If a packet is enqueued, update the variable number of Green
// Packets and number of Yellow Packets.
//
// \param dscp  The dscp of IP TOS field of the packet
//

void EcnRioQueue::RioCoupledThCUpdIntVarForEnqueue(const int dscp)
{
    if (MredReturnPacketProfile(dscp) == GREEN_PROFILE)
    {
         // This packet is an green one, update number of green packets
         // & number of yellow packets for RIO_COUPLED
         numGreenPackets++;
         numYellowPackets++;
    }

    if (MredReturnPacketProfile(dscp) == YELLOW_PROFILE)
    {
        // This packet is an Yellow one, update numYellowPackets
        numYellowPackets++;
    }
}


// If a packet is dequeued, update Number of Green or Red or
// Yellow packets currently in the queue
//
// \param dscp  The dscp or IP TOS field of the packet
// \param currentTime  Current simulation time
//

void EcnRioQueue::RioCoupledThCUpdIntVarForDequeue(
    const int dscp,
    const clocktype currentTime)
{
    if (MredReturnPacketProfile(dscp) == GREEN_PROFILE)
    {
         // This packet is an green one, update number of Green Packets
         // & numYellowPackets for RIO_COUPLED

         numGreenPackets--;
         numYellowPackets--;

         if (numGreenPackets == 0)
         {
             startGreenIdleTime = currentTime;

             if (numYellowPackets == 0)
             {
                startYellowIdleTime = currentTime;
             }
         }
    }

    if (MredReturnPacketProfile(dscp) == YELLOW_PROFILE)
    {
         // This packet is an Yellow one, update number of Yellow Packets
         numYellowPackets--;

         if (numYellowPackets == 0)
         {
             startYellowIdleTime = currentTime;
         }
    }
}


// Returns average queue size and Counter for Generalized Rio
// Decoupled Virtual Queue to GenericRedMarkThePacket()
// In RIO-DC (RIO with Decoupled Queues), the average queue for
// packets of each color is calculated independently. With two
// color markingThe average queue length for green and yellow
// packets will be calculated using number of green & yellow
// packets in the queue respectively. RIO-DC starts WRR
// scheduling scheme between different color profile in a
// single queue.
//
// \param avgQueueSize  The Average Queue size pointer
// \param packetCountPtr  Returning the counter
// \param dscp  The dscp of IP TOS field of the packet
// \param currentTime  Current simulation time
//

void EcnRioQueue::RioDecoupledTCReturnAvgQSizePktCountPtr(
    double *avgQueueSize,
    int** packetCountPtr,
    const int dscp,
    const clocktype currentTime)
{
    // At each packet arrival Update average queue size
    // Updating average queue size for Green packet

    UpdateAverageQueueSize(
            numGreenPackets == 0 ? TRUE : FALSE,
            numGreenPackets,
            rio->redEcnParams->queueWeight,
            rio->redEcnParams->typicalSmallPacketTransmissionTime,
            startGreenIdleTime,
            &averageGreenQueueSize,
            currentTime);


    // Updating average queue size for Yellow packet
    UpdateAverageQueueSize(
            numYellowPackets == 0 ? TRUE : FALSE,
            numYellowPackets,
            rio->redEcnParams->queueWeight,
            rio->redEcnParams->typicalSmallPacketTransmissionTime,
            startYellowIdleTime,
            &averageYellowQueueSize,
            currentTime);

    if (MredReturnPacketProfile(dscp) == GREEN_PROFILE)
    {
        // Returning average queue size for Green packet
        *packetCountPtr = &packetGreenCount;
        *avgQueueSize = averageGreenQueueSize;
    }

    if (MredReturnPacketProfile(dscp) == YELLOW_PROFILE)
    {
        // Returning average queue size for Yellow packet
        *packetCountPtr = &packetYellowCount;
        *avgQueueSize = averageYellowQueueSize;
    }
}


// If  a packet is enqueued, update the variable number of
// green packets and number of yellow packets.
//
// \param dscp  The dscp of IP TOS field of the packet
//

void EcnRioQueue::RioDecoupledTCUpdIntVarForEnqueue(const int dscp)
{
    if (MredReturnPacketProfile(dscp) == GREEN_PROFILE)
    {
         // This packet is an green one, update number of Green Packets
         numGreenPackets++;
    }
    else // YELLOW_PROFILE
    {
        // This packet is an Yellow one, update number of Yellow Packets
        numYellowPackets++;
    }
}


// If a packet is dequeued,update the variable number of Green
// packets and number of Yellow Packets.
//
// \param dscp  The dscp of IP TOS field of the packet
// \param currentTime  Current simulation time
//

void EcnRioQueue::RioDecoupledTCUpdIntVarForDequeue(
        const int dscp,
        const clocktype currentTime)
{
    if (MredReturnPacketProfile(dscp) == GREEN_PROFILE)
    {
         // This packet is an green one, update number of Green Packets
         numGreenPackets--;

         if (numGreenPackets == 0)
         {
             startGreenIdleTime = currentTime;
         }
    }
    else //YELLOW_PROFILE
    {
         // This packet is an Yellow one, update number of Yellow Packets
         numYellowPackets--;

         if (numYellowPackets == 0)
         {
              startYellowIdleTime = currentTime;
         }
    }
}


// Returns average queue size and Counter for Generalized Rio
// Decoupled Virtual Queue to function
// GenericRedMarkThePacket() In RIO-DC (RIO with Decoupled
// Queues), the average queue for packets of each color is
// calculated independently. The average queue length for
// green, yellow and red packets will be calculated using the
// number of green, yellow and red packets in the queue
// respectively. RIO-DC appears to be the most suitable
// MRED-variant if the operator desires to assign weights to
// different colored packets within the same queue i.e. there
// is no relationship between the packets marked green and
// yellow. Thus, the RIO-DC scheme could start to emulate the
// WRR scheduling scheme except that a single queue is used.
//
// \param avgQueueSize  The Average Queue size pointer
// \param packetCountPtr  Returning the counter
// \param dscp  The dscp of IP TOS field of the packet
// \param currentTime  Current simulation time.
//

void EcnRioQueue::RioDecoupledThCReturnAvgQSizePktCountPtr(
    double *avgQueueSize,
    int** packetCountPtr,
    const int dscp,
    const clocktype currentTime)
{
    // Updating average queue size for Red packet
    UpdateAverageQueueSize(
            numRedPackets == 0 ? TRUE : FALSE,
            numRedPackets,
            rio->redEcnParams->queueWeight,
            rio->redEcnParams->typicalSmallPacketTransmissionTime,
            startRedIdleTime,
            &averageRedQueueSize,
            currentTime);

    // Updating average queue size for Green and Yellow packet.
    // Returning the packetCount & avgQueueSize if the packet
    // is Green or Yellow profile.

    RioDecoupledTCReturnAvgQSizePktCountPtr(
         avgQueueSize,
         packetCountPtr,
         dscp,
         currentTime);

    if (MredReturnPacketProfile(dscp) == RED_PROFILE)
    {
         // Returning average queue size for Red packet
         *packetCountPtr = &packetRedCount;
         *avgQueueSize = averageRedQueueSize;
    }
}


// If  a packet is enqueued, update the variable number of
// green packets, number of yellow packets and number of
// red packets for Generalized RIO-DC.
//
// \param dscp  The dscp of IP TOS field of the packet
//

void EcnRioQueue::RioDecoupledThCUpdIntVarForEnqueue(const int dscp)
{
    if (MredReturnPacketProfile(dscp) == RED_PROFILE)
    {
        // This packet is an Red one, update number of Red Packets
        numRedPackets++;
    }
    else
    {
        RioDecoupledTCUpdIntVarForEnqueue(dscp);
    }
}


// If a packet is dequeued,update the variable number of Green
// packets, number of Yellow Packets and number of Red Packets
//
// \param dscp  The dscp of IP TOS field of the packet
// \param currentTime  Current simulation time
//

void EcnRioQueue::RioDecoupledThCUpdIntVarForDequeue(
    const int dscp,
    const clocktype currentTime)
{
    if (MredReturnPacketProfile(dscp) == RED_PROFILE)
    {
         // This packet is an red one, update number of Red Packets
         numRedPackets--;

         if (numRedPackets == 0)
         {
             startRedIdleTime = currentTime;
         }
    }
    else
    {
        RioDecoupledTCUpdIntVarForDequeue(dscp, currentTime);
    }
}


//--------------------------------------------------------------------------
// End: Utility Functions
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// Start: Public Interface functions
//--------------------------------------------------------------------------

// This function prototype determines the arguments that need
// to be passed to a queue data structure in order to insert
// a message into it.  The infoField parameter has a specified
// size infoFieldSize, which is set at Initialization, and
// points to the structure that should be stored along
// with the Message.
//
// \param msg  Pointer to Message structure
// \param infoField  The infoField parameter
// \param QueueIsFull  returns Queue occupancy status
// \param currentTime  Current Simulation time
// \param serviceTag  ServiceTag
//

void EcnRioQueue::insert(
    Message* msg,
    const void* infoField,
    BOOL* QueueIsFull,
    const clocktype currentTime,
    const double serviceTag)
{
    insert(msg, infoField, QueueIsFull, currentTime, NULL, serviceTag);
}

void EcnRioQueue::insert(
    Message* msg,
    const void *infoField,
    BOOL* QueueIsFull,
    const clocktype currentTime,
    TosType* tos,
    const double serviceTag
    )
{
    double avgQueueSize = 0.0;
    int *packetCountPtr;
    BOOL isECTset = FALSE;
    BOOL wasEcnMarked = FALSE;
    BOOL isBetweenMinthMaxth = FALSE;

    ERROR_Assert(MESSAGE_ReturnPacketSize(msg) > 0,
        "Queue Error: Attempted to queue a packet of 0 length");

    int ip_tos;

    if (tos == NULL)
    {
        ip_tos = GetTosFromMsgHeader(msg);
    }
    else
    {
       ip_tos = (int) *tos;
    }

    PhbParameters *phbParam = MredReturnPhbForDs(rio->redEcnParams,
                                                      ip_tos >> 2);

    if (rio->rioMarkerType == TWO_COLOR)
    {
        // As RIO-COLOR-MODE is specified as TWO-COLOR, there should not be
        // any RED profile packet.

        ERROR_Assert(!(((ip_tos >> 2) & DSCP_FOURTH_BIT) &&
            ((ip_tos >> 2) & DSCP_FIFTH_BIT)),
            "Trying to process RED profile packet with TWO-COLOR RIO");
    }

    if (RIO_DEBUG)
    {
        printf("RIO: Trying to Insert a Pkt with DSCP(%u)\n", ip_tos >> 2);
    }

    if (ip_tos & IPTOS_CE)
    {
        wasEcnMarked = TRUE;
    }

    if (ip_tos & IPTOS_ECT)
    {
        isECTset = TRUE;
    }

    if (MESSAGE_ReturnPacketSize(msg) > (queueSizeInBytes - bytesUsed))
    {
        *QueueIsFull = TRUE;

        if (isCollectStats)
        {
            Queue::stats->DropPacket(msg, bytesUsed, currentTime);
        }

        // Update PHB Specific stats
        if (isCollectStats)
        {
            RedEcnStats *stats = phbParam->stats;

            stats->numPacketsDropped++;
            stats->numBytesDropped += MESSAGE_ReturnPacketSize(msg);
        }

        if (RIO_DEBUG)
        {
            printf("\t\t\tRIO: No space in Queue; Just dropping Packet with"
                " DSCP(%u)\n", ip_tos >> 2);
        }

        return; // No space for this item in the queue
    }

    // At each packet arrival Update average queue sizes and return
    // the average queue size for respective packet profile.

    (this->*rioReturnAvgQueueSizePtr)(
        &avgQueueSize,
        &packetCountPtr,
        ip_tos >> 2,
        currentTime);

    if (RIO_DEBUG)
    {
        printf("\t\t\tRIO: Average Queue Size: %f Minth: %u Maxth: %u\n",
            avgQueueSize, phbParam->minThreshold, phbParam->maxThreshold);
    }

    // Check whether packet will be dropped or marked
    if (MarkThePacket(
            phbParam,
            avgQueueSize,
            packetCountPtr,
            &isBetweenMinthMaxth,
            isECTset))
    {
        // When a router has decided from its active queue management
        // mechanism, to drop or mark a packet, it checks the IP-ECT
        // bit in the packet header. It sets the CE bit in the IP header
        // if the IP-ECT bit is set.

        if (rio->redEcnParams->isEcn && isECTset && isBetweenMinthMaxth)
        {
            if (tos == NULL)
            {
                SetCeBitInMsgHeader(msg);
                ip_tos = GetTosFromMsgHeader(msg);
            }
            else
            {
                *tos |= IPTOS_CE ;
                ip_tos = (int) *tos ;
            }
        }
        else
        {
            *QueueIsFull = TRUE;
            if (isCollectStats)
            {
                Queue::stats->DropPacket(msg, bytesUsed, currentTime);
            }

            if (isCollectStats)
            {
                RedEcnStats *stats = phbParam->stats;

                stats->numPacketsDropped++;
                stats->numBytesDropped += MESSAGE_ReturnPacketSize(msg);
            }

            if (RIO_DEBUG)
            {
                    printf("\t\t\tRIO: Random drop;Packet with"
                        " DSCP(%u)\n", ip_tos >> 2);
            }
            return;
        }
    }

    if (RIO_DEBUG)
    {
        printf("\t\t\tRIO: Inserting a Packet with"
            " DSCP(%u)\n", ip_tos >> 2);
    }

    int dscp = ip_tos >> 2;
    UInt64 key = (*msg).originatingNodeId;
    key <<= 32;
    key |= (*msg).sequenceNumber;
    queue_profile[key] = dscp;

    // Inserting a packet in the queue.
    Queue::insert(msg,
                  infoField,
                  QueueIsFull,
                  currentTime,
                  serviceTag);

    // Update variables and statistics.
    if (*QueueIsFull == FALSE)
    {
        (this->*rioUpdateEnqueueVar)(ip_tos >> 2);
    }

    if (isCollectStats)
    {
        RedEcnStats *stats = phbParam->stats;

        if ((!wasEcnMarked) && (ip_tos & IPTOS_CE))
        {
            stats->numPacketsECNMarked++;
            numPacketsECNMarked++;
        }

        if (*QueueIsFull)
        {
            stats->numPacketsDropped++;
            stats->numBytesDropped += MESSAGE_ReturnPacketSize(msg);
        }
        else
        {
            stats->numPacketsQueued++;
            stats->numBytesQueued += MESSAGE_ReturnPacketSize(msg);
        }
    }
}


// This function prototype determines the arguments that need
// to be passed to a queue data structure in order to dequeue,
// peek at, or drop a message in its array of stored messages.
// It now includes the "DropFunction" functionality as well.
//
// \param msg  The retrieved msg
// \param index  The position of the packet in the queue
// \param operation  The retrieval mode
// \param currentTime  Current Simulation time
// \param serviceTag  ServiceTag = NULL
//
// \return TRUE or FALSE

BOOL EcnRioQueue::retrieve(
    Message** msg,
    const int index,
    const QueueOperation operation,
    const clocktype currentTime,
    double* serviceTag)
{
    BOOL isPacketRetrieve = FALSE;

    // Dequeueing a packet from the queue
    isPacketRetrieve = Queue::retrieve(msg,
                                        index,
                                        operation,
                                        currentTime,
                                        serviceTag);

    if ((operation == DEQUEUE_PACKET) || (operation == DISCARD_PACKET))
    {
        UInt64 key = (**msg).originatingNodeId;
        key <<= 32;
        key |= (**msg).sequenceNumber;
        int dscp = queue_profile[key];
        queue_profile.erase(key);

        if (RIO_DEBUG)
        {
            printf("RIO: Dequeueing a Packet with"
                " DSCP(%u) \n", dscp);
        }

        if (isEmpty())
        {
            startIdleTime = currentTime;
        }

        if (isCollectStats)
        {
            PhbParameters *phbParam = MredReturnPhbForDs(rio->redEcnParams,
                                                        dscp);
            RedEcnStats *stats = phbParam->stats;

            if (operation == DEQUEUE_PACKET)
            {
                stats->numPacketsDequeued++;
                stats->numBytesDequeued += MESSAGE_ReturnPacketSize((*msg));
            }

            if (operation == DISCARD_PACKET)
            {
                stats->numPacketsDropped++;
                stats->numBytesDropped += MESSAGE_ReturnPacketSize((*msg));
            }
        }

        (this->*rioUpdateDequeueVar)(dscp, currentTime);
    }
    return isPacketRetrieve;
}


// This function is proposed to replicate the state of the
// queue, as if it had been the operative queue all along.
// If there are packets in the existing queue, they are
// transferred one-by-one into the new queue. This can result
// in additional drops of packets that had previously been
// stored. This function returns the number of additional
// drops.
//
// \param oldQueue  old packetArray
//
// \return Integer

int EcnRioQueue::replicate(Queue* oldQueue)
{
    int numExtraDrop = 0;
    numExtraDrop = Queue::replicate(oldQueue);

    // Replicate RED specific enties
    if (isCollectStats)
    {
        // Assuming that all statistics in old Queue belongs
        // to GREEN profile PHB only

        RedEcnStats* stats = rio->redEcnParams->phbParams[0].stats;

        stats->numPacketsQueued = (int)Queue::stats->GetPacketsEnqueued().GetValue(node->getNodeTime());
        stats->numPacketsDequeued = (int)Queue::stats->GetPacketsDequeued().GetValue(node->getNodeTime());
        stats->numPacketsDropped = (int)Queue::stats->GetPacketsDropped().GetValue(node->getNodeTime());
        stats->numBytesQueued = (int)Queue::stats->GetBytesEnqueued().GetValue(node->getNodeTime());
        stats->numBytesDequeued = (int)Queue::stats->GetBytesDequeued().GetValue(node->getNodeTime());;
        stats->numBytesDropped = (int)Queue::stats->GetBytesDropped().GetValue(node->getNodeTime());;
    }

    return numExtraDrop;
}


// This function prototype outputs the final statistics for
// this queue.  The layer, protocol, interfaceAddress, and
// instanceId parameters are given to IO_PrintStat with each
// of the queue's statistics.
//
// \param node  Pointer to Node structure
// \param layer  The layer string
// \param interfaceIndex  The interface index
// \param instanceId  Instance Ids
// \param invokingProtocol  The protocol string
//

void EcnRioQueue::finalize(
    Node *node,
    const char *layer,
    const int interfaceIndex,
    const int instanceId,
    const char* invokingProtocol,
    const char* splStatStr)
{
    char buf[MAX_STRING_LENGTH] = {0};
    char operationType[MAX_STRING_LENGTH] = {0};
    char intfIndexStr[MAX_STRING_LENGTH] = {0};

    if (!isCollectStats)
    {
        return;
    }

    if (strcmp(invokingProtocol, "IP") == 0)
    {
        // IP queue finalization
        NetworkIpGetInterfaceAddressString(node,
                                           interfaceIndex,
                                           intfIndexStr);
    }
    else
    {
        strcpy(operationType, invokingProtocol);
        sprintf(intfIndexStr, "%d", interfaceIndex);
    }

    if (rio->rioCountingType == RIO_COUPLED)
    {
        strcat(operationType, " RIO[C]");
    }
    else // RIO_DECOUPLED
    {
        strcat(operationType, " RIO[DC]");
    }

    RedEcnQueueFinalize(
        node,
        layer,
        intfIndexStr,
        instanceId,
        GREEN_PROFILE_PHB(rio->redEcnParams)->stats,
        operationType,
        "GREEN");

    RedEcnQueueFinalize(
        node,
        layer,
        intfIndexStr,
        instanceId,
        YELLOW_PROFILE_PHB(rio->redEcnParams)->stats,
        operationType,
        "YELLOW");

    if (rio->rioMarkerType == THREE_COLOR)
    {
        RedEcnQueueFinalize(
            node,
            layer,
            intfIndexStr,
            instanceId,
            RED_PROFILE_PHB(rio->redEcnParams)->stats,
            operationType,
            "RED");
    }


    FinalizeQueue(node,
                  layer,
                  operationType,
                  interfaceIndex,
                  instanceId,
                  invokingProtocol);

    sprintf(buf, "Packets Marked ECN = %d", numPacketsECNMarked);
    IO_PrintStat(
        node,
        layer,
        operationType,
        intfIndexStr,
        instanceId,
        buf);
}

// This function is proposed for RED queue initialization
//
// \param node  node
// \param queueTypeString[]  Queue type string
// \param queueSize  Queue size in bytes
// \param interfaceIndex  used for setting random seed
// \param queueNumber  used for setting random seed
// \param infoFieldSize  Default infoFieldSize = 0
// \param enableQueueStat  Default enableQueueStat = false
// \param showQueueInGui  If want to show this Queue in GUI
//    Default value is false;
// \param currentTime  Current simulation time
// \param configInfo  pointer to a structure that contains
//    configuration parameters for a Queue Type.
//    This is not required for FIFO. Is it is
//    found NULL for any other Queue Type the
//    default setting will be applied for that
//    queue Type. Default value is NULL
//

EcnRioQueue::EcnRioQueue(
    Node* node,
    const char queueTypeString[],
    const int queueSize,
    const int interfaceIndex,
    const int queueNumber,
    const int infoFieldSize,
    const bool enableQueueStat,
    const bool showQueueInGui,
    const clocktype currentTime,
    const RioParameters* configInfo
#ifdef ADDON_DB
    , const char* queuePosition
    , const bool isFromNetworkLayer
#endif
    , const clocktype maxPktAge
    ): EcnWredQueue(node,
                    queueTypeString,
                    queueSize,
                    interfaceIndex,
                    queueNumber,
                    true,
                    infoFieldSize,
                    enableQueueStat,
                    showQueueInGui
#ifdef ADDON_DB
                    , 0 , queuePosition, isFromNetworkLayer
#else
                    ,currentTime
#endif
                    , maxPktAge
                    )
{
    // Initialization of Red queue variables
    RANDOM_SetSeed(randomDropSeed,
                   node->globalSeed,
                   node->nodeId,
                   interfaceIndex,
                   queueNumber);

    averageQueueSize = 0.0;
    packetCount = -1;
    startIdleTime = currentTime;

    // Initialization of RIO specific variable
    numGreenPackets = 0;
    averageGreenQueueSize = 0.0;
    packetGreenCount =  -1;
    startGreenIdleTime = currentTime;

    numYellowPackets = 0;
    averageYellowQueueSize = 0.0;
    packetYellowCount =  -1;
    startYellowIdleTime = currentTime;

    numRedPackets = 0;
    averageRedQueueSize = 0.0;
    packetRedCount = -1;
    startRedIdleTime = currentTime;

    // ECN specific variable Initilization
    numPacketsECNMarked = 0;

    if (configInfo == NULL)
    {
        int i = 0;
        rio = (RioParameters*) MEM_malloc(sizeof (RioParameters));
        rio->rioCountingType = RIO_COUPLED;
        rio->rioMarkerType = THREE_COLOR;
        rio->redEcnParams = (RedEcnParameters*)
                MEM_malloc(sizeof (RedEcnParameters));
        memset(rio->redEcnParams, 0, sizeof (RedEcnParameters));

        // Initiate Plug-n-Play by assigning default values
        rio->redEcnParams->numPhbParams = MRED_NUM_PHB;
        rio->redEcnParams->queueWeight = DEFAULT_RED_QUEUE_WEIGHT;
        rio->redEcnParams->typicalSmallPacketTransmissionTime
                       = DEFAULT_RED_SMALL_PACKET_TRANSMISSION_TIME;

        rio->redEcnParams->phbParams = (PhbParameters*)
            MEM_malloc(sizeof(PhbParameters) * MRED_NUM_PHB);

        rio->redEcnParams->phbParams[GREEN_PROFILE].minThreshold
                  = DEFAULT_GREEN_PROFILE_MIN_THRESHOLD;
        rio->redEcnParams->phbParams[GREEN_PROFILE].maxThreshold
                  = DEFAULT_GREEN_PROFILE_MAX_THRESHOLD;
        rio->redEcnParams->phbParams[GREEN_PROFILE].maxProbability
                          = DEFAULT_RED_MAX_PROBABILITY;
        rio->redEcnParams->phbParams[YELLOW_PROFILE].minThreshold
                  = DEFAULT_YELLOW_PROFILE_MIN_THRESHOLD;
        rio->redEcnParams->phbParams[YELLOW_PROFILE].maxThreshold
                  = DEFAULT_YELLOW_PROFILE_MAX_THRESHOLD;
        rio->redEcnParams->phbParams[YELLOW_PROFILE].maxProbability
                          = DEFAULT_RED_MAX_PROBABILITY;
        rio->redEcnParams->phbParams[RED_PROFILE].minThreshold
                      = DEFAULT_RED_PROFILE_MIN_THRESHOLD;
        rio->redEcnParams->phbParams[RED_PROFILE].maxThreshold
                      = DEFAULT_RED_PROFILE_MAX_THRESHOLD;
        rio->redEcnParams->phbParams[RED_PROFILE].maxProbability
                          = DEFAULT_RED_MAX_PROBABILITY;

        for (i = 0; i < rio->redEcnParams->numPhbParams; i++)
        {
            rio->redEcnParams->phbParams[i].stats
                = (RedEcnStats*) MEM_malloc(sizeof(RedEcnStats));

            memset(rio->redEcnParams->phbParams[i].stats, 0,
                                            sizeof(RedEcnStats));
        }

        if (DEBUG_PLUG_N_PLAY)
        {
            fprintf(stdout, "\n\n$ RIO Plug-n-Play enabled\n"
                "......................................................\n"
                "RIO_COUPLED in THREE_COLOR mode...\n"
                "GREEN-PROFILE-MIN-THRESHOLD        10\n"
                "GREEN-PROFILE-MAX-THRESHOLD        20\n"
                "GREEN-PROFILE-MAX-PROBABILITY      0.02\n"
                "YELLOW-PROFILE-MIN-THRESHOLD       5\n"
                "YELLOW-PROFILE-MAX-THRESHOLD       10\n"
                "YELLOW-PROFILE-MAX-PROBABILITY     0.02\n"
                "RED-PROFILE-MIN-THRESHOLD          2\n"
                "RED-PROFILE-MAX-THRESHOLD          5\n"
                "RED-PROFILE-MAX-PROBABILITY        0.02\n"
                "RED-QUEUE-WEIGHT      0.002\n"
                "RED-SMALL-PACKET-TRANSMISSION-TIME   10MS\n"
                "......................................................\n");
        }
    }
    else
    {
        // Reading Rio parameters from configuration files.
        rio = (RioParameters*) configInfo;
    }

    if (rio->rioCountingType == RIO_COUPLED)
    {
        if (rio->rioMarkerType == TWO_COLOR)
        {
            if (RIO_DEBUG)
            {
                printf("RIO: Two Color Coupled Mode\n");
            }

            rioReturnAvgQueueSizePtr =
                &EcnRioQueue::RioCoupledTCReturnAvgQSizePktCountPtr;

            // Assign the function pointer for updating variable
            // after enqueueing a packet for RIO-C.

            rioUpdateEnqueueVar =
                &EcnRioQueue::RioCoupledTCUpdIntVarForEnqueue;

            // Assign the function pointer for updating variable
            // after dequeueing a packet for RIO-C.

            rioUpdateDequeueVar =
                &EcnRioQueue::RioCoupledTCUpdIntVarForDequeue;
        }
        else
        {
            if (RIO_DEBUG)
            {
                printf("RIO: Three Color Coupled Mode\n");
            }

            // Assign the function pointer Returning AverageQueueSize and
            // PacketCountPtrType for RIO coupled with three color marking.

            rioReturnAvgQueueSizePtr =
            &EcnRioQueue::RioCoupledThCReturnAvgQSizePktCountPtr;

            // Assign the function pointer for updating variable
            // after enqueueing a packet for RIO-C.

            rioUpdateEnqueueVar =
                &EcnRioQueue::RioCoupledThCUpdIntVarForEnqueue;

            // Assign the function pointer for updating variable
            // after dequeueing a packet for RIO-C.

            rioUpdateDequeueVar =
                &EcnRioQueue::RioCoupledThCUpdIntVarForDequeue;
        }
    }
    else // DECOUPLED
    {
        if (rio->rioMarkerType == TWO_COLOR)
        {
            if (RIO_DEBUG)
            {
                printf("RIO: Two Color Decoupled Mode\n");
            }

            // Assign the function pointer Returning AverageQueueSize and
            // PacketCountPtrType for RIO-DC with two color marking.

            rioReturnAvgQueueSizePtr =
                &EcnRioQueue::RioDecoupledTCReturnAvgQSizePktCountPtr;

            // Assign the function pointer for updating variable
            // after enqueueing a packet for RIO-DC.

            rioUpdateEnqueueVar =
                &EcnRioQueue::RioDecoupledTCUpdIntVarForEnqueue;

            // Assign the function pointer for updating variable
            // after dequeueing a packet for RIO-DC.

            rioUpdateDequeueVar =
                &EcnRioQueue::RioDecoupledTCUpdIntVarForDequeue;
        }
        else
        {
            if (RIO_DEBUG)
            {
                printf("RIO: Three Color Decoupled Mode\n");
            }

            // Assign the function pointer Returning AverageQueueSize and
            // PacketCountPtrType for RIO-DC with three color marking.

            rioReturnAvgQueueSizePtr =
                &EcnRioQueue::RioDecoupledThCReturnAvgQSizePktCountPtr;

            // Assign the function pointer for updating variable
            // after enqueueing a packet for RIO-DC.

            rioUpdateEnqueueVar =
                &EcnRioQueue::RioDecoupledThCUpdIntVarForEnqueue;

            // Assign the function pointer for updating variable
            // after dequeueing a packet for RIO-DC.

            rioUpdateDequeueVar =
                &EcnRioQueue::RioDecoupledThCUpdIntVarForDequeue;
        }
    }
}

//--------------------------------------------------------------------------
// End: Public Interface functions
//--------------------------------------------------------------------------

// -------------------------------------------------------------------------
// Start: Read and configure RIO
// -------------------------------------------------------------------------


// Reads Two color MRED Threshold parameters from config file
//
// \param node  Pointer to Node structure
// \param interfaceIndex  Interface Index
// \param nodeInput  Pointer to NodeInput
// \param isCollectStats  Tag for enabling Queue Statistics
// \param queueIndex  Queue Index
// \param mred  Pointer to
//    RedEcnParameters Structure that keeps all configurable
//    entries for ECN enabled Multilevel RED Queue.
//

static
void ReadTwoColorMredPhbParameters(
    Node* node,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const BOOL isCollectStats,
    int queueIndex,
    RedEcnParameters* mred)
{
    BOOL retVal = FALSE;
    int nodeId = node->nodeId;

    IO_ReadIntInstance(
        node,
        nodeId,
        interfaceIndex,
        nodeInput,
        "GREEN-PROFILE-MIN-THRESHOLD",//minth for green profile packet
        queueIndex,  // parameterInstanceNumber
        TRUE,      // fallbackIfNoInstanceMatch
        &retVal,
        &(GREEN_PROFILE_PHB(mred)->minThreshold));

    if (retVal == FALSE)
    {
        GREEN_PROFILE_PHB(mred)->minThreshold
                = DEFAULT_GREEN_PROFILE_MIN_THRESHOLD;
    }
    else
    {
        ERROR_Assert(GREEN_PROFILE_PHB(mred)->minThreshold > 0.0,
                "GREEN minThreshold should be > 0.0\n");
    }

    IO_ReadIntInstance(
        node,
        nodeId,
        interfaceIndex,
        nodeInput,
        "GREEN-PROFILE-MAX-THRESHOLD",//maxth for green profile packet
        queueIndex,  // parameterInstanceNumber
        TRUE,       // fallbackIfNoInstanceMatch
        &retVal,
        &(GREEN_PROFILE_PHB(mred)->maxThreshold));

    if (retVal == FALSE)
    {
        GREEN_PROFILE_PHB(mred)->maxThreshold
                = DEFAULT_GREEN_PROFILE_MAX_THRESHOLD;
    }
    else
    {
        ERROR_Assert(GREEN_PROFILE_PHB(mred)->maxThreshold > 0.0,
                "GREEN maxThreshold should be > 0.0\n");
    }

    ERROR_Assert(GREEN_PROFILE_PHB(mred)->minThreshold <
        GREEN_PROFILE_PHB(mred)->maxThreshold,
        "GREEN maxThreshold should be greater than minThreshold \n");

    IO_ReadDoubleInstance(
        node,
        nodeId,
        interfaceIndex,
        nodeInput,
        "GREEN-PROFILE-MAX-PROBABILITY",//max probability for green
                                        // profile packet
        queueIndex,  // parameterInstanceNumber
        TRUE,        // fallbackIfNoInstanceMatch
        &retVal,
        &(GREEN_PROFILE_PHB(mred)->maxProbability));

    if (retVal == FALSE)
    {
        GREEN_PROFILE_PHB(mred)->maxProbability
                = DEFAULT_RED_MAX_PROBABILITY;
    }
    else
    {
        ERROR_Assert(GREEN_PROFILE_PHB(mred)->maxProbability >= 0.0 &&
                GREEN_PROFILE_PHB(mred)->maxProbability <=1.0,
                "GREEN maxProbability should be > 0.0 and <= 1.0\n");
    }

    // Allocating memory and initializing variables for the statistics
    // of Green profile packets

    if (isCollectStats)
    {
        InitializeRedEcnStats(&(GREEN_PROFILE_PHB(mred)->stats));
    }

    IO_ReadIntInstance(
        node,
        nodeId,
        interfaceIndex,
        nodeInput,
        "YELLOW-PROFILE-MIN-THRESHOLD",//minth for yellow profile packet
        queueIndex,  // parameterInstanceNumber
        TRUE,                // fallbackIfNoInstanceMatch
        &retVal,
        &(YELLOW_PROFILE_PHB(mred)->minThreshold));

    if (retVal == FALSE)
    {
        YELLOW_PROFILE_PHB(mred)->minThreshold
                = DEFAULT_YELLOW_PROFILE_MIN_THRESHOLD;
    }
    else
    {
        ERROR_Assert(YELLOW_PROFILE_PHB(mred)->minThreshold > 0.0,
                "YELLOW minThreshold should be > 0.0\n");
    }

    IO_ReadIntInstance(
        node,
        nodeId,
        interfaceIndex,
        nodeInput,
        "YELLOW-PROFILE-MAX-THRESHOLD",//maxth for yellow profile packet
        queueIndex,  // parameterInstanceNumber
        TRUE,                // fallbackIfNoInstanceMatch
        &retVal,
        &(YELLOW_PROFILE_PHB(mred)->maxThreshold));

    if (retVal == FALSE)
    {
        YELLOW_PROFILE_PHB(mred)->maxThreshold
                = DEFAULT_YELLOW_PROFILE_MAX_THRESHOLD;
    }
    else
    {
        ERROR_Assert(YELLOW_PROFILE_PHB(mred)->maxThreshold > 0.0,
                "YELLOW maxThreshold should be > 0.0\n");
    }

    ERROR_Assert(YELLOW_PROFILE_PHB(mred)->minThreshold <
        YELLOW_PROFILE_PHB(mred)->maxThreshold,
        "YELLOW maxThreshold should be greater than minThreshold\n");

    IO_ReadDoubleInstance(
        node,
        nodeId,
        interfaceIndex,
        nodeInput,
        "YELLOW-PROFILE-MAX-PROBABILITY", //max prob for yellow
                                          //profile packet
        queueIndex,  // parameterInstanceNumber
        TRUE,        // fallbackIfNoInstanceMatch
        &retVal,
        &(YELLOW_PROFILE_PHB(mred)->maxProbability));

    if (retVal == FALSE)
    {
        YELLOW_PROFILE_PHB(mred)->maxProbability
                = DEFAULT_RED_MAX_PROBABILITY;
    }
    else
    {
        ERROR_Assert(YELLOW_PROFILE_PHB(mred)->maxProbability >= 0.0 &&
                YELLOW_PROFILE_PHB(mred)->maxProbability <=1.0,
                "YELLOW maxProbability should be > 0.0 and <= 1.0\n");
    }

    // Allocating memory and initializing variables for the statistics
    // of Yellow profile packets

    if (isCollectStats)
    {
        InitializeRedEcnStats(&(YELLOW_PROFILE_PHB(mred)->stats));
    }
}


// Reads Three color MRED Threshold parameters from config
// file
//
// \param node  Pointer to Node structure
// \param interfaceIndex  Interface Index
// \param nodeInput  Pointer to NodeInput
// \param isCollectStats  Tag for enabling Queue Statistics
// \param queueIndex  Queue Index
// \param mred  Pointer to
//    RedEcnParameters Structure that keeps all configurable
//    entries for ECN enabled Multilevel RED Queue.
//

void ReadThreeColorMredPhbParameters(
        Node* node,
        int interfaceIndex,
        const NodeInput *nodeInput,
        const BOOL isCollectStats,
        int queueIndex,
        RedEcnParameters* mred)
{
    int nodeId = node->nodeId;
    BOOL retVal = FALSE;

    //Reading threshold parameters for green and yellow profile packets
    ReadTwoColorMredPhbParameters(node,
                                interfaceIndex,
                                nodeInput,
                                isCollectStats,
                                queueIndex,
                                mred);

    // Reading threshold parameters for red profile packets
    IO_ReadIntInstance(
        node,
        nodeId,
        interfaceIndex,
        nodeInput,
        "RED-PROFILE-MIN-THRESHOLD",//minth for Red profile packet
        queueIndex,  // parameterInstanceNumber
        TRUE,        // fallbackIfNoInstanceMatch
        &retVal,
        &(RED_PROFILE_PHB(mred)->minThreshold));

    if (retVal == FALSE)
    {
        RED_PROFILE_PHB(mred)->minThreshold
                = DEFAULT_RED_PROFILE_MIN_THRESHOLD;
    }
    else
    {
        ERROR_Assert(RED_PROFILE_PHB(mred)->minThreshold > 0.0,
                "RED minThreshold should be > 0.0\n");
    }

    IO_ReadIntInstance(
        node,
        nodeId,
        interfaceIndex,
        nodeInput,
        "RED-PROFILE-MAX-THRESHOLD",//maxth for Red profile packet
        queueIndex,   // parameterInstanceNumber
        TRUE,         // fallbackIfNoInstanceMatch
        &retVal,
        &(RED_PROFILE_PHB(mred)->maxThreshold));

    if (retVal == FALSE)
    {
        RED_PROFILE_PHB(mred)->maxThreshold
                = DEFAULT_RED_PROFILE_MAX_THRESHOLD;
    }
    else
    {
        ERROR_Assert(RED_PROFILE_PHB(mred)->maxThreshold > 0.0,
                "RED maxThreshold should be > 0.0\n");
    }

    ERROR_Assert(RED_PROFILE_PHB(mred)->minThreshold <
        RED_PROFILE_PHB(mred)->maxThreshold,
        "RED maxThreshold should be greater than minThreshold\n");

    IO_ReadDoubleInstance(
        node,
        nodeId,
        interfaceIndex,
        nodeInput,
        "RED-PROFILE-MAX-PROBABILITY", //max probability for Red profile packet
        queueIndex,  // parameterInstanceNumber
        TRUE,        // fallbackIfNoInstanceMatch
        &retVal,
        &(RED_PROFILE_PHB(mred)->maxProbability));

    if (retVal == FALSE)
    {
        RED_PROFILE_PHB(mred)->maxProbability
                = DEFAULT_RED_MAX_PROBABILITY;
    }
    else
    {
        ERROR_Assert(RED_PROFILE_PHB(mred)->maxProbability >= 0.0 &&
                RED_PROFILE_PHB(mred)->maxProbability <=1.0,
                "RED maxProbability should be > 0.0 and <= 1.0\n");
    }

    // Allocating memory and initializing variables for the statistics
    // of RED profile packets

    if (isCollectStats)
    {
        InitializeRedEcnStats(&(RED_PROFILE_PHB(mred)->stats));
    }
}


// This function reads Two | Three color, ECN enabled RIO
// configuration parameters from QualNet .config file.
//
// \param node  Pointer to Node structure
// \param interfaceIndex  Interface Index
// \param nodeInput  Pointer to NodeInput
// \param enableQueueStat  Tag for enabling Queue Statistics
// \param queueIndex  Queue Index
// \param rioParams  Pointer of pointer to
//    RioParameters Structure that keeps all configurable
//    entries for Two | Three color, ECN enabled RIO Queue.
//

void ReadRio_EcnConfigurationThParameters(
    Node* node,
    int interfaceIndex,
    const NodeInput *nodeInput,
    BOOL enableQueueStat,
    int queueIndex,
    RioParameters** rioParams)
{
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH] = {0};
    char errorStr[MAX_STRING_LENGTH] = {0};
    RioParameters* rio = NULL;

    int nodeId = node->nodeId;

    rio = (RioParameters*) MEM_malloc(sizeof (RioParameters));
    memset(rio, 0, sizeof (RioParameters));
    rio->redEcnParams = (RedEcnParameters*)
        MEM_malloc(sizeof (RedEcnParameters));

    // Read queue weight and small pkt transmission time
    ReadRed_EcnCommonConfigurationParams(node,
                                    interfaceIndex,
                                    nodeInput,
                                    enableQueueStat,
                                    queueIndex,
                                    rio->redEcnParams);
    // Read Mred Thresholds
    rio->redEcnParams->numPhbParams = MRED_NUM_PHB;
    rio->redEcnParams->phbParams = (PhbParameters*)
        MEM_malloc(sizeof(PhbParameters) * MRED_NUM_PHB);
    memset(rio->redEcnParams->phbParams, 0,
        sizeof(PhbParameters) * MRED_NUM_PHB);


    // Read the marking mode type of Generalized RIO
    IO_ReadStringInstance(
        node,
        nodeId,
        interfaceIndex,
        nodeInput,
        "RIO-COLOR-MODE",
        queueIndex,  // parameterInstanceNumber
        TRUE,        // fallbackIfNoInstanceMatch
        &wasFound,
        buf);

    if (!wasFound || (strcmp(buf, "TWO-COLOR") &&
                        strcmp(buf, "THREE-COLOR")))
    {
       sprintf(errorStr, "Node: %d Queue: %d\t"
            "Specify RIO-COLOR-MODE TWO-COLOR | THREE-COLOR",
            node->nodeId, queueIndex);
        ERROR_ReportError(errorStr);
    }
    else if (!strcmp(buf, "TWO-COLOR"))
    {
        // RIO with two color marking
        rio->rioMarkerType = TWO_COLOR;

        // Reading phb parameters
        ReadTwoColorMredPhbParameters(node,
                                    interfaceIndex,
                                    nodeInput,
                                    enableQueueStat,
                                    queueIndex,
                                    rio->redEcnParams);
    }
    else // Default Three color
    {
        // RIO with three color marking
        rio->rioMarkerType = THREE_COLOR;

        // Reading phb parameters
        ReadThreeColorMredPhbParameters(node,
                                    interfaceIndex,
                                    nodeInput,
                                    enableQueueStat,
                                    queueIndex,
                                    rio->redEcnParams);
    }

    // Read the operation mode type of Generalized RIO
    IO_ReadStringInstance(
        node,
        nodeId,
        interfaceIndex,
        nodeInput,
        "RIO-COUNTING-MODE",
        queueIndex,  // parameterInstanceNumber
        TRUE,        // fallbackIfNoInstanceMatch
        &wasFound,
        buf);

    if (!wasFound || (strcmp(buf, "COUPLED") && strcmp(buf, "DECOUPLED")))
    {
        memset(errorStr, 0, MAX_STRING_LENGTH);
        sprintf(errorStr, "Node: %d Queue: %d\t"
            "Specify RIO-COUNTING-MODE COUPLED | DECOUPLED",
            node->nodeId, queueIndex);
        ERROR_ReportError(errorStr);
    }
    else if (!strcmp(buf, "DECOUPLED"))
    {
        rio->rioCountingType = RIO_DECOUPLED;
    }
    else // Coupled
    {
        rio->rioCountingType = RIO_COUPLED;
    }

    // Attach Info
    *rioParams = rio;
}

// -------------------------------------------------------------------------
// End: Read and configure RIO
// -------------------------------------------------------------------------
