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
#include <boost/shared_ptr.hpp>

#ifndef __JLM_H__
#define __JLH_H__

/// \file jlm.h
/// 
/// \brief Contains definitions relevant to Jammer Listener Manager
/// 

class Message;
struct Node;
struct NodeInput;

#include "jlm_observer.h"

/*!
 * \brief Filter definition to identify messages that fit the defintion of a
 * Jammer message
 *
 * A jam message can be any message that arrives in the kernel processing
 * pipeline, but in this case the jammer identifies any message that contains
 * the correct INFO field and contents to that info field \sa jlm.cpp for
 * details. It is a generic piece of code and could be expanded to observe
 * other types of frames.
 *
 * This code is available to the user even though it is employed in the
 * kernel pipeline.
 */

class JamFilter
{
public:

   /*!
    * \brief Determine whether the start and end events should be provided
    * to the Jammer observer
    *
    * \param msg a pointer to the message to be filters
    *
    * \return true if the message times are relevant to the
    * observer/listener, false otherwise
    */

    static bool jammerFrameQ(Message* msg);
};

/*!
 * \brief The Jammer Listener Manager manages the listeners and provides
 * them to the kernel and other interested parties as required
 *
 * The node instantiates a JLM during construction and makes a reference
 * available to requestors as needed.  The kernel uses the JLM when the
 * node is inactive and does not access the JLM when the node is active.
 * Furthermore the JLM is initialized *before* the PHY is initialized and
 * finalized *after* the PHY is finalized so that it is always available
 * to PHY processes.
 */

class Jlm
{
public:

   /*!
    * \brief A small indexing structure used as a key to map the internal
    * listener map
    *
    * \todo There is no need for this to be ordered, it could be an unordered
    * map for performance.
    */

    struct Idx
    {
       /*!
        * /brief The interface index associated with the tuple
        */
        int d_ifidx;

        /*!
        * \brief The channel associated with the tuple
        */
        int d_channel;

        /*!
        * \brief Default constructor has invalid values
        */

        Idx() : d_ifidx(-1), d_channel(-1) { ; }

        /*!
        * \brief full constructor requires both interface index and channel
        */

        Idx(int p_ifidx, int p_channel)
        : d_ifidx(p_ifidx), d_channel(p_channel) { ; }

       /*!
        * \brief convenience method to return interface index
        *
        * \return interface index
        */

        int ifidx() const { return d_ifidx; }

       /*!
        * \brief convenience method to return channel index
        *
        * \return channel index
        */

        int channel() const { return d_channel; }

       /*!
        * \brief Equal operator
        *
        * \return true if all members are equal, false otherwise
        */

        bool operator==(const Idx& rhs) const
        {
            return d_ifidx == rhs.d_ifidx && d_channel == rhs.d_channel;
        }

       /*!
        * \brief ordering operator
        *
        * \return true if the operator is ordered less by member, false
        * otherwise
        */

        bool operator<(const Idx& rhs) const
        {
            if (d_ifidx < rhs.d_ifidx) return true;
            if (rhs.d_ifidx < d_ifidx) return false; 
            if (d_channel < rhs.d_channel) return true;
            return false;
        }
    };

   /*!
    * \brief Object of Idx structure
    */
    typedef Idx Key;
    
   /*!
    * \brief Shared pointer to JammerObserver
    */
    typedef boost::shared_ptr<JammerObserver> Value;

   /*!
    * \brief Underlying data container type
    */
    typedef std::map<Key,Value> ListenerMap;

   /*!
    * \brief Interator typedef
    */
    typedef ListenerMap::iterator ListenerIterator; ///< 

   /*!
    * \brief Listener pair
    */
    typedef ListenerMap::value_type ListenerPair;

private:

   /*!
    * \brief Pointer to the node
    */
    Node* d_node; ///< Pointers to owning node

   /*!
    * \brief Data variable
    */
    ListenerMap d_data;

   /*!
    * \brief Construct a node (unowned)
    *
    * \note Not allowed
    */

    Jlm() = delete;

protected:

   /*!
    * \brief Provide a pointer to owning node
    *
    * \return pointer to node that owns the JLM
    */

    Node* node() { return d_node; }

 public:

   /*!
    * \brief Returns an iterator referring to the first element in the d_data
    */

    ListenerIterator begin() { return d_data.begin(); }

   /*!
    * \brief Returns an iterator referring to the last element in the d_data
    */

    ListenerIterator end() { return d_data.end(); }

   /*!
    * \brief Construct a node
    *
    * \note All JLM must presently be owned
    */

    Jlm(Node* p_node) : d_node(p_node) { ; }

   /*!
    * \brief Register an observer (listener)
    * \param phyIndex the PHY index the observer is representing
    * \param channelIndex The channel index the observer is representing
    * \param jammer A pointer to the observer
    *
    * \return A pointer to the previous observer (or NULL if there
    * wasn't one)
    *
    * \note The old observer must be deleted if it is not longer needed
    * \note the space { node x phyIndex x channelIndex } completely
    * specifies the receiver space.
    */

    Value register_observer(int phyIndex, int channelIndex, Value jammer)
    {
        Key key(phyIndex, channelIndex);
        ListenerIterator pos = d_data.find(key);

        if (pos == d_data.end()) {
            d_data.insert(ListenerPair(key, jammer));
            return Value();
        }
        Value tmp = pos->second;

        pos->second = jammer;
        return tmp;
    }

   /*!
    * \brief Remove observer
    *
    * \param phyIndex integer index of PHY interface
    * \param channelIndex integer index of channel
    *
    * \return pointer to existing observer, if exists, NULL othewise
    */

    Value remove_observer(int phyIndex, int channelIndex)
    {
        ListenerIterator pos = d_data.find(Key(phyIndex, channelIndex));

        if (pos == d_data.end())
        {
            return Value();
        }

        Value tmp = pos->second;
        d_data.erase(pos);
        return tmp;
    }

   /*!
    * \brief Return an observer for a given { phyIndex, channelIndex } pair
    *
    * \param phyIndex the index the PHY the observer represents
    * \param channelIndex the index of the channel the observer represents
    *
    * \return a pointer to the observer, or NULL if no such observer exists
    */

    Value observer(int phyIndex, int channelIndex)
    {
        ListenerIterator pos = d_data.find(Key(phyIndex, channelIndex));

        if (pos == d_data.end())
        {
            return Value();
        }

        return pos->second;
    }

   /*!
    * \brief Return the size of the current number of observers on this node
    *
    * \return the number of observers on this node
    *
    * \note this is used in the kernel to make performance enhancements and
    * reduce the number of calculations.
    */

    size_t size() { return d_data.size(); }

   /*!
    * \brief Initialized the JLM
    *
    * \param nodeInput a pointer to the configuration file data structure
    *
    * Initialization occurs before the PHY is initialization and this can be
    * used to create node-wide listeners, or test listenrs.  Others listeners
    * can be created in PHY processing, as required.
    */

    void initialize(const NodeInput* nodeInput);

   /*!
    * \brief Finalized the JLM
    *
    * The JLM is finalized after the PHY layer is finalized.  It is expected
    * that all observers will output their statistics via IO_PrintStat at
    * this time.  They are called individually with the phyIndex and
    * channelIndex.  No observers should be finalized in the PHY layer.
    */

    void finalize()
    {
        for (ListenerIterator pos(d_data.begin());
             pos != d_data.end();
             ++pos)
        {
            const Key& key = pos->first;
            Value jammer = pos->second;

            if (jammer)
            {
                jammer->finalize(node(), key.ifidx(), key.channel());
            }
        }
    }
};


#endif
