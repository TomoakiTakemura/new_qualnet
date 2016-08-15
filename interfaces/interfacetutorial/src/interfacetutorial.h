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

#ifndef _INTERFACE_TUTORIAL_H_
#define _INTERFACE_TUTORIAL_H_

#include "external.h"
#include "external_socket.h"

/// Data that is used for this external interface
struct InterfaceTutorialData
{
    EXTERNAL_Socket listenSocket;
    EXTERNAL_Socket s;
};

//---------------------------------------------------------------------------
// External Interface API Functions
//---------------------------------------------------------------------------

/// This will listen for a socket connection on port 5132
///
/// \param iface  The interface
/// \param nodeInput  The configuration file data
///
void InterfaceTutorialInitializeNodes(
    EXTERNAL_Interface *iface,
    NodeInput *nodeInput);

/// This function will receive packets through the opened socket
///
/// \param iface  The interface
///
void InterfaceTutorialReceive(EXTERNAL_Interface *iface);

/// This function will send packets through the opened socket
///
/// \param iface  The interface
/// \param node  The node forwarding the data
/// \param forwardData  A pointer to the data to forward
/// \param forwardSize  The size of the data
///
void InterfaceTutorialForward(
    EXTERNAL_Interface *iface,
    Node* node,
    void *forwardData,
    int forwardSize);

/// This function will finalize this interface
///
/// \param iface  The interface
///
void InterfaceTutorialFinalize(EXTERNAL_Interface *iface);

#endif /* _INTERFACE_TUTORIAL_H_ */
