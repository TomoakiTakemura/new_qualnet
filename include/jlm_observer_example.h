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

#include <boost/shared_ptr.hpp>

#include "api.h"
#include "message.h"
#include "node.h"
#include "propagation.h"
#include "clock.h"

#ifdef CYBER_LIB
#include "app_jammer.h"
#endif

#include "jlm_observer.h"

/// \file jlm_observer_example.h

/*!
 * \brief Test fixture for a Jammer Observer
 *
 * This prints out the various state in JSON for
 *
 * States include:
 * - A0: Signal End
 * - A1: Signal Start
 * - B0: PHY Disable (not listening)
 * - B1: PHY Enabled (listening)
 * - C2: Observer Constructed 
 * - C1: Observer Finalized 
 * - C0: Observer Destructed
 */

class SimpleTestObserver : public JammerObserver
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
    * \brief Print out JSON prolog to stdout
    *
    * Prolog contains
    * - "node" (node identifier)
    * - "ifidx" (interface index)
    * - "channel" (channel identifier)
    */

    void prolog()
    {
        std::cout << "@{ " 
            << "\"node\" : " << d_node->nodeId  << ", " 
            << "\"ifidx\" : " <<  d_ifidx << ", " 
            << "\"channel\" : " << d_channel << ", ";
    }

   /*!
    * \brief Print out JSON timestamp to stdout
    *
    * Timestamp contains
    * - "time" (Time in integer nanoseconds)
    */

    void timestamp(clocktype t)
    {
        std::cout << "\"time\" : " << t << ", " ;
    }

   /*!
    * \brief Print out JSON power level to stdout
    *
    * Power level contains
    * "power" (Power level at receiver in mW)
    */

    void powerLevel(double p)
    {
        std::cout << "\"power\" : " << p << ", " ;
    }

   /*!
    * \brief Print out JSON epilog to stdout
    *
    * Epilog prints out the closing bracket and newline
    */

    void epilog() { std::cout << "}" << std::endl; }

   /*!
    * \brief Print out JSON status to stdout
    */

    void status(const std::string& s)
    {
        std::cout << "\"state\" : \"" << s << "\" ";
    }

public:

   /*!
    * \brief Constructor
    */

    SimpleTestObserver(Node* p_node, int p_ifidx, int p_channel)
    : d_node(p_node), d_ifidx(p_ifidx), d_channel(p_channel)
    { prolog(), status("C2"), epilog(); }

   /*!
    * \brief Destructor
    */

    ~SimpleTestObserver()
    { 
        prolog(), status("C0"), epilog(); 
    }

   /*!
    * \brief Log signal arrival event
    */

    void signal_arrival(clocktype t, double signal_mW,
                        int phyIndex, int channelIndex)
    {
        prolog(), timestamp(t), powerLevel(signal_mW), status("A1"), epilog();
    }

   /*!
    * \brief Log signal end event
    */

    void signal_end(clocktype t, int phyIndex, int channelIndex)
    {
        prolog(), timestamp(t), status("A0"), epilog(); 
    }

   /*!
    * \brief Log tuning-out event
    */
    void phy_disable(clocktype t, int phyIndex, int channelIndex)
    {
        prolog(), timestamp(t), status("B0"), epilog(); 
    }

   /*!
    * \brief Log tuning-in event
    */

    void phy_enable(clocktype t, int phyIndex, int channelIndex)
    {
        prolog(), timestamp(t), status("B1"), epilog(); 
    }

   /*!
    * \brief Finalize things
    */

    void finalize(Node* node, int phyIndex, int channel)
    {
        prolog(), status("C1"), epilog(); 
    }
};

//
// Construct and register as:
//  register_observer(0, 0, boost::shared_ptr<JammerObserver>
//                   (new SimpleTestObserver(node(), 0, 0)));
//







