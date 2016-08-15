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
#ifndef __PROC_STAT_DB_CONTROLLER_H__
#define __PROC_STAT_DB_CONTROLLER_H__

#include <string>
#include <map>
#include <set>
#include "partition.h"
// Forward declarations
class Message;
struct Node;
struct PartitionData;



/**
 \brief Overall Proc Namespace

  This namespace is used throughout the Proc module to separate its C++ symbols from
  other symbols within the JNE executable.

  The namespaces contained within it are:

  - App  Application code for the Proc models
  - Phy  Physical layer modeling code for the Proc models
  - DB   Stats DB controller code for populating db

  It contains general classes shared among the PHY, MAC, and Application processes.
*/
namespace Proc 
{

namespace DB
{

#define SMALL_TABLE_STRING_SIZE 512   ///< Size of serialized representation of small amount of tables 
#define MEDIUM_TABLE_STRING_SIZE 1024  ///< Size of serialized representation of medium amount of tables 
#define LARGE_TABLE_STRING_SIZE 2048  ///< Size of serialized representation of large amount of tables 
#define EXTRA_LARGE_TABLE_STRING_SIZE 4096  ///< Size of serialized representation of extra large amount of tables 
#define EXTRA_EXTRA_LARGE_TABLE_STRING_SIZE 8192  ///< Size of serialized representation of very large amount of tables 

/**
\brief Qualnet Message Info for serialized table interests sent between partitions

\note Uses a templated size - use one of the 5 defined estimates, ex: SMALL_TABLE_STRING_SIZE
*/
template <std::size_t sz>
struct INFO_SerializedTableInterest
{
    int stringSize;
    char stringBuffer[sz];
    static int len() { return sizeof(INFO_SerializedTableInterest<sz>); }
};

/**
\brief Table Column Specific information 

Defines a column name and type
*/
struct ColumnDetails
{
    std::string name;
    std::string typeOrValue;

    ColumnDetails() { name = ""; typeOrValue = ""; }
    ColumnDetails(std::string _name, std::string _typeOrValue) { name = _name; typeOrValue = _typeOrValue; }

    bool operator<(const ColumnDetails &b) const
    {
        return name < b.name; 
    }
};

/**
\brief Table schema interest for a table

Simply combines primaryKey string set with a set of ColumnDetails
*/
struct StatsDBTableInterestDetail
{
    std::set<std::string> primaryKeys;
    std::set<ColumnDetails> columns;
};

/**
\brief Stats Database Controller class

 This class is used to synchronize create table sql statement across partitions,
 store & combine partial event data before inserting to the database.
*/
class StatsDBController
{
    private:
        PartitionData* partition;  ///< Parent partition pointer used to access the db driver
        std::map<std::string, StatsDBTableInterestDetail> dbTableInterests;  ///< Registered table interests, ie. table schema, where map key is the table name
        
        typedef std::map <std::string, std::set<ColumnDetails> >  pendingTableRows;  ///< typedef for pendings rows of data for an individual table, where map key is the row key
        std::map<std::string, pendingTableRows> pendingTableRowsPerTable;  ///< Pending rows of data per table to insert into the db, where map key is the table name
        
        int partitionCheckinCount;  ///< simple counter to keep track of number of secondary parititons that have provided their table interests

        /*!
         \brief Compose an sql insert statement and pass it to the db driver 

         \param tableName the db table data is being inserted into
         \param rowKey the key of the row you want to write to the db from pendingTableRowsPerTable
         */        
        void PublishEventRowToDb(const std::string& tableName,
                                 const std::string& rowKey);

        /*!
         \brief Compose an sql insert statement and pass it to the db driver using 

         \param tableName the db table data is being inserted into
         \param newData column details to be used for the insert query

         \note This version should only be called by LogEvent when data is ready to go and is not already in the pendingTableRow object
         */
        void PublishEventRowToDb(const std::string& tableName,
                                 std::set<ColumnDetails> newData);

    public:

        /*!
        \brief Construct a db controller
        */
        StatsDBController();      

        /*!
        \brief Set the parent partition pointer
        */
        void SetPartitionData(PartitionData* partition);

        /*!
        \brief Serialize registered table interests and send the info to partition zero

        /note This function should only be called by end of PARTITION_InitializeNodes (post initalize step) by non-zero partitions
        */        
        void SendSerializedTableInterestsToPZero();        

        /*!
        \brief Handle serialized table interest message from non-zero paritions by deserializing and merging  

        \param node the qualnet node the message is being handled on
        \param serializedTableDataMsg qualnet message containing the serialized table interest

        /note This function should only be called by NODE_ProcessEvent on parition zero 
        */
        void HandleSerializedTableInterestsFromOtherPartition(Node* node,
            Message* serializedTableDataMsg);
        
        /*!
        \brief Compose sql create table statments based on registered table interests

        /note This function should only be called by end of PARTITION_InitializeNodes when there is only one parition or by partition zero when the partitionCheckinCount requiement is met
        */
        void PublishFinishedTablesInterestsToDb();

        /*!
        \brief Main public api for models to register specific table interest

        \param tableName name of db table
        \param interest primary keys and columns that will used to define the table schema
        */
        void RegisterTableInterest(const std::string& tableName,
                                   StatsDBTableInterestDetail interest);
        
        /*!
        \brief Main public api for models to log partitial or complete event data

        \param tableName name of db table
        \param rowKey unique identifer for the row of data
        \param newRowData new event data for the specific row - column name and string formatted data values
        \param isReadyToSend whether the row is complete after logging this info
        */
        void LogEvent(const std::string& tableName,
                      const std::string& rowKey,
                      std::set<ColumnDetails> newRowData,
                      bool isReadyToSend);

#if 0
        void RegisterDummyTables();
        void LogDummyEvent();
#endif 
};

} //DB

} //Proc

#endif /* __PROC_STAT_DB_CONTROLLER_H__ */
