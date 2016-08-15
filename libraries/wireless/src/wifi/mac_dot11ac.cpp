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

/// \file mac_dot11ac.cpp
///
/// \brief Dot11ac cpp file
///
/// This file contains function definitions of dot11ac specific functions


#include "mac_dot11.h"
#include "mac_dot11ac.h"
#include "mac_dot11n.h"

/// \brief Implementation of function which initialises 802.11ac model
///
/// \param node Node pointer
/// \param networkType network type
/// \param nodeInput Pointer to node input
/// \param dot11 dot11 Pointer fo dot11 structure
void MacDot11acInit(Node *node,
                   NetworkType networkType,
                   const NodeInput* nodeInput,
                   MacDataDot11* dot11)
{
    Address address;
    BOOL wasFound = FALSE;
    BOOL parameterValue = FALSE;
    NetworkGetInterfaceInfo(
        node,
        dot11->myMacData->interfaceIndex,
        &address,
        networkType);

    IO_ReadBool(node,
                node->nodeId,
                dot11->myMacData->interfaceIndex,
                nodeInput,
                "MAC-DOT11-AMSDU-ENABLE",
                &wasFound,
                &parameterValue);
    if (wasFound)
    {
        if (parameterValue == TRUE)
        {
            dot11->isAmsduEnable = TRUE;
        }
        else
        {
           dot11->isAmsduEnable = FALSE;
        }
    }
    else
    {
       dot11->isAmsduEnable = FALSE;
    }

    IO_ReadBool(node,
                node->nodeId,
                dot11->myMacData->interfaceIndex,
                nodeInput,
                "MAC-DOT11-VHTTXOPPS-ENABLE",
                &wasFound,
                &parameterValue);
    if (wasFound)
    {
        if (parameterValue == TRUE)
        {
            dot11->isVHTTxopPSEnable = TRUE;
        }
        else
        {
            dot11->isVHTTxopPSEnable = FALSE;
        }
    }
    else
    {
        dot11->isVHTTxopPSEnable = FALSE;
    }

    IO_ReadBool(node,
                node->nodeId,
                dot11->myMacData->interfaceIndex,
                nodeInput,
                "MAC-DOT11-AMPDU-ENABLE",
                &wasFound,
                &parameterValue);
    if (wasFound)
    {
        if (parameterValue == TRUE)
        {
            dot11->isAmpduEnable = TRUE;
        }
        else
        {
           dot11->isAmpduEnable = FALSE;
        }
    }
    else
    {
       dot11->isAmpduEnable = FALSE;
    }

    if (dot11->isAmsduEnable)
    {
       dot11->amsduBuffer = new AmsduBuffer;
    }
    dot11->inputBuffer = new InputBuffer;

    clocktype inputBufferTimerExpireInterval;
    IO_ReadTime(
            node,
            node->nodeId,
            dot11->myMacData->interfaceIndex,
            nodeInput,
            "MAC-DOT11-REORDERING-BUFFER-TIMER-INTERVAL",
            &wasFound,
            &inputBufferTimerExpireInterval);

    if (wasFound)
    {
        if (inputBufferTimerExpireInterval <= 0)
        {
             ERROR_ReportError("Invalid Value Of Parameter"
                            " MAC-DOT11-REORDERING-BUFFER-TIMER-INTERVAL"
                            " Specified In The Configuration File");
        }
        dot11->inputBufferTimerExpireInterval =
                                          inputBufferTimerExpireInterval;
    }
    else
    {
       dot11->inputBufferTimerExpireInterval = 100 * MILLI_SECOND;
    }

    clocktype amsduBufferTimerExpireInterval;
    IO_ReadTime(
            node,
            node->nodeId,
            dot11->myMacData->interfaceIndex,
            nodeInput,
            "MAC-DOT11-AMSDU-BUFFER-TIMER-INTERVAL",
            &wasFound,
            &amsduBufferTimerExpireInterval);

    if (wasFound)
    {
        if (amsduBufferTimerExpireInterval <= 0)
        {
             ERROR_ReportError("Invalid Value Of Parameter"
                 " MAC-DOT11-AMSDU-BUFFER-TIMER-INTERVAL Specified"
                 " In The Configuration File");
        }
        dot11->amsduBufferTimerExpireInterval
            = amsduBufferTimerExpireInterval;
    }
    else
    {
       dot11->amsduBufferTimerExpireInterval = 5 * MILLI_SECOND;
    }

    int macOutputQueueSize;
    IO_ReadInt(
        node,
        node->nodeId,
        dot11->myMacData->interfaceIndex,
        nodeInput,
        "MAC-DOT11-QUEUE-SIZE-PER-DESTINATION",
        &wasFound,
        &macOutputQueueSize);

    if (wasFound)
    {
        if (macOutputQueueSize <= 0)
        {
             ERROR_ReportError("Invalid Value Of Parameter"
                 " MAC-DOT11-QUEUE-SIZE-PER-DESTINATION Specified"
                 " In The Configuration File");
        }
        dot11->macOutputQueueSize = macOutputQueueSize;
    }
    else
    {
       dot11->macOutputQueueSize = DEFAULT_MAC_QUEUE_SIZE;
    }

    Int32 mpduLenIdx = 0;
    IO_ReadInt(node,
                node->nodeId,
                dot11->myMacData->interfaceIndex,
                nodeInput,
                "MAC-DOT11-MPDU-LENGTH-INDEX",
                &wasFound,
                &mpduLenIdx);
    if (wasFound)
    {
        ERROR_Assert(mpduLenIdx >=0 && mpduLenIdx < 3,
                "The range of values for MPDU length"
                " index is [0 to 2]");
        dot11->vhtMaxMpduSize = mpduLenIdx;
        if (mpduLenIdx == 0)
        {
            dot11->maxAmsduSize = AMSDU_SIZE_1;
        }
        else
        {
            dot11->enableBigAmsdu = TRUE;
            dot11->maxAmsduSize = AMSDU_SIZE_2;
        }
    }
    else
    {
       dot11->enableBigAmsdu = FALSE;
       dot11->maxAmsduSize = AMSDU_SIZE_1;
       dot11->vhtMaxMpduSize = 0;
    }

    int ampduLengthExponent = 0;
    IO_ReadInt(
        node,
        node->nodeId,
        dot11->myMacData->interfaceIndex,
        nodeInput,
        "MAC-DOT11-AMPDU-LENGTH-EXPONENT",
        &wasFound,
        &ampduLengthExponent);

    if (wasFound)
    {
        if (ampduLengthExponent < 0 || ampduLengthExponent > 7)
        {
              ERROR_ReportError("The range of values for"
                      " AMPDU length exponent is [0 to 7]");
        }
        else
        {
            dot11->vhtApmduLengthExponent = (UInt8)ampduLengthExponent;
            if (ampduLengthExponent < 4)
            {
                dot11->ampduLengthExponent = ampduLengthExponent;
            }
            else
            {
                dot11->ampduLengthExponent = 3;
            }
        }
    }
    else
    {
       dot11->vhtApmduLengthExponent = 1;
       dot11->ampduLengthExponent = 1;
    }
    
    dot11->vhtMaxAmpduSize =
       MacDot11nCalculateMaxAmpduLength(dot11->vhtApmduLengthExponent);

    dot11->enableImmediateBAAgreement = FALSE;
    dot11->enableDelayedBAAgreement = FALSE;
}
