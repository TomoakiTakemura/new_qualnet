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

// PROTOCOL     :: SIP
//
// SUMMARY      :: Session Initiation Protocol(SIP) is the signalling
/// protocol used to set up, modify and terminate multimedia
/// sessions over TCP/IP network. It is highly scalable,
/// flexible to extension and simpler than similar protocols.
/// 
// LAYER        :: Application
/// 
// STATISTICS   ::
/// + inviteSent  : Number of INVITE requests sent
/// + ackSent     : Number of ACK requests sent
/// + byeSent     : Number of BYE requests sent
/// 
/// + inviteRecd  : Number of INVITE requests received
/// + ackRecd     : Number of ACK requests received
/// + byeRecd     : Number of BYE requests received
/// 
/// + tryingSent  : Number of TRYING responses sent
/// + ringingSent : Number of RINGING responses sent
/// + okSent      : Number of OK responses sent
/// 
/// + tryingRecd  : Number of TRYING responses received
/// + ringingRecd : Number of RINGING responses received
/// + okRecd      : Number of OK responses received
/// 
/// + reqForwarded  : Number of requests forwarded
/// + reqRedirected : Number of requests redireced
/// + reqDropped    : Number of requests dropped
/// 
/// + respForwarded : Number of responses forwarded
/// + respDropped   : Number of responses dropped
/// 
/// + callAttempted : Number of calls/sessions attempted
/// + callConnected : Number of calls/sessions successfully connected
/// + callRejected  : Number of calls/sessions rejected

// CONFIG_PARAM ::
// + MULTIMEDIA-SIGNALLING-PROTOCOL : SIP|H323 : This parameter selects
// one of the available signalling protocols
// + SIP-PROXYLIST : list of int : Comma delimited list of Proxy servers
// + SIP-TRANSPORT-LAYER-PROTOCOL : TCP|UDP|SCTP : selects one of
// transport protocols.
// + SIP-CALL-MODEL : DIRECT|PROXY-ROUTED : Specifies whether the call
// proceeds directly to the called party bypassing proxies or goes
// through the proxies.
// + SIP-STATISTICS : BOOLEAN : Selects if the statistics generated after
// simulation is to be printed or not.
// + TERMINAL-ALIAS-ADDRESS-FILE : TERMINAL-ALIAS-ADDRESS-FILE : Specifies
// the file containing details of the sip nodes
//
// VALIDATION   ::
//
// IMPLEMENTED_FEATURES ::
// + Transport layer protocol : SIP implemented over TCP
// + Two modes of call Establishment : SIP can route calls directly to
// the callee or it can be routed through proxy server
//
// OMITTED_FEATURES     ::
// + Transport layer protocol : UDP/ SCTP are not implemented
// + CANCEL, REGISTER, OPT : These three requests not implemented
// + Inter-ProxyServer Routing : Routing in between two proxy servers
//
// ASSUMPTIONS  ::
// + The users are not mobile,i.e. they are hooked to a single host or
// IP address and it is read initially from the above alias address file
//
// RELATED  ::
// H.323, RTP, RTCP, MGCP

#ifndef SIP_H
#define SIP_H

#include "app_util.h"
#include "multimedia_sipdata.h"
#include "multimedia_sipsession.h"

//---------------------------------------------------------------------------
// PROTOTYPES FOR INTERFACES(WITH EXTERNAL LINKAGE)
//---------------------------------------------------------------------------
/// Initializes the nodes running SIP protocol
///
/// \param node  Pointer to the node to be initialized
/// \param nodeInput  Pointer to input configuration file
/// \param hostType  Type of Sip Host
///
void        SipInit(Node* node,
                    const NodeInput *nodeInput,
                    SipHostType hostType,
                    clocktype waitTime);


/// Initializes the SIP proxy nodes
///
/// \param firstNode  Pointer to the first node
/// \param nodeInput  Pointer to input configuration file
///
void        SipProxyInit(Node* firstNode, const NodeInput* nodeInput);


/// Handles the SIP related events generated during simulation
///
/// \param node  Pointer to the node that receives the event
/// \param msg  Pointer to the received message
///
void        SipProcessEvent(Node* node, Message* msg);


/// Prints the statistics data from simulation for a node
///
/// \param node  Pointer to the node getting finalized
///
void        SipFinalize(Node* node);


/// Interface, for the third party application(now VoIP)
/// using which a SIP node is informed of desire for call initiation
///
/// \param node  Pointer to the calling node
/// \param voipData  Pointer to calling application's data struct.
///
void        SipInitiateConnection(Node* node, void* voipData);


/// Interface, for the third party application(now VoIP)
/// using which a SIP node is informed of desire for call termination.
///
/// \param node  Pointer to the node wishing to terminate call
/// \param voipData  Pointer to calling application's data struct.
///
void        SipTerminateConnection(Node* node, void* voipData);

struct SipCallInfo
{
    char        from[SIP_STR_SIZE64];
    char        to[SIP_STR_SIZE64];
    char        callId[SIP_STR_SIZE64];
    int connectionId;
};
//---------------------------------------------------------------------------
// NAME:        SipIsDHCPClient()
// PURPOSE:     find whether the node is DHCP client at the interface index
// PARAMETERS:
// + node       : Node* : Node which is to be checked for DHCP client
// + nodeInput  : NodeInput* : The input file
// RETURN:
// BOOL.    : TRUE if DHCP client FALSE otherwise
//---------------------------------------------------------------------------
BOOL 
SipIsDHCPClient(Node* node, const NodeInput* nodeInput);

//--------------------------------------------------------------------------
// FUNCTION    :: SipUrlSessionStartCallBack
// PURPOSE     :: To update the client when URL is resolved for TCP based
//                SIP. This is a dummy function for SIP as we do not need to
//                update any data structure after the URL is resolved in this
//                case. This function has been kept only for consistency.
// PARAMETERS   ::
// + node       : Node*          : pointer to the node
// + serverAddr : Address*       : server address
// + sourcePort : UInt16         : source port
// + uniqueId   : Int32          : connection id
// +interfaceId : Int16          : interface index,
// +serverUrl   : std::string    : server URL
// +packetSendingInfo : AppUdpPacketSendingInfo* : information required to 
//                                                send buffered application 
//                                                packets in case of UDP 
//                                                based applications; not
//                                                used by SIP
// RETURN       :
// bool         : TRUE if application packet can be sent; FALSE otherwise
//                TCP based application will always return FALSE as 
//                this callback will initiate TCP Open request and not send 
//                packet
//---------------------------------------------------------------------------
bool
SipUrlSessionStartCallBack(
                     Node* node,
                     Address* serverAddr,
                     UInt16 sourcePort,
                     Int32 uniqueId,
                     Int16 interfaceId,
                     std::string& serverUrl,
                     AppUdpPacketSendingInfo* packetSendingInfo);

#endif  /* SIP_H */
