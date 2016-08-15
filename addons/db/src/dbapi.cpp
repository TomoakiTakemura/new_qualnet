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

#include "api.h"
#include "partition.h"
#include "WallClock.h"
#include "external_util.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include "mapping.h"
#include "dbapi.h"
#include "db.h"
#include "stats_global.h"
#include "db_statsapi_bridge.h"

#include "db-core.h"
#include "db-mariadb.h"
#include "db-sqlite3.h"

#include "mysqld_error.h"
#include "fileio.h"
#include "network_ip.h"
#include <iomanip>

#ifdef SOCKET_INTERFACE
#include "socket-interface.h"
#endif

#include <boost/lockfree/queue.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#if defined(STATS_DEBUG)
# undef STATS_DEBUG
#endif /* STATS_DEBUG */

#define STATS_DEBUG (false)

// this class hides the formalism of the boost thread
// class.
struct aThread
{
    struct callable
    {
      void operator()(aThread* t)
      {
          t->sleep(0);      // wait until wake is called.
          while (!t->m_exitFlag)
          {
              t->sleep(1000);  // wait 1 second or until woken
              t->run();        // perform any available work
          }
          t->run();
          if (STATS_DEBUG) {
              std::cout << "database thread exit\n";}
      }
    };
    boost::thread m_bThread;
    bool m_exitFlag;

    virtual void run() = 0;

    aThread() : m_exitFlag(false)
    {
        callable c;
        m_bThread = boost::thread(c, this);
    }

    void sleep(int ms)
    {
        if (ms == 0) ms = 1000000;
        try {
            boost::this_thread::sleep(boost::posix_time::milliseconds(ms));
        }   catch (boost::thread_interrupted) { }
    }

   void wait()
    {
        wake();
        if (STATS_DEBUG) {
            std::cout << "tell database thread to exit\n";
        }
        m_exitFlag = true;
        wake();
        m_bThread.join();
    }

    void wake()
    {
        m_bThread.interrupt();
    }
};


class DatabaseThread : public aThread
{
    UTIL::Database::DatabaseDriver* m_driver;
    boost::lockfree::queue <std::string* >* m_queue;
    boost::atomic<int> m_queueCount;
    //int m_queueCount;
#ifdef USE_MUTEX
    boost::condition_variable m_cond;
    boost::mutex m_mut;
#endif


  int m_nRows;
  int m_nSleep;
  int m_maxSleep;
  int m_maxQueueCount;
public:

  DatabaseThread() :
    m_driver(NULL),
    m_queue(NULL),
    m_queueCount(0),
    m_maxQueueCount(0),
    m_nRows(0),
    m_nSleep(0),
    m_maxSleep(0)
  { }

    void run()
    {
        int ops = 0;
        const std::string* sql;
        if (!m_queue || !m_queue->pop(sql)) return;
        m_driver->startTransaction();
        do
        {
#ifdef USE_MUTEX
            m_cond.notify_one();
#endif
            //m_queueCount.fetch_sub(1, boost::memory_order_relaxed);
            m_queueCount--;
            m_driver->exec(*sql);
            delete(sql);
            m_nRows++;
        }   while (m_queue->pop(sql));
        m_driver->commit();
    }

    void close()
    {
        if (m_driver != NULL)
        {
            m_driver->close();
            m_driver = NULL;
        }
        if (m_nRows > 0) {
            std::cout << "Database thread stats. Rows:" << m_nRows
                << " Waits:" << m_nSleep << " Max Wait Time (mS):" << m_maxSleep
                << " Max Queue:" << m_maxQueueCount
                << std::endl;
        }
    }

    void open(StatsDb* db)
    {
        if (db->engineType == UTIL::Database::dbMariaDB)
        {
            m_driver = new UTIL::Database::MariaDBNativeDriver(
                *(UTIL::Database::MariaDBNativeDriver*)db->driver);
        }
        else if (db->engineType == UTIL::Database::dbSqlite)
        {
            m_driver = new UTIL::Database::Sqlite3Driver(
                *(UTIL::Database::Sqlite3Driver*)db->driver);
        }
        else
        {
            ERROR_AssertArgs(false, "unsupported engine type:%d", db->engineType);
        }
        m_driver->open(true);
        m_queue = new boost::lockfree::queue<std::string* >(db->maxQueryBuffer);
    }

    void push_back(StatsDb* db, const char* sql)
    {
        push_back(db, new std::string(sql));
    }

    void push_back(StatsDb* db, const std::string &sql)
    {
        push_back(db, new std::string(sql));
    }

    void push_back(StatsDb* db, std::string* sql)
    {
        if (m_driver == NULL)
        {
          open(db);
        }
        int countSleep = 0;
        wake();

#ifdef USE_MUTEX
        boost::unique_lock<boost::mutex> lock(m_mut);
        while (!m_queue->bounded_push(sql)) {
            if (!countSleep) {
              m_nSleep++;
            }
            countSleep++;
            m_cond.wait(lock);
        }
#else
        while (!m_queue->bounded_push(sql))
        {
            boost::this_thread::sleep(boost::posix_time::milliseconds(1));
            if (!countSleep) {
              m_nSleep++;
            }
            countSleep++;
        }
#endif
        if (countSleep > m_maxSleep) m_maxSleep = countSleep;
        //int count = m_queueCount.fetch_add(1, boost::memory_order_relaxed);
        int count = m_queueCount++;
        count++;
        if (count > m_maxQueueCount) m_maxQueueCount = count;
    }

    void finalize() {
        wait();
        close();
    }
};

DatabaseThread dataBase;

STATSDB_Table::STATSDB_Table(const char* tableName, StatsDb* db)
{
    strcpy(tableName_, tableName);
    db_ = db;
}

void STATSDB_Table::Create()
{
    if (db_->partition->partitionId > 0) return;
    char sql[1000];
    size_t offset = sprintf(sql, "CREATE TABLE IF NOT EXISTS %s(%s)", tableName_, columns_.get());
    if (db_->storageEngine[0]) {
        offset += sprintf(sql + offset, " ENGINE %s", db_->storageEngine);
    }
    dataBase.push_back(db_, sql);
}

void STATSDB_Table::Insert()
{
    if (!Use()) return;
    PreInsert();
    char sql[1000];
    sprintf(sql, "INSERT INTO %s(%s) VALUES(%s)", tableName_, columns_.get(), values_.get());
    dataBase.push_back(db_, sql);
}

// Due to the history of the code there are various data types used
// for the columns.  These get mapped to a minimal subset here.
// Since the StatsDb pointer is available here the mapping can depend
// upon the underlying data representation (e.g. use mysql's text field).
const char* STATSDB_Table::FixColType(const char* type) {
    if (TypeIs(type, "float")) return "real";
    if (TypeIs(type, "double")) return "real";
    if (TypeIs(type, "rowid")) {
        if (db_->engineType == UTIL::Database::dbMariaDB) return "bigint auto_increment primary key";
        if (db_->engineType == UTIL::Database::dbSqlite) return "INTEGER PRIMARY KEY AUTOINCREMENT";
    }
    return type;
}

bool STATSDB_Table::TypeIs(const char* type, const char* compare) {
    // might add case blind compare here later.
    return (strcmp(type, compare) == 0);
}

bool STATSDB_Table::InConfig(NodeInput* nodeInput, const char* configName, bool dflt) {
    BOOL wasFound;
    char buf[MAX_STRING_LENGTH];

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        configName, &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0) return true;
        if (strcmp(buf, "NO") == 0) return false;
        // We have invalid values.
        ERROR_ReportWarningArgs(
            "Invalid Value (%s) for "
            "%s parameter,"
            "using \"%s\"\n", buf, configName, dflt ? "YES" : "NO");
        return dflt;
    }
    return dflt;
}

std::string STATSDB_Table::ConfigString(NodeInput* nodeInput, const char* configName, const char* dflt) {
    BOOL wasFound;
    char buf[MAX_STRING_LENGTH];

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        configName, &wasFound, buf);
    if (wasFound) return std::string(buf);
    return std::string(dflt);
}


bool STATSDB_Table::ConfigNumbers(NodeInput* nodeInput, const char* configName, std::set<int>& numbers, const char* dflt, std::map <std::string, std::string>* keywords) {
    BOOL wasFound;
    char buf[MAX_STRING_LENGTH];

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput, configName, &wasFound, buf);
    if (!wasFound) {
        if (dflt == NULL) return false;
        strcpy(buf, dflt);
    }

    std::string s(buf);
    for (size_t f = s.find(','); f != std::string::npos; f = s.find(',')) {
        s.replace(f, 1, 1, ' ');
    }
    for (size_t i = 0; i < s.length(); ++i) {
        s.replace(i, 1, 1, toupper(s[i]));
    }
    std::istringstream stream(s);
    for (;;) {
        std::string item;
        stream >> item;
        if (!stream) break;
        if (keywords != NULL) {
            std::map<std::string, std::string>::iterator it;
            it = keywords->find(item);
            if (it != keywords->end()) item = it->second;
        }
        int n;
        if (!sscanf(item.c_str(), "%d", &n)) {
            ERROR_ReportWarningArgs(
                "%s value:%s ignoring:%s, can only contain comma separated list of numbers",
                configName, buf, item.c_str());
            numbers.clear();
            return false;
        }
        numbers.insert(n);
    }
    return true;
}

void STATSDB_Table::SetTimestamp() {
    SetTimestamp(db_->partition->getGlobalTime());
}
void STATSDB_Table::SetTimestamp(clocktype ts) {
    SetColumn("Timestamp", (double)ts / SECOND);
}

void FlushQueryBufferStatsDb(StatsDb* /*db*/)
{
    dataBase.wake();
}

void AddQueryToBufferStatsDb(StatsDb* db, const std::string &queryStr)
{
    if (STATS_DEBUG)
    {
        printf("Executing query %s\n", queryStr.c_str());
    }
    dataBase.push_back(db, queryStr);
}

void STATSDB_Close()
{
    if (STATS_DEBUG) {
        cout << "STATSDB_Close()" << endl;}
    dataBase.finalize();
}

void STATSDB_Finalize(PartitionData* partition)
{
     StatsDb* db = partition->statsDb;

    if (db == NULL)
    {
        return;
    }
    if (db->appEventsString != "")
    {
        AddInsertQueryToBuffer(db, db->appEventsString);
    }

    if (db->networkEventsBytesUsed)
    {
       AddInsertQueryToBuffer(db, db->networkEventsString);
    }

    if (db->statsAggregateTable->endSimulation)
    {
        HandleStatsDBAppAggregateInsertion(partition->firstNode);
        HandleStatsDBTransAggregateInsertion(partition->firstNode);
        HandleStatsDBNetworkAggregateInsertion(partition->firstNode);
        HandleStatsDBMacAggregateInsertion(partition->firstNode);
        HandleStatsDBPhyAggregateInsertion(partition->firstNode);
        HandleStatsDBQueueAggregateInsertion(partition->firstNode);
        // protocol specific
        if (db->statsOspfTable != NULL)
        {
            if (db->statsOspfTable->createOspfAggregateTable)
            {
                HandleStatsDBOspfAggregateTableInsertion(
                    partition->firstNode);
            }
        }
        HandleStatsDBAppAggregateUpdate(partition->firstNode);
    }

    if (db->statsOspfTable != NULL)
    {
        if (db->statsOspfTable->createOspfNeighborStateTable)
        {
            HandleStatsDBOspfNeighborStateTableInsertion(
                partition->firstNode);
        }
        if (db->statsOspfTable->createOspfInterfaceStateTable)
        {
            HandleStatsDBOspfInterfaceStateTableInsertion(
                partition->firstNode);
        }
        if (db->statsOspfTable->createOspfNetworkLsaTable)
        {
            HandleStatsDBOspfNetworkLsaTableInsertion(
                partition->firstNode);
        }
        if (db->statsOspfTable->createOspfRouterLsaTable)
        {
            HandleStatsDBOspfRouterLsaTableInsertion(
                partition->firstNode);
        }
        if (db->statsOspfTable->createOspfExternalLsaTable)
        {
            HandleStatsDBOspfExternalLsaTableInsertion(
                partition->firstNode);
        }
    }

    if (db->statsSummaryTable->endSimulation)
    {
        HandleStatsDBAppSummaryInsertion(partition->firstNode);
        HandleStatsDBMulticastAppSummaryInsertion(partition->firstNode);

        //multicast network summary handling. Check if the Table exists.
        if (db->statsSummaryTable->createMulticastNetSummaryTable)
        {
            HandleStatsDBIpMulticastNetSummaryTableInsertion(
                                                       partition->firstNode);
            HandleStatsDBPimMulticastNetSummaryTableInsertion(
                                                       partition->firstNode);
            HandleStatsDBMospfMulticastNetSummaryTableInsertion(
                                                       partition->firstNode);
        }
        //multicast protocol specific calling start
        HandleStatsDBPimSmSummaryTableInsertion(partition->firstNode);
        HandleStatsDBPimDmSummaryTableInsertion(partition->firstNode);
        HandleStatsDBMospfSummaryTableInsertion(partition->firstNode);
        HandleStatsDBIgmpSummaryTableInsertion(partition->firstNode);
        //multicast protocol specific calling end
        HandleStatsDBTransSummaryInsertion(partition->firstNode);
        HandleStatsDBNetworkSummaryInsertion(partition->firstNode);
        HandleStatsDBMacSummaryInsertion(partition->firstNode);
        HandleStatsDBPhySummaryInsertion(partition->firstNode);
        HandleStatsDBPhySummaryInsertionForMacProtocols(partition->firstNode);
        HandleStatsDBQueueSummaryInsertion(partition->firstNode);
        if (db->statsOspfTable != NULL)
        {
           if (db->statsOspfTable->createOspfSummaryTable)
            {
                HandleStatsDBOspfSummaryTableInsertion(
                    partition->firstNode);
            }
            if (db->statsOspfTable->createOspfSummaryLsaTable)
            {
                HandleStatsDBOspfSummaryLsaTableInsertion(
                    partition->firstNode);
            }
        }
    }
    if (db->statsStatusTable->endSimulation)
    {
        // nodeStatus table
        Node * nextNode = partition->firstNode;
        while (nextNode)
        {
            StatsDBNodeStatus nodeStatus(nextNode, FALSE);
            // Add this node's status information to the database
            STATSDB_HandleNodeStatusTableInsert(nextNode, nodeStatus);

            STATSDB_HandleInterfaceStatusTableInsert(nextNode, FALSE);

            nextNode = nextNode->nextNodeData;
        }

        // the following functions will handle all nodes on one partition
        STATSDB_HandleMulticastStatusTableInsert(partition->firstNode);
        //multicast protocol specific calling start
        HandleStatsDBPimSmStatusTableInsertion(partition->firstNode);
        //multicast protocol specific calling end
        HandleStatsDBQueueStatusInsertion(partition->firstNode);

    }

    if (db->statsConnTable->endSimulation)
    {
        HandleStatsDBAppConnInsertion(partition->firstNode);
        HandleStatsDBTransConnInsertion(partition->firstNode);
        HandleStatsDBNetworkConnInsertion(partition->firstNode);
        HandleStatsDBMacConnInsertion(partition->firstNode);
        StatsDBHandleMulticastConnInsertion(partition->firstNode, NULL);
        HandleStatsDBPhyConnInsertion(partition);
        //protocol specific
    }
}

void InitializePartitionStatsDb(StatsDb* statsDb)
{

    statsDb->createDbFile = FALSE;
    statsDb->driver = NULL;

    statsDb->maxQueryBuffer = STATSDB_MAX_BUFFER_QUERY;

    statsDb->statsTable = (StatsDBTable*) MEM_malloc(sizeof(StatsDBTable));
    statsDb->statsTable->createDescriptionTable = FALSE;
    statsDb->statsTable->createStatusTable = FALSE;
    statsDb->statsTable->createAggregateTable = FALSE;
    statsDb->statsTable->createSummaryTable = FALSE;
    statsDb->statsTable->createEventsTable = FALSE;
    statsDb->statsTable->createConnectivityTable = FALSE;
    statsDb->statsTable->createUrbanPropTable = FALSE;

    statsDb->statsDescTable = (StatsDBDescTable*) MEM_malloc(sizeof(StatsDBDescTable));
    statsDb->statsDescTable->createNodeDescTable = FALSE;
    statsDb->statsDescTable->createQueueDescTable = FALSE;
    statsDb->statsDescTable->createSchedulerDescTable = FALSE;
    statsDb->statsDescTable->createSessionDescTable = FALSE;
    statsDb->statsDescTable->createConnectionDescTable = FALSE;
    statsDb->statsDescTable->createInterfaceDescTable = FALSE;
    statsDb->statsDescTable->createPhyDescTable = FALSE;

    // constructor in data structure
    statsDb->statsInterfaceDesc = new StatsDBInterfaceDescContent;

    statsDb->statsQueueDesc = (StatsDBQueueDescContent*) MEM_malloc(sizeof(StatsDBQueueDescContent));
    statsDb->statsQueueDesc->isDiscipline = FALSE;
    statsDb->statsQueueDesc->isSize = FALSE;
    statsDb->statsQueueDesc->isPriority = FALSE;
    statsDb->statsSchedulerAlgo = new StatsDBSchedulerDescContent;

    // constructor in data structure
    statsDb->statsSessionDesc = new StatsDBSessionDescContent;
    // constructor in data structure
    statsDb->statsConnectionDesc = new StatsDBConnectionDescContent;

    statsDb->statsStatusTable = new StatsDBStatusTable ;
    statsDb->statsStatusTable->createNodeStatusTable = FALSE;
    statsDb->statsStatusTable->createInterfaceStatusTable = FALSE;
    statsDb->statsStatusTable->createQueueStatusTable = FALSE;
    statsDb->statsStatusTable->createMulticastStatusTable = FALSE;
    statsDb->statsStatusTable->createMalsrStatusTable = FALSE;
    statsDb->statsStatusTable->statusInterval = STATSDB_DEFAULT_STATUS_INTERVAL;
    statsDb->statsStatusTable->endSimulation = TRUE;

    statsDb->statsNodeStatus = (StatsDBNodeStatusContent*) MEM_malloc(sizeof(StatsDBNodeStatusContent));
    statsDb->statsNodeStatus->isActiveState = FALSE;
    statsDb->statsNodeStatus->isDamageState = FALSE;
    statsDb->statsNodeStatus->isPosition = FALSE;
    statsDb->statsNodeStatus->isVelocity = FALSE;
    statsDb->statsNodeStatus->isGateway = FALSE;

    statsDb->statsAggregateTable = (StatsDBAggregateTable*) MEM_malloc(sizeof(StatsDBAggregateTable));
    memset(statsDb->statsAggregateTable, 0, sizeof(StatsDBAggregateTable));
    statsDb->statsAggregateTable->createAppAggregateTable = FALSE;
    statsDb->statsAggregateTable->createTransAggregateTable = FALSE;
    statsDb->statsAggregateTable->createNetworkAggregateTable = FALSE;
    statsDb->statsAggregateTable->createMacAggregateTable = FALSE;
    statsDb->statsAggregateTable->createPhyAggregateTable = FALSE;
    statsDb->statsAggregateTable->aggregateInterval = STATSDB_DEFAULT_AGGREGATE_INTERVAL;
    statsDb->statsAggregateTable->endSimulation = TRUE;

    statsDb->statsAppAggregate = (StatsDBAppAggregateContent*) MEM_malloc(sizeof(StatsDBAppAggregateContent));
    statsDb->statsAppAggregate->isUnicastDelay = FALSE;
    statsDb->statsAppAggregate->isUnicastJitter = FALSE;
    statsDb->statsAppAggregate->isUnicastHopCount = FALSE;
    statsDb->statsAppAggregate->isMulticastDelay = FALSE;
    statsDb->statsAppAggregate->isMulticastJitter = FALSE;
    statsDb->statsAppAggregate->isMulticastHopCount = FALSE;
    statsDb->statsAppAggregate->isAvgDelay = FALSE;
    statsDb->statsAppAggregate->isIntervalBasedAvgDelay = FALSE;
    statsDb->statsAppAggregate->isAvgJitter = FALSE;
    statsDb->statsAppAggregate->isAvgThroughput = FALSE;
    statsDb->statsAppAggregate->isAvgOfferload = FALSE;

    statsDb->statsTransAggregate = new StatsDBTransportAggregateContent;

    statsDb->statsNetAggregate = new StatsDBNetworkAggregateContent;
    //statsDb->statsNetAggregate->isDelay = FALSE;
    //statsDb->statsNetAggregate->isJitter = FALSE;
    //statsDb->statsNetAggregate->isIpOutNoRoutes = FALSE;

    statsDb->statsMacAggregate = (StatsDBMacAggregateContent*) MEM_malloc(sizeof(StatsDBMacAggregateContent));
    Int32 i;
    for (i = 0; i < STAT_NUM_ADDRESS_TYPES; i++)
    {
        statsDb->statsMacAggregate->addrTypes[i].isAvgQueuingDelay = FALSE;
        statsDb->statsMacAggregate->addrTypes[i].isAvgMediumAccessDelay = FALSE;
        statsDb->statsMacAggregate->addrTypes[i].isAvgMediumDelay = FALSE;
        statsDb->statsMacAggregate->addrTypes[i].isAvgJitter = FALSE;
    }
    statsDb->statsPhyAggregate = (StatsDBPhyAggregateContent*) MEM_malloc(sizeof(StatsDBPhyAggregateContent));
    statsDb->statsPhyAggregate->isAvgPathLoss = FALSE;
    statsDb->statsPhyAggregate->isAvgSignalPower = FALSE;
    statsDb->statsPhyAggregate->isAvgDelay = FALSE;

    statsDb->statsSummaryTable = (StatsDBSummaryTable*) MEM_malloc(sizeof(StatsDBSummaryTable));
    memset(statsDb->statsSummaryTable, 0, sizeof(StatsDBSummaryTable));
    statsDb->statsSummaryTable->createAppSummaryTable = FALSE;
    statsDb->statsSummaryTable->createMulticastAppSummaryTable = FALSE;
    statsDb->statsSummaryTable->createMulticastNetSummaryTable = FALSE;
    statsDb->statsSummaryTable->createTransSummaryTable = FALSE;
    statsDb->statsSummaryTable->createNetworkSummaryTable = FALSE;
    statsDb->statsSummaryTable->createMacSummaryTable = FALSE;
    statsDb->statsSummaryTable->createPhySummaryTable = FALSE;
    statsDb->statsSummaryTable->summaryInterval = STATSDB_DEFAULT_SUMMARY_INTERVAL;
    statsDb->statsSummaryTable->endSimulation = TRUE;

    statsDb->statsAppSummary = (StatsDBAppSummaryContent*) MEM_malloc(sizeof(StatsDBAppSummaryContent));
    statsDb->statsAppSummary->isDelay = FALSE;
    statsDb->statsAppSummary->isJitter = FALSE;
    statsDb->statsAppSummary->isHopCount = FALSE;
    statsDb->statsMulticastAppSummary = (StatsDBMulticastAppSummaryContent*) MEM_malloc(sizeof(StatsDBMulticastAppSummaryContent));
    statsDb->statsMulticastAppSummary->isDelay = FALSE;
    statsDb->statsMulticastAppSummary->isJitter = FALSE;
    statsDb->statsMulticastAppSummary->isHopCount = FALSE;

    statsDb->statsNetSummary = (StatsDBNetworkSummaryContent*) MEM_malloc(sizeof(StatsDBNetworkSummaryContent));
    statsDb->statsNetSummary->isDataDelay = FALSE;
    statsDb->statsNetSummary->isControlDelay = FALSE;
    statsDb->statsNetSummary->isDataJitter = FALSE;
    statsDb->statsNetSummary->isControlJitter = FALSE;

    statsDb->statsTransSummary = (StatsDBTransSummaryContent*) MEM_malloc(sizeof(StatsDBTransSummaryContent));
    for (i = 0; i < STAT_NUM_ADDRESS_TYPES; i++)
    {
        statsDb->statsTransSummary->addrTypes[i].isDelay = FALSE;
        statsDb->statsTransSummary->addrTypes[i].isJitter = FALSE;
    }

    statsDb->statsMacSummary = (StatsDBMacSummaryContent*) MEM_malloc(sizeof(StatsDBMacSummaryContent));
    for (i = 0; i < STAT_NUM_ADDRESS_TYPES; i++)
    {
        statsDb->statsMacSummary->addrTypes[i].isAvgQueuingDelay = FALSE;
        statsDb->statsMacSummary->addrTypes[i].isAvgMediumAccessDelay = FALSE;
        statsDb->statsMacSummary->addrTypes[i].isAvgMediumDelay = FALSE;
        statsDb->statsMacSummary->addrTypes[i].isAvgJitter = FALSE;
    }
    statsDb->statsPhySummary = (StatsDBPhySummaryContent*) MEM_malloc(sizeof(StatsDBPhySummaryContent));
    statsDb->statsPhySummary->isAvgDelay = FALSE;
    statsDb->statsPhySummary->isAvgPathLoss = FALSE;
    statsDb->statsPhySummary->isAvgSignalPower = FALSE;

    statsDb->statsEventsTable = (StatsDBEventsTable*) MEM_malloc(sizeof(StatsDBEventsTable));
    memset(statsDb->statsEventsTable, 0, sizeof(StatsDBEventsTable));
    statsDb->statsEventsTable->createAppEventsTable = FALSE;
    statsDb->statsEventsTable->createTransEventsTable = FALSE;
    statsDb->statsEventsTable->createNetworkEventsTable = FALSE;
    statsDb->statsEventsTable->createMacEventsTable = FALSE;
    statsDb->statsMacEvents = new StatsDBMacEventContent ;
    statsDb->statsEventsTable->createPhyEventsTable = FALSE;
    statsDb->statsEventsTable->createQueueEventsTable = FALSE;
    statsDb->statsEventsTable->createExternalEventsTable = FALSE;

    statsDb->statsConnTable = new StatsDBConnTable;
    statsDb->statsConnTable->createAppConnTable = FALSE;
    statsDb->statsConnTable->createTransConnTable = FALSE;
    statsDb->statsConnTable->createNetworkConnTable = FALSE;
    statsDb->statsConnTable->createMacConnTable = FALSE;
    statsDb->statsConnTable->createPhyConnTable = FALSE;
    statsDb->statsConnTable->createMulticastConnTable = FALSE;
    statsDb->statsConnTable->v_AppConnParam = NULL;
    statsDb->statsConnTable->v_TransConnParam = NULL;

    statsDb->statsAppEvents = new StatsDBAppEventContent;

    statsDb->statsNetEvents = new StatsDBNetworkEventContent;
    statsDb->statsNetEvents->isMsgSeqNum = FALSE;
    statsDb->statsNetEvents->isFragId = FALSE;
    statsDb->statsNetEvents->isMacProtocol = FALSE;
    statsDb->statsNetEvents->isControlSize = FALSE;
    statsDb->statsNetEvents->isPriority = FALSE;
    statsDb->statsNetEvents->isProtocolType = FALSE;
    statsDb->statsNetEvents->isPktFailureType = FALSE;
    statsDb->statsNetEvents->isPktType = FALSE;
    statsDb->statsNetEvents->isInterfaceIndex = FALSE;
    statsDb->statsNetEvents->isHopCount = FALSE;
    statsDb->statsNetEvents->networkStatsDBControl = FALSE;
    statsDb->statsNetEvents->networkStatsDBIncoming = FALSE;
    statsDb->statsNetEvents->networkStatsDBOutgoing = FALSE;
    statsDb->statsNetEvents->bufferSizeInBytes = DB_LONG_BUFFER_LENGTH;
    statsDb->statsNetEvents->multipleValues = FALSE;

    statsDb->statsPhyEvents = (StatsDBPhyEventContent*) MEM_malloc(sizeof(StatsDBPhyEventContent));
    statsDb->statsPhyEvents->isChannelIndex = FALSE;
    statsDb->statsPhyEvents->isControlSize = FALSE;
    statsDb->statsPhyEvents->isInterference = FALSE;
    statsDb->statsPhyEvents->isMessageFailureType = FALSE;
    statsDb->statsPhyEvents->isPathLoss = FALSE;
    statsDb->statsPhyEvents->isSignalPower = FALSE;

    statsDb->statsExternalEvents = new StatsDBExternalEventContent();

    statsDb->statsTransEvents = new StatsDBTransEventContent();

    statsDb->statsNetConn = (StatsDBNetworkConnContent*) MEM_malloc(sizeof(StatsDBNetworkConnContent));
    statsDb->statsNetConn->isDstMaskAddr = FALSE;
    statsDb->statsNetConn->isOutgoingInterfaceIndex = FALSE;
    statsDb->statsNetConn->isNextHopAddr = FALSE;
    statsDb->statsNetConn->isRoutingProtocol = FALSE;
    statsDb->statsNetConn->isAdminDistance = FALSE;

    statsDb->statsMacConn = new StatsDBMacConnContent ;

    statsDb->statsPhyConn = (StatsDBPhyConnContent*) MEM_malloc(sizeof(StatsDBPhyConnContent));
    statsDb->statsPhyConn->isPhyIndex = FALSE;
    statsDb->statsPhyConn->isChannelIndex = FALSE;
    statsDb->statsOspfTable = NULL;
    statsDb->statsPimTable = NULL;
    statsDb->statsIgmpTable = NULL;
}


////////////////////////////////////////////////////////////////////////
// Description tables definitions
////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------//
// NAME     : StatsDBQueueDesc::StatsDBQueueDesc
// PURPOSE  : StatsDBQueueDesc constructor. Initialize QUEUE_Description
//            table values
// PARAMETERS :
// + nodeId : ID of node on which the queue resides
// + index : index of interface on which the queue resides
// + queueIndex : Index of the queue, which is merely its position in
//                the list of queues it belongs to
//
// RETURN   : None.
//--------------------------------------------------------------------//
StatsDBQueueDesc::StatsDBQueueDesc(Int32 nodeId,
                                   Int32 index,
                                   Int32 queueIndex,
                                   const char* queueType) :
    m_NodeId(nodeId),
    m_InterfaceIndex(index),
    m_QueueIndex(queueIndex),
    m_QueueType(queueType),
    m_QueueDiscipline(""),
    m_QueueSize(0),
    m_QueuePriority(-1) // no queue priority is -1
    { }


//--------------------------------------------------------------------//
// NAME     : StatsDBSchedulerDesc::StatsDBSchedulerDesc
// PURPOSE  : StatsDBSchedulerDesc constructor. Initialize SCHEDULER_Description
//            table values
// PARAMETERS :
// + nodeId : ID of node on which the scheduler resides
// + index : index of interface on which the scheduler resides
// + type : type of queue (e.g. network input, MAC output)
// + algorithm : name of the scheduling algorithm (e.g. "Strict Priority")
//
// RETURN   : None.
//--------------------------------------------------------------------//
StatsDBSchedulerDesc::StatsDBSchedulerDesc(Int32 nodeId,
                                           Int32 index,
                                           const char* type,
                                           const char* algorithm)
{
    m_NodeId = nodeId;
    m_InterfaceIndex = index;
    m_SchedulerType = type;
    m_SchedulingAlgorithm = algorithm;
}

StatsDBSessionDesc::StatsDBSessionDesc(Int32 sessionId,
                                       Int32 senderId,
                                       Int32 receiverId) :
    m_SessionId(sessionId),
    m_SenderId(senderId),
    m_ReceiverId(receiverId),
    m_AppType(""),
    m_SenderAddr(""),
    m_ReceiverAddr(""),
    m_SenderPort(0),
    m_RecvPort(0),
    m_TransportProtocol("")
    { }




StatsDBConnectionDesc::StatsDBConnectionDesc(double timeValue,
                                             Int32 senderId,
                                             Int32 receiverId) :
    m_timeValue(timeValue),
    m_SenderId(senderId),
    m_ReceiverId(receiverId),
    m_SenderAddr(""),
    m_ReceiverAddr(""),
    m_SenderPort(0),
    m_RecvPort(0),
    m_ConnectionType(""),
    m_NetworkProtocol("")
    { }

//--------------------------------------------------------------------//
// NAME     : StatsDBInterfaceDesc::StatsDBInterfaceDesc
// PURPOSE  : StatsDBInterfaceDesc constructor. Initialize INTERFACE_Description
//            table values
// PARAMETERS :
// + nodeId : ID of node to which the interface belongs
// + interfaceIndex : Index of interface
//
// RETURN   : None.
//--------------------------------------------------------------------//
StatsDBInterfaceDesc::StatsDBInterfaceDesc(Int32 nodeId,
                                           Int32 interfaceIndex) :
    m_NodeId(nodeId),
    m_InterfaceIndex(interfaceIndex),
    m_InterfaceName(""),
    m_InterfaceAddr(""),
    m_SubnetMask(""),
    m_NetworkType(""),
    m_MulticastProtocol(""),
    m_SubnetId(0)
    { }

//--------------------------------------------------------------------//
// NAME     : StatsDBPhyDesc::StatsDBPhyDesc
// PURPOSE  : StatsDBPhyDesc constructor. Initialize PHY_Description
//            table values
// PARAMETERS :
// + nodeId : ID of node to which the radio belongs
// + interfaceIndex : Index of interface to which the radio channel belongs
// + phyIndex : Index of the radio channel (i.e. its index into the list of
//              channels)
//
// RETURN   : None.
//--------------------------------------------------------------------//
StatsDBPhyDesc::StatsDBPhyDesc(Int32 nodeId,
                               Int32 interfaceIndex,
                               Int32 phyIndex)
{
    m_NodeId = nodeId;
    m_InterfaceIndex = interfaceIndex;
    m_PhyIndex = phyIndex;
}


///////////////////////////////////////////////////////////////////////
// Status Tables definitions
///////////////////////////////////////////////////////////////////////



//--------------------------------------------------------------------//
// NAME     : StatsDBNodeStatus::StatsDBNodeStatus
// PURPOSE  : StatsDBNodeStatus constructor. Initialize NODE_Status
//            table values
// PARAMETERS :
// + node : Current node
// + triggered : TRUE if this object was instantiated as the result of
//               a 'triggered' (event-initiated) update. FALSE otherwise.
//
// RETURN   : None.
//--------------------------------------------------------------------//
StatsDBNodeStatus::StatsDBNodeStatus(Node* node,
                                     BOOL triggered)
{
    m_NodeId = node->nodeId;
    m_TriggeredUpdate = triggered;

    m_PositionUpdated = TRUE;
    m_ActiveStateUpdated = TRUE;
    m_DamageStateUpdated = TRUE;
    m_VelocityUpdated = TRUE;

    /* Fill in all of the member fields */

    PartitionData* partition = node->partitionData;

    // Position
    if (partition->terrainData->getCoordinateSystem() == CARTESIAN)
    {
        m_DimensionOnePosition = node->mobilityData->current->position.cartesian.x;
        m_DimensionTwoPosition = node->mobilityData->current->position.cartesian.y;
        m_DimensionThreePosition = node->mobilityData->current->position.cartesian.z;
    }
    else if (partition->terrainData->getCoordinateSystem() == LATLONALT)
    {
        m_DimensionOnePosition = node->mobilityData->current->position.latlonalt.latitude;
        m_DimensionTwoPosition = node->mobilityData->current->position.latlonalt.longitude;
        m_DimensionThreePosition = node->mobilityData->current->position.latlonalt.altitude;
    }

    // Active state
    // A node is functionally defined to be active (enabled) if at least one of its
    // interfaces is currently enabled. If a node has no enabled interfaces, the
    // node is considered to be inactive (disabled).
    m_Active = STATS_DB_Disabled;
    Int32 i;
    for (i = 0; i < node->numberInterfaces; i++)
    {
        if (NetworkIpInterfaceIsEnabled(node, i))
        {
            m_Active = STATS_DB_Enabled;
            break;
        }
    }

    // Damage state and Gatway
    // NOTE: Damage state only carries any meaning when using the CES socket
    // interface. When the socket interface is not in use, all nodes are set
    // to undamaged by default.
#ifdef SOCKET_INTERFACE
    EXTERNAL_Interface* socketIface;
    socketIface = node->partitionData->interfaceTable[EXTERNAL_SOCKET];
    if (socketIface != NULL)
    {
        EntityMapping* mapping;
        mapping = SocketInterface_GetEntityMappingStructure(socketIface);
        EntityData* entity = NULL;
        entity = EntityMappingLookup(mapping, node->nodeId);
        if (entity != NULL)
        {
            if (entity->damageState == SocketInterface_PlatformStateType_Undamaged)
            {
                m_DamageState = STATS_DB_Undamaged;
            }
            else
            {
                m_DamageState = STATS_DB_Damaged;
            }
        }
        else
        {
            m_DamageState = STATS_DB_Undamaged;
        }
    }
    else
    {
        m_DamageState = STATS_DB_Undamaged;
    }
#else
    m_DamageState = STATS_DB_Undamaged;
#endif
    // Velocity

    // Check that CES-SOCKET is present and set to YES
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        partition->nodeInput,
        "CES-SOCKET",
        &wasFound,
        buf);
    if (!wasFound)
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            partition->nodeInput,
            "SOCKET-INTERFACE",
            &wasFound,
            buf);
    }
    if (wasFound && strcmp(buf, "YES") == 0)
    {

        if (partition->terrainData->getCoordinateSystem() == CARTESIAN)
        {
            m_DimensionOneVelocity = node->mobilityData->lastExternalVelocity.cartesian.x;
            m_DimensionTwoVelocity = node->mobilityData->lastExternalVelocity.cartesian.y;
            m_DimensionThreeVelocity = node->mobilityData->lastExternalVelocity.cartesian.z;
        }
        else if (partition->terrainData->getCoordinateSystem() == LATLONALT)
        {
            m_DimensionOneVelocity = node->mobilityData->lastExternalVelocity.latlonalt.latitude;
            m_DimensionTwoVelocity = node->mobilityData->lastExternalVelocity.latlonalt.longitude;
            m_DimensionThreeVelocity = node->mobilityData->lastExternalVelocity.latlonalt.altitude;
        }
    }
    else
    {
        // When not using the socket interface, velocity is not explicity specified.
        // Rather, each mobile node has a speed, a destination, and an arrival time. The
        // directional velocities are computed here.
        double c1 = node->mobilityData->current->position.common.c1;
        double c2 = node->mobilityData->current->position.common.c2;
        double c3 = node->mobilityData->current->position.common.c3;
        double currTime = (double) node->mobilityData->current->time / SECOND;

        double c1_next = node->mobilityData->next->position.common.c1;
        double c2_next = node->mobilityData->next->position.common.c2;
        double c3_next = node->mobilityData->next->position.common.c3;
        double nextTime = (double) node->mobilityData->next->time / SECOND;

        double v_c1 = (c1_next - c1) / (nextTime - currTime);
        double v_c2 = (c2_next - c2) / (nextTime - currTime);
        double v_c3 = (c3_next - c3) / (nextTime - currTime);

        m_DimensionOneVelocity = v_c1;
        m_DimensionTwoVelocity = v_c2;
        m_DimensionThreeVelocity = v_c3;
    }
}

///////////////////////////////////////////////////////////////////////
// Aggregate Tables definitons
///////////////////////////////////////////////////////////////////////


StatsDBAppAggregateParam::StatsDBAppAggregateParam()
{
    m_UnicastMessageSent = 0;
    m_UnicastMessageRecd = 0;
    m_MulticastMessageSent = 0;
    m_EffMulticastMessageSent = 0;
    m_MulticastMessageRecd = 0;
    m_UnicastByteSent = 0;
    m_UnicastByteRecd = 0;
    m_UnicastFragmentSent = 0;
    m_UnicastFragmentRecd = 0;
    m_UnicastMessageCompletionRate = 0;
    m_MulticastMessageCompletionRate = 0;
    m_UnicastOfferedLoad = 0;
    m_UnicastThroughput = 0;
    m_MulticastOfferedLoad = 0;
    m_MulticastThroughput = 0;

    m_MulticastByteSent = 0;
    m_EffMulticastByteSent = 0;
    m_MulticastByteRecd = 0;
    m_MulticastFragmentSent = 0;
    m_EffMulticastFragmentSent = 0;
    m_MulticastFragmentRecd = 0;

    m_UnicastDelay.clear();
    m_MulticastDelay.clear();
    m_UnicastJitter.clear();
    m_MulticastJitter.clear();
    m_UnicastHopCount.clear();
    m_MulticastHopCount.clear();
}

// Network Aggregate Table definitions

StatsDBNetworkAggregateParam::StatsDBNetworkAggregateParam()
{
    m_UDataPacketsSent = 0;
    m_UDataPacketsRecd = 0;
    m_UDataPacketsForward = 0;
    m_UControlPacketsSent = 0;
    m_UControlPacketsRecd = 0;
    m_UControlPacketsForward = 0;

    m_MDataPacketsSent = 0;
    m_MDataPacketsRecd = 0;
    m_MDataPacketsForward = 0;
    m_MControlPacketsSent = 0;
    m_MControlPacketsRecd = 0;
    m_MControlPacketsForward = 0;

    m_BDataPacketsSent = 0;
    m_BDataPacketsRecd = 0;
    m_BDataPacketsForward = 0;
    m_BControlPacketsSent = 0;
    m_BControlPacketsRecd = 0;
    m_BControlPacketsForward = 0;

    m_UDataBytesSent = 0;
    m_UDataBytesRecd = 0;
    m_UDataBytesForward = 0;
    m_UControlBytesSent = 0;
    m_UControlBytesRecd = 0;
    m_UControlBytesForward = 0;

    m_MDataBytesSent = 0;
    m_MDataBytesRecd = 0;
    m_MDataBytesForward = 0;
    m_MControlBytesSent = 0;
    m_MControlBytesRecd = 0;
    m_MControlBytesForward = 0;

    m_BDataBytesSent = 0;
    m_BDataBytesRecd = 0;
    m_BDataBytesForward = 0;
    m_BControlBytesSent = 0;
    m_BControlBytesRecd = 0;
    m_BControlBytesForward = 0;

    Int32 t;
    for (t = 0; t < StatsDBNetworkAggregateContent::s_numTrafficTypes; ++t)
    {
        m_CarrierLoad.push_back(0);

        m_Delay.push_back(0);
        m_DelaySpecified.push_back(FALSE);

        m_Jitter.push_back(0.0);
        m_JitterSpecified.push_back(FALSE);

        m_ipOutNoRoutes.push_back(0);
        m_ipOutNoRoutesSpecified.push_back(FALSE);

        m_totalJitter.push_back(0) ;
        m_jitterDataPoints.push_back(0);
        m_jitterDataPointsSpecified.push_back(FALSE) ;
    }
}

void StatsDBNetworkAggregateParam::SetDelay(double delay,
                                            StatsDBNetworkAggregateContent::NetAggrTrafficType t)
{
    ERROR_Assert(t>= StatsDBNetworkAggregateContent::UNICAST &&
        t <= StatsDBNetworkAggregateContent::BROADCAST,
        "ERROR in StatsDBNetworkAggregateParam traffic type");
    m_Delay[t] = delay;
    m_DelaySpecified[t] = TRUE;
}

void StatsDBNetworkAggregateParam::SetJitter(double jitter,
    StatsDBNetworkAggregateContent::NetAggrTrafficType t)
{
    ERROR_Assert(t>= StatsDBNetworkAggregateContent::UNICAST &&
        t <= StatsDBNetworkAggregateContent::BROADCAST,
        "ERROR in StatsDBNetworkAggregateParam traffic type");
    m_Jitter[t] = jitter;
    m_JitterSpecified[t] = TRUE;
}

void StatsDBNetworkAggregateParam::SetIpOutNoRoutes(
    Int32 ipOutNoRoutes,
    StatsDBNetworkAggregateContent::NetAggrTrafficType t)
{
    ERROR_Assert(t>= StatsDBNetworkAggregateContent::UNICAST
        && t < StatsDBNetworkAggregateContent::BROADCAST,
        "ERROR in StatsDBNetworkAggregateParam traffic type");
    m_ipOutNoRoutes[t] = ipOutNoRoutes;
    m_ipOutNoRoutesSpecified[t] = TRUE;
}

StatsDBPhyAggregateParam::StatsDBPhyAggregateParam()
{
    toInsert = FALSE;
    m_NumTransmittedSignals = 0;
    m_NumLockedSignals = 0;
    m_NumReceivedSignals = 0;
    m_NumDroppedSignals = 0;
    m_NumDroppedInterferenceSignals = 0;
}

StatsDBMacAggregateParam::StatsDBMacAggregateParam()
{
    toInsert = FALSE;

    m_DataFramesSent = 0;
    m_DataFramesReceived = 0;
    m_DataBytesSent = 0;
    m_DataBytesReceived = 0;
    m_ControlFramesSent = 0;
    m_ControlFramesReceived = 0;
    m_ControlBytesSent = 0;
    m_ControlBytesReceived  = 0;
}

StatsDBAppJitterAggregateParam::StatsDBAppJitterAggregateParam()
{
    m_PartitionId = -1;
    m_TotalUnicastJitter = 0;
    m_TotalMulticastJitter = 0;
    m_UnicastMessageReceived = 0;
    m_MulticastMessageReceived =0;
}

///////////////////////////////////////////////////////////////////////
// Summary Tables definitions
///////////////////////////////////////////////////////////////////////

StatsDBAppSummaryParam::StatsDBAppSummaryParam()
{
    m_InitiatorId = 0;
    m_ReceiverId = 0;
    m_TargetAddr[0] = '\0';
    m_SessionId = 0;
    m_Tos = 0;
    m_MessageSent = 0;
    m_EffMessageSent = 0;
    m_MessageRecd = 0;
    m_ByteSent = 0;
    m_EffByteSent = 0;
    m_ByteRecd = 0;
    m_FragmentSent = 0;
    m_EffFragmentSent = 0;
    m_FragmentRecd = 0;
    m_ApplicationType[0] = '\0';
    m_ApplicationName[0] = '\0' ;

#ifdef ADDON_NGCNMS
    isRetrieved = FALSE ;
#endif
}

StatsDBAppJitterSummaryParam::StatsDBAppJitterSummaryParam()
{
    m_InitiatorId = 0;
    m_SessionId = 0;
    m_TotalJitter = 0;
    m_MessageRecd = 0;
}

StatsDBMulticastAppSummaryParam::StatsDBMulticastAppSummaryParam()
{
    m_InitiatorId = 0;
    m_ReceiverId = 0;
    m_GroupAddr[0] = '\0';
    m_SessionId = 0;
    m_Tos = 0;
    m_MessageSent = 0;
    m_MessageRecd = 0;
    m_ByteSent = 0;
    m_ByteRecd = 0;
    m_FragmentSent = 0;
    m_FragmentRecd = 0;
    m_ApplicationType[0] = '\0';
    m_ApplicationName[0] = '\0' ;

#ifdef ADDON_NGCNMS
    isRetrieved = FALSE ;
#endif
}

// Network Summary
StatsDBNetworkSummaryParam::StatsDBNetworkSummaryParam()
{
    m_UDataPacketsSent = 0;
    m_UDataPacketsRecd = 0;
    m_UDataPacketsForward = 0;
    m_UControlPacketsSent = 0;
    m_UControlPacketsRecd = 0;
    m_UControlPacketsForward = 0;

    m_UDataBytesSent = 0;
    m_UDataBytesRecd = 0;
    m_UDataBytesForward = 0;
    m_UControlBytesSent = 0;
    m_UControlBytesRecd = 0;
    m_UControlBytesForward = 0;
}
StatsDBMacSummaryParam::StatsDBMacSummaryParam()
{
    m_BroadcastDataFramesSent = 0;
    m_UnicastDataFramesSent = 0;
    m_BroadcastDataFramesReceived = 0;
    m_UnicastDataFramesReceived = 0;
    m_BroadcastDataBytesSent = 0;
    m_UnicastDataBytesSent = 0;
    m_BroadcastDataBytesReceived = 0;
    m_UnicastDataBytesReceived = 0;

    m_ControlFramesSent = 0;
    m_ControlFramesReceived = 0;
    m_ControlBytesSent = 0;
    m_ControlBytesReceived  = 0;
    m_FramesDropped = 0;
    m_BytesDropped = 0;
}

StatsDBPhySummaryParam::StatsDBPhySummaryParam()
{
    m_SenderId = 0;
    m_RecieverId = 0;
    m_PhyIndex = -1;
    m_Utilization = 0;
    m_NumSignals = 0;
    m_NumErrorSignals = 0;
}

///////////////////////////////////////////////////////////////////////
//Events Tables definitions
///////////////////////////////////////////////////////////////////////


StatsDBAppEventParam::StatsDBAppEventParam()
{
    m_NodeId = 0;
    m_ReceiverId = 0;
    m_fragEnabled = FALSE;
    m_IsFragmentation = FALSE;

        m_MessageId[0] = '\0';
        m_EventType[0] = '\0';
    m_ApplicationType[0] = '\0';
    m_ApplicationName[0] = '\0';
}

void StatsDBAppEventParam::SetMessageId(const char* id, bool recordFragment)
{
        if (!recordFragment) {
                int i = 0;  // need i after loop to tie off string
                for (; id[i] != '\0' && id[i] != 'A'; i++)
                {
                        m_MessageId[i] = id[i];
                }
                m_MessageId[i] = '\0';
        }
        else
        {
                strncpy(m_MessageId, id, sizeof(m_MessageId));
        }
}

void StatsDBAppEventParam::SetAppType(const char* appType)
{
    if (appType)
    {
        strncpy(m_ApplicationType, appType, MAX_STRING_LENGTH);
    }
}

void StatsDBAppEventParam::SetAppName(const char* appName)
{
    if (appName)
    {
        strncpy(m_ApplicationName, appName, MAX_STRING_LENGTH);
    }
}

StatsDBTransportEventParam::StatsDBTransportEventParam(Int32 nodeId,
                                                       char* msgId,
                                                       Int32 size)
{
    m_NodeId = nodeId;
    strcpy(m_MessageId, msgId);

    m_MsgSize = size;
}

StatsDBTransportEventParam::StatsDBTransportEventParam(
    Int32 nodeId,
    const std::string& msgId,
    Int32 size)
{
    m_NodeId = nodeId;
    strcpy(m_MessageId, msgId.c_str());

    m_MsgSize = size;
}

StatsDBNetworkEventParam::StatsDBNetworkEventParam()
{
    m_NodeId = 0;
    m_SenderAddr = 0;
    m_ReceiverAddr = 0;
    m_MsgSize = 0;
}

StatsDBMacEventParam::StatsDBMacEventParam(
    Int32 nodeId,
    const std::string& msgId,
    Int32 interfaceIndex,
    Int32 size,
    const std::string& eventType)
{
    m_NodeId = nodeId;
    m_MessageId = msgId;
    m_InterfaceIndex = interfaceIndex;
    m_MsgSize = size;
    m_EventType = eventType;
}

StatsDBPhyEventParam::StatsDBPhyEventParam(Int32 nodeId,
                                           std::string messageId,
                                           Int32 phyIndex,
                                           Int32 msgSize,
                                           std::string eventType)
{
    m_NodeId = nodeId;
    m_MessageId = messageId;
    m_PhyIndex = phyIndex;
    m_MsgSize = msgSize;
    m_EventType = eventType;
}

///////////////////////////////////////////////////////////////////////
// Connectivity table definitions
///////////////////////////////////////////////////////////////////////


StatsDBNetworkConnParam::StatsDBNetworkConnParam()
{
    m_NodeId = 0;
    m_DstAddress = "";
    m_Cost = 0;
}

StatsDBPhyConnParam::StatsDBPhyConnParam()
{
    m_SenderId = 0;
    m_ReceiverId = 0;

    senderListening = FALSE;
    receiverListening = FALSE;
    reachableWorst = FALSE ;
}


//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleNodeDescTableInsert
// PURPOSE  : Insert a row into the NODE_Description table
// PARAMETERS :
// + node : Current node
// + partition : Current partition
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleNodeDescTableInsert(Node* node, PartitionData* partition)
{
    StatsDb* db = NULL;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsDescTable->createNodeDescTable)
    {
        // Table does not exist
        return;
    }
    // In this table we insert the node content on to the database.
    std::string queryStr = "";

    std::vector<std::string> newValues;
    newValues.reserve(3);
    std::vector<std::string> columns;
    columns.reserve(3);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("HostName");
    newValues.push_back(std::string(node->hostname));

    // Add the meta data columns.
    std::map<std::string, std::string>::iterator iter =
        node->meta_data->m_MetaData.begin();

    while (iter != node->meta_data->m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }

    InsertValues(db, "NODE_Description", columns, newValues);
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleQueueDescTableInsert
// PURPOSE  : Insert a row into the QUEUE_Description table
// PARAMETERS :
// + node : Current node
// + queueDesc : Structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleQueueDescTableInsert(
    Node* node,
    StatsDBQueueDesc queueDesc)
{
    PartitionData* partition = node->partitionData;
    StatsDb* db = NULL;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsDescTable->createQueueDescTable)
    {
        // Table does not exist
        return;
    }
    // In this table we insert the node content on to the database.
    std::string queryStr = "";

    std::vector<std::string> newValues;
    newValues.reserve(8);
    std::vector<std::string> columns;
    columns.reserve(8);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("InterfaceIndex");
    newValues.push_back(STATSDB_IntToString(queueDesc.m_InterfaceIndex));
    columns.push_back("QueueType");
    newValues.push_back(queueDesc.m_QueueType);
    columns.push_back("QueueIndex");
    newValues.push_back(STATSDB_IntToString(queueDesc.m_QueueIndex));

    if (!queueDesc.m_QueueDiscipline.isNULL() &&
        db->statsQueueDesc->isDiscipline)
    {
        columns.push_back("QueueDiscipline");
        newValues.push_back(queueDesc.m_QueueDiscipline.getStr());
    }
    if (!queueDesc.m_QueuePriority.isNULL() &&
        db->statsQueueDesc->isPriority)
    {
        columns.push_back("QueuePriority");
        newValues.push_back(queueDesc.m_QueuePriority.getStr());
    }
    if (!queueDesc.m_QueueSize.isNULL() && db->statsQueueDesc->isSize)
    {
        columns.push_back("QueueSize");
        newValues.push_back(queueDesc.m_QueueSize.getStr());
    }
    std::map<std::string, std::string>::iterator iter =
        queueDesc.m_QueueMetaData.m_MetaData.begin();
    while (iter != queueDesc.m_QueueMetaData.m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }

    InsertValues(db, "QUEUE_Description", columns, newValues);
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleInterfaceDescTableInsert
// PURPOSE  : Insert a new row into the INTERFACE_Description table
// PARAMETERS :
// + node : The node that the interface belongs to
// + interfaceDesc : Structure containing the values to insert into the
//                   table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleInterfaceDescTableInsert(
    Node* node,
    StatsDBInterfaceDesc interfaceDesc)
{
    PartitionData* partition = node->partitionData;
    StatsDb* db = NULL;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsDescTable->createInterfaceDescTable)
    {
        // Table does not exist
        return;
    }
    // In this table we insert the interface content on to the database.
    size_t i;

    std::vector<std::string> newValues;
    std::vector<std::string> columns;
    if (db->statsInterfaceDesc->interfaceDescTableDef.size() > 0)
    {
        newValues.reserve(
            db->statsInterfaceDesc->interfaceDescTableDef.size() * 9);
        columns.reserve(
            db->statsInterfaceDesc->interfaceDescTableDef.size() * 9);
    }

    for (i = 0; i < db->statsInterfaceDesc->interfaceDescTableDef.size(); i++)
    {
        if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "Timestamp")
        {
            columns.push_back(
                db->statsInterfaceDesc->interfaceDescTableDef[i].first);
            newValues.push_back(
                STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "NodeId")
        {
            columns.push_back(
                db->statsInterfaceDesc->interfaceDescTableDef[i].first);
            newValues.push_back(STATSDB_IntToString(node->nodeId));
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "InterfaceIndex")
        {
            columns.push_back(
                db->statsInterfaceDesc->interfaceDescTableDef[i].first);
            newValues.push_back(
                STATSDB_IntToString(interfaceDesc.m_InterfaceIndex));
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "InterfaceName")
        {
            if (!interfaceDesc.m_InterfaceName.isNULL() &&
                db->statsInterfaceDesc->isName)
            {
                columns.push_back(
                    db->statsInterfaceDesc->interfaceDescTableDef[i].first);
                newValues.push_back(interfaceDesc.m_InterfaceName.getStr());
            }
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "InterfaceAddress")
        {
            if (!interfaceDesc.m_InterfaceAddr.isNULL() &&
                db->statsInterfaceDesc->isAddress)
            {
                columns.push_back(
                    db->statsInterfaceDesc->interfaceDescTableDef[i].first);
                newValues.push_back(interfaceDesc.m_InterfaceAddr.getStr());
            }
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "SubnetMask")
        {
            if (!interfaceDesc.m_SubnetMask.isNULL() &&
                db->statsInterfaceDesc->isSubnetMask)
            {
                columns.push_back(
                    db->statsInterfaceDesc->interfaceDescTableDef[i].first);
                newValues.push_back(interfaceDesc.m_SubnetMask.getStr());
            }
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "RoutingProtocol")
        {
            if (!interfaceDesc.m_NetworkType.isNULL() &&
                db->statsInterfaceDesc->isNetworkProtocol)
            {
                columns.push_back(
                    db->statsInterfaceDesc->interfaceDescTableDef[i].first);
                newValues.push_back(interfaceDesc.m_NetworkType.getStr());
            }
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "MulticastProtocol")
        {
            if (!interfaceDesc.m_MulticastProtocol.isNULL() &&
                db->statsInterfaceDesc->isMulticastProtocol)
            {
                columns.push_back(
                    db->statsInterfaceDesc->interfaceDescTableDef[i].first);
                newValues.push_back(interfaceDesc.m_MulticastProtocol.getStr());
            }
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "SubnetId")
        {
        }
    }

    // Add the meta data columns.
    std::map<std::string, std::string>::iterator iter =
        interfaceDesc.m_InterfaceMetaData.m_MetaData.begin();

    while (iter != interfaceDesc.m_InterfaceMetaData.m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }

    InsertValues(db, "INTERFACE_Description", columns, newValues);
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleSchedulerDescTableInsert
// PURPOSE  : Insert a new row into the SCHEDULER_Description table
// PARAMETERS :
// + node : The node that the scheduler belongs to
// + schedulerDesc : Structure containing the values to insert into the
//                   table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleSchedulerDescTableInsert(
    Node* node,
    Int32 interfaceIndex,
    const char* schedulerType,
    const char* schedulerAlgorithm)
{
    PartitionData* partition = node->partitionData;
    StatsDb* db = NULL;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsDescTable->createSchedulerDescTable)
    {
        // Table does not exist
        return;
    }

    StatsDBSchedulerDesc schedulerDesc(node->nodeId, interfaceIndex,
        schedulerType, schedulerAlgorithm);

    schedulerDesc.m_SchedulerMetaData.AddSchedulerMetaData(node,
        node->partitionData,
        node->partitionData->nodeInput);

    // In this table we insert the node content on to the database.
    std::vector<std::string> newValues;
    newValues.reserve(5);
    std::vector<std::string> columns;
    columns.reserve(5);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("InterfaceIndex");
    newValues.push_back(
        STATSDB_InterfaceToString(schedulerDesc.m_InterfaceIndex));
    columns.push_back("SchedulerType");
    newValues.push_back(schedulerDesc.m_SchedulerType);

    if (db->statsSchedulerAlgo->isSchedulerAlgo)
    {
        columns.push_back("SchedulerAlgorithm");
        newValues.push_back(schedulerDesc.m_SchedulingAlgorithm);
    }

    // Add the meta data columns.
    std::map<std::string, std::string>::iterator iter =
        schedulerDesc.m_SchedulerMetaData.m_MetaData.begin();

    while (iter != schedulerDesc.m_SchedulerMetaData.m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }

    // Now to insert the content into the table.
    InsertValues(db, "SCHEDULER_Description", columns, newValues);
}

string STATSDB_ConvertAddressToString(const Address &address)
{
    //std::string addressStr;
    char addressString[MAX_STRING_LENGTH];
    IO_ConvertIpAddressToString((Address *)&address, addressString);

    return addressString;
}

string STATSDB_ConvertAddressToString(const NodeAddress &address)
{
    //std::string addressStr;
    char addressString[MAX_STRING_LENGTH];
    IO_ConvertIpAddressToString((NodeAddress )address, addressString);

    return addressString;
}

std::string STATSDB_InterfaceToString(Int32 index)
{
    if (index >= 0)
    {
        return STATSDB_IntToString(index);
    }
    else if (index == CPU_INTERFACE)
    {
        return "CPU";
    }else
        return "BACKPLANE";
}

std::string STATSDB_ChannelToString(
        Node* node,
        Int32 interfaceIndex,
        Int32 channelIndex)
{
    if (channelIndex >= 0)
    {
        return STATSDB_IntToString(channelIndex);
    }

    std::string buf;
    switch(node->macData[interfaceIndex]->macProtocol )
    {
    case MAC_PROTOCOL_SATCOM:
        buf = "SATCOM";
        break ;
    case MAC_PROTOCOL_LINK:
        buf = "LINK";
        break;
    case MAC_PROTOCOL_802_3:
        buf = "ETHERNET";
        break ;
    default:
        ERROR_Assert(FALSE, "ERROR in STATSDB_ChannelToString.");
    }
    return buf;
}
string STATSDB_IntToString(Int32 num)
{
    char buf[MAX_STRING_LENGTH];
    sprintf(buf, "%d", num);
    return (string) buf;
}

string STATSDB_DoubleToString(double f)
{
    char buf[MAX_STRING_LENGTH];
    sprintf(buf, STATSDB_DOUBLE_FORMAT, f);
    return (string) buf;
}

string STATSDB_Int64ToString(Int64 num)
{
    char buf[MAX_STRING_LENGTH];
    sprintf(buf, "%" TYPES_64BITFMT "d", num);
    return (string) buf;
}

string STATSDB_UInt64ToString(UInt64 num)
{
    char buf[MAX_STRING_LENGTH];
    sprintf(buf, "%" TYPES_64BITFMT "u", num);
    return (string) buf;
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleSessionDescTableInsert
// PURPOSE  : Insert a new row into the SCHEDULER_Description table
// PARAMETERS :
// + node : The node that the scheduler belongs to
// + schedulerDesc : Structure containing the values to insert into the
//                   table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Int32 sessionId,
    const NodeAddress &clientAddr,
    const NodeAddress &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol)
{
    PartitionData* partition = node->partitionData;
    StatsDb* db = NULL;
    db = partition->statsDb;

    // Check if the Table exists.
    if (!db ||!db->statsDescTable->createSessionDescTable)
    {
        // Table does not exist
        return;
    }

    Int32 senderId = MAPPING_GetNodeIdFromInterfaceAddress(node,
                                                           clientAddr);

    Int32 receiverId = MAPPING_GetNodeIdFromInterfaceAddress(node,
                                                             serverAddr);

    // actually sessionDesc seems useless...
    StatsDBSessionDesc sessionDesc(sessionId, senderId, receiverId);

    sessionDesc.m_SessionMetaData.AddSessionMetaData(node,
        node->partitionData,
        node->partitionData->nodeInput);

    sessionDesc.SetSenderAddr(STATSDB_ConvertAddressToString(clientAddr));
    sessionDesc.SetReceiverAddr(STATSDB_ConvertAddressToString(serverAddr));
    sessionDesc.SetSenderPort(clientPort);
    sessionDesc.SetRecvPort(serverPort);
    sessionDesc.SetTransportProtocol(transportProtocol);
    sessionDesc.SetAppType(appType);

    // In this table we insert the node content on to the database.
    std::vector<std::string> newValues;
    newValues.reserve(10);
    std::vector<std::string> columns;
    columns.reserve(10);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("SessionId");
    newValues.push_back(STATSDB_IntToString(sessionDesc.m_SessionId));
    columns.push_back("SenderId");
    newValues.push_back(STATSDB_IntToString(sessionDesc.m_SenderId));
    columns.push_back("ReceiverId");
    newValues.push_back(STATSDB_IntToString(sessionDesc.m_ReceiverId));

    if (!sessionDesc.m_SenderAddr.isNULL() &&
        db->statsSessionDesc->isSenderAddr)
    {
        columns.push_back("SenderAddr");
        newValues.push_back(sessionDesc.m_SenderAddr.getStr());
    }
    if (!sessionDesc.m_ReceiverAddr.isNULL() &&
        db->statsSessionDesc->isReceiverAddr)
    {
        columns.push_back("ReceiverAddr");
        newValues.push_back(sessionDesc.m_ReceiverAddr.getStr());
    }
    if (!sessionDesc.m_SenderPort.isNULL() &&
        db->statsSessionDesc->isSenderPort)
    {
        columns.push_back("SenderPort");
        newValues.push_back(sessionDesc.m_SenderPort.getStr());
    }
    if (!sessionDesc.m_RecvPort.isNULL() &&
        db->statsSessionDesc->isRecvPort)
    {
        columns.push_back("ReceiverPort");
        newValues.push_back(sessionDesc.m_RecvPort.getStr());
    }
    if (!sessionDesc.m_TransportProtocol.isNULL() &&
        db->statsSessionDesc->isTransportProtocol)
    {
        columns.push_back("TransportProtocol");
        newValues.push_back(sessionDesc.m_TransportProtocol.getStr());
    }
    if (!sessionDesc.m_AppType.isNULL() &&
        db->statsSessionDesc->isAppType)
    {
        columns.push_back("AppType");
        newValues.push_back(sessionDesc.m_AppType.getStr());
    }

    // Add the meta data columns.
    std::map<std::string, std::string>::iterator iter =
        sessionDesc.m_SessionMetaData.m_MetaData.begin();

    while (iter != sessionDesc.m_SessionMetaData.m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }

    // Now to insert the content into the table.
    InsertValues(db, "SESSION_Description", columns, newValues);
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleSessionDescTableInsert
// PURPOSE  : Insert a new row into the SCHEDULER_Description table
// PARAMETERS :
// + node : The node that the scheduler belongs to
// + schedulerDesc : Structure containing the values to insert into the
//                   table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Int32 sessionId,
    const Address &clientAddr,
    const Address &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol)
{

    StatsDb* db = node->partitionData->statsDb;
    if (!db || !db->statsDescTable->createSessionDescTable)
    {
        return ;
    }

    NodeAddress convertedClientAddr = clientAddr.interfaceAddr.ipv4;
    NodeAddress convertedServerAddr = serverAddr.interfaceAddr.ipv4;

    STATSDB_HandleSessionDescTableInsert(
        node, sessionId, convertedClientAddr, convertedServerAddr,
        clientPort, serverPort, appType, transportProtocol);

}

void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Message* msg,
    const NodeAddress &clientAddr,
    const NodeAddress &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol)
{

    StatsDb* db = node->partitionData->statsDb;
    if (!db || !db->statsDescTable->createSessionDescTable)
    {
        return ;
    }

    Int32* sessionIdInfo =  (Int32 *)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbAppSessionId);
    ERROR_Assert(sessionIdInfo,
        "Errror in STATSDB_HandleSessionDescTableInsert.");

    STATSDB_HandleSessionDescTableInsert(
        node, *sessionIdInfo, clientAddr, serverAddr,
        clientPort, serverPort, appType, transportProtocol);

}
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Message* msg,
    const Address &clientAddr,
    const Address &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol)
{

    StatsDb* db = node->partitionData->statsDb;
    if (!db || !db->statsDescTable->createSessionDescTable)
    {
        return ;
    }

    Int32* sessionIdInfo =  (Int32 *)
        MESSAGE_ReturnInfo(msg, INFO_TYPE_StatsDbAppSessionId);
    ERROR_Assert(sessionIdInfo,
        "Errror in STATSDB_HandleSessionDescTableInsert.");
    NodeAddress convertedClientAddr = clientAddr.interfaceAddr.ipv4;
    NodeAddress convertedServerAddr = serverAddr.interfaceAddr.ipv4;

    STATSDB_HandleSessionDescTableInsert(
        node, *sessionIdInfo, convertedClientAddr, convertedServerAddr,
        clientPort, serverPort, appType, transportProtocol);

}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleConnectionDescTableInsert
// PURPOSE  : Insert a new row into the SCHEDULER_Description table
// PARAMETERS :
// + node : The node that the scheduler belongs to
// + schedulerDesc : Structure containing the values to insert into the
//                   table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleConnectionDescTableInsert(Node* node,
    /*int appSessionId,*/
    const Address & local_addr,
    const Address & remote_addr,
    short local_port,
    short remote_port,
    const std::string &connection_type)
{

    PartitionData* partition = node->partitionData;
    StatsDb* db = NULL;
    double timeVal = 0.0;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsDescTable->createConnectionDescTable)
    {
        // Table does not exist
        return;
    }

    timeVal = (double) node->getNodeTime() / SECOND;
    // actually connectionDesc seems useless...
    Int32 senderId = MAPPING_GetNodeIdFromInterfaceAddress(node,
                                                           local_addr);

    Int32 receiverId = MAPPING_GetNodeIdFromInterfaceAddress(node,
                                                             remote_addr);
    StatsDBConnectionDesc connectionDesc(timeVal,/* appSessionId,*/ senderId, receiverId);

    connectionDesc.SetSenderAddr(STATSDB_ConvertAddressToString(local_addr));
    connectionDesc.SetReceiverAddr(STATSDB_ConvertAddressToString(remote_addr));
    connectionDesc.SetSenderPort(local_port);
    connectionDesc.SetRecvPort(remote_port);
    connectionDesc.SetConnectionType(connection_type);
    if (local_addr.networkType == NETWORK_IPV4)
    {
        connectionDesc.SetNetworkProtocol("NETWORK_IPV4");
    }
    else {
        connectionDesc.SetNetworkProtocol("NETWORK_IPV6");
    }

    connectionDesc.m_ConnectionMetaData.AddConnectionMetaData(node,
        node->partitionData,
        node->partitionData->nodeInput);

    // In this table we insert the node content on to the database.
    std::vector<std::string> columns;
    columns.reserve(9);
    std::vector<std::string> newValues;
    newValues.reserve(9);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("SenderId");
    newValues.push_back(STATSDB_IntToString(connectionDesc.m_SenderId));
    columns.push_back("ReceiverId");
    newValues.push_back(STATSDB_IntToString(connectionDesc.m_ReceiverId));

    if (!connectionDesc.m_ReceiverAddr.isNULL() &&
        db->statsConnectionDesc->isSenderAddr)
    {
        columns.push_back("SenderAddr");
        newValues.push_back(connectionDesc.m_SenderAddr.getStr());
    }
    if (!connectionDesc.m_ReceiverAddr.isNULL() &&
        db->statsConnectionDesc->isReceiverAddr)
    {
        columns.push_back("ReceiverAddr");
        newValues.push_back(connectionDesc.m_ReceiverAddr.getStr());
    }
    if (!connectionDesc.m_SenderPort.isNULL() &&
        db->statsConnectionDesc->isSenderPort)
    {
        columns.push_back("SenderPort");
        newValues.push_back(connectionDesc.m_SenderPort.getStr());
    }
    if (!connectionDesc.m_RecvPort.isNULL() &&
        db->statsConnectionDesc->isRecvPort)
    {
        columns.push_back("ReceiverPort");
        newValues.push_back(connectionDesc.m_RecvPort.getStr());
    }
    if (!connectionDesc.m_ConnectionType.isNULL() &&
        db->statsConnectionDesc->isConnectionType)
    {
        columns.push_back("ConnectionType");
        newValues.push_back(connectionDesc.m_ConnectionType.getStr());
    }
    if (!connectionDesc.m_NetworkProtocol.isNULL() &&
        db->statsConnectionDesc->isNetworkProtocol)
    {
        columns.push_back("NetworkProtocol");
        newValues.push_back(connectionDesc.m_NetworkProtocol.getStr());
    }

    // Add the meta data columns.
    std::map<std::string, std::string>::iterator iter =
        connectionDesc.m_ConnectionMetaData.m_MetaData.begin();

    while (iter != connectionDesc.m_ConnectionMetaData.m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }

    InsertValues(db, "CONNECTION_Description", columns, newValues);
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandlePhyDescTableInsert
// PURPOSE  : Insert a new row into the PHY_Description table
// PARAMETERS :
// + node : The node that the scheduler belongs to
// + schedulerDesc : Structure containing the values to insert into the
//                   table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandlePhyDescTableInsert(
    Node* node,
    Int32 interfaceIndex,
    Int32 phyIndex)
{
    PartitionData* partition = node->partitionData;
    StatsDb* db = NULL;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsDescTable->createPhyDescTable)
    {
        // Table does not exist
        return;
    }

    StatsDBPhyDesc phyDesc(node->nodeId, interfaceIndex,
        phyIndex);

    phyDesc.m_PhyMetaData.AddPhyMetaData(node,
        interfaceIndex,
        node->partitionData,
        node->partitionData->nodeInput);

    // In this table we insert the node content on to the database.
    std::vector<std::string> newValues;
    newValues.reserve(4);
    std::vector<std::string> columns;
    columns.reserve(4);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("InterfaceIndex");
    newValues.push_back(STATSDB_IntToString(phyDesc.m_InterfaceIndex));
    columns.push_back("PhyIndex");
    newValues.push_back(STATSDB_IntToString(phyDesc.m_PhyIndex));

    // Add the meta data columns.
    std::map<std::string, std::string>::iterator iter =
        phyDesc.m_PhyMetaData.m_MetaData.begin();

    while (iter != phyDesc.m_PhyMetaData.m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }

    InsertValues(db, "PHY_Description", columns, newValues);
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleNodeStatusTableInsert
// PURPOSE  : Insert a row into the NODE_Status table
// PARAMETERS :
// + node : Current node
// + nodeStatus : Structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleNodeStatusTableInsert(Node* node, StatsDBNodeStatus nodeStatus)
{
    StatsDb* db = NULL;
    PartitionData* partition = node->partitionData;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsStatusTable->createNodeStatusTable)
    {
        // Table does not exist
        return;
    }

    // In this table we insert the node content on to the database.
    std::vector<std::string> newValues;
    newValues.reserve(12);
    std::vector<std::string> columns;
    columns.reserve(12);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("TriggeredUpdate");
    if (nodeStatus.m_TriggeredUpdate)
    {
        newValues.push_back("TRUE");
    }
    else {
        newValues.push_back("FALSE");
    }

    if (db->statsNodeStatus->isPosition)
    {
        if (partition->terrainData->getCoordinateSystem() == CARTESIAN)
        {
            if (nodeStatus.m_PositionUpdated)
            {
                columns.push_back("X");
                columns.push_back("Y");
                columns.push_back("Z");
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionOnePosition));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionTwoPosition));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionThreePosition));
            }
        }
        else if (partition->terrainData->getCoordinateSystem() == LATLONALT)
        {
            if (nodeStatus.m_PositionUpdated)
            {
                columns.push_back("Lat");
                columns.push_back("Lon");
                columns.push_back("Alt");
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionOnePosition));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionTwoPosition));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionThreePosition));
            }
        }
    }
    if (db->statsNodeStatus->isVelocity)
    {
        if (partition->terrainData->getCoordinateSystem() == CARTESIAN)
        {
            if (nodeStatus.m_PositionUpdated)
            {
                columns.push_back("XVelocity");
                columns.push_back("YVelocity");
                columns.push_back("ZVelocity");
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionOneVelocity));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionTwoVelocity));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionThreeVelocity));
            }
        }
        else if (partition->terrainData->getCoordinateSystem() == LATLONALT)
        {
            if (nodeStatus.m_PositionUpdated)
            {
                columns.push_back("LatVelocity");
                columns.push_back("LonVelocity");
                columns.push_back("AltVelocity");
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionOneVelocity));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionTwoVelocity));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionThreeVelocity));
            }
        }
    }
    if (db->statsNodeStatus->isActiveState)
    {
        if (nodeStatus.m_ActiveStateUpdated)
        {
            columns.push_back("ActiveState");
            if (nodeStatus.m_Active == STATS_DB_Enabled)
            {
                newValues.push_back("Enabled");
            }
            else
            {
                newValues.push_back("Disabled");
            }
        }
    }
    if (db->statsNodeStatus->isDamageState)
    {
        if (nodeStatus.m_DamageStateUpdated)
        {
            columns.push_back("DamageState");
            if (nodeStatus.m_DamageState == STATS_DB_Damaged)
            {
                newValues.push_back("Damaged");
            }
            else if (nodeStatus.m_DamageState == STATS_DB_Undamaged)
            {
                newValues.push_back("Undamaged");
            }
        }
    }
    if (db->statsNodeStatus->isGateway)
    {
        columns.push_back("IsGateway");
        if (nodeStatus.m_IsGateway)
        {
            newValues.push_back("YES");
        }
        else
        {
            newValues.push_back("NO");
        }
    }
    // Now to insert the content into the table.
    InsertValues(db, "NODE_Status", columns, newValues);
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleMulticastStatusTableInsert
// PURPOSE  : Insert a row into the NODE_Status table
// PARAMETERS :
// + node : Current node
// + nodeStatus : Structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleMulticastStatusTableInsert(Node *node)
{
    if (node != node->partitionData->firstNode)
    {
        return ;
    }

    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL || !db->statsStatusTable->createMulticastStatusTable)
    {
        return ;
    }

    StatsDBStatusTable::Const_MultiStatusIter citer =
        db->statsStatusTable->map_MultiStatus.begin() ;

    for (; citer != db->statsStatusTable->map_MultiStatus.end(); ++citer)
    {
        /*if (citer->second->joinPrint && citer->second->leavePrint )
        {
            continue ;
        }
        if (citer->second->joinPrint &&
            !citer->second->leavePrint && citer->second->timeLeft.empty())
        {
            continue ;
        }
        */

        std::vector<std::string> newValues;
        newValues.reserve(6);
        std::vector<std::string> columns;
        columns.reserve(6);

        columns.push_back("Timestamp");
        newValues.push_back(
            STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));

        char srcAddrStr[MAX_STRING_LENGTH];
        IO_ConvertIpAddressToString(citer->first.first, srcAddrStr);
        columns.push_back("NodeAddress");
        newValues.push_back(std::string(srcAddrStr));

        char grpAddrStr[MAX_STRING_LENGTH];
        IO_ConvertIpAddressToString(citer->first.second, grpAddrStr);
        columns.push_back("GroupAddress");
        newValues.push_back(std::string(grpAddrStr));

        columns.push_back("JoiningTime");
        newValues.push_back(std::string(citer->second->timeJoined));
        columns.push_back("LeavingTime");
        newValues.push_back(std::string(citer->second->timeLeft));
        columns.push_back("GroupName");
        newValues.push_back(std::string(citer->second->groupName));

        /*if (!citer->second->timeJoined.empty() && !citer->second->timeLeft.empty())
        {
            citer->second->joinPrint = TRUE;
            citer->second->leavePrint = TRUE;
        }else if (!citer->second->timeJoined.empty() )
        {
            citer->second->joinPrint = TRUE;
        }else
            ERROR_Assert(FALSE, "Error in STATSDB_HandleMulticastStatusTableInsert.");
            */
        // Now to insert the content into the table.

        InsertValues(db, "MULTICAST_Status", columns, newValues);
    }
    return ;
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleMulticastStatusTableInsert
// PURPOSE  : Insert a row into the NODE_Status table
// PARAMETERS :
// + node : Current node
// + nodeStatus : Structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleMulticastStatusTableInsert(Node* node, Message * /*msg*/)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        /*ERROR_ReportWarning("Unable to extract DB information for Node Status\n");*/
        return ;
    }

    // Time to insert the status stats in the database.

    if (db->statsStatusTable->createMulticastStatusTable)
    {
        STATSDB_HandleMulticastStatusTableInsert(node);
    }
}

void STATSDB_HandleInterfaceStatusTableInsert(Node *node,
    BOOL triggeredUpdate)
{
    Int32 i;
    for (i = 0; i < node->numberInterfaces; ++i)
    {
        STATSDB_HandleInterfaceStatusTableInsert(node,
            triggeredUpdate, i);
    }
}
void STATSDB_HandleInterfaceStatusTableInsert(
    Node* node,
    BOOL triggeredUpdate,
    Int32 interfaceIndex)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return ;
    }
    // Time to insert the status stats in the database.
    if (db->statsStatusTable->createInterfaceStatusTable == FALSE)
    {
        return ;
    }
    StatsDBInterfaceStatus interfaceStatus;
    char interfaceAddrStr[100];
    NetworkIpGetInterfaceAddressString(node, interfaceIndex, interfaceAddrStr);
    interfaceStatus.m_triggeredUpdate = triggeredUpdate;
    interfaceStatus.m_address = interfaceAddrStr;
    interfaceStatus.m_interfaceEnabled =
        NetworkIpInterfaceIsEnabled(node, interfaceIndex);

    STATSDB_HandleInterfaceStatusTableInsert(node, interfaceStatus);
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleInterfaceStatusTableInsert
// PURPOSE  : Insert a row into the INTERFACE_Status table
// PARAMETERS :
// + node : Current node
// + interfaceStatus : Structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleInterfaceStatusTableInsert(PartitionData* partition,
    BOOL triggeredUpdate, Message * /*msg*/)
{
    StatsDb* db = partition->statsDb;
    Int32 i;
    if (db == NULL)
    {
        //ERROR_ReportWarning("Unable to extract DB information for Interface Status\n");
        return ;
    }
    // Time to insert the status stats in the database.
    if (db->statsStatusTable->createInterfaceStatusTable)
    {
        Node * nextNode = partition->firstNode;
        while (nextNode != NULL)
        {
            for (i = 0; i < nextNode->numberInterfaces; ++i)
            {
                STATSDB_HandleInterfaceStatusTableInsert(nextNode,
                    triggeredUpdate, i);
            }
            nextNode = nextNode->nextNodeData;
        }
    }
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleInterfaceStatusTableInsert
// PURPOSE  : Insert a row into the INTERFACE_Status table
// PARAMETERS :
// + node : Current node
// + interfaceStatus : Structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleInterfaceStatusTableInsert(Node* node,
    StatsDBInterfaceStatus interfaceStatus)
{
    StatsDb* db = NULL;
    db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }

    // Check if the Table exists.
    if (db == NULL || !db->statsStatusTable->createInterfaceStatusTable)
    {
        // Table does not exist
        return;
    }

    // In this table we insert the node content on to the database.
    std::vector<std::string> newValues;
    newValues.reserve(5);
    std::vector<std::string> columns;
    columns.reserve(5);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("NodeID");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("InterfaceAddress");
    newValues.push_back(interfaceStatus.m_address);

    columns.push_back("InterfaceEnabled");
    if (interfaceStatus.m_interfaceEnabled)
    {
        newValues.push_back("TRUE");
    }
    else
    {
        newValues.push_back("FALSE");
    }
    columns.push_back("TriggeredUpdate");
    if (interfaceStatus.m_triggeredUpdate)
    {
       newValues.push_back("TRUE");
    }
    else
    {
        newValues.push_back("FALSE");
    }

    InsertValues(db, "INTERFACE_Status", columns, newValues);
}


void STATSDB_HandleTransAggregateTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    node->partitionData->stats->Aggregate(node->partitionData);

    //if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.transportBridge == NULL)
        {
            node->partitionData->stats->global.transportBridge =
                 new STAT_GlobalTransportStatisticsBridge(
                    &node->partitionData->stats->global.transportAggregate,
                    node->partitionData);
        }
        // Compose and Insert SQL
        AddInsertQueryToBuffer(db, node->partitionData->stats->global.transportBridge->
            composeGlobalTransportStatisticsInsertSQLString(node, node->partitionData));
    }
}

////////////////////////////////////////////////////////////////////////
// Aggregate Table Insertion
////////////////////////////////////////////////////////////////////////
void STATSDB_HandleAppAggregateTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Pointer accessing appBridge
    STAT_GlobalAppStatisticsBridge *appBridge =
        node->partitionData->stats->global.appBridge;

    node->partitionData->stats->Aggregate(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (appBridge == NULL)
        {
            node->partitionData->stats->global.appBridge
                = new STAT_GlobalAppStatisticsBridge(
                                node->partitionData->stats->global.appAggregate,
                                node->partitionData);

            appBridge = node->partitionData->stats->global.appBridge;
        }
        else
        {
            appBridge->copyFromGlobalApp(
                        node->partitionData->stats->global.appAggregate);
        }
        // Compose SQL string
        AddInsertQueryToBuffer(db,
            appBridge->composeGlobalAppStatisticsInsertSQLString(node,
                                                     node->partitionData));
    }
}

void STATSDB_HandleAppAggregateTableUpdate(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    clocktype interval = db->statsAggregateTable->aggregateInterval;
    Int64 intervalIndex = 0;

    if (node->partitionData->partitionId == 0)
    {
        // Pointer accessing appBridge
        STAT_GlobalAppStatisticsBridge* appBridge =
            node->partitionData->stats->global.appBridge;
        if (appBridge == NULL)
        {
            node->partitionData->stats->global.appBridge
                = new STAT_GlobalAppStatisticsBridge(
                                node->partitionData->stats->global.appAggregate,
                                node->partitionData);

            appBridge = node->partitionData->stats->global.appBridge;
        }
        else
        {
            appBridge->copyFromGlobalApp(
                        node->partitionData->stats->global.appAggregate);
        }

        while (intervalIndex < db->statsAggregateTable->mcrSize)
        {
            // Compose SQL string
            AddUpdateQueryToBuffer(db,
                appBridge->composeGlobalAppStatisticsUpdateSQLString(
                    node,
                    node->partitionData,
                    intervalIndex,
                    interval));
            intervalIndex++;
            interval = interval + db->statsAggregateTable->aggregateInterval;
            if (interval > node->partitionData->maxSimClock)
            {
                interval = node->partitionData->maxSimClock;
            }
        }
    }
}

// Network Aggregate table update
void STATSDB_HandleNetworkAggregateTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Pointer accessing appBridge
    STAT_GlobalNetStatisticsBridge *netBridge =
        node->partitionData->stats->global.netBridge;

    node->partitionData->stats->Aggregate(node->partitionData);

    //if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (netBridge == NULL)
        {
            node->partitionData->stats->global.netBridge
                = new STAT_GlobalNetStatisticsBridge(
                                node->partitionData->stats->global.netAggregate,
                                node->partitionData);

            netBridge = node->partitionData->stats->global.netBridge;
        }
        else
        {
            netBridge->copyFromGlobalNet(
                        node->partitionData->stats->global.netAggregate);
        }
        // Compose SQL string
        AddInsertQueryToBuffer(db,
            netBridge->composeGlobalNetStatisticsInsertSQLString(node,
                                                     node->partitionData));
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleMacAggregateTableInsert
// PURPOSE  : Insert a new row into the MAC_Aggregate table.
// PARAMETERS :
// + node : current node
// + macParam : a structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleMacAggregateTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    node->partitionData->stats->Aggregate(node->partitionData);

    //if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.macBridge == NULL)
        {
            node->partitionData->stats->global.macBridge =
                 new STAT_GlobalMacStatisticsBridge(
                    &node->partitionData->stats->global.macAggregate,
                    node->partitionData);
        }
        // Compose and Insert SQL
        AddInsertQueryToBuffer(db, node->partitionData->stats->global.macBridge->
            composeGlobalMacStatisticsInsertSQLString(node, node->partitionData));
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandlePhyAggregateTableInsert
// PURPOSE  : Insert a new row into the PHY_Aggregate table.
// PARAMETERS :
// + node : current node
// + phyParam : a structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandlePhyAggregateTableInsert(
    Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL || !db->statsAggregateTable->createPhyAggregateTable)
    {
        return;
    }
    node->partitionData->stats->Aggregate(node->partitionData);

    //if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.phyBridge == NULL)
        {
            node->partitionData->stats->global.phyBridge
                = new STAT_GlobalPhysicalStatisticsBridge(
                                &node->partitionData->stats->global.phyAggregate,
                                node->partitionData);
        }

        // Compose and exectue insert SQL string
        AddInsertQueryToBuffer(db, node->partitionData->stats->global.phyBridge->composeGlobalPhysicalStatisticsInsertSQLString(
            node, node->partitionData));
    }
}

// Summary Table update
void STATSDB_HandleAppSummaryTableInsert(Node* node)
{
    std::vector<std::string> insertList;
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Pointer accessing appBridge
    STAT_AppSummaryBridge *appSummaryBridge =
        node->partitionData->stats->global.appSummaryBridge;

    node->partitionData->stats->SummarizeApp(node->partitionData);
    // need to wait here until all partitions have completed task.
    // brute force:
    //    sleep(5);  // TODO: TW use a synchronization function

    if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (appSummaryBridge == NULL)
        {
            node->partitionData->stats->global.appSummaryBridge
                = new STAT_AppSummaryBridge(
                    node->partitionData->stats->global.appUnicastSummary,
                    node->partitionData->stats->global.appMulticastSummary,
                    node->partitionData);

            appSummaryBridge =
                        node->partitionData->stats->global.appSummaryBridge;
        }
        else
        {
            appSummaryBridge->copyFromGlobalAppSummary(
                    node->partitionData->stats->global.appUnicastSummary,
                    node->partitionData->stats->global.appMulticastSummary);
        }
        // Compose SQL strings
        appSummaryBridge->composeAppSummaryInsertSQLString(
                                                    node,
                                                    node->partitionData,
                                                    &insertList,
                                                    STAT_Unicast);

        appSummaryBridge->composeAppSummaryInsertSQLString(
                                                    node,
                                                    node->partitionData,
                                                    &insertList,
                                                    STAT_Multicast);
        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}

void STATSDB_HandleMulticastAppSummaryTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;

    // Check if the Table exists.
    if (!db)
    {
        return;
    }

    node->partitionData->stats->SummarizeMulticastApp(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        std::vector<std::string> insertList;

        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.multicastAppSummaryBridge == NULL)
        {
                node->partitionData->stats->global.multicastAppSummaryBridge
                    = new STAT_MulticastAppSummaryBridge(
                        &node->partitionData->stats->global.appMulticastSessionSummary,
                        node->partitionData);
        }

        // Compose SQL strings
        node->partitionData->stats->global.multicastAppSummaryBridge->composeMutlicastAppSummaryInsertSQLString(
            node, node->partitionData, &insertList);

        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}

// Multicast Network Summary Table Insert
void STATSDB_HandleMulticastNetSummaryTableInsert(Node* node,
                        const StatsDBMulticastNetworkSummaryContent & stats)
{
    StatsDb* db = node->partitionData->statsDb;

    // Check if the Table exists.
    if (!db || !db->statsSummaryTable ||
        !db->statsSummaryTable->createMulticastNetSummaryTable)
    {
        // Table does not exist
        return;
    }
    std::vector<std::string> newValues;
    newValues.reserve(7);
    std::vector<std::string> columns;
    columns.reserve(7);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));

    columns.push_back("ProtocolType");
    if (!strcmp(stats.m_ProtocolType,"PIM-SM"))
    {
        newValues.push_back("PIM-SM");
    }
    else if (!strcmp(stats.m_ProtocolType,"PIM-DM"))
    {
        newValues.push_back("PIM-DM");
    }
    else if (!strcmp(stats.m_ProtocolType,"MOSPF"))
    {
        newValues.push_back("MOSPF");
    }
    else if (!strcmp(stats.m_ProtocolType,"OTHER"))
    {
        newValues.push_back("OTHER");
    }
    else
    {
        newValues.push_back("");
    }
    columns.push_back("DataSent");
    newValues.push_back(STATSDB_IntToString(stats.m_NumDataSent));
    columns.push_back("DataReceived");
    newValues.push_back(STATSDB_IntToString(stats.m_NumDataRecvd));
    columns.push_back("DataForwarded");
    newValues.push_back(STATSDB_IntToString(stats.m_NumDataForwarded));
    columns.push_back("DataDiscarded");
    newValues.push_back(STATSDB_IntToString(stats.m_NumDataDiscarded));

    InsertValues(db, "MULTICAST_NETWORK_Summary", columns, newValues);
}

// Summary Table update
// Transport Summary
void STATSDB_HandleTransSummaryTableInsert(Node *node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    node->partitionData->stats->SummarizeTransport(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        std::vector<std::string> insertList;

        // Would enter here only once(during first time of insertion)
        //TCP first
        if (node->partitionData->stats->global.transportSummaryBridge == NULL)
        {
                node->partitionData->stats->global.transportSummaryBridge
                    = new STAT_TransportSummaryBridge(
                        &node->partitionData->stats->global.transportSummary,
                        node->partitionData);
        }

        // Compose SQL strings
        node->partitionData->stats->global.transportSummaryBridge->composeTransportSummaryInsertSQLString(
            node, node->partitionData, &insertList);

        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}

// Network Summary Table Insert
void STATSDB_HandleNetworkSummaryTableInsert(Node* node)
{
    std::vector<std::string> insertList;
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    STAT_NetSummaryBridge *netSummaryBridge =
        node->partitionData->stats->global.netSummaryBridge;

    node->partitionData->stats->SummarizeNet(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (netSummaryBridge == NULL)
    {
            node->partitionData->stats->global.netSummaryBridge
                = new STAT_NetSummaryBridge(
                    node->partitionData->stats->global.netSummary,
                    node->partitionData);

            netSummaryBridge =
                        node->partitionData->stats->global.netSummaryBridge;
    }
    else
    {
            netSummaryBridge->copyFromGlobalNetSummary(
                    node->partitionData->stats->global.netSummary);
    }
        // Compose SQL strings
        netSummaryBridge->composeNetSummaryInsertSQLString(
                                                    node,
                                                    node->partitionData,
                                                    &insertList);
        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}

// Mac Summary Table Insert
void STATSDB_HandleMacSummaryTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    node->partitionData->stats->SummarizeMac(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        std::vector<std::string> insertList;

        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.macSummaryBridge == NULL)
        {
                node->partitionData->stats->global.macSummaryBridge
                    = new STAT_MacSummaryBridge(
                        &node->partitionData->stats->global.macSummary,
                        node->partitionData);
        }

        // Compose SQL strings
        node->partitionData->stats->global.macSummaryBridge->composeMacSummaryInsertSQLString(
            node, node->partitionData, &insertList);

        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleQueueAggregateTableInsert
// PURPOSE  : Prepare the SQL INSERT statement for a row in the
//            QUEUE_Aggregate table
// PARAMETERS :
// + node : current node
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleQueueAggregateTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL || !db->statsAggregateTable->createQueueAggregateTable)
    {
        return;
    }
    node->partitionData->stats->Aggregate(node->partitionData);

    //if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.queueBridge == NULL)
        {
            node->partitionData->stats->global.queueBridge
                = new STAT_GlobalQueueStatisticsBridge(
                                &node->partitionData->stats->global.queueAggregate,
                                node->partitionData);
        }

        // Compose and exectue insert SQL string
        AddInsertQueryToBuffer(db, node->partitionData->stats->global.queueBridge->composeGlobalQueueStatisticsInsertSQLString(
            node, node->partitionData));
    }
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleQueueSummaryTableInsert
// PURPOSE  : Prepare the SQL INSERT statement for a row in the
//            QUEUE_Summary table
// PARAMETERS :
// + node : current node
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleQueueSummaryTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    node->partitionData->stats->SummarizeQueue(node->partitionData);

    //if (node->partitionData->partitionId == 0)
    {
        std::vector<std::string> insertList;

        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.queueSummaryBridge == NULL)
        {
                node->partitionData->stats->global.queueSummaryBridge
                    = new STAT_QueueSummaryBridge(
                        &node->partitionData->stats->global.queueSummary,
                        node->partitionData);
        }

        // Compose SQL strings
        node->partitionData->stats->global.queueSummaryBridge->composeQueueSummaryInsertSQLString(
            node, node->partitionData, &insertList);

        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleQueueStatusTableInsertion
// PURPOSE  : Prepare the SQL INSERT statement for a row in the
//            QUEUE_Status table
// PARAMETERS :
// + node : current node
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleQueueStatusTableInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    node->partitionData->stats->SummarizeQueue(node->partitionData);

    //if (node->partitionData->partitionId == 0)
    {
        std::vector<std::string> insertList;

        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.queueStatusBridge == NULL)
        {
                node->partitionData->stats->global.queueStatusBridge
                    = new STAT_QueueStatusBridge(
                        &node->partitionData->stats->global.queueSummary,
                        node->partitionData);
        }

        // Compose SQL strings
        node->partitionData->stats->global.queueStatusBridge->composeQueueStatusInsertSQLString(
            node, node->partitionData, &insertList);

        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandlePhySummaryTableInsert
// PURPOSE  : Prepare the SQL INSERT statement for a row in the
//            PHY_Summary table
// PARAMETERS :
// + node : current node
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandlePhySummaryTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL || !db->statsSummaryTable->createPhySummaryTable)
    {
        return;
    }

    node->partitionData->stats->SummarizePhy(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        std::vector<std::string> insertList;

        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.phySummaryBridge == NULL)
        {
                node->partitionData->stats->global.phySummaryBridge
                    = new STAT_PhySummaryBridge(
                        &node->partitionData->stats->global.phySummary,
                        node->partitionData);
        }

        // Compose SQL strings
        node->partitionData->stats->global.phySummaryBridge->composePhysicalSummaryInsertSQLString(
            node,
            node->partitionData,
            &insertList);

        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandlePhySummaryTableInsert
// PURPOSE  : Prepare the SQL INSERT statement for a row in the
//            PHY_Summary table
// PARAMETERS :
// + node : current node
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandlePhySummaryTableInsert(Node* node,
    const StatsDBPhySummaryParam &phyParam)
{
    char buf[MAX_STRING_LENGTH];
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    std::vector<std::string> newValues;
    newValues.reserve(12);
    std::vector<std::string> columns;
    columns.reserve(12);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("SenderID");
    newValues.push_back(STATSDB_IntToString(phyParam.m_SenderId));
    columns.push_back("ReceiverID");
    newValues.push_back(STATSDB_IntToString(phyParam.m_RecieverId));
    // channel index is always -1 for link,satcom

    if (phyParam.m_ChannelIndex != -1)
    {
        columns.push_back("ChannelIndex");
        newValues.push_back(STATSDB_IntToString(phyParam.m_ChannelIndex));
    }
    columns.push_back("PhyIndex");
    newValues.push_back(STATSDB_IntToString(phyParam.m_PhyIndex));
    columns.push_back("NumRcvdSignals");
    newValues.push_back(STATSDB_UInt64ToString(phyParam.m_NumSignals));
    columns.push_back("NumRcvdErrorSignals");
    newValues.push_back(STATSDB_UInt64ToString(phyParam.m_NumErrorSignals));

    //utilization will be zero only for the satellite node of satcom.
    //In the case when no data is sent also it can be zero, but
    //in this case, there will be no entry for that sender/receiver pair

    if (phyParam.m_Utilization != 0)
    {
        columns.push_back("Utilization");
        sprintf(buf, "%.10f", phyParam.m_Utilization);
        newValues.push_back(buf);
    }

    if (!phyParam.m_AvgInterference.isNULL())
    {
        columns.push_back("AverageInterference");
        newValues.push_back(phyParam.m_AvgInterference.getStr());
    }

    if (!phyParam.m_Delay.isNULL())
    {
        columns.push_back("AverageDelay");
        newValues.push_back(phyParam.m_Delay.getStr());
    }
    if (!phyParam.m_PathLoss.isNULL())
    {
        columns.push_back("AveragePathLoss");
        newValues.push_back(phyParam.m_PathLoss.getStr());
    }
    if (!phyParam.m_SignalPower.isNULL())
    {
        columns.push_back("AverageSignalPower");
        newValues.push_back(phyParam.m_SignalPower.getStr());
    }

    InsertValues(db, "PHY_Summary", columns, newValues);
}

void STATSDB_HandleAppEventsTableUpdate(Node* node,
                                        void* /*data*/,
                                        const StatsDBAppEventParam & appParam)
{
    // In this table we insert the application layer
    // content on to the database.

    StatsDb* db = NULL;
    db = node->partitionData->statsDb;
    StatsDBAppEventContent *appEvent = db->statsAppEvents;

    if (appEvent->multipleValues)
    {
        if (db->engineType == UTIL::Database::dbMariaDB)
        {
            std::string bufferForAppEventsTbColsList;
            char buf[MAX_STRING_LENGTH] = "";

            sprintf(buf,
                    "(" STATSDB_TIME_FORMAT ",%d,%d",
                    ((double) node->getNodeTime() / SECOND),
                    node->nodeId,
                    appParam.m_SessionInitiator);
            bufferForAppEventsTbColsList += buf;

            if (appParam.m_ReceiverId == 0 || appParam.m_ReceiverId == -1)
            {
                bufferForAppEventsTbColsList += ",0";
            }
            else
            {
                sprintf(buf, ", %d", appParam.m_ReceiverId);
                bufferForAppEventsTbColsList += buf;
            }

            if (appParam.m_TargetAddr.isNULL())
            {
                bufferForAppEventsTbColsList += ",NULL";
            }
            else
            {
                bufferForAppEventsTbColsList += ",'" + appParam.m_TargetAddr.getStr() + "'";
            }

            bufferForAppEventsTbColsList += std::string(",'") + std::string(appParam.MessageId()) + std::string("','") +
              appParam.m_MsgSize.getStr() + std::string("','") + appParam.m_EventType + std::string("'");

            if (!appParam.m_SessionId.isNULL() && appEvent->isSession)
            {
                bufferForAppEventsTbColsList += ",'" + appParam.m_SessionId.getStr() + "'";
            }
            else
            {
                bufferForAppEventsTbColsList += ",NULL";
            }

            sprintf(buf,
                    ",'%s','%s'",
                    appParam.m_ApplicationType,
                    appParam.m_ApplicationName);
            bufferForAppEventsTbColsList += buf;

            if (appEvent->isMsgSeqNum)
            {
                if (!appParam.m_MsgSeqNum.isNULL())
                {
                    bufferForAppEventsTbColsList += ",'" + appParam.m_MsgSeqNum.getStr() + "'";
                }
                else
                {
                    bufferForAppEventsTbColsList += ",NULL";
                }
            }

            if (appEvent->isSocketInterfaceMsgIds)
            {
                if (!appParam.m_SocketInterfaceMsgId1.isNULL())
                {
                    bufferForAppEventsTbColsList += ",'" + appParam.m_SocketInterfaceMsgId1.getStr() + "','" +
                                                           appParam.m_SocketInterfaceMsgId2.getStr() + "'";
                }
                else
                {
                    bufferForAppEventsTbColsList += ",NULL,NULL";
                }
            }

            if (appEvent->isPriority)
            {
                if (!appParam.m_Priority.isNULL() && appEvent->isPriority)
                {
                    bufferForAppEventsTbColsList += ",'" + appParam.m_Priority.getStr() + "'";
                }
                else
                {
                    bufferForAppEventsTbColsList += ",NULL";
                }
            }

            if (appEvent->isMsgFailureType)
            {
                if (!appParam.m_MsgFailureType.isNULL())
                {
                    bufferForAppEventsTbColsList += ",'" + appParam.m_MsgFailureType.getStr() + "'";
                }
                else
                {
                    bufferForAppEventsTbColsList += ",NULL";
                }
            }

            if (appEvent->isDelay)
            {
                if (!appParam.m_Delay.isNULL())
                {
                    sprintf(buf, "," STATSDB_TIME_FORMAT, (double) appParam.m_Delay.get() / SECOND);
                    bufferForAppEventsTbColsList += buf;
                }
                else
                {
                    bufferForAppEventsTbColsList += ",NULL";
                }
            }

            if (appEvent->isJitter)
            {
                if (!appParam.m_Jitter.isNULL())
                {
                    sprintf(buf, "," STATSDB_TIME_FORMAT, (double) appParam.m_Jitter.get() / SECOND);
                    bufferForAppEventsTbColsList += buf;
                }
                else
                {
                    bufferForAppEventsTbColsList += ",NULL";
                }
            }



            bufferForAppEventsTbColsList += ")";

            Int32 requiredBytes = (Int32)bufferForAppEventsTbColsList.length();

            if (db->appEventsBytesUsed == 0)
            {
                // in this case, there is no stored app events
                // strings
                requiredBytes += (Int32)appEvent->appEventsTbColsName.length();
            }
            else
            {
                requiredBytes += 1; // strlen(",");
            }

            if (requiredBytes + 1 >
                appEvent->bufferSizeInBytes - db->appEventsBytesUsed)
            {
                ERROR_Assert(requiredBytes + 1 <= appEvent->bufferSizeInBytes,
                    "STATS-DB-APPLICATION-EVENTS-BUFFER-SIZE"
                    "is too small for the statement. \n");

                // if coming here, then there must be a stored string
                // to insert.
                // insert the stored string first
                AddInsertQueryToBuffer(db, db->appEventsString);

                db->appEventsBytesUsed = 0;
                // minus one because "," was accounted in the requiredBytes
                // earlier since the stored string was sent already, the ","
                // is not needed for now
                requiredBytes += (Int32)appEvent->appEventsTbColsName.length() - 1;
            }

            if (db->appEventsBytesUsed)
            {
                db->appEventsString += ",";
            }
            else
            {
                db->appEventsString = appEvent->appEventsTbColsName;
            }

            db->appEventsString += bufferForAppEventsTbColsList;
            db->appEventsBytesUsed += requiredBytes;
            // keep in the buffer
            return;
        }
    }
    // Following code is executed in the following cases:
    // 1) With NATIVE MariaDB and appEvent->multipleValues is FALSE.
    // 2) With SQLlite.
    std::vector<std::string> newValues;
    newValues.reserve(18);
    std::vector<std::string> columns;
    columns.reserve(18);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("SessionInitiator");
    newValues.push_back(STATSDB_IntToString(appParam.m_SessionInitiator));
    columns.push_back("ReceiverId");
    if (appParam.m_ReceiverId == 0 || appParam.m_ReceiverId == -1)
    {
        newValues.push_back("0");
    }
    else
    {
        newValues.push_back(STATSDB_IntToString(appParam.m_ReceiverId));
    }
    if (!appParam.m_TargetAddr.isNULL())
    {
       columns.push_back("ReceiverAddress");
       newValues.push_back(appParam.m_TargetAddr.getStr());
    }
    columns.push_back("MessageId");
    newValues.push_back(appParam.MessageId());
    columns.push_back("Size");
    newValues.push_back(appParam.m_MsgSize.getStr());
    columns.push_back("EventType");
    newValues.push_back(appParam.m_EventType);

    if (!appParam.m_MsgSeqNum.isNULL() && appEvent->isMsgSeqNum)
    {
        columns.push_back("MessageSeqNum");
        newValues.push_back(appParam.m_MsgSeqNum.getStr());
    }
    if (!appParam.m_SessionId.isNULL() && appEvent->isSession)
    {
        columns.push_back("SessionId");
        newValues.push_back(appParam.m_SessionId.getStr());
    }

    columns.push_back("ApplicationType");
    newValues.push_back(std::string(appParam.m_ApplicationType));

    columns.push_back("ApplicationName");
    newValues.push_back(std::string(appParam.m_ApplicationName));

    if (!appParam.m_Priority.isNULL() && appEvent->isPriority)
    {
        columns.push_back("Priority");
        newValues.push_back(appParam.m_Priority.getStr());
    }
    if (!appParam.m_MsgFailureType.isNULL() && appEvent->isMsgFailureType)
    {
        columns.push_back("MessageFailureType");
        newValues.push_back(appParam.m_MsgFailureType.getStr());
    }
    if (!appParam.m_Delay.isNULL() && appEvent->isDelay)
    {
        columns.push_back("Delay");
        newValues.push_back(
                STATSDB_DoubleToString((double) appParam.m_Delay.get() / SECOND));
    }
    if (!appParam.m_Jitter.isNULL() && appEvent->isJitter)
    {
        columns.push_back("Jitter");
        newValues.push_back(
                STATSDB_DoubleToString((double) appParam.m_Jitter.get() / SECOND));
    }
    if (!appParam.m_SocketInterfaceMsgId1.isNULL() && appEvent->isSocketInterfaceMsgIds)
    {
        columns.push_back("SocketInterfaceMessageId1");
        newValues.push_back(appParam.m_SocketInterfaceMsgId1.getStr());
        columns.push_back("SocketInterfaceMessageId2");
        newValues.push_back(appParam.m_SocketInterfaceMsgId2.getStr());
    }

    InsertValues(db, "APPLICATION_Events", columns, newValues);
}


void STATSDB_HandleNetworkEventsTableUpdate(Node* node,
        void* /*data*/,
        const StatsDBNetworkEventParam &networkParam,
        Message *msg,
        const char *failure,
        BOOL failureSpecified,
        const char *eventType)
{
    char senderAddr[MAX_STRING_LENGTH];
    char receiverAddr[MAX_STRING_LENGTH];

    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    StatsDBNetworkEventContent * ipEvent = db->statsNetEvents;

    StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);


    ERROR_Assert(mapParamInfo,
        "Error in HandleStatsDBNetworkEventsInsertion.");

    // Now to add the values for these parameters.

    double timeVal = (double) node->getNodeTime() / SECOND;
    IO_ConvertIpAddressToString(networkParam.m_SenderAddr, senderAddr);
    IO_ConvertIpAddressToString(networkParam.m_ReceiverAddr, receiverAddr);

    // In this table we insert the network layer content on to the database.
    char buf[DB_LONG_BUFFER_LENGTH];
    size_t offset = sprintf(buf,
            "%s(" STATSDB_TIME_FORMAT ",%d,'%s','%s','%s',%d,'%s'",
            db->statsNetEvents->networkEventsTbColsName,
            timeVal,
            node->nodeId,
            mapParamInfo->msgId,
            senderAddr,
            receiverAddr,
            networkParam.m_MsgSize,
            eventType);

    // Now to add the optional stuff
    if (!networkParam.m_InterfaceIndex.isNULL() && ipEvent->isInterfaceIndex)
    {
        int idx = networkParam.m_InterfaceIndex.get();
        if (idx >= 0)
            offset += sprintf(buf + offset, ",%d", idx);
         else if (idx == CPU_INTERFACE)
            offset += sprintf(buf + offset, ",'CPU'");
        else
            offset += sprintf(buf + offset, ",'BACKPLANE'");
    }
    else
    {
      offset += sprintf(buf + offset, ",null");
    }

    if (!networkParam.m_MsgSeqNum.isNULL() && ipEvent->isMsgSeqNum)
        offset += sprintf(buf + offset, ",%d", networkParam.m_MsgSeqNum.get());
    else
        offset += sprintf(buf + offset, ",null");

    if (!networkParam.m_HeaderSize.isNULL() && ipEvent->isControlSize)
        offset += sprintf(buf + offset, ", %d", networkParam.m_HeaderSize.get());
    else
        offset += sprintf(buf + offset, ",null");

    if (!networkParam.m_PktType.isNULL() && ipEvent->isPktType)
    {
        if (networkParam.m_PktType.get() == StatsDBNetworkEventParam::DATA)
            offset += sprintf(buf + offset, ",'%s'", "Data");
        else
            offset += sprintf(buf + offset, ",'%s'", "Control");
    }
    else
    {
        offset += sprintf(buf + offset, ",null");
    }

    if (!networkParam.m_ProtocolType.isNULL() && ipEvent->isProtocolType)
    {
        std::string protocolType;
        NetworkIpConvertIpProtocolNumToString(networkParam.m_ProtocolType.get(), &protocolType);
        offset += sprintf(buf + offset, ",'%s'", protocolType.c_str());
    }
    else
    {
        offset += sprintf(buf + offset, ",null");
    }

    if (!networkParam.m_Priority.isNULL() && ipEvent->isPriority)
        offset += sprintf(buf + offset, ",%d", networkParam.m_Priority.get());
    else
        offset += sprintf(buf + offset, ",null");


    if (failureSpecified && ipEvent->isPktFailureType)
        offset += sprintf(buf + offset, ",'%s'", failure);
    else
        offset += sprintf(buf + offset, ",null");

    if (!networkParam.m_HopCount.isNULL() && ipEvent->isHopCount)
        offset += sprintf(buf + offset, "," STATSDB_DOUBLE_FORMAT, networkParam.m_HopCount.get());
    else
        offset += sprintf(buf + offset, ",null");

    sprintf(buf + offset, ")");
    dataBase.push_back(db, buf);

}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandlePhyEventsTableInsert
// PURPOSE  : Insert a new row into the PHY_Events table
// PARAMETERS :
// + node : current node
// + phyParam : the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandlePhyEventsTableInsert(Node* node,
                                        const StatsDBPhyEventParam &phyParam)
{
    // In this table we insert the phy layer content on to the database.
    StatsDBPhyEventContent *phyEvent;

    StatsDb* db = NULL;
    db = node->partitionData->statsDb;

    if (!db->statsEventsTable->createPhyEventsTable)
    {
        return;
    }
    std::vector<std::string> columns;
    columns.reserve(12);
    std::vector<std::string> newValues;
    newValues.reserve(12);

    columns.push_back("Timestamp");
    newValues.push_back(STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("MessageId");
    newValues.push_back(phyParam.m_MessageId);
    columns.push_back("PhyIndex");
    newValues.push_back(STATSDB_IntToString(phyParam.m_PhyIndex));
    columns.push_back("Size");
    newValues.push_back(STATSDB_IntToString(phyParam.m_MsgSize));
    columns.push_back("EventType");
    newValues.push_back(phyParam.m_EventType);

    phyEvent = db->statsPhyEvents;

    if (!phyParam.m_ChannelIndex.isNULL() && phyEvent->isChannelIndex)
    {
        columns.push_back("ChannelIndex");
        newValues.push_back(phyParam.m_ChannelIndex.getStr());
    }
    if (!phyParam.m_ControlSize.isNULL() && phyEvent->isControlSize)
    {
        columns.push_back("OverheadSize");
        newValues.push_back(phyParam.m_ControlSize.getStr());
    }
    if (!phyParam.m_Interference.isNULL() && phyEvent->isInterference)
    {
        columns.push_back("Interference");
        newValues.push_back(phyParam.m_Interference.getStr());
    }
    if (!phyParam.m_MessageFailureType.isNULL() && phyEvent->isMessageFailureType)
    {
        columns.push_back("FailureType");
        newValues.push_back(phyParam.m_MessageFailureType.getStr());
    }
    if (!phyParam.m_PathLoss.isNULL() && phyEvent->isPathLoss)
    {
        columns.push_back("PathLoss");
        newValues.push_back(phyParam.m_PathLoss.getStr());
    }
    if (!phyParam.m_SignalPower.isNULL() && phyEvent->isSignalPower)
    {
        columns.push_back("SignalPower");
        newValues.push_back(phyParam.m_SignalPower.getStr());
    }

    InsertValues(db, "PHY_Events", columns, newValues);
}

void STATSDB_HandleMacEventsTableInsert(Node* node,
        void* /*data*/,
        const StatsDBMacEventParam &macParam)
{
    // In this table we insert the network layer content on to the database.
    StatsDb* db = NULL;
    db = node->partitionData->statsDb;

    std::vector<std::string> newValues;
    newValues.reserve(17);
    std::vector<std::string> columns;
    columns.reserve(17);

    columns.push_back("Timestamp");
    newValues.push_back(STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("MessageId");
    newValues.push_back(macParam.m_MessageId);
    columns.push_back("InterfaceIndex");
    newValues.push_back(STATSDB_IntToString(macParam.m_InterfaceIndex));
    columns.push_back("MessageSize");
    newValues.push_back(STATSDB_IntToString(macParam.m_MsgSize));
    columns.push_back("EventType");
    newValues.push_back(macParam.m_EventType);

    if (!macParam.m_MsgSeqNum.isNULL())
    {
        columns.push_back("SequenceNumber");
        newValues.push_back(macParam.m_MsgSeqNum.getStr());
    }
    if (!macParam.m_ChannelIndex.isNULL())
    {
        columns.push_back("ChannelIndex");
        newValues.push_back(macParam.m_ChannelIndex.getStr());
    }
    if (!macParam.m_FailureType.isNULL())
    {
        columns.push_back("FailureType");
        newValues.push_back(macParam.m_FailureType.getStr());
    }
    if (!macParam.m_HeaderSize.isNULL())
    {
        columns.push_back("OverheadSize");
        newValues.push_back(macParam.m_HeaderSize.getStr());
    }
    if (!macParam.m_FrameType.isNULL())
    {
        columns.push_back("FrameType");
        newValues.push_back(macParam.m_FrameType.getStr());
    }
    if (!macParam.m_DstAddrStr.isNULL())
    {
        columns.push_back("DestAddress");
        newValues.push_back(macParam.m_DstAddrStr.getStr());
    }

    if (!macParam.m_SrcAddrStr.isNULL())
    {
        columns.push_back("SrcAddress");
        newValues.push_back(macParam.m_SrcAddrStr.getStr());
    }

    InsertValues(db, "MAC_Events", columns, newValues);
}

void HandleStatsDBMessageIdMappingInsert(Node *node,
    const std::string &MsgIdIn,
    const std::string &MsgIdOut,
    const std::string &protocol)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL ||
        db->statsEventsTable == NULL ||
        !db->statsEventsTable->createPhyEventsTable)
    {
        return;
    }

    std::vector<std::string> newValues;
    newValues.reserve(5);
    std::vector<std::string> columns;
    columns.reserve(5);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("MessageIdIn");
    newValues.push_back(MsgIdIn);
    columns.push_back("MessageIdOut");
    newValues.push_back(MsgIdOut);
    columns.push_back("Protocol");
    newValues.push_back(protocol);

    InsertValues(db, "Message_Id_Mapping", columns, newValues);
}

void HandleStatsDBMessageIdMappingInsert(
         Node *node,
         const Message* oldMsg,
         const Message* newMsg,
         const std::string &protocol)
{
    StatsDBMappingParam* inMapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo(oldMsg, INFO_TYPE_StatsDbMapping);
    StatsDBMappingParam* outMapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo(newMsg, INFO_TYPE_StatsDbMapping);

    HandleStatsDBMessageIdMappingInsert(
        node,
        inMapParamInfo->msgId,
        outMapParamInfo->msgId,
        protocol);
}

void HandleStatsDBTransportEventsInsert(Node* node,
        Message * /*msg*/,
        const StatsDBTransportEventParam & transParam)
{
    // In this table we insert the network layer content on to the database.
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    StatsDBTransEventContent *transEvent = db->statsTransEvents;

    std::vector<std::string> newValues;
    newValues.reserve(12);
    std::vector<std::string> columns;
    columns.reserve(12);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("MessageId");
    newValues.push_back(transParam.m_MessageId);
    columns.push_back("Size");
    newValues.push_back(STATSDB_IntToString(transParam.m_MsgSize));

    if (transEvent->isSenderPort)
    {
        columns.push_back("SenderPort");
        newValues.push_back(STATSDB_IntToString(transParam.m_SenderPort));
    }
    if (transEvent->isReceiverPort)
    {
        columns.push_back("ReceiverPort");
        newValues.push_back(STATSDB_IntToString(transParam.m_ReceiverPort));
    }
    if (!transParam.m_MsgSeqNum.isNULL() && transEvent->isMsgSeqNum)
    {
        columns.push_back("MessageSeqNum");
        newValues.push_back(transParam.m_MsgSeqNum.getStr());
    }
    if (!transParam.m_ConnectionType.isNULL() && transEvent->isConnType)
    {
        columns.push_back("ConnectionType");
        newValues.push_back(transParam.m_ConnectionType.getStr());
    }
    if (!transParam.m_HeaderSize.isNULL() && transEvent->isHdrSize)
    {
        columns.push_back("OverheadSize");
        newValues.push_back(transParam.m_HeaderSize.getStr());
    }
    if (!transParam.m_Flags.isNULL() && transEvent->isFlags)
    {
        columns.push_back("SegmentType");
        newValues.push_back(transParam.m_Flags.getStr());
    }
    if (!transParam.m_EventType.isNULL() && transEvent->isEventType)
    {
        columns.push_back("EventType");
        newValues.push_back(transParam.m_EventType.getStr());
    }
    if (!transParam.m_FailureType.isNULL() && transEvent->isMsgFailureType)
    {
        columns.push_back("MessageFailureType");
        newValues.push_back(transParam.m_FailureType.getStr());
    }

    InsertValues(db, "TRANSPORT_Events", columns, newValues);
}

void STATSDB_HandleAppConnTableInsert(Node *node,
    const StatsDBAppConnParam * appConnParam)
{
    // In this table we insert the network layer content on to the database.
    StatsDb* db = NULL;
    db = node->partitionData->statsDb;
    std::vector<std::string> newValues;
    newValues.reserve(4);
    std::vector<std::string> columns;
    columns.reserve(4);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("SenderAddr");
    newValues.push_back(appConnParam->m_SrcAddress);
    columns.push_back("ReceiverAddr");
    newValues.push_back(appConnParam->m_DstAddress);
    columns.push_back("SessionId");
    newValues.push_back(STATSDB_IntToString(appConnParam->sessionId));

    InsertValues(db, "APPLICATION_Connectivity", columns, newValues);
}

void STATSDB_HandleTransConnTableInsert(Node *node,
    const StatsDBTransConnParam * transConnParam)
{
    // In this table we insert the network layer content on to the database.
    StatsDb* db = NULL;
    db = node->partitionData->statsDb;
    std::vector<std::string> newValues;
    newValues.reserve(5);
    std::vector<std::string> columns;
    columns.reserve(5);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("SenderAddr");
    newValues.push_back(transConnParam->m_SrcAddress);
    columns.push_back("SenderPort");
    newValues.push_back(STATSDB_IntToString(transConnParam->m_SrcPort));
    columns.push_back("ReceiverAddr");
    newValues.push_back(transConnParam->m_DstAddress);
    columns.push_back("ReceiverPort");
    newValues.push_back(STATSDB_IntToString(transConnParam->m_DstPort));

    InsertValues(db, "TRANSPORT_Connectivity", columns, newValues);
}

void STATSDB_HandleMulticastConnTableInsert(Node *node,
    StatsDBConnTable::MulticastConnectivity multicastConnParam)
{
    // In this table we insert the multicast connectivity content, that is,
    // the forwarding table content
    char destAddress[20];

    // the main stats db handle
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    // fill up our sql query
    std::vector<std::string> newValues;
    newValues.reserve(8);
    std::vector<std::string> columns;
    columns.reserve(8);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("SenderId");
    newValues.push_back(STATSDB_IntToString(multicastConnParam.nodeId));
    columns.push_back("DestAddr");
    // convert our addresses into strings
    IO_ConvertIpAddressToString(multicastConnParam.destAddr,destAddress);
    newValues.push_back(std::string(destAddress));
    columns.push_back("RootNodeType");
    newValues.push_back(std::string(multicastConnParam.rootNodeType));
    columns.push_back("RootNodeId");
    newValues.push_back(STATSDB_IntToString(multicastConnParam.rootNodeId));

    columns.push_back("OutgoingInterfaceIndex");
    if (multicastConnParam.outgoingInterface >= 0)
    {
        newValues.push_back(
            STATSDB_IntToString(multicastConnParam.outgoingInterface));
    }
    else
    {
        newValues.push_back("null");
    }
    columns.push_back("UpstreamNodeId");
    if (multicastConnParam.rootNodeId != multicastConnParam.nodeId)
    {
        newValues.push_back(
            STATSDB_IntToString(multicastConnParam.upstreamNeighborId));
    }
    else
    {
        newValues.push_back("null");
    }
    columns.push_back("UpstreamInterface");
    if (multicastConnParam.upstreamInterface >= 0)
    {
        newValues.push_back(
            STATSDB_IntToString(multicastConnParam.upstreamInterface));
    }
    else
    {
       newValues.push_back("null");
    }
    InsertValues(db, "MULTICAST_Connectivity", columns, newValues);
}

void STATSDB_HandleNetworkConnTableUpdate(Node* node,
        StatsDBNetworkConnParam networkParam)
{
    // In this table we insert the network connectivity content, that is,
    // the forwarding table content
    StatsDBNetworkConnContent *ipConn = NULL;
    StatsDb* db = NULL;
    db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    ipConn = db->statsNetConn;

    std::vector<std::string> newValues;
    newValues.reserve(9);
    std::vector<std::string> columns;
    columns.reserve(9);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("DestinationAddr");
    newValues.push_back(networkParam.m_DstAddress);
    columns.push_back("Cost");
    newValues.push_back(STATSDB_IntToString(networkParam.m_Cost));

    if (!networkParam.m_DstNetMask.isNULL() && ipConn->isDstMaskAddr)
    {
        columns.push_back("DestMaskAddr");
        newValues.push_back(networkParam.m_DstNetMask.getStr());
    }
    if (!networkParam.m_OutgoingIntIndex.isNULL() && ipConn->isOutgoingInterfaceIndex)
    {
        columns.push_back("OutgoingInterfaceIndex");
        newValues.push_back(networkParam.m_OutgoingIntIndex.getStr());
    }
    if (!networkParam.m_NextHopAddr.isNULL() && ipConn->isNextHopAddr)
    {
        columns.push_back("NextHopAddr");
        newValues.push_back(networkParam.m_NextHopAddr.getStr());
    }
    if (!networkParam.m_RoutingProtocolType.isNULL() && ipConn->isRoutingProtocol)
    {
        columns.push_back("RoutingProtocolType");
        newValues.push_back(networkParam.m_RoutingProtocolType.getStr());
    }
    if (!networkParam.m_AdminDistance.isNULL() && ipConn->isAdminDistance)
    {
        columns.push_back("AdminDistance");
        newValues.push_back(networkParam.m_AdminDistance.getStr());
    }

    InsertValues(db, "NETWORK_Connectivity", columns, newValues);
}

void STATSDB_HandleMacConnTableUpdate(Node *node,
                                      const StatsDBMacConnParam &macParam)
{
    // In this table we insert the phy connectivity content,
    StatsDb* db = NULL;
    db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }

    std::vector<std::string> newValues;
    newValues.reserve(5);
    std::vector<std::string> columns;
    columns.reserve(5);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("SenderId");
    newValues.push_back(STATSDB_IntToString(macParam.m_SenderId));
    columns.push_back("ReceiverId");
    newValues.push_back(STATSDB_IntToString(macParam.m_ReceiverId));
    columns.push_back("SenderInterfaceIndex");
    newValues.push_back(STATSDB_IntToString(macParam.m_InterfaceIndex));
    columns.push_back("ChannelIndex");
    newValues.push_back(macParam.channelIndex_str);

    InsertValues(db, "MAC_Connectivity", columns, newValues);
}

void STATSDB_HandlePhyConnTableUpdate(Node* node,
    const StatsDBPhyConnParam & phyParam)
{
    // In this table we insert the phy connectivity content,

    StatsDb* db = NULL;
    db = node->partitionData->statsDb;

    if (!db || !db->statsConnTable->createPhyConnTable)
    {
        return ;
    }
    if (node->partitionData->partitionId != 0)
    {
        return ;
    }
    if (node->partitionData->maxSimClock == node->getNodeTime()
        && db->statsConnTable->endSimulation == FALSE)
    {
        return ;
    }

    std::vector<std::string> newValues;
    newValues.reserve(11);
    std::vector<std::string> columns;
    columns.reserve(11);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) node->getNodeTime() / SECOND));
    columns.push_back("SenderId");
    newValues.push_back(STATSDB_IntToString(phyParam.m_SenderId));
    columns.push_back("ReceiverId");
    newValues.push_back(STATSDB_IntToString(phyParam.m_ReceiverId));
    columns.push_back("ChannelIndex");
    newValues.push_back(phyParam.m_ChannelIndex.getStr());
    columns.push_back("SenderPhyIndex");
    newValues.push_back(phyParam.m_PhyIndex.getStr());
    columns.push_back("ReceiverPhyIndex");
    newValues.push_back(STATSDB_IntToString(phyParam.m_ReceiverPhyIndex));

    columns.push_back("SenderListening");
    columns.push_back("ReceiverListening");
    columns.push_back("AntennaType");
    columns.push_back("BestAngle");
    columns.push_back("WorstAngle");
    if (isdigit(phyParam.m_ChannelIndex.get().c_str()[0]))
    {
        newValues.push_back(phyParam.senderListening ? "TRUE" : "FALSE");
        newValues.push_back(phyParam.receiverListening ? "TRUE" : "FALSE");

        if (phyParam.antennaType == ANTENNA_SWITCHED_BEAM)
        {
            newValues.push_back("SwitchedBeam");
        }
        else if (phyParam.antennaType == ANTENNA_STEERABLE)
        {
            newValues.push_back("Steerable");
        }
        else if (phyParam.antennaType == ANTENNA_PATTERNED)
        {
            newValues.push_back("Patterned");
        }
        else if (phyParam.antennaType == ANTENNA_OMNIDIRECTIONAL)
        {
            newValues.push_back("Omnidirectional");
        }
        else
        {
            newValues.push_back("None");
        }

        newValues.push_back("TRUE");
        newValues.push_back(phyParam.reachableWorst ? "TRUE" : "FALSE");
    }
    else
    {
        newValues.push_back("");
        newValues.push_back("");
        newValues.push_back("");
        newValues.push_back("");
        newValues.push_back("");
    }
    InsertValues(db, "PHY_Connectivity", columns, newValues);
}
