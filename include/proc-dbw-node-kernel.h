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
#ifndef __PROC_DBW_NODE_KERNEL_H__
#define __PROC_DBW_NODE_KERNEL_H__


#include <stdio.h>

#include <iostream>
#include <map>
#include <string>

#include <assert.h>
#include <stdlib.h>

#include "proc-base.h"



namespace Proc {

namespace Event {

template <class Container>
struct Cache
{
  typedef Container* ContainerPointer;
  typedef typename std::map<std::string, ContainerPointer> ContainerMap;
  typedef typename ContainerMap::const_iterator ContainerIterator;

  ContainerMap d_map;

  ~Cache()
  {
    ContainerIterator pos = d_map.begin();
    while (pos != d_map.end())
    {
      ContainerPointer entry = pos->second;
      delete entry;
      pos++;
    }

    d_map.clear();
  }

  bool contains(const std::string& key) const
  {
    ContainerIterator pos = d_map.find(key);
    return pos != d_map.end();
  }

  ContainerPointer insert(const std::string& key, ContainerPointer value)
  {
    assert(!contains(key));

    d_map[key] = value;
    return d_map[key];
  }

  ContainerPointer find(const std::string& key) const
  {
    ContainerIterator pos = d_map.find(key);
    if (pos == d_map.end())
    {
      return NULL;
    }
    return pos->second;
  }

  ContainerPointer erase(const std::string& key)
  {
    ContainerIterator pos = d_map.find(key);
    if (pos == d_map.end())
    {
      return NULL;
    }

    ContainerPointer value = pos->second;
    d_map.erase(pos);

    return value;
  }

} ;

}

class NodeEventController : public Rooted
{
  unsigned long long d_seq;

public:

  NodeEventController(Node* p_node, int p_ifidx) : Rooted(p_node, p_ifidx), d_seq(0) { ; }
  NodeEventController(Rooted& p_rooted) : Rooted(p_rooted), d_seq(0) { ; }

  unsigned long long seq() { return ++d_seq; }

  unsigned long long dseq()
  {
      return d_seq;
  }
} ;

}


#endif
