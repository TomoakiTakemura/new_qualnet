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

/// \defgroup Package_APP_UTIL APP_UTIL

/// \file
/// \ingroup Package_APP_UTIL
/// This file describes Application Layer utility functions.

#ifndef _APP_UTIL_H_
#define _APP_UTIL_H_


#include <set>

#include "types.h"
#include "app_superapplication.h"
#include "api.h"
#include "app_mdp.h"

#ifdef WIN32
#ifdef ADDON_DB
// Causes compile errors on windows
// Can be removed when tcp/udp send functions are cleaned up
//#include "dbapi.h"
#endif
#endif

#ifdef ADDON_DB
class StatsDBAppEventParam;
//#include "dbapi.h"
#endif

enum PortType {
    START_NONE   = 0,
    START_EVEN   = 1,
    START_ODD    = 2
};

/// TTL default value.
/// Used in TCP/UDP app layer TTL
#define IPDEFTTL 64

/// Epsilon value for comparing two (2) doubles
///
/// \remark Used in app_dos.cpp and app_jammer.cpp
#define APP_DBL_EPSILON 1e-9

/// Maximum HID length value
///
/// \remark Maximum length of HID can't be greater than 50.
/// HID is constructed using 36 characters long uuid + 2 characters
/// for client identiofication + "-" character + integer seq number(max 10 
// character long in case of long integer) + null character
#define MAX_HID_LENGTH 55

enum AppMsgStatus {
    APP_MSG_NEW, // valid and new
    APP_MSG_OLD, // duplicate or out of order
    APP_MSG_UNKNOWN, // I don't know. Use seq cache to detect
};

enum AppMsgType {
    APP_MSGTYP_OTHER, // Neither TCP/UDP APP message
    APP_MSGTYP_TCP,
    APP_MSGTYP_UDP
};

/// Implementation of sequence number cache. It maintains a
/// list of sequence numbers in order to detect whether a
/// newly arrived message is duplicated or out-of-order.
class SequenceNumber
{
    private:
        // private initialization function primarily for supporting multiple
        // constructors. We cannot call another constructor from one
        // constructor.
        void InitializeValues(int sizeLimit, Int64 lowestSeqNotRcvd);

    protected:
        std::set<Int64> m_SeqCache; // To store sequence numbers
        int m_SizeLimit;   // how many elements can be stored in the cache,
                           // 0 means unlimit. Must be 0 or larger than 1
        Int64 m_LowestSeqNotRcvd; // The lowest sequence number have not
                                  // received so far
        Int64 m_HighestSeqRcvd;   // Highest sequence number received so far
                                  // if cache is not empty. When cache is
                                  // empty, it is same as m_LowestSeqNotRcvd.

    public:
        // Status of a sequence number
        enum Status
        {
            SEQ_NEW,          // A new sequence number not seen before
            SEQ_DUPLICATE,    // This sequence number has been seen before
            SEQ_OUT_OF_ORDER  // This sequence number not seen before, but
                              // larger ones have been received. Thus this is
                              // an out of order arrival
        };

        // constructors
        SequenceNumber(); // A constructor without parameter. m_SizeLimit
                          // will be 0 (unlimit). Initial m_LowestSeqNotRcvd
                          // will be 0.

        // A constructor with size limit specified. Initial value of
        // m_LowestSeqNotRcvd will be 0
        SequenceNumber(int sizeLimit);

        // A constructor with size limit and lowest sequence number passed in
        SequenceNumber(int sizeLimit, Int64 lowestSeqNotRcvd);


        // The destructor. Does nothing.
        virtual ~SequenceNumber() {};

        // major interface function. For the specified seqNumber, return
        // whether it is new or duplicate or out-of-order. In addition, it
        // will be inserted into the cache.
        Status Insert(Int64 seqNumber);

        Status Insert(Int64 seqNumber, AppMsgStatus msgStatus);

        // For debug purpose, print out elements in the cache as well as
        // values of member variables.
        void Print(FILE* out);
};

/// Insert a new application into the list of apps
/// on this node.
///
/// \param node  node that is registering the application.
/// \param appType  application type
/// \param dataPtr  pointer to the data space for this app
///
/// \return pointer to the new AppInfo data structure
/// for this app
AppInfo *
APP_RegisterNewApp(Node *node, AppType appType, void *dataPtr);

/// Set a new App Layer Timer and send to self after delay.
///
/// \param node  node that is issuing the Timer.
/// \param appType  application type
/// \param connId  if applicable, the TCP connectionId for this timer
/// \param sourcePort  the source port of the application setting
///    this timer
/// \param timerType  an integer value that can be used to
///    distinguish between types of timers
/// \param delay  send the timer to self after this delay.
void
APP_SetTimer(Node *node, AppType appType, int connId,
             unsigned short sourcePort, int timerType, clocktype delay);

/// Get the timerType for a received App Layer Timer.
#define APP_GetTimerType(x)   (((AppTimer *) (MESSAGE_ReturnInfo(x)))->type)


/// Allocate data and send to UDP.
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddPayload, and APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param appType  application type, to be used as destination port.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data is sent to.
/// \param payload  pointer to the data.
/// \param payloadSize  size of the data in bytes.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application used for
///    packet tracing.
DEPRECATED
Message *
APP_UdpSendNewData(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    char *payload,
    int payloadSize,
    clocktype delay,
    TraceProtocolType traceProtocol);

#ifdef EXATA
/// Allocate data and send to UDP.
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddPayload,
///   APP_SetIsEmulationPacket, and APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param appType  application type, to be used as destination port.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data is sent to.
/// \param payload  pointer to the data.
/// \param payloadSize  size of the data in bytes.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application used for
///                       packet tracing.
/// \param isEmulationPacket  if it's emulation packet
DEPRECATED
void
APP_UdpSendNewData(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    char *payload,
    int payloadSize,
    clocktype delay,
    TraceProtocolType traceProtocol,
    BOOL isEmulationPacket);
#endif


/// Allocate data with specified priority and send to UDP.
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddPayload, and APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param appType  application type, to be used as
///    destination port.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param outgoingInterface  interface used to send data.
/// \param payload  pointer to the data.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
/// \param isMdpEnabled  specify whether MDP is enabled.
/// \param uniqueId  specify uniqueId related to MDP.
/// \param mdpDataObjectInfo  specify the mdp data object info if any.
/// \param mdpDataInfosize  specify the mdp data info size.
DEPRECATED
Message *
APP_UdpSendNewDataWithPriority(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    int outgoingInterface,
    char *payload,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    BOOL isMdpEnabled = FALSE,
    Int32 uniqueId = -1,
    int regionId = -2,
    const char* mdpDataObjectInfo = NULL,
    unsigned short mdpDataInfosize = 0
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

/// Allocate data with specified priority and send to UDP
/// (For IPv6).
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddPayload, and APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param appType  application type, to be used as
///    destination port.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param outgoingInterface  interface used to send data.
/// \param payload  pointer to the data.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
/// \param isMdpEnabled  specify whether MDP is enabled.
/// \param uniqueId  specify uniqueId related to MDP.
///
/// \return The sent message
DEPRECATED
Message*
APP_UdpSendNewDataWithPriority(
    Node *node,
    AppType appType,
    Address sourceAddr,
    short sourcePort,
    Address destAddr,
    int outgoingInterface,
    char *payload,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    BOOL isMdpEnabled = FALSE,
    Int32 uniqueId = -1
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );


/// Allocate header and data and send to UDP..
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddHeader, APP_AddPayload,
///   and APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param appType  application type, to be used as
///    destination port.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param header  header of the payload.
/// \param headerSize  size of the header.
/// \param payload  pointer to the data.
/// \param payloadSize  size of the data in bytes.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
DEPRECATED
void
APP_UdpSendNewHeaderData(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    char *header,
    int headerSize,
    char *payload,
    int payloadSize,
    clocktype delay,
    TraceProtocolType traceProtocol);

/// Allocate header and data with specified priority
/// and send to UDP
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddHeader, APP_AddPayload, and
///   APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param appType  application type, to be used as
///    destination port.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param outgoingInterface  interface used to send data.
/// \param header  header of the payload.
/// \param headerSize  size of the header.
/// \param payload  pointer to the data.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
DEPRECATED
void
APP_UdpSendNewHeaderDataWithPriority(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    int outgoingInterface,
    char *header,
    int headerSize,
    char *payload,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

/// Allocate header + virtual data with specified priority
/// and send to UDP
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddHeader,
///   APP_AddVirtualPayload, and APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param appType  application type, to be used as
///    destination port.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param header  header of the payload.
/// \param headerSize  size of the header.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
/// \param isMdpEnabled  status of MDP layer.
/// \param mdpUniqueId  unique id for MPD session.
DEPRECATED
Message *
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    BOOL isMdpEnabled = FALSE,
    Int32  mdpUniqueId = -1
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

/// Allocate header + virtual data with specified priority
/// and send to UDP. Data is sent to a non-default destination
/// port (port number may not have same value as the AppType).
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddHeader,
///   APP_AddVirtualPayload, and APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param destinationPort  the destination port
/// \param infoData  UDP header to be added in info.
/// \param infoSize  size of the UDP header.
/// \param infoType  info type of the UDP header.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
/// \param isMdpEnabled  status of MDP layer.
/// \param mdpUniqueId  unique id for MPD session.
DEPRECATED
void
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    unsigned short destinationPort,
    char *infoData,
    int infoSize,
    unsigned short infoType,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    BOOL isMdpEnabled = FALSE,
    Int32 mdpUniqueId = -1
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

/// (Overloaded for IPv6) Allocate header + virtual
/// data with specified priority and send to UDP.
/// Data is sent to a non-default destination port
/// (port number may not have same value as the AppType).
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddHeader,
///   APP_AddVirtualPayload, APP_AddInfo, and APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param destinationPort  the destination port
/// \param infoData  UDP header to be added.
/// \param infoSize  size of the UDP header.
/// \param infoType  info type of the UDP header
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
/// \param appname  Application name.
/// \param isMdpEnabled  status of MDP layer.
/// \param mdpUniqueId  unique id for MPD session.
///
/// \return The message that was sent
DEPRECATED
void
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    unsigned short destinationPort,
    char *infoData,
    int infoSize,
    unsigned short infoType,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    char *appName,
    BOOL isMdpEnabled = FALSE,
    Int32 mdpUniqueId = -1
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

/// (Overloaded for IPv6) Allocate header + virtual
/// data with specified priority and send to UDP.
/// Data is sent to a non-default destination port
/// (port number may not have same value as the AppType).
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddHeader,
///    APP_AddVirtualPayload, APP_AddInfo, and APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param destinationPort  the destination port
/// \param header  header of the payload.
/// \param headerSize  size of the header.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
/// \param infoData  UDP header to be added.
/// \param infoSize  size of the UDP header.
/// \param infoType  info type of the UDP header
/// \param isMdpEnabled  status of MDP layer.
/// \param mdpUniqueId  unique id for MPD session.
DEPRECATED
void
APP_UdpSendNewHeaderVirtualDataWithPriority(
                        Node *node,
                        NodeAddress sourceAddr,
                        unsigned short sourcePort,
                        NodeAddress destAddr,
                        unsigned short destinationPort,
                        char *header,
                        int headerSize,
                        int payloadSize,
                        TosType priority,
                        clocktype delay,
                        TraceProtocolType traceProtocol,
                        char *appName,
                        char *infoData,
                        int infoSize,
                        unsigned short infoType,
                        BOOL isMdpEnabled = FALSE,
                        Int32 mdpUniqueId = -1
#ifdef ADDON_DB
                        ,
                        StatsDBAppEventParam* appParam = NULL
#endif
                        );

/// Allocate header + virtual data with specified priority
/// and send to UDP. Data is sent to a non-default destination
/// port (port number may not have same value as the AppType).
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddHeader,
///    APP_AddVirtualPayload, and APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param destinationPort  the destination port
/// \param header  header of the payload.
/// \param headerSize  size of the header.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
///
/// \return The created message
DEPRECATED
Message*
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    unsigned short destinationPort,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );


/// \deprecated \p appName is not used in the current
///   implementation, so this function is
///   no different from the basic operation.
///   Use APP_UdpCreateMessage, APP_AddHeader,
///   APP_AddVirtualPayload, and APP_UdpSend.
DEPRECATED
void
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    unsigned short destinationPort,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    char *appName
#ifdef ADDON_DB
    , StatsDBAppEventParam* appParam
#endif
    );


/// (Overloaded for MDP) Allocate actual + virtual
/// data with specified priority and send to UDP.
/// Data is sent to a non-default destination port
/// (port number may not have same value as the AppType).
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddHeader,
///    APP_AddVirtualPayload, APP_UdpCopyMdpInfo, and APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param appType  specify the application type.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node on which
///    data is sent to.
///    .+ destPort       : short             : the destination port.
/// \param header  pointer to the payload.
/// \param headerSize  size of the payload.
/// \param payloadSize  size of the virtual data in bytes.
/// \param priority  priority of data.
/// \param delay  send the data after this delay.
/// \param mdpInfo  persistent info for Mdp.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
/// \param theMsgPtr  pointer the original message to copy
///    the other persistent info.
///    Default value is NULL.
DEPRECATED
void
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    AppType appType,
    Address sourceAddr,
    unsigned short sourcePort,
    Address destAddr,
    unsigned short destPort,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    char* mdpInfo,
    TraceProtocolType traceProtocol,
    Message* theMsgPtr = NULL
#ifdef ADDON_DB
    ,
    BOOL fragIdSpecified = FALSE,
    Int32 fragId = -1
#endif
    );

/// (Overloaded for IPv6) Allocate header + virtual
/// data with specified priority and send to UDP.
/// Data is sent to a non-default destination port
/// (port number may not have same value as the AppType).
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddHeader,
///    APP_AddVirtualPayload, and APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param destinationPort  the destination port
/// \param header  header of the payload.
/// \param headerSize  size of the header.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
DEPRECATED
void
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    unsigned short destinationPort,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    char *appName
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

/// (Overloaded for IPv6) Allocate header + virtual
/// data with specified priority and send to UDP.
/// Data is sent to a non-default destination port
/// (port number may not have same value as the AppType).
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddHeader,
///    APP_AddVirtualPayload, and APP_UdpSend
///
/// \param node  node that is sending the data.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param destinationPort  the destination port
/// \param header  header of the payload.
/// \param headerSize  size of the header.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
DEPRECATED
Message *
APP_UdpSendNewHeaderVirtualDataWithPriority(
                        Node *node,
                        NodeAddress sourceAddr,
                        unsigned short sourcePort,
                        NodeAddress destAddr,
                        unsigned short destinationPort,
                        char *header,
                        int headerSize,
                        int payloadSize,
                        TosType priority,
                        clocktype delay,
                        TraceProtocolType traceProtocol,
                        char *appName,
                        SuperApplicationUDPDataPacket *data,
                        BOOL isMdpEnabled = FALSE,
                        Int32 mdpUniqueId = -1
#ifdef ADDON_DB
                        ,
                        StatsDBAppEventParam* appParam = NULL
#endif
                        );


/// Create the message.
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddHeader, and
///    APP_AddVirtualPayload
///
/// \param node  node that is sending the data.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param destinationPort  the destination port
/// \param header  header of the payload.
/// \param headerSize  size of the header.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
///
/// \return message created in the function
DEPRECATED
Message * APP_UdpCreateNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    short destinationPort,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    TraceProtocolType traceProtocol);




/// (Overloaded for IPv6) Allocate header + virtual
/// data with specified priority and send to UDP.
/// Data is sent to a non-default destination port
/// (port number may not have same value as the AppType).
///
/// \param node  node that is sending the data.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param destinationPort  the destination port
/// \param header  header of the payload.
/// \param headerSize  size of the header.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
/// \param isMdpEnabled  status of MDP layer.
/// \param mdpUniqueId  unique id for MPD session.

Message *
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    AppType appType,
    Address sourceAddr,
    unsigned short sourcePort,
    Address destAddr,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    BOOL isMdpEnabled = FALSE,
    Int32 mdpUniqueId = -1
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

/// (Overloaded for IPv6) Allocate header + virtual
/// data with specified priority and send to UDP.
/// Data is sent to a non-default destination port
/// (port number may not have same value as the AppType).
///
/// \param node  node that is sending the data.
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param destinationPort  the destination port
/// \param header  header of the payload.
/// \param headerSize  size of the header.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param endTime  zigbeeApp end time.
/// \param itemSize  zigbeeApp item size.
/// \param interval  zigbeeApp interval
/// \param delay  send the data after this delay.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
///
/// \return The created message

Message*
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    AppType appType,
    Address sourceAddr,
    unsigned short sourcePort,
    Address destAddr,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype endTime,
    UInt32 itemSize,
    D_Clocktype interval,
    clocktype delay,
    TraceProtocolType traceProtocol);

/// Listen on a server port.
///
/// \param node  Node pointer that the protocol is
///    being instantiated in
/// \param appType  which application initiates this request
/// \param serverAddr  server address
/// \param serverPort  server port number
void
APP_TcpServerListen(
    Node *node,
    AppType appType,
    NodeAddress serverAddr,
    short serverPort);

/// (Overloaded for IPv6) Listen on a server port.
///
/// \param node  Node pointer that the protocol is
///    being instantiated in
/// \param appType  which application initiates this request
/// \param serverAddr  server address
/// \param serverPort  server port number
void
APP_TcpServerListen(
    Node *node,
    AppType appType,
    Address serverAddr,
    short serverPort);

/// Listen on a server port with specified priority.
///
/// \deprecated This will be replaced by APP_TcpServerListen
///    with a non-default priority argument.
///
/// \param node  Node pointer that the protocol is
///    being instantiated in
/// \param appType  which application initiates this request
/// \param serverAddr  server address
/// \param serverPort  server port number
/// \param priority  priority of this data for
///    this session.
DEPRECATED
void
APP_TcpServerListenWithPriority(
    Node *node,
    AppType appType,
    NodeAddress serverAddr,
    short serverPort,
    TosType priority);

/// Listen on a server port with specified priority.
/// (Overloaded for IPv6)
///
/// \deprecated This will be replaced by APP_TcpServerListen
///    with a non-default priority argment.
///
/// \param node  Node pointer that the protocol is
///    being instantiated in
/// \param appType  which application initiates this request
/// \param serverAddr  server address
/// \param serverPort  server port number
/// \param priority  priority of this data for
///    this session.
DEPRECATED
void
APP_TcpServerListenWithPriority(
    Node *node,
    AppType appType,
    Address serverAddr,
    short serverPort,
    TosType priority);

/// Create the message.
///
/// \deprecated Replaced by APP_TcpCreateMessage that does not
///    include a payload.
///
/// \param node  Node pointer that the protocol is
///    being instantiated in
/// \param connId  connection id.
/// \param payload  data to send.
/// \param length  length of the data to send.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
///
/// \return message created in the function
DEPRECATED
Message * App_TcpCreateMessage(
    Node *node,
    int connId,
    char *payload,
    int length,
    TraceProtocolType traceProtocol,
    UInt8 ttl);

/// send an application data unit.
///
/// \deprecated Use APP_TcpCreateMessage, APP_AddPayload,
///    and APP_TcpSend.
///
/// \param node  Node pointer that the protocol is
///    being instantiated in
/// \param connId  connection id.
/// \param payload  data to send.
/// \param length  length of the data to send.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
DEPRECATED
Message *
APP_TcpSendData(
    Node *node,
    int connId,
    char *payload,
    int length,
    TraceProtocolType traceProtocol
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL,
    UInt8 ttl = IPDEFTTL);
#else
    ,UInt8 ttl = IPDEFTTL);
#endif

/// send an application data unit
///
/// \deprecated Use APP_TcpCreateMessage, APP_AddPayload,
///    APP_SetIsEmulationPacket, and APP_TcpSend.
///
/// \param node  Node pointer that the protocol is
///    being instantiated in
/// \param connId  connection id.
/// \param payload  data to send.
/// \param length  length of the data to send.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
/// \param isEmulationPacket  Emulation packet?
DEPRECATED
void
APP_TcpSendData(
    Node *node,
    int connId,
    char *payload,
    int length,
    TraceProtocolType traceProtocol,
    BOOL isEmulationPacket,
#ifdef ADDON_DB
    StatsDBAppEventParam* appParam = NULL,
#endif
    UInt8 ttl = IPDEFTTL);


/// \deprecated Use APP_TcpCreateMessage, APP_AddPayload,
/// and APP_TcpSend.
///
/// \note appName parameter is not implemented in the deprecated function.
DEPRECATED
void
APP_TcpSendData(
    Node *node,
    int connId,
    char *payload,
    int length,
    TraceProtocolType traceProtocol,
    char *appName,
#ifdef ADDON_DB
    StatsDBAppEventParam* appParam = NULL,
#endif
    UInt8 ttl = IPDEFTTL);

/// \deprecated Use APP_TcpCreateMessage, APP_AddPayload,
///    APP_AddInfo, and APP_TcpSend.
DEPRECATED
Message *
APP_TcpSendData(
    Node *node,
    int connId,
    char *payload,
    int length,
    char *InfoData,
    int infoSize,
    unsigned short infoType,
    TraceProtocolType traceProtocol,
#ifdef ADDON_DB
    StatsDBAppEventParam* appParam = NULL,
#endif
    UInt8 ttl = IPDEFTTL);

/// \deprecated Use APP_TcpCreateMessage, APP_AddPayload,
///                   APP_AddInfo, and APP_TcpSend.
DEPRECATED
Message *
APP_TcpSendData(
    Node *node,
    int connId,
    char *payload,
    int length,
    char *infoData,
    int infoSize,
    unsigned short infoType,
    TraceProtocolType traceProtocol,
    unsigned short tos,
#ifdef ADDON_DB
    StatsDBAppEventParam* appParam,
#endif
    UInt8 ttl = IPDEFTTL);


/// \deprecated Use APP_TcpCreateMessage, APP_AddPayload,
/// and APP_TcpSend.
///
/// \note Most of the parameters
/// declared here are not implemented in the
/// deprecated code anyway.
DEPRECATED
Message *
APP_TcpSendData(
    Node *node,
    int connId,
    char *payload,
    int length,
    char *infoData,
    int infoSize,
    unsigned short infoType,
    TraceProtocolType traceProtocol,
    unsigned short tos,
    NodeAddress clientAddr,
    int sourcePort,
    NodeAddress serverAddr,
    int destinationPort,
    int sessionId,
    int reqSize,
    int requiredTput,
    clocktype requiredDelay,
    float dataRate,
#ifdef ADDON_DB
    StatsDBAppEventParam* appParam,
#endif
    UInt8 ttl);


/// Send header and virtual data using TCP.
///
/// \deprecated Use APP_TcpCreateMessage, APP_AddHeader,
///    APP_AddVirtualPayload, and APP_TcpSend.
///
/// \param node  Node pointer that the protocol is
///    being instantiated in
/// \param connId  connection id.
/// \param header  header to send.
/// \param headerLength  length of the header to send.
/// \param payloadSize  size of data to send along with header.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
///
/// \return Sent message
DEPRECATED
Message*
APP_TcpSendNewHeaderVirtualData(
    Node *node,
    int connId,
    char *header,
    int headerLength,
    int payloadSize,
    TraceProtocolType traceProtocol,
#ifdef ADDON_DB
    StatsDBAppEventParam* appParam = NULL,
#endif
    UInt8 ttl = IPDEFTTL);

/// Close the connection.
///
/// \param node  Node pointer that the protocol is
///    being instantiated in
/// \param connId  connection id.
void
APP_TcpCloseConnection(
    Node *node,
    int connId);

/// Start process of joining multicast group if need to do so.
///
/// \param node  node - node that is joining a group.
/// \param nodeInput  used to access configuration file.
void APP_InitMulticastGroupMembershipIfAny(
    Node *node,
    const NodeInput *nodeInput);

/// Application input parsing API. Parses the source and
/// destination strings.At the same time validates those
/// strings for multicast address.
///
/// \param node  A pointer to Node.
/// \param inputString  The input string.
/// \param sourceString  The source string.
/// \param sourceNodeId  A pointer to NodeAddress.
/// \param sourceAddr  A pointer to NodeAddress.
/// \param destString  The destination string.
/// \param destNodeId  A pointer to NodeAddress.
/// \param destAddr  A pointer to NodeAddress.
/// \param isDestMulticast  Pointer to multicast checking flag.
void APP_CheckMulticastByParsingSourceAndDestString(
    Node *node,
    const char *inputString,
    const char *sourceString,
    NodeAddress *sourceNodeId,
    NodeAddress *sourceAddr,
    const char *destString,
    NodeAddress *destNodeId,
    NodeAddress *destAddr,
    BOOL *isDestMulticast);

/// API to parse the input source and destination strings read
/// from the \c *.app file.  At the same time checks and fills the
/// destination type parameter.
///
/// \param node  A pointer to Node.
/// \param inputString  The input string.
/// \param sourceString  The source string.
/// \param sourceNodeId  A pointer to NodeAddress.
/// \param sourceAddr  A pointer to NodeAddress.
/// \param destString  The destination string.
/// \param destNodeId  A pointer to NodeAddress.
/// \param destAddr  A pointer to NodeAddress.
/// \param destType  A pointer to Destinationtype.
void APP_ParsingSourceAndDestString(
    Node* node,
    const char*  inputString,
    const char*  sourceString,
    NodeAddress* sourceNodeId,
    NodeAddress* sourceAddr,
    const char*  destString,
    NodeAddress* destNodeId,
    NodeAddress* destAddr,
    DestinationType* destType);

/// API to parse the input source and destination strings read
/// from the \c *.app file. At the same time checks and fills the
/// destination type parameter.
///
/// \param node  A pointer to Node.
/// \param inputString  The input string.
/// \param sourceString  The source string.
/// \param sourceNodeId  A pointer to NodeAddress.
/// \param sourceAddr  A pointer to NodeAddress.
/// \param destString  The destination string.
/// \param destNodeId  A pointer to NodeAddress.
/// \param destAddr  A pointer to NodeAddress.
/// \param destType  A pointer to DestinationType.
void APP_ParsingSourceAndDestString(
    Node* node,
    const char* inputString,
    const char* sourceString,
    NodeId* sourceNodeId,
    Address* sourceAddr,
    const char* destString,
    NodeId* destNodeId,
    Address* destAddr,
    DestinationType* destType);


/// Insert a new application into the list of apps on this node.
/// Also inserts the port number being used for this app in the
/// port table.
///
/// \param node  node that is registering the application.
/// \param appType  application type
/// \param dataPtr  pointer to the data space for this app
/// \param myPort  port number to be inserted in the port table
///
/// \return pointer to the new AppInfo data structure
AppInfo *
APP_RegisterNewApp(
    Node *node,
    AppType appType,
    void *dataPtr,
    unsigned short myPort);

/// Check if the port number is free or in use. Also check if
/// there is an application running at the node that uses
/// an AppType that has been assigned the same value as this
/// port number. This is done since applications such as CBR
/// use the value of AppType as destination port.
///
/// \param node  node that is checking it's port table
/// \param portNumber  port number to check
///
/// \return indicates if the port is free
BOOL APP_IsFreePort(
            Node* node,
            unsigned short portNumber);

/// Returns a free port
///
/// \param node  node that is requesting a free port
///
/// \return a free port
unsigned short APP_GetFreePort(Node *node);


short APP_GetFreePortForEachServer(Node *node, NodeAddress serverAddr);

/// Insert an entry in the port table
///
/// \param node  node that needs to be insert in port table
/// \param appType  application running at the port
/// \param myPort  port number to check
void APP_InserInPortTable(
    Node* node,
    AppType appType,
    unsigned short myPort);

/// Returns the protocol for which the message is destined
///
/// \param node  node that received the message
/// \param msg  pointer to the message received
/// \return protocol which will receive the message
unsigned short APP_GetProtocolType(
    Node* node,
    Message* msg);

/// Application input parsing API. Parses the tos string and
/// tos value strings.At the same time validates those
/// strings for proper ranges.
///
/// \param tosString  The tos string.
/// \param tosValString  The tos value string.
/// \param tosVal  A pointer to equivalent 8-bit TOS value.
BOOL APP_AssignTos(
    char tosString[],
    char tosValString[],
    unsigned *tosVal);

/// Allocate header + virtual
/// data with specified priority and send to UDP.
/// Generally used with messenger app.
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddHeader,
///    APP_AddVirtualPayload, and APP_UdpSend.
///
/// \param node  node that is sending the data.
/// \param appType  type of application data
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param header  header of the payload.
/// \param headerSize  size of the header.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
///
/// \return Pointer to allocated message structure
DEPRECATED
Message* APP_UdpCreateNewHeaderVirtualDataWithPriority(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    TraceProtocolType traceProtocol);

/// Allocate header + virtual
/// data with specified priority and send to UDP.
/// Generally used with messenger app.
///
/// \deprecated Use APP_UdpCreateMessage, APP_AddHeader,
///    APP_AddVirtualPayload, and APP_UdpSend.
///
/// \param node  node that is sending the data.
/// \param appType  type of application data
/// \param sourceAddr  the source sending the data.
/// \param sourcePort  the application source port.
/// \param destAddr  the destination node Id data
///    is sent to.
/// \param header  header of the payload.
/// \param headerSize  size of the header.
/// \param payloadSize  size of the data in bytes.
/// \param priority  priority of data.
/// \param traceProtocol  specify the type of application
///    used for packet tracing.
///
/// \return Pointer to allocated message structure
DEPRECATED
Message* APP_UdpCreateNewHeaderVirtualDataWithPriority(
    Node *node,
    AppType appType,
    const Address& sourceAddr,
    short sourcePort,
    const Address& destAddr,
    const char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    TraceProtocolType traceProtocol);

/// Remove an application from list of apps
/// on this node.
///
/// \param node  node that is unregistering the application.
/// \param dataPtr  pointer to the data space for this app.
/// \param freeData  if true, free (via MEM_free) the dataPtr
void
APP_UnregisterApp(
    Node *node,
    void *dataPtr,
    bool freeData = true);

/// Remove an application from the list of apps on this node.
/// Also Remove the port number being used for this app in the
/// port table.
///
/// \param node  node that is registering the application.
/// \param appType  application type
/// \param dataPtr  pointer to the data space for this app
/// \param myPort  port number to be inserted in the port table
void
APP_UnregisterApp(
    Node *node,
    void *dataPtr,
    unsigned short myPort);

/// Check if the port number is free or in use. Also check if
/// there is an application running at the node that uses
/// an AppType that has been assigned the same value as this
/// port number. This is done since applications such as CBR
/// use the value of AppType as destination port.
///
/// \param node  node that is checking it's port table.
/// \param portNumber  port number to check.
///
/// \return indicates if the port is free.
BOOL APP_IsFreePort(
    Node* node,
    unsigned short portNumber,
    AppType appType);

/// Delete an entry from the port table
///
/// \param node  node that needs to be remove from port table
/// \param myPort  port number to check
void APP_RemoveFromPortTable(
    Node* node,
    unsigned short myPort);

short GetFreePort(
    Node *node,
    PortType first_port,
    int num_of_nodes);

BOOL IsNextPortFree(Node *node,
                    unsigned short sourcePort,
                    int num_of_nodes);

short GetFreePort(Node *node,PortType first_port,int num_of_nodes);
BOOL IsNextPortFree(Node *node,unsigned short sourcePort,int num_of_nodes);



#ifdef ADDON_DB
/// Report receive event to StatsDB app event table
/// This function will check duplicate and out of order msgs
///
/// \param node  Pointer to a node who recieves the msg
/// \param msg  The received message or fragment
/// \param seqCache  Pointer to the sequence number cache
///    which is used to detect duplicate
/// \param seqNo  Sequence number of the message or fragment
/// \param delay  Delay of the message/fragment
/// \param jitter  Smoothed jitter of the received message
/// \param size  Size of msg/fragment to be report to db
/// \param numRcvd  # of msgs/frags received so far
/// \param msgStatus  This is for performance optimization. If
///    the app already know this is a new msg, it is
///    helpful when dup record and out of order
///    record are both disabled.
///    Note: The last four parameters are required by the stats DB API
///
/// \return Indicate whether the msg is dup
///    or out of order or new
SequenceNumber::Status APP_ReportStatsDbReceiveEvent(
                           Node* node,
                           Message* msg,
                           SequenceNumber **seqCache,
                           Int64 seqNo,
                           clocktype delay,
                           clocktype jitter,
                           int size,
                           int numRcvd,
                           AppMsgStatus msgStatus = APP_MSG_UNKNOWN);
#endif // ADDON_DB

/// Used to handle ICMP message received from transport layer
///
/// \param node  node that received the message.
/// \param sourcePort  Source port of original message which generated
///                          ICMP Error Message
/// \param destinationPort  Destination port of original message which
///                               generated ICMP Error Message
/// \param icmpType  ICMP Meassage Type
/// \param icmpCode   ICMP Message Code
void App_HandleIcmpMessage(
    Node *node,
    unsigned short sourcePort,
    unsigned short destinationPort,
    unsigned short icmpType,
    unsigned short icmpCode);


// REFACTORED API PROTOTYPES

/// Open a TCP connection. This overload is for IPv4
/// addressing.
///
/// \param node  Node pointer that the protocol is
///                             being instantiated in
/// \param appType  which application initiates this request
/// \param localAddr  address of the source node.
/// \param localPort  port number on the source node.
/// \param remoteAddr  address of the remote node.
/// \param remotePort  port number on the remote node (server port).
/// \param uniqueId  used to determine which client is requesting
///                             connection.
/// \param waitTime  time until the session starts.
/// \param priority  priority of the data.  Use APP_DEFAULT_TOS
///                             when no prioity is specified
/// \param outgoingInterface  User specific outgoing Interface. Use
///                           ANY_INTERFACE as the default value.

void
APP_TcpOpenConnection(
    Node *node,
    AppType appType,
    NodeAddress localAddr,
    short localPort,
    NodeAddress remoteAddr,
    short remotePort,
    int uniqueId,
    clocktype waitTime,
    TosType priority = APP_DEFAULT_TOS,
    int outgoingInterface = ANY_INTERFACE);

/// Open a TCP connection. This overload is for IPv6
/// addressing.
///
/// \param node  Node pointer that the protocol is
///                             being instantiated in
/// \param appType  which application initiates this request
/// \param localAddr  address of the source node.
/// \param localPort  port number on the source node.
/// \param remoteAddr  address of the remote node.
/// \param remotePort  port number on the remote node (server port).
/// \param uniqueId  used to determine which client is requesting
///                             connection.
/// \param waitTime  time until the session starts.
/// \param priority  priority of the data.  Use APP_DEFAULT_TOS
///                             when no prioity is specified
/// \param outgoingInterface  User specific outgoing Interface. Use
///                            ANY_INTERFACE as the default value.

void
APP_TcpOpenConnection(
    Node *node,
    AppType appType,
    const Address& localAddr,
    short localPort,
    const Address& remoteAddr,
    short remotePort,
    int uniqueId,
    clocktype waitTime,
    TosType priority = APP_DEFAULT_TOS,
    int outgoingInterface = ANY_INTERFACE);


/// Create a basic TCP message.
/// \param node  Node pointer that the protocol is
///                                       being instantiated in
/// \param connId  connection id.
/// \param traceProtocol  specify the type of application
///                                       used for packet tracing.
/// \param tos  Type of Service specification
///
/// \return message created in the function
Message*
APP_TcpCreateMessage(
    Node *node,
    int connId,
    TraceProtocolType traceProtocol,
    TosType tos = APP_DEFAULT_TOS);

/// Creates a basic, unmodified UDP message. This morph
/// supports the IPv4 type of addressing.
///
/// \param node            node that is sending the data
/// \param sourceAddr      the source sending the data
/// \param sourcePort      the application source port
/// \param destAddr        the node data is sent to
/// \param destinationPort the destination port; for
///                                        port based on AppType use
///                                        (short)appType.
/// \param traceProtocol   specify the type of application
///                                        used for packet tracing.
/// \param priority        priority of data (type of service)
/// \return  message created in the function

Message*
APP_UdpCreateMessage(
    Node *node,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    short destinationPort,
    TraceProtocolType traceProtocol,
    TosType priority = APP_DEFAULT_TOS);


/// Creates a basic, unmodified UDP message. This morph
/// supports the IPv6 type of addressing.
///
/// \param node            node that is sending the data
/// \param sourceAddr      the source sending the data
/// \param sourcePort      the application source port
/// \param destAddr        the node data is sent to
/// \param destinationPort the destination port; for
///                                        port based on AppType use
///                                        (short)appType.
/// \param traceProtocol   specify the type of application
///                                        used for packet tracing.
/// \param priority        priority of data (type of service)
/// \return           message created in the function

Message*
APP_UdpCreateMessage(
    Node *node,
    const Address& sourceAddr,
    short sourcePort,
    const Address& destAddr,
    short destinationPort,
    TraceProtocolType traceProtocol,
    TosType priority = APP_DEFAULT_TOS);


// These functions can be used to modify either TCP or UDP messages:

/// Add a header section to a TCP or UDP message.
/// \param node           Node pointer
/// \param msg            Message to be modified.
/// \param header         Pointer to header data
/// \param headerSize     length of header data
///                                       used for packet tracing.
void
APP_AddHeader(
    Node* node,
    Message* msg,
    const void* header,
    int headerSize);

/// Add an info section to a TCP or UDP message.
///
/// \param node           Node pointer
/// \param msg            Message to be modified.
/// \param infoData       Pointer to info data
/// \param infoSize       length of info data
/// \param infoType       info type of the header.
///                                       used for packet tracing.
void
APP_AddInfo(
    Node* node,
    Message* msg,
    const void* infoData,
    int infoSize,
    int infoType);

/// Add a payload section to a TCP or UDP message.
/// Normally the payload is accounted in the Message
/// object as header zero [0] and the numberOfHeaders
/// is set to 1. However, if the traceProtocol value
/// equals TRACE_UNDEFINED, numberOfHeaders will be
/// zero.
///
/// APP_AddPayload may not be called more than once
/// per message, and it may not be called after a call
/// to APP_AddHeader.
///
/// \param node           Node pointer
/// \param msg            Message to be modified.
/// \param payload        Pointer to payload data
/// \param payloadSize    length of payload data
///                                       used for packet tracing.
void
APP_AddPayload(
    Node* node,
    Message* msg,
    const void* payload,
    int payloadSize);

/// Add a virtual payload section to a TCP or
///                   UDP message.
///
/// \param node           Node pointer
/// \param msg            Message to be modified.
/// \param payloadSize    length of payload data
///                                       used for packet tracing.
inline
void
APP_AddVirtualPayload(
    Node* node,
    Message* msg,
    int payloadSize)
{
    MESSAGE_AddVirtualPayload(node, msg, payloadSize);
}

#ifdef EXATA
/// Sets the Emulation flag for the message.
///
/// \param node           Node pointer
/// \param msg            Message to be modified.

void
APP_SetIsEmulationPacket(
    Node* node,
    Message* msg);
#endif

// These functions are for use with TCP messages:

/// Sets the Time to Live value for a TCP message.
///
/// \param msg            Message to be modified.
/// \param ttl            Time to Live

void
APP_SetTtl(
    Message* msg,
    UInt8 ttl);

/// Sends the TCP message
///
/// \param node           Node pointer
/// \param msg            Message to be modified.
/// \param appParam       db info class

void
APP_TcpSend(
    Node* node,
    Message* msg
#ifdef ADDON_DB
    , StatsDBAppEventParam* appParam = NULL
#endif
    );


// These functions are for use with UDP messages:

/// Copies info from a previous MDP message, then
/// determines whether to use the destination port
/// supplied to APP_UdpCreate or to change it to
/// APP_MDP. This function is called by
/// MdpSession::OnTxTimeout
///
/// \param node             Node pointer
/// \param msg              Message to be modified.
/// \param mdpInfo          Info field for MDP
/// \param theMsgPtr        Previous MDP message
/// \param fragIdSpecified  indicates fragId present
/// \param fragId           Fragment ID to add as info
void
APP_UdpCopyMdpInfo(
    Node *node,
    Message *msg,
    const char* mdpInfo,
    Message *theMsgPtr
#ifdef ADDON_DB
    , BOOL fragIdSpecified=FALSE,
    Int32 fragId=-1
#endif
    );
/**/

/// Sets the outgoing interface for a UDP message.
///
/// \param msg                Message to be modified.
/// \param outgoingInterface  Interface number.

void
APP_UdpSetOutgoingInterface(
    Message* msg,
    int outgoingInterface);


/// Enables MdpQueueDataObject and accepts
/// optional parameters for MDP data object info.
///
/// \param msg                Message to be modified.
/// \param mdpUniqueId        unique id for MPD session.
/// \param mdpDataObjectInfo  specify the mdp data object info if any.
/// \param mdpDataInfosize    specify the mdp data info size.
void
APP_UdpSetMdpEnabled(
    Message* msg,
    Int32 mdpUniqueId,
    const char* mdpDataObjectInfo = NULL,
    unsigned short mdpDataInfosize = 0);

/// Sends the UDP message
///
/// \param node               Node pointer
/// \param msg                Message to be modified.
/// \param delay              delay time
/// \param appParam           db info class
void
APP_UdpSend(
    Node* node,
    Message* msg,
    clocktype delay = 0
#ifdef ADDON_DB
    , StatsDBAppEventParam* appParam = NULL
#endif
    );


/// \brief To truncate multiple spaces into a single space
///
/// \param str String from which spaces are to be truncated
void TruncateSpaces(char* str);

/// \brief To resolve host name into node pointer
///
/// It is assumed that the caller function has sent the name
/// to be resolved after transforming it into Upper case.
///
/// \param partition Partition which received HITL command
/// \param hostName Host name being resolved
/// \param success Whether resolved the host name or not
///
/// \return Node having the resolved name

Node* ResolveHostName(PartitionData* partition,
                      const char* hostName,
                      bool* success);

/// \brief To send HITL command status
/// This function will send HITL command status back to issuing client.
///
/// \param hid HID of the HITL command corresponding to
/// which status is to be sent
/// \param responseType Status to be sent
void
AppSendHitlStatus(const std::string& hid,
                  GuiHITLResponse responseType);

/// Determines if the message is TCP, UDP, or other
///
/// \param msg                Message to be checked for type.
AppMsgType APP_GetMsgType(Message *msg);

/// Returns the original trace protocol used with
/// APP_TcpCreateMessage or APP_UdpCreateMessage.
///
/// \param msg                Message to get trace protocol from

TraceProtocolType APP_GetTraceProtocol(Message *msg);

/// \brief To resolve node id into node pointer
///
/// \param partition   partition which received HITL command
/// \param nodeId      node id being resolved
///
/// \return node having the resolved name
/// and NULL in case of invalid node id

Node* ResolveNodeId(PartitionData* partition,
                    NodeAddress nodeId);

#endif /* _APP_UTIL_H_ */
