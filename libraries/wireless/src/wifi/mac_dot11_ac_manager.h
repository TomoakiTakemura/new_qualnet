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

/// \file mac_dot11_ac_manager.h
///
/// \brief Dot11 Access Category Class hierarchy
///
/// This file contains the class hierarchy of access categories

#ifndef __MAC_DOT11_QOS_AC_H__
#define __MAC_DOT11_QOS_AC_H__
#include "mac_dot11.h"
#include "mac.h"
#include "mac_dot11_maccontroller.h"

namespace Dot11
{
namespace Qos
{

/*!
* \brief Single Access Category-based Subqueue
*/
class AccessCategory
{
public:
    /// Maximum Contention Window
    clocktype m_cwMax;
    /// Difs Interval
    clocktype m_Difs;
    /// Contention Window
    clocktype m_CW;
    /// Minimum Contention Window
    clocktype m_cwMin;
    /// Arbitration inter-frame spacing
    clocktype m_AIFS;
    /// Transmission Opportunity Limit
    clocktype m_TXOPLimit;
    /// AIFS Number
    clocktype m_AIFSN;
    /// Slot Time
    clocktype m_slotTime;
    /// Backoff Interval
    clocktype m_BO;
    /// Remaining Backoff Interval
    clocktype m_lastBOTimeStamp;
    /// Sequence Number Entry
    DOT11_SeqNoEntry* m_seqNoHead;
    /// Frame Info
    DOT11_FrameInfo* m_frameInfo;
    /// Is Access Category has Packet
    BOOL m_isACHasPacket;
    /// Output Buffer
    OutputBuffer* m_outputBuffer;
    /// Access Category State
    AcState m_state;
    /// Backoff End Time
    clocktype m_difs_bo_endTime;
    /// Frame to Send of this Access Category
    Message* m_frameToSend;
    /// Next Hop Address
    Mac802Address m_currentNextHopAddress;
    /// Network Type
    Int32 m_networkType;
    /// Priority Value
    TosType m_priority;
    /// FrameInfo List
    std::list<DOT11_FrameInfo*> frameInfoList;
    /// Short Packet Retry Count
    Int32 m_QSRC;
    /// Large Packet Retry Count
    Int32 m_QLRC;
    /// total no of packet of this type queued
    unsigned int m_totalNoOfthisTypeFrameQueued;
    /// total no of packet of this type dequeued
    unsigned int m_totalNoOfthisTypeFrameDeQueued;
    /// total no of this type frame retried
    unsigned int m_totalNoOfthisTypeFrameRetried;
    /// total number of this type frame queued under BAA
    unsigned int m_totalNoOfthisTypeFrameQueuedUnderBAA;

    /*!
    * \brief Default Constructor
    */
    AccessCategory() 
    {
        // Initialize all the data members
        m_cwMax = 0;
        m_Difs = 0;
        m_CW = 0;
        m_cwMin = 0;
        m_AIFS = 0;
        m_TXOPLimit = 0;
        m_AIFSN = 0;
        m_slotTime = 0;
        m_BO = 0;
        m_lastBOTimeStamp = 0;
        m_seqNoHead = NULL;
        m_frameInfo = NULL;
        m_isACHasPacket = FALSE;
        m_outputBuffer = NULL;
        m_state = k_State_IDLE;
        m_difs_bo_endTime = 0;
        m_frameToSend = NULL;
        m_currentNextHopAddress = INVALID_802ADDRESS;
        m_networkType = DOT11_NET_UNKNOWN;
        m_priority = 0;
        m_QSRC = 0;
        m_QLRC = 0;
        m_totalNoOfthisTypeFrameQueued = 0;
        m_totalNoOfthisTypeFrameDeQueued = 0;
        m_totalNoOfthisTypeFrameRetried = 0;
        m_totalNoOfthisTypeFrameQueuedUnderBAA = 0;
    }
    
    /*!
    * \brief Function to get bo end time 
    */
    clocktype getBoEndTime() {return m_difs_bo_endTime;}

    /*!
    * \brief Function to get the output buffer
    */
    OutputBuffer* getBuffer() {return m_outputBuffer;}

    /*!
    * \brief Function to set is Ac has packet
    */
    void setIsAcHasPacket(BOOL value) {m_isACHasPacket = value;}
    
    /*!
    * \brief Function to set bo end time 
    */
    void setBoEndTime(clocktype endTime) {m_difs_bo_endTime = endTime;}
    
    /*!
    * \brief Function to start DIFS 
    */
    void startDifs(clocktype endTime);

    /*!
    * \brief Function to set the Backoff Interval
    */
    void setBackoffInterval(clocktype backOff) {m_BO = backOff;}
    
    /*!
    * \brief Function to get the Backoff Interval
    */
    clocktype getBackoffInterval() {return m_BO;}

    /*!
    * \brief Function to get the difs Interval
    */
    clocktype getDifs() {return m_Difs;}
    
    /*!
    * \brief Function to get the AIFS Interval
    */
    clocktype getAifsInterval() {return  m_AIFS;}
    
    /*!
    * \brief Function to get the current Contention Window
    */
    clocktype getCW() {return m_CW;}

    /*!
    * \brief Function to set the current Contention Window
    */
    void setCW(clocktype value) {m_CW = value;}

    /*!
    * \brief Function to get the minimum Contention Window
    */
    clocktype getMinCW() {return m_cwMin;}
    
    /*!
    * \brief Function to check if AC has packet
    */
    BOOL isAcHasPacket() {return m_isACHasPacket;}
    
    /*!
    * \brief Function to get the slot time Window
    */
    clocktype getSlotTime() {return m_slotTime;}
    
    /*!
    * \brief Function to reset the Access Category
    */
    void resetAccessCategory();
    
    /*!
    * \brief Function to set the Access Category state
    */
    void setState(AcState state) {m_state = state;}
    
    /*!
    * \brief Function to get the Access Category state
    */
    AcState getState() {return m_state;}

    /*!
    * \brief Access Category destructor
    */
    ~AccessCategory(){;}
    
};


/*!
 * \brief Controller for Access Categories
 */
class AcManager
{
protected:
    /// Access Categories
    std::vector<AccessCategory> ac;
    /// Ac Timer Message 
    Message* m_timerMsg;
    /// Number of Access Categories
    UInt8 m_numAccessCategories;
    /// Interface Index
    int m_interfaceIndex;
    // Ac Mode
    AcMode m_mode;
    // Node Pointer
    Node* m_node;

    /*!
    * \brief Function to get the node pointer
    */
    Node* node() {return (m_node);}
    
    /*!
    * \brief Function to get the interface index
    */
    int ifidx() {return m_interfaceIndex;}
    
    /*!
    * \brief Function to get the Access Category Mode
    */
    AcMode mode() {return m_mode;}

public:

    /*!
    * \brief Access Category Controller Constructor
    */
    AcManager(Node* node, int interfaceIndex, AcMode mode) 
    {
        m_node = node;
        m_interfaceIndex = interfaceIndex;
        m_mode = mode;
    }

    /*!
    * \brief Function to initialize Ac
    */
    virtual void InitAc(Node* node, MacDataDot11* dot11, PhyModel phyModel);

    /*!
    * \brief Function to Dequeue Packet from Network Layer
    */
    virtual BOOL dqFromNetwork(MacDataDot11* dot11) {return FALSE;}

    /*!
    * \brief Function to recalculate difs and bo and return min Ac Index
    */
    virtual void ReCalculateDifsBoandStartContention(MacDataDot11* dot11);
    
    /*!
    * \brief Function to set the current message
    */
    virtual void setCurrentMessage(MacDataDot11* dot11, UInt8 acIndex) {}
    
    /*!
    * \brief Virtual Function to set number of Access Category 
    */
    void setNumAc(UInt8 index) {m_numAccessCategories = index;}

    /*!
    * \brief Virtual Function to get number of Access Category 
    */
    UInt8 getNumAc() {return m_numAccessCategories;}
    
    /*!
    * \brief Virtual Function to calculate extra delay
    */
    void CalculateExtraDelay(MacDataDot11* dot11,
                             clocktype& totaldelay);
    
    /*!
    * \brief Virtual Function to dequeue packet from Mac Queue
    */
    virtual void dqFromMac(MacDataDot11* dot11){}

    /*!
    * \brief Virtual Function to Set More Fields in Ac
    */
    virtual void setMoreFrameField(UInt8 acIndex){}

    /*!
    * \brief Function to get the Ac with index
    */
    AccessCategory& getAc(int i) {return ac[i];}

    /*!
    * \brief Initialize the Access categories
    */
    void initializeAcManager();

    /*!
    * \brief Check if the state of Access Category is DIFS or BO
    */
    BackoffState CheckStateIfDifsOrBo(MacDataDot11* dot11);

    /*!
    * \brief Start Contention
    */ 
    virtual void startContention(MacDataDot11* dot11);
    
    /*!
    * \brief Handle Timer
    */
    void handleTimer(MacDataDot11* dot11, UInt8 timerIndex);
    
    /*!
    * \brief Interrupt Contention
    */
    void interruptContention(MacDataDot11* dot11);

    /*!
    * \brief Pause Ac States
    */
    virtual void pauseAc(MacDataDot11* dot11);

    /*!
    * \brief Function to insert packet in Access Category
    */
    virtual void enqueue(MacDataDot11* dot11,
                         DOT11_FrameInfo *frameInfo,
                         TosType priority,
                         Mac802Address NextHopAddress,
                         BOOL isKeyAlreadyPresent) { ; }

    /*!
    * \brief Function to set backoff if zero
    */
    void SetBackoffIfZero(Node* node,
                          MacDataDot11* dot11,
                          int currentACIndex);

    /*!
    * \brief Function to check if the packet can be queued
    */
    BOOL CanPacketBeQueuedInOutputBuffer(
        MacDataDot11* dot11,
        Mac802Address nextHopAddress,
        TosType priority,
        BOOL* isKeyAlreadyPresent);

/*** UAPSD *********************************START*******************/
    /*!
    * \brief Function to check if the packet exists for a destination
    *        in the output buffer
    */
    int packetsForThisRA(
        MacDataDot11* dot11,
        Mac802Address nextHopAddress);
    
    /*!
    * \brief Function to check if an access category (acIndex) is
    *        allowed to contend
    */
    bool canThisACContend(
        MacDataDot11* dot11,
        int acIndex);
/*** UAPSD *********************************END*******************/

    /*!
    * \brief Function to classify packet
    */
    BOOL ClassifyPacket(MacDataDot11* dot11,
                        Message* msg,
                        TosType queuePriority,
                        Mac802Address NextHopAddress,
                        BOOL SendQosFrame,
                        BOOL legacyMode = FALSE);

    /*!
    * \brief Get Minimum Difs and BO Inteval Access Category
    */
    UInt8 getMinDifsBoIntervalAc(MacDataDot11* dot11);
    
    /*!
    * \brief Start the contention timer for particular acIndex
    */
    Message* startContentionTimer(MacDataDot11* dot11, clocktype delay, UInt8 acIndex);
    
    /*!
    * \brief Access Category Controller Destructor
    */
    virtual ~AcManager() {;}
    
};

/*!
 * \brief class for LegacyDot11 Access Category
 */
class LegacyAcManager: public AcManager
{

public:

    /*!
    * \brief Function to Dequeue Packet from Network Layer
    */
    BOOL dqFromNetwork(MacDataDot11* dot11);

    /*!
    * \brief Function to initialize Ac
    */
    void InitAc(Node* node, MacDataDot11* dot11, PhyModel phyModel);

    /*!
    * \brief Legacy Access Category Controller Constructor
    */
    LegacyAcManager(Node* node, int interfaceIndex, AcMode mode) : 
                                AcManager(node, interfaceIndex, mode) {;}

    /*!
    * \brief Function to start contention
    */ 
    void startContention(MacDataDot11* dot11);
    
    /*!
    * \brief Function to set the current message
    */
    void setCurrentMessage(MacDataDot11* dot11, UInt8 acIndex);

    /*!
    * \brief Virtual Function to dequeue packet from Mac Queue
    */
    void dqFromMac(MacDataDot11* dot11);

    /*!
    * \brief Interrupt Contention
    */
    void interruptContention(MacDataDot11* dot11);
    
    /*!
    * \brief Pause Ac States
    */
    void pauseAc(MacDataDot11* dot11);
    
    /*!
    * \brief Legacy Access Controller destructor
    */
    ~LegacyAcManager() {;}
};


/*!
 * \brief class for Dot11e access categories
 */
class Dot11eAcManager : public AcManager
{

public:

    /*!
    * \brief Dot11e Access Category Controller Constructor
    */
    Dot11eAcManager(Node* node, int interfaceIndex, AcMode mode) :
                                AcManager(node, interfaceIndex, mode) {;}
    
    /*!
    * \brief Function to set the current message
    */
    void setCurrentMessage(MacDataDot11* dot11, UInt8 acIndex);
    
    /*!
    * \brief Function to Dequeue Packet from Network Layer
    */
    BOOL dqFromNetwork(MacDataDot11* dot11);

    /*!
    * \brief Virtual Function to dequeue packet from Mac Queue
    */
    void dqFromMac(MacDataDot11* dot11);

    /*!
    * \brief Set More Fields in Ac
    */
    void setMoreFrameField(UInt8 acIndex);

    /*!
    * \brief Dot11e Access Category Controller Destructor
    */
    ~Dot11eAcManager() {;}
};

/*!
* \brief class for Dot11n access categories
*/
class Dot11nAcManager : public AcManager
{

public:
    
    /*!
    * \brief Dot11n Access Category Controller Constructor
    */
    Dot11nAcManager(Node* node, int interfaceIndex, AcMode mode) :
                                AcManager(node, interfaceIndex, mode) {;}
    
    /*!
    * \brief Function to set the current message
    */
    void setCurrentMessage(MacDataDot11* dot11, UInt8 acIndex);
    
    /*!
    * \brief Function to Dequeue Packet from Network Layer
    */
    BOOL dqFromNetwork(MacDataDot11* dot11);

    /*!
    * \brief Virtual Function to dequeue packet from Mac Queue
    */
    void dqFromMac(MacDataDot11* dot11);

    /*!
    * \brief Function to insert packet in Access Category
    */
    void enqueue(MacDataDot11* dot11,
                 DOT11_FrameInfo *frameInfo,
                 TosType priority,
                 Mac802Address NextHopAddress,
                 BOOL isKeyAlreadyPresent);
        
    /*!
    * \brief Dot11n Access Category Controller Destructor
    */
    ~Dot11nAcManager() {;}
};

/*!
* \brief class for Dot11ac Access Queue
*/
class Dot11acAcManager : public AcManager
{

public:
    /*!
    * \brief Dot11ac Access Category Controller Constructor
    */
    Dot11acAcManager();
    /*!
    * \brief Dot11ac Access Category Controller Destructor
    */
    ~Dot11acAcManager() {;}
};


} // namespace Qos

} // namespace Dot11

#endif
