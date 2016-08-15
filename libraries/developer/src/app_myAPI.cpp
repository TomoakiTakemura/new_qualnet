#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "api.h"
#include "partition.h"
#include "app_util.h"

#include "mydef.h"
#include "app_myAPI.h"
#include "mac_802_15_4.h"

#define KANDEB 1
#define DEBUG 1
#define DEBUG_TIMER 1

#ifdef CHANGE_BI
UInt8
AppServerMakeRecmBO(MQTTServerAllData *serverAll){
	int disc = serverAll->numDisconnectNodes;
	int act = serverAll->numActiveNodes;
	int sleep = serverAll->numSleepNodes;
	int lost = serverAll->numLostNodes;

	//make RecmBI
	//caution SO <= RecmBO <=BO
	
	return 0; //test
}

void
AppServerSetRecmBO(Node *node, Int32 interfaceIndex, UInt8 RecmBO){
	MacData802_15_4* mac = NULL;
	mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;
	if(mac == NULL){
		printf("cant get macData!!!\nfaild to set RecmBO\n");
	}
	mac->RecmBO = RecmBO;
}

void
MacServerRecmBIToBO(UInt8 RecmBO){

}

MQTTClientData*
MacClientGetClientApp(Node *node){
	AppInfo *appList = node->appData.appPtr;
	MQTTClientData *mqttClient;
	int count = 0;
	char error[MAX_STRING_LENGTH];

	for (; appList != NULL; appList = appList->appNext){
		if (appList->appType == APP_MQTT_CLIENT){
			mqttClient = (MQTTClientData *) appList->appDetail;
			count++;
		}
	}
	if (count != 1){
		//not found or any client found.
		//cant use my API
		sprintf(error, "any or not found MQTT client in node\n");
		ERROR_ReportError(error);
	}
	return mqttClient;
}

bool
MacClientIsUsedBeacon(Node *node, Message *msg, MacData802_15_4 *mac){
	MyApiBeaconData *bcnInfo;
	MQTTClientData *client;
	
	bcnInfo = (MyApiBeaconData *)MESSAGE_ReturnInfo(msg, INFO_TYPE_MYAPI_BEACON);

	client = MacClientGetClientApp(node);
	if(bcnInfo->isVariableBeacon && client->func != FN_SEND_CONNECT)
		return false;
	return true;
}

clocktype MyApiCalculateVarBcnRxTimer(Node* node, Int32 interfaceIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: myAPI.cpp in the #CalculateVarBcnRxTimer#\n",node->getNodeTime(),node->nodeId);
}

    MacData802_15_4* mac = NULL;
    clocktype BI = 0;
    clocktype bcnRxTime = 0;
    clocktype now = 0;
    clocktype len12s = 0;
    clocktype wtime = 0;
    clocktype tmpf = 0;

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;
     BI = ((aBaseSuperframeDuration * (1 << mac->RecmBO)) * SECOND)
                / Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]);

    bcnRxTime = mac->macBcnRxTime;
    now = node->getNodeTime();
    while (now - bcnRxTime > BI)
    {
        bcnRxTime += BI;
    }

    len12s = 12 * SECOND
            / Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]);

    tmpf = (now - bcnRxTime);
    wtime = BI - tmpf;

    if (wtime >= len12s)
    {
        wtime -= len12s;
    }

    tmpf = now + wtime;
   if (tmpf - mac->macBcnRxLastTime < BI - len12s)
    {
        tmpf = 2 * BI;
        tmpf = tmpf - now;
        tmpf = tmpf + bcnRxTime;
        wtime = tmpf - len12s;
    }

    mac->macBcnRxLastTime = now + wtime ;

    if (DEBUG_TIMER)
    {
        printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4MAC : Rx Timer"
                     " scheduled at %" TYPES_64BITFMT "d from "
                     " CalculateBcnRxTimer \n",
                    node->getNodeTime(),
                    node->nodeId,
                    now + wtime);
    }
    return wtime;
}

void MyApiVarBeaconRxHandler(Node* node, Int32 interfaceIndex)
{
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: myAPI.cpp in the #BeaconRxHandler#\n",node->getNodeTime(),node->nodeId);
}

    MacData802_15_4* mac = NULL;
    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;

    if (DEBUG_TIMER)
    {
        printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4MAC : Var Beacon Rx"
                "Timer expired\n", node->getNodeTime(), node->nodeId);
    }

    if (mac->macBeaconOrder2 != 15)      // beacon enabled
    {
        if (mac->txAck)
        {
            MESSAGE_Free(node, mac->txAck);
            mac->txAck = NULL;
        }

        // enable the receiver
        mac->trx_state_req = RX_ON;
        Phy802_15_4PlmeSetTRX_StateRequest(node,
                                           interfaceIndex,
                                           RX_ON);
        if (mac->taskP.mlme_sync_request_tracking)
        {
            if (mac->numLostBeacons > aMaxLostBeacons)
            {
                Sscs802_15_4MLME_SYNC_LOSS_indication(node,
                                                      interfaceIndex,
						                              M802_15_4_BEACON_LOSS);
                mac->numLostBeacons = 0;
            }
            else
            {
				//caluclate number of Variable Beacon during a normal BI
				int numVarB=1, i;
				for(i=0;i<mac->macBeaconOrder2 - mac->LastMyBeacon.VariableBO;i++){
					numVarB = numVarB * 2;
				}
				numVarB-=1;
				if(numVarB == 0){
					char error[MAX_STRING_LENGTH];
					sprintf(error, "Call Handler for VarBeacon. But VarB is 0\n");
					ERROR_ReportError(error);
				}
				//end calculate
				if(numVarB == mac->numVarBcnHandler){
					//next beacon is NormalBeacon. VarB_Handler is not set.
					if(DEBUG){
						printf("VarBcnRxHandler : next bcn is normal bcn\n"
								"                  not set timer for bcn\n");
					}
					mac->bcnRxT = mac->NorBcnRxT;
				}
				else{
                	mac->numLostBeacons++;

                	if (DEBUG_TIMER){
                    	printf("%" TYPES_64BITFMT "d : Node %d: 802.15.4MAC : "
                           "Set Rx timer"
                           " called from VarBcnRxHandler\n",
                           node->getNodeTime(),
                           node->nodeId );
                	}
               		mac->VarBcnRxT = Mac802_15_4SetTimer(
                                        node,
                                        mac,
                                        M802_15_4VARBEACONRXTIMER,
                                        MyApiCalculateVarBcnRxTimer(node,
                                            interfaceIndex),
                                        NULL);
					mac->bcnRxT = mac->VarBcnRxT;
				}
            }
        }
    }
}

clocktype MyApiCalculateVarBcnRxTimer2(Node *node, Int32 interfaceIndex){
if (KANDEB){
	printf("%" TYPES_64BITFMT "d : Node %d: .myAPPcpp in the #CalculateVarBcnRxTimer2#\n",node->getNodeTime(),node->nodeId);
}

    MacData802_15_4* mac = NULL;
    clocktype BI = 0;
    clocktype bcnRxTime = 0;
    clocktype now = 0;
    clocktype len12s = 0;

    mac = (MacData802_15_4*) node->macData[interfaceIndex]->macVar;

    BI = ((aBaseSuperframeDuration * (1 << mac->LastMyBeacon.VariableBO)) * SECOND)
              / Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]);

    bcnRxTime = mac->macBcnRxTime;
    now = node->getNodeTime();
    len12s = 12 * SECOND
                / Phy802_15_4GetSymbolRate(node->phyData[interfaceIndex]);
   return bcnRxTime + BI - len12s - now;
}
#endif //CHANGE_BI
