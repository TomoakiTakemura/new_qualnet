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

/// \file jlm_observer.cpp

#include "node.h"
#include "jlm_observer.h"

//# define DEBUG_JLM 1


/// \brief Implementation of f API
///
/// This function returns reference to standard output stream
///
/// \return Output stream
std::ostream& JamDurationObserver::f()
{
    return std::cout << d_node->nodeId << ","
        << d_ifidx << "," << d_channel;
}

/// \brief Implementation of debugPrint API
///
/// This function returns reference to standard output stream
///
/// \return Output stream
std::ostream& JamDurationObserver::debugPrint()
{
    return std::cout << "[" << d_node->getNodeTime() << "]["
        << d_node->nodeId << "][" << d_ifidx << "][" << d_channel << "]";
}

/// \brief Implementation of signal_arrival callback API
///
/// This function keeps track of jammer signal arrival events.
///
/// \param tstamp Time stamp of signal arrival
/// \param signal_mW Signal power of arrived signal
/// \param phyIndex Interface index
/// \param chIdx Channel Index
///
/// \return n/a

void JamDurationObserver::signal_arrival(clocktype tstamp, double signal_mW,
                                         int phyIndex, int chIdx)
{
#ifdef DEBUG_JLM
    debugPrint() << "SignalArrival" << std::endl;
    //f() << "," << t << "," << signal_mW << "," << "A1" << std::endl;
#endif
    d_count++;
    if (d_isActive && d_count == 1)
    {
        // Keep track of the time when a signal arrival event was
        // received when count was zero
        d_tStart = tstamp;
    }
}

/// \brief Implementation of signal_end callback API
///
/// This function keeps track of jammer signal end events.
///
/// \param tstamp Time stamp of signal arrival
/// \param phyIndex Interface index
/// \param chIdx Channel Index
///
/// \return n/a

void JamDurationObserver::signal_end(
    clocktype tstamp, int phyIndex, int chIdx)
{
    d_count--;
    if (!d_count && d_isActive)
    {
        assert(d_tStart != -1);
        d_duration += tstamp - d_tStart;
#ifdef DEBUG_JLM
       debugPrint() << "SignalEnd JDuration" << "[" << tstamp - d_tStart
           << "]OverallJDuration" << "[" << d_duration << "]" << std::endl;
#endif
        d_tStart = -1;
    }
    else
    {
#ifdef DEBUG_JLM
        debugPrint() << "SignalEnd" << std::endl;
        //f() << "," << t << "," << "A0" << std::endl;
#endif
    }
}

/// \brief Implementation of phy_disable callback API
///
/// This function keeps track of event when PHY stops listening to a
/// "chIdx" channel
///
/// \param tstamp Time stamp of signal arrival
/// \param phyIndex Interface index
/// \param chIdx Channel Index
///
/// \return n/a

void JamDurationObserver::phy_disable(clocktype tstamp,
                                      int phyIndex, int chIdx)
{
    d_isActive = false;
    if (d_count)
    {
        assert(d_tStart != -1);
        d_duration += tstamp - d_tStart;
#ifdef DEBUG_JLM
        debugPrint() << "Tuning-Out JDuration" << "[" << tstamp - d_tStart
           << "]OverallJDuration" << "[" << d_duration << "]" << std::endl;
#endif
        d_tStart = -1;
    }
    else
    {
#ifdef DEBUG_JLM
        debugPrint() << "Tuning-Out" << std::endl;
        //f() << "," << t << "," << "B0" << std::endl;
#endif
    }
}

/// \brief Implementation of phy_enable callback API
///
/// This function keeps track of event when PHY starts listening to a
/// "chIdx" channel
///
/// \param tstamp Time stamp of signal arrival
/// \param phyIndex Interface index
/// \param chIdx Channel Index
///
/// \return n/a

void JamDurationObserver::phy_enable(clocktype tstamp,
                                     int phyIndex, int chIdx)
{
    d_isActive = true;
#ifdef DEBUG_JLM
    debugPrint() << "Tuning-In" << std::endl;
    //f() << "," << t << "," << "B1" << std::endl;
#endif
    if (d_count)
    {
        d_tStart = tstamp;
    }
}

/// \brief Implementation of duration API
///
/// This function retuns the overall jamming duration calculated
/// by the observer.
///
/// \return clocktype Duration value

clocktype JamDurationObserver::duration()
{
    if (d_count > 0 && d_isActive)
    {
        // Active jammer signal(s) on the medium at the instant when
        // simulation ends
        assert(d_tStart != -1);
        d_duration += d_node->getNodeTime() - d_tStart;
#ifdef DEBUG_JLM
        debugPrint() << "Finalize JDuration["
            << d_node->getNodeTime() - d_tStart
            << "]OverallJDuration:" << d_duration << std::endl;
#endif
    }
    else
    {
#ifdef DEBUG_JLM
        debugPrint() << "Finalize OverallJDuration:"
            << d_duration << std::endl;
#endif
    }
    return d_duration;
};

