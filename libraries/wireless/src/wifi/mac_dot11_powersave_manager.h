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

/// \file mac_dot11_powersave_manager.h
///
/// \brief Dot11 power save Class hierarchy
///
/// This file contains the class hierarchy of Dot11 modes
#ifndef MAC_DOT11_PS_H
#define MAC_DOT11_PS_H
#include "mac_dot11.h"
#include "mac.h"
#include "mac_dot11_maccontroller.h"
#include "mac_phy_802_11n.h"

namespace Dot11
{
namespace Qos
{

/*!
* \brief Controller for vht power save
*/

class VhtPowerSave
{
public: 
    /*!
    * \brief Vht powersave controller constructor
    */
    VhtPowerSave() {}

    /*!
    * \brief Function to make the radio off
    */
    void stopListening(MacDataDot11* dot11, Node* node);

    /*!
    * \brief Function to make the radio on
    */
    void startListening(MacDataDot11* dot11, Node* node);

    /*!
    * \brief Function to check whether mode is enabled on both the ap and the station
    */
    BOOL isModeEnabled(MacDataDot11* dot11,
                       Mac802Address tempNextHopAddress,
                       Node* node);

    /*!
    * \brief Function to set the param in tx vector
    */
    void updateTxVector(MacDataDot11* dot11, 
                        Mac802Address tempNextHopAddress,
                        MAC_PHY_TxRxVector* txVector,
                        Node* node);

    /*!
    * \brief Function to check whether the station can go to sleep
    */
    BOOL canSleep(MacDataDot11* dot11,Node* node);

    /*!
    * \brief Vhtpowersave controller destructor
    */
    ~VhtPowerSave() {;}
};

/*!
* \brief Controller for Sm power save
*/

class SmPowerSave
{
public: 
    /*!
    * \brief sm powersave controller Constructor
    */
    SmPowerSave(){}

    /*!
    * \brief Function to return the mode of operation of SMPS
    */
    SmMode getMode(Node* node,
                   MacDataDot11* dot11);

    /*!
    * \brief Function to return the current rf chain mode
    */
    RfChainMode getCurrentRfChainMode(Node* node,
                                      MacDataDot11* dot11);
    /*!
    * \brief smpspowersave Controller Destructor
    */
    virtual ~SmPowerSave() {;}
};

/*!
* \brief Controller for all DOt11 modes
*/

class Dot11Controller
{
public:
    SmPowerSave* smps;
    VhtPowerSave* vhtps;
    /*!
    * \brief Dot11 controller constructor
    */
    Dot11Controller(){}

    /*!
    * \brief Function to go in sleep mode
    */
    virtual void sleep(MacDataDot11* dot11, Node* node){}

    /*!
    * \brief Function to switch back to wake up mode
    */
    virtual void wakeUp(MacDataDot11* dot11, Node* node){}

    /*!
    * \brief Function to switch back to wake up mode
    */
    virtual void dot11_UpdateTxVector(MacDataDot11* dot11,
                                      Mac802Address tempNextHopAddress,
                                      MAC_PHY_TxRxVector* txVector,
                                      Node* node){}

    /*!
    * \brief Function to switch the number of active antennas
    */
    virtual void switchRfChains(MacDataDot11* dot11,
                                Node* node,
                                RfChainMode mode){}

    /*!
    * \brief Decision controller destructor
    */
    ~Dot11Controller() {;}

};



/*!
 * \brief class for LegacyDot11 power save
 */
class LegacyController: public Dot11Controller
{
public:
    /*!
    * \brief Legacycontroller constructor
    */
    LegacyController()
    {
        smps = NULL;
        vhtps = NULL;
    }

    /*!
    * \brief LegacyMode controller destructor
    */
    ~LegacyController() {;}
};

/*!
 * \brief class for Dot11e power save
 */
class Dot11eController: public Dot11Controller
{
public:
    /*!
    * \brief Dot11econtroller constructor
    */
    Dot11eController()
    {
        smps = NULL;
        vhtps = NULL;
    }

    /*!
    * \brief Dot11eMode controller destructor
    */
    virtual ~Dot11eController() {;}
};

/*!
 * \brief class for Dot11n power save
 */
class Dot11nController: public Dot11Controller
{
public:
    /*!
    * \brief Dot11ncontroller constructor
    */
    Dot11nController(MacDataDot11* dot11)
    {
        if (dot11->smpsMode)
        {
            smps = new SmPowerSave();
        }
        vhtps = NULL;
    }

    /*!
    * \brief Function to switch the number of active antennas
    */
    void switchRfChains(MacDataDot11* dot11,
                        Node* node,
                        RfChainMode mode);

    /*!
    * \brief Dot11nMode controller destructor
    */
    ~Dot11nController() {;}
};

/*!
 * \brief class for Dot11ac power save
 */
class Dot11acController: public Dot11Controller
{
public:
    /*!
    * \brief Dot11accontroller constructor
    */
    Dot11acController(MacDataDot11* dot11) 
    {
        if (dot11->smpsMode)
        {
            smps = new SmPowerSave();
        }
        if (dot11->isVHTTxopPSEnable)
        {
            vhtps = new VhtPowerSave();
        }
    }

    /*!
    * \brief Function to go in sleep mode
    */
    void sleep(MacDataDot11* dot11, Node* node);

    /*!
    * \brief Function to switch back to wake up mode
    */
    void wakeUp(MacDataDot11* dot11, Node* node);

    /*!
    * \brief Function to update tx vector
    */
    void dot11_UpdateTxVector(MacDataDot11* dot11,
                              Mac802Address tempNextHopAddress, 
                              MAC_PHY_TxRxVector* txVector, 
                              Node* node);

    /*!
    * \brief Function to switch the number of active antennas
    */
    void switchRfChains(MacDataDot11* dot11,
                        Node* node,
                        RfChainMode mode);

    /*!
    * \brief Dot11acMode Controller Destructor
    */
   ~Dot11acController() {;}
};

} // namespace Qos

} // namespace Dot11

#endif