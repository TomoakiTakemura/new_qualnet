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
*  Name: multicast_msdp.h
*  Purpose: To simulate Protocol Multicast Source Discovery Protocol (MSDP)
*
 */

#ifndef MULTICAST_MSDP_H
#define MULTICAST_MSDP_H

#include <list>
#include "network_ip.h"
#include "main.h"
#include "clock.h"

#define MAX_ACCESS_LIST_ID_LENGTH                       4
#define MSDP_RANDOM_TIMER   2 * MICRO_SECOND

#define MSDP_MIN_HDR_LEN                                3
#define MSDP_MESSAGE_TYPE_SIZE                          1
#define MSDP_MESSAGE_LENGTH_SIZE                        2
#define MSDP_MESSAGE_ENTRY_COUNT_SIZE                   1
#define MSDP_MESSAGE_RP_ADDRESS_SIZE                    4
#define MSDP_MESSAGE_RESERVED_SIZE                      3
#define MSDP_MESSAGE_SPREFIX_LENGTH_SIZE                1
#define MSDP_MESSAGE_SG_ADDRESS_SIZE                    8

#define MSDP_SA_MESSAGE_MAX_SIZE                        9192
#define MSDP_SA_MESSAGE_MAX_Z                           765

#define MSDP_KEEP_ALIVE_TLV_LENGTH          (MSDP_MESSAGE_TYPE_SIZE +\
                                            MSDP_MESSAGE_LENGTH_SIZE)

#define MSDP_SOURCE_ACTIVE_HEADER_LENGTH    (MSDP_MESSAGE_TYPE_SIZE +\
                                            MSDP_MESSAGE_LENGTH_SIZE +\
                                            MSDP_MESSAGE_ENTRY_COUNT_SIZE +\
                                            MSDP_MESSAGE_RP_ADDRESS_SIZE)

#define MSDP_GPREFIX_LENGTH                             1

#define MSDP_SOURCE_ACTIVE_SG_ENTRY_LENGTH \
                            (MSDP_MESSAGE_RESERVED_SIZE +\
                            MSDP_MESSAGE_SPREFIX_LENGTH_SIZE +\
                            MSDP_MESSAGE_SG_ADDRESS_SIZE)

#define MSDP_SA_ADVERTISEMENT_PERIOD        (clocktype)(60 * SECOND)
#define MSDP_MINIMUM_HOLDTIME_PERIOD        (clocktype)(3 * SECOND)
#define MSDP_MINIMUM_KEEPALIVE_PERIOD       (clocktype)(1 * SECOND)
#define MSDP_DEFAULT_HOLDTIME_PERIOD        (clocktype)(75 * SECOND)
#define MSDP_DEFAULT_KEEPALIVE_TIMER        (clocktype)(60 * SECOND)
#define MSDP_DEFAULT_CONNECT_RETRY_PERIOD   (clocktype)(30 * SECOND)
#define MSDP_DEFAULT_TTL_VALUE              0
#define MSDP_DEFAULT_ACCESS_LIST_ID         0

#define MSDP_LIBNET_CKSUM_CARRY(x) \
    (x = (x >> 16) + (x & 0xffff), (~(x + (x >> 16)) & 0xffff))

#define MSDP_NO_MESHGROUP 0

// Enumeration for different states in MSDP connection state m/c.
enum MsdpStateType
{
    MSDP_DISABLED,
    MSDP_INACTIVE,
    MSDP_LISTEN,
    MSDP_CONNECTING,
    MSDP_ESTABLISHED
};

// Enumeration for different types of filters in MSDP
enum MsdpFilterType
{
    MSDP_REDISTRIBUTE,
    MSDP_SA_FILTER_IN,
    MSDP_SA_FILTER_OUT
};

//  1 - MSDP Start (initiated by the system or operator)
//  2 - MSDP Transport Active connection open
//  3 - MSDP Transport Passive connection open
//  4 - ConnectRetry timer expired
//  5 - Disable MSDP peering
//  6 - MSDP TLV format error
//  7 - Hold Timer expired
//  8 - KeepAlive timer expired
//  9 - SA-Advertisement-Timer expired
//  10 - Receive KEEPALIVE message
//  11 - Receive Source-Active messages

enum MsdpEvent
{
    MSDP_START,
    MSDP_TRANSPORT_ACTIVE_CONNECTION_OPEN,
    MSDP_TRANSPORT_PASSIVE_CONNECTION_OPEN,
    MSDP_TRANSPORT_ACTIVE_CONNECTION_CLOSED,
    MSDP_TRANSPORT_PASSIVE_CONNECTION_CLOSED,
    MSDP_CONNECTRETRY_TIMER_EXPIRED,
    MSDP_HOLD_TIMER_EXPIRED,
    MSDP_TLV_FORMAT_ERROR,
    MSDP_KEEPALIVE_TIMER_EXPIRED,
    MSDP_SEND_SA_MESSAGE,
    MSDP_SA_ADVERTISEMENT_TIMER_EXPIRED,
    MSDP_CACHE_SA_STATE_TIMER_EXPIRED,
    MSDP_RECEIVE_KEEPALIVE_MESSAGE,
    MSDP_RECEIVE_SOURCE_ACTIVE_MESSAGE
};

enum MsdpMessageType
{
    MSDP_SOURCE_ACTIVE = 1,
    MSDP_KEEPALIVE = 4
};

// timers
enum MsdpTimersType
{
    MSDP_CONNECTRETRY_TIMER,
    MSDP_HOLD_TIMER,
    MSDP_KEEPALIVE_TIMER,
    MSDP_SA_ADVERTISEMENT_TIMER,
    MSDP_CACHE_SA_STATE_TIMER
};

// errors
enum MsdpError
{
    MSDP_NO_ERROR,
    MSDP_INVALID_INFO,
    MSDP_CACHE_STATE_ALREADY_EXIST,
    MSDP_CACHE_STATE_DONT_EXIST,
    MSDP_CACHE_INSERT_CACHE_DISABLED
};


typedef std :: vector<Int32>     MsdpFilterList;

struct MsdpFilter
{
    bool isEnabled;
    MsdpFilterList* filterList;
};

// Sourge-Group Entry
struct MsdpSGEntry
{
    NodeAddress     srcAddr;
    NodeAddress     groupAddr;
};

struct MsdpSAState
{
    MsdpSGEntry     sgEntry;
    Message*        saStateTimer;
};

typedef pair<NodeAddress, NodeAddress>          MsdpSGEntryPair;
typedef std :: map<MsdpSGEntryPair, MsdpSAState*>      MsdpSAStateMap;

struct MsdpCacheSAItem
{
    NodeAddress     rpAddr;
    MsdpSAStateMap* saStateMap;
};

struct MsdpCacheSAStateTimerInfo
{
    NodeAddress     rpAddr;
    MsdpSGEntry     sgEntry;
};

// Buffer used to reassemble raw transport packet
typedef struct struct_msdp_msg_buffer
{
    unsigned char* data;
    Int32   currentSize;
    Int32   expectedSize;
} MsdpMessageBuffer;

typedef std :: map<NodeAddress, MsdpCacheSAItem*>  MsdpSACacheMap;
typedef std :: vector<MsdpSGEntry>                 MsdpSGEntryList;

struct MsdpReceivedSAData
{
    NodeAddress rpAddr;
    MsdpSGEntryList sgEntryList;
    char* data;
    UInt32 dataLength;
};

// Structure to store connection specific data
struct MsdpConnectionData
{
    NodeAddress peerAddr;               // Address of the peer
    NodeAddress interfaceAddr;          // Interface Address
    unsigned short localPort;           // Port used
    unsigned short asIdOfPeer;          // AS ID of the peer
    unsigned short domainIdOfPeer;      // Domain ID of the peer
    Int32 connectionId;                   // Connection ID
    MsdpStateType state;                // Used to save the current state

    MsdpFilter incomingSaMessagesFilterData;
    MsdpFilter outgoingSaMessagesFilterData;
    MsdpMessageBuffer buffer;
    Int32 threshholdTTL;

    Message* connectRetryTimer;
    Message* keepAliveTimer;
    Message* holdTimer;
};

typedef std :: map<NodeAddress, MsdpConnectionData*>  MsdpConnectionDataMap;

typedef std :: vector<MsdpConnectionData*>            MsdpMeshMemberList;
typedef std :: map<UInt32, MsdpMeshMemberList*>       MsdpMeshDataMap;
typedef std :: vector<NodeAddress>                    MsdpDefaultPeerList;

struct MsdpStat
{
    UInt32 peers;
    UInt32 saPacketsReceived;
    UInt32 saPacketsDiscarded;
    UInt32 saPacketsOriginated;
    UInt32 saPacketsForwarded;
    UInt32 saPacketsAdvertised;
    UInt32 keepAliveSent;
    UInt32 keepAliveReceived;
    UInt32 encapsulatedSAForwarded;
};

// Structure used to store common data of node
struct MsdpData
{
    bool isCacheEnabled;
    clocktype   saAdvertisementPeriod;
    clocktype   holdTimePeriod;
    clocktype   sgStatePeriod;
    clocktype   keepAlivePeriod;
    clocktype   connectRetryPeriod;

    RandomSeed  timerSeed;
    MsdpFilter saRedistributeData;
    MsdpConnectionDataMap* connectionDataMap;
    MsdpSACacheMap* saCache;

    Message* saAdvertisementTimer;
    MsdpStat stats;
    MsdpMeshDataMap* meshDataMap;
    MsdpDefaultPeerList* defaultPeerList;

    bool statsAreOn;
};

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
MsdpInit(Node* node, const NodeInput* nodeInput);

/*
 * NAME:        AppLayerMSDP.
 * PURPOSE:     Models the behaviour of MSDP Speaker on receiving the
 *              message encapsulated in msg.
 * PARAMETERS:  node - pointer to the node which received the message.
 *              msg - message received by the layer
 * RETURN:      None.
 */
void
MsdpLayer(Node *node, Message *msg);

/*
 * NAME:        MsdpFinalize
 * PURPOSE:     Finalize function for MSDP
 * PARAMETERS:  node - pointer to the node
 * RETURN:      None
 */
void
MsdpFinalize(Node* node);

#endif // MULTICAST_MSDP_H
