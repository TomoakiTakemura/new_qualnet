#ifndef _MQTT_APP_H_
#define _MQTT_APP_H_

#include <vector>
#include <string>

#include "types.h"
#include "dynamic.h"
#include "stats_app.h"
#include "application.h"


//number is Octet
#define MQTT_SHORT_HEADER_SIZE 2 //total length is less than 255
#define MQTT_LONG_HEADER_SIZE 4 //total length is less than 65535

//with in headersize
#define MQTT_CONNECT_SIZE 6 //+n (n is clientId)
#define MQTT_CONACK_SIZE 3 
#define MQTT_PUBLISH_SIZE 7 //+ data
#define MQTT_DISCONNECT_R_SIZE 2//use server
#define MQTT_DISCONNECT_SIZE 4//use client
#define MQTT_PINGREQ_SIZE 2//if client want to go "awake" state, size+n (n is clientId)
#define MQTT_PINGRESP_SIZE 2
#define MQTT_CLIENTID_SIZE 5//length is 1<=n<=23

#define MQTT_KEEP_ALIVE_TIMER_INTERVAL 11000000000
#define MQTT_CONNECT_INTERVAL 5000000000	//retransmit connect interval
#define MQTT_DISCONNECT_INTERVAL 5000000000	//retransmit disconnect interval
#define MQTT_CHECK_LOST MQTT_CONNECT_INTERVAL*2 //if connect isnt come than MQTT_JUDGE_LOST, its client is be ST_LOST

#ifdef ADDON_DB
class SequenceNumber;
#endif // ADDON_DB

enum MQTTDataType
{
	TY_CONNECT,
	TY_CONACK,
	TY_PUBLISH,
	TY_DISCONNECT,
	TY_PINGREQ,
	TY_PINGRESP
		
};

enum MQTTNodeStatus
{
	ST_DISCONNECT,
	ST_ACTIVE,
	ST_SLEEP,
	ST_LOST
};
enum MQTTClientFunc
{
	FN_NOTHING,
	FN_SEND_CONNECT,
	FN_RECV_DISCONNECT
};


typedef
struct struct_app_mqtt_data
{
	MQTTDataType dataType;
	clocktype txTime;
	short sourcePort;
	Int32 seqNo;
	TosType tos;
	char appName[MAX_STRING_LENGTH];
}
MQTTData;

typedef
struct struct_app_mqtt_connect_data
{
	//MQTTDataType dataType;
	//clocktype txTime;
	//short sourcePort;
	//Int32 seqNo;
	bool isResend;
	clocktype keepAlive_t; //keep alive timer interval
	//char *appName;
}
MQTTConnectData;


typedef
struct struct_app_mqtt_publish_data
{
	//MQTTDataType dataType;
	//clocktype txTime;
	//short sourcePort;
	//Int32 seqNo;
	
	int QoS;
	int DataSize;
}
MQTTPublishData;

typedef
struct struct_app_mqtt_disconnect_data
{
	//MQTTDataType dataType;
	//clocktype txTime;
	//short sourcePort;
	//Int32 seqNo;
	bool isResend;
	clocktype sleep_t; //sleepduration
}
MQTTDisconnectData;

typedef
struct struct_app_mqtt_resend_data
{
	Int32 thisSeqNo;
	MQTTDataType dataType;
	MQTTNodeStatus status;
}
MQTTResendInfo;

typedef
struct struct_app_mqtt_client_send_stats_info
{
	//num send packet (within resend packet)
	int numConnect;
	int numPublish;
	int numDisconnect;

	//num receive packet
	int rec_numConack;
	int rec_numDisconnect;

	//num resend packet
	int res_numConnect;
	int res_numDisconnect;
	public:
	void print(){
		printf("send packet count\n");
		printf("    connect    : %d\n",numConnect);
		printf("    publish    : %d\n",numPublish);
		printf("    disconnect : %d\n",numDisconnect);
		printf("receive packet count\n");
		printf("    conack     : %d\n",rec_numConack);
		printf("    disconnect : %d\n",rec_numDisconnect);
		printf("resend packet count\n");
		printf("    connect    : %d\n",res_numConnect);
		printf("    disconnect : %d\n",res_numDisconnect);	
	}
}
MQTTClientStats;

typedef
struct struct_app_mqtt_server_send_stats_info
{
	//num send packet (within resend packet)
	int numConack;
	int numDisconnect;

	//num receive packet
	int rec_numConnect;
	int rec_numPublish;
	int rec_numDisconnect;
	public:
	void print(){
		printf("send packet count\n");
		printf("    conack     : %d\n",numConack);
		printf("    disconnect : %d\n",numDisconnect);
		printf("receive packet count\n");
		printf("    connect    : %d\n",rec_numConnect);
		printf("    publish    : %d\n",rec_numPublish);
		printf("    disconnect : %d\n",rec_numDisconnect);
	}
}
MQTTServerStats;

typedef
struct struct_app_MQTT_client_str
{
	Address localAddr;
	Address remoteAddr;
	D_Clocktype interval;
	clocktype sessionStart;
	clocktype sessionFinish;
    clocktype sessionLasttime;
	clocktype endTime;
	//clocktype sessionLasttime;
    std::string* applicationName;

    STAT_AppStatistics* stats;
	short sourcePort;
    // Dynamic Address
    NodeId destNodeId; // destination node id for this app session 
    Int16 clientInterfaceIndex; // client interface index for this app 
                                // session
    Int16 destInterfaceIndex; // destination interface index for this app
                              // session
    // dns
    std::string* serverUrl;
    D_UInt32 tos;
    int  uniqueId;
#ifdef ADDON_DB
	Int32 sessionId;
	Int32 receiverId;
#endif
	MQTTNodeStatus myStatus;
	Int32 seqNo;

	Int32 itemsToPub;
	Int32 itemSize;
	int QoS;
	Int32 numPubSeq; //sum of send publish sequence
	MQTTClientStats packetStats;
	MQTTClientFunc func;


	
}
MQTTClientData;

typedef
struct struct_app_MQTT_server_str
{
	Address localAddr;
	Address remoteAddr;
    short sourcePort;
    clocktype sessionStart;
    clocktype sessionFinish;
    clocktype sessionLasttime;
	clocktype pktrecvLasttime;
    std::string* clientUrl;
    D_UInt32 tos;
    Int32 seqNo;
    int uniqueId ;
    std::string* applicationName;


#ifdef ADDON_DB
    Int32 sessionId;
    Int32 sessionInitiator;
    Int32 hopCount;
    SequenceNumber *seqNumCache;
#endif // ADDON_DB


    STAT_AppStatistics* stats;

	int numResendDisc; //count resend disconnect.
	MQTTServerStats packetStats;
}
MQTTServerData;

typedef
struct struct_app_MQTT_node_status_str
{
	//Address *clientAddr;
	char clientAddr[MAX_STRING_LENGTH]; //change set, new ,print,get
	MQTTNodeStatus nodeStatus;
}
MQTTNodeStatusData;

typedef
struct struct_app_MQTT_all_server_str
{
	std::vector<MQTTNodeStatusData> clients;
	int numDisconnectNodes;
	int numActiveNodes;
	int numSleepNodes;
	int numLostNodes;
}
MQTTServerAllData;

typedef
struct struct_app_MQTT_keep_alive_timer_str
{
	Address clientAddr;
	clocktype interval;
	clocktype txTime;
}
MQTTKeepAliveData;

typedef
struct struct_app_mqtt_check_lost_info
{
	MQTTServerData *server;
	Int32 thisSeqNo;
	clocktype setTime;
}
MQTTCheckLostInfo;

void
MQTTAddStatsToReceivePkt(Node *node, Message *msg, STAT_AppStatistics *stats);

void
MQTTSetKeepAliveTimer(clocktype kpAlTime, Node *node, Address clientAddr, short sourcePort);

void
MQTTSendConnect(Node *node, MQTTClientData *clientPtr, bool isResend);

void
MQTTSendDisconnect(Node *node, MQTTClientData *clientPtr, bool isResend);

void
AppMQTTClient(Node *node, Message *msg);

void
AppMQTTServer(Node *node, Message *msg);

void
AppMQTTClientInit(
		Node *node,
		Address clientAddr,
		Address serverAddr,
		Int32 temToPub,
		Int32 itemSize,
		clocktype interval,
		clocktype startTime,
		clocktype endTime,
		int QoS,
		unsigned tos,
		const char* srcString,
		const char* destString,
		char* appName
		); //Initialize client

void
AppMQTTServerInit(Node *node); //Initialize server

void
AppMQTTClientFinalize(Node *node, AppInfo *appInfo);

void
AppMQTTServerFinalize(Node *node, AppInfo *appInfo);

void
AppMQTTServerAllFinalize(Node *node, AppInfo *appInfo);

MQTTClientData *
AppMQTTClientGetClient(Node *node, short sourcePort);

MQTTServerData *
AppMQTTServerGetServer(Node *node, Address remoteAddr, short sourcePort);

MQTTServerAllData *
AppMQTTServerGetServerAll(Node *node);


MQTTClientData *
AppMQTTClientNewClient(
    Node *node,
    Address localAddr,
    Address remoteAddr,
    Int32 itemsToPub,
    Int32 itemSize,
    clocktype interval,
    clocktype startTime,
    clocktype endTime,
	int QoS,
    TosType tos,
    char* appName);

MQTTServerData *
AppMQTTServerNewServer(
    Node *node,
    Address localAddr,
    Address remoteAddr,
	TosType tos,
    short sourcePort,
	char *appName);

MQTTServerAllData *
AppMQTTServerNewServerAll(Node *node);

void
AppMQTTInitTrace(Node *node, const NodeInput *nodeInput);

void
AppMQTTClientAddAddressInformation(Node* node, MQTTClientData* clientPtr);

AppAddressState
AppMQTTClientGetSessionAddressState(Node *node, MQTTClientData *clientPtr);

void
MQTTDataTypeToString(MQTTDataType type, string *str);

void
MQTTNodeStatusToString(MQTTNodeStatus status, string *str);

void
MQTTServerSetStatusData(MQTTServerAllData *serverAll, Address clientAddr, MQTTNodeStatus status);

MQTTNodeStatus
MQTTServerGetStatusData(MQTTServerAllData *serverAll, Address clientAddr);

void
MQTTPrintServerAllData(MQTTServerAllData *serverAll);

void
AppMQTTClientPrintStats(Node *node, MQTTClientData *clientPtr);

void
AppMQTTServerPrintStats(Node *node, MQTTServerData *serverPtr);

void
AppMQTTServerAllPrintStats(Node *node, MQTTServerAllData *serverAll);

void
MQTTPrintMQTTData(MQTTData data);

bool
AppMQTTUrlSessionStartCallBack(
                    Node* node,
                    Address* serverAddr,
                    UInt16 sourcePort,
                    Int32 uniqueId,
                    Int16 interfaceId,
                    std::string& serverUrl,
                    struct AppUdpPacketSendingInfo* packetSendingInfo);

#endif//end _MQTT_APP_H_
