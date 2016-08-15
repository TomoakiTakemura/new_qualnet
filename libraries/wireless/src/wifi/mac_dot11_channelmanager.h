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



/// \file mac_dot11_channelmanager.h
///
/// \brief Dot11 channel access manager
///
/// This file contains the class hierarchy for channel access

#ifndef __MAC_DOT11_CHANNEL_H__
#define __MAC_DOT11_CHANNEL_H__


#include "api.h"
#include "partition.h"
#include "dynamic.h"
#include "mac_dot11_maccontroller.h"

#include "phy.h"
#include "interference.h"
class Phy802_11Manager;


namespace Dot11
{

namespace Qos
{

class Phy802_11ac;

/*!
 * \brief Frequency bands supported in 802.11
 */
enum FreqBand
{
    GHz_2_4 = 0,
    GHz_5 = 1
};

/*!
 * \brief The class defines a subchannel and various operations on it
 */
class SubChannel
{
    /*!
     * \brief Number of sub-carriers in a sub-channel
     */
    int m_numSubCarriers;

    /*!
     * \brief Channel index of center frequency
     */
    Int32 m_chIndex;

    /*!
     * \brief Subchannel PHY State
     */
    PhyStatusType m_phyState;

    /*!
     * \brief Interference at a sub-channel
     */
    double m_interference_mW;

public:

    /*!
     * \brief Returns the interference at a sub-channel
     */
    double getInterference_mW() { return m_interference_mW;}

   /*!
    * \brief Sets the sub-channel PHY state
    */
    void setSChannelPhyState( PhyStatusType state ) { m_phyState = state;}

    /*!
     * \brief Check if the sub-channel is idle
     */
    bool isSubChannelIdle()
    {
        if (m_phyState == PHY_IDLE)
        {
            return true;
        }
        return false;
    }

    /*!
     * \brief Returns the channel index
     */
    UInt32 getChIndex() {return m_chIndex;}

    /*!
     * \brief Set the channel index
     */
    void setChIndex(Int32 p_chIndex) { m_chIndex = p_chIndex;}

    /*!
     * \brief Updates the interference at a sub-channel
     */
    void updateInterference(double rxPower, bool addInterefence)
    {
        if (addInterefence)
        {
            m_interference_mW += rxPower;
        }
        else
        {
            m_interference_mW -= rxPower;
            if (m_interference_mW < 0)
            {
                m_interference_mW = 0;
            }
        }
    }

    /*!
     * \brief Constructor
     */
    SubChannel(int numSubCarriers, /*double startFreq,*/
        PhyStatusType phyState, double inteference_mw,
        Int32 chIndex)
    : m_numSubCarriers(numSubCarriers) , /*m_freq(startFreq),*/
      m_chIndex(chIndex),
      m_phyState(phyState),
      m_interference_mW(inteference_mw){;}

    /*!
     * \brief Destructor
     */
    ~SubChannel(){};
};

/*!
 * \brief Class ChannelController
 *
 * This class contains the sub-channel information of an interface and
 * defines various functions for sub-channel manipulations
 */
class ChannelController
{
private:
    /*!
     * \brief Number of sub-channel configured
     */
    int m_numSubChannels;

    /*!
     * \brief Sub-channel list at an interface
     */
    std::vector<SubChannel*> m_subChList;

    /*!
     * \brief spectral band list at an interface
     */
    std::vector<spectralBand*> m_sbList;


    /*!
     * \brief Place holder for Nav timer.
     */
    Message* m_navTimer;

    /*!
     * \brief Reference of MAC controller
     */
    MacController* m_macController;

    /*!
     * \brief Reference of PHY802.11Manager PHY structure
     */
    Phy802_11Manager* m_phy802_11Manager;

    /*!
     * \brief Dot11 modes of operation
     */
    AcMode m_mode;

    /*!
     * \brief Maximum Channel bandwidth configured at an interface
     */
    ChBandwidth m_chBwdth;

    /*!
     * \brief Frequency band configured at an interface
     */
    int m_freqBand;

    /*!
     * \brief Radio overlay id of interface
     */
    int m_radioOverlayId;

public:

    /*!
    * \brief Function to get Mac Controller
    */
    MacController* mc() { return m_macController; }

    /*!
     * \brief Stores the phymanager class pointer
     */
    void setManager(Phy802_11Manager* phyMngr)
    {
        m_phy802_11Manager = phyMngr;
    }

    /*!
    * \brief Function to get 802.11ac phy reference
    */
    Phy802_11Manager* phyManager() { return m_phy802_11Manager; }

    /*!
    * \brief Function to get the node pointer
    */
    Node* node() { return mc()->node(); }

    /*!
    * \brief Function to get the interface index
    */
    int ifidx() { return mc()->ifidx(); }

    /*!
     * \brief Set the 802.11 mode
     */
    void setMode(AcMode mode) {m_mode = mode;}

    /*!
     * \brief Get the 802.11 mode
     */
    AcMode getMode() { return m_mode;}

    /*!
     * \brief Get Radio Overlay Id
     */
    int getRadioOverlayId() { return m_radioOverlayId;}

    /*!
     * \brief Set the maximum channel Bandwidth
     */
    void setChBwdth(ChBandwidth chBwdth) {m_chBwdth = chBwdth;}

    /*!
     * \brief Get the maximum channel bandwidth
     */
    ChBandwidth getConfChBwdth() { return m_chBwdth;}

    /*!
     * \brief Set the frequency band
     */
    void setFreqBand(double freqBand)
    {
        if (freqBand == 2400.0e6)
        {
            m_freqBand = GHz_2_4;
        }

        else if (freqBand == 5000.0e6)
        {
            m_freqBand = GHz_5;
        }
    }

    /*!
     * \brief Get the frequency band
     */
    int getFreqBand() { return m_freqBand;}

    /*!
     * \brief Returns the specral band of a bandwidth
     */
    spectralBand* getSBand(ChBandwidth chBwth);

    /*!
     * \brief Returns a string based on the phy mode configured
     */
    const char* getPhyModeStr(int phyMode)
    {
        switch (phyMode)
        {
            case PHY802_11a:
            {
                return "802.11a";
            }
            case PHY802_11b:
            {
                return "802.11b";
            }
            case PHY802_11n:
            {
                return "802.11n";
            }
            case PHY802_11ac:
            {
                return "802.11ac";
            }
            default:
            {
                return "";
            }
        }
    }

    /*!
     * \brief Returns noise and interference
     */
    double getNi(spectralBand* bd, PropRxInfo *propRxInfo = NULL);

    /*!
     * \brief Returns the channel index of a bandwidth
     */
    Int32 getChannelIndex(double centerFreq, int frequencyBand);

    /*!
     * \brief Returns primary channel index of a bandwidth
     */
    Int32 getPChIndex(ChBandwidth chBwth)
    {
        Int32 chIdx = 0;
        if (chBwth == CHBWDTH_20MHZ)
        {
            chIdx = m_subChList[0]->getChIndex();
        }
        else if (chBwth == CHBWDTH_40MHZ)
        {
            if (m_subChList[0]->getChIndex()
                < m_subChList[1]->getChIndex())
            {
                chIdx = m_subChList[0]->getChIndex() + 2;
            }
            else
            {
                chIdx = m_subChList[0]->getChIndex() - 2;
            }
        }
        else if (chBwth == CHBWDTH_80MHZ)
        {
            if (m_subChList[3]->getChIndex() > m_subChList[2]->getChIndex())
            {
                chIdx = m_subChList[2]->getChIndex() - 2;
            }
            else
            {
                chIdx = m_subChList[2]->getChIndex() + 2;
            }
        }
        else if (chBwth == CHBWDTH_160MHZ)
        {
            if (m_subChList[5]->getChIndex() > m_subChList[4]->getChIndex())
            {
                chIdx = m_subChList[4]->getChIndex() - 2;
            }
            else
            {
                chIdx = m_subChList[4]->getChIndex() + 2;
            }
        }
        return chIdx;
    }

    /*!
     * \brief Returns secondary channel index of a bandwidth
     */
    Int32 getSChIndex(ChBandwidth chBwth)
    {
        Int32 chIdx = 0;
        if (chBwth == CHBWDTH_20MHZ)
        {
            chIdx = m_subChList[1]->getChIndex();
        }
        else if (chBwth == CHBWDTH_40MHZ)
        {
            if (m_subChList[2]->getChIndex()
                > m_subChList[1]->getChIndex())
            {
                chIdx = m_subChList[2]->getChIndex() + 2;
            }
            else
            {
                chIdx = m_subChList[2]->getChIndex() - 2;
            }
        }
        else if (chBwth == CHBWDTH_80MHZ)
        {
            if (m_subChList[4]->getChIndex() > m_subChList[3]->getChIndex())
            {
                chIdx = m_subChList[4]->getChIndex() + 6;
            }
            else
            {
                chIdx = m_subChList[4]->getChIndex() - 6;
            }
        }
        else if (chBwth == CHBWDTH_160MHZ)
        {
            ERROR_ReportError("160MHZ does not have secondary channel index");
        }
        return chIdx;
    }

    /*!
     * \brief Returns the subchannel list
     */
    std::vector<SubChannel*> getSubChList(){ return m_subChList;}

    /*!
     * \brief Returns if 20MHz secondary channel is above 20MHz
     *  primary channel
     */
    BOOL is20MHzSChAbovePCh()
    {
        return (m_subChList[0]->getChIndex()
            < m_subChList[1]->getChIndex());
    }

    /*!
     * \brief Initialized a channel and its subchannels
     */
    void init(AcMode mode, const NodeInput* nodeInput);

    /*!
     * \brief Returns the available channel bandwidth
     */
    bool getAvlbChBwdth(ChBandwidth confChBwdth, ChBandwidth& avlChBwdth);

    /*!
     * \brief Checks whether DIFS/backoff needs to be cancelled
     * after reception of the 1st bit of the packet
     */
    BOOL needToCancelDIFSOrPauseBackoff();

    /*!
     * \brief Read configuration parameters
     */
    void readCfgParams(const NodeInput* nodeInput);

     /*!
     * \brief Read configuration parameters for 10MHz bandwidth
     */
    void readCfgParamsFor10MHz(const NodeInput* nodeInput);
    /*!
     * \brief Read configuration parameters for 20MHz bandwidth
     */
    void readCfgParamsFor20MHz(const NodeInput* nodeInput);

    /*!
     * \brief Read configuration parameters for 40MHz bandwidth
     */
    void readCfgParamsFor40MHz(const NodeInput* nodeInput);

    /*!
     * \brief Read configuration parameters for 80MHz bandwidth
     */
    void readCfgParamsFor80MHz(const NodeInput* nodeInput);

    /*!
     * \brief Read configuration parameters for 160MHz bandwidth
     */
    void readCfgParamsFor160MHz(const NodeInput* nodeInput);

    /*!
     * \brief Finalize function of channel controller
     */
    void finalize();

    /*!
     * \brief Run time statistics
     */
    void runTimeStats();

    /*!
     * \brief Default constructor
     */
    ChannelController()
    {}

    /*!
     * \brief Constructor
     */
    ChannelController(int numSubChannels, MacController* macController, Phy802_11Manager* phyac)
        : m_numSubChannels(numSubChannels), m_macController(macController),
        m_phy802_11Manager(phyac)
    {
    }

    /*!
     * \brief Destructor
     */
    ~ChannelController();

};

} //namespace Qos

} //namespace Dot11

UInt8 getSupportedChWidthSet();

#endif
