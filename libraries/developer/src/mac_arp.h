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

/// \defgroup Package_Address_Resulution_Protocol_header Address Resulution Protocol header

/// \file
/// \ingroup Package_Address_Resulution_Protocol_header
/// Data structures and parameters used in network layer
/// are defined here.

#ifndef ARP_H
#define ARP_H


/// The ARP cache timeout value is user configurable.
/// If not specified, the default ARP cache timeout value is
/// used, i.e, each entry lasts 20 minutes after it is created
/// RFC 1122:
/// Periodically time out cache entries, even if they are in
/// use. Note that this timeout should be restarted when the
/// cache entry is "refreshed".
#define ARP_DEFAULT_TIMEOUT_INTERVAL                 20 * MINUTE

/// Default buffer size to store IP packet, 
/// when mac address is not resolved
#define ARP_BUFFER_SIZE                              1

/// The cache entry of ARP never deleted
#define ARP_STATICT_TIMEOUT                          0

/// Number of attribute of ARP cache File
#define ARP_STATIC_FILE_ATTRIBUTE                    7

/// Entry type static in the ARP table
#define ARP_ENTRY_TYPE_STATIC                        0

/// Entry type dynamic in the ARP table
#define ARP_ENTRY_TYPE_DYNAMIC                       1

/// From KA9Q NET/ROM pseudo
/// ARP protocol HARDWARE identifiers.
#define ARP_HRD_NETROM                               0

/// Ethernet 10/100Mbps
/// ARP protocol HARDWARE identifiers.
#define ARP_HRD_ETHER                                1

/// ARP protocol hardware type Experimental Ethernet
#define ARP_HRD_EETHER                               2

/// ARP protocol hardware type AX.25 Level 2
#define ARP_HRD_AX25                                 3

/// ARP protocol hardware type PROnet token ring
#define ARP_HRD_PRONET                               4

/// ARP protocol hardware Chaosnet
#define ARP_HRD_CHAOS                                5

/// IEEE 802.2 Ethernet/TR/TB
#define ARP_HRD_IEEE802                              6

/// Hardware type ARCnet
#define ARP_HRD_ARCNET                               7

/// Hardware type APPLEtalk
#define ARP_HRD_APPLETLK                             8

/// Frame Relay DLCI
#define ARP_HRD_DLCI                                 15

/// ATM 10/100Mbps
#define ARP_HRD_ATM                                  19

/// Hardware type ARPHRD_METRICOM
#define ARPHRD_METRICOM                              23

/// Hardware type ARPHRD_IEEE_1394
#define ARPHRD_IEEE_1394                             24

/// Hardware identifier
#define ARPHRD_EUI_64                                27

/// Unknown Hardware type
/// ARP protocol HARDWARE identifiers.
#define ARP_HRD_UNKNOWN                              0xffff

/// IPv4 protocol address size in byte
#define ARP_PROTOCOL_ADDR_SIZE_IP                    4





/// RARP type
#define PROTOCOL_TYPE_RARP                           0x08035

/// Queue Size in bytes
#define ARP_QUEUE_SIZE 30000

/// Maximum Retry count
#define MAX_RETRY_COUNT 5

/// The opcode field in ArpPacket represents different type of
/// opcode
typedef enum arp_opcode
{
    ARPOP_INVALID,
    ARPOP_REQUEST,          // ARP request
    ARPOP_REPLY,            // ARP reply
    ARPOP_RREQUEST,         // RARP request
    ARPOP_RREPLY,           // RARP reply
    ARPOP_InREQUEST = 8,    // InARP request
    ARPOP_InREPLY,          // InARP reply.
    ARPOP_NAK,              // (ATM)ARP NAK.
} ArpOp;


/// Struture of the ARP message.
typedef struct arp_packet_str
{
    unsigned short hardType;
    unsigned short protoType;
    unsigned char hardSize;
    unsigned char protoSize;
    unsigned short opCode;

    unsigned char  s_macAddr[MAX_MACADDRESS_LENGTH];
    NodeAddress s_IpAddr;
    unsigned char d_macAddr[MAX_MACADDRESS_LENGTH];
    NodeAddress d_IpAddr;

} ArpPacket;


/// Struture of the element of Arp translation Table.
typedef struct arp_translation_table_element_str
{
    NodeAddress protocolAddr;
    unsigned char  hwAddr[MAX_STRING_LENGTH];
    unsigned short protoType;
    unsigned short hwType;
    unsigned short hwLength;
    clocktype expireTime;
    unsigned int interfaceIndex;
    BOOL entryType;  //for Static ARP Cache Entries
                     //the value is zero otherwise 1
} ArpTTableElement;


/// Struture of the ARP Buffer.
typedef struct arp_buffer
{
    NodeAddress nextHopAddr;
    int interfaceIndex;
    int incomingInterface;
    TosType priority;
    int networkType;
    Message *buffer;
} ArpBuffer;


/// Struture of the ARP statistics collection.
typedef struct arp_stat_str
{

    unsigned totalReqSend;
    unsigned totalReplySend;
    unsigned totalReqRecvd;
    unsigned totalReplyRecvd;

#if 0
    //Count the number of proxy reply done
    //The proxy reply also included into the arpReply
    unsigned totalProxyReply;
#endif
    //The Gratuitous request also included into the arpRequest
    unsigned totalGratReqSend;

    unsigned numPktDiscarded;

    // ARP Cache Statistics
    unsigned totalCacheEntryCreated;
    // Change in Hardware Address
    unsigned totalCacheEntryUpdated;
    unsigned totalCacheEntryAgedout;
    // Age out +  deletion due to fault
    unsigned totalCacheEntryDeleted;
    unsigned arppacketdropped;
} ArpStat;


/// Struture of the ARP request Sent Database.
/// A mechanism to prevent ARP flooding (repeatedly sending an
/// ARP Request for the same IP address, at a high rate) MUST
/// be included.  The recommended maximum rate is 1 per second
/// per destination (RFC 1122)
typedef struct arp_request_sentdb_str
{
    NodeAddress protocolAddr;
    clocktype   sentTime;
    int         RetryCount;
    LinkedList* arpBuffer;
    // DHCP
    bool isDHCP;
} ArpRequestSentDb;


/// Structure hold the information related to interface
typedef struct arp_data_interface_info
{
    unsigned short  hardType;
    unsigned short  protoType;
    BOOL            isEnable;
    LinkedList*     arpDb;
    Queue*          requestQueue;
    BOOL            isArpBufferEnable;
    int             maxBufferSize;
    BOOL            isArpStatEnable;
    struct arp_stat_str stats;
} ArpInterfaceInfo;

/// Main Data Struture of ARP at each interface.
typedef struct arp_data_str
{
    ArpInterfaceInfo    interfaceInfo[MAX_NUM_INTERFACES];
    LinkedList*         arpTTable;
    clocktype           arpExpireInterval;
    int                 maxRetryCount;
} ArpData;


/// Struture stored in MacData where data structures related to
/// Address Resolution Protocols like ARP, RARP, etc are stored.
typedef struct address_resolution_module
{
    ArpData* arpData;
    ArpData* rarpData;
} AddressResolutionModule;


// FUNCTION     ArpReceivePacket
// PURPOSE:     This is the API function between IP and ARP.
/// Receive ARP packet
/// PARAMETERS   Node *node
/// Pointer to node.
/// msg
/// which will be process.
/// interfaceIndex
/// incoming interface.
void ArpReceivePacket(
         Node* node,
         Message* msg,
         int interfaceIndex);

// PURPOSE:   ::  Sneak ARP packet, node is working in promiscous mode
///
///    + node : Node* Pointer to node.
///    + msg :  Message pointer 
///    + interfaceIndex :incoming interface.
///
 //**/
void ArpSneakPacket(
         Node* node,
         Message* msg,
         int interfaceIndex);

/// This is the API function between IP and ARP.
/// This function search for the Ethernet address
/// corresponding to protocolAddress from ARP
/// cache or Translation Table.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  outgoing interface.
/// \param protoType  Protocol Type
///    + priority : TosType  :
/// \param protocolAddress  Ethernet address surch for
///    this Ip address.
/// \param macAddr  Ethernet address corresponding
///    Ip address if exist.
/// \param msg  Message pointer
/// \param  incomingInterface  Incoming Interface
/// \param  networkType  Network Type
///
/// \return If success TRUE

BOOL ArpTTableLookup(
         Node* node,
         int interfaceIndex,
         unsigned short protoType,
         TosType priority,
         NodeAddress protocolAddress,
         MacHWAddress* macAddr,
         Message** msg,
         int incomingInterface,
         int networkType);

/// Initialize ARP all data structure and configuration
///
/// \param node  Pointer to node.
/// \param nodeInput  Pointer to node input.
///
void ArpInit(
         Node* node,
         const NodeInput* nodeInput);

/// Handle Interface Fault Situations
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param isInterfaceFaulty  Denotes interface fault status.
///
void ArpHandleHWInterfaceFault(
         Node* node,
         int interfaceIndex,
         BOOL isInterfaceFaulty);

/// This function is used for Proxy ARP reply.
/// It is special used by home agent or gateway to
/// to response in absence of host in home network.
///
/// \param node  Pointer to node.
/// \param msg  arp Request or Reply message
/// \param interfaceIndex  Interface Index
/// \param proxyIp  The Ip address of the host
///    whose proxy has been given.
///
void ArpProxy(
         Node* node,
         Message* msg,
         int interfaceIndex,
         NodeAddress proxyIp);

/// This function creates an ARP reply packet
/// for the node which is not in same home network.
///
/// \param node  Pointer to node which indiates the host.
/// \param arpData  arp main Data structure of this interface.
/// \param opCode  Type of operation that is Reply in case of
///    proxy arp.
/// \param proxyAddress  used for proxy IP
///    + destIPAddr   : NodeAddress :
/// \param dstMacAddr  Destination hardware address.
///    + interfaceIndex: int        :
///
/// \return Pointer to a Message structure
Message* ArpProxyCreatePacket(
             Node* node,
             ArpData* arpData,
             ArpOp opCode,
             NodeAddress proxyAddress,
             NodeAddress destAddr,
             MacHWAddress& dstMacAddr,
             int interfaceIndex);

/// This function is used for Gratuitous ARP reply.
/// It is special used by home agent or gateway to
/// to update the others ARP cache when the host
/// (which is responsible) came to know that the
/// host is not at home network. Gratuitous may be either
/// ARP request or ARP reply. Here ARP-request has been used
///
/// \param node  Pointer to node that is host which
///    is either gateway or home agent.
/// \param msg  arp Request or Reply message
/// \param interfaceIndex  Interface Index
/// \param gratIPAddress  The Ip address of the host
///    whose mac address has beenupdated
///    in the ARP-cache table of other nodes.
///
void ArpGratuitous(
         Node* node,
         Message* msg,
         int interfaceIndex,
         NodeAddress gratIPAddress);

/// This function creates an ARP reply packet
/// for the node which is not in same home network.
///
/// \param node  Pointer to node which indiates the host.
/// \param arpData  arp main Data structure of this interface.
/// \param opCode  The ARP message type
/// \param gratIPAddress  The ip against whose the MAC addess
///    will be updated.
/// \param destIPAddr  The node who has send the ARP-request
/// \param dstMacHWAddr  The hardware address
/// \param interfaceIndex  Interface of the node
///
/// \return Pointer to a Message structure
Message* ArpGratuitousCreatePacket(
             Node* node,
             ArpData* arpData,
             ArpOp opCode,
             NodeAddress gratIPAddress,
             NodeAddress destIPAddr,
             MacHWAddress& dstMacAddr,
             int interfaceIndex);

/// Handle ARP events
///
/// \param node  Pointer to node.
/// \param msg  Pointer to message.
///
void ArpLayer(
         Node* node,
         Message* msg);

/// Finalize function for the ARP protocol.
///
/// \param node  Pointer to node.
///
void ArpFinalize(Node* node);

/// Print out packet trace information.
///
/// \param node  node printing out the trace information.
/// \param msg  packet to be printed.
///
void ArpPrintTrace(Node *node, Message *msg);

/// The function is used whenever required
/// to get the hardware address against the ip address
///
/// \param node  Pointer to node whome require to know the
///    hardware address
/// \param protocolAddress  The ip-address whose hardware
///    address is looking for.
/// \param interfaceIndex  The interface number of the protocol address
///    + hwType     : unsigned short :
/// \param hwAddr  The hardware address has been assigned
///    to the character pointer
///
/// \return Return the boolean value
BOOL ArpLookUpHarwareAddress(
         Node* node,
         NodeAddress protocolAddress,
         int interface,
         unsigned short hwType,
         unsigned char  *hwAddr);

/// This function is used to retrieve the interface through
/// which the neighbour has been connected to node
///
/// \param node  Pointer to node.
/// \param neighbourIp  The ip-address which will is the static
///    entry
/// \param nodeInterface  The interface through which the neighbourIp
///    has been connected
///
void ArpInterfaceChecking(
         Node* node,
         NodeAddress neighbourIp,
         int *nodeInterface);

/// It does check whether the ipAddressStr is either
/// nodeid-interface (nodeid-interface) or ip-address
/// if this is nodeid-interface, the values set into
/// nodeId and interface with the value and the BOOL set TRUE
/// Otherwise the BOOL value is FALSE
///
///    + ipAddressStr[] : const char :
///    Character string which has either nodeid-interface or
///    ip-address string
///    - the multicast protocol to get.
///    interfaceId - specific interface index or ANY_INTERFACE
/// \param nodeId  The value to be set after extracting the
///    nodeid from string
/// \param intfIndex  The value to be set after extracting the interface
///    from string
/// \param isNodeId  The value to be set TRUE if the string is
///    nodeid-interface otherwise FALSE
///
void ArpSeperateNodeAndInterface(
         const char ipAddressStr[],
         NodeAddress* nodeId,
         int* intfIndex,
         BOOL* isNodeId);

/// Convert the input ip address in character
/// format to NodeAddress
///
/// \param addressString  String which has ip-string
///    in character format
/// \param outputNodeAddress  The string value converted
///    to NodeAddress value
///
void ArpConvertStringAddressToNetworkAddress(
         const char addressString[],
         NodeAddress *outputNodeAddress);

/// The function is used for checking whether ARP is
/// enable or not.
///
/// \param node  Pointer to node.
///
/// \return The boolean (either True or false) value
BOOL ArpIsEnable(Node* node);

/// The function is used for checking whether ARP is
/// enable or not.
///
/// \param node  Pointer to node.
/// \param int  Pointer to node.
///
/// \return The boolean (either True or false) value
BOOL ArpIsEnable(Node* node, int interface);

/// This function enqueues an ARP packet into the ARP Queue
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface of the node
///    + msg:  Message* message to enqueue
/// \param arpData  arp main Data structure of this interface.
/// \param isFull  Queue is Full or not
/// \param nextHop  nextHop Ip address.
/// \param dstHWAddr  destination hardware address address

void EnqueueArpPacket(Node* node,
                      int interfaceIndex,
                      Message *msg,
                      ArpData *arpData,
                      BOOL *isFull,
                      NodeAddress nextHop,
                      MacHWAddress& destHWAddr);

/// This function dequeues an ARP packet from the ARP Queue
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface of the node
///    + msg:  Message** variable for the mesaage dequeued
/// \param nextHop  nextHop Ip address.
/// \param dstHWAddr  destination hardware address address
///    Return BOOL : True when message is successfully dequeued
BOOL DequeueArpPacket(Node *node,
                      int interfaceIndex,
                      Message **msg,
                      NodeAddress *nextHopAddr,
                      MacHWAddress *destMacAddr);

/// Checks if ARP queue is empty
///
///    + node : Node* Pointer to node
/// \param interfaceIndex  interface for which request queue is checked
///    RETURN: BOOL True when Queue is empty
///    **/
BOOL ArpQueueIsEmpty(Node *node,
                     int interfaceIndex);

/// Peeks at the top packet in  ARP queue
///
///    + node : Node* Pointer to node
/// \param interfaceIndex  interface for which request queue is checked
///    + msg:  Message** variable for the mesaage
/// \param nextHop  nextHop Ip address.
/// \param destMacAddr  destination hardware address address
///    Return BOOL : True when message is successfully found
///    **/

BOOL ArpQueueTopPacket(
                  Node *node,
                  int interfaceIndex,
                  Message **msg,
                  NodeAddress *nextHopAddr,
                  MacHWAddress *destMacAddr);

/// Finds IP address for a give Mac Address
///
///    + node : Node* Pointer to node
/// \param interfaceIndex  interface of the node
/// \param macAddr  hardware address
///    Return NodeAddress : Ip address for a mac address
///    **/
NodeAddress ReverseArpTableLookUp(
              Node* node,
              int interfaceIndex,
              MacHWAddress* macAddr);

/// Notify for ARP Drop
///
/// \param node  Pointer to node.
/// \param msg  Message pointer
/// \param nextHopAddress  Ip address of node
/// \param interfaceIndex  outgoing interface.
///
void ARPNotificationOfPacketDrop(Node *node,
                                 Message *msg,
                                 NodeAddress nextHopAddress,
                                 int interfaceIndex);
// DHCP
//---------------------------------------------------------------------------
// FUNCTION            :: ArpCheckAddressForDhcp
// LAYER       :: NETWORK
// PURPOSE             :: Validate the Ipaddress for DHCP
// PARAMETERS  ::
// + node : Node* : Pointer to node.
// + Address : ipAddress  : Address to check
// + incomingInterface : int  : interface for broadcast
// RETURN      ::
// BOOL ......result
//---------------------------------------------------------------------------
void ArpCheckAddressForDhcp(
             Node* node,
             Address ipAddress,
             Int32 interfaceIndex);
#endif ///* ARP_H */
