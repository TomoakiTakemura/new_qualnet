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
#ifndef __PROC_DBW_CS_IMPL_H__
#define __PROC_DBW_CS_IMPL_H__

#include <stdio.h>

#include <iostream>
#include <map>
#include <string>

#include <assert.h>
#include <stdlib.h>

#include "proc-dbw-node-kernel.h"
#include "proc-dbw-ds.h"


namespace Proc {

namespace Configuration {

template <class T>
class Implementation : public Stream
{
  Event::Cache<Database::Element>& d_dmap;
  Event::Cache<std::string>& d_dem;

  std::string d_dataset;

  T d_nm;
public:

  Implementation(const std::string& p_dataset,  Event::Cache<std::string>& p_dem,
                 Event::Cache<Database::Element>& p_dmap)
      : Stream(), d_nm(), d_dem(p_dem), d_dmap(p_dmap), d_dataset(p_dataset) { ; }

  Stream& operator[] (const std::string& eventName)
  {
    std::string remappedName = d_nm.emap(eventName);

    std::string* name_ptr = new std::string(d_dataset);
    std::string* old_ptr = d_dem.insert(remappedName, name_ptr);

    return *this;
  }

  Stream& operator() (const std::string& item, const std::string& type)
  {
    std::string remapped_item = d_nm.cmap(item);
    Database::Element* elem = d_dmap.find(remapped_item);

    if (elem == NULL)
    {
      d_dmap.insert(remapped_item, new Database::Element(remapped_item, type));

      Database::Element& dbd = *(d_dmap.find(remapped_item));

/*
      std::cout << "{" << item << ", " << type 
                << "} => {" << dbd.d_item << ", " 
                << dbd.d_type << ", " << dbd.d_fmt << ", " 
                << dbd.d_sql << "}" << std::endl;
*/
    }

    return *this;
  }

  Stream& operator() (const std::string& item, const std::string& type, const std::string& fmt)
  {
    std::string remapped_item = d_nm.cmap(item);
    Database::Element* elem = d_dmap.find(remapped_item);

    if (elem == NULL)
    {
      d_dmap.insert(remapped_item, new Database::Element(remapped_item, type, fmt));

      Database::Element& dbd = *(d_dmap.find(remapped_item));

/*
      std::cout << "{" << item << ", " << type 
                << ", " << fmt << "} => {" 
                << dbd.d_item << ", " << dbd.d_type 
                << ", " << dbd.d_fmt << ", " << dbd.d_sql << "}" << std::endl;
*/
    }

    return *this;
  }

  Stream& operator() (const std::string& item, const std::string& type, 
                      const std::string& fmt, const std::string& sql)
  {
    std::string remapped_item = d_nm.cmap(item);
    Database::Element* elem = d_dmap.find(remapped_item);

    if (elem == NULL)
    {
      d_dmap.insert(remapped_item, new Database::Element(remapped_item, type, fmt, sql));

      Database::Element& dbd = *(d_dmap.find(remapped_item));

/*
      std::cout << "{" << item << ", " << type << ", " 
                << fmt << ", " << sql << "} => {" 
                << dbd.d_item << ", " << dbd.d_type << ", " 
                << dbd.d_fmt << ", " << dbd.d_sql << "}" << std::endl;
*/
    }

    return *this;
  }

  Event::Cache<Database::Element>::ContainerIterator begin()
  {
    return d_dmap.d_map.begin();
  }

  Event::Cache<Database::Element>::ContainerIterator end()
  {
    return d_dmap.d_map.end();
  }

} ;

} }

#endif
