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
#ifndef __PROC_DBW_DS_H__
#define __PROC_DBW_DS_H__

#include <stdio.h>

#include <iostream>
#include <map>
#include <string>

#include <assert.h>
#include <stdlib.h>


namespace Proc {

namespace Database {

  class Stream
  {
  public:
  Stream() { ; }
  virtual ~Stream() { ; }

  virtual Stream& operator() () = 0;
  virtual Stream& operator() (const std::string& attr, int val) = 0;
  virtual Stream& operator() (const std::string& attr, double val) = 0;
  virtual Stream& operator() (const std::string& attr, const std::string& val) = 0;
  virtual Stream& operator() (const std::string& attr, unsigned int val) = 0;
  virtual Stream& operator() (const std::string& attr, clocktype val) = 0;

  virtual bool closed() = 0;
  } ;

  struct Element
  {
    std::string d_item;
    std::string d_type;
    std::string d_fmt;
    std::string d_sql;
  
    static std::string type_to_fmt(const std::string& type)
    {
      if (type == "integer")
        return std::string("%ld");

      if (type == "unsigned")
        return std::string("%lu");

      if (type == "real" || type == "double" || type == "float")
        return std::string("%0.6lf");

      if (type == "string" || type == "varchar(8)" || type == "varchar(16)" 
          || type == "varchar(32)" || type == "varchar(64)" || type == "varchar(128)"
          || type == "varchar(256)")
        return std::string("%s");


      if (type == "clocktype")
      {
          return std::string("%lld");
      }

      /* binary not supported yet */
      printf("ABORTING from Proc::Database::Element::type_to_fmt() with type %s\n", type.c_str());
      fflush(NULL);
      abort();

      return std::string(""); // never reached
    }

    static std::string type_to_sql(const std::string& type)
    {
      if (type == "unsigned")
        return std::string("unsigned bigint");
      else if (type == "double")
        return std::string("real");
      else if (type == "string")
        return std::string("varchar(64)");
      else if (type == "binary")
        return std::string("blob");
      else if (type == "float" || type == "real" || type == "integer" 
        || type == "varchar(8)" || type == "varchar(16)" || type == "varchar(32)" || type == "varchar(64)"
        || type == "varchar(128)" || type == "varchar(256)" || type == "clocktype")
        return type;

      printf("ABORTING from Proc::Database::Element::type_to_sql() with type %s\n", type.c_str());
      fflush(NULL);
      abort();

      return std::string("NONE"); // never reached
    }

    Element(const std::string& p_item, const std::string& p_type) 
    : d_item(p_item), d_type(p_type), 
      d_fmt(type_to_fmt(p_type)), d_sql(type_to_sql(p_type)) { ; }

    Element (const std::string& p_item, const std::string& p_type, 
             const std::string& p_fmt) 
    : d_item(p_item), d_type(p_type), d_fmt(p_fmt), 
      d_sql(type_to_sql(p_type)) { ; }

    Element(const std::string& p_item, const std::string& p_type, 
            const std::string& p_fmt, const std::string& p_sql) 
    : d_item(p_item), d_type(p_type), 
      d_fmt(p_fmt), d_sql(p_sql) { ; }

    Element() { ; }
  } ;

} 

}

#endif

