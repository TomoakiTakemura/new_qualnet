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

#ifndef _DBSQLITE3_H_
#define _DBSQLITE3_H_

#include <string>

#include "db-core.h"

struct sqlite3;

namespace UTIL {
namespace Database {

class Sqlite3Driver : public DatabaseDriver
{
public:
    Sqlite3Driver(std::string p_dbFileName);
    Sqlite3Driver(Sqlite3Driver& db);

    void open(bool dropDatabase);
    void close();
    void create() {}

    void DriverStartTransaction() {exec("BEGIN");}
    void DriverCommit()           {exec("COMMIT"); flush();}

    void exec(const std::string& query);
    void exec(const char* query);
    void exec(std::string in, std::string& out);

    void flush();
    void flush(const char* cmds);

    const char* fileName() {return dbFileName.c_str();}
private:
    const char* error();

    sqlite3* dbFile;
    std::string dbFileName;
    std::string queryBuffer;
    static const size_t defaultMaxEngineBuffer = 8000;

}; // class Sqlite3Driver
}  // namespace Database
}  // namespace UTIL

#endif

