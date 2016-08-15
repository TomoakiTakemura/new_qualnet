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
#ifndef __PROC_DATAMODEL_H__
#define __PROC_DATAMODEL_H__

#include <string>
#include <exception>
#include <map>
#include <sstream>

#include <assert.h>
#include <stdlib.h>

#ifndef PROC_STANDALONE
#include "api.h"
#include "node.h"
#endif

#include "proc-base.h"
#include "proc-dbw.h"

#include "proc-dbw-ds.h"
#include "proc-dbw-cs.h"

#include "proc-dbw-ds-impl.h"
#include "proc-dbw-cs-impl.h"

#include "proc-stats-db-controller.h"

/*!
 \brief General Proc namespace
*/
namespace Proc {

/*!
 \brief Exception class for unknown statistic
*/
class UnknownStatistic : public std::exception
{
  std::string d_reason;

public:

  UnknownStatistic(const std::string& p_name) 
  {
    d_reason = "Unknown statistic name: " + p_name + ".";
  }

  ~UnknownStatistic() throw() { ; }

  const char* what() const throw()
  {
    return d_reason.c_str();
  }

} ;

/*!
 \brief Exception class for unknown parameter
*/
class UnknownParameter : public std::exception
{
  std::string d_reason;
public:
  UnknownParameter(const std::string& p_name)
  {
    d_reason = "Unknown parameter name: " + p_name + ".";
  }

  ~UnknownParameter() throw() { ; }

  const char* what() const throw()
  {
    return d_reason.c_str();
  }
} ;

/*!
 \brief Data model class
*/
class DataModel : public Rooted
{
  const NodeInput* d_nodeInput;

public:

  DataModel(Node* p_node, int p_phyIndex, const NodeInput* p_nodeInput) 
  : Rooted(p_node, p_phyIndex), d_nodeInput(p_nodeInput) { ; } 
  DataModel(Rooted& p_rooted, const NodeInput* p_nodeInput) 
  : Rooted(p_rooted), d_nodeInput(p_nodeInput) { ; }

  virtual ~DataModel() { ; }

  Database::Stream& dcr(Database::Stream& s)
  {
    return s("NodeId", nodeId())("PhyIndex", ifidx());
  }

  void initialize() { ; }
  virtual void initialized() = 0;

  virtual std::string readParam(const std::string& item) 
  { 
    char buf[MAX_STRING_LENGTH];
    BOOL wasFound(FALSE);

    IO_ReadStringInstance(
        node(),
        node()->nodeId,
        node()->phyData[ifidx()]->macInterfaceIndex,
        nodeInput(),
        item.c_str(),
        ifidx(),
        TRUE,
        &wasFound,
        buf);

    if (wasFound == FALSE)
        return "";

    return std::string(buf); 
  }

  const NodeInput* nodeInput() { return d_nodeInput; }

  void writeBatchStatistic(const std::string& statName, 
                           const std::string& level, const std::string& output_str)
  {
    Node* me = node();
    IO_PrintStat(me, level.c_str(), statName.c_str(), 
                 ANY_DEST, ifidx(), output_str.c_str());
  }

  virtual Database::Stream& ds(const std::string& event_name, long long t) = 0;
  virtual Configuration::Stream& cs(const std::string& dataset)  = 0;
  virtual NodeEventController& nec() = 0;
} ;

/*!
 \brief Mapped data model template class
*/
template <class NamingModel = NamingModelInterface>
class MappedDataModel : public DataModel
{
  NamingModel d_nameModel;

  Event::Cache<Database::Stream> d_dsc;
  Event::Cache<Configuration::Stream> d_csc;
  Event::Cache<Database::Element> d_dds;

  Event::Cache<std::string> d_dem;

  NodeEventController& d_nec;

  bool d_initialized;

protected:


  Database::Stream* dfactory(long long ct, const std::string& event_name)
  {
    unsigned long long seq = d_nec.seq();
    double rt = (double)ct / 1.0e9;

    std::string remapped_event = d_nameModel.emap(event_name);
    const std::string& remapped_dataset = *(d_dem.find(remapped_event));

    Database::Stream* impl = (Database::Stream*)
      new Database::Implementation<NamingModel>(node(), ifidx(), rt, seq, remapped_dataset, remapped_event, d_dds);

    return impl;
  }

  Configuration::Stream* cfactory(const std::string& dataset)
  {
    Configuration::Stream* impl = (Configuration::Stream*)
      new Configuration::Implementation<NamingModel>(dataset, d_dem, d_dds);

    (*impl)("EventTimestamp","real")("EventSequence","double")
           ("EventName","varchar(64)")("NodeID","integer")
           ("InterfaceIndex", "integer")
           ["PhyReceived"]["PhySent"]
           ["MacSent"]["MacReceived"]
           ["NavUpdated"]["NavEnd"]
           ["IFS"];

    return impl;
  }

  std::string qualify_event_name(const std::string& event_name, long long t)
  {
    std::string time_str = to_fmt<long long>("%lld", t);
    return locale() + std::string("/") + event_name + std::string("/") + time_str;
  }

  void gc(long long t_now)
  {
    double horizon = (double)t_now / 1.0e9;

    std::map<std::string, Database::Stream*>::iterator pos = d_dsc.d_map.begin();
    while (pos != d_dsc.d_map.end())
    {
      Database::Implementation<NamingModel>* impl = (Database::Implementation<NamingModel>*)pos->second;
      if (impl->t() < horizon || impl->closed())
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

public:

  Event::Cache<Database::Element>& dds() { return d_dds; }
  NodeEventController& nec() { return d_nec; }

  MappedDataModel(NodeEventController& p_nec, const NodeInput* p_nodeInput) 
      : DataModel(p_nec, p_nodeInput), d_nec(p_nec), d_initialized(false) { ; }

  std::string readParam(const std::string& item)
  {
    std::string mapped_item = d_nameModel.rewriteParamName(item);
    return DataModel::readParam(mapped_item);
  }


  void writeBatchStatistic(const std::string& statName, 
                           const std::string& item, const::string value)
  {
    std::string mapped_stat = d_nameModel.rewriteStatName(statName);
    std::string stat_level = d_nameModel.statLevel();
    std::string stat_output = item + "=" + value;

    DataModel::writeBatchStatistic(mapped_stat, stat_level, stat_output);
  }

  Configuration::Stream& cs(const std::string& dataset)
  {
    assert(!d_initialized);
    std::string remapped_dataset = d_nameModel.tmap(dataset);

    Configuration::Stream* impl = d_csc.find(remapped_dataset);

    if (impl == NULL)
    {
      impl = cfactory(remapped_dataset);
      d_csc.insert(remapped_dataset, impl);
    }


    return *impl;
  }

  Database::Stream& ds(const std::string& event_name, long long t)
  {
    std::string remapped_event = d_nameModel.emap(event_name);
    std::string qualified_event_name = qualify_event_name(remapped_event, t);

    gc(t);

    Database::Stream* impl = d_dsc.find(qualified_event_name);

    if (impl == NULL)
    {
      impl = dfactory(t, remapped_event);
      d_dsc.insert(qualified_event_name, impl);
    }

    return *impl;
  }

  void initialized()
  { 
      d_initialized = true; 

#ifdef ADDON_DB
      if (node()->partitionData->statsDb != NULL)
      {
          for (Event::Cache<Configuration::Stream>::ContainerIterator streamIt = d_csc.d_map.begin(); streamIt != d_csc.d_map.end();
              streamIt++)
          {
              DB::StatsDBTableInterestDetail tableInterest;

              for (Event::Cache<Database::Element>::ContainerIterator elementIt = ((Configuration::Implementation<NamingModel>*)streamIt->second)->begin();
                  elementIt != ((Configuration::Implementation<NamingModel>*)streamIt->second)->end();
                  elementIt++)
              {
                  tableInterest.columns.insert(DB::ColumnDetails(elementIt->first, elementIt->second->d_sql));
              }
              node()->partitionData->dbController->RegisterTableInterest(streamIt->first, tableInterest);
          }
      }
#endif
  }

  void to_s(std::string& str, std::string prefix)
  {
    str += prefix + "MappedDataModel {" + "\n";
    std::string new_prefix = prefix + "  ";
    str += new_prefix + "\n";
    str += prefix + "}";
  }
} ;

} // namespace Proc

#endif /* __PROC_DATAMODEL_H__ */
