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


/*
*  Name: multicast_msdp.cpp
*  Purpose: To simulate Protocol Multicast Source Discovery Protocol (MSDP)
*
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "api.h"
#include "external_socket.h"
#include "network_ip.h"
#include "multicast_msdp.h"
#include "app_util.h"
#include "transport_tcp.h"
#include "multicast_pim.h"
#include "network_access_list.h"
#include "transport_udp.h"
#include "routing_bgp.h"

#define MSDP_DEBUG              0   // Layer Events
#define MSDP_DEBUG_CONF         0   // Configuration
#define MSDP_DEBUG_CSM          0   // Connection State Machine
#define MSDP_DEBUG_SA           0   // Origination and Forwading SA message
#define MSDP_DEBUG_TIMER        0   // Set or Reset MSDP timers
#define MSDP_DEBUG_PEER_RPF     0   // Peer RPF check
#define MSDP_DEBUG_FILTER       0   // SA filter in/out check
#define MSDP_DEBUG_MESH         0   // Mesh Check
#define MSDP_DEBUG_FINALIZE     0   // Finalize function
#define MSDP_DEBUG_ACL_CHECK    0   // ACL checks

/*
 * NAME:        MsdpCheckIfSGPassesACL.
 * PURPOSE:     Checks whether (S, G) pair passes the ACL list or not.
 * PARAMETERS:  node - pointer to the node which received the message.
 *              sgEntry - (S, G) pair
 *              filterType - type of msdp filter eg. MSDP_SA_FILTER_IN
 *              msdpData - structure stroring msdp Data
 *              connectionData - stores connection data of the peer
 * RETURN:      bool.
 */
static
bool MsdpCheckIfSGPassesACL(Node* node,
                            MsdpSGEntry* sgEntry,
                            MsdpFilterType filterType,
                            MsdpData* msdpData,
                            MsdpConnectionData* connectionData)
{
    char accessListId[MAX_ACCESS_LIST_ID_LENGTH];
    NodeAddress sgSource;
    NodeAddress sgGroup;

    MsdpFilter* filter = NULL;
    AccessList* accessList = NULL;

    switch (filterType)
    {
        case MSDP_SA_FILTER_IN:
        {
            if (connectionData)
            {
                filter = &connectionData->incomingSaMessagesFilterData;
            }
            else
            {
                ERROR_Assert(false, "\n\nMSDP_SA_FILTER_IN: connnectionData"
                                    "cannot be null.");
            }
            break;
        }

        case MSDP_SA_FILTER_OUT:
        {
            if (connectionData)
            {
                filter = &connectionData->outgoingSaMessagesFilterData;
            }
            else
            {
                ERROR_Assert(false, "\n\nMSDP_SA_FILTER_OUT: "
                                    "connnectionData cannot be null.");
            }
            break;
        }
        case MSDP_REDISTRIBUTE:
        {
            if (msdpData)
            {
                filter = &msdpData->saRedistributeData;
            }
            else
            {
                ERROR_Assert(false, "\n\nMSDP_REDISTRIBUTE: "
                                    "msdpData cannot be null.");
            }
            break;
        }
        default:
        {
            ERROR_Assert(false, "\n\nInvalid Filter Type is specified.");
        }
    }

    // If corresponding filter in is not enabled for this peer
    // then pass the (S, G)pair
    if (!filter->isEnabled)
    {
        // true implies don't drop any packet
        if (MSDP_DEBUG_ACL_CHECK)
        {
            printf("ACL check Passed : returning TRUE\n");
            printf("Filter not enabled\n");
        }
        return true;
    }

    // Filter is enabled but corresponding list is empty
    else if ((filter->isEnabled) && (filter->filterList == NULL))
    {
        // Return false if the specified filter type is enabled but
        // there is no entry for given (S, G) pair in the list.
        if (MSDP_DEBUG_ACL_CHECK)
        {
            printf("ACL check failed : returning FALSE\n");
            printf("Filter enabled: list not specified\n");
        }
        return false;
    }

    MsdpFilterList::iterator filterIterator;
    filterIterator = filter->filterList->begin();
    while (filterIterator != filter->filterList->end())
    {
        sprintf(accessListId, "%d", *filterIterator);
        accessList = AccessListSearchACL(
                        node,
                        accessListId);

        if (accessList == NULL)
        {
            char errStr[MAX_STRING_LENGTH];
            sprintf(errStr, "Node %u: Access List does not exist in"
                            " .router-config file for Id %d\n",
                            node->nodeId,
                            *filterIterator);
            ERROR_ReportWarning(errStr);
        }

        while (accessList != NULL)
        {
            // IP access list
            //  if the list is an extended IP access list
            //  Extended IP access list is ranged from 100 to 199
            if ((*filterIterator >= ACCESS_LIST_MIN_EXTENDED) &&
                (*filterIterator <= ACCESS_LIST_MAX_EXTENDED))
            {
                sgSource = (sgEntry->srcAddr) &
                                (accessList->srcMaskAddr);
                sgGroup = (sgEntry->groupAddr) &
                                (accessList->destMaskAddr);
                if ((sgSource == ((accessList->srcAddr) &
                                  (accessList->srcMaskAddr))) &&
                    (sgGroup == ((accessList->destAddr) &
                                  (accessList->destMaskAddr))))
                {
                    if (accessList->filterType ==
                            ACCESS_LIST_PERMIT)
                    {
                        if (MSDP_DEBUG_ACL_CHECK)
                        {
                            printf("ACL check Passed : returning TRUE\n");
                            printf("Group Address:%x Source Address:%x"
                                    " Permitted\n", sgEntry->groupAddr,
                                    sgEntry->srcAddr);
                        }
                        return true;
                    }
                    else if (accessList->filterType ==
                                ACCESS_LIST_DENY)
                    {
                        if (MSDP_DEBUG_ACL_CHECK)
                        {
                            printf("ACL check failed : returning FALSE\n");
                            printf("Group Address:%x Source Address:%x"
                                    " Denied\n", sgEntry->groupAddr,
                                    sgEntry->srcAddr);
                        }
                        return false;
                    }
                }
            }
            // If the list is a standard IP access list
            //  Standard IP access list is ranged from 1 to 99
            else if ((*filterIterator >= ACCESS_LIST_MIN_STANDARD) &&
                        (*filterIterator <= ACCESS_LIST_MAX_STANDARD))
            {
                NodeAddress sgSource =
                    (sgEntry->srcAddr) &
                    (accessList->srcMaskAddr);
                if (sgSource == ((accessList->srcAddr) &
                                (accessList->srcMaskAddr)))
                {
                    if (accessList->filterType ==
                            ACCESS_LIST_PERMIT)
                    {
                        if (MSDP_DEBUG_ACL_CHECK)
                        {
                            printf("ACL check Passed : returning TRUE\n");
                            printf("Source Address:%x Permitted\n",
                                    sgEntry->srcAddr);
                        }
                        return true;
                    }
                    else if (accessList->filterType ==
                                ACCESS_LIST_DENY)
                    {
                        if (MSDP_DEBUG_ACL_CHECK)
                        {
                            printf("ACL check failed : returning FALSE\n");
                            printf("Source Address:%x Denied\n",
                                    sgEntry->srcAddr);
                        }
                        return false;
                    }
                }
            }
            accessList = accessList->next;
        }
        filterIterator++;
    }

    // Return false if
    // The specified filter type is enabled but there is no entry for given
    //  (S, G) pair in the list.
    if (MSDP_DEBUG_ACL_CHECK)
    {
        printf("ACL check failed : returning FALSE\n");
        printf("Not Present in the specified list\n");
    }
    return false;
}

/*
 * NAME:        MsdpConvertStringToNodeAddress.
 * PURPOSE:     Validates whether the given IP address is in valid dotted
 *              format or not, If it is a valid Ip address it converts it
 *              into NodeAddress.
 * PARAMETERS:  nodeAddressString - pointer to string containing the IP addr
 *              peerAddr - stores the output
 * RETURN:      NONE.
 */
void
MsdpConvertStringToNodeAddress(char* nodeAddressString,
                                    NodeAddress* peerAddr)
{
    bool isDottedFormat = false;
    Int32 p = 0;
    Int32 i = 0;
    while (nodeAddressString[i])
    {
        if (nodeAddressString[i] == '.')
        {
            p++;
        }
        i++;
    }

    // a valid IP address contains three '.'s
    if (p == 3 && nodeAddressString[strlen(nodeAddressString) - 1] != '.')
    {
        IO_ConvertStringToNodeAddress(nodeAddressString, peerAddr);
    }
    else
    {
        char errString[MAX_STRING_LENGTH];
        sprintf(errString,
                "\n%s is not a valid Node Address\n",
                nodeAddressString);
        ERROR_ReportError(errString);
    }
}

/*
 * NAME:        MsdpGetIdIfValidAccessList
 * PURPOSE:     Returns Access List Id if it is valid
 *              Access List Id should be an integer
 *              ranging from 1 to 199
 * PAREMETERS:  node, node concerned
 *              accessListId, array storing access list id
 * RETURN:      Int32
 */
Int32
MsdpGetIdIfValidAccessList(Node* node, char* accessListId)
{
    if (IO_IsStringNonNegativeInteger(accessListId))
    {
        // Valid Access list id is ranged from 1 to 199
        if ((atoi(accessListId) < ACCESS_LIST_MIN_STANDARD) ||
            (atoi(accessListId) > ACCESS_LIST_MAX_EXTENDED))
        {
            char errString[MAX_STRING_LENGTH];
            sprintf(errString, "\nnode :%u\nAccess List should be between 1"
                    " and 199.\n", node->nodeId);
            ERROR_ReportError(errString);
        }
    }
    else
    {
        char errString[MAX_STRING_LENGTH];
        sprintf(errString, "\nnode :%u Enter a valid number"
                           "\nAccess List should be between 1"
                           " and 199.\n", node->nodeId);
        ERROR_ReportError(errString);
    }
    return atoi(accessListId);
}

/*
 * NAME:        MsdpSetOrResetTimer
 * PURPOSE:     If the timer is not set it sets else it resets it.
 * PAREMETERS:  node - node concerned
 *              type - timer type
 *              msdpData - MSDP node specific data
 *              connectionData - connection specific data
 *              reschedule - true if the timer need to be rescheduled
 *              rpAddress - required if the type of timer is
 *                         MSDP_CACHE_SA_STATE_TIMER (default 0)
 *              saState - MsdpSAState for which the
 *                        MSDP_CACHE_SA_STATE_TIMER need to be set or reset.
 * RETURN:      true on success else false
 */
bool
MsdpSetOrResetTimer(Node* node,
                    MsdpTimersType type,
                    MsdpData* msdpData,
                    MsdpConnectionData* connectionData,
                    bool reschedule = false,
                    NodeAddress rpAddress = 0,
                    MsdpSAState* saState = NULL)
{
    Message* timer = NULL;
    clocktype delay = 0;
    Int32 appEventType;
    switch (type)
    {
        case MSDP_CONNECTRETRY_TIMER:
        {
            if (MSDP_DEBUG_TIMER)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d peer:%x Set or Reset:"
                    " MSDP_CONNECTRETRY_TIMER\n",
                    node->nodeId, connectionData->peerAddr);
            }
            timer = connectionData->connectRetryTimer;
            delay = msdpData->connectRetryPeriod;
            appEventType = MSG_APP_MSDP_ConnectRetryTimerExpired;
            break;
        }
        case MSDP_HOLD_TIMER:
        {
            if (MSDP_DEBUG_TIMER)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d peer:%x Set or Reset: MSDP_HOLD_TIMER\n",
                    node->nodeId, connectionData->peerAddr);
            }
            timer = connectionData->holdTimer;
            delay = msdpData->holdTimePeriod;
            appEventType = MSG_APP_MSDP_HoldTimerExpired;
            break;
        }
        case MSDP_KEEPALIVE_TIMER:
        {
            if (MSDP_DEBUG_TIMER)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d peer:%x Set or Reset:"
                    " MSDP_KEEPALIVE_TIMER\n",
                    node->nodeId, connectionData->peerAddr);
            }
            timer = connectionData->keepAliveTimer;
            delay = msdpData->keepAlivePeriod;
            appEventType = MSG_APP_MSDP_KeepAliveTimerExpired;
            break;
        }
        case MSDP_SA_ADVERTISEMENT_TIMER:
        {
            if (MSDP_DEBUG_TIMER)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d Set or Reset:"
                    " MSDP_SA_ADVERTISEMENT_TIMER\n",
                    node->nodeId);
            }
            timer = msdpData->saAdvertisementTimer;
            delay = msdpData->saAdvertisementPeriod;
            appEventType = MSG_APP_MSDP_AdvertisementTimerExpired;
            break;
        }
        case MSDP_CACHE_SA_STATE_TIMER:
        {
            if (MSDP_DEBUG_TIMER)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d (S, G):(%x, %x) Set or Reset:"
                    " MSDP_CACHE_SA_STATE_TIMER\n", node->nodeId,
                    saState->sgEntry.srcAddr, saState->sgEntry.groupAddr);
            }
            if (saState != NULL)
            {
                timer = saState->saStateTimer;
                delay = msdpData->sgStatePeriod;
                appEventType = MSG_APP_MSDP_SAStateTimerExpired;
            }
            else
            {
                return false;
            }
            break;
        }
        default:
        {
            return false;
        }
    }
    if (!reschedule)
    {
        if (timer != NULL)
        {
            MESSAGE_CancelSelfMsg(node, timer);
        }
        timer = MESSAGE_Alloc(node,
                              APP_LAYER,
                              APP_MSDP,
                              appEventType);
        if (type == MSDP_CACHE_SA_STATE_TIMER)
        {
            MESSAGE_InfoAlloc(node,
                              timer,
                              sizeof(MsdpCacheSAStateTimerInfo));
            MsdpCacheSAStateTimerInfo info;
            info.rpAddr = rpAddress;
            info.sgEntry = saState->sgEntry;
            memcpy(MESSAGE_ReturnInfo(timer),
                   &(info), sizeof(MsdpCacheSAStateTimerInfo));
        }
        else if (type != MSDP_SA_ADVERTISEMENT_TIMER)
        {
            MESSAGE_InfoAlloc(node,
                              timer,
                              sizeof(NodeAddress));
            memcpy(MESSAGE_ReturnInfo(timer),
                &(connectionData->peerAddr), sizeof(NodeAddress));
        }
        switch (type)
        {
            case MSDP_CONNECTRETRY_TIMER:
            {
                connectionData->connectRetryTimer = timer;
                break;
            }
            case MSDP_HOLD_TIMER:
            {
                connectionData->holdTimer = timer;
                break;
            }
            case MSDP_KEEPALIVE_TIMER:
            {
                connectionData->keepAliveTimer = timer;
                break;
            }
            case MSDP_SA_ADVERTISEMENT_TIMER:
            {
                msdpData->saAdvertisementTimer = timer;
                break;
            }
            case MSDP_CACHE_SA_STATE_TIMER:
            {
                saState->saStateTimer = timer;
                break;
            }
        }
    }
    MESSAGE_Send(node, timer, delay);
    return true;
}

/*
 * NAME:        MsdpProcessInactiveState.
 * PURPOSE:     Processes the events to occur in inactive state and
 *              ignore the event if the state is not inactive.
 * PARAMETERS:  node - pointer to the node which received the event.
 *              msdpData - MSDP node specific data
 *              event - type of MSDP event
 *              connectionData - connection specific data
 * RETURN:      NONE.
 */
static
void MsdpProcessInactiveState(
    Node* node,
    MsdpData* msdpData,
    MsdpEvent event,
    MsdpConnectionData* connectionData)
{
    ERROR_Assert(connectionData, "Invalid MSDP connectionData structure\n");

    if (connectionData->state != MSDP_INACTIVE)
    {
        return;
    }
    switch (event)
    {
        case MSDP_START:
        {
            if (msdpData->saAdvertisementTimer == NULL)
            {
                // Set advertisement timer
                MsdpSetOrResetTimer(node,
                                    MSDP_SA_ADVERTISEMENT_TIMER,
                                    msdpData,
                                    connectionData);
            }

            if (connectionData->interfaceAddr > connectionData->peerAddr)
            {
                APP_TcpServerListen(node,
                                    APP_MSDP,
                                    connectionData->interfaceAddr,
                                    (Int16) APP_MSDP);
                connectionData->state = MSDP_LISTEN;
                if (MSDP_DEBUG_CSM)
                {
                    char time[MAX_STRING_LENGTH];
                    TIME_PrintClockInSecond(node->getNodeTime(), time);

                    printf("Time:%s ", time);

                    printf("Node:%d peer:%x "
                        "Event: MSG_APP_MSDP_StartTimerExpired"
                        " State Changed To: MSDP_LISTEN\n",
                        node->nodeId, connectionData->peerAddr);
                }
            }
            else if (
                connectionData->interfaceAddr <
                connectionData->peerAddr)
            {

                // Send one self event to set maximum time it will wait for tcp
                // to open the connection. Within this time if tcp fails the
                // MSDP speaker will again try to open the transport connection
                Message* newMessage = MESSAGE_Alloc(node,
                                       APP_LAYER,
                                       APP_MSDP,
                                       MSG_APP_MSDP_ConnectRetryTimerExpired);

                MESSAGE_InfoAlloc(node, newMessage, sizeof(NodeAddress));

                memcpy(MESSAGE_ReturnInfo(newMessage),
                       &(connectionData->peerAddr),
                       sizeof(NodeAddress));

                clocktype delay = (clocktype)
                            ((0.85 * msdpData->connectRetryPeriod)
                            + RANDOM_erand (msdpData->timerSeed)
                            * (0.15 * msdpData->connectRetryPeriod));

                connectionData->connectRetryTimer = newMessage;

                MESSAGE_Send(node, newMessage, delay);

                APP_TcpOpenConnection(
                    node,
                    APP_MSDP,
                    connectionData->interfaceAddr,
                    node->appData.nextPortNum++,
                    connectionData->peerAddr,
                    (Int16) APP_MSDP,
                     /* key for connectionDataMap */
                    connectionData->peerAddr,
                    (clocktype) 0);

                connectionData->state = MSDP_CONNECTING;
                if (MSDP_DEBUG_CSM)
                {
                    char time[MAX_STRING_LENGTH];
                    TIME_PrintClockInSecond(node->getNodeTime(), time);

                    printf("Time:%s ", time);

                    printf("Node:%d peer:%x "
                        "Event: MSG_APP_MSDP_StartTimerExpired"
                        " State Changed To: MSDP_CONNECTING\n",
                        node->nodeId, connectionData->peerAddr);
                }
            }
            else
            {
                char errStr[MAX_STRING_LENGTH];
                sprintf(errStr, "Peer address for node:%d is invalid.\n",
                    node->nodeId);
                ERROR_ReportError(errStr);
            }
            break;
        }
        default:
        {
            // Ignore rest of the events
        }
    }
}

/*
 * NAME:        MsdpSendKeepAliveTLV.
 * PURPOSE:     Construct a MSDP Keep-Alive TLV and
 *              sends it on the connection ID stored in connectionData.
 * PARAMETERS:  node - pointer to the node
 *              msdpData - MSDP node specific data.
 *              connectionData - connection specific data.
 * RETURN:      NONE.
 */
BEGIN_IGNORE_DEPRECATIONS
void MsdpSendKeepAliveTLV(
    Node* node,
    MsdpData* msdpData,
    MsdpConnectionData* connectionData)
{
    char* payload = NULL;
    char* temp = NULL;

    ERROR_Assert(connectionData, "Invalid MSDP connectionData structure\n");

    // Allocating space for the keepalive TLV
    payload = (char*) MEM_malloc(MSDP_KEEP_ALIVE_TLV_LENGTH);
    memset(payload, 0, MSDP_KEEP_ALIVE_TLV_LENGTH);
    temp = payload;

    // MSDP message type
    *temp = (char) MSDP_KEEPALIVE;
    temp += MSDP_MESSAGE_TYPE_SIZE;

    // MSDP message length
    *temp = (UInt16) MSDP_KEEP_ALIVE_TLV_LENGTH;
    EXTERNAL_ntoh(temp, sizeof(UInt16));

    Message *msg = App_TcpCreateMessage(node,
                              connectionData->connectionId,
                              payload,
                              MSDP_KEEP_ALIVE_TLV_LENGTH,
                              TRACE_MSDP,
                              IPDEFTTL);
    MESSAGE_Send(node, msg, 0);

    msdpData->stats.keepAliveSent++;

    MEM_free(payload);
}
END_IGNORE_DEPRECATIONS

/*
 * NAME:        MsdpSACacheGetSAStateMapByRPAddress
 * PURPOSE:     Searches for MsdpSAStateMap by RPAddress.
 * PAREMETERS:  node - node concerned
 *              rpAddress - RP address
 *              msdpData - MSDP node specific data
 * RETURN:      MsdpSAStateMap if found else NULL
 */
MsdpSAStateMap*
MsdpSACacheGetSAStateMapByRPAddress(Node* node,
                  NodeAddress rpAddress,
                  MsdpData* msdpData)
{
    MsdpSAStateMap* saStateMap = NULL;

    if (msdpData->saCache != NULL)
    {
        MsdpSACacheMap* saCache = msdpData->saCache;
        MsdpSACacheMap::iterator saCacheMapIt = saCache->find(rpAddress);
        MsdpCacheSAItem* cacheSAItem;
        if (saCacheMapIt != saCache->end())
        {
            cacheSAItem = saCacheMapIt->second;
            saStateMap = cacheSAItem->saStateMap;
        }
    }
    return saStateMap;
}

/*
 * NAME:        MsdpSACacheGetSGEntryListByRPAddress
 * PURPOSE:     Searches for SG entries for the RPAddress.
 * PAREMETERS:  node - node concerned
 *              rpAddress - RP address
 *              msdpData - MSDP node specific data
 *              sgEntryList - pointer to MsdpSGEntryList to insert
 *                            found SG entries
 * RETURN:      None
 */
void
MsdpSACacheGetSGEntryListByRPAddress(Node* node,
                  NodeAddress rpAddress,
                  MsdpData* msdpData,
                  MsdpSGEntryList* sgEntryList)
{
    sgEntryList->clear();
    MsdpSAStateMap* saStateMap =
        MsdpSACacheGetSAStateMapByRPAddress(node,
                                            rpAddress,
                                            msdpData);
    if (saStateMap != NULL)
    {
        MsdpSAStateMap::iterator saStateMapIt = saStateMap->begin();
        for (;saStateMapIt != saStateMap->end(); saStateMapIt++)
        {
            sgEntryList->push_back(saStateMapIt->second->sgEntry);
        }
    }
}

/*
 * NAME:        MsdpSACacheInsert.
 * PURPOSE:     Inserts the sgEntry in the cache to be advertised preodically.
 * PARAMETERS:  node - pointer to the node
 *              rpAddress - originator RP Address.
 *              sgEntry - Source, Group pair
 *              msdpData - MSDP node specific data.
 *              setTimer - Whether SG State Expiry Timer needs to be set
 *                         (Default true).
 * RETURN:      Returns MsdpError:
 *              MSDP_NO_ERROR if there is no error in inserting the sgEntry
 *              MSDP_CACHE_STATE_ALREADY_EXIST - sgEntry already exists
 *              MSDP_CACHE_INSERT_CACHE_DISABLED - cache is disabled
 *              (in current implementation this error should not be returned,
 *              since, cache is enabled by default).
 */
MsdpError
MsdpSACacheInsert(Node* node,
                  NodeAddress rpAddress,
                  MsdpSGEntry sgEntry,
                  MsdpData* msdpData,
                  bool setTimer = true)
{
    if (msdpData->saCache == NULL)
    {
        return MSDP_CACHE_INSERT_CACHE_DISABLED;
    }

    MsdpSACacheMap* saCache = msdpData->saCache;
    MsdpSACacheMap::iterator saCacheMapIt;
    MsdpCacheSAItem* cacheSAItem = NULL;
    MsdpSGEntryPair sgEntryPair(sgEntry.srcAddr, sgEntry.groupAddr);

    if (saCache->size() > 0)
    {
        saCacheMapIt = saCache->find(rpAddress);
    }

    if (saCache->size() == 0
        || saCacheMapIt == saCache->end())
    {
        cacheSAItem = (MsdpCacheSAItem*)MEM_malloc(sizeof(MsdpCacheSAItem));
        cacheSAItem->rpAddr = rpAddress;
        cacheSAItem->saStateMap = new MsdpSAStateMap;
        saCache->insert(pair<NodeAddress, MsdpCacheSAItem*>
                            (rpAddress, cacheSAItem));
    }
    else
    {
        cacheSAItem = saCacheMapIt->second;
        if (cacheSAItem->saStateMap->find(sgEntryPair) !=
            cacheSAItem->saStateMap->end())
        {
            return MSDP_CACHE_STATE_ALREADY_EXIST;
        }
    }

    MsdpSAState* saState = (MsdpSAState*) MEM_malloc(sizeof(MsdpSAState));
    saState->sgEntry = sgEntry;

    if (setTimer == true)
    {
        Message* saStateTimer = MESSAGE_Alloc(node,
                                           APP_LAYER,
                                           APP_MSDP,
                                           MSG_APP_MSDP_SAStateTimerExpired);
        saState->saStateTimer = saStateTimer;
        MESSAGE_InfoAlloc(node,
                          saStateTimer,
                          sizeof(MsdpCacheSAStateTimerInfo));
        MsdpCacheSAStateTimerInfo info;
        info.rpAddr = rpAddress;
        info.sgEntry = sgEntry;
        memcpy(MESSAGE_ReturnInfo(saStateTimer),
               &(info), sizeof(MsdpCacheSAStateTimerInfo));

        MESSAGE_Send(node, saStateTimer, msdpData->sgStatePeriod);
    }
    else
    {
        saState->saStateTimer = NULL;
    }
    cacheSAItem->saStateMap->insert(
        pair<MsdpSGEntryPair, MsdpSAState*>(sgEntryPair, saState));

    return MSDP_NO_ERROR;
}

/*
 * NAME:        MsdpSACacheExpireSAState.
 * PURPOSE:     Deletes the sgEntry from the MSDP cache.
 * PARAMETERS:  node - pointer to the node.
 *              rpAddress - originator RP Address.
 *              sgEntry - Source, Group pair
 *              msdpData - MSDP node specific data.
 * RETURN:      Returns MsdpError:
 *              MSDP_NO_ERROR if there is no error in deleting the sgEntry
 *              MSDP_CACHE_STATE_DONT_EXIST - sgEntry does not exists.
 *              MSDP_CACHE_INSERT_CACHE_DISABLED - cache is disabled.
 *              (in current implementation this error should not be returned,
 *              since, cache is enabled by default).
 */
MsdpError
MsdpSACacheExpireSAState(Node* node,
                         NodeAddress rpAddress,
                         MsdpSGEntry sgEntry,
                         MsdpData* msdpData)
{
    if (msdpData->saCache == NULL)
    {
        return MSDP_CACHE_INSERT_CACHE_DISABLED;
    }

    MsdpSACacheMap* saCache = msdpData->saCache;
    MsdpSACacheMap::iterator saCacheMapIt;
    MsdpCacheSAItem* cacheSAItem;
    MsdpSGEntryPair sgEntryPair(sgEntry.srcAddr, sgEntry.groupAddr);

    if (saCache->size() > 0)
    {
        saCacheMapIt = saCache->find(rpAddress);
    }
    else
    {
        return MSDP_CACHE_STATE_DONT_EXIST;
    }

    if (saCacheMapIt != saCache->end())
    {
        MsdpSAStateMap* saStateMap;
        cacheSAItem = saCacheMapIt->second;
        saStateMap = cacheSAItem->saStateMap;
        MsdpSAStateMap::iterator saStateMapIt;
        if (saStateMap->size() > 0)
        {
            saStateMapIt = saStateMap->find(sgEntryPair);
        }
        else
        {
            return MSDP_CACHE_STATE_DONT_EXIST;
        }
        if (saStateMapIt != saStateMap->end())
        {
            MsdpSAState* saState = saStateMapIt->second;
            if (saState->saStateTimer != NULL)
            {
                MESSAGE_CancelSelfMsg(node, saState->saStateTimer);
            }
            MEM_free(saState);
            saStateMap->erase(saStateMapIt);
        }
        else
        {
            return MSDP_CACHE_STATE_DONT_EXIST;
        }
        if (saStateMap->size() == 0)
        {
            delete(saStateMap);
            MEM_free(cacheSAItem);
            saCache->erase(saCacheMapIt);
        }
    }
    else
    {
        return MSDP_CACHE_STATE_DONT_EXIST;
    }

    return MSDP_NO_ERROR;
}

/*
 * NAME:        MsdpSACacheResetSAState.
 * PURPOSE:     Cancels the SA State Timer and resets it for the SG Entry.
 * PARAMETERS:  node
 *              rpAddress - originator RP Address.
 *              sgEntry - Source, Group pair
 *              msdpData - MSDP node specific data.
 * RETURN:      Returns MsdpError:
 *              MSDP_NO_ERROR if there is no error in deleting the sgEntry
 *              MSDP_CACHE_STATE_DONT_EXIST - sgEntry does not exists.
 *              MSDP_CACHE_INSERT_CACHE_DISABLED - cache is disabled.
 *              (in current implementation this error should not be returned,
 *              since, cache is enabled by default).
 */
MsdpError
MsdpSACacheResetSAState(Node* node,
                         NodeAddress rpAddress,
                         MsdpSGEntry sgEntry,
                         MsdpData* msdpData)
{
    if (msdpData->saCache == NULL)
    {
        return MSDP_CACHE_INSERT_CACHE_DISABLED;
    }

    MsdpSACacheMap* saCache = msdpData->saCache;
    MsdpSACacheMap::iterator saCacheMapIt;
    MsdpCacheSAItem* cacheSAItem;
    MsdpSGEntryPair sgEntryPair(sgEntry.srcAddr, sgEntry.groupAddr);

    if (saCache->size() > 0)
    {
        saCacheMapIt = saCache->find(rpAddress);
    }
    else
    {
        return MSDP_CACHE_STATE_DONT_EXIST;
    }

    if (saCacheMapIt != saCache->end())
    {
        MsdpSAStateMap* saStateMap;
        cacheSAItem = saCacheMapIt->second;
        saStateMap = cacheSAItem->saStateMap;
        MsdpSAStateMap::iterator saStateMapIt;
        if (saStateMap->size() > 0)
        {
            saStateMapIt = saStateMap->find(sgEntryPair);
        }
        else
        {
            return MSDP_CACHE_STATE_DONT_EXIST;
        }
        if (saStateMapIt != saStateMap->end())
        {
            MsdpSAState* saState = saStateMapIt->second;
            MsdpSetOrResetTimer(node,
                                MSDP_CACHE_SA_STATE_TIMER,
                                msdpData,
                                NULL,
                                false,
                                rpAddress,
                                saState);
        }
        else
        {
            return MSDP_CACHE_STATE_DONT_EXIST;
        }
    }
    else
    {
        return MSDP_CACHE_STATE_DONT_EXIST;
    }

    return MSDP_NO_ERROR;
}

/*
 * NAME:        MsdpSendSAMsgWithMaxSGEntryList.
 * PURPOSE:     Construct a MSDP Source Active Message and
 *              sends it on the connection ID stored in connectionData.
 * PARAMETERS:  node
 *              mdata - data to be encapsulated
 *                     (NULL if no data is encapsulated)
 *              rpAddress - originator RP Address.
 *              sgGEntryList - list of the sgEntries
 *              connectionData - connection specific data
 *              msgType - SA
 *              msg - message to be encapsulated (default NULL)
 * RETURN:      NULL.
 */
void
MsdpSendSAMsgWithMaxSGEntryList(Node* node,
                                MsdpData* mdata,
                                NodeAddress rpAddress,
                                MsdpSGEntryList* sgEntryList,
                                MsdpConnectionData* connectionData,
                                MsdpMessageType msgType,
                                Message* msg = NULL)
{
    char* payload = NULL;
    char* temp = NULL;

    bool messageOverFlow = false;

    ERROR_Assert(connectionData, "Invalid MSDP connectionData structure\n");

    // Calculating SA message length
    UInt32 z = (UInt32)sgEntryList->size(); // Entry Count

    // check for no entry in sgEntryList
    if (z == 0)
    {
        return;
    }
    UInt32 x = MSDP_SOURCE_ACTIVE_HEADER_LENGTH
                + z * MSDP_SOURCE_ACTIVE_SG_ENTRY_LENGTH;
    UInt32 y = 0;
    char* data = NULL;
    if (msg != NULL)
    {
        y = MESSAGE_ReturnPacketSize(msg);
        data = MESSAGE_ReturnPacket(msg);
    }
    else
    {
        y = 0;
        data = NULL;
    }
    UInt32 length = x + y; // packet size

    if (length > MSDP_SA_MESSAGE_MAX_SIZE)
    {
        if (x > MSDP_SA_MESSAGE_MAX_SIZE)
        {
            length = MSDP_SOURCE_ACTIVE_HEADER_LENGTH
                + MSDP_SA_MESSAGE_MAX_Z * MSDP_SOURCE_ACTIVE_SG_ENTRY_LENGTH;
        }
        else
        {
            length = x;
            y = 0;
        }
    }

    // recalculating z
    z = ((length - y) - MSDP_SOURCE_ACTIVE_HEADER_LENGTH)/
        MSDP_SOURCE_ACTIVE_SG_ENTRY_LENGTH;
    // Allocating space for the SA TLV
    payload = (char*) MEM_malloc(length);
    memset(payload, 0, length);
    temp = payload;

    // MSDP message type
    *temp = (char) msgType;

    temp += MSDP_MESSAGE_TYPE_SIZE;

    // MSDP SA message length
    memcpy(temp, &length, MSDP_MESSAGE_LENGTH_SIZE);
    EXTERNAL_ntoh(temp, MSDP_MESSAGE_LENGTH_SIZE);
    temp += MSDP_MESSAGE_LENGTH_SIZE;

    // MSDP SA message Entry Count
    *temp = (char) z;
    temp += MSDP_MESSAGE_ENTRY_COUNT_SIZE;

    MsdpSGEntryList::iterator sgEntryListIt;

    // MSDP SA message RP Address
    memcpy(temp, &(rpAddress), MSDP_MESSAGE_RP_ADDRESS_SIZE);
    EXTERNAL_ntoh(temp, MSDP_MESSAGE_RP_ADDRESS_SIZE);
    temp += MSDP_MESSAGE_RP_ADDRESS_SIZE;

    // MSDP SA message SG Entry with Reserved and Sprefix Length
    for (sgEntryListIt = sgEntryList->begin();
        sgEntryListIt != sgEntryList->end();)
    {
        MsdpSGEntry sgEntry = *sgEntryListIt;
        if ((temp - payload) / sizeof(char)
            + MSDP_SOURCE_ACTIVE_SG_ENTRY_LENGTH
            > MSDP_SA_MESSAGE_MAX_SIZE)
        {
            messageOverFlow = true;
            break;
        }
        // MSDP SA message Reserved
        temp += MSDP_MESSAGE_RESERVED_SIZE;

        // MSDP SA message Sprefix Length
        *temp = (char) 32;
        temp += MSDP_MESSAGE_SPREFIX_LENGTH_SIZE;

        // MSDP SA message Group Address
        memcpy(temp, &(sgEntry.groupAddr), sizeof(NodeAddress));
        EXTERNAL_ntoh(temp, sizeof(NodeAddress));
        temp += sizeof(NodeAddress);

        // MSDP SA message Source Address
        memcpy(temp, &(sgEntry.srcAddr), sizeof(NodeAddress));
        EXTERNAL_ntoh(temp, sizeof(NodeAddress));
        temp += sizeof(NodeAddress);
        sgEntryList->erase(sgEntryListIt);
    }

    if ((y > 0
        && ((temp - payload) / sizeof(char) + y > MSDP_SA_MESSAGE_MAX_SIZE))
        || !sgEntryList->empty())
    {
        y = 0;
    }

    ERROR_Assert((temp - payload)/sizeof(char) <= MSDP_SA_MESSAGE_MAX_SIZE,
        "SA message maximum packet size exceeded\n");

    ERROR_Assert(connectionData->state == MSDP_ESTABLISHED,
        "Trying to send SA message on an unstablished connection\n");

    if (MSDP_DEBUG_SA)
    {
        char time[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(node->getNodeTime(), time);

        printf("Time:%s ", time);

        printf("Node:%d peer:%x Event:Sending SA message\n",
            node->nodeId, connectionData->peerAddr);
    }

    // Sending the Source-Active packet to the msdp peer
    if (y == 0)
    {
        msg = MESSAGE_Alloc(
                  node,
                  TRANSPORT_LAYER,
                  TransportProtocol_TCP,
                  MSG_TRANSPORT_FromAppSend);
        MESSAGE_PacketAlloc(node, msg, length, TRACE_MSDP);
        memcpy(MESSAGE_ReturnPacket(msg), payload, length);
    }
    else
    {
        // MSDP SA message Encapulated data
        MESSAGE_AddHeader(node,
                          msg,
                          x,
                          TRACE_MSDP);
        memcpy(MESSAGE_ReturnPacket(msg), payload, x);
        ERROR_Assert(MESSAGE_ReturnPacketSize(msg) <= MSDP_SA_MESSAGE_MAX_SIZE,
            "SA message maximum packet size exceeded\n");
        // Set layer, protocol, event type
        MESSAGE_SetLayer(msg, TRANSPORT_LAYER, TransportProtocol_TCP);
        MESSAGE_SetEvent(msg, MSG_TRANSPORT_FromAppSend);
        mdata->stats.encapsulatedSAForwarded++;
    }

    // Sending Encapsulated SA message
    AppToTcpSend *sendRequest;
    ActionData acnData;

    MESSAGE_InfoAlloc(node, msg, sizeof(AppToTcpSend));
    sendRequest = (AppToTcpSend *) MESSAGE_ReturnInfo(msg);
    sendRequest->connectionId = connectionData->connectionId;
    sendRequest->ttl = IPDEFTTL;

    //Trace Information
    acnData.actionType = SEND;
    acnData.actionComment = NO_COMMENT;
    TRACE_PrintTrace(node, msg, TRACE_APPLICATION_LAYER,
        PACKET_OUT, &acnData);

    MESSAGE_Send(node, msg, 0);

    MEM_free(payload);
}

/*
 * NAME:        MsdpSendSAMessage.
 * PURPOSE:     Construct a MSDP Source Active Message if
 *              (S, G) entry list is not empty.
 * PARAMETERS:  node
 *              mdata - data to be encapsulated
 *                     (NULL if no data is encapsulated)
 *              rpAddress - originator RP Address.
 *              sgGEntryList - list of the sgEntries
 *              connectionData - connection specific data
 *              msgType - SA
 *              msg - message to be encapsulated (default NULL)
 * RETURN:      None.
 */
void
MsdpSendSAMessage(Node* node,
                 MsdpData* mdata,
                 NodeAddress rpAddress,
                 MsdpSGEntryList sgEntryList,
                 MsdpConnectionData* connectionData,
                 MsdpMessageType msgType,
                 Message* msg = NULL)
{
    while (sgEntryList.size() != 0)
    {
        MsdpSendSAMsgWithMaxSGEntryList(node,
                                        mdata,
                                        rpAddress,
                                        &sgEntryList,
                                        connectionData,
                                        msgType,
                                        msg);
    }
}


/*
 * NAME:        MsdpProcessConnectingAndListenState.
 * PURPOSE:     Processes the events to occur in Connecting and Listen State
 *              and ignore the event if the state is neither of the two.
 * PARAMETERS:  node - pointer to the node which received the event.
 *              msg - message received
 *              msdpData - MSDP node specific data
 *              event - type of MSDP event
 *              connectionData - connection specific data
 * RETURN:      None.
 */
static
void MsdpProcessConnectingAndListenState(
    Node* node,
    Message* msg,
    MsdpData* msdpData,
    MsdpEvent event,
    MsdpConnectionData* connectionData)
{
    if (connectionData != NULL
        && connectionData->state != MSDP_CONNECTING
        && connectionData->state != MSDP_LISTEN)
    {
        return;
    }
    switch (event)
    {
        case MSDP_TRANSPORT_ACTIVE_CONNECTION_OPEN:
        {
            if (connectionData->connectRetryTimer)
            {
                MESSAGE_CancelSelfMsg(node,
                    connectionData->connectRetryTimer);
                // MESSAGE_Free(node, connectionData->connectRetryTimer);

                connectionData->connectRetryTimer = NULL;
            }
        }

        case MSDP_TRANSPORT_PASSIVE_CONNECTION_OPEN:
        {
            TransportToAppOpenResult* openResult =
                (TransportToAppOpenResult*) MESSAGE_ReturnInfo(msg);
            connectionData->connectionId = openResult->connectionId;
            connectionData->localPort = openResult->localPort;
            connectionData->state = MSDP_ESTABLISHED;

            MsdpSendKeepAliveTLV(node, msdpData, connectionData);
            // Set keepalive timer
            MsdpSetOrResetTimer(node,
                                MSDP_KEEPALIVE_TIMER,
                                msdpData,
                                connectionData);
            // Set hold timer
            MsdpSetOrResetTimer(node,
                                MSDP_HOLD_TIMER,
                                msdpData,
                                connectionData);
            break;
        }
        case MSDP_TRANSPORT_ACTIVE_CONNECTION_CLOSED:
        {
            break;
        }
        case MSDP_TRANSPORT_PASSIVE_CONNECTION_CLOSED:
        {
            break;
        }
        default:
        {
            // Ignore rest of the events
        }
    }
}


/*
 * NAME:        MsdpHandleSAStateTimerExpiry.
 * PURPOSE:     Processes the SAStateTimerExpiry event.
 * PARAMETERS:  node - pointer to the node which received the event.
 *              msg - timer message
 *              msdpData - MSDP node specific data
 * RETURN:      None.
 */
void
MsdpHandleSAStateTimerExpiry(Node* node, Message* msg, MsdpData* msdpData)
{
    MsdpCacheSAStateTimerInfo* saStateTimerInfo =
        (MsdpCacheSAStateTimerInfo*)MESSAGE_ReturnInfo(msg);
    MsdpSACacheExpireSAState(
                     node,
                     saStateTimerInfo->rpAddr,
                     saStateTimerInfo->sgEntry,
                     msdpData);
}

/*
 * NAME:        MsdpReconstructStructureSA.
 * PURPOSE:     Reconstructs the received SA packet
 * PARAMETERS:  packet - stores the packet received
 *              MsdpReceivedSAData - MSDP received SA data
 * RETURN:      NONE.
 */
void
MsdpReconstructStructureSA(char* packet, MsdpReceivedSAData* receivedSAData)
{
    UInt16 packetSize = 0;
    NodeAddress rpAddress;

    receivedSAData->data = NULL;
    memcpy(&packetSize, packet, sizeof(UInt16));
    packet += MSDP_MESSAGE_LENGTH_SIZE;

    // Entry Count
    char z = (char)packet[0];
    UInt32 x = MSDP_SOURCE_ACTIVE_HEADER_LENGTH
                + (UInt32)z * MSDP_SOURCE_ACTIVE_SG_ENTRY_LENGTH;
    // Data Length
    UInt32 y = packetSize - x;

    packet += MSDP_MESSAGE_ENTRY_COUNT_SIZE;
    EXTERNAL_ntoh((void*)packet, sizeof(NodeAddress));
    memcpy(&rpAddress, packet, sizeof(NodeAddress));
    receivedSAData->rpAddr = rpAddress;

    packet += MSDP_MESSAGE_RP_ADDRESS_SIZE;
    MsdpSGEntryList sgEntryList;

    MsdpSGEntry sgEntry;
    while (z > 0)
    {
        NodeAddress sourceAddr;
        NodeAddress groupAddr;

        packet += MSDP_MESSAGE_RESERVED_SIZE + MSDP_MESSAGE_SPREFIX_LENGTH_SIZE;
        EXTERNAL_ntoh((void*)packet, sizeof(NodeAddress));
        memcpy(&groupAddr, packet, sizeof(NodeAddress));

        packet += MSDP_MESSAGE_SG_ADDRESS_SIZE / 2;
        EXTERNAL_ntoh((void*)packet, sizeof(NodeAddress));
        memcpy(&sourceAddr, packet, sizeof(NodeAddress));

        sgEntry.srcAddr = sourceAddr;
        sgEntry.groupAddr = groupAddr;

        packet += MSDP_MESSAGE_SG_ADDRESS_SIZE / 2;
        sgEntryList.push_back(sgEntry);
        z--;
    }

    receivedSAData->dataLength = y;
    receivedSAData->sgEntryList = sgEntryList;

    if (y != 0)
    {
        receivedSAData->data = (char*) MEM_malloc(y);
        memcpy(receivedSAData->data, packet, y);
    }
}

/*
 * NAME:        MsdpPerformSAFilterInCheckAndCacheSA.
 * PURPOSE:     Perform SAFilter In check on sgEntryList in saData
 *              and Caches it.
 * PARAMETERS:  node - pointer to the node which received the event.
 *              msdpData - MSDP node specific data
 *              MsdpConnectionData* - pointer to the connection database.
 *              saData - parsed SA message.
 *              msg - message pointer
 *              incomingInterface - found incoming interface
 * RETURN:      None.
 */
void
MsdpPerformSAFilterInCheckAndCacheSA(
                Node* node,
                MsdpData* msdpData,
                MsdpConnectionData* connectionData,
                MsdpReceivedSAData* saData,
                Message* msg,
                Int32 incomingInterface)
{
    MsdpSGEntryList::iterator sgEntryListIt = saData->sgEntryList.begin();
    bool lastEntryDeleted = false;
    while (sgEntryListIt != saData->sgEntryList.end())
    {
        MsdpSGEntry sgEntry = *sgEntryListIt;
        bool saFilterIn = MsdpCheckIfSGPassesACL(node,
                                                 &sgEntry,
                                                 MSDP_SA_FILTER_IN,
                                                 msdpData,
                                                 connectionData);
        if (saFilterIn == false)
        {
            if (MSDP_DEBUG_FILTER)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d S,G:%x,%x Event:saFilterIn == FALSE\n",
                    node->nodeId, sgEntry.srcAddr, sgEntry.groupAddr);
            }
            saData->sgEntryList.erase(sgEntryListIt);
            // lastEntryDeleted will remain true if this was the last entry.
            lastEntryDeleted = true;
        }
        else
        {
            if (MSDP_DEBUG_FILTER)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d S,G:%x,%x Event:saFilterIn == TRUE\n",
                    node->nodeId, sgEntry.srcAddr, sgEntry.groupAddr);
            }
            MsdpError error = MsdpSACacheInsert(node,
                                                saData->rpAddr,
                                                sgEntry,
                                                msdpData);
            if (error == MSDP_CACHE_STATE_ALREADY_EXIST)
            {
                MsdpSACacheResetSAState(node,
                                        saData->rpAddr,
                                        sgEntry,
                                        msdpData);
            }
            // Sending source-active information to PIM-SM
            // and the decapsulated packet to be forward.
            Message* pimSmSAInfoMsg =
                MESSAGE_Alloc(node,
                            NETWORK_LAYER,
                            MULTICAST_PROTOCOL_PIM,
                            MSG_ROUTING_PimSmNewSourceDiscoveredByMsdp);

            if (saData->dataLength > 0)
            {
                MESSAGE_CopyInfo(node, pimSmSAInfoMsg, msg);
            }

            MESSAGE_InfoAlloc(node,
                            pimSmSAInfoMsg,
                            sizeof(MsdpSGEntry) + sizeof(Int32));

            memcpy(MESSAGE_ReturnInfo(pimSmSAInfoMsg),
                   &sgEntry,
                   sizeof(MsdpSGEntry));
            memcpy(MESSAGE_ReturnInfo(pimSmSAInfoMsg) + sizeof(MsdpSGEntry),
                   &incomingInterface,
                   sizeof(Int32));
            if (saData->dataLength > sizeof(IpHeaderType)
                                     + sizeof(TransportUdpHeader))
            {
                MESSAGE_PacketAlloc(node,
                                    pimSmSAInfoMsg,
                                    saData->dataLength
                                        - sizeof(IpHeaderType)
                                        - sizeof(TransportUdpHeader),
                                    TRACE_PIM);
                memcpy(MESSAGE_ReturnPacket(pimSmSAInfoMsg),
                       saData->data
                        + sizeof(IpHeaderType)
                        + sizeof(TransportUdpHeader),
                       saData->dataLength
                        - sizeof(IpHeaderType)
                        - sizeof(TransportUdpHeader));

                // Recreating IP and UDP TRACE
                MESSAGE_AddHeader(node,
                                  pimSmSAInfoMsg,
                                  sizeof(TransportUdpHeader),
                                  TRACE_UDP);
                memcpy(MESSAGE_ReturnPacket(pimSmSAInfoMsg),
                       saData->data + sizeof(IpHeaderType),
                       sizeof(TransportUdpHeader));
                MESSAGE_AddHeader(node,
                                  pimSmSAInfoMsg,
                                  sizeof(IpHeaderType),
                                  TRACE_IP);
                memcpy(MESSAGE_ReturnPacket(pimSmSAInfoMsg),
                       saData->data,
                       sizeof(IpHeaderType));
            }
            MESSAGE_Send(node, pimSmSAInfoMsg, 0);
            sgEntryListIt++;
            // lastEntryDeleted will remain false if this was the last entry.
            lastEntryDeleted = false;
        }
    }
    if (lastEntryDeleted == true && saData->data != NULL)
    {
        MEM_free(saData->data);
        saData->data = NULL;
        saData->dataLength = 0;
    }
}

/*
 * NAME:        MsdpPerformSAFilterOutCheck.
 * PURPOSE:     Perform SAFilter Out check on sgEntryList in saData.
 * PARAMETERS:  node - pointer to the node which received the event.
 *              msdpData - MSDP node specific data
 *              connectionData - pointer to the connection database.
 *              saData - parsed SA message.
 * RETURN:      None.
 */
void
MsdpPerformSAFilterOutCheck(
                Node* node,
                MsdpData* msdpData,
                MsdpConnectionData* connectionData,
                MsdpReceivedSAData* saData)
{
    MsdpSGEntryList::iterator sgEntryListIt = saData->sgEntryList.begin();
    bool lastEntryDeleted = false;
    while (sgEntryListIt < saData->sgEntryList.end())
    {
        MsdpSGEntry sgEntry = *sgEntryListIt;
        bool saFilterOut = MsdpCheckIfSGPassesACL(node,
                                                  &sgEntry,
                                                  MSDP_SA_FILTER_OUT,
                                                  msdpData,
                                                  connectionData);
        if (saFilterOut == false)
        {
            if (MSDP_DEBUG_FILTER)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d S,G:%x,%x Event:saFilterOut == FALSE\n",
                    node->nodeId, sgEntry.srcAddr, sgEntry.groupAddr);
            }
            saData->sgEntryList.erase(sgEntryListIt);
            // lastEntryDeleted will remain true if this was the last entry.
            lastEntryDeleted = true;
        }
        else
        {
            if (MSDP_DEBUG_FILTER)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d S,G:%x,%x Event:saFilterOut == TRUE\n",
                    node->nodeId, sgEntry.srcAddr, sgEntry.groupAddr);
            }
            sgEntryListIt++;
            // lastEntryDeleted will remain false if this was the last entry.
            lastEntryDeleted = false;
        }
    }
    if (lastEntryDeleted == true && saData->data != NULL)
    {
        MEM_free(saData->data);
        saData->data = NULL;
        saData->dataLength = 0;
    }
}

/*
 * NAME:        MsdpMeshFindMeshIdByConnectionData.
 * PURPOSE:     Searches for mesh-id for a connectionData.
 * PARAMETERS:  meshDataMap - MSDP mesh specific data
 *              connectionData* - pointer to the connection database.
 * RETURN:      mesh-id.
 */
Int32
MsdpMeshFindMeshIdByConnectionData(MsdpMeshDataMap* meshDataMap,
                                  MsdpConnectionData* connectionData)
{
    MsdpMeshDataMap::iterator meshDataMapIt = meshDataMap->begin();
    for (;meshDataMapIt != meshDataMap->end(); meshDataMapIt++)
    {
        MsdpMeshMemberList::iterator memberListIt =
                        meshDataMapIt->second->begin();
        for (;memberListIt != meshDataMapIt->second->end(); memberListIt++)
        {
            if (*memberListIt == connectionData)
            {
                return meshDataMapIt->first;
            }
        }
    }
    return MSDP_NO_MESHGROUP;
}

/*
 * NAME:        MsdpMeshCheck.
 * PURPOSE:     Checks mesh id for outgoingConData is same for
 *              incomingConData.
 * PARAMETERS:  node - pointer to the node which received the event.
 *              msdpData - MSDP node specific data
 *              incomingConData - pointer to the incomming connection data.
 *              outgoingConData - pointer to the outgoing connection data.
 * RETURN:      true if it passes the mesh check else false.
 */
bool
MsdpMeshCheck(Node* node,
              MsdpData* msdpData,
              MsdpConnectionData* incomingConData,
              MsdpConnectionData* outgoingConData)
{
    if (incomingConData == outgoingConData)
    {
        return false;
    }
    MsdpMeshDataMap* meshDataMap = msdpData->meshDataMap;
    if (meshDataMap != NULL) // Mesh has been configured
    {
        Int32 incomingMeshId =
            MsdpMeshFindMeshIdByConnectionData(meshDataMap, incomingConData);
        Int32 outgoingMeshId =
            MsdpMeshFindMeshIdByConnectionData(meshDataMap, outgoingConData);
        if (incomingMeshId != 0 && outgoingMeshId != 0
            && incomingMeshId == outgoingMeshId)
        {
            return false;
        }
    }
    return true;
}

/*
 * NAME:        MsdpPeerRPFCheck.
 * PURPOSE:     Checks perform the MSDP Peer-RPF check.
 * PARAMETERS:  node - pointer to the node which received the event.
 *              msdpData - MSDP node specific data
 *              connectionData - pointer to the incomming connection data.
 *              saData - parsed SA message
                incomingInterface - found incoming interface
 * RETURN:      true if it passes the Peer-RPF check else false.
 */
bool
MsdpPeerRPFCheck(Node* node,
                 MsdpData* msdpData,
                 MsdpConnectionData* connectionData,
                 MsdpReceivedSAData saData,
                 Int32* incomingInterface)
{
    /*
     * An SA message originated by R and received by X from N is accepted
     * if N is the peer-RPF neighbor for X, and is discarded otherwise.
     *
     *          MPP(R,N)                 MP(N,X)
     *  R ---------....-------> N ------------------> X
     *         SA(S,G,R)                SA(S,G,R)
     */

    NodeAddress destinationAddress = saData.rpAddr;
    NodeAddress nextHopAddress;

    // Finding incoming interface
    NetworkGetInterfaceAndNextHopFromForwardingTable(
                            node,
                            connectionData->peerAddr,
                            incomingInterface,
                            &nextHopAddress);

    // 1. N == R (X has an MSDP peering with R).
    if (saData.rpAddr == connectionData->peerAddr)
    {
        if (MSDP_DEBUG_PEER_RPF)
        {
            char time[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(node->getNodeTime(), time);

            printf("Time:%s ", time);

            printf("Node:%d peer:%x Peer RPF Passed:1.N == R\n",
                node->nodeId, connectionData->peerAddr);
        }
        return true;
    }

    Int32 interfaceIndex = -1;
    // 2. N is the eBGP NEXT_HOP of the Peer-RPF route for R.
    NetworkGetInterfaceAndNextHopFromForwardingTable(
                                node,
                                destinationAddress,
                                &interfaceIndex,
                                &nextHopAddress,
                                true,
                                EXTERIOR_GATEWAY_PROTOCOL_EBGPv4);

    if (nextHopAddress == connectionData->peerAddr)
    {
        if (MSDP_DEBUG_PEER_RPF)
        {
            char time[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(node->getNodeTime(), time);

            printf("Time:%s ", time);

            printf("Node:%d peer:%x Peer RPF Passed:"
                "2.N is the eBGP NEXT_HOP\n",
                node->nodeId, connectionData->peerAddr);
        }
        return true;
    }

    if (nextHopAddress == (UInt32)NETWORK_UNREACHABLE)
    {
        /*
         * 3. The Peer-RPF route for R is learned through a distance-vector
         *    or path-vector routing protocol (e.g., BGP, RIP, DVMRP) and N
         *    is the neighbor that advertised the Peer-RPF route for R
         *    (e.g., N is the iBGP advertiser of the route for R),
         *    or N is the IGP next hop for R if the route for R is learned
         *    via a link-state protocol.
         */
        NetworkGetInterfaceAndNextHopFromForwardingTable(
                                        node,
                                        destinationAddress,
                                        &interfaceIndex,
                                        &nextHopAddress,
                                        false,
                                        EXTERIOR_GATEWAY_PROTOCOL_EBGPv4);

        if (nextHopAddress == connectionData->peerAddr)
        {
            if (MSDP_DEBUG_PEER_RPF)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d peer:%x Peer RPF Passed:"
                    "3.N is the IGP or iBGP next hop for R\n",
                    node->nodeId, connectionData->peerAddr);
            }
            return true;
        }
    }
    else
    {
        /*
         * 4. N resides in the closest AS in the best path towards R.  If
         *    multiple MSDP peers reside in the closest AS, the peer with
         *    the highest IP address is the rpf-peer.
         */
        BgpData* bgp = (BgpData*) node->appData.exteriorGatewayVar;
        BgpConnectionInformationBase* bgpConnInfo =
            (BgpConnectionInformationBase*)
            BUFFER_GetData(&(bgp->connInfoBase));
        Int32 numEntries = BUFFER_GetCurrentSize(&(bgp->connInfoBase))
            / sizeof(BgpConnectionInformationBase);
        Int32 remoteAsId = 0;
        Int32 i;
        for (i = 0; i < numEntries; i++)
        {
            NodeAddress remoteAddr =
                bgpConnInfo->remoteAddr.interfaceAddr.ipv4;
            if (nextHopAddress == remoteAddr)
            {
                remoteAsId = bgpConnInfo->asIdOfPeer;
                break;
            }
            bgpConnInfo++;
        }

        if (remoteAsId == 0)
        {
            if (MSDP_DEBUG_PEER_RPF)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d peer:%x Peer RPF Failed:\n",
                    node->nodeId, connectionData->peerAddr);
            }
            return false;
        }

        /*
         * Check whether N resides in the closest AS
         * in the best path towards R
         */
        if (remoteAsId == connectionData->asIdOfPeer)
        {
            MsdpConnectionData* conData = NULL;
            MsdpConnectionDataMap::iterator connectionDataMapIt;
            connectionDataMapIt = msdpData->connectionDataMap->begin();
            bool isPeerIpGreater = true;
            for (;connectionDataMapIt != msdpData->connectionDataMap->end();
                connectionDataMapIt++)
            {
                conData = connectionDataMapIt->second;
                /*
                 * The peer with the highest IP address is the rpf-peer.
                 * MSDP peers, which are NOT in state ESTABLISHED,
                 * are not eligible for peer RPF consideration.
                 */
                if (remoteAsId == conData->asIdOfPeer
                        && conData->state == MSDP_ESTABLISHED)
                {
                    if (conData->peerAddr > connectionData->peerAddr)
                    {
                        isPeerIpGreater = false;
                        break;
                    }
                }
            }
            if (isPeerIpGreater == true)
            {
                if (MSDP_DEBUG_PEER_RPF)
                {
                    char time[MAX_STRING_LENGTH];
                    TIME_PrintClockInSecond(node->getNodeTime(), time);

                    printf("Time:%s ", time);

                    printf("Node:%d peer:%x Peer RPF Passed:"
                        "4.N resides in the closest AS and"
                        " is highest IP address peer\n",
                        node->nodeId, connectionData->peerAddr);
                }
                return true;
            }
        }
    }

    // 5. N is configured as the static RPF-peer for R.
    if (msdpData->defaultPeerList != NULL)
    {
        MsdpDefaultPeerList::iterator defaultPeerListIt =
                                    msdpData->defaultPeerList->begin();
        for (; defaultPeerListIt != msdpData->defaultPeerList->end();
            defaultPeerListIt++)
        {
            if (*defaultPeerListIt == connectionData->peerAddr)
            {
                if (MSDP_DEBUG_PEER_RPF)
                {
                    char time[MAX_STRING_LENGTH];
                    TIME_PrintClockInSecond(node->getNodeTime(), time);

                    printf("Time:%s ", time);

                    printf("Node:%d peer:%x Peer RPF Passed:"
                        "5.N is configured as the static RPF-peer for R\n",
                        node->nodeId, connectionData->peerAddr);
                }
                return true;
            }
        }
    }
    if (MSDP_DEBUG_PEER_RPF)
    {
        char time[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(node->getNodeTime(), time);

        printf("Time:%s ", time);

        printf("Node:%d peer:%x Peer RPF Failed:\n",
            node->nodeId, connectionData->peerAddr);
    }
    return false;
}

/*
 * NAME:        MsdpForwardSAWithMeshCheck.
 * PURPOSE:     Perform mesh check and forward the SA message.
 * PARAMETERS:  node - pointer to the node which received the event.
 *              msdpData - MSDP node specific data
 *              saData - parsed SA message
 *              connectionData* - pointer to the connection database.
 *              msg - message pointer
 * RETURN:      Number of SA messages sent to the peers after Mesh Check.
 */
Int32
MsdpForwardSAWithMeshCheck(Node* node,
                          MsdpData* msdpData,
                          MsdpReceivedSAData* saData,
                          MsdpConnectionData* connectionData,
                          Message* msg = NULL)
{
    MsdpConnectionDataMap::iterator outgoingConnection =
        msdpData->connectionDataMap->begin();
    bool outgoingMeshCeck;
    Int32 saMessageSent = 0;
    for (;outgoingConnection != msdpData->connectionDataMap->end();
        outgoingConnection++)
    {
        MsdpPerformSAFilterOutCheck(node,
                                    msdpData,
                                    outgoingConnection->second,
                                    saData);
        MsdpConnectionData* outgoingConnectionData =
                                        outgoingConnection->second;
        // Outgoing Mesh check
        outgoingMeshCeck = MsdpMeshCheck(node,
                                         msdpData,
                                         connectionData,
                                         outgoingConnectionData);

        // Forwarding the SA message
        if (outgoingMeshCeck == true)
        {
            if (MSDP_DEBUG_MESH)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d peer:%x Event:outgoingMeshCeck == TRUE\n",
                    node->nodeId, outgoingConnectionData->peerAddr);
            }
            if (outgoingConnectionData->state == MSDP_ESTABLISHED)
            {
                Message* newMsg = NULL;
                if (msg != NULL)
                {
                    newMsg = MESSAGE_Duplicate(node, msg);
                }
                MsdpSendSAMessage(node,
                                  msdpData,
                                  saData->rpAddr,
                                  saData->sgEntryList,
                                  outgoingConnectionData,
                                  MSDP_SOURCE_ACTIVE,
                                  newMsg);
                saMessageSent++;

                // Reset keepalive timer
                MsdpSetOrResetTimer(node,
                                    MSDP_KEEPALIVE_TIMER,
                                    msdpData,
                                    outgoingConnectionData);
            }
        }
        else
        {
            if (MSDP_DEBUG_MESH)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d peer:%x Event:outgoingMeshCeck == FALSE\n",
                    node->nodeId, outgoingConnectionData->peerAddr);
            }
        }
    }
    return saMessageSent;
}

/*
 * NAME:        MsdpSAStateMapFindSGEntryByGroup.
 * PURPOSE:     Searches for SG Entry Map for group address from SA State Map.
 * PARAMETERS:  saStateMap - SA State Map from Msdp SA Cache for an RP
 *              grpAddr - Group Address
 *              sgEntryList - List of SG Entry if found else empty list.
 * RETURN:      NULL
 */
void
MsdpSAStateMapFindSGEntryByGroup(
    MsdpSAStateMap* saStateMap,
    NodeAddress grpAddr,
    MsdpSGEntryList* sgEntryList)
{
    MsdpSAStateMap::iterator saStateMapIt = saStateMap->begin();
    for (;saStateMapIt != saStateMap->end(); saStateMapIt++)
    {
        MsdpSGEntry sgEntry = saStateMapIt->second->sgEntry;
        if (sgEntry.groupAddr == grpAddr)
        {
            sgEntryList->push_back(sgEntry);
        }
    }
}

/*
 * NAME:        MsdpGetConnctionDataByPeerAdd.
 * PURPOSE:     Searches for connectionData in msdpData for a Peer
 * PARAMETERS:  msdpData - MSDP node specific data
 *              peer - Peer address
 * RETURN:      if found, pointer to the searched MsdpConnectionData
 *              else NULL.
 */
MsdpConnectionData*
MsdpGetConnctionDataByPeerAdd(MsdpData* msdpData, NodeAddress peer)
{
    MsdpConnectionData* connectionData = NULL;
    MsdpConnectionDataMap::iterator connectionDataMapIt;
    connectionDataMapIt =
        msdpData->connectionDataMap->find(peer);

    if (connectionDataMapIt !=
        msdpData->connectionDataMap->end())
    {
        connectionData = connectionDataMapIt->second;
    }

    return connectionData;
}

/*
 * NAME:        MsdpProcessEstablishedState.
 * PURPOSE:     Processes the events to occure in Established State
 *              and ignore the event for any other state.
 * PARAMETERS:  node - pointer to the node which received the event.
 *              msg - message received
 *              msdpData - MSDP node specific data
 *              event - type of MSDP event
 *              connectionData - connection specific data
 * RETURN:      NONE.
 */
static
void MsdpProcessEstablishedState(
    Node* node,
    Message* msg,
    MsdpData* msdpData,
    MsdpEvent event,
    MsdpConnectionData* connectionData,
    char* packet = NULL)
{
    if (connectionData != NULL
        && connectionData->state != MSDP_ESTABLISHED)
    {
        char* info = MESSAGE_ReturnInfo(msg);
            MsdpSGEntry* sgEntry = (MsdpSGEntry*)info;
            NodeAddress* rpAddr = (NodeAddress*)(info + sizeof(MsdpSGEntry));
        RoutingPimSmTreeInfoBaseRow* treeInfoBaseRowPtr =
            RoutingPimSmSearchTreeInfoBaseForThisGroup
            (node, sgEntry->groupAddr, sgEntry->srcAddr, ROUTING_PIMSM_SG);
        if (treeInfoBaseRowPtr != NULL &&
            treeInfoBaseRowPtr->isFirstRegisterPacket == false)
        {
            treeInfoBaseRowPtr->isFirstRegisterPacket = true;
        }
        return;
    }

    MsdpError error = MSDP_NO_ERROR;

    switch (event)
    {
        case MSDP_HOLD_TIMER_EXPIRED:
        {
            connectionData->holdTimer = NULL;
        }
        case MSDP_TLV_FORMAT_ERROR:
        case MSDP_TRANSPORT_PASSIVE_CONNECTION_CLOSED:
        {
            APP_TcpCloseConnection(node, connectionData->connectionId);
            break;
        }
        case MSDP_TRANSPORT_ACTIVE_CONNECTION_CLOSED:
        {
            if (connectionData->keepAliveTimer
                && !connectionData->keepAliveTimer->cancelled)
            {
                MESSAGE_CancelSelfMsg(node,
                    connectionData->keepAliveTimer);
            }
            if (connectionData->holdTimer
                && !connectionData->holdTimer->cancelled)
            {
                MESSAGE_CancelSelfMsg(node,
                    connectionData->holdTimer);
            }
            connectionData->keepAliveTimer = NULL;
            connectionData->holdTimer = NULL;
            connectionData->state = MSDP_DISABLED;
            connectionData->connectionId = -1;
            connectionData->connectRetryTimer = NULL;
            break;
        }
        case MSDP_KEEPALIVE_TIMER_EXPIRED:
        {
            MsdpSendKeepAliveTLV(node, msdpData, connectionData);
            // Reset keepalive timer
            MsdpSetOrResetTimer(node,
                                MSDP_KEEPALIVE_TIMER,
                                msdpData,
                                connectionData,
                                true);
            break;
        }
        case MSDP_SEND_SA_MESSAGE:
        {
            char* info = MESSAGE_ReturnInfo(msg);
            MsdpSGEntry* sgEntry = (MsdpSGEntry*)info;
            NodeAddress* rpAddr = (NodeAddress*)(info + sizeof(MsdpSGEntry));

            bool saRedistribute = MsdpCheckIfSGPassesACL(node,
                                                     sgEntry,
                                                     MSDP_REDISTRIBUTE,
                                                     msdpData,
                                                     connectionData);

            if (saRedistribute == true)
            {
                if (msdpData->isCacheEnabled)
                {
                    error = MsdpSACacheInsert(node,
                                              *rpAddr,
                                              *sgEntry,
                                              msdpData,
                                              false);
                }

                MsdpSGEntryList sgEntryList;
                sgEntryList.push_back(*sgEntry);

                if (connectionData->state == MSDP_ESTABLISHED)
                {
                    MsdpSendSAMessage(
                                      node,
                                      msdpData,
                                      *rpAddr,
                                      sgEntryList,
                                      connectionData,
                                      MSDP_SOURCE_ACTIVE,
                                      MESSAGE_Duplicate(node, msg));

                    // Increment the statistical variable
                    msdpData->stats.saPacketsOriginated++;

                    // Reset keepalive timer
                    MsdpSetOrResetTimer(node,
                                        MSDP_KEEPALIVE_TIMER,
                                        msdpData,
                                        connectionData);
                }
            }
            else
            {
                if (MSDP_DEBUG_SA)
                {
                    char time[MAX_STRING_LENGTH];
                    TIME_PrintClockInSecond(node->getNodeTime(), time);

                    printf("Time:%s ", time);

                    printf("Node:%d peer:%x Event:"
                        " saRedistribute == FALSE\n",
                        node->nodeId, connectionData->peerAddr);
                }
            }
            break;
        }
        case MSDP_SA_ADVERTISEMENT_TIMER_EXPIRED:
        {
            if ((msdpData->saCache != NULL) && (msdpData->saCache->size() > 0))
            {
                MsdpSACacheMap::iterator saCacheMapIt;
                saCacheMapIt = msdpData->saCache->begin();

                for (; saCacheMapIt != msdpData->saCache->end();
                    saCacheMapIt++)
                {
                    MsdpReceivedSAData saData;
                    saData.rpAddr = (NodeAddress)saCacheMapIt->first;

                    if (!MAPPING_IsIpAddressOfThisNode(
                                                node,
                                                node->nodeId,
                                                saData.rpAddr))
                    {
                        continue;
                    }

                    MsdpSACacheGetSGEntryListByRPAddress(node,
                                                     saData.rpAddr,
                                                     msdpData,
                                                     &(saData.sgEntryList));
                    saData.data = NULL;
                    saData.dataLength = 0;
                    // Forwarding of SA message
                    msdpData->stats.saPacketsAdvertised +=
                    MsdpForwardSAWithMeshCheck(node,
                                              msdpData,
                                              &saData,
                                              NULL);
                }
            }
            MsdpSetOrResetTimer(node,
                                MSDP_SA_ADVERTISEMENT_TIMER,
                                msdpData,
                                connectionData,
                                true);
            break;
        }
        case MSDP_CACHE_SA_STATE_TIMER_EXPIRED:
        {
            MsdpHandleSAStateTimerExpiry(node, msg, msdpData);
            break;
        }
        case MSDP_RECEIVE_KEEPALIVE_MESSAGE:
        {

            msdpData->stats.keepAliveReceived++;

            // Reset hold timer
            MsdpSetOrResetTimer(node,
                                MSDP_HOLD_TIMER,
                                msdpData,
                                connectionData);
            break;
        }
        case MSDP_RECEIVE_SOURCE_ACTIVE_MESSAGE:
        {
            // Reset hold timer
            MsdpSetOrResetTimer(node,
                                MSDP_HOLD_TIMER,
                                msdpData,
                                connectionData);

            msdpData->stats.saPacketsReceived++;

            MsdpReceivedSAData saData;
            MsdpReconstructStructureSA(
                            packet,
                            &saData);
            // Incoming Peer RPF check
            Int32 incomingInterface = -1;
            bool incomingPeerRPFCeck = MsdpPeerRPFCheck(node,
                                                        msdpData,
                                                        connectionData,
                                                        saData,
                                                        &incomingInterface);
            if (incomingPeerRPFCeck == true)
            {
                // SA filter In Check and Cache the SA message
                MsdpPerformSAFilterInCheckAndCacheSA(node,
                                                     msdpData,
                                                     connectionData,
                                                     &saData,
                                                     msg,
                                                     incomingInterface);
                if (saData.sgEntryList.size() > 0)
                {
                    Message* newMsg = NULL;
                    if (saData.dataLength > 0)
                    {
                        newMsg = MESSAGE_Alloc(node,
                                               APP_LAYER,
                                               APP_MSDP,
                                               MSG_APP_MSDP_SendSAMessage);
                        MESSAGE_CopyInfo(node, newMsg, msg);
                        MESSAGE_PacketAlloc(node,
                                            newMsg,
                                            saData.dataLength,
                                            TRACE_PIM);
                        memcpy(MESSAGE_ReturnPacket(newMsg),
                            saData.data,
                            saData.dataLength);
                    }
                    // SA filter Out Check
                    MsdpPerformSAFilterOutCheck(node,
                                                msdpData,
                                                connectionData,
                                                &saData);
                    if (saData.sgEntryList.size() > 0)
                    {
                        // Forwarding of SA message
                        msdpData->stats.saPacketsForwarded +=
                                MsdpForwardSAWithMeshCheck(node,
                                                          msdpData,
                                                          &saData,
                                                          connectionData,
                                                          newMsg);
                    }
                    if (newMsg != NULL)
                    {
                        MESSAGE_Free(node, newMsg);
                    }
                }
                else
                {
                    msdpData->stats.saPacketsDiscarded++;
                }
            }
            else
            {
                msdpData->stats.saPacketsDiscarded++;
            }
            if (saData.data != NULL)
            {
                MEM_free(saData.data);
                saData.data = NULL;
            }
            break;
        }
        default:
        {
            // Ignore rest of the events
        }
    }
}

/*
 * NAME:        MsdpProcessEventOrPacket.
 * PURPOSE:     Processes the events or Packet.
 * PARAMETERS:  node - pointer to the node which received the event.
 *              msg - message received
 *              event - type of MSDP event
 *              connectionData - connection specific data
 *              packet - assembled payload byte stream.
 * RETURN:      NONE.
 */
void
MsdpProcessEventOrPacket(
    Node* node,
    Message* msg,
    MsdpEvent event,
    MsdpConnectionData* connectionData,
    char* packet = NULL)
{

    MsdpData* msdpData = (MsdpData*) node->appData.msdpData;

    switch (event)
    {
        case MSDP_START:
        {
            MsdpProcessInactiveState(node,
                                     msdpData,
                                     event,
                                     connectionData);
            break;
        }

        case MSDP_TRANSPORT_ACTIVE_CONNECTION_OPEN:
        case MSDP_TRANSPORT_PASSIVE_CONNECTION_OPEN:
        {
            MsdpProcessConnectingAndListenState(node,
                                                msg,
                                                msdpData,
                                                event,
                                                connectionData);
            break;
        }
        case MSDP_TRANSPORT_ACTIVE_CONNECTION_CLOSED:
        case MSDP_TRANSPORT_PASSIVE_CONNECTION_CLOSED:
        {
            if (connectionData->state == MSDP_CONNECTING)
            {
                MsdpProcessConnectingAndListenState(node,
                                                    msg,
                                                    msdpData,
                                                    event,
                                                    connectionData);
            }
            else if (connectionData->state == MSDP_ESTABLISHED)
            {
                MsdpProcessEstablishedState(node,
                                            msg,
                                            msdpData,
                                            event,
                                            connectionData);
            }
            break;
        }

        case MSDP_TLV_FORMAT_ERROR:
        case MSDP_KEEPALIVE_TIMER_EXPIRED:
        case MSDP_HOLD_TIMER_EXPIRED:
        case MSDP_SEND_SA_MESSAGE:
        case MSDP_SA_ADVERTISEMENT_TIMER_EXPIRED:
        case MSDP_CACHE_SA_STATE_TIMER_EXPIRED:
        case MSDP_RECEIVE_KEEPALIVE_MESSAGE:
        case MSDP_RECEIVE_SOURCE_ACTIVE_MESSAGE:
        {
            MsdpProcessEstablishedState(node,
                                        msg,
                                        msdpData,
                                        event,
                                        connectionData,
                                        packet);
            break;
        }
    }
}

/*
 * NAME:        MsdpGetConnctionDataByConnectionId.
 * PURPOSE:     Searches for connectionData in msdpData by connection ID
 * PARAMETERS:  msdpData - MSDP node specific data
 *              connectionId - TCP connection ID
 * RETURN:      if found, pointer to the searched MsdpConnectionData
 *              else NULL.
 */
MsdpConnectionData*
MsdpGetConnctionDataByConnectionId(MsdpData* msdpData, Int32 connectionId)
{
    MsdpConnectionData* connectionData = NULL;
    MsdpConnectionDataMap::iterator connectionDataMapIt;

    for (connectionDataMapIt = msdpData->connectionDataMap->begin();
        connectionDataMapIt != msdpData->connectionDataMap->end();
        connectionDataMapIt++)
    {
        if (connectionDataMapIt->second->connectionId == connectionId)
        {
            connectionData = connectionDataMapIt->second;
            break;
        }
    }
    return connectionData;
}

/*
 * NAME:        MsdpProcessPacket.
 * PURPOSE:     processes the packet received and calls respective handler
 *              depending on the type of event
 * PARAMETERS:  node - the node which is receiving data
 *              msg - message received
 *              ConnectionData - the connection which has received the data
 *              packet - stores packet received
 * RETURN:      NONE.
 */
static
void
MsdpProcessPacket(
    Node* node,
    Message* msg,
    MsdpConnectionData* connectionData,
    char* packet)
{
    // Swapping bytes of common header
    // i.e. type and length
    // no need to swap type field, as it is only one byte long
    // First byte contains type of packet
    MsdpMessageType type = (MsdpMessageType)packet[0];

    char* tmpPacket = packet + MSDP_MESSAGE_TYPE_SIZE;

    EXTERNAL_ntoh(
        tmpPacket,
        MSDP_MESSAGE_LENGTH_SIZE);

    // Need to check if it is message header error

    switch (type)
    {
        case MSDP_SOURCE_ACTIVE:
        {
            if (MSDP_DEBUG_SA)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d peer:%x Event:"
                    " MSDP_RECEIVE_SOURCE_ACTIVE_MESSAGE\n",
                    node->nodeId, connectionData->peerAddr);
            }
            MsdpProcessEventOrPacket(node,
                                     msg,
                                     MSDP_RECEIVE_SOURCE_ACTIVE_MESSAGE,
                                     connectionData,
                                     tmpPacket);
            break;
        }
        case MSDP_KEEPALIVE:
        {
            UInt16 length = (UInt16)tmpPacket[0];
            if (MSDP_DEBUG)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d peer:%x Event:"
                    " MSDP_RECEIVE_KEEPALIVE_MESSAGE\n",
                    node->nodeId, connectionData->peerAddr);
            }
            if (length == MSDP_KEEP_ALIVE_TLV_LENGTH)
            {
                MsdpProcessEventOrPacket(node,
                                         msg,
                                         MSDP_RECEIVE_KEEPALIVE_MESSAGE,
                                         connectionData,
                                         NULL);
            }
            else
            {
                MsdpProcessEventOrPacket(node,
                                         NULL,
                                         MSDP_TLV_FORMAT_ERROR,
                                         connectionData,
                                         NULL);
            }
            break;
        }
        default:
        {
            MsdpProcessEventOrPacket(node,
                                     NULL,
                                     MSDP_TLV_FORMAT_ERROR,
                                     connectionData,
                                     NULL);
        }
    }
    MEM_free(packet);
}

/*
 * NAME:        MsdpProcessRawData.
 * PURPOSE:     handling raw data bytes received from the transport layer and
 *              reassembling them or dividing them to make one complete msdp
 *              packet and then sending that to the relevent function to
 *              process that
 * PARAMETERS:  node - the node which is receiving data
 *              msg - message received
 *              connectionData - the connection which has received the data
 *              data - the raw trasport data bytes
 *              dataSize - the size of data received
 * RETURN:      NONE.
 */
void
MsdpProcessRawData(Node* node,
                   Message* msg,
                   MsdpConnectionData* connectionData,
                   char* data,
                   Int32 dataSize)
{
    UInt16 expectedLengthOfPacket;
    MsdpMessageBuffer* buffer = &(connectionData->buffer);
    if (buffer->currentSize == 0)
    {
        // buffer size 0 means its a fresh data stream, and it contains
        // multiple packets
        if (dataSize >= MSDP_MIN_HDR_LEN)
        {
            // data size is greater than the header size so I can
            // look at the msdp packet length
            memcpy(&expectedLengthOfPacket, data + MSDP_MESSAGE_TYPE_SIZE,
                MSDP_MESSAGE_LENGTH_SIZE);
            EXTERNAL_ntoh(
                    &expectedLengthOfPacket,
                    MSDP_MESSAGE_LENGTH_SIZE);
            if (expectedLengthOfPacket <= dataSize)
            {
                // Got one complete packet. send it to the corresponding
                // msdp function
                char* newData = NULL;
                char* packet = NULL;
                Int32   remainingDataSize = 0;

                packet = (char*) MEM_malloc(expectedLengthOfPacket);
                memcpy(packet, data, expectedLengthOfPacket);

                // Call the process packet function to process the complete
                // packet received
                MsdpProcessPacket(node, msg, connectionData, packet);
                remainingDataSize = dataSize - expectedLengthOfPacket;

                if (remainingDataSize)
                {
                    newData = (char *) MEM_malloc(remainingDataSize);
                    memcpy(newData,
                           data + expectedLengthOfPacket,
                           remainingDataSize);
                    MEM_free(data);

                    // Process the remaining size of data.
                    MsdpProcessRawData(node,
                                      msg,
                                      connectionData,
                                      newData,
                                      remainingDataSize);
                }
                else
                {
                    MEM_free(data);
                }
            }
            else
            {
                // This is not one complete msdp packet so store it in the
                // buffer
                buffer->data
                    = (unsigned char*) MEM_malloc(expectedLengthOfPacket);
                memcpy(buffer->data, data, dataSize);
                MEM_free(data);
                buffer->expectedSize = expectedLengthOfPacket;
                buffer->currentSize  = dataSize;
            }
        }
        else
        {
            // Data is too small to learn the expected size so just buffer it
            buffer->data = (unsigned char*) MEM_malloc(dataSize);
            memcpy(buffer->data, data, dataSize);
            MEM_free(data);
            buffer->expectedSize = 0;
            buffer->currentSize  = dataSize;
        }
    }
    else
    {
        // There is data in the buffer
        if (buffer->currentSize && buffer->expectedSize)
        {
            // There is data in the buffer and it is with the header
            if (buffer->expectedSize - buffer->currentSize <= dataSize)
            {
                // The new data is big enough for reassembly.
                char* newData = NULL;
                Int32 newDataSize = 0;

                newData = (char *)MEM_malloc(buffer->currentSize + dataSize);
                memcpy(newData, buffer->data, buffer->currentSize);
                memcpy(newData + buffer->currentSize, data, dataSize);
                MEM_free(buffer->data);
                buffer->data = NULL;
                MEM_free(data);
                newDataSize = buffer->currentSize + dataSize;
                buffer->currentSize = 0;
                buffer->expectedSize = 0;

                MsdpProcessRawData(node,
                                  msg,
                                  connectionData,
                                  newData,
                                  newDataSize);
            }
            else
            {
                // New data is not big enough for reassembly, so keep looking.
                memcpy(buffer->data + buffer->currentSize, data, dataSize);
                buffer->currentSize += dataSize;
                MEM_free(data);
            }
        }
        else
        {
            // The data in the buffer is not with header
            char* newData = NULL;
            Int32 newDataSize = 0;

            newData = (char *) MEM_malloc(buffer->currentSize + dataSize);
            memcpy(newData, buffer->data, buffer->currentSize);
            memcpy(newData + buffer->currentSize, data, dataSize);
            MEM_free(buffer->data);
            buffer->data = NULL;
            MEM_free(data);
            newDataSize = buffer->currentSize + dataSize;
            buffer->currentSize = 0;
            buffer->expectedSize = 0;

            MsdpProcessRawData(node,
                              msg,
                              connectionData,
                              newData,
                              newDataSize);
        }
    }
}

/*
 * NAME:        MsdpProcessTCPData.
 * PURPOSE:     Reassembling transport raw data to form one complete msdp
 *              packet
 * PARAMETERS:  node - the node which is receiving data
 *              msg - message received
 *              connectionData - Connection Data for the Peer from which
 *              the TCP data has been recieved
 * RETURN:      NONE.
 */
void
MsdpProcessTCPData(Node* node,
                   Message* msg,
                   MsdpConnectionData* connectionData)
{
    TransportToAppDataReceived* dataRecvd = NULL;
    char* packetRecvd = NULL;
    char* data = NULL;
    Int32 packetSize = 0;

    dataRecvd = (TransportToAppDataReceived*) MESSAGE_ReturnInfo(msg);

    packetRecvd = MESSAGE_ReturnPacket(msg);
    packetSize = MESSAGE_ReturnPacketSize(msg);

    MsdpMessageBuffer* buffer = NULL;
    UInt16  expectedLengthOfPacket = 0;

    buffer = &connectionData->buffer;
    if (buffer->currentSize == 0)
    {
        // buffer size 0 means its a fresh data stream, so parse it
        // if it is a single packet or multiple packet
        if (packetSize >= MSDP_MIN_HDR_LEN)
        {
            memcpy(&expectedLengthOfPacket,
                (packetRecvd + MSDP_MESSAGE_TYPE_SIZE),
                MSDP_MESSAGE_LENGTH_SIZE);
            EXTERNAL_ntoh(
                    (void*)&expectedLengthOfPacket,
                    MSDP_MESSAGE_LENGTH_SIZE);
            if (expectedLengthOfPacket == packetSize)
            {
                data = (char*) MEM_malloc(packetSize);
                memcpy(data, packetRecvd, packetSize);
                MsdpProcessPacket(node, msg, connectionData, data);
                MESSAGE_Free(node, msg);
                return;
            }
        }
    }
    // At this point the data either contains multiple msdp packets
    // or it needs re-assembling
    data = (char*) MEM_malloc(packetSize);
    memcpy(data, packetRecvd, packetSize);
    MsdpProcessRawData(node, msg, connectionData, data, packetSize);
    MESSAGE_Free(node, msg);
}

/*
 * NAME:        MsdpLayer.
 * PURPOSE:     Models the behaviour of MSDP Speaker on receiving the
 *              message encapsulated in msg.
 * PARAMETERS:  node - pointer to the node which received the message.
 *              msg - message received by the layer
 * RETURN:      None.
 */
void
MsdpLayer(Node* node, Message* msg)
{
    //MsdpEvent eventType = (MsdpEvent)MESSAGE_GetEvent(msg);
    Int32 eventType = (MsdpEvent)MESSAGE_GetEvent(msg);

    MsdpData* msdpData = (MsdpData*) node->appData.msdpData;
    MsdpConnectionData* connectionData = NULL;

    MsdpError error = MSDP_NO_ERROR;

    switch (eventType)
    {
        case MSG_APP_MSDP_StartTimerExpired:
        {

            NodeAddress* peer = (NodeAddress*)MESSAGE_ReturnInfo(msg);

            if (MSDP_DEBUG)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d peer:%x Event:"
                    " MSG_APP_MSDP_StartTimerExpired\n", node->nodeId, *peer);
            }
            if ((connectionData =
                MsdpGetConnctionDataByPeerAdd(msdpData, *peer)) == NULL)
            {
                error = MSDP_INVALID_INFO;
                break;
            }

            MsdpProcessEventOrPacket(node,
                                     msg,
                                     MSDP_START,
                                     connectionData);
            MESSAGE_Free(node, msg);
            break;
        }

        case MSG_APP_MSDP_AdvertisementTimerExpired:
        {
            if (MSDP_DEBUG)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d Event: "
                    "MSG_APP_MSDP_AdvertisementTimerExpired\n",
                    node->nodeId);
            }

            MsdpProcessEventOrPacket(node,
                                     msg,
                                     MSDP_SA_ADVERTISEMENT_TIMER_EXPIRED,
                                     NULL);
            break;
        }

        case MSG_APP_MSDP_KeepAliveTimerExpired:
        {
            NodeAddress* peer = (NodeAddress*)MESSAGE_ReturnInfo(msg);

            if (MSDP_DEBUG)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d peer:%x Event: "
                    "MSG_APP_MSDP_KeepAliveTimerExpired\n",
                    node->nodeId, *peer);
            }

            if ((connectionData =
                MsdpGetConnctionDataByPeerAdd(msdpData, *peer)) == NULL)
            {
                error = MSDP_INVALID_INFO;
                break;
            }

            MsdpProcessEventOrPacket(node,
                                 msg,
                                 MSDP_KEEPALIVE_TIMER_EXPIRED,
                                 connectionData);
            break;
        }

        case MSG_APP_MSDP_HoldTimerExpired:
        {
            NodeAddress* peer = (NodeAddress*)MESSAGE_ReturnInfo(msg);

            if (MSDP_DEBUG)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d peer:%x Event: "
                    "MSG_APP_MSDP_HoldTimerExpired\n",
                    node->nodeId, *peer);
            }

            if ((connectionData =
                MsdpGetConnctionDataByPeerAdd(msdpData, *peer)) == NULL)
            {
                error = MSDP_INVALID_INFO;
                break;
            }

            MsdpProcessEventOrPacket(node,
                                 msg,
                                 MSDP_HOLD_TIMER_EXPIRED,
                                 connectionData);
            MESSAGE_Free(node, msg);
            break;
        }

        case MSG_APP_MSDP_ConnectRetryTimerExpired:
        {
            NodeAddress* peer = (NodeAddress*)MESSAGE_ReturnInfo(msg);

            if (MSDP_DEBUG_CSM)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d peer:%x Event: "
                    "MSDP_CONNECTRETRY_TIMER_EXPIRED\n",
                    node->nodeId, *peer);
            }

            if ((connectionData =
                MsdpGetConnctionDataByPeerAdd(msdpData, *peer)) == NULL)
            {
                error = MSDP_INVALID_INFO;
                break;
            }

            connectionData->state = MSDP_INACTIVE;
            MsdpProcessEventOrPacket(node,
                                     msg,
                                     MSDP_START,
                                     connectionData);
            MESSAGE_Free(node, msg);
            break;
        }

        case MSG_APP_FromTransOpenResult:
        {
            TransportToAppOpenResult* openResult =
                (TransportToAppOpenResult*) MESSAGE_ReturnInfo(msg);
            NodeAddress peer;

            if (openResult->type == TCP_CONN_ACTIVE_OPEN)
            {
                if (openResult->connectionId == -1)
                {
                    break;
                }
                peer = openResult->uniqueId;
                if (MSDP_DEBUG_CSM)
                {
                    char time[MAX_STRING_LENGTH];
                    TIME_PrintClockInSecond(node->getNodeTime(), time);

                    printf("Time:%s ", time);

                    printf("Node:%d peer:%x Event: MSG_APP_FromTransOpenResult"
                        " Result: TCP_CONN_ACTIVE_OPEN"
                        " State Changed To: MSDP_ESTABLISHED\n",
                        node->nodeId, peer);
                }

                if ((connectionData =
                    MsdpGetConnctionDataByPeerAdd(msdpData, peer)) == NULL)
                {
                    error = MSDP_INVALID_INFO;
                    break;
                }

                MsdpProcessEventOrPacket(node,
                                     msg,
                                     MSDP_TRANSPORT_ACTIVE_CONNECTION_OPEN,
                                     connectionData);
            }
            else if (openResult->type == TCP_CONN_PASSIVE_OPEN)
            {
                peer = GetIPv4Address(openResult->remoteAddr);

                if ((connectionData =
                    MsdpGetConnctionDataByPeerAdd(msdpData, peer)) == NULL)
                {
                    if (MSDP_DEBUG_CSM)
                    {
                        char time[MAX_STRING_LENGTH];
                        TIME_PrintClockInSecond(node->getNodeTime(), time);

                        printf("Time:%s ", time);

                        printf("Node:%d peer:%x Event: Connection Request"
                            " regected \n", node->nodeId, peer);
                    }
                    break;
                }
                if (MSDP_DEBUG_CSM)
                {
                    char time[MAX_STRING_LENGTH];
                    TIME_PrintClockInSecond(node->getNodeTime(), time);

                    printf("Time:%s ", time);

                    printf("Node:%d peer:%x"
                        " Event: MSG_APP_FromTransOpenResult"
                        " Result: TCP_CONN_PASSIVE_OPEN"
                        " State Changed To: MSDP_ESTABLISHED\n",
                        node->nodeId, peer);
                }

                MsdpProcessEventOrPacket(node,
                                     msg,
                                     MSDP_TRANSPORT_PASSIVE_CONNECTION_OPEN,
                                     connectionData);
            }
            MESSAGE_Free(node, msg);
            break;
        }

        case MSG_APP_FromTransListenResult:
        {
            if (MSDP_DEBUG_CSM)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d Event: MSG_APP_FromTransListenResult\n",
                    node->nodeId);
            }
            MESSAGE_Free(node, msg);
            break;
        }

        case MSG_APP_FromTransDataSent:
        {
            if (MSDP_DEBUG)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d Event: MSG_APP_FromTransDataSent\n",
                    node->nodeId);
            }
            MESSAGE_Free(node, msg);
            break;
        }

        case MSG_APP_FromTransDataReceived:
        {
            TransportToAppDataReceived* dataRecvd = NULL;
            Int32  connectionId = -1;
            dataRecvd =
                (TransportToAppDataReceived*) MESSAGE_ReturnInfo(msg);
            connectionId = dataRecvd->connectionId;

            if (connectionId >= 0)
            {
                if ((connectionData =
                    MsdpGetConnctionDataByConnectionId(msdpData,
                                                    connectionId)) == NULL)
                {
                    error = MSDP_INVALID_INFO;
                    MESSAGE_Free(node, msg);
                    break;
                }
                MsdpProcessTCPData(node, msg, connectionData);
            }
            else
            {
                  ERROR_Assert(FALSE, "Packet came for a connection"
                            " id which does not exist\n");      
            }
            break;
        }

        case MSG_APP_FromTransCloseResult:
        {
            TransportToAppCloseResult* closeResult =
                (TransportToAppCloseResult*) MESSAGE_ReturnInfo(msg);
            if (closeResult->connectionId == -1)
            {
                break;
            }
            if (closeResult->type == TCP_CONN_ACTIVE_CLOSE)
            {
                if ((connectionData =
                    MsdpGetConnctionDataByConnectionId(msdpData,
                                        closeResult->connectionId )) == NULL)
                {
                    error = MSDP_INVALID_INFO;
                    break;
                }
                if (MSDP_DEBUG_CSM)
                {
                    char time[MAX_STRING_LENGTH];
                    TIME_PrintClockInSecond(node->getNodeTime(), time);

                    printf("Time:%s ", time);

                    printf("Node:%d peer:%x Event: MSG_APP_FromTransCloseResult"
                        " Result: TCP_CONN_ACTIVE_CLOSE\n",
                        node->nodeId, connectionData->peerAddr);
                }
                MsdpProcessEventOrPacket(node,
                                     msg,
                                     MSDP_TRANSPORT_ACTIVE_CONNECTION_CLOSED,
                                     connectionData);

            }
            else
            {
                if ((connectionData =
                    MsdpGetConnctionDataByConnectionId(msdpData,
                                        closeResult->connectionId )) == NULL)
                {
                    error = MSDP_INVALID_INFO;
                    break;
                }
                if (MSDP_DEBUG_CSM)
                {
                    char time[MAX_STRING_LENGTH];
                    TIME_PrintClockInSecond(node->getNodeTime(), time);

                    printf("Time:%s ", time);

                    printf("Node:%d peer:%x Event: MSG_APP_FromTransCloseResult"
                        " Result: TCP_CONN_PASSIVE_CLOSE\n",
                        node->nodeId, connectionData->peerAddr);
                }
                MsdpProcessEventOrPacket(
                                    node,
                                    msg,
                                    MSDP_TRANSPORT_PASSIVE_CONNECTION_CLOSED,
                                    connectionData);
            }
            MESSAGE_Free(node, msg);
            break;
        }
        case MSG_APP_MSDP_SAStateTimerExpired:
        case MSG_APP_MSDP_ExpireSGState:
        {
            MsdpCacheSAStateTimerInfo* saStateTimerInfo =
                (MsdpCacheSAStateTimerInfo*)MESSAGE_ReturnInfo(msg);
            if (MSDP_DEBUG)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d S,G:%x,%x"
                    " Event: MSG_APP_MSDP_SAStateTimerExpired\n",
                    node->nodeId, saStateTimerInfo->sgEntry.srcAddr,
                    saStateTimerInfo->sgEntry.groupAddr);
            }
            MsdpProcessEventOrPacket(node,
                                     msg,
                                     MSDP_CACHE_SA_STATE_TIMER_EXPIRED,
                                     NULL);
            MESSAGE_Free(node, msg);
            break;
        }
        case MSG_APP_MSDP_SendSAMessage:
        {
            MsdpConnectionDataMap::iterator connectionDataMapIt;
            connectionDataMapIt = msdpData->connectionDataMap->begin();

            for (; connectionDataMapIt !=
                msdpData->connectionDataMap->end();
                connectionDataMapIt++)
            {
                MsdpProcessEventOrPacket(node,
                                         msg,
                                         MSDP_SEND_SA_MESSAGE,
                                         connectionDataMapIt->second);
            }
            if (MSDP_DEBUG_SA)
            {
                char time[MAX_STRING_LENGTH];
                TIME_PrintClockInSecond(node->getNodeTime(), time);

                printf("Time:%s ", time);

                printf("Node:%d Event: SA message Originated\n",
                    node->nodeId);
            }
            MESSAGE_Free(node, msg);
            break;
        }
        default:
        {
            // ignore rest of the events
        }
    }

    if (error == MSDP_INVALID_INFO)
    {
        char errStr[MAX_STRING_LENGTH];
        sprintf(errStr, "Node:%d Invalid info added in the msg.\n",
            node->nodeId);
        ERROR_ReportError(errStr);
    }
}

/*
 * NAME:        MsdpReadConfigurationFile.
 * PURPOSE:     Read configuration data from *.msdp file
 * PARAMETERS:  node - pointer to the node
 *              configFile - pointer to input lines
 *              msdpData - MSDP node specific data
 * RETURN:      None.
 */
void
MsdpReadConfigurationFile(Node* node,
                          const NodeInput* configFile,
                          MsdpData* msdpData)
{
    Int32 i = 0;
    msdpData->connectionDataMap = new MsdpConnectionDataMap;
    MsdpConnectionData* tempMsdpConnectionData = NULL;
    NetworkDataIp *ip = (NetworkDataIp*) node->networkData.networkVar;
    bool isPeerFound = false;

    if (MSDP_DEBUG_CONF)
    {
        printf("In MsdpReadConfigurationFile(...)\n");
    }

    for (; i < configFile->numLines; i++)
    {
        Int32 id = 0;
        char c;
        char token[MAX_INPUT_FILE_LINE_LENGTH];
        char accessListIdToken[MAX_INPUT_FILE_LINE_LENGTH];
        Int32 accessListId;
        MsdpConnectionData* connectionData = NULL;
        std :: stringstream currentLine(configFile->inputStrings[i]);
        currentLine >> c;

        // A valid configuration line in .msdp file begins with
        //  node id enclosed in square brackets, e.g. [1].
        if (c != '[')
        {
            char errStr[MAX_STRING_LENGTH];
            sprintf(errStr, "MSDP File is not as per the format"
                            ": should specify node id"
                            " enclosed in square brackets\n%s",
                            configFile->inputStrings[i]);
            ERROR_ReportError(errStr);
        }

        currentLine >> c;
        while (isdigit(c) && !currentLine.eof())
        {
            id = id * 10 + c - '0';
            currentLine >> c;
        }

        if (c != ']')
        {
            char errStr[MAX_STRING_LENGTH];
            sprintf(errStr, "MSDP File is not as per the format"
                              ": Closing bracket missing:\n%s",
                              configFile->inputStrings[i]);
            ERROR_ReportError(errStr);
        }

        // If node id specified in configuration
        //  line is same as current node id
        if (id == node->nodeId)
        {
            currentLine >> token;
            //printf("\n%s", token);
            IO_ConvertStringToLowerCase(token);
            if (!strcmp(token, "msdp"))
            {
                NodeAddress peerAddr;
                currentLine >> token;
                IO_ConvertStringToLowerCase(token);
                if (!strcmp(token, "peer"))
                {
                    isPeerFound = true;
                    if (currentLine.eof())
                    {
                        char errStr[MAX_STRING_LENGTH];
                        sprintf(errStr, "%s:\nPeer Address not specified",
                                        configFile->inputStrings[i]);
                        ERROR_ReportError(errStr);
                    }
                    currentLine >> token;
                    Int32 len = (Int32)strlen(token);
                    MsdpConvertStringToNodeAddress(token, &peerAddr);

                    connectionData =
                        MsdpGetConnctionDataByPeerAdd(msdpData, peerAddr);
                    if (connectionData)
                    {
                        char errStr[MAX_STRING_LENGTH];
                        sprintf(errStr, "\n%s:%x is already a peer",
                            configFile->inputStrings[i], peerAddr);
                        ERROR_ReportWarning(errStr);
                        continue;
                    }

                    tempMsdpConnectionData = (MsdpConnectionData*)
                        MEM_malloc(sizeof(MsdpConnectionData));
                    memset(tempMsdpConnectionData,
                           0,
                           sizeof(MsdpConnectionData));
                    tempMsdpConnectionData->peerAddr = peerAddr;

                    // connect-source and remote-as are optional parameters
                    if (!currentLine.eof())
                    {
                        bool isSourceGiven = false;
                        Int32 interfaceIndex = 0;
                        PimData* pim = NULL;
                        while (!currentLine.eof())
                        {
                            currentLine >> token;
                            IO_ConvertStringToLowerCase(token);

                            if (!strcmp(token, "connect-source"))
                            {
                                if (!currentLine.eof())
                                {
                                    isSourceGiven = true;
                                    currentLine >> token;
                                    MsdpConvertStringToNodeAddress(
                                        token,
                                        &(tempMsdpConnectionData->
                                            interfaceAddr));

                                    if (!MAPPING_IsIpAddressOfThisNode(
                                                node,
                                                node->nodeId,
                                                tempMsdpConnectionData->
                                                    interfaceAddr))
                                    {
                                        char errStr[MAX_STRING_LENGTH];
                                        sprintf(errStr,
                                            "%s:\nInvalid connect-source",
                                            configFile->inputStrings[i]);
                                        ERROR_ReportError(errStr);
                                    }

                                    interfaceIndex =
                                      NetworkIpGetInterfaceIndexFromAddress(
                                        node,
                                        tempMsdpConnectionData->
                                            interfaceAddr);

                                    pim = (PimData*)
                                        ip->interfaceInfo[interfaceIndex]->
                                        multicastRoutingProtocol;

                                    if (pim == NULL ||
                                        (pim->modeType !=
                                            ROUTING_PIM_MODE_SPARSE &&
                                         pim->modeType !=
                                            ROUTING_PIM_MODE_SPARSE_DENSE))
                                    {
                                        char errStr[MAX_STRING_LENGTH];
                                        sprintf(errStr,
                                            "Node %u: pim is not "
                                            "enabled on interface %d",
                                            node->nodeId,
                                            interfaceIndex);
                                        ERROR_ReportError(errStr);
                                    }

                                    tempMsdpConnectionData->asIdOfPeer = 0;
                                }
                                else
                                {
                                    char errStr[MAX_STRING_LENGTH];
                                    sprintf(errStr,
                                        "%s:\nConnect-Source not specified",
                                        configFile->inputStrings[i]);
                                    ERROR_ReportError(errStr);
                                }
                            }
                            else if (!strcmp(token, "remote-as"))
                            {
                                if (currentLine.eof())
                                {
                                    char errStr[MAX_STRING_LENGTH];
                                    sprintf(errStr,
                                        "%s:\nRemote-AS ID not specified",
                                        configFile->inputStrings[i]);
                                    ERROR_ReportError(errStr);
                                }

                                char asIdToken[MAX_INPUT_FILE_LINE_LENGTH];
                                currentLine >> asIdToken;
                                if (IO_IsStringNonNegativeInteger(
                                        asIdToken))
                                {
                                    tempMsdpConnectionData->
                                            asIdOfPeer = atoi(asIdToken);
                                    if (!isSourceGiven)
                                    {
                                        tempMsdpConnectionData->
                                            interfaceAddr =
                                            GetDefaultIPv4InterfaceAddress(
                                                node);
                                    }
                                    break;
                                }
                                else
                                {
                                    char errStr[MAX_STRING_LENGTH];
                                    sprintf(errStr,
                                        "%s:\nRemote-AS ID is not valid",
                                        configFile->inputStrings[i]);
                                    ERROR_ReportError(errStr);
                                }
                            }
                            else
                            {
                                char errStr[BIG_STRING_LENGTH];
                                sprintf(errStr, "%s:\nEnter a valid command:"
                                                "Expected commands:"
                                                "\n"
                                                "msdp peer <peer-address>"
                                                "\n"
                                                "msdp peer <peer-address>"
                                                " connect-source "
                                                "<interfaces address> "
                                                "remote-as <remote-as-ID>"
                                                "\n"
                                                "msdp peer <peer-address>"
                                                " connect-source "
                                                "<interfaces address>"
                                                "\n"
                                                "msdp peer <peer-address>"
                                                " remote-as "
                                                "<remote-as-ID>\n",
                                                configFile->inputStrings[i]);
                                ERROR_ReportError(errStr);
                            }
                        }
                    }
                    // If optional parameters are not specified then assign
                    //  default values to the corresponding parameters
                    //  in connection data.
                    else
                    {
                        bool isPimEnabled = false;
                        for (Int32 i = 0; i < node->numberInterfaces; i++)
                        {
                            PimData* pim =
                                (PimData*) ip->interfaceInfo[i]->
                                    multicastRoutingProtocol;
                            if (ip->interfaceInfo[i]->multicastProtocolType
                                    == MULTICAST_PROTOCOL_PIM)
                            {
                                if (pim->modeType !=
                                        ROUTING_PIM_MODE_SPARSE ||
                                    pim->modeType !=
                                        ROUTING_PIM_MODE_SPARSE_DENSE)
                                {
                                    isPimEnabled = true;
                                    tempMsdpConnectionData->interfaceAddr =
                                        NetworkIpGetInterfaceAddress(
                                            node,
                                            i);
                                    break;
                                }
                            }
                        }

                        if (!isPimEnabled)
                        {
                            char errStr[MAX_STRING_LENGTH];
                            sprintf(errStr,
                                "Node %u: PIM Sparse or PIM Dense Sparse is"
                                " not enabled on any of the interfaces",
                                node->nodeId);
                            ERROR_ReportError(errStr);
                        }
                        tempMsdpConnectionData->asIdOfPeer = 0;
                    }

                    // Initialize rest of the parameters in connection data
                    tempMsdpConnectionData->state = MSDP_DISABLED;

                    tempMsdpConnectionData->incomingSaMessagesFilterData.
                        isEnabled = false;
                    tempMsdpConnectionData->outgoingSaMessagesFilterData.
                        isEnabled = false;
                    tempMsdpConnectionData->connectRetryTimer = NULL;
                    tempMsdpConnectionData->keepAliveTimer = NULL;
                    tempMsdpConnectionData->holdTimer = NULL;
                    tempMsdpConnectionData->threshholdTTL =
                                                    MSDP_DEFAULT_TTL_VALUE;

                    msdpData->connectionDataMap->insert(
                        pair<NodeAddress, MsdpConnectionData*>
                        (tempMsdpConnectionData->peerAddr,
                        tempMsdpConnectionData));

                    if (MSDP_DEBUG_CONF)
                    {
                        MsdpConnectionData* connectionData = NULL;
                        connectionData = MsdpGetConnctionDataByPeerAdd(
                                            msdpData,
                                            tempMsdpConnectionData->peerAddr);
                        printf("\nPeer Address = %u\n", connectionData->
                                                            peerAddr);
                        printf("Interface Address = %u\n", connectionData->
                                                            interfaceAddr);
                        printf("AS ID of Peer= %d\n", connectionData->
                                                        asIdOfPeer);
                    }
                }
                else if (!strcmp(token, "redistribute"))
                {
                    msdpData->saRedistributeData.isEnabled = true;

                    if (MSDP_DEBUG_CONF)
                    {
                        printf("SA Redistribute Filter enabled = TRUE\n");
                    }

                    // list is an optional parameters
                    if (!currentLine.eof())
                    {
                        Int32 list = 0;
                        bool isAlreadyPresent = false;
                        MsdpFilterList :: iterator it;
                        currentLine >> token;
                        IO_ConvertStringToLowerCase(token);

                        if (!strcmp(token, "list"))
                        {
                            if (currentLine.eof())
                            {
                                char errStr[MAX_STRING_LENGTH];
                                sprintf(errStr, "%s:\nRedistribute List "
                                                "Id not specified",
                                                configFile->inputStrings[i]);
                                ERROR_ReportError(errStr);

                            }
                            currentLine >> accessListIdToken;
                            accessListId = MsdpGetIdIfValidAccessList(node,
                                                    accessListIdToken);
                            list = accessListId;
                        }
                        else
                        {
                            char errStr[MAX_STRING_LENGTH];
                            sprintf(errStr, "%s:\nEnter a valid command:"
                                              "Expected commands:"
                                              "\n"
                                              "msdp redistribute "
                                              "list <list-Id> "
                                              "\n"
                                              "msdp redistribute\n",
                                              configFile->inputStrings[i]);
                            ERROR_ReportError(errStr);
                        }

                        if (msdpData->saRedistributeData.
                                    filterList == NULL)
                        {
                            msdpData->saRedistributeData.filterList =
                                new MsdpFilterList;
                        }

                        for (it = msdpData->saRedistributeData.
                                filterList->begin();
                            it != msdpData->saRedistributeData.
                                filterList->end(); it++)
                        {
                            if (*it == list)
                            {
                                isAlreadyPresent = true;
                                char errStr[MAX_STRING_LENGTH];
                                sprintf(errStr, "%s:\nSpecified list "
                                                "already exists\n",
                                                configFile->inputStrings[i]);
                                ERROR_ReportWarning(errStr);
                                break;
                            }
                        }

                        if (!isAlreadyPresent)
                        {
                            msdpData->saRedistributeData.filterList->
                                    push_back(list);
                            if (MSDP_DEBUG_CONF)
                            {
                                if (msdpData->
                                      saRedistributeData.filterList->size())
                                {
                                    printf("\nUpdated Redistribute List : ");
                                    for (it = msdpData->
                                                saRedistributeData.
                                                filterList->begin();
                                        it != msdpData->
                                                saRedistributeData.
                                                filterList->end();
                                        it++)
                                    {
                                        printf("%u\t", *it);
                                    }
                                    printf("\n");
                                }
                            }
                        }
                    }
                }
                else if (!strcmp(token, "sa-filter"))
                {
                    currentLine >> token;
                    IO_ConvertStringToLowerCase(token);
                    if (!strcmp(token, "in"))
                    {
                        MsdpFilterList :: iterator it;
                        if (currentLine.eof())
                        {
                            char errStr[MAX_STRING_LENGTH];
                            sprintf(errStr, "%s:\nsa-filter-in Peer Address"
                                            " not specified",
                                            configFile->inputStrings[i]);
                            ERROR_ReportError(errStr);
                        }
                        currentLine >> token;
                        MsdpConvertStringToNodeAddress(token,
                            &peerAddr);
                        connectionData = MsdpGetConnctionDataByPeerAdd(
                                                msdpData,
                                                peerAddr);
                        if (connectionData)
                        {
                            connectionData->incomingSaMessagesFilterData.
                                isEnabled = true;
                            if (MSDP_DEBUG_CONF)
                            {
                                printf("\nPeer Address = %u",
                                            connectionData->peerAddr);
                                printf("\nInterface Address = %u",
                                            connectionData->interfaceAddr);
                                printf("\nIncoming SA message filter "
                                            "enabled = TRUE\n");
                            }
                            if (!currentLine.eof())
                            {
                                currentLine >> token;
                                bool isAlreadyPresent = false;
                                IO_ConvertStringToLowerCase(token);

                                // list is an optional parameter
                                if (!strcmp(token, "list"))
                                {
                                    if (currentLine.eof())
                                    {
                                        char errStr[MAX_STRING_LENGTH];
                                        sprintf(errStr, "%s:\nsa-filter in "
                                            "List Id not specified",
                                            configFile->inputStrings[i]);
                                        ERROR_ReportError(errStr);

                                    }
                                    currentLine >> accessListIdToken;
                                    accessListId =
                                        MsdpGetIdIfValidAccessList(
                                                   node,
                                                   accessListIdToken);

                                    // sa-filter in uses extended IP list
                                    //  only i.e. it must be ranged from
                                    //  100 to 199
                                    if (!((accessListId >=
                                            ACCESS_LIST_MIN_EXTENDED) &&
                                            (accessListId <=
                                            ACCESS_LIST_MAX_EXTENDED)))
                                    {
                                        char errStr[MAX_STRING_LENGTH];
                                        sprintf(errStr, "%s:\nEnter valid "
                                            "access list ID: it must be"
                                            " ranged from 100 to 199",
                                            configFile->inputStrings[i]);
                                        ERROR_ReportError(errStr);
                                    }
                                    if (connectionData->
                                        incomingSaMessagesFilterData.
                                        filterList == NULL)
                                    {
                                        connectionData->
                                           incomingSaMessagesFilterData.
                                           filterList = new MsdpFilterList;
                                    }

                                    for (it = connectionData->
                                        incomingSaMessagesFilterData.
                                        filterList->begin();
                                        it < connectionData->
                                        incomingSaMessagesFilterData.
                                        filterList->end(); it++)
                                    {
                                        if (*it == accessListId)
                                        {
                                            isAlreadyPresent = true;
                                            char errStr[MAX_STRING_LENGTH];
                                            sprintf(errStr, "%s:\n"
                                               "Specified list "
                                               "already exists\n",
                                               configFile->inputStrings[i]);
                                            ERROR_ReportWarning(errStr);
                                            break;
                                        }
                                    }
                                    if (!isAlreadyPresent)
                                    {
                                        connectionData->
                                            incomingSaMessagesFilterData.
                                            filterList->
                                            push_back(accessListId);

                                        if (MSDP_DEBUG_CONF)
                                        {
                                            if (connectionData->
                                                incomingSaMessagesFilterData.
                                                    filterList->size())
                                            {
                                                printf("\nUpdated SA "
                                                  "Request Filter List : ");
                                                for (it = connectionData->
                                                            incomingSaMessagesFilterData.
                                                            filterList->begin();
                                                    it != connectionData->
                                                            incomingSaMessagesFilterData.
                                                            filterList->end();
                                                    it++)
                                                {
                                                    printf("%u\t", *it);
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    char errStr[MAX_STRING_LENGTH];
                                    sprintf(errStr, "%s:\nEnter a valid "
                                               "keyword: 'list' expected",
                                               configFile->inputStrings[i]);
                                    ERROR_ReportError(errStr);
                                }
                            }
                        }
                        else
                        {
                            char errStr[MAX_STRING_LENGTH];
                            sprintf(errStr,
                                "%s:\nPeering with %u does not exist",
                                configFile->inputStrings[i], peerAddr);
                            ERROR_ReportError(errStr);
                        }
                    }
                    else if (!strcmp(token, "out"))
                    {
                        if (currentLine.eof())
                        {
                            char errStr[MAX_STRING_LENGTH];
                            sprintf(errStr,"%s:\nsa-filter out Peer Address"
                                            " not specified",
                                            configFile->inputStrings[i]);
                            ERROR_ReportError(errStr);

                        }
                        currentLine >> token;
                        MsdpConvertStringToNodeAddress(token,
                                                      &peerAddr);
                        connectionData = MsdpGetConnctionDataByPeerAdd(
                                                        msdpData,
                                                        peerAddr);
                        if (connectionData)
                        {
                            connectionData->outgoingSaMessagesFilterData.
                                isEnabled = true;

                            if (MSDP_DEBUG_CONF)
                            {
                                printf("\nPeer Address = %u",
                                            connectionData->peerAddr);
                                printf("\nInterface Address = %u",
                                            connectionData->interfaceAddr);
                                printf("\nOutgoing SA messages filter "
                                            "enabled = TRUE\n");
                            }

                            // list is an optional parameter
                            if (!currentLine.eof())
                            {
                                currentLine >> token;
                                IO_ConvertStringToLowerCase(token);
                                if (!strcmp(token, "list"))
                                {
                                    if (currentLine.eof())
                                    {
                                        char errStr[MAX_STRING_LENGTH];
                                        sprintf(errStr, "%s:\nsa-filter out"
                                               "List Id not specified",
                                               configFile->inputStrings[i]);
                                        ERROR_ReportError(errStr);

                                    }

                                    currentLine >> accessListIdToken;
                                    accessListId =
                                        MsdpGetIdIfValidAccessList(
                                        node,
                                        accessListIdToken);

                                    // sa-filter out uses extended IP list
                                    //  only i.e. it must be ranged from
                                    //  100 to 199
                                    if (!((accessListId >=
                                            ACCESS_LIST_MIN_EXTENDED) &&
                                            (accessListId <=
                                            ACCESS_LIST_MAX_EXTENDED)))
                                    {
                                        char errStr[MAX_STRING_LENGTH];
                                        sprintf(errStr, "%s:\nEnter valid "
                                            "access list ID: it must be"
                                            "ranged from 100 to 199",
                                            configFile->inputStrings[i]);
                                        ERROR_ReportError(errStr);
                                    }

                                    MsdpFilterList
                                         :: iterator it;
                                    bool isAlreadyPresent = false;
                                    if (connectionData->
                                        outgoingSaMessagesFilterData.
                                        filterList == NULL)
                                    {
                                        connectionData->
                                           outgoingSaMessagesFilterData.
                                           filterList =
                                           new MsdpFilterList;
                                    }

                                    for (it = connectionData->
                                            outgoingSaMessagesFilterData.
                                            filterList->begin();
                                        it < connectionData->
                                            outgoingSaMessagesFilterData.
                                            filterList->end(); it++)
                                    {
                                        if (*it == accessListId)
                                        {
                                            isAlreadyPresent = true;
                                            char errStr[MAX_STRING_LENGTH];
                                            sprintf(errStr, "%s:\nSpecified"
                                               " list already exists\n",
                                               configFile->inputStrings[i]);
                                            ERROR_ReportWarning(errStr);
                                            break;
                                        }
                                    }
                                    if (!isAlreadyPresent)
                                    {
                                        connectionData->
                                            outgoingSaMessagesFilterData.
                                            filterList->
                                            push_back(accessListId);
                                        if (MSDP_DEBUG_CONF)
                                        {
                                            if (connectionData->
                                                outgoingSaMessagesFilterData.
                                                    filterList->size())
                                            {
                                                printf("\nUpdated SA "
                                                  "Request Filter List : ");
                                                for (it = connectionData->
                                                     outgoingSaMessagesFilterData.
                                                      filterList->begin();
                                                    it != connectionData->
                                                     outgoingSaMessagesFilterData.
                                                      filterList->end();
                                                    it++)
                                                {
                                                    printf("%u\t", *it);
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    char errStr[MAX_STRING_LENGTH];
                                    sprintf(errStr, "%s:\nEnter a valid "
                                               "keyword:'list' expected",
                                               configFile->inputStrings[i]);
                                    ERROR_ReportError(errStr);
                                }
                            }
                        }
                        else
                        {
                            char errStr[MAX_STRING_LENGTH];
                            sprintf(errStr,
                                "%s:\nPeering with %x does not exist",
                                configFile->inputStrings[i], peerAddr);
                            ERROR_ReportError(errStr);
                        }
                    }
                    else
                    {
                        char errStr[MAX_STRING_LENGTH];
                        sprintf(errStr,
                            "%s:\nSA Filter should be either in or out",
                            configFile->inputStrings[i]);
                        ERROR_ReportError(errStr);
                    }
                }
                else if (!strcmp(token, "ttl-threshold"))
                {
                    if (currentLine.eof())
                    {
                        char errStr[MAX_STRING_LENGTH];
                        sprintf(errStr, "%s:\nPeer Address not specified",
                            configFile->inputStrings[i]);
                        ERROR_ReportError(errStr);
                    }
                    currentLine >> token;
                    MsdpConvertStringToNodeAddress(token, &peerAddr);
                    connectionData = MsdpGetConnctionDataByPeerAdd(
                                            msdpData,
                                            peerAddr);
                    if (connectionData)
                    {
                        if (currentLine.eof())
                        {
                            char errStr[MAX_STRING_LENGTH];
                            sprintf(errStr,
                                "%s:\nTTL-Threshold value not specified",
                                configFile->inputStrings[i]);
                            ERROR_ReportError(errStr);
                        }
                        char ttlValue[MAX_INPUT_FILE_LINE_LENGTH];
                        currentLine >> ttlValue;
                        if (IO_IsStringNonNegativeInteger(ttlValue))
                        {
                            connectionData->threshholdTTL =
                                atoi(ttlValue);
                            if (MSDP_DEBUG_CONF)
                            {
                                printf("\nPeer Address = %u",
                                            connectionData->peerAddr);
                                printf("\nInterface Address = %u",
                                            connectionData->interfaceAddr);
                                printf("\nTTL threshold value = %d",
                                            connectionData->threshholdTTL);
                            }
                        }
                    }
                }
                else if (!strcmp(token, "mesh-group"))
                {
                    UInt32 meshId;
                    if (currentLine.eof())
                    {
                        char errStr[MAX_STRING_LENGTH];
                            sprintf(errStr, "%s:\nmesh id not specified",
                                configFile->inputStrings[i]);
                        ERROR_ReportError(errStr);
                    }
                    char meshIdToken[MAX_INPUT_FILE_LINE_LENGTH];
                    currentLine >> meshIdToken;
                    if (IO_IsStringNonNegativeInteger(meshIdToken))
                    {
                        meshId = (UInt32)atoi(meshIdToken);
                        NodeAddress peerAddr;
                        MsdpMeshDataMap::iterator meshDataMapIt;
                        MsdpMeshMemberList::iterator meshMemberIt;

                        if (currentLine.eof())
                        {
                            char errStr[MAX_STRING_LENGTH];
                            sprintf(errStr,
                                "%s:\nmesh Peer Address not specified",
                                configFile->inputStrings[i]);
                            ERROR_ReportError(errStr);
                        }
                        currentLine >> token;
                        MsdpConvertStringToNodeAddress(token, &peerAddr);
                        connectionData = MsdpGetConnctionDataByPeerAdd(
                                                msdpData,
                                                peerAddr);
                        if (connectionData)
                        {
                            if (msdpData->meshDataMap == NULL)
                            {
                                msdpData->meshDataMap = new MsdpMeshDataMap;
                            }
                            meshDataMapIt =
                                msdpData->meshDataMap->find(meshId);
                            if (meshDataMapIt !=
                                    msdpData->meshDataMap->end())
                            {
                                bool isFound = false;
                                for (meshMemberIt = meshDataMapIt->
                                        second->begin();
                                    meshMemberIt < meshDataMapIt->
                                        second->end();
                                    meshMemberIt++)
                                {
                                    if ((*meshMemberIt)->peerAddr ==
                                            connectionData->peerAddr)
                                    {
                                        isFound = true;
                                        char errStr[MAX_STRING_LENGTH];
                                        sprintf(errStr,
                                                "%s:\nSpecified peer %u "
                                                "already exists in the mesh"
                                                "\n",
                                                configFile->inputStrings[i],
                                                connectionData->peerAddr);
                                        ERROR_ReportWarning(errStr);
                                        break;
                                    }
                                }
                                if (!isFound)
                                {
                                    meshDataMapIt->
                                      second->push_back(connectionData);

                                    if (MSDP_DEBUG_CONF)
                                    {
                                        meshDataMapIt =
                                            msdpData->meshDataMap->
                                                        find(meshId);
                                        printf("\nUpdated mesh %d peers",
                                                    meshId);
                                        for (meshMemberIt = meshDataMapIt->
                                                            second->begin();
                                            meshMemberIt < meshDataMapIt->
                                                            second->end();
                                            meshMemberIt++)
                                        {
                                            printf("\nPeer Address = %u",
                                                (*meshMemberIt)->peerAddr);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                MsdpMeshMemberList* tempMeshMemberList =
                                    new MsdpMeshMemberList;
                                tempMeshMemberList->
                                            push_back(connectionData);
                                msdpData->meshDataMap->insert(
                                        pair<UInt32, MsdpMeshMemberList*>
                                        (meshId, tempMeshMemberList));

                                if (MSDP_DEBUG_CONF)
                                {
                                    meshDataMapIt = msdpData->meshDataMap->
                                                        find(meshId);
                                    if (meshDataMapIt != msdpData->
                                                        meshDataMap->end())
                                    {
                                        printf("\nNew mesh %d peer", meshId);
                                        meshMemberIt = meshDataMapIt->
                                                            second->begin();
                                        printf("\nPeer Address = %u",
                                                (*meshMemberIt)->peerAddr);
                                    }
                                }
                            }
                        }
                        else
                        {
                            char errStr[MAX_STRING_LENGTH];
                            sprintf(errStr,
                                "%s:\nPeering with %u does not exist\n",
                                configFile->inputStrings[i], peerAddr);
                            ERROR_ReportError(errStr);
                        }
                    }
                    else
                    {
                        char errStr[MAX_STRING_LENGTH];
                        sprintf(errStr, "%s:\nEnter a valid command: "
                                        "mesh-group <mesh-ID> <peer-address>",
                                        configFile->inputStrings[i]);
                        ERROR_ReportError(errStr);
                    }
                }
                else if (!strcmp(token, "default-peer"))
                {
                    char defaultPeerToken[MAX_INPUT_FILE_LINE_LENGTH];
                    NodeAddress defaultPeerAddr;
                    if (currentLine.eof())
                    {
                        char errStr[MAX_STRING_LENGTH];
                        sprintf(errStr,
                            "%s:\nPeer Address not specified",
                            configFile->inputStrings[i]);
                        ERROR_ReportError(errStr);
                    }
                    currentLine >> defaultPeerToken;
                    MsdpConvertStringToNodeAddress(defaultPeerToken,
                                                   &defaultPeerAddr);
                    connectionData = MsdpGetConnctionDataByPeerAdd(msdpData,
                                                           defaultPeerAddr);
                    if (connectionData)
                    {
                        if (msdpData->defaultPeerList)
                        {
                            bool isPresent = false;
                            MsdpDefaultPeerList :: iterator it;
                            for (it = msdpData->defaultPeerList->begin();
                                it != msdpData->defaultPeerList->end(); it++)
                            {
                                if (*it == defaultPeerAddr)
                                {
                                    isPresent = true;
                                    ERROR_ReportWarning("default peer already exists");
                                    break;
                                }
                            }
                            if (!isPresent)
                            {
                                msdpData->defaultPeerList->push_back(defaultPeerAddr);
                            }
                        }
                        else
                        {
                            msdpData->defaultPeerList = new MsdpDefaultPeerList;
                            msdpData->defaultPeerList->push_back(defaultPeerAddr);
                        }
                    }
                    else
                    {
                        ERROR_ReportError("Peering doesn't exist");
                    }
                }
                else
                {
                    char errStr[MAX_STRING_LENGTH];
                    sprintf(errStr, "%s:\nEnter a valid command: "
                                        "Expected keywords\n"
                                        "peer\n"
                                        "redistribute\n"
                                        "sa-filter\n"
                                        "mesh-group\n",
                                        configFile->inputStrings[i]);
                    ERROR_ReportError(errStr);
                }
            }
            else
            {
                char errStr[MAX_STRING_LENGTH];
                sprintf(errStr, "%s:\nEnter a valid keyword:"
                                    "'msdp' expected\n",
                                    configFile->inputStrings[i]);
                ERROR_ReportError(errStr);
            }
        }
    }

    if (!isPeerFound)
    {
        char errStr[MAX_STRING_LENGTH];
        sprintf(errStr, "Node %u: Peer configuration does"
                        " not exist in .msdp file",
                        node->nodeId);
        ERROR_ReportError(errStr);
    }
}

/*
 * NAME:        MsdpInit
 * PURPOSE:     Handling all initializations needed for Msdp.
 *              initializing the internal structure
 *              initializing neighboring info
 * PARAMETERS:  node - pointer to the node
 *              nodeInput - configuration information
 * RETURN:      None
 */
void
MsdpInit(Node* node, const NodeInput* nodeInput)
{
    char buf[MAX_STRING_LENGTH];
    BOOL wasFound = FALSE;
    clocktype timeVal = 0;
    NodeInput msdpInput;

    MsdpData* msdpData = (MsdpData*)
            MEM_malloc (sizeof(MsdpData));
    memset(msdpData, 0, sizeof (MsdpData));
    node->appData.msdpData = msdpData;

    msdpData->saAdvertisementPeriod = MSDP_SA_ADVERTISEMENT_PERIOD;
    msdpData->saRedistributeData.isEnabled = false;
    msdpData->isCacheEnabled = true;

    if (MSDP_DEBUG_CONF)
    {
        printf("In RoutingMSDPInit(...)\n");
    }

    IO_ReadTime(node->nodeId,
                ANY_ADDRESS,
                nodeInput,
                "MULTICAST-MSDP-HOLDTIME-PERIOD",
                &wasFound,
                &timeVal);

    if (wasFound)
    {
        if (timeVal >= MSDP_MINIMUM_HOLDTIME_PERIOD)
        {
            msdpData->holdTimePeriod = timeVal;
        }
        else
        {
            ERROR_ReportError("HoldTime-Period is not valid\n");
        }
    }
    else
    {
        msdpData->holdTimePeriod = MSDP_DEFAULT_HOLDTIME_PERIOD;
    }

    BOOL val = FALSE;
    IO_ReadBool(node->nodeId,
                  ANY_ADDRESS,
                  nodeInput,
                  "MULTICAST-MSDP-ENABLE-CACHING",
                  &wasFound,
                  &val);

    if (wasFound && !val)
    {
        msdpData->isCacheEnabled = false;
    }
    else
    {
        IO_ReadTime(node->nodeId,
                  ANY_ADDRESS,
                  nodeInput,
                  "MULTICAST-MSDP-SG-STATE-PERIOD",
                  &wasFound,
                  &timeVal);

        if (wasFound)
        {
            if (timeVal >= msdpData->saAdvertisementPeriod +
                              msdpData->holdTimePeriod)
            {
                msdpData->sgStatePeriod = timeVal;
            }
            else
            {
                char errStr[MAX_STRING_LENGTH];
                sprintf(errStr, "MULTICAST-MSDP-SG-STATE-PERIOD "
                                "should be greater than or equal "
                                "to (60 seconds (multicast MSDP SA "
                                "advertisement period) + "
                                "MULTICAST-MSDP-HOLDTIME-PERIOD)"
                                "for node: %d", node->nodeId);
                ERROR_ReportError(errStr);
            }
        }
        else if (msdpData->holdTimePeriod <= MSDP_DEFAULT_HOLDTIME_PERIOD)
        {
            msdpData->sgStatePeriod = msdpData->saAdvertisementPeriod +
                          MSDP_DEFAULT_HOLDTIME_PERIOD;
        }
        else
        {
            char errStr[MAX_STRING_LENGTH];
            sprintf(errStr, "MULTICAST-MSDP-SG-STATE-PERIOD "
                            "should be greater than or equal "
                            "to (60 seconds (multicast MSDP SA "
                            "advertisement period) + "
                            "MULTICAST-MSDP-HOLDTIME-PERIOD)"
                            "for node: %d", node->nodeId);
            ERROR_ReportError(errStr);
        }
    }

    IO_ReadTime(node->nodeId,
                  ANY_ADDRESS,
                  nodeInput,
                  "MULTICAST-MSDP-KEEPALIVE-PERIOD",
                  &wasFound,
                  &timeVal);

    if (wasFound)
    {
        if (timeVal < msdpData->holdTimePeriod
            && timeVal > MSDP_MINIMUM_KEEPALIVE_PERIOD)
        {
            msdpData->keepAlivePeriod = timeVal;
        }
        else
        {
            ERROR_ReportError("MULTICAST-MSDP-KEEPALIVE-PERIOD"
                              " is not valid");
        }
    }
    else
    {
        msdpData->keepAlivePeriod = MSDP_DEFAULT_KEEPALIVE_TIMER;
    }

    IO_ReadTime(node->nodeId,
                  ANY_ADDRESS,
                  nodeInput,
                  "MULTICAST-MSDP-CONNECT-RETRY-PERIOD",
                  &wasFound,
                  &timeVal);

    if (wasFound)
    {
        if (timeVal <= 0)
        {
            ERROR_ReportError("MULTICAST-MSDP-CONNECT-RETRY-PERIOD"
                              " is not valid");
        }
        msdpData->connectRetryPeriod = timeVal;
    }
    else
    {
        msdpData->connectRetryPeriod = MSDP_DEFAULT_CONNECT_RETRY_PERIOD;
    }

    IO_ReadCachedFile(node->nodeId,
                      ANY_ADDRESS,
                      nodeInput,
                      "MULTICAST-MSDP-CONFIG-FILE",
                      &wasFound,
                      &msdpInput);

    if (!wasFound)
    {
        ERROR_ReportError("The MSDP speaker config file "
                          "(MULTICAST-MSDP-CONFIG-FILE)"
                          " is not specified");
    }

    // Check if statistics to be printed at the end of simulation
    IO_ReadString(node->nodeId,
                  ANY_ADDRESS,
                  nodeInput,
                  "MULTICAST-MSDP-STATISTICS",
                  &wasFound,
                  buf);

    if (wasFound)
    {
        if (!strcmp(buf, "YES"))
        {
            msdpData->statsAreOn = true;
        }
        else
        {
            msdpData->statsAreOn = false;
        }
    }
    else
    {
        msdpData->statsAreOn = false;
    }

    RANDOM_SetSeed(msdpData->timerSeed,
                   node->globalSeed,
                   node->nodeId,
                   APP_MSDP);

    if (MSDP_DEBUG_CONF)
    {
        printf("MSDP Configuration Parameters:\n");
        printf("saAdvertisementPeriod = %" TYPES_64BITFMT "d\n",
                msdpData->saAdvertisementPeriod);
        printf("holdTimePeriod = %" TYPES_64BITFMT "d\n",
                msdpData->holdTimePeriod);
        printf("sgStatePeriod = %" TYPES_64BITFMT "d\n",
                msdpData->sgStatePeriod);
        printf("keepAlivePeriod = %" TYPES_64BITFMT "d\n",
                msdpData->keepAlivePeriod);
        printf("connectRetryPeriod = %" TYPES_64BITFMT "d\n",
                msdpData->connectRetryPeriod);
    }

    memset(&msdpData->stats, 0, sizeof(MsdpStat));
    clocktype delay = 0;
    msdpData->saAdvertisementTimer = NULL;

    MsdpReadConfigurationFile(node, &msdpInput, msdpData);
    if (msdpData->isCacheEnabled)
    {
        msdpData->saCache = new MsdpSACacheMap;
    }

    MsdpConnectionDataMap::iterator connectionDataMapIt;

    for (connectionDataMapIt =
        msdpData->connectionDataMap->begin();
        connectionDataMapIt !=
        msdpData->connectionDataMap->end();
        connectionDataMapIt++)
    {
        connectionDataMapIt->second->state = MSDP_INACTIVE;
        NodeAddress peer = connectionDataMapIt->first;
        if (connectionDataMapIt->second->interfaceAddr > peer)
        {
            delay = 0;
        }
        else if (connectionDataMapIt->second->interfaceAddr < peer)
        {
            delay = RANDOM_nrand(msdpData->timerSeed) % MSDP_RANDOM_TIMER;
        }
        else
        {
            char errStr[MAX_STRING_LENGTH];
            sprintf(errStr, "Peer address for node:%d is invalid\n.",
                node->nodeId);
            ERROR_ReportError(errStr);
        }

        Message* newMessage = NULL;
        newMessage = MESSAGE_Alloc(node,
                                   APP_LAYER,
                                   APP_MSDP,
                                   MSG_APP_MSDP_StartTimerExpired);

        MESSAGE_InfoAlloc(node, newMessage, sizeof(NodeAddress));

        memcpy(MESSAGE_ReturnInfo(newMessage),
               &peer,
               sizeof(NodeAddress));

        MESSAGE_Send(node, newMessage, delay);
    }
}

/*
 * NAME:        MsdpPrintStats
 * PURPOSE:     Printing statistical information for MSDP
 * PARAMETERS:  node - node which is printing the statistical information
 * RETURN:      None
 */
void
MsdpPrintStats(Node* node)
{
    char buf[MAX_STRING_LENGTH];
    MsdpData* msdpData = (MsdpData*) node->appData.msdpData;

    sprintf (buf, "Keep Alive Messages Sent = %u",
        msdpData->stats.keepAliveSent);
    IO_PrintStat(
        node,
        "Application",
        "MSDP",
        ANY_DEST,
        -1, // instance Id
        buf);

    sprintf (buf, "Keep Alive Messages Received = %u",
        msdpData->stats.keepAliveReceived);
    IO_PrintStat(
        node,
        "Application",
        "MSDP",
        ANY_DEST,
        -1, // instance Id
        buf);

    sprintf (buf, "SA Messages Originated = %u",
        msdpData->stats.saPacketsOriginated);
    IO_PrintStat(
        node,
        "Application",
        "MSDP",
        ANY_DEST,
        -1, // instance Id
        buf);

    sprintf (buf, "SA Messages Received = %u",
        msdpData->stats.saPacketsReceived);
    IO_PrintStat(
        node,
        "Application",
        "MSDP",
        ANY_DEST,
        -1, // instance Id
        buf);

    sprintf (buf, "SA Messages Discarded = %u",
        msdpData->stats.saPacketsDiscarded);
    IO_PrintStat(
            node,
            "Application",
            "MSDP",
            ANY_DEST,
            -1, // instance Id
            buf);

    sprintf (buf, "SA Messages Forwarded = %u",
        msdpData->stats.saPacketsForwarded);
    IO_PrintStat(
        node,
        "Application",
        "MSDP",
        ANY_DEST,
        -1, // instance Id
        buf);

    sprintf (buf, "SA Messages Advertised = %u",
        msdpData->stats.saPacketsAdvertised);
    IO_PrintStat(
        node,
        "Application",
        "MSDP",
        ANY_DEST,
        -1, // instance Id
        buf);

    sprintf (buf, "Encapsulated SA Forwarded = %u",
        msdpData->stats.encapsulatedSAForwarded);
    IO_PrintStat(
        node,
        "Application",
        "MSDP",
        ANY_DEST,
        -1, // instance Id
        buf);
}

/*
 * NAME:        MsdpFinalize
 * PURPOSE:     Finalize function for MSDP
 * PARAMETERS:  node - pointer to the node
 * RETURN:      None
 */
void
MsdpFinalize(Node* node)
{
    MsdpData* msdpData = (MsdpData*) node->appData.msdpData;
    MsdpConnectionData* connectionData = NULL;

    if (msdpData != NULL)
    {
        if (MSDP_DEBUG_FINALIZE)
        {
            printf("Routing informations at the end of the "
                "simulation\n");
        }

        if (msdpData->statsAreOn)
        {
            MsdpPrintStats(node);
        }
    }

    if (msdpData->saRedistributeData.filterList != NULL)
    {
        delete(msdpData->saRedistributeData.filterList);
    }

    MsdpConnectionDataMap::iterator connectionDataMapIt =
        msdpData->connectionDataMap->begin();
    for (; connectionDataMapIt != msdpData->connectionDataMap->end();
        connectionDataMapIt++)
    {
        connectionData = connectionDataMapIt->second;
        switch(connectionData->state)
        {
            case MSDP_ESTABLISHED:
            {
                MsdpProcessEstablishedState(node,
                                    NULL,
                                    msdpData,
                                    MSDP_TRANSPORT_PASSIVE_CONNECTION_CLOSED,
                                    connectionData);

                MsdpProcessEstablishedState(node,
                                    NULL,
                                    msdpData,
                                    MSDP_TRANSPORT_ACTIVE_CONNECTION_CLOSED,
                                    connectionData);
                break;
            }
        }
        if (connectionData->incomingSaMessagesFilterData.filterList != NULL)
        {
            delete(connectionData->incomingSaMessagesFilterData.filterList);
        }
        if (connectionData->outgoingSaMessagesFilterData.filterList != NULL)
        {
            delete(connectionData->outgoingSaMessagesFilterData.filterList);
        }
        MEM_free(connectionData);
    }

    if (msdpData->connectionDataMap != 0)
    {
        delete(msdpData->connectionDataMap);
    }

    if (msdpData->saCache != NULL)
    {
        MsdpSACacheMap::iterator saCacheMapIt;
        while (msdpData->saCache->size() != 0)
        {
            saCacheMapIt = msdpData->saCache->begin();
            MsdpSGEntryList sgEntryList;
            MsdpSACacheGetSGEntryListByRPAddress(node,
                                                 saCacheMapIt->first,
                                                 msdpData,
                                                 &(sgEntryList));
            MsdpSGEntryList::iterator sgEntryListIt = sgEntryList.begin();
            for (; sgEntryListIt != sgEntryList.end(); sgEntryListIt++)
            {
                MsdpSACacheExpireSAState(node,
                                         saCacheMapIt->first,
                                         *sgEntryListIt,
                                         msdpData);
                if (msdpData->saCache->size() == 0)
                {
                    break;
                }
            }
        }
        delete(msdpData->saCache);
    }

    if (msdpData->meshDataMap != NULL)
    {
        MsdpMeshDataMap::iterator meshDataMapIt =
            msdpData->meshDataMap->begin();
        while (meshDataMapIt != msdpData->meshDataMap->end())
        {
            MsdpMeshMemberList* tempMeshMemberList =
                meshDataMapIt->second;
            meshDataMapIt++;
            if (tempMeshMemberList != NULL)
            {
                delete(tempMeshMemberList);
            }
        }
        delete(msdpData->meshDataMap);
    }

    if (msdpData->defaultPeerList != NULL)
    {
        delete(msdpData->defaultPeerList);
    }

    MEM_free(msdpData);
}
