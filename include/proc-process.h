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
#ifndef __PROC_PROCESS_H__
#define __PROC_PROCESS_H__

#include <string>
#include <exception>

#include "proc-base.h"
#include "proc-datamodel.h"

namespace Proc {

/*!
 \brief Exception class for failure to initialize
*/
class CannotInitialize : public std::exception
{
  std::string d_reason;
public:
  CannotInitialize(const std::string& p_reason) : d_reason(p_reason) { ; }
  ~CannotInitialize() throw() { ; }

  const char* what() const throw()
  {
    return d_reason.c_str();
  }
} ;

/*!
 \brief Process base class used by Applications among other objects
*/
class Process : public Rooted
{
  DataModel& d_dataModel;

protected:

  DataModel& dm() { return d_dataModel; }

public:

  Process(DataModel& p_dataModel)
  : Rooted(p_dataModel), d_dataModel(p_dataModel)
  { ; }

  virtual ~Process() { ; }

  virtual void initialize() { ; }
  virtual void finalize() { ; }

  virtual void enable() { ; }
  virtual void disable() { ; }
  virtual void reset() { disable(); }

  bool enabled() { return true; }

  //virtual void layer(Message* msg, Phy::SignalStatus* status = NULL) { MESSAGE_Free(node(), msg); }
   virtual void runTimeStat() { ; }

  virtual void to_s(std::string& str, std::string prefix)
  {
    str += prefix + "Process {" + "\n";
    std::string new_prefix = prefix + "  ";
    str += new_prefix + "\n";
    str += prefix + "}" ;
  }

} ;

} // namespace Proc

#endif /* __PROC_PROCESS_H__ */
