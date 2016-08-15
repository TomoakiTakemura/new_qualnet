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


/// \defgroup Package_MESSAGE MESSAGE

/// \file
/// \ingroup Package_MESSAGE
/// This file describes the message structure used to implement events
/// and functions for message operations.


#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <string>

#include "main.h"
#include "clock.h"
#include "trace.h"
#include "qualnet_error.h"
#include "spectrum.h"

// Forward declarations
class TimerManager;
struct MIMO_Data;

#ifdef _WIN32
typedef struct pthread_mutex_t_ * pthread_mutex_t;
#endif

// Size constants should be multiples of 8/sizeof(double).

/// Maximum Header Size
#define MSG_MAX_HDR_SIZE                           512

/// Size of small Info field.  Should be larger
/// than all commonly used info field data structures,
/// especially PropTxInfo and PropRxInfo.
#define SMALL_INFO_SPACE_SIZE                      144

/// Maximum message payload list
#define MSG_PAYLOAD_LIST_MAX                       1000

/// Maximum cached payload size
#define MAX_CACHED_PAYLOAD_SIZE                    1024

/// Maximum message info list
#define MSG_INFO_LIST_MAX                          1000

/// Maximum number of info fields
#define MAX_INFO_FIELDS                            12

// Arbitrary constant to support packet trace facility

/// Maximum number of headers
#define MAX_HEADERS                                20


/// This is a structure which contains information
/// about a info field.
struct MessageInfoHeader
{
    unsigned short infoType; // type of the info field
    unsigned int infoSize; // size of buffer pointed to by "info" variable
    char* info;              // pointer to buffer for holding info
};

struct MessageInfoBookKeeping
{
    int msgSeqNum; // Sequence number of the message
    int fragSize;  // Fragment size.
    int infoLowerLimit; // starting index for the info field.
    int infoUpperLimit; // ending index for the info field + 1
};

/// Type of information in the info field. One message can only
/// have up to one info field with a specific info type.
enum MessageInfoType
{
    INFO_TYPE_UNDEFINED = 0,  // an empty info field.
    INFO_TYPE_DEFAULT = 1,    // default info type used in situations where
                              // specific type is given to the info field.
    INFO_TYPE_AbstractCFPropagation, // type for abstract contention free
                                     // propagation info field.
    INFO_TYPE_AppName,      // Pass the App name down to IP layer
    INFO_TYPE_StatCategoryName,
    INFO_TYPE_DscpName,
    INFO_TYPE_SourceAddr,
    INFO_TYPE_SourcePort,
    INFO_TYPE_DestAddr,
    INFO_TYPE_DestPort,
    INFO_TYPE_DeliveredPacketTtlTotal, // Pass from IP to APP for session-based hop counts
    INFO_TYPE_IpTimeStamp,
    INFO_TYPE_DataSize,
    INFO_TYPE_AbstractPhy,

    INFO_TYPE_TransportOverhead,
    INFO_TYPE_NetworkOverhead,
    INFO_TYPE_MacOverhead,
    INFO_TYPE_PhyOverhead,
    INFO_TYPE_ALE_ChannelIndex,
    INFO_TYPE_PhyIndex,
    INFO_TYPE_MdpInfoData,
    INFO_TYPE_MdpObjectInfo,
    INFO_TYPE_SuperAppUDPData,
    INFO_TYPE_SuperAppTCPData,

    // cellular
    INFO_TYPE_UmtsRlcPduListInfo,
    INFO_TYPE_UmtsRlcSduInfo,
    INFO_TYPE_UmtsRlcSduSegInfo,
    INFO_TYPE_AppUmtsCallData,
    INFO_TYPE_UmtsCellSwitch,

    INFO_TYPE_VoipParameters,

    INFO_TYPE_OriginalInsertTime,
    INFO_TYPE_ExternalData,
    INFO_TYPE_UdpFragData,
    INFO_TYPE_StatsTiming,
    INFO_TYPE_AppStatsDbContent,
    INFO_TYPE_StatsDbMapping,
    INFO_TYPE_StatsDbAppSessionId,
    INFO_TYPE_NetStatsDbContent,
    INFO_TYPE_MacSummaryStats,
    INFO_TYPE_MacConnStats,
    INFO_TYPE_MessageNextPrevHop,
    INFO_TYPE_MessageAddrInfo,

    // EXATA
    INFO_TYPE_ForwardInfo,
    INFO_TYPE_PhyConnCrossPartition,
    INFO_TYPE_ForwardTcpHeader,

    INFO_TYPE_SocketInterfacePktHdr, // SocketInterface_PacketHeader

    // Network Security
    INFO_TYPE_PadLen,
    INFO_TYPE_IAHEP_RUTNG,
    INFO_TYPE_RPProcessed,
    INFO_TYPE_JAM,
    INFO_TYPE_IAHEP_NextHop,

    INFO_TYPE_IPPacketSentTime,
    INFO_TYPE_IpHeaderSize,
    INFO_TYPE_DelayInfo,
    INFO_TYPE_DidDropPacket,

    INFO_TYPE_SNMPV3,

    INFO_TYPE_Dot16BurstInfo,    // represents 802.16 burst information type

    // LTE
    INFO_TYPE_LtePhyDlTtiInfo,
    INFO_TYPE_LtePhyTxInfo,
    INFO_TYPE_LtePhyCqiInfo,
    INFO_TYPE_LtePhyRiInfo,
    INFO_TYPE_LtePhySrsInfo,
    INFO_TYPE_LtePhyPss,
    INFO_TYPE_LtePhyRandamAccessGrant, // RA Grant
    INFO_TYPE_LtePhyRrcConnectionSetupComplete, // RRCConnectionSetupComplete
    INFO_TYPE_LtePhyRrcConnectionReconfComplete,
    INFO_TYPE_LtePhyRandamAccessTransmitPreamble,
    INFO_TYPE_LtePhyRandamAccessTransmitPreambleTimerDelay,
    INFO_TYPE_LtePhyRandamAccessTransmitPreambleInfo,
    INFO_TYPE_LtePhyLtePhyToMacInfo,
    INFO_TYPE_LteUlTbTxInfo,
    INFO_TYPE_LteDci0Info,
    INFO_TYPE_LteDci1Info,
    INFO_TYPE_LteDci2aInfo,
    INFO_TYPE_LteHarqFeedback,
    INFO_TYPE_LteMacDestinationInfo,
    INFO_TYPE_LteMacNoTransportBlock, // If no TB in Message from MAC to
                                      // PHY, add this INFO
    INFO_TYPE_LteMacPeriodicBufferStatusReport, // Periodic BSR
    INFO_TYPE_LteMacRRELCIDFLWith7bitSubHeader,
    INFO_TYPE_LteMacRRELCIDFLWith15bitSubHeader,
    INFO_TYPE_LteMacRRELCIDSubHeader,
    INFO_TYPE_LteMacMultiplexingMsg,
    INFO_TYPE_LteMacNumMultiplexingMsg,
    INFO_TYPE_LteMacTxInfo,
    INFO_TYPE_LteRlcAmSduToPdu,
    INFO_TYPE_LteRlcAmPduFormatFixed,
    INFO_TYPE_LteRlcAmPduSegmentFormatFixed,
    INFO_TYPE_LteRlcAmPduFormatExtension,
    INFO_TYPE_LteRlcAmStatusPduPduFormatFixed,
    INFO_TYPE_LteRlcAmStatusPduPduFormatExtension,
    INFO_TYPE_LteRlcAmStatusPdu,
    INFO_TYPE_LteRlcAmResetData,
    INFO_TYPE_LteRlcAmPdcpPduInfo,
    INFO_TYPE_LtePdcpTxInfo,
    INFO_TYPE_LtePdcpSrcMsg,

    INFO_TYPE_LteEpcAppContainer,
    INFO_TYPE_LtePdcpDiscardTimerInfo,
    INFO_TYPE_LtePdcpBufferType,
    INFO_TYPE_LtePhyRrcMeasReport,      // measurement report list
    INFO_TYPE_LtePhyRrcConnReconf,

    INFO_TYPE_LteStatsDbSduPduInfo,

    INFO_TYPE_Dot11nTimerInfo,
    INFO_TYPE_Dot11nBER,

    // zigbee gts feature
    INFO_TYPE_Gts_Trigger_Precedence,
    INFO_TYPE_Gts_Slot_Start,
    INFO_TYPE_ZigbeeApp_Info,
    INFO_TYPE_AppMessenger_Status,
    INFO_TYPE_AppServerListen,

    INFO_TYPE_DhcpPacketOriginator,

    INFO_TYPE_JammerNewPower,
    INFO_TYPE_JammerPowerIncrement,

    INFO_TYPE_DOS_RATE_INCREMENT,
    INFO_TYPE_DOS_CHANGE_RATE,
    INFO_TYPE_DOSAttackerInfo,
    INFO_TYPE_DOT11_AC_INDEX,

	//takemura
	INFO_TYPE_MQTT_PACKET,
	INFO_TYPE_MQTT_CONNECT,
	INFO_TYPE_MQTT_PUBLISH,
	INFO_TYPE_MQTT_DISCONNECT,
	INFO_TYPE_MQTT_RESEND,
	INFO_TYPE_MQTT_LOST_CHECK,
	INFO_TYPE_MQTT_KPALTIMER,
	INFO_TYPE_MYAPI_BEACON
};

/// \brief Main data structure that represents a discrete event in qualnet.
///
/// A Message can represent either a timer or a simulated packet actually
/// sent across the network.  The kernel maintains a priority queue of
/// Messages which have been sent and takes care of scheduling handler
/// functions for each in the appropriate order (and if running in real time
/// mode, such as with EXata, also at the appropriate time).
///
/// Each Message can optionally contain the following:
/// * A base payload.
/// * Virtual payload; this represents data which would be sent over an
///   actual network but is eliminated from the internal representation in
///   simulation for efficiency.
/// * One or more headers.  These headers are typically added one at a time
///   to the head of a packet as it travels down through the sender's protocol
///   stack, and then removed one at a time (in the reverse order) as it
///   travels up through the receiver's protocol stack.
/// * One or more "info fields".  These are extra bits of information
///   attached to a Message which are not actually part of the data contents
///   of a packet, but are used for bookkeeping purposes.  Info fields are
///   also the usual way to pass data to the handler of a Message representing
///   a timer event.
///
/// \sa MESSAGE_* functions in message.h
class Message
{
private:
    static const UInt8 SENT = 0x01; // Message is being sent
    static const UInt8 FREED = 0x02; // MESSAGE_Free has been called
    static const UInt8 DELETED = 0x04; // Deleted using "delete"
    UInt8 m_flags;
public:
    /// The default constructor should not be used unless under specific
    /// circumstances.  The message is not initialized here.
    Message();

    Message(const Message& m);
    Message(PartitionData *partition,
            int  layerType,
            int  protocol,
            int  eventType,
            bool isMT = false);
    virtual ~Message();
    void operator = (const Message &p);

    /// Initialize the message with default values
    void initialize(PartitionData* partition);

    Message*  next; // For kernel use only.
    PartitionData* m_partitionData; // For kernel use only.

    // The following fields are simulation related information.

    short layerType;    ///< Layer which will receive the message
    short protocolType; ///< Protocol which will receive the message in the layer.
    short instanceId;   ///< Which instance to give message to (for multiple
                        ///< copies of a protocol or application).
    short m_radioId;    ///< which radio this belongs to (if any)
    short eventType;    ///< Message's Event type.

    unsigned int naturalOrder;  ///< used to maintain natural ordering
                                ///< for events at same time & node

    char error;         ///< Does the packet contain errors?

    bool    mtWasMT;            // Messages handed to the worker thread
                                // can't participate in the message recycling.
                                // As the partitionData->msgFreeList isn't
                                // locked.

    bool getSent() { return (m_flags & SENT) != 0; }
    void setSent(bool v);
    bool getFreed() { return (m_flags & FREED) != 0; }
    void setFreed(bool v);
    bool getDeleted() { return (m_flags & DELETED) != 0; }
    void setDeleted(bool v);

    bool      allowLoose;   // used only by the parallel code
    NodeId    nodeId;       // used only by the parallel code
    clocktype eventTime;    // used only by the parallel code
    clocktype eot;          // used only by the parallel code
    int sourcePartitionId;  // used only by the parallel code


    // An array of fields carries any information that needs to be
    // transported between layers.
    // It can be used for carrying data for messages which are not packets.
    // It can also be used to carry additional information for messages
    // which are packets.

    double smallInfoSpace[SMALL_INFO_SPACE_SIZE / sizeof(double)];


    // The following two fields are only used when the message is being
    // used to simulate an actual packt.

    // PacketSize field will indicate the simulated packet size. As a
    // packet moves up or down between the various layers, this field
    // will be updated to reflect the addition or deletion of the various
    // layer headers. For most purposes this does not have to be modified
    // by the users as it will be controlled through the following
    // functions: MESSAGE_AllocPacket, MESSAGE_AddHeader,
    // MESSAGE_RemoveHeader

    int packetSize;

    // The "packet" as seen by a particular layer for messages
    // which are used to simulate packets.

    char *packet;

    // This field is used for messages used to send packets. It is
    // used for internal error checking and should not be used by users.

    char *payload;

    // Size of the buffer pointed to by payload.
    // This field should never be changed by the user.

    int payloadSize;

    // Size of additional payload which should affect the
    // transmission delay of the packet, but need not be stored
    // in the actual char *payload

    int virtualPayloadSize;

    // If this is a packet, its the creation time.
    clocktype packetCreationTime;
    clocktype pktNetworkSendTime;

    bool cancelled;

    // Extra fields to support packet trace facility.
    // Will slow things down.
    NodeAddress originatingNodeId;
    int sequenceNumber;
    int originatingProtocol;
    int numberOfHeaders;
    int headerProtocols[MAX_HEADERS];
    int headerSizes[MAX_HEADERS];
    // Added field for SatCom parallel mode
    // holds the hw address of relay ground
    // node to prevent message repeat
    NodeAddress relayNodeAddr;

    std::vector<MessageInfoHeader> infoArray;
    std::vector<MessageInfoBookKeeping> infoBookKeeping;

// MILITARY_RADIOS_LIB
    int subChannelIndex;  // for multiple frequencies per interface
// MILITARY_RADIOS_LIB

    BOOL isPacked;
    int actualPktSize;

    bool isEmulationPacket;

    TimerManager* timerManager;

    bool isScheduledOnMainHeap;
    clocktype timerExpiresAt;

    double d_realStampTime;
    double d_simStampTime;
    double d_stamped;
    long long d_stampId;

    double now();

    void stamp(Node* node);

    double stampRealTime() const { return (d_stamped) ? d_realStampTime : 0.0; }
    double stampSimTime() const { return (d_stamped) ? d_simStampTime : 0.0; }
    double deltaRealTime() { return (d_stamped) ? now() - d_realStampTime : 0.0; }
    double deltaSimTime(Node* node);
    long long stampId() { return d_stampId; }

    double rtr(Node* node)
    {
      if (!d_stamped) return 0.0;

      double dst = deltaSimTime(node);
      double drt = deltaRealTime();

      if (dst == 0.0 && drt == 0.0) return 1.0;
      if (dst == 0.0) return 0.0;
      if (drt == 0.0) return 0.0;

      return drt / dst;
    }

    // Users should not modify anything above this line.
    // or below this one.
    // inline methods
    /// inserts virtual payload
    void addVirtualPayload(int size) { virtualPayloadSize += size; }

    /// removes virtual payload
    void removeVirtualPayload(int size)
    {
        virtualPayloadSize -= size;
        ERROR_Assert(virtualPayloadSize >= 0, "invalid virtual payload size");
    }

    /// returns a pointer to the beginning of the data packet.
    char* returnPacket() const { return packet; }

    /// returns the packet size, including virtual data.
    int returnPacketSize() const { return packetSize + virtualPayloadSize; }

    /// returns the size the packet is supposed to represent in cases where
    /// the implementation differs from the standard
    int returnActualSize() const { return (isPacked)? actualPktSize : packetSize;}

    /// returns the amount of virtual data in the packet
    int returnVirtualSize() const { return virtualPayloadSize; }

    /// sets the layer and protocol for mapping the appropriate event handler
    void setLayer(int layer, int protocol) {
        layerType = layer;
        protocolType = protocol;
    }

    /// Returns the layer associated with a message
    int getLayer() const { return layerType; }

    /// Returns the protocol associated with a message
    int getProtocol() const { return protocolType; }

    /// Set the event associated with a message
    void setEvent(int event) { eventType = event; }

    /// Returns the event type of a message
    int getEvent() const { return eventType; }

    /// Sets the instanceId of a message
    void setInstanceId(int instance) { instanceId = instance; }

    /// Returns the instanceId of a message
    int getInstanceId() const { return instanceId; }

    void radioId(int p_radioId) { m_radioId = (short)p_radioId; }
    int radioId() const { return (int)m_radioId; }
    bool hasRadioId() const { return m_radioId >= 0; }

    /// Returns the packet creation time
    clocktype getPacketCreationTime() { return packetCreationTime; }

    spectralBand* m_band;
    MIMO_Data* m_mimoData;
};

/// Shorter version of the message data structure used to reduce the amount of data send to
/// remote distributed partitions when using MPI
/// All the variables are the relevant variables from the Message class that is required
/// When sending a remote message
struct ShortMessage
{
    short layerType;    /// Layer which will receive the message
    short protocolType; /// Protocol which will receive the message in the layer.
    short instanceId;   /// Which instance to give message to (for multiple
                        /// copies of a protocol or application).
    short m_radioId;    /// which radio this belongs to (if any)
    short eventType;    /// Message's Event type.

    unsigned int naturalOrder;  /// used to maintain natural ordering
                                /// for events at same time & node

    char error;         /// Does the packet contain errors?

    bool    mtWasMT;            // Messages handed to the worker thread
                                // can't participate in the message recycling.
                                // As the partitionData->msgFreeList isn't
                                // locked.

    bool      allowLoose;   // used only by the parallel code
    NodeId    nodeId;       // used only by the parallel code
    clocktype eventTime;    // used only by the parallel code
    clocktype eot;          // used only by the parallel code
    int sourcePartitionId;  // used only by the parallel code
    
    int packetSize;
    // Size of the buffer pointed to by payload.
    // This field should never be changed by the user.

    int payloadSize;

    int offset;

    // Size of additional payload which should affect the
    // transmission delay of the packet, but need not be stored
    // in the actual char *payload

    int virtualPayloadSize;

    // Extra fields to support packet trace facility.
    // Will slow things down.
    NodeAddress originatingNodeId;
    int sequenceNumber;
    int originatingProtocol;
    int originatingApplication;
    
    // Added field for SatCom parallel mode
    // holds the hw address of relay ground
    // node to prevent message repeat
    NodeAddress relayNodeAddr;

// MILITARY_RADIOS_LIB
    int subChannelIndex;  // for multiple frequencies per interface
// MILITARY_RADIOS_LIB

    BOOL isPacked;
    int actualPktSize;
    bool isEmulationPacket;

};


/// Implements an efficient message queue.  The base
/// class is not thread safe.
class MessageQueue
{
public:
    MessageQueue();
    virtual ~MessageQueue();

    // Enqueue one or more messages
    virtual void enqueue(Message* msg);

    // Dequeue one message
    // Returns NULL if empty
    virtual Message* dequeue();

    // Dequeue all messages
    // Returns NULL if empty
    virtual Message* dequeueAll();

    virtual bool empty() { return m_head == NULL; }

    // Return the time of the earliest event in queue
    // Return CLOCKTYPE_MAX if no events
    clocktype getEarliestEvent() { return m_earliestEvent; }

protected:
    Message* m_head;
    Message* m_tail;
    int m_size;
    clocktype m_earliestEvent;
};

/// Implements an efficient message queue.  This class
/// is safe for multiple readers and writers.
class ThreadSafeMessageQueue : public MessageQueue
{
public:
    ThreadSafeMessageQueue();
    virtual ~ThreadSafeMessageQueue();

    // Enqueue one or more messages
    virtual void enqueue(Message* msg);

    // Dequeue one message
    // Returns NULL if empty
    virtual Message* dequeue();

    // Dequeue all messages
    // Returns NULL if empty
    virtual Message* dequeueAll();

protected:
    pthread_mutex_t* m_mutex;
};

/// \name Macros for legacy support.
/// @{
#define MESSAGE_AddVirtualPayload(node, msg, payloadSize) \
        (msg->addVirtualPayload(payloadSize))
#define MESSAGE_RemoveVirtualPayload(node, msg, payloadSize) \
        (msg->removeVirtualPayload(payloadSize))
#define MESSAGE_ReturnPacket(msg) (msg->returnPacket())
#define MESSAGE_ReturnPacketSize(msg) (msg->returnPacketSize())
#define MESSAGE_ReturnActualPacketSize(msg) (msg->returnActualSize())
#define MESSAGE_ReturnVirtualPacketSize(msg) (msg->returnVirtualSize())
#define MESSAGE_SetLayer(msg, layer, protocol) (msg->setLayer(layer, protocol))
#define MESSAGE_GetLayer(msg) (msg->getLayer())
#define MESSAGE_GetProtocol(msg) (msg->getProtocol())
#define MESSAGE_SetEvent(msg, event) (msg->setEvent(event))
#define MESSAGE_GetEvent(msg) (msg->getEvent())
#define MESSAGE_SetInstanceId(msg, instance) (msg->setInstanceId(instance))
#define MESSAGE_GetInstanceId(msg) (msg->getInstanceId())
#define MESSAGE_GetPacketCreationTime(msg) (msg->getPacketCreationTime())
/// @}

// mtPendingSend count that prevents wrap around - will leak, but won't crash.
#define MESSAGE_MT_PENDING_SEND_INFINITE 255

/// Print out the contents of the message for debugging purposes.
///
/// \param node  node which is sending message
/// \param msg  message to be printed
void MESSAGE_PrintMessage(Message* msg);

/// Function call used to send a message within QualNet. When
/// a message is sent using this mechanism, only the pointer
/// to the message is actually sent through the system. So the
/// user has to be careful not to do anything with the content
/// of the pointer once MESSAGE_Send has been called.
///
/// \param node  node which is sending message
/// \param msg  message to be delivered
/// \param delay  delay suffered by this message.
/// \param isMT  is the function being called from a thread?
void MESSAGE_Send(Node*     node,
                  Message*  msg,
                  clocktype delay,
                  bool      isMT = false);

/// Function call used to send a message from independent
/// threads running within QualNet, for example those associated
/// with external interfaces.
///
/// \param node  node which is sending message
/// \param msg  message to be delivered
/// \param delay  delay suffered by this message.
inline
void MESSAGE_SendMT(Node *node, Message *msg, clocktype delay) {
    MESSAGE_Send(node, msg, delay, true);
}

/// Function used to send a message to a node that might be
/// on a remote partition.  The system will make a shallow copy
/// of the message, meaning it can't contain any pointers in
/// the info field or the packet itself.
///
/// \warning This function is very unsafe.  If you use it, your program
/// will probably crash.  Only I can use it.
///
/// \param node  node which is sending message
/// \param destNodeId  nodeId of receiving node
/// \param msg  message to be delivered
/// \param delay  delay suffered by this message.
void MESSAGE_RemoteSend(Node*     node,
                        NodeId    destNodeId,
                        Message*  msg,
                        clocktype delay);

/// Counterpart to MESSAGE_RemoteSend, this function allows
/// models that send remote messages to provide special handling
/// for them on the receiving partition.  This function is
/// called in real time as the messages are received, so must
/// be used carefully.
///
/// \param node  node which is sending message
/// \param msg  message to be delivered
void MESSAGE_RouteReceivedRemoteEvent(Node*    node,
                                      Message* msg);

/// Function call used to cancel a event message in the
/// QualNet scheduler.  The Message must be a self message
/// (timer) .i.e. a message a node sent to itself.  The
/// msgToCancelPtr must a pointer to the original message
/// that needs to be canceled.
///
/// \param node  node which is sending message
/// \param msgToCancelPtr  message to be cancelled
inline
void MESSAGE_CancelSelfMsg(Node *, Message *msgToCancelPtr) {
   msgToCancelPtr->cancelled = TRUE;
}

inline
void MESSAGE_SetLooseScheduling(Message *msg) {
    msg->allowLoose = true;
}

inline
bool MESSAGE_AllowLooseScheduling(Message *msg) {
    return (msg->allowLoose);
}

/// Allocate a new Message structure. This is called when a
/// new message has to be sent through the system. The last
/// three parameters indicate the layerType, protocol and the
/// eventType that will be set for this message.
///
/// \param node  node which is allocating message
/// \param layerType  Layer type to be set for this message
/// \param protocol  Protocol to be set for this message
/// \param eventType  event type to be set for this message
///
/// \return Pointer to allocated message structure
Message* MESSAGE_Alloc(
    Node *node, int layerType, int protocol, int eventType, bool isMT = false);

/// Allocate a new Message structure. This is called when a
/// new message has to be sent through the system. The last
/// three parameters indicate the layerType, protocol and the
/// eventType that will be set for this message.
///
/// \param partition  partition that is allocating message
/// \param layerType  Layer type to be set for this message
/// \param protocol  Protocol to be set for this message
/// \param eventType  event type to be set for this message
///
/// \return Pointer to allocated message structure
Message* MESSAGE_Alloc(PartitionData *partition,
                       int layerType,
                       int protocol,
                       int eventType,
    bool isMT = false);

/// Mutli-thread safe version of MESSAGE_Alloc for use
/// by worker threads.
///
/// \param partition  partition that is allocating message
/// \param layerType  Layer type to be set for this message
/// \param protocol  Protocol to be set for this message
/// \param eventType  event type to be set for this message
///
/// \return Pointer to allocated message structure
inline
Message* MESSAGE_AllocMT(PartitionData *partition,
                         int layerType,
                         int protocol,
                         int eventType)
{
    return MESSAGE_Alloc(partition, layerType, protocol, eventType, true);
}

/// Allocate space for one "info" field
///
/// \param node  node which is allocating the space.
/// \param infoSize  size of the space to be allocated
///
/// \return pointer to the allocated space.
char* MESSAGE_InfoFieldAlloc(Node *node, int infoSize, bool isMT = false);

/// Allocate space for one "info" field
///
/// \param partition  partition which is allocating the space.
/// \param infoSize  size of the space to be allocated
///
/// \return pointer to the allocated space.
char* MESSAGE_InfoFieldAlloc(PartitionData *partition, int infoSize,
                             bool isMT = false);

/// Multi-thread safe version of MESSAGE_InfoFieldAlloc
///
/// \param partition  partition which is allocating the space.
/// \param infoSize  size of the space to be allocated
///
/// \return pointer to the allocated space.
inline
char* MESSAGE_InfoFieldAllocMT(PartitionData *partition, int infoSize)
{
    return MESSAGE_InfoFieldAlloc(partition, infoSize, true);
}

/// Free space for one "info" field
///
/// \param node  node which is allocating the space.
/// \param hdrPtr  pointer to the "info" field
void MESSAGE_InfoFieldFree(Node *node, MessageInfoHeader* hdrPtr,
                           bool isMT = false);

/// Free space for one "info" field
///
/// \param partition  partition which is allocating the space.
/// \param hdrPtr  pointer to the "info" field
void MESSAGE_InfoFieldFree(PartitionData *partition,
                           MessageInfoHeader* hdrPtr, bool isMT);

/// Multithread safe version of MESSAGE_InfoFieldFree()
///
/// \param partition  partition which is allocating the space.
/// \param hdrPtr  pointer to the "info" field
inline
void MESSAGE_InfoFieldFreeMT(PartitionData *partition,
                             MessageInfoHeader* hdrPtr) {
    MESSAGE_InfoFieldFree(partition, hdrPtr, true);
}

/// \brief Add an info field to a Message.
///
/// Allocate one "info" field with given info type for the
/// message. This function is used for the delivery of data
/// for messages which are NOT packets as well as the delivery
/// of extra information for messages which are packets. If a
/// "info" field with the same info type has previously been
/// allocated for the message, it will be replaced by a new
/// "info" field with the specified size. Once this function
/// has been called, MESSAGE_ReturnInfo function can be used
/// to get a pointer to the allocated space for the info field
/// in the message structure.
///
/// \param node  node which is allocating the info field.
/// \param msg  message for which "info" field
///    has to be allocated
/// \param infoSize  size of the "info" field to be allocated
/// \param infoType  type of the "info" field to be allocated.
///
/// \return Pointer to the added info field
char* MESSAGE_AddInfo(Node *node,
                      Message *msg,
                      int infoSize,
                      unsigned short infoType = INFO_TYPE_DEFAULT);

/// \brief Add an info field to a Message.
///
/// Allocate one "info" field with given info type for the
/// message. This function is used for the delivery of data
/// for messages which are NOT packets as well as the delivery
/// of extra information for messages which are packets. If a
/// "info" field with the same info type has previously been
/// allocated for the message, it will be replaced by a new
/// "info" field with the specified size. Once this function
/// has been called, MESSAGE_ReturnInfo function can be used
/// to get a pointer to the allocated space for the info field
/// in the message structure.
///
/// \param partition  partition which is allocating the info field.
/// \param msg  message for which "info" field
///    has to be allocated
/// \param infoSize  size of the "info" field to be allocated
/// \param infoType  type of the "info" field to be allocated.
///
/// \return Pointer to the added info field
char* MESSAGE_AddInfo(PartitionData *partition,
                      Message *msg,
                      int infoSize,
                      unsigned short infoType = INFO_TYPE_DEFAULT);

/// \brief Remove an info field from a Message.
/// Remove one "info" field with given info type from the
/// info array of the message.
///
/// \param node  node which is removing info field.
/// \param msg  message for which "info" field
///    has to be removed
/// \param infoType  type of the "info" field to be removed.
void MESSAGE_RemoveInfo(Node *node, Message *msg, unsigned short infoType);

/// Allocate the default "info" field for the message. This
/// function is similar to MESSAGE_AddInfo. The difference
/// is that it assumes the type of the info field to be
/// allocated is INFO_TYPE_DEFAULT.
///
/// \param node  node which is allocating the info field.
/// \param msg  message for which "info" field
///    has to be allocated
/// \param infoSize  size of the "info" field to be allocated
inline
char * MESSAGE_InfoAlloc(Node *node, Message *msg, int infoSize)
{
    return (MESSAGE_AddInfo(node,
                            msg,
                            infoSize));
}

/// Allocate the default "info" field for the message. This
/// function is similar to MESSAGE_AddInfo. The difference
/// is that it assumes the type of the info field to be
/// allocated is INFO_TYPE_DEFAULT.
///
/// \param partition  partition which is allocating the info field.
/// \param msg  message for which "info" field
///    has to be allocated
/// \param infoSize  size of the "info" field to be allocated
inline
char * MESSAGE_InfoAlloc(PartitionData *partition, Message *msg, int infoSize)
{
    return (MESSAGE_AddInfo(partition,
                            msg,
                            infoSize));
}

/// Returns the size of a "info" field with given info type
/// in the info array of the message.
///
/// \param msg  message for which "info" field
///    has to be returned
/// \param infoType  type of the "info" field.
/// \param fragmentNumber  Location of the fragment in the TCP packet
///
/// \return size of the info field.
inline
int MESSAGE_ReturnInfoSize(Message *msg,
                                  unsigned short infoType,
                                  int fragmentNumber)
{
    int i;
    if ((unsigned)fragmentNumber >= msg->infoBookKeeping.size())
    {
        return 0;
    }
    int infoLowerLimit =
        msg->infoBookKeeping.at(fragmentNumber).infoLowerLimit;
    int infoUpperLimit =
        msg->infoBookKeeping.at(fragmentNumber).infoUpperLimit;

    for (i = infoLowerLimit; i < infoUpperLimit; i ++)
    {
        if (msg->infoArray[i].infoType == infoType)
        {
            return msg->infoArray[i].infoSize;
        }
    }

    return 0;
}
/// Returns the size of a "info" field with given info type
/// in the info array of the message.
///
/// \param msg  message for which "info" field
///    has to be returned
/// \param infoType  type of the "info" field.
///
/// \return size of the info field.
inline
int MESSAGE_ReturnInfoSize(const Message* msg,
                                  unsigned short infoType = INFO_TYPE_DEFAULT)
{
    unsigned int i;

    if (msg->infoArray.size() > 0)
    {
        /*if (infoType == INFO_TYPE_DEFAULT)
        {
            return msg->infoArray[0].infoSize;
        }*/


        for (i = 0; i < msg->infoArray.size(); i ++)
        {
            MessageInfoHeader* hdrPtr = (MessageInfoHeader*)&(msg->infoArray[i]);
            if (hdrPtr->infoType == infoType)
            {
                return hdrPtr->infoSize;
            }
        }
    }

    return 0;
}

/// Returns a pointer to the "info" field with given info type
/// in the info array of the message.
///
/// \param msg  message for which "info" field
///    has to be returned
/// \param infoType  type of the "info" field to be returned.
///
/// \return Pointer to the "info" field with given type.
/// NULL if not found.
inline
char* MESSAGE_ReturnInfo(const Message *msg,
                                unsigned short infoType = INFO_TYPE_DEFAULT)
{
    unsigned int i;

    if (msg->infoArray.size() > 0)
    {
        /*if (infoType == INFO_TYPE_DEFAULT)
        {
            return msg->infoArray[0].info;
        }*/

        for (i = 0; i < msg->infoArray.size(); i ++)
        {
            MessageInfoHeader* hdrPtr = (MessageInfoHeader*)&(msg->infoArray[i]);
            if (hdrPtr->infoType == infoType)
            {
                return hdrPtr->info;
            }
        }
    }
    return NULL;
}

/// Copy the "info" fields of the source message to
/// the destination message.
///
/// \param node  Node which is copying the info fields
/// \param dsgMsg  Destination message
/// \param srcMsg  Source message
void MESSAGE_CopyInfo(Node *node, Message *dstMsg, Message *srcMsg);

/// Copy the "info" fields of the source info header to
/// the destination message.
///
/// \param node  Node which is copying the info fields
/// \param dsgMsg  Destination message
/// \param srcInfo  Info Header structure
void MESSAGE_CopyInfo(Node *node, Message *dstMsg, std::vector<MessageInfoHeader*> srcInfo);

/// Returns a pointer to the "info" field with given info type
/// in the info array of the message.
///
/// \param msg  message for which "info" field
///    has to be returned
/// \param infoType  type of the "info" field to be returned.
/// \param fragmentNumber  Location of the fragment in the TCP packet.
///
/// \return Pointer to the "info" field with given type.
/// NULL if not found.
inline
char* MESSAGE_ReturnInfo(const Message *msg,
                                unsigned short infoType,
                                int fragmentNumber)
{
    int i;
    if ((unsigned int)fragmentNumber >= msg->infoBookKeeping.size())
    {
        return NULL;
    }
    int infoLowerLimit =
        msg->infoBookKeeping.at(fragmentNumber).infoLowerLimit;
    int infoUpperLimit =
        msg->infoBookKeeping.at(fragmentNumber).infoUpperLimit;

    for (i = infoLowerLimit ; i < infoUpperLimit; i ++)
    {
        if (msg->infoArray[i].infoType == infoType)
        {
            return msg->infoArray[i].info;
        }
    }

    return NULL;
}


/// Fragment one packet into multiple fragments
///
/// \note The original packet will be freed in this function.
/// The array for storing pointers to fragments will be
/// dynamically allocated. The caller of this function
/// will need to free the memory.
///
/// \param node  node which is fragmenting the packet
/// \param msg  The packet to be fragmented
/// \param fragUnit  The unit size for fragmenting the packet
/// \param fragList  A list of fragments created.
/// \param numFrags  Number of fragments in the fragment list.
/// \param protocolType  Protocol type for packet tracing.
void MESSAGE_FragmentPacket(
         Node* node,
         Message* msg,
         int fragUnit,
         Message*** fragList,
         int* numFrags,
         TraceProtocolType protocolType);

/// Reassemble multiple fragments into one packet
///
/// \note All the fragments will be freed in this function.
///
/// \param node  node which is assembling the packet
/// \param fragList  A list of fragments.
/// \param numFrags  Number of fragments in the fragment list.
/// \param protocolType  Protocol type for packet tracing.
///
/// \return The reassembled packet.
Message* MESSAGE_ReassemblePacket(
             Node* node,
             Message** fragList,
             int numFrags,
             TraceProtocolType protocolType);

/// Serialize a single message info a buffer so that the original
/// message can be recovered from the buffer
///
/// \param node  Pointer to node.
/// \param msg  Pointer to a message
/// \param buffer  The string buffer the message will be serialized into
///    (append to the end)
void MESSAGE_Serialize(Node* node,
                       Message* msg,
                       std::string& buffer);

/// Short serialize (reduce meta data) a single message info a buffer so that the original
/// message can be recovered from the buffer
///
/// \param node  Pointer to node.
/// \param msg  Pointer to a message
/// \param buffer  The string buffer the message will be serialized into
///    (append to the end)
void MESSAGE_ShortSerialize(Node* node,
                       Message* msg,
                       std::string& buffer);

/// Used in MPI code to serialize a single message into a buffer
/// for transmission to another processor
///
/// \param msg  Pointer to a message
/// \param buffer  The buffer the message will be serialized into
///
/// \return size of message in buffer
int MESSAGE_Serialize(Message* msg,
              unsigned char buffer[]);

/// recover the original message from the buffer
///
/// \param partitionData  Pointer to partition data
/// \param buffer  The string buffer containing the message was serialized into
/// \param bufIndex  the start position in the buffer pointing to the message
///    updated to the end of the message after the unserialization.
///
/// \return Message pointer to be recovered
Message* MESSAGE_Unserialize(PartitionData* partitionData,
                             const char* buffer,
                             int& bufIndex,
                             bool mt = false);

/// Store a list of message into a buffer so that the original
/// messages can be recovered from the buffer
///
/// \param node  Pointer to node.
/// \param msg  Pointer to a message list
/// \param buffer  The string buffer the messages will be serialized into
///    (append to the end)
///
/// \return number of messages in the list
int MESSAGE_SerializeMsgList(Node* node,
                             Message* msgList,
                             std::string& buffer);

/// Pack a list of messages to be one message structure
/// Whole contents of the list messages will be put as
/// payload of the new message. So the packet size of
/// the new message cannot be directly used now.
/// The original lis of msgs will be freed.
///
/// \param node  Pointer to node.
/// \param msgList  Pointer to a list of messages
/// \param origProtocol  Protocol allocating this packet
/// \param actualPktSize  For return sum of packet size of msgs in list
///
/// \return The super msg contains a list of msgs as payload
Message* MESSAGE_PackMessage(Node* node,
                             Message* msgList,
                             TraceProtocolType origProtocol,
                             int* actualPktSize);

/// Unpack a super message to the original list of messages
/// The list of messages were stored as payload of this super
/// message.
///
/// \param node  Pointer to node.
/// \param msg  Pointer to the supper msg contains list of msgs
/// \param copyInfo  Whether copy info from old msg to first msg
/// \param freeOld  Whether the original message should be freed
///
/// \return A list of messages unpacked from original msg
Message* MESSAGE_UnpackMessage(Node* node,
                               Message* msg,
                               bool copyInfo,
                               bool freeOld);

/// recover the original message list from the buffer
///
/// \param partitionData  Pointer to partition data.
/// \param buffer  The string buffer containing the message list serialized into
/// \param bufIndex  the start position in the buffer pointing to the message
///    list updated to the end of the message list after the unserialization.
/// \param numMsgs  Number of messages in the list
///
/// \return Pointer to the message list to be recovered
Message* MESSAGE_UnserializeMsgList(PartitionData* partitionData,
                                    const char* buffer,
                                    int& bufIndex,
                                    unsigned int numMsgs);

/// Allocate the \c payload field for the packet to be delivered.
/// Add additional free space in front of the packet for
/// headers that might be added to the packet. This function
/// can be called from the application layer or anywhere else
/// (e.g TCP, IP) that a packet may originiate from. The
/// \c packetSize variable will be set to the \p packetSize
/// parameter specified in the function call. Once this function
/// has been called the \c packet variable in the message
/// structure can be used to access this space.
///
/// \param node  node which is allocating the packet
/// \param msg  message for which packet has to be allocated
/// \param packetSize  size of the packet to be allocated
/// \param originalProtocol  Protocol allocating this packet
void MESSAGE_PacketAlloc(Node *node,
                         Message *msg,
                         int packetSize,
                         TraceProtocolType originalProtocol);

/// Allocate the \c payload field for the packet to be delivered.
/// Add additional free space in front of the packet for
/// headers that might be added to the packet. This function
/// can be called from the application layer or anywhere else
/// (e.g TCP, IP) that a packet may originiate from. The
/// \c packetSize variable will be set to the \p packetSize
/// parameter specified in the function call. Once this function
/// has been called the \c packet variable in the message
/// structure can be used to access this space.
///
/// \param partition  artition which is allocating the packet
/// \param msg  message for which packet has to be allocated
/// \param packetSize  size of the packet to be allocated
/// \param originalProtocol  Protocol allocating this packet
/// \param isMT  Is this packet being created from a worker thread
void MESSAGE_PacketAlloc(PartitionData *partition,
                         Message *msg,
                         int packetSize,
                         TraceProtocolType originalProtocol,
                         bool isMT = false);

/// \brief Add a header to a Message.
///
/// This function is called to reserve additional space for a
/// header of size \p hdrSize for the packet enclosed in the
/// message. The \c packetSize variable in the message structure
/// will be increased by \p hdrSize.
/// Since the header has to be prepended to the current packet,
/// after this function is called the \c packet variable in the
/// message structure will point the space occupied by this new
/// header.
///
/// \param node  node which is adding header
/// \param msg  message for which header has to be added
/// \param hdrSize  size of the header to be added
/// \param traceProtocol  protocol name, from trace.h
void MESSAGE_AddHeader(Node *node,
                       Message *msg,
                       int hdrSize,
                       TraceProtocolType traceProtocol);

/// \brief Remove a header from a Message.
///
/// This function is called to remove a header from the packet.
/// The \c packetSize variable in the message will be decreased
/// by \p hdrSize.
///
/// \param node  node which is removing the packet header
/// \param msg  message for which header is being removed
/// \param hdrSize  size of the header being removed
/// \param traceProtocol  protocol removing this header.
void MESSAGE_RemoveHeader(Node *node,
                          Message *msg,
                          int hdrSize,
                          TraceProtocolType traceProtocol);

/// This is kind of a hack so that MAC protocols (dot11) that
/// need to peek at a packet that still has the PHY header can
/// return the contents after the first (N) headers without
/// first removing those headers.
///
/// \param msg  message containing a packet with headers
/// \param header  number of the header to return.
///
/// \return the packet starting at the header'th header
char* MESSAGE_ReturnHeader(const Message* msg,
                           int            header);

/// Expand packet by a specified size
///
/// \param node  node which is expanding the packet
/// \param msg  message which is to be expanded
/// \param size  size to expand
void MESSAGE_ExpandPacket(Node *node,
                          Message *msg,
                          int size);

/// This function is called to shrink
/// packet by a specified size.
///
/// \param node  node which is shrinking packet
/// \param msg  message whose packet is be shrinked
/// \param size  size to shrink
void MESSAGE_ShrinkPacket(Node *node,
                          Message *msg,
                          int size);

/// When the message is no longer needed it
/// can be freed. Firstly the "payload" and "info" fields
/// of the message are freed. Then the message itself is freed.
/// It is important to remember to free the message. Otherwise
/// there will nasty memory leaks in the program.
///
/// \param partition  partition which is freeing the message
/// \param msg  message which has to be freed
void MESSAGE_Free(PartitionData *partition, Message *msg);

/// Multithread safe version of MESSAGE_Free
///
/// \param partition  partition which is freeing the message
/// \param msg  message which has to be freed
inline
void MESSAGE_FreeMT(PartitionData *partition, Message *msg)
{
    msg->mtWasMT = true;
    MESSAGE_Free(partition, msg);
}

/// \brief Free a Message and all associated storage.
///
/// When the message is no longer needed it
/// can be freed. Firstly the "payload" and "info" fields
/// of the message are freed. Then the message itself is freed.
/// It is important to remember to free the message. Otherwise
/// there will nasty memory leaks in the program.
///
/// \param node  node which is freeing the message
/// \param msg  message which has to be freed
///
/// \note By default, the message contents will not necessarily be freed
/// at the system level, but may be put onto a list for recycling by future
/// calls to MESSAGE_Alloc().  This can cause issues when trying to debug
/// memory leaks using valgrind and similar tools.  In order to debug
/// Message related memory leaks, it is recommended to first edit
/// \c main/message.cpp to uncomment the line
/// \code
/// #define MESSAGE_NO_RECYCLE
/// \endcode
/// and recompile.
void MESSAGE_Free (Node *node, Message *msg);


/// Free a list of message until the next pointer of the
/// message is NULL.
///
/// \param node  node which is freeing the message
/// \param msg  message which has to be freed
void MESSAGE_FreeList(Node *node, Message *msg);

/// Create a new message which is an exact duplicate
/// of the message supplied as the parameter to the function and
/// return the new message.
///
/// \param node  node is calling message copy
/// \param msg  message for which duplicate has to be made
///
/// \return Pointer to the new message
Message* MESSAGE_Duplicate(Node *node, const Message *msg, bool isMT = false);

/// Create a new message which is an exact duplicate
/// of the message supplied as the parameter to the function and
/// return the new message.
///
/// \param partition  partition is calling message copy
/// \param msg  message for which duplicate has to be made
/// \param isMT  Is this function being called from the context
///    of multiple threads
///
/// \return Pointer to the new message
Message* MESSAGE_Duplicate (PartitionData *partition, const Message *msg,
    bool isMT = false);

/// Create a new message which is an exact duplicate
/// of the message supplied as the parameter to the function and
/// return the new message.
///
/// \param partition  partition is calling message copy
/// \param msg  message for which duplicate has to be made
///
/// \return Pointer to the new message
inline
Message* MESSAGE_DuplicateMT(PartitionData *partition, const Message *msg)
{
    return MESSAGE_Duplicate(partition, msg, true);
}

/// Allocate a character payload out of the free list,
/// if possible otherwise via malloc.
///
/// \param node  node which is allocating payload
/// \param payloadSize  size of the field to be allocated
///
/// \return pointer to the allocated memory
char* MESSAGE_PayloadAlloc(Node *node, int payloadSize, bool isMT = false);

/// Allocate a character payload out of the free list,
/// if possible otherwise via malloc.
///
/// \param partition  partition which is allocating payload
/// \param payloadSize  size of the field to be allocated
/// \param isMT  Is this packet being created from a worker thread
///
/// \return pointer to the allocated memory
char* MESSAGE_PayloadAlloc(PartitionData *partition, int payloadSize, bool isMT = false);

/// Return a character payload to the free list,
/// if possible otherwise free it.
///
/// \param partition  partition which is freeing payload
/// \param payload  Pointer to the payload field
/// \param payloadSize  size of the payload field
///
void MESSAGE_PayloadFree(PartitionData *partition, char *payload, int payloadSize,
    bool wasMT);

/// Return a character payload to the free list,
/// if possible otherwise free it.
///
/// \param node  node which is freeing payload
/// \param payload  Pointer to the payload field
/// \param payloadSize  size of the payload field
///
void MESSAGE_PayloadFree(Node *node, char *payload, int payloadSize,
                         bool wasMT = false);

/// Multithread safe version of MESSAGE_PayloadFree()
///
/// \param partition  partition which is allocating payload
/// \param payloadSize  size of the "info" field to be allocated
inline
void MESSAGE_PayloadFreeMT(PartitionData* partition,
                           char* payload,
                           int payloadSize)
{
    MESSAGE_PayloadFree(partition, payload, payloadSize, true);
}

/// Returns the number of fragments used to create a TCP packet.
///
/// \param msg  message for which "info" field
///    has to be returned
///
/// \return Number of Fragments.
/// 0 if none.
inline
int MESSAGE_ReturnNumFrags(const Message* msg)
{
    return (int)msg->infoBookKeeping.size();
}

/// Returns the sequence number of a particular fragments
/// in the TCP packet.
///
/// \param msg  message for which "info" field
///    has to be returned
/// \param fragmentNumber  fragment location in the TCP message.
///
/// \return Sequence number of the fragment.
/// -1 if none.
inline
int MESSAGE_ReturnFragSeqNum (const Message* msg,
                                     unsigned int fragmentNumber)
{
    if (fragmentNumber < msg->infoBookKeeping.size())
    {
        return msg->infoBookKeeping.at(fragmentNumber).msgSeqNum;
    }
    return -1;
}

/// Returns the size of a particular fragment
/// in the TCP packet.
///
/// \param msg  message for which "info" field
///    has to be returned
/// \param fragmentNumber  fragment location in the TCP message.
///
/// \return Sequence number of the fragment.
/// 0 if none.
inline
int MESSAGE_ReturnFragSize (const Message* msg,
                                   unsigned int fragmentNumber)
{
    if (fragmentNumber < msg->infoBookKeeping.size())
    {
        return msg->infoBookKeeping.at(fragmentNumber).fragSize;
    }
    return 0;
}

/// Returns the number of info fields associated with
/// a particular fragment in the TCP packet.
///
/// \param msg  message for which "info" field
///    has to be returned
/// \param fragmentNumber  fragment location in the TCP message.
///
/// \return Sequence number of the fragment.
/// 0 if none.
inline
int MESSAGE_ReturnFragNumInfos (const Message* msg,
                                       unsigned int fragmentNumber)
{
    if (fragmentNumber < msg->infoBookKeeping.size())
    {
        int numInfos = 0;
        numInfos = msg->infoBookKeeping.at(fragmentNumber).infoUpperLimit -
                   msg->infoBookKeeping.at(fragmentNumber).infoLowerLimit;
        return numInfos;
    }
    return 0;
}

/// Appends the "info" fields of the source message to
/// the destination message.
///
/// \param partitionData  Partition which is copying the info fields
/// \param msg  Destination message
/// \param infosize  size of the info field
/// \param infoType  type of info field.
char* MESSAGE_AppendInfo(PartitionData* partitionData,
                        Message *msg,
                        int infoSize,
                        unsigned short infoType);

/// Appends the "info" fields of the source message to
/// the destination message.
///
/// \param node  Node which is copying the info fields
/// \param msg  Destination message
/// \param infosize  size of the info field
/// \param infoType  type of info field.
char* MESSAGE_AppendInfo(Node* node,
                        Message *msg,
                        int infoSize,
                         unsigned short infoType);

/// Appends the "info" fields of the source message to
/// the destination message.
///
/// \param node  Node which is copying the info fields
/// \param dsgMsg  Destination message
/// \param srcInfo  Source message info vector
void MESSAGE_AppendInfo(Node *node, Message *dstMsg, std::vector<MessageInfoHeader> srcInfo);

/// Appends the "info" fields of the source message to
/// the destination message.
///
/// \param node  Node which is copying the info fields
/// \param dsgMsg  Destination message
/// \param srcMsg  Source message
void MESSAGE_AppendInfo(Node *node, Message *dstMsg, Message* srcMsg);

/// Returns the size of a message.  Used in place of sizeof() in
/// the kernel code to allow for users to add more fields to
/// the message.
///
/// \return sizeof(msg)
size_t MESSAGE_SizeOf();

/// Fragment one packet into TWO fragments
///
/// \note This API treats the original packet as raw packet
///    and does not take account of fragmentation related
///    information like fragment id. The caller of this API
///    will have to itself put in logic for distinguishing
///    the fragmented packets
///
/// \param node  node which is fragmenting the packet
/// \param msg  The packet to be fragmented
/// \param fragmentedMsg  First fragment
/// \param remainingMsg  Remaining packet
/// \param fragUnit  The unit size for fragmenting the packet
/// \param protocolType  Protocol type for packet tracing.
/// \param freeOriginalMsg  If TRUE, then original msg is set to NULL
///
/// \return TRUE if any fragment is created, FALSE otherwise

BOOL
MESSAGE_FragmentPacket(
    Node* node,
    Message*& msg,
    Message*& fragmentedMsg,
    Message*& remainingMsg,
    int fragUnit,
    TraceProtocolType protocolType,
    bool freeOriginalMsg);


/// Reassemble TWO fragments into one packet
///
/// \note None of the fragments will be freed in this API.
///    The caller of this API will itself have to free
///    the fragments
///
/// \param node  node which is assembling the packet
/// \param fragMsg1  First fragment
/// \param fragMsg2  Second fragment
/// \param protocolType  Protocol type for packet tracing.
///
/// \return The reassembled packet.
Message*
MESSAGE_ReassemblePacket(
             Node* node,
             Message* fragMsg1,
             Message* fragMsg2,
             TraceProtocolType protocolType);

/// This function is used primarily by external interfaces to
/// inject events into the Simulator as soon as possible without
/// causing problems for parallel execution.
///
/// \param node  node which is sending message
/// \param msg  message to be delivered
void MESSAGE_SendAsEarlyAsPossible(Node *node, Message *msg);

void MESSAGE_RemoteSendSafely(
    Node* node,
    NodeId destNodeId,
    Message*  msg,
    clocktype delay);

/// Free memory for the message contents: Info fields, payload, and so on.
///
/// \param partition  partition which is freeing the message
/// \param msg  message which has to be freed
void MESSAGE_FreeContents(PartitionData *partition, Message *msg);

/// Perform debugging and tracing activities for this message
/// as it is sent
///
/// \param partition  partition which is debugging
/// \param node  node which is debugging.  May be null.
/// \param msg  message to debug and trace
void MESSAGE_DebugSend(PartitionData *partition, Node* node, Message* msg);

/// Perform debugging and tracing activities for this message
/// as it is processed
///
/// \param partition  partition which is debugging
/// \param node  node which is debugging.  May be null.
/// \param msg  message to debug and trace
void MESSAGE_DebugProcess(PartitionData *partition, Node* node, Message* msg);

#endif /* _MESSAGE_H_ */

