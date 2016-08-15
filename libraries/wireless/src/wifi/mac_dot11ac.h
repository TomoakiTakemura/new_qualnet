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


/// \file mac_dot11ac.h
///
/// \brief Dot11ac header file
///
/// This file contains function declarations

#ifndef __MAC_DOT11_AC_H__
#define __MAC_DOT11_AC_H__

#include "mac_dot11.h"

/*!
* \brief Function to initialize dot11ac model
*/
void MacDot11acInit(Node *node,
                   NetworkType networkType,
                   const NodeInput* nodeInput,
                   MacDataDot11* dot11);

#endif
