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

#ifndef _DBAPI_H_
#define _DBAPI_H_

#include <string>
#include <vector>
#include <iostream>
#include <list>

#include "main.h"
#include "fileio.h"
#include "node.h"
#include "db.h"

#ifdef ENTERPRISE_LIB
#include "db_multimedia.h"
#endif
#include "db_developer.h"

#include "db-core.h"

#define STATSDB_MAX_BUFFER_QUERY 10000
#define LARGE_INDEX 99999

#define STATSDB_DEFAULT_AGGREGATE_INTERVAL (600 * SECOND)
#define STATSDB_DEFAULT_SUMMARY_INTERVAL (600 * SECOND)
#define STATSDB_DEFAULT_STATUS_INTERVAL (600 * SECOND)

typedef UInt8 STATSDB_TABLE_TYPE;

#define STATSDB_DESCRIPTION_TABLE 1
#define STATSDB_NODE_DESCRIPTION_TABLE 2
#define STATSDB_QUEUE_DESCRIPTION_TABLE 3
#define STATSDB_SCHEDULER_DESCRIPTION_TABLE 4
#define STATSDB_SESSION_DESCRIPTION_TABLE 5
#define STATSDB_CONNECTION_DESCRIPTION_TABLE 6
#define STATSDB_INTERFACE_DESCRIPTION_TABLE 7
#define STATSDB_PHY_DESCRIPTION_TABLE 8

#define STATSDB_STATUS_TABLE 9

#define STATSDB_AGGREGATE_TABLE 14
#define STATSDB_APP_AGGREGATE_TABLE 15
#define STATSDB_TRANS_AGGREGATE_TABLE 17
#define STATSDB_NETWORK__AGGREGATE_TABLE 18
#define STATSDB_MAC_AGGREGATE_TABLE 19
#define STATSDB_PHY_AGGREGATE_TABLE 20

#define STATSDB_SUMMARY_TABLE 21

#define STATSDB_EVENTS_TABLE 26
#define STATSDB_APP_EVENTS_TABLE 27
#define STATSDB_TRANS_EVENTS_TABLE 29
#define STATSDB_NETWORK_EVENTS_TABLE 30
#define STATSDB_MAC_EVENTS_TABLE 31
#define STATSDB_PHY_EVENTS_TABLE 32
#define STATSDB_QUEUE_EVENTS_TABLE 33

#define STATSDB_CONNECTIVITY_TABLE 34
#define STATSDB_APP_CONNECTIVITY_TABLE 35
#define STATSDB_TRANSPORT_CONNECTIVITY_TABLE 36
#define STATSDB_NETWORK_CONNECTIVITY_TABLE 37
#define STATSDB_MAC_CONNECTIVITY_TABLE 38
#define STATSDB_PHY_CONNECTIVITY_TABLE 39
#define STATSDB_MULTICAST_CONNECTIVITY_TABLE 40

typedef UInt8 STATSDB_DescriptionTableNum;

#define STATSDB_NODE_DESCRIPTION_TABLE_NUMBER 1
#define STATSDB_QUEUE_DESCRIPTION_TABLE_NUMBER 2
#define STATSDB_SCHEDULER_DESCRIPTION_TABLE_NUMBER 3
#define STATSDB_SESSION_DESCRIPTION_TABLE_NUMBER 4
#define STATSDB_CONNECTION_DESCRIPTION_TABLE_NUMBER 5
#define STATSDB_INTERFACE_DESCRIPTION_TABLE_NUMBER 6
#define STATSDB_PHY_DESCRIPTION_TABLE_NUMBER 7

typedef UInt8 STATSDB_TableCategory;

#define STATSDB_DESCRIPTION_CATEGORY 1
#define STATSDB_STATUS_CATEGORY 2
#define STATSDB_AGGREGATE_CATEGORY 3
#define STATSDB_SUMMARY_CATEGORY 4
#define STATSDB_EVENTS_CATEGORY 5
#define STATSDB_CONNECTIVITY_CATEGORY 6

typedef UInt8 STATSDB_AggregateTableNum;

#define STATSDB_APP_AGGREGATE_TABLE_NUMBER 1
#define STATSDB_TRANS_AGGREGATE_TABLE_NUMBER 3
#define STATSDB_NETWORK_AGGREGATE_TABLE_NUMBER 4
#define STATSDB_MAC_AGGREGATE_TABLE_NUMBER 5
#define STATSDB_PHY_AGGREGATE_TABLE_NUMBER 6
#define STATSDB_QUEUE_AGGREGATE_TABLE_NUMBER 7

typedef UInt8 STATSDB_SummaryTableNum;

#define STATSDB_APP_SUMMARY_TABLE_NUMBER 1
#define STATSDB_TRANS_SUMMARY_TABLE_NUMBER 3
#define STATSDB_NETWORK_SUMMARY_TABLE_NUMBER 4
#define STATSDB_MAC_SUMMARY_TABLE_NUMBER 5
#define STATSDB_PHY_SUMMARY_TABLE_NUMBER 6
#define STATSDB_QUEUE_SUMMARY_TABLE_NUMBER 7

typedef UInt8 STATSDB_EventsTableNum;

#define STATSDB_APP_EVENTS_TABLE_NUMBER 1
#define STATSDB_TRANS_EVENTS_TABLE_NUMBER 3
#define STATSDB_NETWORK_EVENTS_TABLE_NUMBER 4
#define STATSDB_MAC_EVENTS_TABLE_NUMBER 5
#define STATSDB_PHY_EVENTS_TABLE_NUMBER 6
#define STATSDB_QUEUE_EVENTS_TABLE_NUMBER 7

typedef UInt8 STATSDB_CONNECTIVITY_TABLE_NUMBER;

#define STATSDB_APP_CONNECTIVITY_TABLE_NUMBER 1
#define STATSDB_TRANSPORT_CONNECTIVITY_TABLE_NUMBER 2
#define STATSDB_NETWORK_CONNECTIVITY_TABLE_NUMBER 3
#define STATSDB_MAC_CONNECTIVITY_TABLE_NUMBER 4
#define STATSDB_PHY_CONNECTIVITY_TABLE_NUMBER 5
#define STATSDB_MULTICAST_CONNECTIVITY_TABLE_NUMBER 6

typedef UInt8 STATSDB_StatusTableNum;

#define STATSDB_NODE_STATUS_TABLE_NUMBER 1
#define STATSDB_INTERFACE_STATUS_TABLE_NUMBER 2
#define STATSDB_MULTICAST_STATUS_TABLE_NUMBER 3
#define STATSDB_QUEUE_STATUS_TABLE_NUMBER 4

#define STATSDB_ADMIN_DISTANCE 1
#define STATSDB_APPLICATION_TYPE 2
#define STATSDB_CHANNEL_INDEX 3
#define STATSDB_CONNECTION_ID 4
#define STATSDB_CONNECTION_TYPE 5
#define STATSDB_CONTROL_SIZE 6
#define STATSDB_DELAY 7
#define STATSDB_DEST_NETWORK_MASK 8
#define STATSDB_EVENT_TYPE 9
#define STATSDB_FRAGMENT_ID 10
#define STATSDB_HOP_COUNT 11
#define STATSDB_INTERFACE_INDEX 12
#define STATSDB_INTERFACE_ADDRESS 13
#define STATSDB_INTERFACE_NAME 14
#define STATSDB_INTERFERENCE_POWER 15
#define STATSDB_JITTER 16
#define STATSDB_MAC_PROTOCOL 17
#define STATSDB_FAILURE_TYPE 18
#define STATSDB_MESSAGE_TYPE 19
#define STATSDB_MSG_SEQUENCE_NUM 20
#define STATSDB_META_DATA 21
#define STATSDB_NETWORK_PROTOCOL 22
#define STATSDB_NETWORK_TYPE 23
#define STATSDB_NEXT_HOP_ADDRESS 24
#define STATSDB_OUTGOING_INTERFACE_INDEX 25
#define STATSDB_PATHLOSS 26
#define STATSDB_PHY_INDEX 27
#define STATSDB_PRIORITY 28
#define STATSDB_PROTOCOL_TYPE 29
#define STATSDB_QUEUE_INDEX 30
#define STATSDB_QUEUE_DISCIPLINE 31
#define STATSDB_RECEIVER_ADDRESS 32
#define STATSDB_RECEIVER_PORT 33
#define STATSDB_ROUTING_PROTOCOL_TYPE 34
#define STATSDB_SENDER_ADDRESS 35
#define STATSDB_SENDER_PORT 36
#define STATSDB_SERVICE_TIME 37
#define STATSDB_SESSION_ID 38
#define STATSDB_SIGNAL_POWER 39
#define STATSDB_SIZE 40
#define STATSDB_SUBNET_MASK 41
#define STATSDB_TRANSPORT_PROTOCOL 42

#define STATSDB_MULTICAST_PROTOCOL 43
#define STATSDB_NODE_POSITION 44
#define STATSDB_NODE_VELOCITY 45
#define STATSDB_NODE_ACTIVE_STATE 46
#define STATSDB_NODE_DAMAGE_STATE 47
#define STATSDB_QUEUE_PRIORITY 48
#define STATSDB_QUEUE_TYPE 49
#define STATSDB_DATA_RETRAX 50
#define STATSDB_CONTROL_RETRAX 51
#define STATSDB_HEADER_FLAGS 52
#define STATSDB_TRANSPORT_SEQ_NUM 53

typedef UInt8 STATSDB_MetaDataType;

#define STATSDB_NodeMetaData 1
#define STATSDB_QueueMetaData 2
#define STATSDB_SchedulerMetaData 3
#define STATSDB_SessionMetaData 4
#define STATSDB_ConnectionMetaData 5
#define STATSDB_InterfaceMetaData 6
#define STATSDB_PhyMetaData 7

#ifdef ADDON_NGCNMS
class StatsDBAppSummaryParam ;
#endif

struct StatsDb
{
    BOOL createDbFile;
    UTIL::Database::DatabaseDriver* driver;
    PartitionData* partition;
    UTIL::Database::dbEngineType engineType;
    char statsDatabase[MAX_STRING_LENGTH];
    char storageEngine[MAX_STRING_LENGTH];
    StatsDBLevelSetting levelSetting;
    StatsDBTable* statsTable;
    StatsDBDescTable* statsDescTable;
    StatsDBInterfaceDescContent* statsInterfaceDesc;
    StatsDBQueueDescContent* statsQueueDesc;
    StatsDBSessionDescContent *statsSessionDesc;
    StatsDBConnectionDescContent *statsConnectionDesc;
    StatsDBSchedulerDescContent *statsSchedulerAlgo;

    StatsDBAggregateTable* statsAggregateTable;
    StatsDBAppAggregateContent* statsAppAggregate;
    StatsDBNetworkAggregateContent* statsNetAggregate;
    StatsDBMacAggregateContent* statsMacAggregate;
    StatsDBPhyAggregateContent* statsPhyAggregate;
    StatsDBTransportAggregateContent* statsTransAggregate;

    StatsDBSummaryTable* statsSummaryTable;
    StatsDBAppSummaryContent* statsAppSummary;
    StatsDBMulticastAppSummaryContent* statsMulticastAppSummary;
    StatsDBTransSummaryContent* statsTransSummary;

    StatsDBNetworkSummaryContent* statsNetSummary;
    StatsDBMacSummaryContent* statsMacSummary;
    StatsDBPhySummaryContent* statsPhySummary;

    StatsDBStatusTable* statsStatusTable;
    StatsDBNodeStatusContent* statsNodeStatus;

    StatsDBEventsTable* statsEventsTable;
    StatsDBAppEventContent* statsAppEvents;
    StatsDBNetworkEventContent* statsNetEvents;
    StatsDBMacEventContent* statsMacEvents;
    StatsDBPhyEventContent* statsPhyEvents;
    StatsDBExternalEventContent* statsExternalEvents;
    StatsDBTransEventContent* statsTransEvents;
    StatsDBConnTable* statsConnTable;
    StatsDBNetworkConnContent* statsNetConn;
    StatsDBMacConnContent* statsMacConn;
    StatsDBPhyConnContent* statsPhyConn;
    StatsDBLinkUtilizationTable* StatsDBLinkUtilTable;

    bool doEXataEvents() {
        if (this == NULL) return false;
        if (statsExternalEvents == NULL) return false;
        return statsExternalEvents->Use();
    }
    int maxQueryBuffer;

    /*---------------------------------*/
    StatsDb(): engineType(UTIL::Database::dbSqlite), queueDbPtr(0), networkEventsBytesUsed(0), appEventsBytesUsed(0),
        statsExternalEvents(0)
    {
        networkEventsString = NULL;
        memset(storageEngine, 0, sizeof(storageEngine));
    }
    ~StatsDb()
    {
        if (networkEventsString)
        {
            MEM_free(networkEventsString);
        }
    }
    StatsQueueDB::StatsQueueDb* queueDbPtr;
    /*---------------------------------*/

    // Stores input from metadata file if one exists
    NodeInput metaDataInput;

#ifdef ENTERPRISE_LIB
    // Protocol Specific Code
    StatsDBOspfTable* statsOspfTable;
    StatsDBPimTable* statsPimTable;
#endif
    StatsDBIgmpTable* statsIgmpTable;

#ifdef ADDON_NGCNMS
    struct SenderSessionComparator
    {
        bool operator() (std::pair<Int32, Int32> p1,
                         std::pair<Int32, Int32> p2) const
        {
            bool retVal = FALSE;
            if (p1.first < p2.first)
            {
                retVal = TRUE;
            }
            else if ((p1.first == p2.first) && (p1.second < p2.second))
            {
                retVal = TRUE;
            }

            return retVal;
        }
    };

    // key - pair<senderId, sessionId>
    // value - sender's app summary param
    map<pair<Int32, Int32>, StatsDBAppSummaryParam,
    SenderSessionComparator > appSummaryNodePairList ;
    typedef map<pair<Int32, Int32>, StatsDBAppSummaryParam,
    SenderSessionComparator >::iterator ITER ;
#endif

    Int32 networkEventsBytesUsed;
    char* networkEventsString;

    Int32 appEventsBytesUsed;
    std::string appEventsString;

};

struct MACAddress {
    unsigned char v_[6];
};

class STATSDB_NULLable_Base
{
public:
    STATSDB_NULLable_Base() : isNULL_(true) { }
   virtual std::string getStr() const = 0;
   virtual bool isString() { return false; }
   bool isNULL() const { return isNULL_; }
   void clear() {isNULL_ = true; }

protected:
    void set() { isNULL_ = false; }

private:
   bool isNULL_;
};

template<class T>
class STATSDB_NULLable : public STATSDB_NULLable_Base
{
public:
    STATSDB_NULLable() : STATSDB_NULLable_Base() { }
     STATSDB_NULLable(T defVal) : STATSDB_NULLable_Base(), value_(defVal) { }

    void set(const T& newValue) {
        STATSDB_NULLable_Base::set();
        value_ = newValue;
    }
    void set(const STATSDB_NULLable<T>& src) {
      if (src.isNULL()) return;
      set(src.value_);
    }

    T get() const {
      return value_;
    }

    std::string getStr() const {return getStr(value_);}

protected:
    T  value_;

private:
    std::string getStr(int i) const {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, "%d", i);
        return string(buf);
    }
    std::string getStr(double d) const {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, STATSDB_DOUBLE_FORMAT, d);
        return string(buf);
    }
    std::string getStr(Int64 i) const {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, "%" TYPES_64BITFMT "u", i);
        return string(buf);
    }
    std::string getStr(UInt64 u) const {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, "%" TYPES_64BITFMT "u", u);
        return string(buf);
    }
    std::string getStr(char c) const {
      return std::string(1, c);
    }
    std::string getStr(std::string s) const {
        return s;
    }
    std::string getStr(Address a) const {
        char buf[MAX_STRING_LENGTH];
        IO_ConvertIpAddressToString(const_cast<Address*>(&a), buf);
        return std::string(buf);
    }
    std::string getStr(MACAddress a) const
    {
        char str[MAX_STRING_LENGTH];
        sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", a.v_[0], a.v_[1], a.v_[2], a.v_[3], a.v_[4], a.v_[5]);
        return std::string(str);
    }
};

typedef STATSDB_NULLable<char>    STATSDB_NULLable_char;
typedef STATSDB_NULLable<int>    STATSDB_NULLable_int;
typedef STATSDB_NULLable<bool>   STATSDB_NULLable_bool;
typedef STATSDB_NULLable<Int64>  STATSDB_NULLable_Int64;
typedef STATSDB_NULLable<UInt64> STATSDB_NULLable_UInt64;
typedef STATSDB_NULLable<double> STATSDB_NULLable_double;
typedef STATSDB_NULLable<clocktype> STATSDB_NULLable_clocktype;
class STATSDB_NULLable_string : public STATSDB_NULLable<std::string> {
public:
    STATSDB_NULLable_string(const char* s = "") : STATSDB_NULLable<std::string>(s == NULL ? "" : s) { }
    bool isString() { return true; }
};

class STATSDB_NULLable_Address : public STATSDB_NULLable<Address>
{
public:
    STATSDB_NULLable_Address()
   {
     value_.networkType = NETWORK_IPV4;
     value_.interfaceAddr.ipv4 = ANY_DEST;
   }

    void set(Address* addr)
    {
        STATSDB_NULLable_Base::set();
        MAPPING_AddressCopy(&value_, addr);
    }
    void set(Address addr)
    {
        STATSDB_NULLable_Base::set();
        MAPPING_AddressCopy(&value_, &addr);
    }
    void set(NodeAddress addr)
    {
        STATSDB_NULLable_Base::set();
        MAPPING_SetAddress(NETWORK_IPV4, &value_, &addr);
    }
    void set(const STATSDB_NULLable_Address& addr)
    {
      if (addr.isNULL()) return;
      set(addr.value_);
    }
    bool isString() { return true; }
};


class STATSDB_NULLable_MACAddress : public STATSDB_NULLable < MACAddress >
{
public:
    void set(const char* addr) {
        STATSDB_NULLable_Base::set();
        memcpy(value_.v_, addr, 6);
    }
    void set(const unsigned char* addr) { set((const char*)addr); }
    void set(const STATSDB_NULLable_MACAddress& addr) {
        if (addr.isNULL()) return;
        set(addr.value_.v_);
    }

    bool isString() { return true; }
};

class STATSDB_String {
public:
    STATSDB_String() { Reset(); }

    void Reset() {
        first_ = true;
        offset_ = 0;
    }

    void push_back(const char* s, bool quote = false) {
        if (!first_) str_[offset_++] = ',';
        const char* fmt = (quote) ? "'%s'" : "%s";
        offset_ += sprintf(str_ + offset_, fmt, s);
        ERROR_AssertArgs(offset_ < sizeof(str_), "appending %s (%s) to %s", s, quote ? "true" : "false", str_);
        first_ = false;
    }
    void push_back(const std::string& s) { push_back(s.c_str(), true); }

    void push_back(double d) {
        if (!first_) str_[offset_++] = ',';
        offset_ += sprintf(str_ + offset_, STATSDB_DOUBLE_FORMAT, d);
        ERROR_AssertArgs(offset_ < sizeof(str_), "appending " STATSDB_DOUBLE_FORMAT " to %s", d, str_);
        first_ = false;
    }

    void push_back(int i) {
        if (!first_) str_[offset_++] = ',';
        offset_ += sprintf(str_ + offset_, "%d", i);
        ERROR_AssertArgs(offset_ < sizeof(str_), "appending %d to %s", i, str_);
        first_ = false;
    }

    void push_back(NodeAddress n) { push_back((int)n); }

    // this is used to build the create table statement
    void push_back(const char* name, const char* type) {
        if (!first_) str_[offset_++] = ',';
        offset_ += sprintf(str_ + offset_, "%s %s", name, type);
        ERROR_AssertArgs(offset_ < sizeof(str_), "appending %s %s to %s", name, type, str_);
        first_ = false;
    }

    const char* get() {
        return str_;
    }

private:
    bool first_;
    size_t offset_;
    char str_[1000];
};


class STATSDB_Table {
public:
    STATSDB_Table(const char* tableName, StatsDb* db);
    void Create();
    void Insert();
    virtual void Reset() {
        columns_.Reset();
        values_.Reset();
    }
    virtual bool Use() { return true; }
    virtual void PreInsert() { }

    void AddColumn(const char* name, const char* type) {
        columns_.push_back(name, FixColType(type));
    }

    static bool InConfig(NodeInput* nodeInput, const char* configName, bool dflt);
    static std::string ConfigString(NodeInput* nodeInput, const char* configName, const char* dflt);
    static bool ConfigNumbers(NodeInput* nodeInput, const char* configName, std::set<int>& numbers, const char* dflt = NULL, std::map<std::string, std::string>* keywords = NULL);

    void SetTimestamp();
    void SetTimestamp(clocktype ts);
protected:
    void SetColumn(const char* name, STATSDB_NULLable_Base& value) {
        if (value.isNULL()) return;
        columns_.push_back(name, false);
        values_.push_back(value.getStr().c_str(), value.isString());
    }

    void SetColumn(const char* name, const char* value) {
        columns_.push_back(name, false);
        values_.push_back(value, true);
    }

    void SetColumn(const char* name, int value) {
        columns_.push_back(name, false);
        values_.push_back(value);
    }

    void SetColumn(const char* name, double value) {
        columns_.push_back(name, false);
        values_.push_back(value);
    }

    void SetColumn(const char* name, NodeAddress addr) {
        char buf[MAX_STRING_LENGTH];
        IO_ConvertIpAddressToString(addr, buf);
        SetColumn(name, buf);
    }

    void SetColumnMAC(const char* name, const uint8_t* mac) {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        SetColumn(name, buf);
    }

    StatsDb* db_;

private:
    const char* FixColType(const char* type);
    bool TypeIs(const char* type, const char* compare);

    char tableName_[MAX_STRING_LENGTH];
    STATSDB_String columns_;
    STATSDB_String values_;
};

//--------------------------------------------------------------------//
// CLASS    : MetaDataStruct
// PURPOSE  : This object is used for recording user-defined columns
//            and their values for insertion into the stats database
//--------------------------------------------------------------------//
class MetaDataStruct
{
public:
    std::map<std::string, std::string> m_MetaData;

    void AddNodeMetaData(Node* node,
                         PartitionData* partition,
                         NodeInput* input);
    /*void AddInterfaceMetaData(Node* node,
                         PartitionData* partition,
                         const NodeInput* input);*/
    void AddInterfaceMetaData(Node* node,
                              Int32 interfaceIndex,
                              PartitionData* partition,
                              const NodeInput* nodeInput);
    void AddQueueMetaData(Node* node,
                          PartitionData* partition,
                          const NodeInput* input);
    void AddSchedulerMetaData(Node* node,
                              PartitionData* partition,
                              const NodeInput* input);

    void AddSessionMetaData(Node* node,
                            PartitionData* partition,
                            const NodeInput* nodeInput);

    void AddConnectionMetaData(Node* node,
                               PartitionData* partition,
                               const NodeInput* nodeInput);

    /*void AddPhyMetaData(Node* node,
            PartitionData* partition,
            const NodeInput* nodeInput);*/

    void AddPhyMetaData(Node* node,
                        Int32 interfaceIndex,
                        PartitionData* partition,
                        const NodeInput* nodeInput);
};


//--------------------------------------------------------------------//
// CLASS    : StatsDBQueueDesc
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the QUEUE_Description table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBQueueDesc
{
public:
    StatsDBQueueDesc(Int32 id,
                     Int32 index,
                     Int32 queueIndex,
                     const char* queueType);

    void SetQueueDiscipline(std::string discipline) {m_QueueDiscipline.set(discipline);}
    void SetQueuePriority(Int32 priority)           {m_QueuePriority.set(priority);}
    void SetQueueSize(Int32 size)                   {m_QueueSize.set(size);}

    Int32 m_NodeId;
    Int32 m_InterfaceIndex;
    Int32 m_QueueIndex;
    // What the queue is for, e.g. network input, mac output, etc.
    std::string m_QueueType;

    STATSDB_NULLable_string m_QueueDiscipline;
    STATSDB_NULLable_int m_QueuePriority;
    STATSDB_NULLable_int m_QueueSize;

    MetaDataStruct m_QueueMetaData;

};

//--------------------------------------------------------------------//
// CLASS    : StatsDBSchedulerDesc
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the SCHEDULER_Description table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBSchedulerDesc
{
public:
    Int32 m_NodeId;
    Int32 m_InterfaceIndex;
    std::string m_SchedulerType;
    std::string m_SchedulingAlgorithm;
    MetaDataStruct m_SchedulerMetaData;

    StatsDBSchedulerDesc(Int32 nodeId,
                         Int32 index,
                         const char* type,
                         const char* algorithm);
};

class StatsDBSessionDesc
{
public:
    Int32 m_SessionId;
    Int32 m_SenderId;
    Int32 m_ReceiverId;
    STATSDB_NULLable_string m_AppType;
    STATSDB_NULLable_string m_SenderAddr;
    STATSDB_NULLable_string m_ReceiverAddr;
    STATSDB_NULLable_int    m_SenderPort;
    STATSDB_NULLable_int    m_RecvPort;
    STATSDB_NULLable_string m_TransportProtocol;
    MetaDataStruct m_SessionMetaData;

    StatsDBSessionDesc(Int32 sessionId,
                       Int32 senderId,
                       Int32 receiverId); // not unique...

    void SetAppType(std::string AppType)        {m_AppType.set(AppType);}
    void SetSenderAddr(std::string sdrAddr)     {m_SenderAddr.set(sdrAddr);}
    void SetReceiverAddr(std::string recvAddr)  {m_ReceiverAddr.set(recvAddr);}
    void SetSenderPort(int sdrPort)             {m_SenderPort.set(sdrPort);}
    void SetRecvPort(int recvPort)              {m_RecvPort.set(recvPort);}
    void SetTransportProtocol (const std::string &protocol) {m_TransportProtocol.set(protocol);}
};

class StatsDBConnectionDesc
{
public:
    double m_timeValue;
    Int32 m_SenderId;
    Int32 m_ReceiverId;
    STATSDB_NULLable_string m_SenderAddr;
    STATSDB_NULLable_string m_ReceiverAddr;
    STATSDB_NULLable_int    m_SenderPort;
    STATSDB_NULLable_int    m_RecvPort;
    STATSDB_NULLable_string m_ConnectionType;
    STATSDB_NULLable_string m_NetworkProtocol;
    MetaDataStruct m_ConnectionMetaData;

    StatsDBConnectionDesc(double timeValue, Int32 senderId, Int32 receiverId);

    void SetSenderAddr(std::string sdrAddr)       {m_SenderAddr.set(sdrAddr);}
    void SetReceiverAddr(std::string recvAddr)    {m_ReceiverAddr.set(recvAddr);}
    void SetSenderPort(int sdrPort)               {m_SenderPort.set(sdrPort);}
    void SetRecvPort(int recvPort)                {m_RecvPort.set(recvPort);}
    void SetConnectionType(std::string connType)  {m_ConnectionType.set(connType);}
    void SetNetworkProtocol(std::string protocol) {m_NetworkProtocol.set(protocol);}
};


//--------------------------------------------------------------------//
// CLASS    : StatsDBInterfaceDesc
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the INTERFACE_Description table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBInterfaceDesc
{
public:
    Int32 m_NodeId;
    Int32 m_InterfaceIndex;
    STATSDB_NULLable_string m_InterfaceName;
    STATSDB_NULLable_string m_InterfaceAddr;
    STATSDB_NULLable_string m_SubnetMask;
    STATSDB_NULLable_string m_NetworkType;
    STATSDB_NULLable_string m_MulticastProtocol;
    STATSDB_NULLable_int    m_SubnetId;
    MetaDataStruct m_InterfaceMetaData;

    StatsDBInterfaceDesc(Int32 nodeId,
                         Int32 interfaceIndex);

    void SetInterfaceName(std::string name) {m_InterfaceName.set(name);}
    void SetInterfaceAddr(std::string addr) {m_InterfaceAddr.set(addr);}
    void SetSubnetMask(std::string mask)    {m_SubnetMask.set(mask);}
    void SetNetworkType(std::string type)   {m_NetworkType.set(type);}
    void SetMulticastProtocol(std::string protocol) {m_MulticastProtocol.set(protocol);}
    void SetSubnetId(Int32 id)              {m_SubnetId.set(id);}
};


//--------------------------------------------------------------------//
// CLASS    : StatsDBPhyDesc
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the PHY_Description table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBPhyDesc
{
public:
    Int32 m_NodeId;
    Int32 m_InterfaceIndex;
    Int32 m_PhyIndex;
    MetaDataStruct m_PhyMetaData;

    StatsDBPhyDesc(Int32 nodeId,
                   Int32 interfaceIndex,
                   Int32 phyIndex);
};
class StatsDBAppAggregateParam
{
public:
    UInt64 m_UnicastMessageSent;
    UInt64 m_UnicastMessageRecd;
    UInt64 m_MulticastMessageSent;
    UInt64 m_EffMulticastMessageSent;
    UInt64 m_MulticastMessageRecd;
    UInt64 m_UnicastByteSent;
    UInt64 m_UnicastByteRecd;
    UInt64 m_UnicastFragmentSent;
    UInt64 m_UnicastFragmentRecd;

    UInt64 m_MulticastByteSent;
    UInt64 m_EffMulticastByteSent;
    UInt64 m_MulticastByteRecd;
    UInt64 m_MulticastFragmentSent;
    UInt64 m_EffMulticastFragmentSent;
    UInt64 m_MulticastFragmentRecd;

    double m_UnicastMessageCompletionRate;
    double m_MulticastMessageCompletionRate;
    double m_UnicastOfferedLoad;
    double m_UnicastThroughput;
    double m_MulticastOfferedLoad;
    double m_MulticastThroughput;

    STATSDB_NULLable_double m_UnicastDelay;
    STATSDB_NULLable_double m_UnicastJitter;
    STATSDB_NULLable_double m_MulticastDelay;
    STATSDB_NULLable_double m_MulticastJitter;
    STATSDB_NULLable_double m_UnicastHopCount;
    STATSDB_NULLable_double m_MulticastHopCount;
    StatsDBAppAggregateParam();

    void SetUnicastDelay(double delay)         {m_UnicastDelay.set(delay);}
    void SetMulticastDelay(double delay)       {m_MulticastDelay.set(delay);}
    void SetUnicastJitter(double jitter)       {m_UnicastJitter.set(jitter);}
    void SetMulticastJitter(double jitter)     {m_MulticastJitter.set(jitter);}
    void SetUnicastHopCount(double hopCount)   {m_UnicastHopCount.set(hopCount);}
    void SetMulticastHopCount(double hopCount) {m_MulticastHopCount.set(hopCount);}
};

class StatsDBTransAggregateParam
{
public:
    UInt64 m_UnicastSegmentSentToApp;
    UInt64 m_MulticastSegmentSentToApp;
    UInt64 m_BroadcastSegmentSentToApp;

    UInt64 m_UnicastSegmentSentToNet;
    UInt64 m_MulticastSegmentSentToNet;
    UInt64 m_BroadcastSegmentSentToNet;

    UInt64 m_UnicastByteSentToApp;
    UInt64 m_MulticastByteSentToApp;
    UInt64 m_BroadcastByteSentToApp;

    UInt64 m_UnicastByteSentToNet;
    UInt64 m_MulticastByteSentToNet;
    UInt64 m_BroadcastByteSentToNet;

    double m_UnicastOfferedLoad;
    double m_MulticastOfferedLoad;
    double m_BroadcastOfferedLoad;

    double m_UnicastThroughput;
    double m_MulticastThroughput;
    double m_BroadcastThroughput;

    STATSDB_NULLable_double m_UnicastAverageDelay;
    STATSDB_NULLable_double m_MulticastAverageDelay;
    STATSDB_NULLable_double m_BroadcastAverageDelay;

    STATSDB_NULLable_double m_UnicastAverageJitter;
    STATSDB_NULLable_double m_MulticastAverageJitter;
    STATSDB_NULLable_double m_BroadcastAverageJitter;

    double totalJitter;

    void SetUnicastAveDelay(double delay)     {m_UnicastAverageDelay.set(delay);}
    void SetUnicastAveJitter(double jitter)   {m_UnicastAverageJitter.set(jitter);}
    void SetMulticastAveDelay(double delay)   {m_MulticastAverageDelay.set(delay);}
    void SetMulticastAveJitter(double jitter) {m_MulticastAverageJitter.set(jitter);}
    void SetBroadcastAveDelay(double delay)   {m_BroadcastAverageDelay.set(delay);}
    void SetBroadcastAveJitter(double jitter) {m_BroadcastAverageJitter.set(jitter);}

    StatsDBTransAggregateParam()
    {
        m_UnicastSegmentSentToApp = 0;
        m_MulticastSegmentSentToApp = 0;
        m_BroadcastSegmentSentToApp = 0;

        m_UnicastSegmentSentToNet = 0;
        m_MulticastSegmentSentToNet = 0;
        m_BroadcastSegmentSentToNet = 0;

        m_UnicastByteSentToApp = 0;
        m_MulticastByteSentToApp = 0;
        m_BroadcastByteSentToApp = 0;

        m_UnicastByteSentToNet = 0;
        m_MulticastByteSentToNet = 0;
        m_BroadcastByteSentToNet = 0;

        m_UnicastOfferedLoad = 0;
        m_MulticastOfferedLoad = 0;
        m_BroadcastOfferedLoad = 0;
        m_UnicastThroughput = 0;
        m_MulticastThroughput = 0;
        m_BroadcastThroughput = 0;

        totalJitter = 0;
    }
};
class StatsDBNetworkAggregateParam
{
public:
    UInt64 m_UDataPacketsSent;
    UInt64 m_UDataPacketsRecd;
    UInt64 m_UDataPacketsForward;
    UInt64 m_UControlPacketsSent;
    UInt64 m_UControlPacketsRecd;
    UInt64 m_UControlPacketsForward;

    UInt64 m_MDataPacketsSent;
    UInt64 m_MDataPacketsRecd;
    UInt64 m_MDataPacketsForward;
    UInt64 m_MControlPacketsSent;
    UInt64 m_MControlPacketsRecd;
    UInt64 m_MControlPacketsForward;

    UInt64 m_BDataPacketsSent;
    UInt64 m_BDataPacketsRecd;
    UInt64 m_BDataPacketsForward;
    UInt64 m_BControlPacketsSent;
    UInt64 m_BControlPacketsRecd;
    UInt64 m_BControlPacketsForward;

    UInt64 m_UDataBytesSent;
    UInt64 m_UDataBytesRecd;
    UInt64 m_UDataBytesForward;
    UInt64 m_UControlBytesSent;
    UInt64 m_UControlBytesRecd;
    UInt64 m_UControlBytesForward;

    UInt64 m_MDataBytesSent;
    UInt64 m_MDataBytesRecd;
    UInt64 m_MDataBytesForward;
    UInt64 m_MControlBytesSent;
    UInt64 m_MControlBytesRecd;
    UInt64 m_MControlBytesForward;

    UInt64 m_BDataBytesSent;
    UInt64 m_BDataBytesRecd;
    UInt64 m_BDataBytesForward;
    UInt64 m_BControlBytesSent;
    UInt64 m_BControlBytesRecd;
    UInt64 m_BControlBytesForward;

    vector<double> m_CarrierLoad;

    vector<double> m_Delay;
    vector<BOOL> m_DelaySpecified;

    vector<double> m_Jitter;    // avgJitter for sequential
    vector<BOOL> m_JitterSpecified;

    vector<Int32> m_ipOutNoRoutes;
    vector<BOOL> m_ipOutNoRoutesSpecified;

    vector<double> m_totalJitter ; // jitter information for parallel
    vector<UInt64> m_jitterDataPoints;
    vector<BOOL> m_jitterDataPointsSpecified ;

    StatsDBNetworkAggregateParam();

    void SetDelay(double delay,
                  StatsDBNetworkAggregateContent::NetAggrTrafficType);
    void SetJitter(double jitter,
                   StatsDBNetworkAggregateContent::NetAggrTrafficType);
    void SetIpOutNoRoutes(
        Int32 ipOutNoRoutes,
        StatsDBNetworkAggregateContent::NetAggrTrafficType);

};

//--------------------------------------------------------------------//
// CLASS    : StatsDBPhyAggregateParam
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the PHY_Aggregate table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBPhyAggregateParam
{
public:
    BOOL toInsert ;
    UInt64 m_NumTransmittedSignals;
    UInt64 m_NumLockedSignals;
    UInt64 m_NumReceivedSignals;
    UInt64 m_NumDroppedSignals;
    UInt64 m_NumDroppedInterferenceSignals;

    STATSDB_NULLable_double m_Utilization;
    STATSDB_NULLable_double m_AvgInterference;
    STATSDB_NULLable_double m_Delay;
    STATSDB_NULLable_double m_PathLoss;
    STATSDB_NULLable_double m_SignalPower;

    StatsDBPhyAggregateParam();

    void SetUtilization(double u)                   {m_Utilization.set(u);}
    void SetAvgInterference(double avgInterference) {m_AvgInterference.set(avgInterference);}
    void SetDelay(double avgDelay)                  {m_Delay.set(avgDelay);}
    void SetPathLoss(double avgPathLoss)            {m_PathLoss.set(avgPathLoss);}
    void SetSignalPower_db(double signalPower_db)   {m_SignalPower.set(signalPower_db);}
    void SetSignalPower(double avgSignalPower)      {SetSignalPower_db(in_db(avgSignalPower));}
};


//--------------------------------------------------------------------//
// CLASS    : StatsDBMacAggregateParam
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the MAC_Aggregate table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBMacAggregateParam
{
public:
    BOOL toInsert;
    UInt64 m_DataFramesSent;
    UInt64 m_DataFramesReceived;
    UInt64 m_DataBytesSent;
    UInt64 m_DataBytesReceived;
    UInt64 m_ControlFramesSent;
    UInt64 m_ControlFramesReceived;
    UInt64 m_ControlBytesSent;
    UInt64 m_ControlBytesReceived;

    STATSDB_NULLable_double m_AvgDelay;
    STATSDB_NULLable_double m_AvgJitter;

    double m_CarriedLoad;

    StatsDBMacAggregateParam();

    void SetAvgDelay(double delay)     {m_AvgDelay.set(delay);}
    void SetAvgJitter(double jitter)   {m_AvgJitter.set(jitter);}
};

class StatsDBAppJitterAggregateParam
{
public:
    Int32 m_PartitionId;
    Int64 m_TotalUnicastJitter;
    Int64 m_TotalMulticastJitter;
    UInt64 m_UnicastMessageReceived;
    UInt64 m_MulticastMessageReceived;
    StatsDBAppJitterAggregateParam();
};

class StatsDBAppSummaryParam
{
public:
    Int32 m_InitiatorId;
    Int32 m_ReceiverId;
    char m_TargetAddr[MAX_STRING_LENGTH];
    Int32 m_SessionId;
    Int32 m_Tos;
    UInt64 m_MessageSent;
    UInt64 m_EffMessageSent;
    UInt64 m_MessageRecd;
    UInt64 m_ByteSent;
    UInt64 m_EffByteSent;
    UInt64 m_ByteRecd;
    UInt64 m_FragmentSent;
    UInt64 m_EffFragmentSent;
    UInt64 m_FragmentRecd;
    char m_ApplicationType[MAX_STRING_LENGTH];
    char m_ApplicationName[MAX_STRING_LENGTH];

    STATSDB_NULLable_double m_MessageCompletionRate;
    STATSDB_NULLable_double m_OfferedLoad;
    STATSDB_NULLable_double m_Throughput;
    STATSDB_NULLable_double m_Delay;
    STATSDB_NULLable_double m_Jitter;
    STATSDB_NULLable_double m_HopCount;

    StatsDBAppSummaryParam();

    void SetCompletionRate(double rate) {m_MessageCompletionRate.set(rate);}
    void SetOfferedLoad(double load)    {m_OfferedLoad.set(load);}
    void SetThroughput(double tp)       {m_Throughput.set(tp);}
    void SetDelay(double delay)         {m_Delay.set(delay);}
    void SetJitter(double jitter)       {m_Jitter.set(jitter);}
    void SetHopCount(double hopCount)   {m_HopCount.set(hopCount);}
#ifdef ADDON_NGCNMS
    BOOL isRetrieved ;
#endif

};

class StatsDBMulticastAppSummaryParam
{
public:
    Int32 m_InitiatorId;
    Int32 m_ReceiverId;
    char m_GroupAddr[MAX_STRING_LENGTH];
    Int32 m_SessionId;
    Int32 m_Tos;
    UInt64 m_MessageSent;
    UInt64 m_MessageRecd;
    UInt64 m_ByteSent;
    UInt64 m_ByteRecd;
    UInt64 m_FragmentSent;
    UInt64 m_FragmentRecd;
    char m_ApplicationType[MAX_STRING_LENGTH];
    char m_ApplicationName[MAX_STRING_LENGTH];

    STATSDB_NULLable_double m_MessageCompletionRate;
    STATSDB_NULLable_double m_OfferedLoad;
    STATSDB_NULLable_double m_Throughput;
    STATSDB_NULLable_double m_Delay;
    STATSDB_NULLable_double m_Jitter;
    STATSDB_NULLable_double m_HopCount;

    StatsDBMulticastAppSummaryParam();

    void SetCompletionRate(double rate) {m_MessageCompletionRate.set(rate);}
    void SetOfferedLoad(double load)    {m_OfferedLoad.set(load);}
    void SetThroughput(double tp)       {m_Throughput.set(tp);}
    void SetDelay(double delay)         {m_Delay.set(delay);}
    void SetJitter(double jitter)       {m_Jitter.set(jitter);}
    void SetHopCount(double hopCount)   {m_HopCount.set(hopCount);}
#ifdef ADDON_NGCNMS
    BOOL isRetrieved ;
#endif

};

class StatsDBAppJitterSummaryParam
{
public:
    Int32 m_InitiatorId;
    Int32 m_SessionId;
    Int64 m_TotalJitter;
    UInt64 m_MessageRecd;
    StatsDBAppJitterSummaryParam();
};

struct StatsDBTransSummaryParam
{
    std::string senderAddr;
    std::string receiverAddr;
    short SenderPort;
    short ReceiverPort;

    UInt64 m_SegmentSentToApp;
    UInt64 m_SegmentSentToNet;
    UInt64 m_ByteSentToApp;
    UInt64 m_ByteSentToNet;
    double m_OfferedLoad;
    double m_Throughput;
    Int32 m_TotalHopCount;

    STATSDB_NULLable_double m_AverageDelay;
    STATSDB_NULLable_double m_AverageJitter;

    StatsDBTransSummaryParam()
    {
        SenderPort = 0;
        ReceiverPort = 0;

        m_SegmentSentToApp = 0;
        m_SegmentSentToNet = 0;
        m_ByteSentToApp = 0;
        m_ByteSentToNet = 0;
        m_OfferedLoad = 0;
        m_Throughput = 0;
        m_TotalHopCount = 0;
    }
    void SetAveDelay(double delay)   {m_AverageDelay.set(delay);}
    void SetAveJitter(double jitter) {m_AverageJitter.set(jitter);}
};

class StatsDBNetworkSummaryParam
{
public:
    char m_SenderAddr[MAX_STRING_LENGTH];
    char m_ReceiverAddr[MAX_STRING_LENGTH];
    char m_DestinationType[MAX_STRING_LENGTH];

    UInt64 m_UDataPacketsSent;
    UInt64 m_UDataPacketsRecd;
    UInt64 m_UDataPacketsForward;
    UInt64 m_UControlPacketsSent;
    UInt64 m_UControlPacketsRecd;
    UInt64 m_UControlPacketsForward;

    UInt64 m_UDataBytesSent;
    UInt64 m_UDataBytesRecd;
    UInt64 m_UDataBytesForward;
    UInt64 m_UControlBytesSent;
    UInt64 m_UControlBytesRecd;
    UInt64 m_UControlBytesForward;

    STATSDB_NULLable_double m_Delay;
    STATSDB_NULLable_double m_Jitter;
    STATSDB_NULLable_double m_DataDelay;
    STATSDB_NULLable_double m_DataJitter;
    STATSDB_NULLable_double m_ControlDelay;
    STATSDB_NULLable_double m_ControlJitter;

    StatsDBNetworkSummaryParam();

    void SetDelay(double delay)          {m_Delay.set(delay);}
    void SetJitter(double jitter)        {m_Jitter.set(jitter);}
    void SetDataDelay(double delay)      {m_DataDelay.set(delay);}
    void SetDataJitter(double jitter)    {m_DataJitter.set(jitter);}
    void SetControlDelay(double delay)   {m_ControlDelay.set(delay);}
    void SetControlJitter(double jitter) {m_ControlJitter.set(jitter);}
};

class StatsDBMacSummaryParam
{
public:
    Int32 m_NodeId;
    Int32 m_InterfaceIndex;

    UInt64 m_BroadcastDataFramesSent;
    UInt64 m_UnicastDataFramesSent;
    UInt64 m_BroadcastDataFramesReceived;
    UInt64 m_UnicastDataFramesReceived;
    UInt64 m_BroadcastDataBytesSent;
    UInt64 m_UnicastDataBytesSent;
    UInt64 m_BroadcastDataBytesReceived;
    UInt64 m_UnicastDataBytesReceived;
    UInt64 m_ControlFramesSent;
    UInt64 m_ControlFramesReceived;
    UInt64 m_ControlBytesSent;
    UInt64 m_ControlBytesReceived;

    UInt64 m_FramesDropped;
    UInt64 m_BytesDropped;

    STATSDB_NULLable_double m_AvgDelay;
    STATSDB_NULLable_double m_AvgJitter;

    StatsDBMacSummaryParam();

    void SetAvgDelay(double delay)   {m_AvgDelay.set(delay);}
    void SetAvgJitter(double jitter) {m_AvgJitter.set(jitter);}
};

//--------------------------------------------------------------------//
// CLASS    : StatsDBPhySummaryParam
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the PHY_Summary table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBPhySummaryParam
{
public:
    Int32 m_SenderId;
    Int32 m_RecieverId;
    Int32 m_PhyIndex;
    Int32 m_ChannelIndex;
    double m_Utilization;
    UInt64 m_NumSignals;
    UInt64 m_NumErrorSignals;

    STATSDB_NULLable_double m_AvgInterference;
    STATSDB_NULLable_double m_Delay;
    STATSDB_NULLable_double m_PathLoss;
    STATSDB_NULLable_double m_SignalPower;

    StatsDBPhySummaryParam();

    void SetAvgInterference(double avgInterference) {m_AvgInterference.set(avgInterference);}
    void SetDelay(double avgDelay)                  {m_Delay.set(avgDelay);}
    void SetPathLoss(double avgPathLoss)            {m_PathLoss.set(avgPathLoss);}
    void SetSignalPower_db(double signalPower_db)   {m_SignalPower.set(signalPower_db);}
    void SetSignalPower(double avgSignalPower)      {SetSignalPower_db(in_db(avgSignalPower));}
};


class StatsDBAppEventParam
{
public:
    Int32 m_NodeId;
    Int32 m_SessionInitiator;
    Int32 m_ReceiverId;
    STATSDB_NULLable_Address m_TargetAddr;
private:
    char m_MessageId[MAX_STRING_LENGTH];
public:
    char m_EventType[MAX_STRING_LENGTH];

    char m_ApplicationType[MAX_STRING_LENGTH];
    char m_ApplicationName[MAX_STRING_LENGTH];

    STATSDB_NULLable_int m_MsgSize;
    STATSDB_NULLable_int m_MsgSeqNum;
    STATSDB_NULLable_int m_FragId;
    STATSDB_NULLable_int m_SessionId;
    STATSDB_NULLable_int m_Priority;
    STATSDB_NULLable_string m_MsgFailureType;
    STATSDB_NULLable_clocktype m_Delay;
    STATSDB_NULLable_clocktype m_Jitter;

    Int32 m_TotalMsgSize;
    BOOL m_fragEnabled;
    BOOL m_IsFragmentation;

    int getSessionId() {
      if (m_SessionId.isNULL()) return -1;    // try to signal error when value not set
      return m_SessionId.get();
    }

    STATSDB_NULLable_clocktype m_PktCreationTime;
    STATSDB_NULLable_UInt64 m_SocketInterfaceMsgId1;
    STATSDB_NULLable_UInt64 m_SocketInterfaceMsgId2;

    StatsDBAppEventParam();

        void SetMessageId(const char* id, BOOL recordFragment) { SetMessageId(id, recordFragment ? true : false); }
        void SetMessageId(const char* id, bool recordFragment);

    void SetReceiverAddr(Address* addr)                        {m_TargetAddr.set(addr);}
    void SetReceiverAddr(Address addr)                         {m_TargetAddr.set(addr);}
    void SetReceiverAddr(NodeAddress addr)                     {m_TargetAddr.set(addr);}
    void SetReceiverAddr(const STATSDB_NULLable_Address& addr) {m_TargetAddr.set(addr);}

    void SetAppType(const char* appType);
    void SetAppName(const char* appName);

    void SetMsgSize(Int32 size)           {m_MsgSize.set(size);}
    void SetMsgSeqNum(Int32 msgSeqNum)    {m_MsgSeqNum.set(msgSeqNum);}
    void SetFragNum(Int32 fragNum)        {m_FragId.set(fragNum);}
    void SetSessionId(Int32 id)           {m_SessionId.set(id);}
    void SetPriority(Int32 priority)      {m_Priority.set(priority);}
    void SetMessageFailure(char* failure) {m_MsgFailureType.set(failure);}
    void SetDelay(clocktype delay)        {m_Delay.set(delay);}
    void SetJitter(clocktype jitter)      {m_Jitter.set(jitter);}

    void SetPacketCreateTime(clocktype time)           {m_PktCreationTime.set(time);}

    void SetSocketInterfaceMsgId(UInt64 id1, UInt64 id2)
    {
        m_SocketInterfaceMsgId1.set(id1);
        m_SocketInterfaceMsgId2.set(id2);
    }

    void SetSocketInterfaceMsgId(const StatsDBAppEventParam& src)
    {
        m_SocketInterfaceMsgId1.set(src.m_SocketInterfaceMsgId1);
        m_SocketInterfaceMsgId2.set(src.m_SocketInterfaceMsgId2);
    }

        const char* MessageId() const {return m_MessageId;}

};


class StatsDBTransportEventParam
{
public:
    Int32 m_NodeId;
    char m_MessageId[MAX_STRING_LENGTH];
    Int32 m_MsgSize;

    short m_SenderPort;
    short m_ReceiverPort;

    STATSDB_NULLable_int m_MsgSeqNum;
    STATSDB_NULLable_string m_ConnectionType;
    STATSDB_NULLable_int m_HeaderSize;
    STATSDB_NULLable_string m_Flags;
    STATSDB_NULLable_string m_EventType;
    STATSDB_NULLable_string m_FailureType;
    STATSDB_NULLable_clocktype transPktSendTime;

    StatsDBTransportEventParam(Int32 nodeId,
                               char* msgId,
                               Int32 size);

    StatsDBTransportEventParam(Int32 nodeId,
                               const std::string& msgId,
                               Int32 size);


    void SetMsgSeqNum(Int32 msgSeqNum)               {m_MsgSeqNum.set(msgSeqNum);}
    void SetConnectionType(const std::string& type)  {m_ConnectionType.set(type);}
    void SetHdrSize(Int32 size)                      {m_HeaderSize.set(size);}
    void SetFlags(const std::string &flags)          {m_Flags.set(flags);}
    void SetEventType(const std::string &eventType)  {m_EventType.set(eventType);}
    void SetMessageFailure(char* failure)            {m_FailureType.set(failure);}
    void SetPktSendTime(clocktype time)              {transPktSendTime.set(time);}
};

class StatsDBNetworkEventParam
{
public:
    enum{
       DATA,
       CONTROL
    };
    Int32 m_NodeId;
    NodeAddress m_SenderAddr;
    NodeAddress m_ReceiverAddr;
    Int32 m_MsgSize;

    STATSDB_NULLable_int m_MsgSeqNum;
    STATSDB_NULLable_int m_HeaderSize;
    STATSDB_NULLable_int m_Priority;
    STATSDB_NULLable_char m_ProtocolType;
    STATSDB_NULLable_char m_PktType;
    STATSDB_NULLable_int m_InterfaceIndex;
    STATSDB_NULLable_double m_HopCount;

    StatsDBNetworkEventParam();

    void SetMsgSeqNum(Int32 msgSeqNum)   {m_MsgSeqNum.set(msgSeqNum);}
    void SetHdrSize(Int32 size)          {m_HeaderSize.set(size);}
    void SetPriority(Int32 priority)     {m_Priority.set(priority);}
    void SetProtocolType(char type)      {m_ProtocolType.set(type);}
    void SetPktType(char type)           {m_PktType.set(type);}
    void SetInterfaceIndex(Int32 index)  {m_InterfaceIndex.set(index);}
    void SetHopCount(double count)       {m_HopCount.set(count);}
    void AddToHopCount(int n)
    {
        if (m_HopCount.isNULL())
        {
            m_HopCount.set(n);
        }
        else
        {
            m_HopCount.set(m_HopCount.get() + n);
        }
    }
};

class StatsDBMacEventParam
{
public:
    Int32 m_NodeId;
    std::string m_MessageId;
    Int32 m_InterfaceIndex;
    Int32 m_MsgSize;
    std::string m_EventType;

    STATSDB_NULLable_int m_MsgSeqNum;
    STATSDB_NULLable_int m_ChannelIndex;
    STATSDB_NULLable_string m_FailureType;
    STATSDB_NULLable_int m_HeaderSize;
    STATSDB_NULLable_string m_FrameType;
    STATSDB_NULLable_string m_DstAddrStr;
    STATSDB_NULLable_string m_SrcAddrStr;

    StatsDBMacEventParam(Int32 nodeId,
                         const std::string&,
                         Int32 phyIndex,
                         Int32 size,
                         const std::string &);

    void SetMsgId(const std::string &msgId)        {m_MessageId = msgId;}
    void SetMsgEventType(const std::string &event) {m_EventType = event;}
    void SetMsgSize(Int32 msgSize)                 {m_MsgSize = msgSize;}
    void SetMsgSeqNum(Int32 msgSeqNum)             {m_MsgSeqNum.set(msgSeqNum);}
    void SetChannelIndex (Int32 index)             {m_ChannelIndex.set(index);}
    void SetFailureType(const std::string &type)   {m_FailureType.set(type);}
    void SetHdrSize(Int32 size)                    {m_HeaderSize.set(size);}
    void SetFrameType(const std::string &type)     {m_FrameType.set(type);}
    void SetDstAddr(const std::string &addr)       {m_DstAddrStr.set(addr);}
    void SetSrcAddr(const std::string &addr)       {m_SrcAddrStr.set(addr);}
};

//--------------------------------------------------------------------//
// CLASS    : StatsDBPhyEventParam
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the PHY_Events table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBPhyEventParam
{
public:
    Int32 m_NodeId;
    std::string m_MessageId;
    Int32 m_PhyIndex;
    Int32 m_MsgSize;
    std::string m_EventType;

    STATSDB_NULLable_int m_ChannelIndex;
    STATSDB_NULLable_int m_ControlSize; // Preamble size
    STATSDB_NULLable_string m_MessageFailureType;
    STATSDB_NULLable_double m_SignalPower;
    STATSDB_NULLable_double m_Interference;
    STATSDB_NULLable_double m_PathLoss;

    StatsDBPhyEventParam(Int32 nodeId,
                         std::string messageId,
                         Int32 phyIndex,
                         Int32 msgSize,
                         std::string eventType);

    void SetChannelIndex(Int32 channel)             {m_ChannelIndex.set(channel);}
    void SetControlSize(Int32 size)                 {m_ControlSize.set(size);}
    void SetMessageFailureType(const char* type)    {m_MessageFailureType.set(type);}
    void SetSignalPower_db(double signalPower_db)   {m_SignalPower.set(signalPower_db);}
    void SetSignalPower(double signalPower)         {SetSignalPower_db(in_db(signalPower));}
    void SetInterference_db(double interference_db) {m_Interference.set(interference_db);}
    void SetInterference(double interference)       {SetInterference_db(in_db(interference));}
    void SetPathLoss_db(double pathLoss_db)         {m_PathLoss.set(pathLoss_db);}

};

struct StatsDBAppConnParam
{
    std::string m_SrcAddress;
    std::string m_DstAddress;
    Int32 sessionId;

    StatsDBAppConnParam (const std::string &srcAddr,
                         const std::string &dstAddr,
                         Int32 s_id):
                         m_SrcAddress(srcAddr),
                         m_DstAddress(dstAddr)
    {
        sessionId = s_id;
    }
};

struct StatsDBTransConnParam
{
    std::string m_SrcAddress;
    std::string m_DstAddress;
    short m_SrcPort;
    short m_DstPort;

    StatsDBTransConnParam (const std::string &srcAddr,
                           const std::string &dstAddr,
                           short sport, short dport):
            m_SrcAddress(srcAddr), m_DstAddress(dstAddr)
    {
        m_SrcPort = sport;
        m_DstPort = dport;
    }
};

class StatsDBNetworkConnParam
{
public:
    Int32 m_NodeId;
    std::string m_DstAddress;
    Int32 m_Cost;

    STATSDB_NULLable_string m_DstNetMask;
    STATSDB_NULLable_int m_OutgoingIntIndex;
    STATSDB_NULLable_string m_NextHopAddr;
    STATSDB_NULLable_string m_RoutingProtocolType;
    STATSDB_NULLable_int m_AdminDistance;

    StatsDBNetworkConnParam();

    void SetDstnetworkMask(std::string mask)       {m_DstNetMask.set(mask);}
    void SetOutgoingInterface(Int32 index)         {m_OutgoingIntIndex.set(index);}
    void SetNextHopAddr(std::string addr)          {m_NextHopAddr.set(addr);}
    void SetRoutingProtocol(std::string protocol)  {m_RoutingProtocolType.set(protocol);}
    void SetAdminDistance(Int32 distance)          {m_AdminDistance.set(distance);}

};

struct StatsDBMacConnParam
{
    Int32 m_SenderId;
    Int32 m_ReceiverId;
    Int32 m_InterfaceIndex;
    Int32 m_ChannelIndex;
    char channelIndex_str[64];

    StatsDBMacConnParam()
    {
        m_InterfaceIndex = 0;
        m_ChannelIndex = 0;
    };
};

class StatsDBPhyConnParam
{
public:
    Int32 m_SenderId;
    Int32 m_ReceiverId;

    Int32 m_ReceiverPhyIndex;

    STATSDB_NULLable_int m_PhyIndex;
    STATSDB_NULLable_string m_ChannelIndex;

    void SetPhyIndex(Int32 index)          {m_PhyIndex.set(index);}
    void SetChannelIndex(int index)        {m_ChannelIndex.set(std::string(1, index + '0'));}
    void SetChannelIndex(const char* name) {m_ChannelIndex.set(name);}

    BOOL senderListening;
    BOOL receiverListening;

    AntennaModelType antennaType;
    BOOL reachableWorst;
    StatsDBPhyConnParam();
};

//--------------------------------------------------------------------//
// CLASS    : StatsDBInterfaceStatus
// PURPOSE  : This class is used to record all the values to use
//            for inserting a new row into the INTERFACE_Status table.
//--------------------------------------------------------------------//
class StatsDBInterfaceStatus
{
public:
    std::string m_address;
    BOOL m_interfaceEnabled;
    BOOL m_triggeredUpdate;
};


//--------------------------------------------------------------------//
// TYPE    : StatsDBDamageStateType
// PURPOSE  : This type is used by the DamageState column of the
//            NODE_Status table. Note that this type is defined only
//            for clarity -- when the insertion into the DB occurs,
//            a string value is used (either "Damaged" or "Undamaged")
//--------------------------------------------------------------------//
enum StatsDBDamageStateType
{
    STATS_DB_Undamaged,
    STATS_DB_Damaged,
};


//--------------------------------------------------------------------//
// TYPE    : StatsDBActiveStateType
// PURPOSE  : This type is used by the Active column of the
//            NODE_Status table. Note that this type is defined only
//            for clarity -- when the insertion into the DB occurs,
//            a string value is used (either "Enabled" or "Disabled")
//--------------------------------------------------------------------//
enum StatsDBActiveStateType
{
    STATS_DB_Enabled,
    STATS_DB_Disabled
};



//--------------------------------------------------------------------//
// CLASS    : StatsDBNodeStatus
// PURPOSE  : This class is used to record all the values to use
//            for inserting a new row into the NODE_Status table.
//--------------------------------------------------------------------//
class StatsDBNodeStatus
{
public:
    Int32 m_NodeId;
    BOOL m_TriggeredUpdate;

    double m_DimensionOnePosition;
    double m_DimensionTwoPosition;
    double m_DimensionThreePosition;
    StatsDBActiveStateType m_Active;
    StatsDBDamageStateType m_DamageState;
    double m_DimensionOneVelocity;
    double m_DimensionTwoVelocity;
    double m_DimensionThreeVelocity;

    BOOL m_IsGateway;
    BOOL m_PositionUpdated;
    BOOL m_ActiveStateUpdated;
    BOOL m_DamageStateUpdated;
    BOOL m_VelocityUpdated;

    StatsDBNodeStatus(Node* node, BOOL triggered);
};

struct StatsDBMulticastStatus
{

    std::string timeJoined;
    std::string timeLeft;
    std::string groupName;

    //BOOL joinPrint;
    //BOOL leavePrint;

    //StatsDBMulticastStatus() : joinPrint(FALSE), leavePrint(FALSE) {}
};

class STATSDB_Table_External_Events : public STATSDB_Table
{
public:
    STATSDB_Table_External_Events(StatsDb* db) : STATSDB_Table("EXTERNAL_Events", db) { }
    virtual bool Use() {return db_ && db_->statsExternalEvents && db_->statsExternalEvents->Use();}
    virtual void PreInsert() {
        if (!db_->statsExternalEvents->Use(protocol_.get())) return;
        STATSDB_Table::Reset();
        SetTimestamp();
        SetColumn("Protocol", protocol_);
        SetColumn("NodeID", nodeId_);
        SetColumn("MessageID", messageId_);
        SetColumn("SourceAddress", sourceAddress_);
        SetColumn("DestinationAddress", destinationAddress_);
        SetColumn("EventType", eventType_);
        SetColumn("TOS", tos_);
        SetColumn("TotalLength", messageLength_);
        SetColumn("Identification", messageId_);
        SetColumn("FragmentFlags", headerFragmentFlags_);
        SetColumn("FragmentOffset", headerFragmentOffset_);
        SetColumn("TTL", ttl_);
        SetColumn("Protocol", protocol_);
        SetColumn("Device", deviceName_);
        SetColumn("DeviceIPAddress", deviceAddress_);
        SetColumn("SrcMacAddress", sourceMAC_);
        SetColumn("DstMacAddress", destinationMAC_);
    }

    // SetTimestamp(ts) inherited from parent
    // I expect that as more tables are moved to this idiom more column setters
    // will move to parent, thus standardizing column names
    void SetNodeID(int nodeId)           { nodeId_.set(nodeId);}
    void SetMessageID(const char* id)    { messageId_.set(id); }
    void SetSource(NodeAddress addr)     { sourceAddress_.set(addr); }
    void SetDestination(NodeAddress addr){ destinationAddress_.set(addr); }
    void SetIncoming()                   { eventType_.set("Incoming"); }
    void SetOutgoing()                   { eventType_.set("Outgoing"); }
    void SetTOS(int tos)                 { tos_.set(tos); }
    void SetTotalLength(int len)         { messageLength_.set(len); }
    void SetDatagramID(int id)           { headerId_.set(id); }
    void SetFragment(int fragment) {
        SetFragmentFlags(fragment >> 13);
        SetFragmentOffset(fragment & 0x1fff);
    }
    void SetFragmentFlags(int flags)     { headerFragmentFlags_.set(flags); }
    void SetFragmentOffset(int offset)   { headerFragmentOffset_.set(offset); }
    void SetTTL(int ttl)                 { ttl_.set(ttl); }
    void SetProtocol(int p)              { protocol_.set(p); }
    void SetDevice(const char* device, const char* description) {
        // the values passed in come from pcap.  Under windows the device string
        // is not very useful (its a UUID and other characters), so use the
        // description.  Linux will have a decent device name (eth0 or similar)
#ifdef _WIN32
        if (description != NULL && description[0]) device = description;
#endif
        deviceName_.set(device);
    }
    void SetDeviceAddr(NodeAddress addr) { deviceAddress_.set(addr); }
    void SetSourceMac(const uint8_t* addr)  { sourceMAC_.set(addr); }
    void SetDestMac(const uint8_t* addr)    { destinationMAC_.set(addr); }

private:
    STATSDB_NULLable_int nodeId_;
    STATSDB_NULLable_string messageId_;
    STATSDB_NULLable_Address sourceAddress_;
    STATSDB_NULLable_Address destinationAddress_;
    STATSDB_NULLable_string eventType_;
    STATSDB_NULLable_int tos_;
    STATSDB_NULLable_int messageLength_;
    STATSDB_NULLable_int headerId_;
    STATSDB_NULLable_int headerFragmentFlags_;
    STATSDB_NULLable_int headerFragmentOffset_;
    STATSDB_NULLable_int ttl_;
    STATSDB_NULLable_int protocol_;
    STATSDB_NULLable_string deviceName_;
    STATSDB_NULLable_Address deviceAddress_;
    STATSDB_NULLable_MACAddress sourceMAC_;
    STATSDB_NULLable_MACAddress destinationMAC_;
};

void AddQueryToBufferStatsDb(StatsDb* db, const std::string &queryStr);
void FlushQueryBufferStatsDb(StatsDb* db);

void InitializePartitionStatsDb(StatsDb* statsDb);

void STATSDB_CreateNodeMetaDataColumns(PartitionData* partition,
                                       NodeInput* nodeInput);
void STATSDB_CreateQueueMetaDataColumns(PartitionData* partition,
                                        NodeInput* nodeInput);
void STATSDB_CreateInterfaceMetaDataColumns(PartitionData* partition,
        NodeInput* nodeInput);
void STATSDB_CreateConnectionMetaDataColumns(PartitionData* partition,
        NodeInput* nodeInput);
void STATSDB_CreateSchedulerMetaDataColumns(PartitionData* partition,
        NodeInput* nodeInput);
void STATSDB_CreateSessionMetaDataColumns(PartitionData* partition,
        NodeInput* nodeInput);
void STATSDB_CreatePhyMetaDataColumns(PartitionData*, NodeInput*);

// API's for Description Table
void STATSDB_HandleNodeDescTableInsert(Node* node, PartitionData* partition);
void STATSDB_HandleQueueDescTableInsert(Node* node, StatsDBQueueDesc queueDesc);
class Scheduler;
void STATSDB_HandleSchedulerDescTableInsert(Node* node,
                                            Int32 interfaceIndex,
                                            const char* schedulerType,
                                            const char*);
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Message* msg,
    const Address &clientAddr,
    const Address &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol);
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Int32 sessionId,
    const Address &clientAddr,
    const Address &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol);
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Int32 sessionId,
    const NodeAddress &clientAddr,
    const NodeAddress &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol);
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Message* msg,
    const NodeAddress &clientAddr,
    const NodeAddress &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol);

void STATSDB_HandleConnectionDescTableInsert(Node* node,
        const Address &, const Address & ,
        short, short, const std::string &);
void STATSDB_HandleInterfaceDescTableInsert(Node* node, StatsDBInterfaceDesc interfaceDesc);
void STATSDB_HandlePhyDescTableInsert(Node* node,
                                      Int32 interfaceIndex,
                                      Int32 phyIndex);

void STATSDB_PrintTable(StatsDb* db,
                        STATSDB_TABLE_TYPE type);

// API's for Aggregate Table
void STATSDB_HandleAppAggregateTableInsert(Node* node);
void STATSDB_HandleTransAggregateTableInsert(Node *);
void STATSDB_HandleNetworkAggregateTableInsert(Node* node);
void STATSDB_HandlePhyAggregateTableInsert(Node* node);
void STATSDB_HandleMacAggregateTableInsert(Node* node);
void STATSDB_HandleQueueAggregateTableInsert(Node* node);

void STATSDB_HandleAppAggregateTableUpdate(Node* node);

// API's for Summary Table
void STATSDB_HandleAppSummaryTableInsert(Node* node);
void STATSDB_HandleTransSummaryTableInsert(Node *node);
void STATSDB_HandleNetworkSummaryTableInsert(Node* node);
void STATSDB_HandlePhySummaryTableInsert(Node* node,
        const StatsDBPhySummaryParam &phyParam);
void STATSDB_HandlePhySummaryTableInsert(Node* node);
void STATSDB_HandleQueueSummaryTableInsert(Node* node);
void STATSDB_HandleMulticastAppSummaryTableInsert(Node* node);

void STATSDB_HandleMulticastNetSummaryTableInsert(Node* node,
                        const StatsDBMulticastNetworkSummaryContent & stats);

void STATSDB_HandleMacSummaryTableInsert(Node* node);

// API's for Events Table
void STATSDB_HandleAppEventsTableUpdate(Node* node,
                                        void* data,
                                        const StatsDBAppEventParam & appParam);
void STATSDB_HandleTransEventsTableUpdate(Node* node,
        void* data,
        StatsDBTransportEventParam transParam);
void STATSDB_HandleNetworkEventsTableUpdate(Node* node,
        void* data,
        const StatsDBNetworkEventParam& networkParam, Message *msg,
        const char * failure, BOOL failureSpecified,
        const char *eventTyp);
void STATSDB_HandleMacEventsTableInsert(Node* node,
                                        void* data,
                                        const StatsDBMacEventParam & macParam);
void STATSDB_HandlePhyEventsTableInsert(Node* node,
                                        const StatsDBPhyEventParam &phyParam);

void HandleStatsDBMessageIdMappingInsert(Node *node,
    const std::string &, const std::string &,
    const std::string &);
void HandleStatsDBMessageIdMappingInsert(
         Node *node,
         const Message* oldMsg,
         const Message* newMsg,
         const std::string &protocol);
void HandleStatsDBTransportEventsInsert(Node* node,
                                        Message *msg,
                                        const StatsDBTransportEventParam & transParam);
// API's for Connectivity Table
void STATSDB_HandleAppConnTableInsert(Node *node,
                                      const StatsDBAppConnParam * appConnParam) ;
void STATSDB_HandleTransConnTableInsert(Node *node,
                                        const StatsDBTransConnParam * transConnParam);
void STATSDB_HandleMulticastConnTableInsert(Node *node,
        StatsDBConnTable::MulticastConnectivity multicastConnParam);
void STATSDB_HandleNetworkConnTableUpdate(Node* node,
        StatsDBNetworkConnParam ipParam);
void STATSDB_HandleMacConnTableUpdate(Node *node,
                                      const StatsDBMacConnParam &macParam);
void STATSDB_HandlePhyConnTableUpdate(Node* node,
                                      const StatsDBPhyConnParam & phyParam);


void StatsDBRetrieveDataDatabase(Node* node,
                                 StatsDb* db,
                                 std::string query,
                                 char*** data,
                                 Int32* ncol,
                                 Int32* nrow);

void StatsDbFinishEventTablesInsertion(PartitionData* partition);
void STATSDB_Finalize(PartitionData* partition);
void STATSDB_Close(void);
void STATSDB_CloseDriver(PartitionData* partition);


// APIs for Status Table
void STATSDB_HandleNodeStatusTableInsert(Node* node, StatsDBNodeStatus );
void STATSDB_HandleInterfaceStatusTableInsert(Node *node, BOOL);
void STATSDB_HandleInterfaceStatusTableInsert(Node* node,
                                              BOOL triggeredUpdate,
                                              Int32);
void STATSDB_HandleInterfaceStatusTableInsert(PartitionData* partition,
        BOOL triggeredUpdate, Message *msg);
void STATSDB_HandleInterfaceStatusTableInsert(Node* node, StatsDBInterfaceStatus );
void STATSDB_HandleMulticastStatusTableInsert(Node* node, Message *msg);
void STATSDB_HandleMulticastStatusTableInsert(Node* node) ;
void STATSDB_HandleQueueStatusTableInsertion(Node* node);



//--------------------------------------------------------------------------
// FUNCTION:  HandleLinkUtilizationTableInsert
// PURPOSE:  to process the Stats DB event inputa.
// PARAMETERS
// + node : Node* : Pointer to a node
// + LinkUtilizationParam : vector <StatsDBLinkUtilizationParam>*
//                       : pointer to the parameters.
// RETURN void.
//--------------------------------------------------------------------------


void
STATSDB_HandleLinkUtilizationTableInsert(Node* node,
                    vector<StatsDBLinkUtilizationParam>* linkUtilizationParam,
                    const std::string* str);



//--------------------------------------------------------------------------
// FUNCTION:  STATSDB_HandleLinkUtilizationPerNodeTableInsert
// PURPOSE:   to put node buffer for link utilization in to lupernodetable.
// PARAMETERS
// + node : Node* : Pointer to a node
//
//
// RETURN void.
//--------------------------------------------------------------------------

void STATSDB_HandleLinkUtilizationPerNodeTableInsert
(Node* node, const std::string* str);



//--------------------------------------------------------------------------
// FUNCTION:  STATSDB_HandleLinkUtilizationTableRetrieveFromPerNodeAndCalculate
// PURPOSE:  to process the Stats DB event inputa.
// PARAMETERS
// + node : Node* : Pointer to a node
// + LinkUtilizationParam : vector <StatsDBLinkUtilizationParam>*
//                       : pointer to the parameters.
// RETURN void.
//--------------------------------------------------------------------------

void
STATSDB_HandleLinkUtilTableCalculate(Node* node,
                vector<StatsDBLinkUtilizationParam>* linkUtilizationParam,
                const std::string* str);

#endif


