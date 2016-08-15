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

/// \file jlm_observer.h

#ifndef __JLM_OBSERVER_H__
#define __JLM_OBSERVER_H__

#include "clock.h"

/*!
 * \brief Observer class for Jammer Listener
 *
 * Invoked by kernel pipeline under following conditions
 * - Jamming signal arrived from propagation layer
 * - Jamming signal end has occurred from propagation layer
 * - PHY has stopped listening to channel
 * - PHY has started listening to channel
 */

class JammerObserver
{
 public:

   /*
    * \brief Destructor
    */

    virtual ~JammerObserver() { ; } ///< \brief Required chained dtor

   /*!
    * \brief Signal arrival callback
    *
    * \param tstamp time that signal began arriving at physical  layer
    * \param signal_mW power level (in mW) of signal
    * \param phyIndex the index of the local PHY
    * \param channelIndex the index of the local channel
    */

    virtual void signal_arrival(clocktype tstamp, double signal_mW,
                                int phyIndex, int channelIndex) = 0;

   /*!
    * \brief Signal end callback
    *
    * \param tstamp time that the end of signal occured at physical layer
    * \param phyIndex the index of the local PHY
    * \param channelIndex the index of the local channel
    */

    virtual void signal_end(clocktype tstamp, int phyIndex,
                                              int channelIndex) = 0;

   /*!
    * \brief Start listening to channel callback
    *
    * \param tstamp time that PHY started listening to channel
    * \param phyIndex the index of the local PHY
    * \param channelIndex the index of the local channel
    */

    virtual void phy_enable(clocktype tstamp, int phyIndex,
                                              int channelIndex) = 0;

   /*!
    * \brief Stop listening to channel callback
    *
    * \param tstamp time that PHY stopped listening to channel callback
    * \param phyIndex the index of the local PHY
    * \param channelIndex the index of the local channel
    */

    virtual void phy_disable(clocktype tstamp, int phyIndex,
                                               int channelIndex) = 0;

    /*!
    * \brief Complete processing and write stats out to stat buffer
    *
    * \param node a pointer to the local node structure
    * \param phyIndex the index of the local PHY
    * \param channelIndex the index of the local channel
    */

    virtual void finalize(Node* node, int phyIndex, int channelIndex) = 0;
};

/*!
 * \brief Jammer Duration observer class
 */

class JamDurationObserver : public JammerObserver
{
   /*!
    * \brief Pointer to the node
    */
    Node* d_node;

   /*!
    * \brief Interface index
    */
    int d_ifidx;

   /*!
    * \brief Channel index
    */
    int d_channel;

   /*!
    * \brief Jammer duration on a channel
    */
    clocktype d_duration;

   /*!
    * \brief Time stamp of signal arrival event
    */
    clocktype d_tStart;

   /*!
    * \brief tuned-in/tuned-out state of observer
    */
    bool d_isActive;

   /*!
    * \brief Count of signal arrival events whose corresponding
    * signal end events haven't arrived yet
    */
    int d_count;

    /*!
    * \brief Print debug prints
    */

    std::ostream& f();

    /*!
    * \brief Print debug prints
    */

    std::ostream& debugPrint();

public:

   /*!
    * \brief Constructor
    */

    JamDurationObserver(Node* p_node, int p_ifidx, int p_channel)
     : d_node(p_node), d_ifidx(p_ifidx), d_channel(p_channel),
     d_isActive(false), d_tStart(-1), d_duration(0), d_count(0) { ; }

   /*!
    * \brief Destructor
    */

    ~JamDurationObserver() { ; }

   /*!
    * \brief Get count of jammer instances logged by this observer
    */
    int count(){ return d_count; }

    /*!
    * \brief Log signal arrival event
    */

    void signal_arrival(clocktype tstamp, double signal_mW,
                      int phyIndex, int channelIndex);

   /*!
    * \brief Log signal end event
    */

    void signal_end(clocktype tstamp, int phyIndex, int channelIndex);

   /*!
    * \brief Log tuning-out event
    */

    void phy_disable(clocktype tstamp, int phyIndex, int channelIndex);

   /*!
    * \brief Log tuning-in event
    */

    void phy_enable(clocktype tstamp, int phyIndex, int channelIndex);

   /*!
    * \brief Get jammer duration logged by the observer
    */

    clocktype duration();

   /*!
    * \brief Finalize things
    */

    void finalize(Node* node, int phyIndex, int channelIndex) { ; }
};

#endif

