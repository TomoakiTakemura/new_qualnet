//
// Copyright (c) 2001-2015, SCALABLE Network Technologies, Inc.  All Rights Reserved.
// Reserved.
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

#include <map>
#include <iostream>
#include <map>

#include "api.h"
#include "message.h"
#include "node.h"
#include "propagation.h"
#include "clock.h"

#ifdef CYBER_LIB
#include "app_jammer.h"
#endif

/// \file jlm.cpp

// #define CONSIDER_WORMHOLE_AS_JAMMING 1

/// \brief Implementation of jammerFrameQ API
///
/// This function checks whether frame is a jammer frame
///
/// \param msg Jammer frame encapsulated in Message
///
/// \return True if frame is a jammer frame, false otherwise

bool JamFilter::jammerFrameQ(Message* msg)
{
#ifdef CYBER_LIB
    bool isJammerFrame(false);
    JamType* jam_p =
    reinterpret_cast<JamType*>(MESSAGE_ReturnInfo(msg, INFO_TYPE_JAM));
    if (jam_p != NULL && *jam_p == JAM_TYPE_APP)
    {
        isJammerFrame = true;
    }

    bool isWormholeFrame(false);
#if defined(CONSIDER_WORMHOLE_AS_JAMMING)
    if (msg->originatingProtocol == MAC_PROTOCOL_WORMHOLE)
    {
        isWormholeFrame = true;
    }
#endif

    return isJammerFrame || isWormholeFrame;

#else

    return false;

#endif
}

/// \brief Implementation of initialize API
///
/// Initialization funtion of JLM
///
/// \param nodeInput Pointer to nodeInput structure
///
/// \return n/a

void Jlm::initialize(const NodeInput* nodeInput) { ; }

