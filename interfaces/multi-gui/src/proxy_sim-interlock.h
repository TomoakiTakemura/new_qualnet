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

#ifndef _PROXY_INTERLOCK_H_
#define _PROXY_INTERLOCK_H_

#include <string>
#include <vector>
#include <iostream>
#include <list>

#define MAX_MDBOBJECT_LIST 100
#define MIN_MDBOBJECT_LIST 5

#include "proxy_util_interlock.h"

extern void Proxy_SendToSecondaryGui(const char* strFromMaster, size_t size);
extern void Proxy_SendSteppedCmd();

class ProxySimInterlock : public UTIL::Interlock<std::string>, public UTIL::Worker<std::string>
{
public:
    ProxySimInterlock()
    : UTIL::Interlock<std::string>("ProxySim Interlock",
                                   false)
    {
        receivedStepCmd = false;
    }

    void start()
    {
        setWorker(this);
    }

    void run(std::list<std::string>& list)
    {
        std::list<std::string>::iterator pos = list.begin();

        for (; pos != list.end(); pos++)
        {
            std::string cmd = *pos;
            Proxy_SendToSecondaryGui(cmd.c_str(),cmd.size());
        }
        list.clear();
        setReceivedStepCmdFlag(false);
    }

    int push_back(std::string cmd)
    {
        take();
        int size = UTIL::Interlock<std::string>::push_back_xxx(cmd);
        give();
        return size;
    }

    void close()
    {
        receivedStepCmd = false;
    }
    bool dropData()
    {
        int size = UTIL::Interlock<std::string>::size();
        if (size >= MAX_MDBOBJECT_LIST)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    void startSendingDataToGui()
    {
        int size = UTIL::Interlock<std::string>::size();
        if (getReceivedStepCmdFlag() && size >= MIN_MDBOBJECT_LIST)
        {
            force_wake();
        }
    }
    void setReceivedStepCmdFlag(bool flag)
    {
        take();
        receivedStepCmd = flag;
        if (!flag)
        {
            Proxy_SendSteppedCmd();
        }
        give();
    }
    volatile bool getReceivedStepCmdFlag()
    {
        take();
        bool flag = receivedStepCmd;
        give();
        return flag;
    }
private :
    volatile bool receivedStepCmd;
};


#endif

