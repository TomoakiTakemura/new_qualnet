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

#include <string>
#include <vector>
#include <iostream>
#include <list>

#include "fileio.h"
#include "node.h"

#include "db-core.h"
#include "db-sqlite3.h"
#include "sqlite3.h"

UTIL::Database::Sqlite3Driver::Sqlite3Driver(std::string p_dbFileName)
    : dbFileName(p_dbFileName)
{
  engineType = dbSqlite;
  maxEngineBuffer = defaultMaxEngineBuffer;
  queryBuffer.reserve(maxEngineBuffer);
}

UTIL::Database::Sqlite3Driver::Sqlite3Driver(Sqlite3Driver& db)
{
    engineType = dbSqlite;
    dbFileName = db.dbFileName;
    maxEngineBuffer = db.maxEngineBuffer;
    if (maxEngineBuffer == 0) maxEngineBuffer = defaultMaxEngineBuffer;
    queryBuffer.reserve(maxEngineBuffer);
}

const char* UTIL::Database::Sqlite3Driver::error()
{
    return sqlite3_errmsg(dbFile);
}

void UTIL::Database::Sqlite3Driver::close()
{
    if (STATS_DEBUG) printf("SQLITE3:closing\n");
    flush();
    sqlite3_close(dbFile);
    if (STATS_DEBUG) printf("SQLITE3:closed\n");
}

void UTIL::Database::Sqlite3Driver::open(bool dropDatabase)
{
    int err;

    err = sqlite3_enable_shared_cache(1);

    if (err != SQLITE_OK)
    {
        ERROR_ReportErrorArgs("Problem setting Shared Cache mode, error %d\n", err);
    }

    // Remove a db file before opening it
    // The file may not exist, so ignore errors
    if (dropDatabase)
    {
        remove(dbFileName.c_str());
    }

    err = sqlite3_open(dbFileName.c_str(), &dbFile);

    if (err)
    {
        ERROR_ReportErrorArgs("Can't open database: %s", error());
        close();
    }

    exec("PRAGMA synchronous=OFF");
    flush();
}

void UTIL::Database::Sqlite3Driver::exec(const std::string& query)
{
    if (queryBuffer.length() + query.size() + 1 > maxEngineBuffer) flush();
    queryBuffer.append(query);
    queryBuffer.append(";");
}

void UTIL::Database::Sqlite3Driver::flush()
{
    flush(queryBuffer.c_str());
    queryBuffer.clear();
}

void UTIL::Database::Sqlite3Driver::flush(const char* cmds)
{
    char* errMsg = NULL;
    sleepCounter = 0;

    int err = sqlite3_exec(dbFile,
                           cmds,
                           0,
                           0,
                           &errMsg);

    while (err == SQLITE_LOCKED || err == SQLITE_BUSY)
    {
        sleepCounter++;

        err = sqlite3_exec(dbFile,
                           cmds,
                           0,
                           0,
                           &errMsg);

        if (err == SQLITE_OK) break;

        if (sleepCounter > MAX_DB_SLEEP_COUNTER)
        {
            ERROR_ReportErrorArgs("Sleep Timeout: Cannot execute SQLite statement: %s.", error());
            close();
        }
        else if (STATS_DEBUG_LOCK)
        {
            ERROR_ReportWarningArgs("Error in SQL Read: %s, count %d\n", error(), sleepCounter);
        }

        EXTERNAL_Sleep(1 * SECOND);
    }

    if (err != SQLITE_OK)
    {
        ERROR_ReportWarningArgs("Error:%s\nDML:%s\n", errMsg, cmds);
        sqlite3_free(errMsg);
    }
}

void UTIL::Database::Sqlite3Driver::exec(const char* query)
{
    std::string strQuery( query);
    exec(strQuery);
}

void UTIL::Database::Sqlite3Driver::exec(std::string in, std::string& out)
{
    int err(0);
    char** tbl;
    int nrow(0);
    int ncol(0);
    char *errMsg(NULL);

#if defined(TRACE_SQL)
    printf("ExecuteAsString receives query: \"%s\"\n", in.c_str());
#endif

    out = "";
    sleepCounter = 0;
    err = sqlite3_get_table(dbFile,
                            in.c_str(),
                            &tbl,
                            &nrow,
                            &ncol,
                            &errMsg);

    while (err == SQLITE_LOCKED || err == SQLITE_BUSY)
    {
        sleepCounter++;

        err = sqlite3_get_table(dbFile,
                                in.c_str(),
                                &tbl,
                                &nrow,
                                &ncol,
                                &errMsg);

        if (err == SQLITE_OK) break;

        if (sleepCounter > MAX_DB_SLEEP_COUNTER)
        {
          close();
          ERROR_ReportErrorArgs("Sleep Timeout: Cannot execute SQLite statement: %s.", errMsg);
        }
        else if (STATS_DEBUG_LOCK)
        {
            ERROR_ReportWarningArgs("Error in SQL Read: %s, count %d\n", error(), sleepCounter);
        }
        EXTERNAL_Sleep(1 * SECOND);
    }

    if (err != SQLITE_OK)
    {
        sqlite3_close(dbFile);
        ERROR_ReportErrorArgs("SQL error: %s\n", errMsg);
    }

#if defined(DEBUG_MARSHALL)
    printf("ExecuteAsString found [%d, %d]\n", nrow, ncol);
#endif

    // The result data structure captures the result from the database.
    // We now use these results to update the stats.

    if (ncol == 0 && nrow == 0)
    {
      // We have no data to process.
      out = "";
    }
    else
    {
      out = marshall(tbl, nrow, ncol);
    }

    sqlite3_free_table(tbl);

#if defined(DEBUG_MARSHALL)
    printf("ExecuteAsString returns response \"%s\"\n", out.c_str());
#endif
}


