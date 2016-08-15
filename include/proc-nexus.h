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
#ifndef __PROC_NEXUS_H__
#define __PROC_NEXUS_H__

#include <string>
#include <set>

#include "main.h"
#include "api.h"
#include "partition.h"
#include "node.h"
#include "network_ip.h"

#include "mac.h"
#include "phy.h"

#include "fileio.h"
#include "api.h"

#include "proc-stats-db-controller.h"

namespace Proc
{
namespace Nexus
{

namespace DataModel
{

void DumpEvent(Node* node, int phyIndex, const std::string& tableName,
               const std::string& eventName, unsigned long long seq, 
               double t, std::set<Proc::DB::ColumnDetails>& details);

}

} // Nexus

} // Mode 5



#endif /* __PROC_NEXUS_H__ */
