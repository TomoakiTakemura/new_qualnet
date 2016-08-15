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
#ifndef __PROC_DBW_DS_IMPL_H__
#define __PROC_DBW_DS_IMPL_H__


#include <stdio.h>

#include <iostream>
#include <map>
#include <string>
#include <set>

#include <assert.h>
#include <stdlib.h>

#include "proc-stats-db-controller.h"

#include "proc-dbw-node-kernel.h"
#include "proc-dbw-ds.h"

#include "proc-nexus.h"


namespace Proc {
namespace Database {

  template <class T>
  class Implementation : public Stream
  {
    T d_nm;
    const Event::Cache<Database::Element>& d_dmap;

    std::string d_evname;
    double d_t;
    unsigned long long d_seq;

    Node* d_node;
    int d_phyIndex;

    std::set<DB::ColumnDetails> d_data;

    std::string d_table_name;

    bool d_closed;

    protected:
 
    void flush()
    {
      if (!d_closed)
      {
        Proc::Nexus::DataModel::DumpEvent(d_node, d_phyIndex, 
          d_table_name, d_evname, d_seq, d_t, d_data); 

        d_closed = true;
      }
    }

    bool oc()
    {
      if (d_closed)
      {
        ERROR_ReportWarning("Attempting to write to closed data stream, dropping data.");
      }
      return !d_closed;
    }

    public:

    Implementation(Node* node, int phyIndex, double t, unsigned long long seq, 
                   const std::string& p_table_name, const std::string& p_event_name, 
                   const Event::Cache<Database::Element>& p_dmap)
    : Stream(), d_node(node), d_phyIndex(phyIndex), d_table_name(p_table_name),
      d_nm(), d_t(t), d_seq(seq), d_evname(p_event_name), d_dmap(p_dmap), d_data(), d_closed(false) { ; }

    ~Implementation() { flush(); }

    Stream& operator()() { flush(); return *this; }

    Stream& operator() (const std::string& attr, int val)
    {
      if (oc())
      {
        std::string remapped_attr = d_nm.dmap(attr);
        const Database::Element* elem = d_dmap.find(remapped_attr);

        if (elem != NULL)
        {
          const Element& dbd = *elem;

          char buf[1024];
          sprintf(buf, dbd.d_fmt.c_str(), val);
          std::string refmt_str(buf);
  
          DB::ColumnDetails p(remapped_attr, refmt_str);
          d_data.insert(p);

          //std::cout << "{" << attr << ", " << val 
          //          << "} => {"<< remapped_attr << ", " << refmt_str 
          //          << "}" << std::endl;
        }
        else
        {
          printf("ABORTING from Proc::Database::Implementation::operator() with signed int element %s not found\n", attr.c_str());
          fflush(NULL);
          abort();
        }
      }

      return *this;
    }

    Stream& operator() (const std::string& attr, unsigned int val)
    {
      if (oc())
      {
        std::string remapped_attr = d_nm.dmap(attr);
        const Database::Element* elem = d_dmap.find(remapped_attr);

        if (elem != NULL)
        {
          const Element& dbd = *elem;

          char buf[1024];
          sprintf(buf, dbd.d_fmt.c_str(), val);
          std::string refmt_str(buf);

          DB::ColumnDetails p(remapped_attr, refmt_str);
          d_data.insert(p);

          //std::cout << "{" << attr << ", " << val 
          //          << "} => {"<< remapped_attr << ", " << refmt_str 
          //          << "}" << std::endl;
        }
        else
        {
          printf("ABORTING from Proc::Database::Implementation::operator() with unsigned int element %s not found\n", attr.c_str());
          fflush(NULL);
          abort();
        }
      }

      return *this;
    }
      
    Stream& operator() (const std::string& attr, double val)
    {
      if (oc())
      {
        std::string remapped_attr = d_nm.dmap(attr);
        const Database::Element* elem = d_dmap.find(remapped_attr);

        if (elem != NULL)
        {
          const Element& dbd = *elem;
  
          char buf[1024];
          sprintf(buf, dbd.d_fmt.c_str(), val);
          std::string refmt_str(buf);
  
          DB::ColumnDetails p(remapped_attr, refmt_str);
          d_data.insert(p);

          //std::cout << "{" << attr << ", " << val 
          //          << "} => {"<< remapped_attr << ", " 
          //          << refmt_str << "}" << std::endl;
        }
        else
        {
          printf("ABORTING from Proc::Database::Implementation::operator() with double element %s not found\n", attr.c_str());
          fflush(NULL);
          abort();
        }
      }

      return *this;
    }

    Stream& operator() (const std::string& attr, const std::string& val)
    {
      if (oc()) 
      {
        std::string remapped_attr = d_nm.dmap(attr);
        const Database::Element* elem = d_dmap.find(remapped_attr);

        if (elem != NULL)
        {
          const Element& dbd = *elem;
  
          char buf[1024];
          sprintf(buf, dbd.d_fmt.c_str(), val.c_str());
          std::string refmt_str(buf);

          DB::ColumnDetails p(remapped_attr, refmt_str);
          d_data.insert(p);

          //std::cout << "{" << attr << ", " << val 
          //          << "} => {"<< remapped_attr << ", " 
          //          << refmt_str << "}" << std::endl;
        }
        else
        {
          printf("ABORTING from Proc::Database::Implementation::operator() with string element %s not found\n", attr.c_str());
          fflush(NULL);
          abort();
        }
      }
      
      return *this;
    }

    Stream& operator() (const std::string& attr, clocktype val)
    {
      if (oc())
      {
        std::string remapped_attr = d_nm.dmap(attr);
        const Database::Element* elem = d_dmap.find(remapped_attr);

        if (elem != NULL)
        {
          const Element& dbd = *elem;

          char buf[1024];
          sprintf(buf, dbd.d_fmt.c_str(), val);
          std::string refmt_str(buf);

          DB::ColumnDetails p(remapped_attr, refmt_str);
          d_data.insert(p);

          //std::cout << "{" << attr << ", " << val
          //          << "} => {"<< remapped_attr << ", " << refmt_str
          //          << "}" << std::endl;
        }
        else
        {
          printf("ABORTING from Proc::Database::Implementation::operator() with clocktype element %s not found\n", attr.c_str());
          fflush(NULL);
          abort();
        }
      }

      return *this;
    }

    double t() { return d_t; }

    bool closed() { return d_closed; }
  }; 

}

}

#endif
