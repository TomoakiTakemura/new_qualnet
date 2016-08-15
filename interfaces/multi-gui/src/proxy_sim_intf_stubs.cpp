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

#include "api.h"
#include "network_ip.h"
#include "mac.h"
#include "gui.h"
#include "network.h"

// This file contains stubs for some functions called from core library
// files needed for loading the NodeInput structure, but which should be
// unreachable in the context of running proxy-sim.

// In all the following functions, any function which takes a Node* should
// be unreachable, as proxy-sim doesn't progress to the stage of creating
// Node objects.

/// \brief stub function for proxy-sim needed for loading the NodeInput
/// structure. Called from Address_IsSubnetBroadcastAddress in mapping.cpp
BOOL IsOutgoingBroadcast(Node *,
                         NodeAddress,
                         int *,
                         NodeAddress *)
{
    ERROR_ReportError("IsOutgoingBroadcast() should be unreachable in proxy-sim");
    return FALSE;
}

/// \brief stub function for proxy-sim needed for loading the NodeInput
/// structure. Called from MAPPING_CreateIpv6LinkLocalAddr in mapping.cpp
MacHWAddress GetMacHWAddress(Node* node, int interfaceIndex)
{
    ERROR_ReportError("GetMacHWAddress() should be unreachable in proxy-sim");
    return MacHWAddress();
}

/// \brief stub function for proxy-sim
/// Called from MAPPING_SetAddressInGUI in mapping.cpp
void GUI_SendAddressChange(
                        NodeId,
                        Int32,
                        Address,
                        NetworkType)
{
    ERROR_ReportError("GUI_SendAddressChange() should be unreachable in proxy-sim");
}

/// \brief stub function for proxy-sim
/// Called from IO_ReadStringUsingIpAddress in fileio.cpp
NetworkType NetworkIpGetInterfaceType(
    Node*,
    int)
{
    ERROR_ReportError("NetworkIpGetInterfaceType() should be unreachable in proxy-sim");
    return NETWORK_INVALID;
}

/// \brief stub function for proxy-sim
/// Called from IO_ReadStringUsingIpAddress in fileio.cpp
void NetworkGetInterfaceInfo(
    Node*,
    int,
    Address *,
    NetworkType)
{
    ERROR_ReportError("NetworkGetInterfaceInfo() should be unreachable in proxy-sim");
}

/// \brief stub function for proxy-sim
/// Called from GUI_SendReply in gui_core.cpp; this one could be reachable
/// in case of a socket failure or premature closure.
void PARTITION_RequestEndSimulation()
{
    extern bool g_requestedEndSimulation;
    g_requestedEndSimulation = true;
}
