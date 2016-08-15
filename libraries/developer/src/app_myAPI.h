#ifndef MYAPI_H
#define MYAPI_H

#include "node.h"
#include "types.h"

#include "mydef.h"
#include "app_mqtt.h"
#include "mac_802_15_4.h"

#ifdef CHANGE_BI
//configuration RecomBI used by MQTT stats data
//return Recm BO
//
UInt8
AppServerMakeRecmBO(MQTTServerAllData *serverAll);

//set new RecmBI
void
AppServerSetRecmBO(Node *node, Int32 interfaceIndex, UInt8 RecmBO);

//change variable BI to RecmBI
void
MacServerRecmBOToBO(UInt8 RecmBO);

MQTTClientData*
MacClientGetClientApp(Node *node);

//decide this beacon should be received.
//msg must be beacon
bool
MacClientIsUsedBeacon(Node *node, Message *msg, MacData802_15_4 *mac);

void
MyApiVarBeaconRxHandler(Node* node, Int32 interfaceIndex);

clocktype 
MyApiCalculateVarBcnRxTimer(Node* node, Int32 interfaceIndex);

clocktype 
MyApiCalculateVarBcnRxTimer2(Node* node, Int32 interfaceIndex);

#endif//CHANGE_BI

#endif //_MYAPI_H_
