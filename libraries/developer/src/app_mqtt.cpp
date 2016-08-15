#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>

#include "api.h"
#include "partition.h"
#include "app_util.h"
#include "app_mqtt.h"
#include "ipv6.h"

#include "mydef.h"

#ifdef ADDON_DB
#include "db.h"
#include "dbapi.h"
#endif // ADDON_DB

//#define DEBUG 
//#define KANDEB 

#include "app_trafficSender.h"

void
MQTTAddStatsToReceivePkt(Node *node, Message *msg, STAT_AppStatistics *stats){
	if(node->appData.appStats){
		stats->AddFragmentReceivedDataPoints(
				node,
				msg,
				MESSAGE_ReturnPacketSize(msg),
				STAT_Unicast);

		stats->AddMessageReceivedDataPoints(
				node,
				msg,
				0,
				MESSAGE_ReturnPacketSize(msg),
				0,
				STAT_Unicast);
	}
	else{
		printf("not set appData!!!!\n");
	}
#ifdef DEBUG
	{
		char clockStr[24];
		TIME_PrintClockInSecond(
				stats->GetTotalDelay().GetValue(node->getNodeTime()),clockStr);
		printf("Received massage size is %d\n",MESSAGE_ReturnPacketSize(msg));
		printf("total end to ent delay so far is %sS\n",clockStr);
	}
#endif
}

void
MQTTSetKeepAliveTimer(clocktype kpAlTime, Node *node, Address clientAddr, short sourcePort){

	AppTimer *timer;
	Message *timerMsg;
	MQTTKeepAliveData kpdata;
	memset(&kpdata, 0, sizeof(kpdata));

	timerMsg = MESSAGE_Alloc(node,
               				APP_LAYER,
               				APP_MQTT_SERVER,
             				MSG_APP_TimerExpired);

	MESSAGE_InfoAlloc(node, timerMsg, sizeof(AppTimer));

	timer = (AppTimer *)MESSAGE_ReturnInfo(timerMsg);
	timer->sourcePort = sourcePort;
	timer->type = APP_TIMER_KEEP_ALIVE;

	kpdata.interval = kpAlTime;
	kpdata.txTime = node->getNodeTime();
	kpdata.clientAddr = clientAddr;

	APP_AddInfo(node,
				timerMsg,
				(char*) &kpdata,
				sizeof(kpdata),
				INFO_TYPE_MQTT_KPALTIMER);

#ifdef DEBUG
	{
		char clockStr[24];
		printf("MQTT Server: Node %u set Keep Alive timer\n",node->nodeId);
		TIME_PrintClockInSecond(kpAlTime, clockStr);
		printf("    interval is %sS\n", clockStr);
	}
#endif /* DEBUG */

   	MESSAGE_Send(node, timerMsg, kpAlTime);
}

void
MQTTSendConnect(Node *node, MQTTClientData *clientPtr, bool isResend){
	MQTTConnectData ConnectData;
	MQTTData data;
	int datasize = MQTT_CONNECT_SIZE + MQTT_CLIENTID_SIZE;
	char MQTTData[datasize];
	memset(&ConnectData, 0, sizeof(ConnectData));
	data.dataType = TY_CONNECT;
	data.txTime = node->getNodeTime();
	data.sourcePort = clientPtr->sourcePort;
	data.seqNo = clientPtr->seqNo;

	data.tos = clientPtr->tos;
	//data.appName = clientPtr->applicationName->c_str();
	strcpy(data.appName,clientPtr->applicationName->c_str());
	ConnectData.keepAlive_t = MQTT_KEEP_ALIVE_TIMER_INTERVAL;
	ConnectData.isResend = isResend;
#ifdef DEBUG
	{
		char clockStr[MAX_STRING_LENGTH];
		char addrStr[MAX_STRING_LENGTH];
		char localaddrStr[MAX_STRING_LENGTH];
		TIME_PrintClockInSecond(node->getNodeTime(), clockStr, node);
		IO_ConvertIpAddressToString(&clientPtr->remoteAddr, addrStr);
		IO_ConvertIpAddressToString(&clientPtr->localAddr, localaddrStr);

		printf("MQTT Client: node %d (addr: %s)sending connect packet"
			" at time %sS to MQTT server %s\n",
			node->nodeId, localaddrStr, clockStr, addrStr);
		MQTTPrintMQTTData(data);
		TIME_PrintClockInSecond(ConnectData.keepAlive_t, clockStr, node);
		printf("    keepA_t   : %s\n",clockStr);
		printf("    isResend  : %d\n",ConnectData.isResend);
		
	}
#endif /* KANDEB */

	clientPtr->packetStats.numConnect++;
	if(isResend)
		clientPtr->packetStats.res_numConnect++;

	//Note: An overloaded Function
	memset(MQTTData, '0', datasize);
#ifdef ADDON_DB
	StatsDBAppEventParam appParam;
	appParam.m_SessionInitiator = node->nodeId;
	//appParam.m_ReceiverId = clientPtr->receiverId;
	appParam.SetAppType("MQTT");
	appParam.SetFragNum(0);

	if (!clientPtr->applicationName->empty()){
		appParam.SetAppName(
  		clientPtr->applicationName->c_str());
	}
	// dns
	if (clientPtr->remoteAddr.networkType != NETWORK_INVALID){
		appParam.SetReceiverAddr(&clientPtr->remoteAddr);
	}
	appParam.SetPriority(clientPtr->tos);
	appParam.SetSessionId(clientPtr->sessionId);
	appParam.SetMsgSize((UInt32)(MQTT_CONNECT_SIZE+MQTT_CLIENTID_SIZE
			-MQTT_SHORT_HEADER_SIZE));
	appParam.m_TotalMsgSize = (UInt32)(MQTT_CONNECT_SIZE
			+MQTT_CLIENTID_SIZE);
	appParam.m_fragEnabled = FALSE;
#endif // ADDON_DB

	if (AppMQTTClientGetSessionAddressState(node, clientPtr)
        		== ADDRESS_FOUND){
  		Message* sentMsg = APP_UdpCreateMessage(node,
                                                clientPtr->localAddr,
                                                (short) clientPtr->sourcePort,
                                                clientPtr->remoteAddr,
                                                (short) APP_MQTT_SERVER,
                                                TRACE_MQTT,
                                                clientPtr->tos);
		APP_AddPayload(node,
						sentMsg,
						MQTTData,
						datasize);
		APP_AddInfo(node,
					sentMsg,
					(char*) &data,
					sizeof(data),
					INFO_TYPE_MQTT_PACKET);
		APP_AddInfo(node,
					sentMsg,
					(char*) &ConnectData,
					sizeof(ConnectData),
					INFO_TYPE_MQTT_CONNECT);

                        // dns
		AppUdpPacketSendingInfo packetSendingInfo;
#ifdef ADDON_DB
		packetSendingInfo.appParam = appParam;
#endif
   		packetSendingInfo.itemSize = MQTT_CONNECT_SIZE + MQTT_CLIENTID_SIZE;
   		packetSendingInfo.stats = clientPtr->stats;
   		packetSendingInfo.fragNo = NO_UDP_FRAG;
   		packetSendingInfo.fragSize = 0;

  		node->appData.appTrafficSender->appUdpSend(node,
                                                sentMsg,
                                                *clientPtr->serverUrl,
                                                clientPtr->localAddr,
                                                APP_MQTT_CLIENT,
                                                (short)clientPtr->sourcePort,
                                                packetSendingInfo);
		clientPtr->sessionLasttime = node->getNodeTime();
   		//if (clientPtr->itemsToSend > 0){
		//    clientPtr->itemsToSend--;
		//}

		//if (clientPtr->sessionIsClosed == FALSE){
  		//    AppCbrClientScheduleNextPkt(node, clientPtr);
  		//}
	}
	else{

		printf("mqtt.cpp in #client#  client is not found!!!");
		//clientPtr->sessionStart = 0;
   		//clientPtr->sessionIsClosed = TRUE;
	}
	//for retransmit connect pkt caused by send faild connect pkt
	//
	//
	AppTimer *timer;
	Message *timerMsg;
	MQTTResendInfo sendResInfo;
	memset(&sendResInfo, 0, sizeof(sendResInfo));

	timerMsg = MESSAGE_Alloc(node,
               				APP_LAYER,
               				APP_MQTT_CLIENT,
             				MSG_APP_TimerExpired);

	MESSAGE_InfoAlloc(node, timerMsg, sizeof(AppTimer));

	timer = (AppTimer *)MESSAGE_ReturnInfo(timerMsg);
	timer->sourcePort = clientPtr->sourcePort;
	timer->type = APP_TIMER_RESEND_PKT;

	sendResInfo.thisSeqNo = clientPtr->seqNo;
	sendResInfo.dataType = TY_CONNECT;
	sendResInfo.status = clientPtr->myStatus;

	APP_AddInfo(node,
				timerMsg,
				(char*) &sendResInfo,
				sizeof(sendResInfo),
				INFO_TYPE_MQTT_RESEND);

#ifdef DEBUG
	{
		char clockStr[24];
		printf("MQTT Client: Node %u scheduling connect packet for retransmit\n",node->nodeId);
		printf("    timer type is %d\n", timer->type);
		TIME_PrintClockInSecond(MQTT_CONNECT_INTERVAL, clockStr);
		printf("    interval is %sS\n", clockStr);
	}
#endif /* DEBUG */

#ifdef ADDON_NGCNMS
	clientPtr->lastTimer = timerMsg;
#endif
	
   	MESSAGE_Send(node, timerMsg, MQTT_CONNECT_INTERVAL);
	clientPtr->func = FN_SEND_CONNECT;
}

void
MQTTSendDisconnect(Node *node, MQTTClientData *clientPtr, bool isResend){
	MQTTDisconnectData DiscData;
	MQTTData newdata;
	int datasize = MQTT_DISCONNECT_SIZE;
	char MQTTData[datasize];

	memset(&DiscData, 0, sizeof(DiscData));
	newdata.dataType = TY_DISCONNECT;
	newdata.txTime = node->getNodeTime();
	newdata.sourcePort = clientPtr->sourcePort;
	newdata.seqNo = clientPtr->seqNo;

	newdata.tos = clientPtr->tos;
	//newdata.appName = clientPtr->applicationName->c_str();
	strcpy(newdata.appName,clientPtr->applicationName->c_str());
	DiscData.sleep_t = clientPtr->interval;
	DiscData.isResend = isResend;
#ifdef DEBUG
	{
		char clockStr[MAX_STRING_LENGTH];
  		char addrStr[MAX_STRING_LENGTH];
  		char laddrStr[MAX_STRING_LENGTH];
		TIME_PrintClockInSecond(node->getNodeTime(), clockStr, node);
		IO_ConvertIpAddressToString(&clientPtr->remoteAddr, addrStr);
		IO_ConvertIpAddressToString(&clientPtr->localAddr, laddrStr);
		printf("MQTT Client: node %d (addr: %s)sending  disconnect packet"
				" at time %sS to MQTT server %s\n",
				node->nodeId, laddrStr, clockStr, addrStr);
		MQTTPrintMQTTData(newdata);
		TIME_PrintClockInSecond(DiscData.sleep_t, clockStr, node);
		printf("    sleep_t   : %s\n",clockStr);
		printf("    isResend  : %d\n",DiscData.isResend);
	}
#endif /* KANDEB */

	clientPtr->packetStats.numDisconnect++;
	if(isResend)
		clientPtr->packetStats.res_numDisconnect++;

	//Note: An overloaded Function
	memset(MQTTData, '0', datasize);
#ifdef ADDON_DB
					
	StatsDBAppEventParam appParam;
	appParam.m_SessionInitiator = node->nodeId;
	//appParam.m_ReceiverId = clientPtr->receiverId;
	appParam.SetAppType("MQTT");
	appParam.SetFragNum(0);

	if (!clientPtr->applicationName->empty())
		appParam.SetAppName(clientPtr->applicationName->c_str());
	// dns
	if (clientPtr->remoteAddr.networkType != NETWORK_INVALID)
    	appParam.SetReceiverAddr(&clientPtr->remoteAddr);

	appParam.SetPriority(clientPtr->tos);
	appParam.SetSessionId(clientPtr->sessionId);

   	appParam.SetMsgSize((UInt32)(datasize-MQTT_SHORT_HEADER_SIZE));
	appParam.m_TotalMsgSize = (UInt32)(datasize);
	appParam.m_fragEnabled = FALSE;
				
#endif // ADDON_DB

                    // Dynamic address
                    // Create and send a UDP msg with header and virtual
                    // payload.
                    // if the client has not yet acquired a valid
                    // address then the application packets should not be
                    // generated
                    // check whether client and server are in valid address
                    // state
                    // if this is session start then packets will not be sent
                    // if in invalid address state and session will be closed
                    // ; if the session has already started and address
                    // becomes invalid during application session then
                    // packets will get generated but will be dropped at
                    //  network layer

	//if (AppMQTTClientGetSessionAddressState(node, clientPtr)== ADDRESS_FOUND){
 		Message* sentMsg = APP_UdpCreateMessage(
                                                node,
                                                clientPtr->localAddr,
                                                (short) clientPtr->sourcePort,
                                                clientPtr->remoteAddr,
                                                (short) APP_MQTT_SERVER,
                                                TRACE_MQTT,
                                                clientPtr->tos);
		APP_AddPayload(
						node,
						sentMsg,
						MQTTData,
						datasize);
		APP_AddInfo(
					node,
					sentMsg,
					(char*) &newdata,
					sizeof(newdata),
					INFO_TYPE_MQTT_PACKET);
		APP_AddInfo(
					node,
					sentMsg,
					(char*) &DiscData,
					sizeof(DiscData),
					INFO_TYPE_MQTT_DISCONNECT);

		// dns
   		AppUdpPacketSendingInfo packetSendingInfo;
#ifdef ADDON_DB
   		packetSendingInfo.appParam = appParam;
#endif
  		packetSendingInfo.itemSize = datasize;
   		packetSendingInfo.stats = clientPtr->stats;
   		packetSendingInfo.fragNo = NO_UDP_FRAG;
   		packetSendingInfo.fragSize = 0;

   		node->appData.appTrafficSender->appUdpSend(
                                                node,
                                                sentMsg,
                                                *clientPtr->serverUrl,
                                                clientPtr->localAddr,
                                                APP_MQTT_CLIENT,
                                                (short)clientPtr->sourcePort,
                                                packetSendingInfo);
		clientPtr->sessionLasttime = node->getNodeTime();

  		//if (clientPtr->itemsToSend > 0){
   		//    clientPtr->itemsToSend--;
  		//}

   		//if (clientPtr->sessionIsClosed == FALSE)
   		//    AppCbrClientScheduleNextPkt(node, clientPtr);
   	//}

	AppTimer *timer;
	Message *timerMsg;
	MQTTResendInfo sendResInfo;
	memset(&sendResInfo, 0, sizeof(sendResInfo));
	timerMsg = MESSAGE_Alloc(node,
               				APP_LAYER,
               				APP_MQTT_CLIENT,
               				MSG_APP_TimerExpired);

	MESSAGE_InfoAlloc(node, timerMsg, sizeof(AppTimer));

	timer = (AppTimer *)MESSAGE_ReturnInfo(timerMsg);
	timer->sourcePort = clientPtr->sourcePort;
	timer->type = APP_TIMER_RESEND_PKT;

	sendResInfo.thisSeqNo = clientPtr->seqNo;
	sendResInfo.dataType = TY_DISCONNECT;
	sendResInfo.status = clientPtr->myStatus;

	APP_AddInfo(
				node,
				timerMsg,
				(char*) &sendResInfo,
				sizeof(sendResInfo),
				INFO_TYPE_MQTT_RESEND);
#ifdef DEBUG
	{
		char clockStr[24];
		printf("MQTT Client: Node %u scheduling disconnect packet for retransmit\n",node->nodeId);
		printf("    timer type is %d\n", timer->type);
		TIME_PrintClockInSecond(MQTT_DISCONNECT_INTERVAL, clockStr);
		printf("    interval is %sS\n", clockStr);
	}
#endif /* DEBUG */

#ifdef ADDON_NGCNMS
	clientPtr->lastTimer = timerMsg;
#endif

   	MESSAGE_Send(node, timerMsg, MQTT_DISCONNECT_INTERVAL);
}


void
AppMQTTClient(Node *node, Message *msg)
{
#ifdef KANDEB
	printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #Client#\n",node->getNodeTime(),node->nodeId);
#endif

	char buf[MAX_STRING_LENGTH];
	char error[MAX_STRING_LENGTH];
	MQTTClientData *clientPtr;

	switch (msg->eventType){
		case MSG_APP_TimerExpired:
		{
			//send connection or ping, resend connect disconnect

			AppTimer *timer;
            timer = (AppTimer *) MESSAGE_ReturnInfo(msg);

#ifdef DEBUG
            TIME_PrintClockInSecond(node->getNodeTime(), buf, node);
            printf("MQTT Client: Node %d at %s got timer\n",
                   node->nodeId, buf);
#endif /* DEBUG */

            clientPtr = AppMQTTClientGetClient(node, timer->sourcePort);

            if (clientPtr == NULL)
            {
                sprintf(error, "MQTT Client: Node %d cannot find cbr"
                    " client\n", node->nodeId);

                ERROR_ReportError(error);
            }

            switch (timer->type)
            {
				case APP_TIMER_RESEND_PKT:
				{
					//re-send connect or disconnect packet
					//if conack or disconnect packet already received, do nothing
#ifdef DEBUG
					printf("timer type is re-send packet\n");
#endif

					MQTTResendInfo *resInfo;
					resInfo = (MQTTResendInfo *) MESSAGE_ReturnInfo(msg,INFO_TYPE_MQTT_RESEND);
					if(clientPtr->seqNo!=resInfo->thisSeqNo || 
							clientPtr->myStatus!=resInfo->status){
#ifdef DEBUG
						printf("is not use to re-send packet\n");
#endif
						break;
					}
					
					switch(resInfo->dataType)
					{
						case TY_CONNECT:
						{
#ifdef DEBUG
							printf("re-send connect packet\n");
#endif
							//re-send connect packet
							MQTTSendConnect(node, clientPtr, true);
							break;
						}
						case TY_DISCONNECT:
						{
#ifdef DEBUG
							printf("re-send disconnect packet\n");
#endif
							//re-send disconnect packet
							MQTTSendDisconnect(node, clientPtr, true);
							break;
						}
						default:
						{
							//error
						}
					}

					break;
				}
                case APP_TIMER_SEND_PKT:
                {
					//send connection mesaage to MQTT server
					MQTTSendConnect(node, clientPtr, false);
                    break;
                }
				case APP_TIMER_SEND_PNG:
				{
					//Send a  ping request message to server

					break;
				}

                default:
					break; //iranai
            }

            break;

			

		}

		case MSG_APP_FromTransport:
		{
			//receive CONACK or DISCONNECT (orPUBACK)

			UdpToAppRecv *info;
			MQTTData *data;


			info = (UdpToAppRecv *) MESSAGE_ReturnInfo(msg);
			data = (MQTTData *) MESSAGE_ReturnInfo(msg,INFO_TYPE_MQTT_PACKET);
			//memcpy(&data, MESSAGE_ReturnPacket(msg), sizeof(data));

            clientPtr = AppMQTTClientGetClient(node, data->sourcePort);
            if (clientPtr == NULL){
                sprintf(error, "MQTT Client: Node %d cannot find cbr"
                    " client\n", node->nodeId);
                ERROR_ReportError(error);
            }

			// trace recd pkt
			ActionData acnData;
			acnData.actionType = RECV;
			acnData.actionComment = NO_COMMENT;
			TRACE_PrintTrace(node, msg, TRACE_APPLICATION_LAYER, PACKET_IN, &acnData);

#ifdef DEBUG
			{
				char addrStr[MAX_STRING_LENGTH];
				string str;
				TIME_PrintClockInSecond(data->txTime, buf, node);
				IO_ConvertIpAddressToString(&info->sourceAddr, addrStr);

				printf("data structure\n");

				printf("MQTT Client %d: (addr: %s) packet transmitted at %sS\n",
						node->nodeId, addrStr, buf);
				TIME_PrintClockInSecond(node->getNodeTime(), buf, node);
				printf("    received at %s\n", buf);
				printf("    connections Id is %d\n", data->sourcePort);
				MQTTDataTypeToString(data->dataType, &str);
				printf("    dataType is %s\n", str.c_str());
				printf("    seqNo is %d\n", data->seqNo);

			}
#endif

			MQTTAddStatsToReceivePkt(node, msg, clientPtr->stats);

			switch (data->dataType){
				case TY_CONACK:
					{
						clientPtr->packetStats.rec_numConack++;
						//receive CONACK message
						//client will send publish and disconnect message
						//and set timer for send ping and re-send disconnect
						if(clientPtr->myStatus == ST_ACTIVE)
							//already send pub and dic pkt
							break;
						
						clientPtr->myStatus = ST_ACTIVE;

						//sending publish message for "itemsToPub"
						//

						int i;
						for(i=0;i<clientPtr->itemsToPub;++i){
                    		MQTTPublishData PubData;
							MQTTData newdata;
                    		int datasize = MQTT_PUBLISH_SIZE+clientPtr->itemSize;
							if(datasize>255)
								datasize = datasize-MQTT_SHORT_HEADER_SIZE
									+MQTT_LONG_HEADER_SIZE;
					
							char MQTTData[datasize];

                    		memset(&PubData, 0, sizeof(PubData));
							newdata.dataType = TY_PUBLISH;
                    		newdata.txTime = node->getNodeTime();
                    		newdata.sourcePort = clientPtr->sourcePort;
                    		newdata.seqNo = clientPtr->seqNo;

							newdata.tos = clientPtr->tos;
							//newdata.appName = clientPtr->applicationName->c_str();
							strcpy(newdata.appName,clientPtr->applicationName->c_str());
							PubData.QoS = clientPtr->QoS;
							PubData.DataSize = clientPtr->itemSize;

#ifdef DEBUG
                    		{
                        		char clockStr[MAX_STRING_LENGTH];
                        		char addrStr[MAX_STRING_LENGTH];

                        		TIME_PrintClockInSecond(node->getNodeTime(), clockStr, node);
                       			IO_ConvertIpAddressToString(
                            		&clientPtr->remoteAddr, addrStr);
                        		printf("MQTT Client: node %d sending publish packet"
                               		" at time %sS to MQTT server %s\n",
                               	node->nodeId, clockStr, addrStr);
								MQTTPrintMQTTData(newdata);

                    		}
#endif /* DEBUG */
                    		//Note: An overloaded Function
                    		memset(MQTTData, '0', datasize);
#ifdef ADDON_DB
					
                    		StatsDBAppEventParam appParam;
                    		appParam.m_SessionInitiator = node->nodeId;
                    		//appParam.m_ReceiverId = clientPtr->receiverId;
                    		appParam.SetAppType("MQTT");
                    		appParam.SetFragNum(0);

                    		if (!clientPtr->applicationName->empty())
                    		{
                        		appParam.SetAppName(
                            		clientPtr->applicationName->c_str());
                    		}
                    		// dns
                    		if (clientPtr->remoteAddr.networkType != NETWORK_INVALID)
                    		{
                        		appParam.SetReceiverAddr(&clientPtr->remoteAddr);
                    		}
                    		appParam.SetPriority(clientPtr->tos);
                    		appParam.SetSessionId(clientPtr->sessionId);

							if(datasize>255)
            	        		appParam.SetMsgSize((UInt32)
										(datasize-MQTT_SHORT_HEADER_SIZE));
							else
            	        		appParam.SetMsgSize((UInt32)
										(datasize-MQTT_LONG_HEADER_SIZE));
							appParam.m_TotalMsgSize = (UInt32)(datasize);
                    		appParam.m_fragEnabled = FALSE;
					
#endif // ADDON_DB

                    // Dynamic address
                    // Create and send a UDP msg with header and virtual
                    // payload.
                    // if the client has not yet acquired a valid
                    // address then the application packets should not be
                    // generated
                    // check whether client and server are in valid address
                    // state
                    // if this is session start then packets will not be sent
                    // if in invalid address state and session will be closed
                    // ; if the session has already started and address
                    // becomes invalid during application session then
                    // packets will get generated but will be dropped at
                    //  network layer

                    		//if (AppMQTTClientGetSessionAddressState(node, clientPtr)
                            //	== ADDRESS_FOUND)
                    		//{
                        		Message* sentMsg = APP_UdpCreateMessage(
                                                node,
                                                clientPtr->localAddr,
                                                (short) clientPtr->sourcePort,
                                                clientPtr->remoteAddr,
                                                (short) APP_MQTT_SERVER,
                                                TRACE_MQTT,
                                                clientPtr->tos);
								APP_AddPayload(
										node,
										sentMsg,
										MQTTData,
										datasize);
								APP_AddInfo(
										node,
										sentMsg,
										(char*) &newdata,
										sizeof(newdata),
										INFO_TYPE_MQTT_PACKET);
								APP_AddInfo(
										node,
										sentMsg,
										(char*) &PubData,
										sizeof(PubData),
										INFO_TYPE_MQTT_PUBLISH);

                        		// dns
                        		AppUdpPacketSendingInfo packetSendingInfo;
#ifdef ADDON_DB
                        		packetSendingInfo.appParam = appParam;
#endif
                        		packetSendingInfo.itemSize = datasize;
                        		packetSendingInfo.stats = clientPtr->stats;
                        		packetSendingInfo.fragNo = NO_UDP_FRAG;
                        		packetSendingInfo.fragSize = 0;

                        		node->appData.appTrafficSender->appUdpSend(
                                                node,
                                                sentMsg,
                                                *clientPtr->serverUrl,
                                                clientPtr->localAddr,
                                                APP_MQTT_CLIENT,
                                                (short)clientPtr->sourcePort,
                                                packetSendingInfo);
								clientPtr->sessionLasttime = node->getNodeTime();
								clientPtr->packetStats.numPublish++;

                        		//if (clientPtr->itemsToSend > 0)
                        		//{
                        		//    clientPtr->itemsToSend--;
                        		//}

						                      		//if (clientPtr->sessionIsClosed == FALSE)
                        		//{
                        		//    AppCbrClientScheduleNextPkt(node, clientPtr);
                        		//}
							//}
						}

						//send disconnect
						MQTTSendDisconnect(node, clientPtr, false);
						
						break;
					}
				case TY_DISCONNECT:
					{
						clientPtr->packetStats.rec_numDisconnect++;
						//receive DISCONNECT message
						//client will set timer to next connect message
						clientPtr->myStatus = ST_SLEEP;
						clientPtr->seqNo++;

    					AppTimer *timer;
    					Message *timerMsg;

    					timerMsg = MESSAGE_Alloc(node,
                              						APP_LAYER,
                              						APP_MQTT_CLIENT,
                              						MSG_APP_TimerExpired);

    					MESSAGE_InfoAlloc(node, timerMsg, sizeof(AppTimer));

    					timer = (AppTimer *)MESSAGE_ReturnInfo(timerMsg);
    					timer->sourcePort = clientPtr->sourcePort;
    					timer->type = APP_TIMER_SEND_PKT;

#ifdef DEBUG
    					{
        					char clockStr[24];
        					printf("MQTT Client: Node %u scheduling next connect packet\n",
               				node->nodeId);
        					TIME_PrintClockInSecond(clientPtr->interval, clockStr);
        					printf("    interval is %sS\n", clockStr);
    					}
#endif /* DEBUG */

#ifdef ADDON_NGCNMS
    					clientPtr->lastTimer = timerMsg;
#endif
    					MESSAGE_Send(node, timerMsg, clientPtr->interval);

						clientPtr->func = FN_RECV_DISCONNECT;
						break;
					}
				default:
					{
            			TIME_PrintClockInSecond(node->getNodeTime(), buf, node);
            			sprintf(error, "MQTT Client: At time %sS, node %d received "
                    		"message of unknown datatype "
                    		"%d\n", buf, node->nodeId, data->dataType);
            			ERROR_ReportError(error);
					
					}
			}
			break;
		}

		default:
		{
			TIME_PrintClockInSecond(node->getNodeTime(), buf, node);
			sprintf(error, "MQTT Client: At time %sS, node %d received message of "
					"unknown type %d\n", buf, node->nodeId, msg->eventType);
			ERROR_ReportError(error);
		}
	
	}

	MESSAGE_Free(node, msg);

}

void
AppMQTTServer(Node *node, Message *msg)
{
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #Server#\n",node->getNodeTime(),node->nodeId);
#endif

	char buf[MAX_STRING_LENGTH];
	char error[MAX_STRING_LENGTH];
	MQTTServerData *server;
	MQTTServerAllData *serverAll;

	switch (msg->eventType){
		case MSG_APP_TimerExpired:
		{
			//received Sleep Timer or keepAlive timer
			serverAll = AppMQTTServerGetServerAll(node);
			if(serverAll == NULL){
				TIME_PrintClockInSecond(node->getNodeTime(), buf, node);
				sprintf(error, "MQTT Server: At time %sS, node %d"
						" ServerAllData is not found\n", buf, node->nodeId);
				ERROR_ReportError(error);
			}

			AppTimer *timer;
            timer = (AppTimer *) MESSAGE_ReturnInfo(msg);

#ifdef DEBUG
            TIME_PrintClockInSecond(node->getNodeTime(), buf, node);
            printf("MQTT Server: Node %d at %s got timer %d\n",
                   node->nodeId, buf, timer->type);
#endif /* DEBUG */

            switch (timer->type)
            {
				case APP_TIMER_SLEEP_TIMER:
				{
					MQTTCheckLostInfo *lostInfo;
					MQTTNodeStatus cliStatus;
					lostInfo = (MQTTCheckLostInfo *) 
						MESSAGE_ReturnInfo(msg, INFO_TYPE_MQTT_LOST_CHECK);
					server = lostInfo->server;
					cliStatus = MQTTServerGetStatusData(
							serverAll, server->remoteAddr);
#ifdef DEBUG
					printf("timer type is sleep timer\n");
#endif
					//if true, client is not lost
					if(cliStatus != ST_SLEEP || server->seqNo != lostInfo->thisSeqNo 
							|| server->pktrecvLasttime > lostInfo->setTime)
						break;
#ifdef DEBUG
					printf("---client is lost---\n");
					TIME_PrintClockInSecond(node->getNodeTime(), buf);
					printf("    nowtime : %s\n",buf);
					IO_ConvertIpAddressToString(&server->remoteAddr, buf);
					printf("    client addr : %s\n",buf);
#endif
					MQTTServerSetStatusData(serverAll, server->remoteAddr, ST_LOST);
					break;
				}
				case APP_TIMER_KEEP_ALIVE:
				{
					//get keep alive timer.
					//compare time that timer seted and packet last sending.
					//if packet sending is latest, reset KPTimer packet sending
					//time + intertval. Otherwise, clients status set LOST.

					MQTTKeepAliveData *kpdata;
					MQTTNodeStatus clist;

					kpdata = (MQTTKeepAliveData *)
						MESSAGE_ReturnInfo(msg, INFO_TYPE_MQTT_KPALTIMER);
            		server = AppMQTTServerGetServer(
							node, kpdata->clientAddr, timer->sourcePort);
					clist = MQTTServerGetStatusData(serverAll, kpdata->clientAddr);
#ifdef DEBUG
					printf("timer type is keep alive timer\n");
#endif
					
					//Only active client keep alive timer use
					if(clist != ST_ACTIVE)
						break;

					if(node->getNodeTime() < server->pktrecvLasttime + kpdata->interval)
						//client isnt lost. set next timer.
						MQTTSetKeepAliveTimer(
								server->pktrecvLasttime 
								+ kpdata->interval - node->getNodeTime(), 
								node, server->remoteAddr, server->sourcePort);
					else{
						//client is lost.
#ifdef DEBUG
						printf("---client is lost---\n");
						TIME_PrintClockInSecond(node->getNodeTime(), buf);
						printf("    nowtime : %s\n",buf);
						IO_ConvertIpAddressToString(&server->remoteAddr, buf);
						printf("    client addr : %s\n",buf);
#endif
						MQTTServerSetStatusData(serverAll, kpdata->clientAddr, ST_LOST);
					}

					break;
				}
				default:
				{
					//error
                    assert(FALSE);
				}
			}

			break;
		}

		case MSG_APP_FromTransport:
		{
			//received connect or publish or disconnect
			//case connnect: client status change to active and send conack
			//case publish: keepalive timer update (and send puback if QoS!=0)
			//case disconnect: client status change to sleep and send disconnect
			//		and start sleep timer
			
			UdpToAppRecv *info;
			MQTTData *data;

#ifdef ADDON_DB
            AppMsgStatus msgStatus = APP_MSG_OLD;
#endif // ADDON_DB

			info = (UdpToAppRecv *) MESSAGE_ReturnInfo(msg);
			data = (MQTTData *) MESSAGE_ReturnInfo(msg,INFO_TYPE_MQTT_PACKET);
			//memcpy(&data, MESSAGE_ReturnPacket(msg), sizeof(data));

			// trace recd pkt
			ActionData acnData;
			acnData.actionType = RECV;
			acnData.actionComment = NO_COMMENT;
			TRACE_PrintTrace(node, msg, TRACE_APPLICATION_LAYER, PACKET_IN, &acnData);

#ifdef DEBUG
			{
				char addrStr[MAX_STRING_LENGTH];
				string str;
				TIME_PrintClockInSecond(data->txTime, buf, node);
				IO_ConvertIpAddressToString(&info->sourceAddr, addrStr);

				printf("MQTT Server %d: packet transmitted at %sS\n",
						node->nodeId, buf);
				TIME_PrintClockInSecond(node->getNodeTime(), buf, node);
				printf("    received at %s\n", buf);
				printf("    client is  %s\n", addrStr);
				printf("    connections Id is %d\n", data->sourcePort);
				MQTTDataTypeToString(data->dataType, &str);
				printf("    dataType is %s\n", str.c_str());
				printf("    seqNo is %d\n", data->seqNo);
			}
#endif

            server = AppMQTTServerGetServer(node, info->sourceAddr, data->sourcePort);

            /* New connection, so create new cbr server to handle client. */
            if (server == NULL)
            {
                server = AppMQTTServerNewServer(node, info->destAddr, info->sourceAddr, data->tos, data->sourcePort, data->appName);

                // Create statistics
                if (node->appData.appStats)
                {
                    server->stats = new STAT_AppStatistics(
                        node,
                        "mqttServer",
                        STAT_Unicast,
                        STAT_AppSenderReceiver,
                        "MQTT Server");
                    server->stats->Initialize(
                        node,
                        info->sourceAddr,
                        info->destAddr,
                        STAT_AppStatistics::GetSessionId(msg),
                        server->uniqueId);
					server->stats->setTos(server->tos);
                    server->stats->SessionStart(node);
                }
#ifdef ADDON_DB
                // cbr application, clientPort == serverPort
                STATSDB_HandleSessionDescTableInsert(node, msg,
                    info->sourceAddr, info->destAddr,
                    info->sourcePort, info->destPort,
                    "MQTT", "UDP");

                StatsDBAppEventParam* appParamInfo = NULL;
                appParamInfo = (StatsDBAppEventParam*) MESSAGE_ReturnInfo(
                                   msg,
                                   INFO_TYPE_AppStatsDbContent);
                if (appParamInfo != NULL)
                {
                    STATSDB_HandleAppConnCreation(node, info->sourceAddr,
                                info->destAddr, appParamInfo->getSessionId());
                }
#endif
			}
			
            if (server == NULL)
            {
                sprintf(error, "MQTT Server: Node %d unable to "
                        "allocation server\n", node->nodeId);

                ERROR_ReportError(error);
            }

			//sset packet receive time
			server->pktrecvLasttime = node->getNodeTime();
			
			

            //if (data->seqNo >= server->seqNo)
            //{
				MQTTAddStatsToReceivePkt(node, msg, server->stats);

                //server->seqNo = data->seqNo + 1;

				//set serverAll
				serverAll = AppMQTTServerGetServerAll(node);
				if (serverAll == NULL)
					serverAll = AppMQTTServerNewServerAll(node);

				switch(data->dataType)
				{
					case TY_CONNECT:
					{
						//send conack and client status update
						//start keepalive timer
						
						server->packetStats.rec_numConnect++;
						server->packetStats.numConack++;
						
                		server->seqNo = data->seqNo;
						MQTTConnectData *ConnectData;
						ConnectData = (MQTTConnectData *) MESSAGE_ReturnInfo(msg,INFO_TYPE_MQTT_CONNECT);

#ifdef DEBUG
						{
							char addrStr[MAX_STRING_LENGTH];
							IO_ConvertIpAddressToString(&info->sourceAddr, addrStr);
							printf("MQTTServer Node %d is get connect message.\n"
									"    transmitted client is %s\n",node->nodeId, addrStr);
                    		TIME_PrintClockInSecond(ConnectData->keepAlive_t, buf);
							printf("    keep alive timer is %s\n",buf);
						}
#endif
						//client change active state
						//
						MQTTServerSetStatusData(serverAll,server->remoteAddr,ST_ACTIVE);
						
						//set keep aliive timer
						MQTTSetKeepAliveTimer(ConnectData->keepAlive_t, 
								node, server->remoteAddr, server->sourcePort);


						//send conack
                    	//MQTTConnectData ConnectData;
						MQTTData newdata;
                    	int datasize = MQTT_CONACK_SIZE;
					
						char MQTTData[datasize];

						newdata.dataType = TY_CONACK;
                    	newdata.txTime = node->getNodeTime();
                    	newdata.sourcePort = server->sourcePort;
                    	newdata.seqNo = server->seqNo;

#ifdef DEBUG
                    	{
                        	char clockStr[MAX_STRING_LENGTH];
                        	char addrStr[MAX_STRING_LENGTH];

                        	TIME_PrintClockInSecond(node->getNodeTime(), clockStr, node);
                        	IO_ConvertIpAddressToString(
                            	&server->remoteAddr, addrStr);

                        	printf( "MQTT Server: node %d sending conack packet\n"
                               		"             at time %sS to MQTT client %s\n",
                               		node->nodeId, clockStr, addrStr);
							MQTTPrintMQTTData(newdata);
                    	}
#endif /* DEBUG */
                    	//Note: An overloaded Function
                    	memset(MQTTData, '0', datasize);
#ifdef ADDON_DB
                    	StatsDBAppEventParam appParam;
                    	appParam.m_SessionInitiator = node->nodeId;
                    	//appParam.m_ReceiverId = server->receiverId;
                    	appParam.SetAppType("MQTT");
                    	appParam.SetFragNum(0);

                    	if (!server->applicationName->empty())
                    	{
                        	appParam.SetAppName(
                            	server->applicationName->c_str());
                    	}

                    	// dns
                    	if (server->remoteAddr.networkType != NETWORK_INVALID)
                    	{
                        	appParam.SetReceiverAddr(&server->remoteAddr);
                    	}
                    	appParam.SetPriority(server->tos);
                    	appParam.SetSessionId(server->sessionId);
                    	appParam.SetMsgSize((UInt32)(MQTT_CONACK_SIZE - MQTT_SHORT_HEADER_SIZE));
                    	appParam.m_TotalMsgSize = (UInt32)(MQTT_CONACK_SIZE);
                    	appParam.m_fragEnabled = FALSE;
					
#endif // ADDON_DB

                    // Dynamic address
                    // Create and send a UDP msg with header and virtual
                    // payload.
                    // if the client has not yet acquired a valid
                    // address then the application packets should not be
                    // generated
                    // check whether client and server are in valid address
                    // state
                    // if this is session start then packets will not be sent
                    // if in invalid address state and session will be closed
                    // ; if the session has already started and address
                    // becomes invalid during application session then
                    // packets will get generated but will be dropped at
                    //  network layer

                    //if (AppMQTTClientGetSessionAddressState(node, clientPtr)
                    //        == ADDRESS_FOUND)
                        Message* sentMsg = APP_UdpCreateMessage(
                                                node,
                                                server->localAddr,
                                                (short) server->sourcePort,
                                                server->remoteAddr,
                                                (short) APP_MQTT_CLIENT,
                                                TRACE_MQTT,
                                                server->tos);
						APP_AddPayload(
								node,
								sentMsg,
								MQTTData,
								datasize);
						APP_AddInfo(
								node,
								sentMsg,
								(char*) &newdata,
								sizeof(newdata),
								INFO_TYPE_MQTT_PACKET);

                        // dns
                        AppUdpPacketSendingInfo packetSendingInfo;
#ifdef ADDON_DB
                        packetSendingInfo.appParam = appParam;
#endif
                        packetSendingInfo.itemSize = MQTT_CONACK_SIZE;
                        packetSendingInfo.stats = server->stats;
                        packetSendingInfo.fragNo = NO_UDP_FRAG;
                        packetSendingInfo.fragSize = 0;

                        node->appData.appTrafficSender->appUdpSend(
                                                node,
                                                sentMsg,
                                                *server->clientUrl,
                                                server->localAddr,
                                                APP_MQTT_SERVER,
                                                (short)server->sourcePort,
                                                packetSendingInfo);
						server->sessionLasttime = node->getNodeTime();

						//set keep alive timer
						//
						//
						//
						//
						break;
					}
					case TY_PUBLISH:
					{
						server->packetStats.rec_numPublish++;
						//if client status isnt active, not received
						MQTTNodeStatus clist = MQTTServerGetStatusData(
								serverAll, server->remoteAddr);
						if(clist != ST_ACTIVE)
							break;

						
						MQTTPublishData *PubData;
						PubData = (MQTTPublishData *) MESSAGE_ReturnInfo(msg,INFO_TYPE_MQTT_PUBLISH);
#ifdef DEBUG
						{
							char addrStr[MAX_STRING_LENGTH];
							IO_ConvertIpAddressToString(&info->sourceAddr, addrStr);
							printf("MQTTServer Node %d is get publish message.\n"
									"    transmitted client is %s\n",node->nodeId, addrStr);
							printf("    QoS is %d\n",PubData->QoS);
							printf("    DataSize is %d\n",PubData->DataSize);
						}
#endif
						break;
					}
					case TY_DISCONNECT:
					{
						//send disconnect and client status update
						//start sleep timer

						server->packetStats.rec_numDisconnect++;
						server->packetStats.numDisconnect++;

						MQTTDisconnectData *DiscData;
						DiscData = (MQTTDisconnectData *) MESSAGE_ReturnInfo(msg,INFO_TYPE_MQTT_DISCONNECT);
#ifdef DEBUG
						{
							char addrStr[MAX_STRING_LENGTH];
							IO_ConvertIpAddressToString(&info->sourceAddr, addrStr);
							printf("MQTTServer Node %d is get Disconnect message.\n"
									"    transmitted client is %s\n",node->nodeId, addrStr);
							TIME_PrintClockInSecond(DiscData->sleep_t, buf, node);
							printf("    sleep time is %s\n",buf);
						}
#endif

						//saisou ka douka wo kakunin
						//saisou nara seqNo -1
						if(DiscData->isResend == true)
							server->seqNo--;
						MQTTServerSetStatusData(serverAll, server->remoteAddr, ST_SLEEP);
						//send a disconnect message for client
						
						MQTTData newdata;
                    	int datasize = MQTT_DISCONNECT_R_SIZE;
					
						char MQTTData[datasize];

						newdata.dataType = TY_DISCONNECT;
                    	newdata.txTime = node->getNodeTime();
                    	newdata.sourcePort = server->sourcePort;
                    	newdata.seqNo = server->seqNo++;

#ifdef DEBUG
                    	{
                        	char clockStr[MAX_STRING_LENGTH];
                        	char addrStr[MAX_STRING_LENGTH];

                        	TIME_PrintClockInSecond(node->getNodeTime(), clockStr, node);
                        	IO_ConvertIpAddressToString(
                            	&server->remoteAddr, addrStr);

                        	printf( "MQTT Server: node %d sending disconnect packet\n"
                               		"             at time %sS to MQTT client %s\n",
                               		node->nodeId, clockStr, addrStr);
							MQTTPrintMQTTData(newdata);
                    	}
#endif /* DEBUG */
                    	//Note: An overloaded Function
                    	memset(MQTTData, '0', datasize);
#ifdef ADDON_DB
                    	StatsDBAppEventParam appParam;
                    	appParam.m_SessionInitiator = node->nodeId;
                    	//appParam.m_ReceiverId = server->receiverId;
                    	appParam.SetAppType("MQTT");
                    	appParam.SetFragNum(0);

                    	if (!server->applicationName->empty())
                    	{
                        	appParam.SetAppName(
                            	server->applicationName->c_str());
                    	}

                    	// dns
                    	if (server->remoteAddr.networkType != NETWORK_INVALID)
                    	{
                        	appParam.SetReceiverAddr(&server->remoteAddr);
                    	}
                    	appParam.SetPriority(server->tos);
                    	appParam.SetSessionId(server->sessionId);
                    	appParam.SetMsgSize((UInt32)(MQTT_CONACK_SIZE - MQTT_SHORT_HEADER_SIZE));
                    	appParam.m_TotalMsgSize = (UInt32)(MQTT_CONACK_SIZE);
                    	appParam.m_fragEnabled = FALSE;
					
#endif // ADDON_DB

                    // Dynamic address
                    // Create and send a UDP msg with header and virtual
                    // payload.
                    // if the client has not yet acquired a valid
                    // address then the application packets should not be
                    // generated
                    // check whether client and server are in valid address
                    // state
                    // if this is session start then packets will not be sent
                    // if in invalid address state and session will be closed
                    // ; if the session has already started and address
                    // becomes invalid during applicatioDisconnectiion then
                    // packets will get generated but will be dropped at
                    //  network layer

                    //if (AppMQTTClientGetSessionAddressState(node, clientPtr)
                    //        == ADDRESS_FOUND)
                        Message* sentMsg = APP_UdpCreateMessage(
                                                node,
                                                server->localAddr,
                                                (short) server->sourcePort,
                                                server->remoteAddr,
                                                (short) APP_MQTT_CLIENT,
                                                TRACE_MQTT,
                                                server->tos);
						APP_AddPayload(
								node,
								sentMsg,
								MQTTData,
								datasize);
						APP_AddInfo(
								node,
								sentMsg,
								(char*) &newdata,
								sizeof(newdata),
								INFO_TYPE_MQTT_PACKET);

                        // dns
                        AppUdpPacketSendingInfo packetSendingInfo;
#ifdef ADDON_DB
                        packetSendingInfo.appParam = appParam;
#endif
                        packetSendingInfo.itemSize = MQTT_CONACK_SIZE;
                        packetSendingInfo.stats = server->stats;
                        packetSendingInfo.fragNo = NO_UDP_FRAG;
                        packetSendingInfo.fragSize = 0;

                        node->appData.appTrafficSender->appUdpSend(
                                                node,
                                                sentMsg,
                                                *server->clientUrl,
                                                server->localAddr,
                                                APP_MQTT_SERVER,
                                                (short)server->sourcePort,
                                                packetSendingInfo);
						server->sessionLasttime = node->getNodeTime();

                        //if (clientPtr->itemsToSend > 0)
                        //{
                        //    clientPtr->itemsToSend--;
                        //}

                        //if (clientPtr->sessionIsClosed == FALSE)
                        //{
                        //    AppCbrClientScheduleNextPkt(node, clientPtr);
                        //}
						
                		//server->seqNo = data->seqNo + 1;

						//set sleep timer
						//
						
    					AppTimer *timer;
    					Message *timerMsg;
						MQTTCheckLostInfo lostInfo;

    					timerMsg = MESSAGE_Alloc(node,
                              						APP_LAYER,
                              						APP_MQTT_SERVER,
                              						MSG_APP_TimerExpired);

    					MESSAGE_InfoAlloc(node, timerMsg, sizeof(AppTimer));

    					timer = (AppTimer *)MESSAGE_ReturnInfo(timerMsg);
    					timer->sourcePort = server->sourcePort;
    					timer->type = APP_TIMER_SLEEP_TIMER;
						lostInfo.thisSeqNo = server->seqNo;
						lostInfo.server = server;
						lostInfo.setTime = node->getNodeTime();

						APP_AddInfo(node,
									timerMsg,
									(char*) &lostInfo,
									sizeof(lostInfo),
									INFO_TYPE_MQTT_LOST_CHECK);


#ifdef DEBUG
    					{
        					char clockStr[24];
        					printf("MQTT Server: Node %u set sleep timer\n",
               				node->nodeId);
        					//TIME_PrintClockInSecond(MQTT_CHECK_LOST, clockStr);
        					TIME_PrintClockInSecond(DiscData->sleep_t*2, clockStr);
							
        					printf("    interval is %sS\n", clockStr);
    					}
#endif /* DEBUG */

#ifdef ADDON_NGCNMS
    					server->lastTimer = timerMsg;
#endif
    					MESSAGE_Send(node, timerMsg, DiscData->sleep_t*2);

						break;
					}
					default:
					{
                    	assert(FALSE);
					}
				} //end switch(dataType)
			//}
			break;
		} //end MSG_APP_FromTransport:

		default:
		{
			TIME_PrintClockInSecond(node->getNodeTime(), buf, node);
			sprintf(error, "MQTT Server: At time %sS, node %d received message of "
					"unknown type %d\n", buf, node->nodeId, msg->eventType);
			ERROR_ReportError(error);
		}
	
	}

	MESSAGE_Free(node, msg);

}


void
AppMQTTClientInit(
		Node *node,
		Address clientAddr,
		Address serverAddr,
		Int32 itemsToPub,
		Int32 itemSize,
		clocktype interval,
		clocktype startTime,
		clocktype endTime,
		int QoS,
		unsigned tos,
		const char* srcString,
		const char* destString,
		//BOOL isRsvpTeEnable, //not use
		char* appName
		)
{

#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ClientInit#\n",node->getNodeTime(),node->nodeId);
#endif

	char error[MAX_STRING_LENGTH];
	AppTimer *timer;
	MQTTClientData *clientPtr;
    Message *timerMsg;
    int minSize;

    //ERROR_Assert(sizeof(MQTTData) <= MQTT_HEADER_SIZE,
    //             "MQTTData size cant be greater than MQTT_HEADER_SIZE");
    //minSize = MQTT_HEADER_SIZE;

	//error check
	if(itemsToPub<0){
		sprintf(error,"MQTT Client: Node %d item to sends needs to be >= 0\n",node->nodeId);
		ERROR_ReportError(error);
	}
	if(itemSize<=0){
		sprintf(error,"MQTT Client: Node %d item size needs to be > 0\n",node->nodeId);
		ERROR_ReportError(error);
	}
	if(interval<=0){
		sprintf(error,"MQTT Client: Node %d interval needs to be > 0\n",node->nodeId);
		ERROR_ReportError(error);
	}
	if(!(QoS==-1 || QoS==0 || QoS==1 || QoS==2)){
		sprintf(error,"MQTT Client: Node %d QoS needs to be -1,0,1,2\n",node->nodeId);
		ERROR_ReportError(error);
	}
	if(startTime<0){
		sprintf(error,"MQTT Client: Node %d start time needs to be > 0\n",node->nodeId);
		ERROR_ReportError(error);
	}
	
    if (/*tos < 0 || */tos > 255)
    {
        sprintf(error, "MQTT Client: Node %d should have tos value "
            "within the range 0 to 255.\n",
            node->nodeId);
        ERROR_ReportError(error);
    }

	clientPtr = AppMQTTClientNewClient(
			node,
			clientAddr,
			serverAddr,
			itemsToPub,
			itemSize,
			interval,
			startTime,
			endTime,
			QoS,
			(TosType) tos,
			appName);

	
    if (clientPtr == NULL)
    {
        sprintf(error,
                "MQTT Client: Node %d cannot allocate memory for "
                    "new client\n", node->nodeId);

        ERROR_ReportError(error);
    }
    // dns
    // Skipped if the server network type is not valid
    // it should happen only when the server address is given by a URL
    // statistics will be initialized when url is resolved by dns
    if (serverAddr.networkType != NETWORK_INVALID && node->appData.appStats)
    {
        std::string customName;
        if (clientPtr->applicationName->empty())
        {
            customName = "MQTT Client";
        }
        else
        {
            customName = *clientPtr->applicationName;
        }
        clientPtr->stats = new STAT_AppStatistics(
                                      node,
                                      "mqtt",
                                      STAT_Unicast,
                                      STAT_AppSenderReceiver,
                                      customName.c_str());
        clientPtr->stats->Initialize(node,
                                     clientAddr,
                                     serverAddr,
                                     (STAT_SessionIdType)clientPtr->uniqueId,
                                     clientPtr->uniqueId);

        clientPtr->stats->setTos(tos);
	}
	// Create application hop by hop flow filter button
	if (GUI_IsAppHopByHopFlowEnabled())
	{
  		GUI_CreateAppHopByHopFlowFilter(clientPtr->stats->GetSessionId(),
			node->nodeId, srcString, destString, "MQTT");
   	}

	/*
    if (node->transportData.rsvpProtocol && isRsvpTeEnabled)
    {
        // Note: RSVP is enabled for Ipv4 networks only.
        Message *rsvpRegisterMsg;
        AppToRsvpSend *info;

        rsvpRegisterMsg = MESSAGE_Alloc(node,
                                  TRANSPORT_LAYER,
                                  TransportProtocol_RSVP,
                                  MSG_TRANSPORT_RSVP_InitApp);

        MESSAGE_InfoAlloc(node,
                           rsvpRegisterMsg,
                           sizeof(AppToRsvpSend));

        info = (AppToRsvpSend *) MESSAGE_ReturnInfo(rsvpRegisterMsg);

        info->sourceAddr = GetIPv4Address(clientAddr);
        info->destAddr = GetIPv4Address(serverAddr);

        info->upcallFunctionPtr = NULL;

        MESSAGE_Send(node, rsvpRegisterMsg, startTime);
    }
	*/
	

    timerMsg = MESSAGE_Alloc(node,
                              APP_LAYER,
                              APP_MQTT_CLIENT,
                              MSG_APP_TimerExpired);

    MESSAGE_InfoAlloc(node, timerMsg, sizeof(AppTimer));

    timer = (AppTimer *)MESSAGE_ReturnInfo(timerMsg);

    timer->sourcePort = clientPtr->sourcePort;
    timer->type = APP_TIMER_SEND_PKT;

#ifdef DEBUG
    {
        char clockStr[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(startTime, clockStr, node);
        printf("MQTT Client: Node %d starting client at %sS\n",
               node->nodeId, clockStr);
    }
#endif /* DEBUG */

    MESSAGE_Send(node, timerMsg, startTime);

#ifdef ADDON_NGCNMS

    clientPtr->lastTimer = timerMsg;

#endif

    // Dynamic Address
    // update the client with url info if destination configured as URL
    if (serverAddr.networkType == NETWORK_INVALID && destString)
    {
        // set the dns info in client pointer if server url
        // is present
        // set the server url if it is not localhost
        if (node->appData.appTrafficSender->ifUrlLocalHost(destString)
                                                                    == FALSE)
        {
            node->appData.appTrafficSender->appClientSetDnsInfo(
                                        node,
                                        destString,
                                        clientPtr->serverUrl);
        }
    }
    // Update the CBR clientPtr with address information
    AppMQTTClientAddAddressInformation(node, clientPtr);
}

void
AppMQTTServerInit(Node *node)
{
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ServerInit#\n",node->getNodeTime(),node->nodeId);
#endif
    APP_InserInPortTable(node, APP_MQTT_SERVER, APP_MQTT_SERVER);
}

void
AppMQTTClientFinalize(Node *node, AppInfo *appInfo)
{
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ClinetFinalize#\n",node->getNodeTime(),node->nodeId);
#endif

    MQTTClientData *clientPtr = (MQTTClientData*)appInfo->appDetail;

    if (node->appData.appStats == TRUE)
    {
        AppMQTTClientPrintStats(node, clientPtr);
        // Print stats in .stat file using Stat APIs
        if (clientPtr->stats)
        {
            clientPtr->stats->Print(node,
                                    "Application",
                                    "MQTT Client",
                                    ANY_ADDRESS,
                                    clientPtr->sourcePort);
        }
    }
#ifdef ADDON_DB
    //if (clientPtr->seqNumCache != NULL)
    //{
    //    delete client->seqNumCache;
    //    clientPtr->seqNumCache = NULL;
    //}
#endif // ADDON_DB
	char addrStr[MAX_STRING_LENGTH];
	IO_ConvertIpAddressToString(&clientPtr->localAddr, addrStr);
	printf("client %s\n ",addrStr);
	clientPtr->packetStats.print();
}


void
AppMQTTServerFinalize(Node *node, AppInfo *appInfo)
{
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ServerFinalize#\n",node->getNodeTime(),node->nodeId);
#endif

    MQTTServerData *serverPtr = (MQTTServerData*)appInfo->appDetail;

    if (node->appData.appStats == TRUE)
    {
        AppMQTTServerPrintStats(node, serverPtr);
        // Print stats in .stat file using Stat APIs
        if (serverPtr->stats)
        {
            serverPtr->stats->Print(node,
                                    "Application",
                                    "MQTT Server",
                                    ANY_ADDRESS,
                                    serverPtr->sourcePort);
        }
    }
#ifdef ADDON_DB
    //if (serverPtr->seqNumCache != NULL)
    //{
    //    delete serverPtr->seqNumCache;
    //    serverPtr->seqNumCache = NULL;
    //}
#endif // ADDON_DB
	char cliaddrStr[MAX_STRING_LENGTH]; 
	char seraddrStr[MAX_STRING_LENGTH]; 
	IO_ConvertIpAddressToString(&serverPtr->remoteAddr, cliaddrStr);
	IO_ConvertIpAddressToString(&serverPtr->localAddr, seraddrStr);
	printf("server %s (remote : %s)\n",seraddrStr,cliaddrStr);
	serverPtr->packetStats.print();

}

void
AppMQTTServerAllFinalize(Node *node, AppInfo *appInfo)
{
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ServerAllFinalize#\n",node->getNodeTime(),node->nodeId);
#endif	

}


MQTTClientData *
AppMQTTClientGetClient(Node *node, short sourcePort)
{
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ClinetGetClient#\n",node->getNodeTime(),node->nodeId);
#endif
    AppInfo *appList = node->appData.appPtr;
    MQTTClientData *mqttClient;

    for (; appList != NULL; appList = appList->appNext)
    {
        if (appList->appType == APP_MQTT_CLIENT)
        {
            mqttClient = (MQTTClientData *) appList->appDetail;

            if (mqttClient->sourcePort == sourcePort)
            {
                return mqttClient;
            }
        }
    }

    return NULL;
}


MQTTServerData * //inline//
AppMQTTServerGetServer(
    Node *node, Address remoteAddr, short sourcePort)
{
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ServerGetServer#\n",node->getNodeTime(),node->nodeId);
#endif	
    AppInfo *appList = node->appData.appPtr;
    MQTTServerData *mqttServer;

    for (; appList != NULL; appList = appList->appNext)
    {
        if (appList->appType == APP_MQTT_SERVER)
        {
            mqttServer = (MQTTServerData *) appList->appDetail;

            if ((mqttServer->sourcePort == sourcePort) &&
                IO_CheckIsSameAddress(
                    mqttServer->remoteAddr, remoteAddr))
            {
                return mqttServer;
            }
       }
   }

    return NULL;
}

MQTTServerAllData * //inline//
AppMQTTServerGetServerAll(Node *node)
{
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ServerGetServerAll#\n",node->getNodeTime(),node->nodeId);
#endif
    AppInfo *appList = node->appData.appPtr;
    MQTTServerAllData *mqttServerAll;

    for (; appList != NULL; appList = appList->appNext)
    {
        if (appList->appType == APP_MQTT_SERVER_ALL)
        {
            mqttServerAll = (MQTTServerAllData *) appList->appDetail;
            return mqttServerAll;
       }
   }

    return NULL;
}

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
		char* appName)
{
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ClientNewClient#\n",node->getNodeTime(),node->nodeId);
#endif	

    MQTTClientData *mqttClient;

    mqttClient = (MQTTClientData *)
                MEM_malloc(sizeof(MQTTClientData));
    memset(mqttClient, 0, sizeof(MQTTClientData));

    /*
     * fill in cbr info.
     */
    memcpy(&(mqttClient->localAddr), &localAddr, sizeof(Address));
    memcpy(&(mqttClient->remoteAddr), &remoteAddr, sizeof(Address));
    mqttClient->interval = interval;


#ifndef ADDON_NGCNMS
    mqttClient->sessionStart = node->getNodeTime() + startTime;
#else
    if (!NODE_IsDisabled(node))
    {
        mqttClient->sessionStart = node->getNodeTime() + startTime;
    }
    else
    {
        // start time was already figured out in caller function.
        mqttClient->sessionStart = startTime;
    }
#endif
	
    //mqttClient->sessionIsClosed = FALSE;
    //mqttClient->sessionLastSent = node->getNodeTime();
    mqttClient->sessionFinish = node->getNodeTime();
    mqttClient->endTime = endTime;
    mqttClient->itemsToPub = itemsToPub;
    mqttClient->itemSize = itemSize;
    mqttClient->sourcePort = node->appData.nextPortNum++;
    mqttClient->seqNo = 0;
    mqttClient->tos = tos;
    mqttClient->uniqueId = node->appData.uniqueId++;
    //client pointer initialization with respect to mdp
    //cbrClient->isMdpEnabled = FALSE;
    //cbrClient->mdpUniqueId = -1;  //invalid value
    mqttClient->stats = NULL;

	//takemura
	mqttClient->QoS = QoS;
	mqttClient->myStatus = ST_DISCONNECT;
	mqttClient->numPubSeq = 0;
	
    // dns
    mqttClient->serverUrl = new std::string();
    mqttClient->serverUrl->clear();
    mqttClient->destNodeId = ANY_NODEID;
    mqttClient->clientInterfaceIndex = ANY_INTERFACE;
    mqttClient->destInterfaceIndex = ANY_INTERFACE;
    if (appName)
    {
        mqttClient->applicationName = new std::string(appName);
    }
    else
    {
        mqttClient->applicationName = new std::string();
    }
#ifdef ADDON_DB
    mqttClient->sessionId = mqttClient->uniqueId;

	mqttClient->func = FN_NOTHING;

    // dns
    // Skipped if the server network type is not valid
    // it should happen only when the server is given by a URL
    // receiverId will be initialized when url is resolved
    if (mqttClient->remoteAddr.networkType != NETWORK_INVALID)
    {
        if (Address_IsAnyAddress(&(mqttClient->remoteAddr)) ||
            Address_IsMulticastAddress(&mqttClient->remoteAddr))
        {
            mqttClient->receiverId = 0;
        }
        else
        {
            mqttClient->receiverId =
                MAPPING_GetNodeIdFromInterfaceAddress(node, remoteAddr);
        }
    }
#endif // ADDON_DB

    // Add MQTT variables to hierarchy
    std::string path;
    D_Hierarchy *h = &node->partitionData->dynamicHierarchy;

    if (h->CreateApplicationPath(
            node,                   // node
            "mqttClient",            // protocol name
            mqttClient->sourcePort,  // port
            "interval",             // object name
            path))                  // path (output)
    {
        h->AddObject(
            path,
            new D_ClocktypeObj(&mqttClient->interval));
    }

// The HUMAN_IN_THE_LOOP_DEMO is part of a gui user-defined command
// demo.
// The type of service value for this CBR application is added to
// the dynamic hierarchy so that the user-defined-command can change
// it during simulation.
#ifdef HUMAN_IN_THE_LOOP_DEMO
    if (h->CreateApplicationPath(
            node,
            "mqttClient",
            mqttClient->sourcePort,
            "tos",                  // object name
            path))                  // path (output)
    {
        h->AddObject(
            path,
            new D_UInt32Obj(&mqttClient->tos));
    }
#endif

#ifdef DEBUG
    {
        char clockStr[MAX_STRING_LENGTH];
        char localAddrStr[MAX_STRING_LENGTH];
        char remoteAddrStr[MAX_STRING_LENGTH];

        IO_ConvertIpAddressToString(&mqttClient->localAddr, localAddrStr);
        IO_ConvertIpAddressToString(&mqttClient->remoteAddr, remoteAddrStr);

        printf("MQTT Client: Node %u created new MQTT client structure\n",
               node->nodeId);
        printf("    localAddr = %s\n", localAddrStr);
        printf("    remoteAddr = %s\n", remoteAddrStr);
        TIME_PrintClockInSecond(mqttClient->interval, clockStr);
        printf("    interval = %s\n", clockStr);
        TIME_PrintClockInSecond(mqttClient->sessionStart, clockStr, node);
        printf("    sessionStart = %sS\n", clockStr);
        printf("    itemsToPub = %u\n", mqttClient->itemsToPub);
        printf("    itemSize = %u\n", mqttClient->itemSize);
        printf("    sourcePort = %d\n", mqttClient->sourcePort);
        printf("    seqNo = %d\n", mqttClient->seqNo);
    }
#endif /* DEBUG */

    APP_RegisterNewApp(node, APP_MQTT_CLIENT, mqttClient);

    return mqttClient;
}


MQTTServerData *
AppMQTTServerNewServer(
		Node *node,
		Address localAddr,
		Address remoteAddr,
		TosType tos,
		short sourcePort,
		char *appName)
{
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ServerNewServer#\n",node->getNodeTime(),node->nodeId);
#endif	

	
    MQTTServerData *mqttServer;

    mqttServer = (MQTTServerData *)
                MEM_malloc(sizeof(MQTTServerData));
    memset(mqttServer, 0, sizeof(MQTTServerData));
    /*
     * Fill in mqt info.
     */
    memcpy(&(mqttServer->localAddr), &localAddr, sizeof(Address));
    memcpy(&(mqttServer->remoteAddr), &remoteAddr, sizeof(Address));
    mqttServer->sourcePort = sourcePort;
    mqttServer->sessionStart = node->getNodeTime();
    mqttServer->sessionFinish = node->getNodeTime();
	mqttServer->sessionLasttime = node->getNodeTime();
	mqttServer->pktrecvLasttime = node->getNodeTime();
    //mqttServer->sessionIsClosed = FALSE;
	mqttServer->tos = tos;
    mqttServer->seqNo = 0;
    mqttServer->uniqueId = node->appData.uniqueId++;
	
    mqttServer->clientUrl = new std::string();
    mqttServer->clientUrl->clear();

    mqttServer->stats = NULL;
    if (appName)
    {
        mqttServer->applicationName = new std::string(appName);
    }
    else
    {
        mqttServer->applicationName = new std::string();
    }
#ifdef ADDON_DB
    mqttServer->sessionId = -1;
    mqttServer->sessionInitiator = 0;
    mqttServer->hopCount = 0;
    mqttServer->seqNumCache = NULL;
#endif // ADDON_DB

    APP_RegisterNewApp(node, APP_MQTT_SERVER, mqttServer);

    return mqttServer;
}

MQTTServerAllData *
AppMQTTServerNewServerAll(Node *node)
{
	
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ServerNewServerAll#\n",node->getNodeTime(),node->nodeId);
#endif


	MQTTServerAllData *serverAll;

	serverAll = (MQTTServerAllData *)
		MEM_malloc(sizeof(MQTTServerAllData));
	memset(serverAll, 0, sizeof(MQTTServerAllData));

	serverAll->numDisconnectNodes = 0;
	serverAll->numActiveNodes = 0;
	serverAll->numSleepNodes =0;
	serverAll->numLostNodes = 0;

	APP_RegisterNewApp(node, APP_MQTT_SERVER_ALL, serverAll);

	return serverAll;

}



void
AppMQTTInitTrace(Node *node, const NodeInput *nodeInput)
{
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #InitTrace#\n",node->getNodeTime(),node->nodeId);
#endif	
}

AppAddressState
AppMQTTClientGetSessionAddressState(Node *node, MQTTClientData *clientPtr){
	
	printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ClientGetSessionAddressState#\n",node->getNodeTime(),node->nodeId);
    // variable to determine the server current address state
    Int32 serverAddresState = 0;
    // variable to determine the client current address state
    Int32 clientAddressState = 0;

    // Get the current client and destination address if the
    // session is starting
    // Address state is checked only at the session start; if this is not
    // starting of session then return FOUND_ADDRESS
	
    if (clientPtr->seqNo == 0)
    {
        clientAddressState =
            MAPPING_GetInterfaceAddrForNodeIdAndIntfId(
                                    node,
                                    node->nodeId,
                                    clientPtr->clientInterfaceIndex,
                                    clientPtr->localAddr.networkType,
                                    &(clientPtr->localAddr));

        if (NetworkIpCheckIfAddressLoopBack(node, clientPtr->remoteAddr))
        {
            serverAddresState = clientAddressState;
        }
        else if (clientPtr->destNodeId != ANY_NODEID &&
            clientPtr->destInterfaceIndex != ANY_INTERFACE)
        {
            serverAddresState =
                MAPPING_GetInterfaceAddrForNodeIdAndIntfId(
                                    node,
                                    clientPtr->destNodeId,
                                    clientPtr->destInterfaceIndex,
                                    clientPtr->remoteAddr.networkType,
                                    &(clientPtr->remoteAddr));
        }
    }
    // if either client or server are not in valid address state
    // then mapping will return INVALID_MAPPING
    if (clientAddressState == INVALID_MAPPING ||
        serverAddresState == INVALID_MAPPING)
    {
        return ADDRESS_INVALID;
    }
    return ADDRESS_FOUND;
}


void
AppMQTTClientAddAddressInformation(Node* node,
                                  MQTTClientData* clientPtr)
{
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ClientAddAddressInfomation#\n",node->getNodeTime(),node->nodeId);
#endif
    // Store the client and destination interface index such that we can get
    // the correct address when the application starts
    NodeId destNodeId = MAPPING_GetNodeIdFromInterfaceAddress(
                                                node,
                                                clientPtr->remoteAddr);
    if (destNodeId > 0)
    {
        clientPtr->destNodeId = destNodeId;
        clientPtr->destInterfaceIndex =
            (Int16)MAPPING_GetInterfaceIdForDestAddress(
                                                node,
                                                destNodeId,
                                                clientPtr->remoteAddr);
    }
    // Handle loopback address in destination
    if (destNodeId == INVALID_MAPPING)
    {
        if (NetworkIpCheckIfAddressLoopBack(node, clientPtr->remoteAddr))
        {
            clientPtr->destNodeId = node->nodeId;
            clientPtr->destInterfaceIndex = DEFAULT_INTERFACE;
        }
    }
    clientPtr->clientInterfaceIndex =
        (Int16)MAPPING_GetInterfaceIndexFromInterfaceAddress(
                                            node,
                                            clientPtr->localAddr);
}

void
MQTTDataTypeToString(MQTTDataType type, string *str){
	char error[MAX_STRING_LENGTH];
	string ch;
	switch(type){
		case TY_CONNECT:
			{
				ch = "TY_CONNECT";
				break;
			}
		case TY_CONACK:
			{
				ch = "TY_CONACK";
				break;
			}
		case TY_PUBLISH:
			{
				ch = "TY_PUBLISH";
				break;
			}
		case TY_DISCONNECT:
			{
				ch = "TY_DISCONNECT";
				break;
			}
		default:
			{
				ch = "";
				sprintf(error,"not found such a MQTT Data Type\n");
				ERROR_ReportError(error);
			}
	}
	*str = ch;
}

void
MQTTNodeStatusToString(MQTTNodeStatus status, string *str){
	char error[MAX_STRING_LENGTH];
	string ch;
	switch(status){
		case ST_DISCONNECT:
			{
				ch = "ST_DISCONNECT";
				break;
			}
		case ST_ACTIVE:
			{
				ch = "ST_ACTIVE";
				break;
			}
		case ST_SLEEP:
			{
				ch = "ST_SLEEP";
				break;
			}
		case ST_LOST:
			{
				ch = "ST_LOST";
				break;
			}
		default:
			{
				ch ="";
				sprintf(error,"not found such a MQTT Node Status\n");
				ERROR_ReportError(error);
			}
	}
	*str = ch.c_str();
}


//set client data and num client status
void
MQTTServerSetStatusData(MQTTServerAllData *serverAll, Address clientAddr, MQTTNodeStatus status)
{
#ifdef KANDEB
		printf("mqtt.cpp in the #SetStatsData#\n");
#endif
	
	char i_addrStr[MAX_STRING_LENGTH];
	char c_addrStr[MAX_STRING_LENGTH];
	MQTTNodeStatus oldStatus;
	bool isFound = false;

	char error[MAX_STRING_LENGTH];

	IO_ConvertIpAddressToString(&clientAddr, c_addrStr);

	for(int i = 0; i != serverAll->clients.size(); ++i){
		//IO_ConvertIpAddressToString(serverAll->clients[i].clientAddr, i_addrStr);
		//if(strcmp(i_addrStr,c_addrStr)==0){
		if(strcmp(c_addrStr,serverAll->clients[i].clientAddr)==0){
			oldStatus = serverAll->clients[i].nodeStatus;
			serverAll->clients[i].nodeStatus = status;

			switch(status){
				case ST_DISCONNECT:
				{
					serverAll->numDisconnectNodes++;
					break;
				}
				case ST_ACTIVE:
				{
					serverAll->numActiveNodes++;
					break;
				}
				case ST_SLEEP:
				{
					serverAll->numSleepNodes++;
					break;
				}
				case ST_LOST:
				{
					serverAll->numLostNodes++;
					break;
				}
				default:
				{
					sprintf(error,"not found such a MQTT Node Status\n");
					ERROR_ReportError(error);
				}
			}
			switch(oldStatus){
				case ST_DISCONNECT:
				{
					serverAll->numDisconnectNodes--;
					break;
				}
				case ST_ACTIVE:
				{
					serverAll->numActiveNodes--;
					break;
				}
				case ST_SLEEP:
				{
					serverAll->numSleepNodes--;
					break;
				}
				case ST_LOST:
				{
					serverAll->numLostNodes--;
					break;
				}
				default:
				{
					sprintf(error,"not found such a MQTT Node Status\n");
					ERROR_ReportError(error);
				}
			}
			if(serverAll->numDisconnectNodes<0 || 
					serverAll->numActiveNodes<0 ||
					serverAll->numSleepNodes<0 ||
					serverAll->numLostNodes<0){
					sprintf(error,"MQTTserverAllData is <0, All num data must >=0\n");
					ERROR_ReportError(error);
			}
			isFound = true;

		}
	}
		//client is first connection. status should active and add client to clients
	if(isFound == false){
#ifdef DEBUG
		{
			char addr[MAX_STRING_LENGTH];
			IO_ConvertIpAddressToString(&clientAddr, addr);
			printf("Add new Client to AddServer data\n"
				"client address is %s\n",addr);
		}
#endif
		switch(status){
			case ST_ACTIVE:
			{
				serverAll->numActiveNodes++;
				break;
			}
			default:
			{
				sprintf(error,"At first, client status will be Active.\n"
						"Or this client is NOT Registration\n");
				ERROR_ReportError(error);
			}
		}

		MQTTNodeStatusData newData;
		//newData.clientAddr = &clientAddr;
		IO_ConvertIpAddressToString(&clientAddr, newData.clientAddr); //test
		newData.nodeStatus = ST_ACTIVE;
		serverAll->clients.push_back(newData);
	}
#ifdef DEBUG
	MQTTPrintServerAllData(serverAll);
#endif
}

MQTTNodeStatus
MQTTServerGetStatusData(MQTTServerAllData *serverAll, Address clientAddr){
	char cliaddr[MAX_STRING_LENGTH];
	IO_ConvertIpAddressToString(&clientAddr, cliaddr);

	for(int i = 0; i != serverAll->clients.size(); ++i){
		if(strcmp(serverAll->clients[i].clientAddr,cliaddr)==0)
			return serverAll->clients[i].nodeStatus;
	}
	char error[MAX_STRING_LENGTH];
	sprintf(error,"Not found such a client.  Cant get NodeStatus\n");
	ERROR_ReportError(error);
}

void
MQTTPrintServerAllData(MQTTServerAllData *serverAll)
{
	printf("---------------- MQTTserverAllData ----------------\n");
	char addr[MAX_STRING_LENGTH];
	std::string status;

	for(int i = 0; i != serverAll->clients.size(); ++i){
		IO_ConvertIpAddressToString(serverAll->clients[i].clientAddr, addr);
		MQTTNodeStatusToString(serverAll->clients[i].nodeStatus,&status);

		printf("clients[ %d ] : address[ %s ]\n"
				"               status[ %s ]\n"
//				,i,addr,status.c_str());
				,i,serverAll->clients[i].clientAddr,status.c_str());
	}
	printf("\nStatus count\n");
	printf("DISCONNECT : %d\nACTIVE : %d\nSLEEP : %d\nLOST : %d\n"
			,serverAll->numDisconnectNodes
			,serverAll->numActiveNodes
			,serverAll->numSleepNodes
			,serverAll->numLostNodes);
	printf("---------------------------------------------------\n");
}

void
AppMQTTClientPrintStats(Node *node, MQTTClientData *clientPtr){

#ifdef KANDEB
	printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ClientPrintStats#\n",node->getNodeTime(),node->nodeId);
#endif

    char addrStr[MAX_STRING_LENGTH];
    char sessionStatusStr[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if (clientPtr->remoteAddr.networkType != NETWORK_INVALID) {
    	clientPtr->sessionFinish = node->getNodeTime();
       	sprintf(sessionStatusStr, "Not closed");
       	if (!clientPtr->stats->IsSessionFinished(STAT_Unicast))
            clientPtr->stats->SessionFinish(node);
    }
    else {
        sprintf(sessionStatusStr, "Closed");
    }

    if (clientPtr->remoteAddr.networkType == NETWORK_ATM)
    {
        const LogicalSubnet* dstLogicalSubnet =
            AtmGetLogicalSubnetFromNodeId(
            node,
            clientPtr->remoteAddr.interfaceAddr.atm.ESI_pt1,
            0);
        IO_ConvertIpAddressToString(
            dstLogicalSubnet->ipAddress,
            addrStr);
    }
    else
    {
        // dns
        if (clientPtr->remoteAddr.networkType != NETWORK_INVALID)
        {
            IO_ConvertIpAddressToString(&clientPtr->remoteAddr, addrStr);
        }
        else
        {
            strcpy(addrStr, (char*)clientPtr->serverUrl->c_str());
        }
    }

    sprintf(buf, "Server Address = %s", addrStr);
    IO_PrintStat(
        node,
        "Application",
        "MQTT Client",
        ANY_DEST,
        clientPtr->sourcePort,
        buf);

    sprintf(buf, "Session Status = %s", sessionStatusStr);
    IO_PrintStat(
        node,
        "Application",
        "MQTT Client",
        ANY_DEST,
        clientPtr->sourcePort,
        buf);
}

void
AppMQTTServerPrintStats(Node *node, MQTTServerData *serverPtr){

#ifdef KANDEB
	printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ServerPrintStats#\n",node->getNodeTime(),node->nodeId);
#endif

    char addrStr[MAX_STRING_LENGTH];
    char sessionStatusStr[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

  	serverPtr->sessionFinish = node->getNodeTime();
	sprintf(sessionStatusStr, "Not closed");
   	if (!serverPtr->stats->IsSessionFinished(STAT_Unicast))
            serverPtr->stats->SessionFinish(node);
    else
        sprintf(sessionStatusStr, "Closed");

    IO_ConvertIpAddressToString(&serverPtr->remoteAddr, addrStr);

    sprintf(buf, "Client address = %s", addrStr);
    IO_PrintStat(
        node,
        "Application",
        "MQTT Server",
        ANY_DEST,
        serverPtr->sourcePort,
        buf);


    sprintf(buf, "Session Status = %s", sessionStatusStr);
    IO_PrintStat(
        node,
        "Application",
        "MQTT Server",
        ANY_DEST,
        serverPtr->sourcePort,
        buf);
}

void
AppMQTTServerAllPrintStats(Node *node, MQTTServerAllData *serverAll){
#ifdef KANDEB
	printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #ServerAllPrintStats#\n",node->getNodeTime(),node->nodeId);
#endif
}

void
MQTTPrintMQTTData(MQTTData data){
	char buf[MAX_STRING_LENGTH];
	string str;
	printf("MQTTData packet\n");
	MQTTDataTypeToString(data.dataType, &str);
	printf("    dataType  : %s\n",str.c_str());
	TIME_PrintClockInSecond(data.txTime, buf);
	printf("    txTime    : %s\n",buf);
	printf("    sourcePort: %d\n",data.sourcePort);
	printf("    seqNo     : %d\n",data.seqNo);
	printf("    appName   : %s\n",data.appName);

}

bool
AppMQTTUrlSessionStartCallBack(
		Node *node,
		Address *serverAddr,
		UInt16 sourcePort,
		Int32 uniqueId,
		Int16 interfaceId,
		std::string& serverUrl,
		struct AppUdpPacketSendingInfo *packetSendingInfo)
{
#ifdef KANDEB
		printf("%" TYPES_64BITFMT "d : Node %d: mqtt.cpp in the #SessionStartCallBack#\n",node->getNodeTime(),node->nodeId);
#endif
}

