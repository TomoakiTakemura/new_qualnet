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
#ifndef __PROC_BASE_H__
#define __PROC_BASE_H__

#include <string>
#include <exception>
#include <map>
#include <sstream>
#include <iostream>

#include <list>
#include <map>

#if defined(WIN32) && !defined(__HAS_ISFINITE__)
#define __HAS_ISFINITE__
#include <float.h>
#define isfinite(X) (_isnan(X) == 0)
#endif // WIN32 && !__HAS_ISFINITE__

/*!
 \brief General Proc namespace
 */

namespace Proc {

template <typename T>
class ArgumentStream
{
  std::list<T> d_data;
  std::vector<T> d_vec;
  bool d_vectorized;

public:

  ArgumentStream() : d_vectorized(false) { ; }

  ArgumentStream& operator<<(const T& x)
  {
    if (d_vectorized)
    {
      d_vec.clear();
      d_vectorized = false;
    }

    d_data.push_back(x);
    return *this;
  }

  T& operator[](int idx)
  {
    if (!d_vectorized)
    {
      d_vec.insert(d_vec.begin(), d_data.begin(), d_data.end());
      d_vectorized = true;
    }

    return d_vec[idx];
  }

  int size() { return d_data.size(); }

  void reset()
  {
    d_vec.clear();
    d_data.clear();
    d_vectorized = false;
  }
} ;

template <typename T>
class VariadicInterface
{
public:
  VariadicInterface<T>& operator,(const T& x) { return *this; }
  VariadicInterface<T>& operator=(const T& x) { return *this; }
} ;

template <typename T> 
class Variadic : public VariadicInterface<T>, public ArgumentStream<T>
{
public:

  Variadic() { ; }

  Variadic<T>& operator,(const T& x)
  {
    (*this) << x;
    return *this;
  }

  Variadic<T>& operator=(const T& x)
  {
    ArgumentStream<T>::reset();

    (*this) << x;
    return *this;
  }

} ;


/*!
 \brief Sprintf formatting wrapper class
*/
template <typename T>
static std::string to_fmt(const char* fmt, T data)
{
  char buf[1024]; // const size for now, will make dynamic later
  snprintf(buf, sizeof(buf), fmt, data);
  std::string str(buf);
  return str;
}

/*!
 \brief Rooted base class that encapsulates a Node* and interface index
*/
class Rooted 
{

  Node* d_node;
  int d_ifidx;

protected:

  static std::string locale_to_name(int node_id, int ifidx)
  { 
    std::string str = to_fmt("%d", node_id) + std::string("/") + to_fmt("%d", ifidx);
    return str;
  }

public:

  Rooted(Node* p_node, int p_ifidx) : d_node(p_node), d_ifidx(p_ifidx) { ; }

  Rooted(const Rooted& rooted)
  {
    d_node = rooted.d_node;
    d_ifidx = rooted.d_ifidx;
  }

  ~Rooted() { d_node = NULL; }

  Node* node() const { return d_node; }
  NodeId nodeId() const { return d_node->nodeId; }
  int ifidx() const { return d_ifidx; }
  clocktype now() { return node()->getNodeTime(); }

  void to_s(std::string& str, const std::string& prefix) 
  {
    str += locale();
  }

  std::string locale()
  {
    return locale_to_name(node()->nodeId, ifidx());
  }

} ;

/*!
 \brief Naming model interface class
*/
class NamingModelInterface
{
public:
  virtual ~NamingModelInterface(){};
  virtual std::string rewriteBatchName(const std::string& statName) { return statName; }
  virtual std::string rewriteParamName(const std::string& paramName) { return paramName; }

  virtual std::string cmap(const std::string& item) { return item; }
  virtual std::string dmap(const std::string& attr) { return attr; }
  virtual std::string emap(const std::string& attr) { return attr; }
  virtual std::string tmap(const std::string& table) { return table; }
} ;

} // namespace Proc


#endif /* __PROC_BASE_H__ */
