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

#include "node.h"

#include "product_info.h"
#include "db-core.h"
#include "db-mariadb.h"

// #define DB_PERFORMANCE_MONITOR
// #define DB_TRACE_SQL_STATEMENTS

namespace UTIL {
namespace Database {
  static const char* insStr = "INSERT INTO ";
  static const char* valStr = " VALUES";
}
}


UTIL::Database::MariaDBNativeDriver::MariaDBNativeDriver(MariaDBNativeDriver& db) {
    server = db.server;
    username = db.username;
    password = db.password;
    database = db.database;
    socketFilePath = db.socketFilePath;

    maxEngineBuffer = db.maxEngineBuffer;
}


UTIL::Database::MariaDBNativeDriver::MariaDBNativeDriver(std::string p_server,
                                                         std::string p_username,
                                                         std::string p_password,
                                                         std::string p_database,
                                                         std::string p_socketFilePath)
    : server(p_server),
      username(p_username),
      password(p_password),
      database(p_database),
      socketFilePath(p_socketFilePath)
{
    maxEngineBuffer = defaultMaxEngineBuffer;
}


UTIL::Database::MariaDBNativeDriver::~MariaDBNativeDriver()
{
}


const char* UTIL::Database::MariaDBNativeDriver::error()
{
    return mysql_error(&conn);
}


unsigned int UTIL::Database::MariaDBNativeDriver::errorNum()
{
  return mysql_errno(&conn);
}

void UTIL::Database::MariaDBNativeDriver::create()
{
    // Drop the database before opening it
    // The database may not exist, so ignore errors
    std::string sql;
    sql = "DROP DATABASE `" + database + "`;";
    mysql_real_query(&conn, sql.c_str(), (unsigned long)sql.size());


    sql = "CREATE DATABASE IF NOT EXISTS `" + database + "`;" ;

    if (mysql_real_query(&conn, sql.c_str(), (unsigned long)sql.size())) {
      ERROR_ReportErrorArgs("Cannot create simulation database: %s.", error());
      close();
    }
}

void UTIL::Database::MariaDBNativeDriver::open(bool dropDatabase)
{
    if (mysql_init(&conn) == NULL) {
      ERROR_ReportError("Cannot initialize MariaDB API.");
    }


    void* connect = NULL;
    const char* socketPathPtr = (!socketFilePath.empty()) ?
      socketFilePath.c_str() : NULL;

    connect = mysql_real_connect(&conn,
                                     server.c_str(),
                                     username.c_str(),
                                     password.c_str(),
                                     NULL,
                                     0,
                                     socketPathPtr,
                                     0);
    if (!connect)
    {
        ERROR_ReportErrorArgs("Error:%s connecting to MYSQL", error());
    }

    if (dropDatabase) create();

    if (mysql_select_db(&conn, database.c_str())) {
        ERROR_ReportErrorArgs("Error:%s connecting to simulation database:%s", 
            error(), database.c_str());
        close();
    }
}

void UTIL::Database::MariaDBNativeDriver::flush()
{
    for (map_t::iterator pos = inserts.begin();
         pos != inserts.end();
         ++pos)
    {
        if (pos->second.length() > 0)
        {
            std::string sql = insStr + pos->first + valStr + pos->second;
            execNow(sql);
            pos->second = "";
        }
    }
}

void UTIL::Database::MariaDBNativeDriver::exec(const std::string& sql)
{
    size_t insIndex = sql.find(insStr);
    if (insIndex == string::npos)
    {
        flush();
        execNow(sql);
        return;
    }
    insIndex += strlen(insStr);
    size_t valIndex = sql.find(valStr);
    ERROR_AssertArgs(valIndex != string::npos, "DML:%s", sql.c_str());
    std::string tbl = sql.substr(insIndex, valIndex - insIndex);
    valIndex += strlen(valStr);
    string val = sql.substr(valIndex);
    map_t::iterator here = inserts.find(tbl);
    if (here != inserts.end())
    {
      if (here->second.length() + val.length() > maxEngineBuffer)
      {
          std::string sql = insStr + tbl + valStr + here->second;
          execNow(sql);
          here->second = val;
      }
      else if (here->second.length() > 0)
      {
        here->second.append(",");
        here->second.append(val);
      }
      else
      {
        here->second = val;
      }
    }
    else
    {
      inserts[tbl] = val;
    }
}

void UTIL::Database::MariaDBNativeDriver::exec(std::string in, std::string& out)
{
    flush();
     const char* query_c_str = in.c_str();

#ifdef DB_TRACE_SQL_STATEMENTS
    printf("Expect Response: %s\n", query_c_str);
#endif /* DB_TRACE_SQL_STATEMENTS */

#ifdef DB_PERFORMANCE_MONITOR
    clocktype start = WallClock::getTrueRealTime();
#endif /* DB_PERFORMANCE_MONITOR */

    int err = mysql_real_query(&conn,
                                       query_c_str,
                                       (unsigned long)in.size());

    while (err == ER_TABLE_NOT_LOCKED_FOR_WRITE ||
           err == ER_LOCK_OR_ACTIVE_TRANSACTION ||
           err == ER_CANT_UPDATE_WITH_READLOCK)
    {
        sleepCounter++;

        int err = mysql_real_query(&conn,
                                           query_c_str,
                                           (unsigned long)in.size());

        if (err == 0) break;

        if (sleepCounter > MAX_DB_SLEEP_COUNTER)
        {
            close();
            ERROR_ReportErrorArgs("Sleep Timeout: Cannot execute MariaDB statement: %s.", error());
        }
        else if (STATS_DEBUG_LOCK)
        {
            ERROR_ReportWarningArgs("Error in SQL Read: %s, count %d\n", error(), sleepCounter);
        }

        EXTERNAL_Sleep(1 * SECOND);
    }

    if (err > 0) {
        ERROR_ReportErrorArgs("Cannot execute MariaDB statement: %s %s.", in.c_str(), error());
        close();
    }

    MYSQL_RES *result = mysql_store_result(&conn);
    int nrow = (int)mysql_affected_rows(&conn);

    if (nrow < 1) {
        out = "";
    }
    else {
        int ncol = mysql_num_fields(result);
        MYSQL_FIELD *fields = mysql_fetch_fields(result);

        int tblsize = (nrow+1) * ncol;
        char **tbl = new char*[tblsize];
        memset(tbl, 0, sizeof(char *) * tblsize) ;

        for (int c = 0; c < ncol; c++) {
            tbl[c] = fields[c].name;
        }

        MYSQL_ROW row;

        int r = 0;
        while ((row = mysql_fetch_row(result)) != NULL) {
            for (int c = 0; c < ncol; c++) {
              tbl[(r + 1) * ncol + c] = row[c];
            }
            ++r;
        }

        out = marshall(tbl, nrow, ncol);

        delete[] tbl;
    }

    mysql_free_result(result);

#ifdef DB_PERFORMANCE_MONITOR
    clocktype end = WallClock::getTrueRealTime();
    clocktype diff = end - start;

    char temp[MAX_STRING_LENGTH];
    TIME_PrintClockInSecond(diff, temp);

    printf ("@S %s\n", temp);
#endif /* DB_PERFORMANCE_MONITOR */
}

void UTIL::Database::MariaDBNativeDriver::exec(const char* query_str) {

#ifdef DB_TRACE_SQL_STATEMENTS
    printf("Expect NO Response: %s\n", query_str);
#endif /* DB_TRACE_SQL_STATEMENTS */

#ifdef DB_PERFORMANCE_MONITOR
    clocktype start = WallClock::getTrueRealTime();
#endif /* DB_PERFORMANCE_MONITOR */

    sleepCounter = 0;

    int err = mysql_real_query(&conn,
                                   query_str,
                                   (int)strlen(query_str));

    while (err == ER_TABLE_NOT_LOCKED_FOR_WRITE ||
           err == ER_LOCK_OR_ACTIVE_TRANSACTION ||
           err == ER_CANT_UPDATE_WITH_READLOCK)
    {
        sleepCounter++;

        int err = mysql_real_query(&conn,
                                       query_str,
                                       (int)strlen(query_str));

        if (err == 0) break;

        if (sleepCounter > MAX_DB_SLEEP_COUNTER)
        {
            close();
            ERROR_ReportErrorArgs("Sleep Timeout: Cannot execute MariaDB statement: %s.", error());
        }
        else if (STATS_DEBUG_LOCK)
        {
            ERROR_ReportWarningArgs("Error in SQL Read: %s, count %d\n", error(), sleepCounter);
        }

        EXTERNAL_Sleep(1 * SECOND);
    }

    if (err > 0)
    {
        close();
        ERROR_ReportErrorArgs("Cannot execute MariaDB statement: %s.", error());
    }

#ifdef DB_PERFORMANCE_MONITOR
    clocktype end = WallClock::getTrueRealTime();
    clocktype diff = end - start;

    char temp[MAX_STRING_LENGTH];
    TIME_PrintClockInSecond(diff, temp);

    printf ("@I %s\n", temp);
#endif /* DB_PERFORMANCE_MONITOR */
}

void UTIL::Database::MariaDBNativeDriver::execNow(const std::string& query_str) {
    const char* query_c_str = query_str.c_str();

#ifdef DB_TRACE_SQL_STATEMENTS
    printf("Expect NO Response: %s\n", query_c_str);
#endif /* DB_TRACE_SQL_STATEMENTS */

#ifdef DB_PERFORMANCE_MONITOR
    clocktype start = WallClock::getTrueRealTime();
#endif /* DB_PERFORMANCE_MONITOR */

    sleepCounter = 0;

    int err = mysql_real_query(&conn,
                                       query_c_str,
                                       (int)query_str.size());

    while (err == ER_TABLE_NOT_LOCKED_FOR_WRITE ||
           err == ER_LOCK_OR_ACTIVE_TRANSACTION ||
           err == ER_CANT_UPDATE_WITH_READLOCK)
    {
        sleepCounter++;

        int err = mysql_real_query(&conn,
                                   query_c_str,
                                   (int)query_str.size());

        if (err == 0) break;

        if (sleepCounter > MAX_DB_SLEEP_COUNTER)
        {
            ERROR_ReportErrorArgs("Sleep Timeout: Cannot execute MYSQL statement: %s.", error());
            close();
        }
        else if (STATS_DEBUG_LOCK)
        {
            ERROR_ReportWarningArgs("Error in SQL Read: %s, count %d\n", error(), sleepCounter);
        }

        EXTERNAL_Sleep(1 * SECOND);
    }

    if (err > 0)
    {
        ERROR_ReportErrorArgs("SQL Error:%s performing:%s\n", error(), query_c_str);
        close();
    }

#ifdef DB_PERFORMANCE_MONITOR
    clocktype end = WallClock::getTrueRealTime();
    clocktype diff = end - start;

    char temp[MAX_STRING_LENGTH];
    TIME_PrintClockInSecond(diff, temp);

    printf ("@I %s\n", temp);
#endif /* DB_PERFORMANCE_MONITOR */
}

void UTIL::Database::MariaDBNativeDriver::close()
{
    mysql_close(&conn);
}
