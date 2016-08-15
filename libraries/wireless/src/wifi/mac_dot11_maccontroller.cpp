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

/// \file mac_dot11_maccontroller.cpp
///
/// \brief Dot11 channel access manager
///
/// This file contains the class hierarchy for channel access

#include "mac_dot11_maccontroller.h"
#include "mac_dot11_ac_manager.h"
#include "mac_dot11_powersave_manager.h"
#include "mac_dot11-sta.h"
#include "message.h"

namespace Dot11
{
namespace Qos
{
/// \brief Initialize Dot11 controller
///
/// This function will initialize the respective Dot11 Controller
///
/// \param node Node Pointer
/// \param mode Ac Mode
/// \param dot11 Dot11 Pointer
void MacController::initializeDot11Controller(AcMode mode,
                                              MacDataDot11* dot11)
{
    switch (mode)
    {
        case k_Mode_Legacy:
            m_Dot11Controller = new LegacyController();
            break;
        case k_Mode_Dot11e:
            m_Dot11Controller = new Dot11eController();
            break;
        case k_Mode_Dot11n:
            m_Dot11Controller = new Dot11nController(dot11);
            break;
        case k_Mode_Dot11ac:
            m_Dot11Controller = new Dot11acController(dot11);
            break;
    }
}

/// \brief Initialize Access category controller
///
/// This function will initialize the respective Ac Manager
///
/// \param node Node Pointer
/// \param interfaceIndex interface Index
/// \param mode Ac Mode
void MacController::initializeAcManager(Node* node,
                             int interfaceIndex,
                             AcMode mode)
{
    switch (mode)
    {
        case k_Mode_Legacy:
            m_AcManager = new LegacyAcManager(node, interfaceIndex, mode);
            break;
        case k_Mode_Dot11e:
            m_AcManager = new Dot11eAcManager(node, interfaceIndex, mode);
            break;
        case k_Mode_Dot11n:
            m_AcManager = new Dot11nAcManager(node, interfaceIndex, mode);
            break;
        case k_Mode_Dot11ac:
            m_AcManager = new Dot11nAcManager(node, interfaceIndex, mode);
            break;
    }
    m_sController = (StatsController*)PHY_GetStatsController(node,
                                node->macData[interfaceIndex]->phyNumber);
}

/// \brief Implementation of updateStats API
///
/// This function calls the API to update the stats for MAC layer
///
/// \param eventName Name of the event type
/// \param size Size of the msg
/// \param overheadSize Size of the header
/// \param address Address of the source/destination node
void MacController::updateStats(const std::string& eventName,
                                int size,
                                int overheadSize,
                                const std::string& address)
{
    if (m_sController)
    {
        m_sController->updateStat(eventName,
                              size,
                              overheadSize,
                              address);
    }
}

/// \brief Implementation of updateStats API
///
/// This function calls the API to update the stats for NAV management
///
/// \param eventName Name of the event type
/// \param navEndTime Value of nav end time
void MacController::updateStats(const std::string& eventName, clocktype navEndTime)
{
    if (m_sController)
    {
        m_sController->updateStat(eventName, navEndTime);
    }
}


/// \brief Implementation of updateStats API
///
/// This function calls the API to update the stats for IFS mangement
///
/// \param eventName Name of the event type
/// \param difs Value of difs/eifs
/// \param sifs Value of sifs/rifs
void MacController::updateStats(const std::string& eventName,
                                const std::string& difs,
                                const std::string& sifs)
{
    if (!m_sController)
    {
        return;
    }
    if (eventName == "IFS")
    {
        if (difs.empty() && sifs.empty())
        {
            return;
        }
        if (!difs.empty())
        {
            if (m_sController->difs().empty()
                || m_sController->difs() != difs)
            {
                m_sController->setDifs(difs);
                m_sController->updateStat(eventName, difs);
            }
        }
        if (!sifs.empty())
        {
            if (m_sController->sifs().empty()
                || m_sController->sifs() != sifs)
            {
                m_sController->setSifs(sifs);
                m_sController->updateStat(eventName, sifs);
            }
        }
    }
}

/// \brief Start contention
///
/// Function to start contention of the respective Ac manager
///
/// \param dot11 Dot11 Structure pointer
void MacController::startContention(MacDataDot11* dot11)
{
    m_AcManager->startContention(dot11);

}

/// \brief Interrupt contention
///
/// Function to Interrupt contention of the respective Ac manager
///
/// \param dot11 Dot11 structure pointer
void MacController::interruptContention(MacDataDot11* dot11)
{
    m_AcManager->interruptContention(dot11);
}

/// \brief Callback function after contention is successful
///
/// Function to set current Ac Index and set current message
///
/// \param dot11 Dot11 structure pointer
/// \param acIndex Ac Index
void MacController::contentionCallback(MacDataDot11* dot11, UInt8 acIndex)
{
    dot11->currentACIndex = acIndex;
    this->ac()->setCurrentMessage(dot11, dot11->currentACIndex);
    dot11->BO = 0;
    getAc(dot11,dot11->currentACIndex).setBackoffInterval(0);
    if (dot11->currentMessage != NULL || dot11->instantMessage != NULL)
    {
        MacDot11StationTransmitFrame(node(), dot11);
    }
    else
    {
        MacDot11StationSetState(node(), dot11, DOT11_S_IDLE);
    }
}

/// \brief To get the Mac Controller Pointer
///
/// Function to get the Mac Controller pointer
///
/// \param dot11 Dot11 structure pointer
MacController* mc(MacDataDot11* dot11)
{
    return ((MacController*)(dot11->macController));
}

/// \brief To get the Access Manager reference
///
/// Function to get the Access Manager reference
///
/// \param dot11 Dot11 structure pointer
/// \param acIndex Ac Index
AccessCategory& getAc(MacDataDot11* dot11, int acIndex)
{
    return (((MacController*)(dot11->macController))->ac()->getAc(acIndex));
}


/// \brief Function to check if all Ac has packet
///
/// This function will check if all Ac has packet
///
/// \param dot11 Dot11 structure pointer
BOOL MacController::allAcHasPacket(MacDataDot11* dot11)
{
    ERROR_Assert(m_mode != k_Mode_Legacy,
                 "Access category is not supported in Legacy 802.11 model");

    BOOL allAcHasPacket = TRUE;
    for (int i = 0; i < 4; i++)
    {
        if (!(getAc(dot11, i).m_frameInfo || getAc(dot11, i).isAcHasPacket()))
        {
            allAcHasPacket = FALSE;
            break;
        }
    }
    return allAcHasPacket;
}

/*** UAPSD *********************************START*******************/
/// \brief Function to print UAPSD statistics
///
/// This function will print UAPSD related statistics
///
/// \param node Pointer to node data structure
/// \param dot11 Pointer to Dot11 data structure
/// \param interfaceIndex Interface Index
void MacController::uapsdPrintStats(
    Node* node,
    MacDataDot11* dot11,
    int interfaceIndex)
{
    char buf[MAX_STRING_LENGTH];
    if (MacDot11IsBssStation(dot11)
        && dot11->isUapsdEnable)
    {
        sprintf(buf, "Explicit Trigger Frames Sent = %d",
            dot11->uapsdInfo.numTriggerSent);

        IO_PrintStat(
            node,
            "MAC",
            DOT11_UAPSD_STATS_LABEL,
            ANY_DEST,
            interfaceIndex,
            buf);

        sprintf(buf, "DTIM Frames Received = %d",
            dot11->psModeDTIMFrameReceived);

        IO_PrintStat(
            node,
            "MAC",
            DOT11_UAPSD_STATS_LABEL,
            ANY_DEST,
            interfaceIndex,
            buf);

        sprintf(buf, "TIM Frames Received = %d",
            dot11->psModeTIMFrameReceived);

        IO_PrintStat(
            node,
            "MAC",
            DOT11_UAPSD_STATS_LABEL,
            ANY_DEST,
            interfaceIndex,
            buf);
    }

    if (MacDot11IsAp(dot11)
        && dot11->isUapsdEnable)
    {

        sprintf(buf, "DTIM Frames Sent = %d",
            dot11->psModeDTIMFrameSent);

        IO_PrintStat(
            node,
            "MAC",
            DOT11_UAPSD_STATS_LABEL,
            ANY_DEST,
            interfaceIndex,
            buf);

        sprintf(buf, "TIM Frames Sent = %d",
            dot11->psModeTIMFrameSent);

        IO_PrintStat(
            node,
            "MAC",
            DOT11_UAPSD_STATS_LABEL,
            ANY_DEST,
            interfaceIndex,
            buf);

        sprintf(buf, "Explicit Trigger Frames Received = %d",
            dot11->uapsdInfo.numTriggerReceived);

        IO_PrintStat(
            node,
            "MAC",
            DOT11_UAPSD_STATS_LABEL,
            ANY_DEST,
            interfaceIndex,
            buf);

        sprintf(buf, "Broadcast Data Packets Sent = %d",
            dot11->broadcastDataPacketSentToPSModeSTAs);

        IO_PrintStat(
            node,
            "MAC",
            DOT11_UAPSD_STATS_LABEL,
            ANY_DEST,
            interfaceIndex,
            buf);

        sprintf(buf, "Unicast Data Packets Sent = %d",
            dot11->unicastDataPacketSentToPSModeSTAs);

        IO_PrintStat(
            node,
            "MAC",
            DOT11_UAPSD_STATS_LABEL,
            ANY_DEST,
            interfaceIndex,
            buf);
    }// end of if
}
/*** UAPSD *********************************END*******************/

}

}
