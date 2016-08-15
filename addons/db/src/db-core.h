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

#ifndef _DBCORE_H_
#define _DBCORE_H_

#include <string>

#include "node.h"

namespace UTIL {
namespace Database {

#define MAX_DB_SLEEP_COUNTER 10

#if !defined(STATS_DEBUG)
#define STATS_DEBUG false
#endif
#if !defined(STATS_DEBUG_LOCK)
#define STATS_DEBUG_LOCK 0
#endif

enum dbEngineType
{
    dbSqlite,
    dbMariaDB,
    dbMaxEngineType
};

struct DatabaseDriver
{
    virtual void open(bool dropDatabase) = 0;
    virtual void close() = 0;

    DatabaseDriver() : inTransaction(false) { }

    void startTransaction()
    {
        if (inTransaction) return;
        DriverStartTransaction();
        inTransaction = true;
    }

    void commit()
    {
        if (!inTransaction) return;
        DriverCommit();
        inTransaction = false;
    }

    void flush()
    {
        if (inTransaction) commit();
        DriverFlush();
    }

    virtual void DriverStartTransaction() { }
    virtual void DriverCommit() { }
    virtual void DriverFlush() { }

    virtual const char* error() = 0;

    virtual void exec(const std::string& query) = 0;
    virtual void exec(std::string in, std::string& out) = 0;

    // the properties and methods below should probably be private,
    // in that they are clearly implimentation details.  However
    // it all works like this and there is little real gain from
    // the access control.
    dbEngineType engineType;
    unsigned sleepCounter;
    size_t maxEngineBuffer;
    bool inTransaction;

    static std::string marshall(char** table, int nrow, int ncol) {
        std::string result;
        //char buf[BUFSIZ];

        for (int row(0); row < (nrow+1); row++)
        {
            for (int col(0); col < ncol; col++)
            {
                int index = row * ncol + col;
                std::string val ;
                if (table[index] != NULL)
                {
                    val = table[index];
                }
                result += val;

                if (col != ncol-1)
                {
                    result += ",";
                }
            }

            if (row != nrow)
            {
                result += "|";
            }
        }

        return result;
    }

    static void free_table(char** table, int nrow, int ncol)
    {
        if (table != NULL)
        {
            int n = (nrow + 1) * ncol;

            for (int k = 0; k < n; k++)
            {
                delete[] table[k];
            }

            delete[] table;
       }
    }

    static void unmarshall(std::string str, char**& table, int& nrow, int& ncol)
    {
        size_t baridx(0);
        size_t rowStart;

        table = NULL;
        nrow = 0;
        ncol = 0;

        if (str == "")
        {
            return;
        }

        // Calculate num rows
        rowStart = 0;
        do
        {
            nrow++;
            baridx = str.find("|", rowStart);
            rowStart = baridx + 1;
        } while (baridx != std::string::npos);
        nrow--;

        // Calculate num cols
        size_t commaIdx = 0;
            baridx = str.find("|", 0);
        ncol = 0;
        do
        {
            ncol++;
            commaIdx = str.find(",", commaIdx + 1);
        } while (commaIdx < baridx);

        // Allocate table
        table = new char*[(nrow + 1) * ncol];


        bool done(false);
        int pos(0);
        rowStart = 0;
        do {
            // Find next | barrier (end of row)
            baridx = str.find("|", rowStart);
            std::string row_str;

            // Get this row as a string
            if (baridx != std::string::npos) {
                row_str = str.substr(rowStart, baridx - rowStart);
            }
            else {
                row_str = str.substr(rowStart);
                done = true;
            }

            // Read each column of this row, inserting to the table
            size_t comma_idx(0);
            for (int i = 0; i < ncol; i++)
            {
                comma_idx = row_str.find(",", 0);
                std::string col_str = row_str.substr(0, comma_idx);
                row_str = row_str.substr(comma_idx+1);

                table[pos] = new char[col_str.size() + 1];
                strcpy(table[pos], col_str.c_str());
                pos++;

            }

            // Advance to next row
            rowStart = baridx + 1;
        } while (!done);
        assert(pos == (nrow + 1) * ncol);
    }
};

}  // namepsace Database
}  // namespace UTIL

#endif
