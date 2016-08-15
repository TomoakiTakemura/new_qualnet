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

#ifndef _MULTI_GUI_INTERFACE_H_
#define _MULTI_GUI_INTERFACE_H_

#include "external_socket.h"

enum MultiGuiInterfaceMsgType
{
    SEND_MESG = 1,
    SEND_GUI_JOIN_EVENTS,
    REQUEST_OLDER_EVENTS
};

/// \brief Master Simulation Interface Initialize
///
/// this function is to read nodeInput and to open socket
/// connections to all proxies
///
/// \param iface  pointer to master simulation interface
/// \param nodeInput  pointer to data structure containing all
///                    configuration information
void MultiGuiIntf_Initialize(EXTERNAL_Interface *iface,
                            NodeInput *nodeInput);


/// \brief Master Simulation Interface Receive
///
/// this function is to read data from proxy connections
///
/// \param iface  Pointer to master simulation interface
void MultiGuiIntf_Receive(EXTERNAL_Interface *iface);


/// \brief Master Simulation Interface Send
///
/// this function is to send data to all proxies
///
/// \param outStr  string of data to be sent
/// \param size    size of string
/// \param size    connectionId  socket descriptor of the connection to be sent on (default to all)
void MultiGuiIntf_Send(char* outStr,
                        unsigned int size,
                        Int64 connectionId = -1);

/// \brief Master Simulation Interface Process Event
///
/// this function is to process event for Multi Gui Intf
///
/// \param node  pointer to node
/// \param msg   message to be processed
void MultiGuiIntf_ProcessEvent(Node* node, Message* msg);

/// \brief To send UUID to a proxy
///
/// \param connectSocket Pointer to proxy socket
void MultiGuiIntf_SendUidToClient(EXTERNAL_Socket* connectSocket);

/// \brief Master Simulation Interface Finalize
///
/// this function is to send GUI_FINISHED event to connected proxies
///
/// \param iface  pointer to master simulation interface
void MultiGuiIntf_Finalize (EXTERNAL_Interface *iface);

/// \brief Request for Older Events on new connections
///
/// \param iface  pointer to master simulation interface
/// \param connectionId  socket descriptor for new connection
void MultiGuiIntf_RequestForOlderEvents(EXTERNAL_Interface *iface, Int64 connectionId);

#ifdef AUTO_IPNE_INTERFACE
/// \brief Sends all mapping information to newlly joined GUI
///
/// \param partition  pointer to PartitionData of the current partition
/// \param connectionId  socket descriptor for new connection
void MultiGuiIntf_SendMapping(PartitionData* partition, Int64 connectionId);
#endif

/// \brief Sends node status (enabled or disabled and node position) for the nodes in the partition
///
/// \param partition  pointer to PartitionData of the current partition
/// \param connectionId  socket descriptor for new connection
void MultiGuiIntf_SendNodeInfo(PartitionData* partition, Int64 connectionId);

#endif

