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

#include "proc-stats-db-controller.h"
#include "partition.h"
#include "api.h"
#include <iostream>

#ifdef ADDON_DB
#include "dbapi.h"
#endif
#include "proc-nexus.h"
#include "proc-base.h"

Proc::DB::StatsDBController::StatsDBController()
{
    partitionCheckinCount = 0;
}

void Proc::DB::StatsDBController::RegisterTableInterest(const std::string& tableName,
    StatsDBTableInterestDetail interest)
{
    //** this will automatically merge new with existing ones in dbTableInterests (previous registrations)

    if (dbTableInterests.find(tableName) != dbTableInterests.end())
    {
        //table interest already exists, get a pointer to the existing table, so
        //you don't have to keep querying the map
        StatsDBTableInterestDetail* existing = &dbTableInterests[tableName];

        //first merge the primary keys
        for (std::set<std::string>::iterator it = interest.primaryKeys.begin();
                it != interest.primaryKeys.end(); it++)
        {
            existing->primaryKeys.insert(*it);
        }

        //now merge the columns
        for (std::set<ColumnDetails>::iterator it = interest.columns.begin();
                it != interest.columns.end(); it++)
        {
            //only new column names will be inserted
            existing->columns.insert(*it);
        }
    }
    else
    {
        //this is the first interest for the table
        dbTableInterests[tableName] = interest;
        pendingTableRowsPerTable[tableName] = pendingTableRows();
    }
}

void Proc::DB::StatsDBController::SendSerializedTableInterestsToPZero()
{
    //The serialized format is the following...
    //"<table-name>;pk1,pk2,p3;name,type;name,type;|<table-name>;p1;name,type;name,type;"

    std::string serialized;
    //loop over each table that has been registered on this partition
    for (std::map<std::string, StatsDBTableInterestDetail>::iterator it = dbTableInterests.begin();
            it != dbTableInterests.end(); it++)
    {
        //add the table name
        serialized += it->first;
        serialized += ";";

        //first loop over primary keys
        for (std::set<std::string>::iterator it2 = it->second.primaryKeys.begin();
                it2 != it->second.primaryKeys.end(); it2++)
        {
            serialized += *it2;
            serialized += ",";
        }
        //chop the last comma
        serialized = serialized.substr(0, serialized.size()-1);

        serialized += ";";

        //now loop over the columns
        for (std::set<ColumnDetails>::iterator it2 = it->second.columns.begin();
                it2 != it->second.columns.end(); it2++)
        {
            serialized += it2->name;
            serialized += ",";
            serialized += it2->typeOrValue;
            serialized += ";";
        }

        serialized += "|";
    }
    //chop the last table's | delimeter
    if (serialized.size() > 0)
    {
        serialized = serialized.substr(0, serialized.size()-1);
    }

    //now create a partition event message (not a node event) for p0
    Message* serializedTablesMsg = MESSAGE_Alloc(partition->firstNode,
        STATSDB_CONTROLLER_LAYER,
        0,
        MSG_STATSDB_CONTROLLER_SerializedTableInterest);

    size_t stringSize = serialized.size() + 1;  //need to include space for the null terminating char

    //create a info field for the message to store the serialized table info,
    //pick an info field size based on string size
    if (stringSize <= SMALL_TABLE_STRING_SIZE)
    {
        INFO_SerializedTableInterest<SMALL_TABLE_STRING_SIZE>* info =
            (INFO_SerializedTableInterest<SMALL_TABLE_STRING_SIZE>*)
            MESSAGE_InfoAlloc(partition->firstNode, serializedTablesMsg,
                sizeof(INFO_SerializedTableInterest<SMALL_TABLE_STRING_SIZE>));

        memset(info->stringBuffer, 0, sizeof(info->stringBuffer));
        sprintf(info->stringBuffer, "%s", serialized.c_str());
        info->stringSize = stringSize;
    }
    else if (stringSize <= MEDIUM_TABLE_STRING_SIZE)
    {
        INFO_SerializedTableInterest<MEDIUM_TABLE_STRING_SIZE>* info =
            (INFO_SerializedTableInterest<MEDIUM_TABLE_STRING_SIZE>*)
            MESSAGE_InfoAlloc(partition->firstNode, serializedTablesMsg,
                sizeof(INFO_SerializedTableInterest<MEDIUM_TABLE_STRING_SIZE>));

        memset(info->stringBuffer, 0, sizeof(info->stringBuffer));
        sprintf(info->stringBuffer, "%s", serialized.c_str());
        info->stringSize = stringSize;
    }
    else if (stringSize <= LARGE_TABLE_STRING_SIZE)
    {
        INFO_SerializedTableInterest<LARGE_TABLE_STRING_SIZE>* info =
            (INFO_SerializedTableInterest<LARGE_TABLE_STRING_SIZE>*)
            MESSAGE_InfoAlloc(partition->firstNode, serializedTablesMsg,
                sizeof(INFO_SerializedTableInterest<LARGE_TABLE_STRING_SIZE>));

        memset(info->stringBuffer, 0, sizeof(info->stringBuffer));
        sprintf(info->stringBuffer, "%s", serialized.c_str());
        info->stringSize = stringSize;
    }
    else if (stringSize <= EXTRA_LARGE_TABLE_STRING_SIZE)
    {
        INFO_SerializedTableInterest<EXTRA_LARGE_TABLE_STRING_SIZE>* info =
            (INFO_SerializedTableInterest<EXTRA_LARGE_TABLE_STRING_SIZE>*)
            MESSAGE_InfoAlloc(partition->firstNode, serializedTablesMsg,
                sizeof(INFO_SerializedTableInterest<EXTRA_LARGE_TABLE_STRING_SIZE>));

        memset(info->stringBuffer, 0, sizeof(info->stringBuffer));
        sprintf(info->stringBuffer, "%s", serialized.c_str());
        info->stringSize = stringSize;
    }
    else if (stringSize <= EXTRA_EXTRA_LARGE_TABLE_STRING_SIZE)
    {
        INFO_SerializedTableInterest<EXTRA_EXTRA_LARGE_TABLE_STRING_SIZE>* info =
            (INFO_SerializedTableInterest<EXTRA_EXTRA_LARGE_TABLE_STRING_SIZE>*)
            MESSAGE_InfoAlloc(partition->firstNode, serializedTablesMsg,
                sizeof(INFO_SerializedTableInterest<EXTRA_EXTRA_LARGE_TABLE_STRING_SIZE>));

        memset(info->stringBuffer, 0, sizeof(info->stringBuffer));
        sprintf(info->stringBuffer, "%s", serialized.c_str());
        info->stringSize = stringSize;
    }
    else
    {
        char errorString[MAX_STRING_LENGTH];
        sprintf(errorString, "Serialized DB Table interest for partition %d is too big for any availble info struct\n", partition->partitionId);
        ERROR_ReportError(errorString);
    }

#ifdef PARALLEL
    serializedTablesMsg->nodeId = ANY_DEST;
    serializedTablesMsg->eventTime = partition->safeTime;   //we have not started the sim yet, so safetime will be 0S
    MESSAGE_SetLooseScheduling(serializedTablesMsg);

    //Now send the message directly to partition zero
    PARALLEL_SendRemoteMessages(serializedTablesMsg,
                                partition,
                                0);
#ifdef USE_MPI

    MESSAGE_Free(partition->firstNode, serializedTablesMsg);
#endif
#endif
}


void Proc::DB::StatsDBController::HandleSerializedTableInterestsFromOtherPartition(
    Node* node,
    Message* serializedTableDataMsg)
{
    if (partition->partitionId != 0)
    {
        //This message should only ever be received on p0
        //if not delete the message immediately
        MESSAGE_Free(node, serializedTableDataMsg);
        return;
    }
    //extract the serialized contents and merge it into dbTableInterests
    INFO_SerializedTableInterest<EXTRA_EXTRA_LARGE_TABLE_STRING_SIZE>* info =
        (INFO_SerializedTableInterest<EXTRA_EXTRA_LARGE_TABLE_STRING_SIZE>*)
            MESSAGE_ReturnInfo(serializedTableDataMsg);

    //We always include room from the null terminating string, so if a partition's
    //serialized string was blank the size is still 1. if its equal to one the other
    //partition had no registered interests so no merging is needed
    if (info->stringSize > 1)
    {
        char tempStr[EXTRA_EXTRA_LARGE_TABLE_STRING_SIZE];
        char* tempStrPointer = tempStr;
        memcpy(tempStr, info->stringBuffer, info->stringSize);
        char* p = NULL;
        char fieldStr[MAX_STRING_LENGTH] = "";
        char entireTable[MEDIUM_TABLE_STRING_SIZE];

        while (strcmp(tempStrPointer, "") != 0)
        {
            //first get an entire table record
            p = IO_GetDelimitedToken(entireTable, tempStrPointer, "|", &tempStrPointer);

            //build up a StatsDBTableInterestDetail object then simply calls RegisterTableInterest to merge it here on p0
            StatsDBTableInterestDetail interestDetail;

            IO_TrimLeft(entireTable);
            IO_TrimRight(entireTable);

            char* entireTablePointer = entireTable;

            //now get the table name
            p = IO_GetDelimitedToken(fieldStr, entireTablePointer, ";", &entireTablePointer);
            IO_TrimLeft(fieldStr);
            IO_TrimRight(fieldStr);
            ERROR_Assert(p != NULL, "Can't read table name");
            string tableName(fieldStr);

            //now get the primary keys
            p = IO_GetDelimitedToken(fieldStr, entireTablePointer, ";", &entireTablePointer);
            IO_TrimLeft(fieldStr);
            IO_TrimRight(fieldStr);
            ERROR_Assert(p != NULL, "Can't read primary keys");

            char part[MAX_STRING_LENGTH] = "";
            char* pKeys = fieldStr;

            //now get each individual primary key
            while (IO_GetDelimitedToken(part, pKeys, ",", &pKeys) != NULL)
            {
                IO_TrimLeft(part);
                IO_TrimRight(part);
                interestDetail.primaryKeys.insert(part);
            }

            //now get each individual column
            while (IO_GetDelimitedToken(fieldStr, entireTablePointer, ";", &entireTablePointer) != NULL)
            {
                //for each column get the name then the type
                char* column = fieldStr;
                ColumnDetails col;
                p = IO_GetDelimitedToken(part, column, ",", &column);

                IO_TrimLeft(part);
                IO_TrimRight(part);
                ERROR_Assert(p != NULL, "Can't read col name");

                col.name = part;

                p = IO_GetDelimitedToken(part, column, ",", &column);

                IO_TrimLeft(part);
                IO_TrimRight(part);
                ERROR_Assert(p != NULL, "Can't read col type");
                col.typeOrValue = part;

                //store each column in the tableInterest
                interestDetail.columns.insert(col);
            }

            //Finally Register the table interest with this paritions, merging will be handled by the below function
            RegisterTableInterest(tableName, interestDetail);
        }
    }

    //update the check in record
    partitionCheckinCount++;

    //Now delete the message
    MESSAGE_Free(node, serializedTableDataMsg);

    //if all partitions checked in, send all the info to the db
    if (partitionCheckinCount == partition->getNumPartitions() - 1)
    {
        PublishFinishedTablesInterestsToDb();
    }
}

void Proc::DB::StatsDBController::PublishFinishedTablesInterestsToDb()
{
    //**Proc section PARTITION_InitilizeNodes is blocked by a db enable check.
    //HandleSerializedTableInterestsFromOtherPartition doesn't need that check since
    //SendSerializedTableInterestsToPZero is only called from PARTITION_InitilizeNodes

    //do a create table sql statement for each registered table interes
    for (std::map<std::string, StatsDBTableInterestDetail>::iterator it = dbTableInterests.begin();
        it != dbTableInterests.end(); it++)
    {
        std::string query = "CREATE TABLE IF NOT EXISTS ";
        query += it->first;
        query += "(";
#ifdef ADDON_DB
        //Add in rowid for all tables, we do this for all statsdb tables in qualnet
        if (partition->statsDb->engineType == UTIL::Database::dbMariaDB)
        {
            query += "RowId bigint auto_increment primary key";
        }
        else if (partition->statsDb->engineType == UTIL::Database::dbSqlite)
        {
            query += "RowId INTEGER PRIMARY KEY AUTOINCREMENT";
        }
#endif
        //loop over all the individual columns
        for (std::set<ColumnDetails>::iterator it2 = it->second.columns.begin();
            it2 != it->second.columns.end(); it2++)
        {
            query += ",";
            query += it2->name;
            query += " ";
            query += it2->typeOrValue;

            //You cannot have multiple primary key clauses within a single create table call
            //You can define a combined primary key, however we want RowId to autoincrement,
            //however you can only do that if its a singular primary key.  There custom primary
            //key functionality is disabled as of now.
            /*if (it->second.primaryKeys.find(it2->name) != it->second.primaryKeys.end())
            {
                query += " PRIMARY KEY";
            }*/
        }
        query += ")";
#ifdef ADDON_DB
        AddQueryToBufferStatsDb(partition->statsDb, query.c_str());
#endif
    }
}

void Proc::DB::StatsDBController::LogEvent(const std::string& tableName,
    const std::string& rowKey,
    std::set<ColumnDetails> newRowData,
    bool isReadyToSend)
{
    //first get the pendingTableRow object for the table
    if (pendingTableRowsPerTable.find(tableName) != pendingTableRowsPerTable.end())
    {
        //table interest already exists, get a pointer to the existing table, so
        //you don't have to keep querying the map
        pendingTableRows* existingTable = &pendingTableRowsPerTable[tableName];

        if (existingTable->find(rowKey) != existingTable->end())
        {
            //pending rowData already exists, get a pointer to the existing row, so
            //you don't have to keep querying the map
            set<ColumnDetails>* existingRow = &(*existingTable)[rowKey];

            //merge newRowData with existingRowData.
            for (std::set<ColumnDetails>::iterator it = newRowData.begin();
                it != newRowData.end(); it++)
            {
                //**only new column names will be inserted, so if the column has
                //already has data for it, it will not be replaced
                existingRow->insert(*it);
            }

            //if the data is complete, publish it
            if (isReadyToSend)
            {
                PublishEventRowToDb(tableName, rowKey);
            }
        }
        else
        {
            //This is the first data for this rowKey, if its complete skip inserting
            //to the map and publish the data now
            if (isReadyToSend)
            {
                PublishEventRowToDb(tableName, newRowData);
            }
            else
            {
                (*existingTable)[rowKey] = newRowData;
            }
        }
    }
    else
    {
        return ;
        //This shouldn't happen.  When a model registers a table interest, a pendingTableRows object is created for that
        //table name.
        ERROR_Assert(0, "Table has not been registered beforing trying to log an event");
    }

}

void Proc::DB::StatsDBController::PublishEventRowToDb(
                                          const std::string& tableName,
                                          std::set<ColumnDetails> newData)
{
#ifdef ADDON_DB
    //sanity check that you have input data is not empty before creating the sql statement
    if (partition->statsDb != NULL && newData.size() > 0)
    {
        std::string query = "INSERT INTO ";
        query += tableName;
        query += "(";

        //loop over all the column names
        for (std::set<ColumnDetails>::iterator it = newData.begin(); it != newData.end(); it++)
        {
            query += it->name;
            query += ",";
        }
        //chop the last comma in query
        query = query.substr(0, query.size()-1);
        query += ") VALUES(";

        //loop over all the column values
        for (set<ColumnDetails>::iterator it = newData.begin();
            it != newData.end(); it++)
        {
            query += "'";
            query += it->typeOrValue;
            query += "',";
        }
        //chop the last comma from values
        query = query.substr(0, query.size()-1);

        query += ")";

        //Pass off the complete query string to the db driver
        AddQueryToBufferStatsDb(partition->statsDb, query);
    }
#endif
}

void Proc::DB::StatsDBController::PublishEventRowToDb(
                                        const std::string& tableName,
                                        const std::string& rowKey)
{
#ifdef ADDON_DB
    //first do a sanity check to see that db driver exists and there is a pendTableRows object
    //set up for the table you are inserting to.
    if (partition->statsDb != NULL &&
        pendingTableRowsPerTable.find(tableName) != pendingTableRowsPerTable.end())
    {
        pendingTableRows* existingTable = &pendingTableRowsPerTable[tableName];

        //see if there is a row for the rowKey you are looking up
        if (existingTable->find(rowKey) != existingTable->end())
        {
            //pending rowData already exists, get a pointer to the existing row, so
            //you don't have to keep querying the map
            set<ColumnDetails>* existingRow = &(*existingTable)[rowKey];

            //another sanity check that you have data for the row before creating the sql statement
            if (existingRow->size() > 0)
            {
                std::string query = "INSERT INTO ";
                query += tableName;
                query += "(";

                //loop over all the column names
                for (std::set<ColumnDetails>::iterator it = existingRow->begin(); it != existingRow->end(); it++)
                {
                    query += it->name;
                    query += ",";
                }
                //chop the last comma in query
                query = query.substr(0, query.size()-1);
                query += ") VALUES(";

                //loop over all the column values
                for (set<ColumnDetails>::iterator it = existingRow->begin();
                    it != existingRow->end(); it++)
                {
                    query += "'";
                    query += it->typeOrValue;
                    query += "',";
                }
                //chop the last comma from values
                query = query.substr(0, query.size()-1);

                query += ")";

                //Pass off the complete query string to the db driver
                AddQueryToBufferStatsDb(partition->statsDb, query);
            }
            //Finally remove the rowKey from pendingTableRows
            existingTable->erase(rowKey);
        }
    }
#endif
}

void Proc::DB::StatsDBController::SetPartitionData(PartitionData* p)
{
    partition = p;
}


void Proc::Nexus::DataModel::DumpEvent(Node* node,
                                       int phyIndex,
                                       const std::string& tableName,
                                       const std::string& eventName,
                                       unsigned long long seq,
                                       double t,
                                       std::set<Proc::DB::ColumnDetails>& details)
{
#ifdef ADDON_DB
      std::string seq_str = to_fmt<unsigned long long>("%llu", seq);

      std::string t_str = to_fmt<double>("%lf", t);

      std::string uuid = eventName + "-" + t_str + "-" + seq_str;

      DB::ColumnDetails p_eventTimestamp(std::string("EventTimestamp"), t_str);

      DB::ColumnDetails p_eventSequence(std::string("EventSequence"), seq_str);

      DB::ColumnDetails p_eventName(std::string("EventName"), eventName);


      details.insert(p_eventTimestamp);

      details.insert(p_eventSequence);

      details.insert(p_eventName);


      node->partitionData->dbController->LogEvent(tableName, eventName, details, true);
#endif
}
