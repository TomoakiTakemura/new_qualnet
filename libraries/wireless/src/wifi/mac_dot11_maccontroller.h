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


/// \file mac_dot11_maccontroller.h
///
/// \brief Dot11 channel access manager
///
/// This file contains the class hierarchy for channel access

#ifndef __MAC_DOT11_CONTROLLER_H__
#define __MAC_DOT11_CONTROLLER_H__

#include "mac_dot11.h"
#include "mac.h"

/*** UAPSD *********************************START*******************/
#define DOT11_UAPSD_STATS_LABEL "UAPSD"
/*** UAPSD *********************************END*******************/

namespace Dot11
{

namespace Qos
{

class AcManager;
class AccessCategory;
class Dot11Controller;
/// \brief Main Dot11 controller class
///
/// Currently it manages only the access category part.
/// In future, it would probably be the backbone controller for
/// dot11 family of models.
class MacController
{
    /// Node Pointer
    Node* m_node;
    /// Interface Index
    int m_interfaceIndex;
    /// Access Category Mode
    AcMode m_mode;
    /// Access Category Controller Reference
    AcManager* m_AcManager;
    /// Access Stats Controller Reference
    StatsController* m_sController;
    /// Dot11Controller Reference
    Dot11Controller* m_Dot11Controller;
public:
    /*!
    * \brief Initialize Dot11 controller
    */
    void initializeDot11Controller(AcMode mode, MacDataDot11* dot11);

    /*!
    * \brief Initialize Access category controller
    */
    void initializeAcManager(Node* node,
                             int interfaceIndex,
                             AcMode mode);

    /*!
    * \brief Start contention
    */
    void startContention(MacDataDot11* dot11);

    /*!
    * \brief Function to check if all Ac has packet
    */
    BOOL allAcHasPacket(MacDataDot11* dot11);

    /*!
    * \brief Interrupt Contention
    */
    void interruptContention(MacDataDot11* dot11);

    /*!
    * \brief Callback function after contention is successful
    */
    void contentionCallback(MacDataDot11* dot11, UInt8 acIndex);

    /*!
    * \brief To get Node Pointer
    */
    Node* node() {return m_node;}

    /*!
    * \brief To get Node Id
    */
    NodeId nodeId() {return m_node->nodeId;}

    /*!
    * \brief To get Ac Mode
    */
    AcMode getAcMode() {return m_mode;}

    /*!
    * \brief To get Interface Index
    */
    int ifidx() {return m_interfaceIndex;}

    /*!
    * \brief To return Access Category Mode
    */
    AcMode mode() {return m_mode;}

    /*!
    * \brief To get reference of Access Category Controller
    */
    AcManager* ac(){return m_AcManager;}

    /*!
    * \brief To get reference of Dot11 Controller
    */
    Dot11Controller* dc(){return m_Dot11Controller;}

/*** UAPSD *********************************START*******************/
    /*!
    * \brief Print UAPSD statistics
    */
    void uapsdPrintStats(
        Node* node,
        MacDataDot11* dot11,
        int interfaceIndex);
/*** UAPSD *********************************END*******************/

    /*!
    * \brief Layer Function of MacController
    */
    void layer(Message* msg)
    {
        MESSAGE_Free(node(), msg);
    }
    /*!
    * \brief UpdateStats API for PHY events
    */
    void updateStats(const std::string& eventName,
                    int size,
                    int overheadSize,
                    double signalPower,
                    double sinr = 0,
                    double rssi = 0,
                    double rsni = 0,
                    double pathloss = 0);

    /*!
    * \brief UpdateStats API for MAC events
    */
    void updateStats(const std::string& eventName,
                    int size,
                    int overheadSize,
                    const std::string& address);

    /*!
    * \brief UpdateStats API for NAV events
    */
    void updateStats(const std::string& eventName, clocktype navEndTime);

    /*!
    * \brief UpdateStats API for IFS events
    */
    void updateStats(const std::string& eventName,
                     const std::string& difs = "",
                     const std::string& sifs = "");

    /*!
    * \brief MacController Constructor
    */
    MacController(Node* node, 
                  int interfaceIndex,
                  AcMode mode, 
                  MacDataDot11* dot11)
        : m_node(node), m_interfaceIndex(interfaceIndex), m_mode(mode)
    {
        m_AcManager = NULL;
        m_sController = NULL;
        m_Dot11Controller = NULL;
        initializeDot11Controller(mode,dot11);
        initializeAcManager(node, interfaceIndex, mode);
    }

    /*!
    * \brief MacController Destructor
    */
    ~MacController();

};

/// \brief To get the Mac Controller Pointer
///
/// Function to get the Mac Controller pointer
///
/// \param dot11 Dot11 structure pointer
MacController* mc(MacDataDot11* dot11);

/// \brief To get the Access Manager reference
///
/// Function to get the Access Manager reference
///
/// \param dot11 Dot11 structure pointer
/// \param acIndex Ac Index
AccessCategory& getAc(MacDataDot11* dot11, int acIndex);

}
}

#endif
