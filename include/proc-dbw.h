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
#ifndef __PROC_DBW_H__
#define __PROC_DBW_H__


#include <stdio.h>

#include <iostream>
#include <map>
#include <string>

#include <assert.h>
#include <stdlib.h>

#include "proc-base.h"

#include "proc-dbw-ds.h"
#include "proc-dbw-cs.h"

#include "proc-dbw-ds-impl.h"
#include "proc-dbw-cs-impl.h"


namespace Proc {

template <class NamingModel = NamingModelInterface>
class MappedEventModel : public Rooted
{
  Event::Cache<Database::Stream> d_dsc;
  Event::Cache<Configuration::Stream> d_csc;
  Event::Cache<Database::Element> d_dds;

  NamingModel d_nm;
  NodeEventController& d_nec;

  void gc(long long t_now)
  {
    std::map<std::string, Database::Stream*>::iterator pos = d_dsc.d_map.begin();
    while (pos != d_dsc.d_map.end())
    {
      Database::Implementation<NamingModel>* impl = (Database::Implementation<NamingModel>*)pos->second;
      if (impl->d_t < t_now)
      {
        d_dsc.d_map.erase(pos++);
        delete impl;
      }
      else
      {
        ++pos;
      }
    }
  }

protected:

  Database::Stream* dfactory(const std::string& qualified_event_name, 
                             long long t, const std::string& event_name)
  {
    unsigned long long seq = d_nec.seq();

    Database::Stream* impl = (Database::Stream*) 
      new Database::Implementation<NamingModel>(t, seq, event_name, d_dds);

    return impl;
  }

  Configuration::Stream* cfactory(const std::string& dataset)
  {
    Configuration::Stream* impl = (Configuration::Stream*) 
      new Configuration::Implementation<NamingModel>(dataset, d_dds);

    return impl;
  }

public:

  MappedEventModel(NodeEventController& p_nec) : d_nec(p_nec), Rooted(p_nec) { ; }

  Configuration::Stream& cs(const std::string& dataset) 
  {
    Configuration::Stream* impl = d_csc.find(dataset);

    if (impl == NULL)
    {
      impl = cfactory(dataset);
      d_csc.insert(dataset, impl);
    }

    return *impl;
  }

  std::string qualify_event_name(const std::string& event_name, long long t)
  {
    std::string time_str = to_fmt<long long>("%lld", t);
    return locale() + std::string("/") + event_name + std::string("/") + time_str;
  }

  Database::Stream& ds(const std::string& event_name, long long t)
  {
    gc(t);

    std::string qualified_event_name = qualify_event_name(event_name, t);

    Database::Stream* impl = d_dsc.find(qualified_event_name);

    if (impl == NULL)
    {
      impl = dfactory(qualified_event_name, t, event_name);
      d_dsc.insert(qualified_event_name, impl);
    }

    return *impl;
  }

} ;

}


#endif
