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

#ifndef _DBMARIADB_NATIVE_H_
#define _DBMARIADB_NATIVE_H_

#include <string>
#include <vector>
#include <iostream>
#include <list>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#pragma warning(disable: 4514 4786)
#pragma warning(push, 3)
#endif /* WIN32 */

#if defined (__unix) || defined(__APPLE__)
#include <dlfcn.h>
#endif

#include <mysql.h>
#include <mysqld_error.h>

// #ifdef WIN32
// #include <unistd.h>
// #endif /* WIN32 */

#include "fileio.h"
#include "node.h"
#include "gestalt.h"

#include "db-core.h"

// #define DB_PERFORMANCE_MONITOR
// #define DB_TRACE_SQL_STATEMENTS

namespace UTIL {
namespace Database {

struct MariaDBNativeDriver : public DatabaseDriver
{
    MYSQL conn;
    std::string server;
    std::string username;
    std::string password;
    std::string database;
    std::string socketFilePath;

    typedef std::map<std::string, std::string> map_t;
    map_t inserts;
    static const size_t defaultMaxEngineBuffer = 1000000;

    MariaDBNativeDriver(MariaDBNativeDriver& db);

    MariaDBNativeDriver(std::string p_server,
                      std::string p_username,
                      std::string p_password,
                      std::string p_database,
                      std::string p_socketFilePath);

    ~MariaDBNativeDriver();
    const char* error();
    unsigned int errorNum();
    void create();
    void open(bool dropDatabase);

    void DriverStartTransaction() {execNow("START TRANSACTION");}
    void DriverCommit()           {flush(); execNow("COMMIT");}

    void flush();
    void exec(const std::string& sql);
    void exec(std::string in, std::string& out);
    void exec(const char* query_str);
    void execNow(const std::string& query_str);
    void close();
};

}  // namespace Database
}  // namespace UTIL


#endif
