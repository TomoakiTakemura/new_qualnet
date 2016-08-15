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

#include "api.h"
#include "external_socket.h"
#include "partition.h"
#ifdef USE_MPI
#include "external_util.h"
#endif

#include "multi_gui_interface.h"
#include "external_util.h"
#include "ClientIDGenerator.h"

#ifdef AUTO_IPNE_INTERFACE
#include "auto-ipnetworkemulator.h"
#ifdef _WIN32
#include "winsock.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#endif

// Default port number
#define MULTI_GUI_DEFAULT_PORT 4100
// Max number of attemps to open port
#define MULTIPLE_GUI_INTERFACE_NUMBER_ATTEMPTS  20

// Global variable of multiGuiIntf
EXTERNAL_Interface *multiGuiIntf = NULL;

#define DEBUG             0

struct ConnectToProxy
{
    EXTERNAL_Socket* listenSocket;
    std::vector<EXTERNAL_Socket*> connectSocket;
    BOOL debug;
};

void MultiGuiIntf_Initialize(EXTERNAL_Interface *iface,
                        NodeInput *nodeInput)
{
    multiGuiIntf = iface;
    // Only initialize on partition 0
    if (iface->partition->partitionId != 0)
    {
        return;
    }

    int intBuf;
    int portNumber;
    BOOL retVal;
    EXTERNAL_Socket* listenSocket;
    char errString[MAX_STRING_LENGTH];
    EXTERNAL_SocketErrorType err;

    ConnectToProxy* connectToProxy = new ConnectToProxy;

    // Read the debug
    IO_ReadBool(ANY_NODEID,
                ANY_ADDRESS,
                nodeInput,
                "MULTI-GUI-INTERFACE-DEBUG-ENABLED",
                &retVal,
                &connectToProxy->debug);
    if (retVal)
    {
        // Debug was set
    }
    else
    {
        connectToProxy->debug = false;
    }

    bool multiGuiEnabled = false;
    if (GUI_isMultigui())
    {
        portNumber = getMultiguiPort();
        multiGuiEnabled = true;
    }
    else
    {
        // Read the port for each socket
        IO_ReadInt( ANY_NODEID,
                    ANY_ADDRESS,
                    nodeInput,
                    "MULTI-GUI-INTERFACE-PORT",
                    &retVal,
                    &portNumber);
        setMultiguiEnabled();
    }
    if (retVal || multiGuiEnabled)
    {
        if (portNumber <0 || portNumber > 65535)
        {
            sprintf(errString,"Invalid MULTI-GUI-INTERFACE-PORT %d\n",portNumber);
            ERROR_ReportError(errString);
        }

        if (connectToProxy->debug)
        {
            printf("PORT for MULTI-GUI-INTERFACE is %d \n",portNumber);
        }
    }
    else
    {
        portNumber = MULTI_GUI_DEFAULT_PORT;
    }

    listenSocket = (EXTERNAL_Socket*)MEM_malloc(sizeof(EXTERNAL_Socket));
    EXTERNAL_SocketInit(listenSocket,FALSE, FALSE);

    unsigned int counter = 0 ;
    do
    {
        err = EXTERNAL_SocketInitListen(listenSocket,
                                        portNumber,
                                        FALSE,
                                        FALSE);
        counter++;
        if (err != EXTERNAL_NoSocketError)
        {
            sprintf(
                errString,
                "Unable to open socket on port %d.  Trying again in 3 seconds.",
                portNumber);
            ERROR_ReportWarning(errString);
            EXTERNAL_Sleep(3 * SECOND);
        }
        else
        {
            if (connectToProxy->debug)
            {
                printf("MULTI-GUI-INTERFACE opened listenning port %d \n",portNumber);
            }
            break;
        }
    } while (counter <= MULTIPLE_GUI_INTERFACE_NUMBER_ATTEMPTS);

    if (err != EXTERNAL_NoSocketError)
    {
        iface->data = NULL;
        delete(connectToProxy);

        sprintf(
            errString,
            "Unable to open socket on port %d. Maximum retry limit is reached",
            portNumber);
        ERROR_ReportWarning(errString);
        return;
    }

    connectToProxy->listenSocket = listenSocket;
    iface->data = (void*) connectToProxy;
}

void MultiGuiIntf_Receive(EXTERNAL_Interface *iface)
{
    // Only receive on partition 0
    if (iface->partition->partitionId != 0 || iface->data == NULL)
    {
        return;
    }

    ConnectToProxy* connectToProxy;
    EXTERNAL_Socket* connectSocket;
    bool socketDataAvail;
    EXTERNAL_SocketErrorType error;
    char command[BIG_STRING_LENGTH];
    unsigned int size = 0;
    char errorStr[MAX_STRING_LENGTH];

    connectToProxy = (ConnectToProxy*)iface->data;
    socketDataAvail = FALSE;

    if (EXTERNAL_SocketValid(connectToProxy->listenSocket))
    {
        EXTERNAL_SocketDataAvailable(connectToProxy->listenSocket,
                                &socketDataAvail);
        if (socketDataAvail)
        {
            connectSocket = (EXTERNAL_Socket*)MEM_malloc(sizeof(EXTERNAL_Socket));
            EXTERNAL_SocketInit(connectSocket,FALSE, FALSE);
            error = EXTERNAL_SocketAccept(connectToProxy->listenSocket, connectSocket);
            if (error != EXTERNAL_NoSocketError)
            {
                sprintf(errorStr,
                        "MULTI-GUI-INTERFACE gets error %d when accepting the connection",
                        error);
                ERROR_ReportError(errorStr);
            }
            else
            {
                connectToProxy->connectSocket.push_back(connectSocket);
                MultiGuiIntf_SendUidToClient(connectSocket);
                if (connectToProxy->debug)
                {
                    printf("MasterSimIntf: Got connected to proxy #%ld \n",
                    connectToProxy->connectSocket.size());
                }
            }
        }
    }

    for (int i=0 ; i < connectToProxy->connectSocket.size() ; i++)
    {
        socketDataAvail = FALSE;
        connectSocket = connectToProxy->connectSocket[i];
        if (EXTERNAL_SocketValid(connectSocket))
        {

            memset(command,0,sizeof(command));

            EXTERNAL_SocketDataAvailable(connectSocket,
                                        &socketDataAvail);
            if (socketDataAvail)
            {
                 error = EXTERNAL_SocketRecv(connectSocket,
                                            command,
                                            BIG_STRING_LENGTH,
                                            &size,
                                            FALSE);

                 if (error != EXTERNAL_NoSocketError)
                 {
                     sprintf(errorStr,
                            "MULTI-GUI-INTERFACE gets error %d when reading socket. Closing the socket.\n",
                            error);
                     ERROR_ReportWarning(errorStr);
                     EXTERNAL_SocketClose(connectSocket);
                     // Remove the socket from the list
                     connectToProxy->connectSocket.erase(connectToProxy->connectSocket.begin() + i);
                     MEM_free(connectSocket);
                     // Since we delete an element the vector would be reorganized
                     i--;
                     continue;
                 }

                 if (size > 0)
                 {
                     GuiCommands type = (GuiCommands)atoi(command);
                     int pos = std::string(command).find(" ");
                     if (pos != string::npos)
                     {
                         char commandStr[BIG_STRING_LENGTH] = { 0 };
                         // skipping the found space
                         pos++;
                         memcpy(commandStr, &command[pos], size - pos);
                         switch (type)
                         {
                             case GuiCommands::GUI_STEP:
                             {
                                 if (connectToProxy->debug)
                                 {
                                     printf("MULTI-GUI-INTERFACE got GUI_STEP command\n");
                                 }
                                 MultiGuiIntf_RequestForOlderEvents(iface, connectSocket->socketFd);
                                 break;
                             }
                             case GuiCommands::GUI_USER_DEFINED:
                             {
                                 if (connectToProxy->debug)
                                 {
                                     printf("MULTI-GUI-INTERFACE got HITL string from proxy: \n    %s\n",
                                         commandStr);
                                 }
                                 GUI_HandleHITLInput(commandStr, iface->partition);
                                 break;
                             }
                         }
                     }
                 }
            }
        }
    }// End of for (int i=0 ; i < connectToProxy->connectSocket.size() ; i++)
}

void MultiGuiIntf_Send(char* outStr,
                        unsigned int size,
                        Int64 connectionId)
{
    if (multiGuiIntf == NULL)
    {
        return;
    }

#ifdef USE_MPI
    // In MPI mode, scheduling message for partition 0
    if (multiGuiIntf->partition->partitionId > 0)
    {
        GuiReplies reply;
        sscanf(outStr,"%d",(int*)&reply);
        if (reply == GUI_FINISHED)
        {
            return;
        }

        Node* node = multiGuiIntf->partition->firstNode;
        // Set eventType = 0 as EXTERNAL_MULTI_GUI has only one event type so far
        int eventType  = 0;
        Message* msg = MESSAGE_Alloc(node,
                                    EXTERNAL_LAYER,
                                    EXTERNAL_MULTI_GUI,
                                    eventType);

        MultiGuiInterfaceMsgType type;
        UInt32 newSize = 0;
        UInt32 uSize = size;
        if (connectionId == -1)
        {
            type = SEND_MESG;
            newSize = uSize + sizeof(type) + sizeof(uSize);
        }
        else
        {
            type = SEND_GUI_JOIN_EVENTS;
            newSize = uSize + sizeof(type) + sizeof(uSize) + 2 * sizeof(connectionId);
        }
        char* newBuffer = (char*)MEM_malloc(newSize);
        UInt32 index = 0;
        memcpy(newBuffer, &type, sizeof(type));
        index += sizeof(type);
        memcpy(newBuffer + index, &uSize, sizeof(uSize));
        index += sizeof(uSize);
        memcpy(newBuffer + index, outStr, uSize);
        if (connectionId != -1)
        {
            index += size;
            int connectionIdSize = sizeof(connectionId);
            memcpy(newBuffer + index, &connectionIdSize, sizeof(connectionId));
            index += sizeof(connectionId);
            memcpy(newBuffer + index, &connectionId, sizeof(connectionId));
        }

        MESSAGE_AddInfo(node, msg, newSize);
        memcpy(MESSAGE_ReturnInfo(msg), newBuffer, newSize);
        MEM_free(newBuffer);

        EXTERNAL_MESSAGE_RemoteSend(multiGuiIntf->partition,0, msg,0, EXTERNAL_SCHEDULE_SAFE);
        return;
    }
#endif

    ConnectToProxy* connectToProxy;
    EXTERNAL_SocketErrorType error;
    EXTERNAL_Socket* connectSocket;
    char errorStr[MAX_STRING_LENGTH];

    connectToProxy = (ConnectToProxy*)multiGuiIntf->data;
    for (int i = 0; i < connectToProxy->connectSocket.size(); i++)
    {
        connectSocket = connectToProxy->connectSocket[i];
        if (connectionId == -1 || connectSocket->socketFd == connectionId)
        {
            if (EXTERNAL_SocketValid(connectSocket))
            {
                GuiReplies reply;
                sscanf(outStr, "%d", (int*)&reply);
                if (reply == GUI_FINISHED)
                {
                    error = EXTERNAL_SocketSend(connectSocket,
                        outStr,
                        size,
                        TRUE);
                    EXTERNAL_Sleep(1 * MILLI_SECOND);
                    EXTERNAL_SocketClose(connectSocket);
                }
                else
                {
                    error = EXTERNAL_SocketSend(connectSocket,
                        outStr,
                        size,
                        FALSE);
                }
                if (error != EXTERNAL_NoSocketError)
                {
                    sprintf(errorStr,
                        "MULTI-GUI-INTERFACE gets error %d when sending on socket",
                        error);
                    ERROR_ReportWarning(errorStr);
                    continue;
                }
            }
            else
            {
                sprintf(errorStr, "MULTI-GUI-INTERFACE: connectSocket is no longer valid");
                ERROR_ReportWarning(errorStr);
                continue;
            }
            if (connectSocket->socketFd == connectionId)
            {
                break;
            }
        }
    }// End of for (int i=0 ; i < connectToProxy->connectSocket.size() ; i++)
}

void MultiGuiIntf_SendUidToClient(EXTERNAL_Socket* connectSocket)
{
    if (multiGuiIntf == NULL)
    {
        return;
    }

    std::string clientID = ClientIdGenerator::generateClientId(LEGACY_CLIENT);

    char buffer[10] = {0};
    std::string outString;

    sprintf(buffer, "%d ", GUI_UID);
    outString.append(buffer);
    outString.append(clientID);
    outString.append("\n");
    const char* outStr = outString.c_str();
    UInt32 size = outString.size();

#ifdef USE_MPI
    // In MPI mode, scheduling message for partition 0
    if (multiGuiIntf->partition->partitionId > 0)
    {
        Node* node = multiGuiIntf->partition->firstNode;
        // Set eventType = 0 as EXTERNAL_MULTI_GUI has only one event type so far
        int eventType  = 0;
        Message* msg = MESSAGE_Alloc(node,
                                     EXTERNAL_LAYER,
                                     EXTERNAL_MULTI_GUI,
                                     eventType);

        MultiGuiInterfaceMsgType type = SEND_MESG;
        unsigned int newSize = size + sizeof(type) + sizeof(size);
        char* newBuffer = (char*)MEM_malloc(newSize);
        int index = 0;
        memcpy(newBuffer, &type, sizeof(type));
        index += sizeof(type);
        memcpy(newBuffer + index, &size, sizeof(size));
        index += sizeof(size);
        memcpy(newBuffer + index, outStr, size);

        MESSAGE_AddInfo(node, msg, newSize);
        memcpy(MESSAGE_ReturnInfo(msg), newBuffer, newSize);

        EXTERNAL_MESSAGE_RemoteSend(multiGuiIntf->partition,
                                    0,
                                    msg,
                                    0,
                                    EXTERNAL_SCHEDULE_SAFE);
        return;
    }
#endif

    EXTERNAL_SocketErrorType error;
    char errorStr[MAX_STRING_LENGTH];

    if (EXTERNAL_SocketValid(connectSocket))
    {
        error = EXTERNAL_SocketSend(connectSocket,
                                    outStr,
                                    size,
                                    TRUE);

        if (error != EXTERNAL_NoSocketError)
        {
            sprintf(errorStr,
                    "MULTI-GUI-INTERFACE gets error %d when sending on socket",
                    error);
            ERROR_ReportWarning(errorStr);
        }
    }
    else
    {
        sprintf(errorStr,"MULTI-GUI-INTERFACE: connectSocket is no longer valid");
        ERROR_ReportWarning(errorStr);
    }
}


void MultiGuiIntf_ProcessEvent(Node* node, Message* msg)
{
    PartitionData* partition = node->partitionData;
    char* msgInfo = (char*)MESSAGE_ReturnInfo(msg);
    if (msgInfo == NULL)
    {
        return;
    }
    unsigned int msgInfoLen = MESSAGE_ReturnInfoSize(msg);
    if (msgInfoLen == 0)
    {
        return;
    }
    MultiGuiInterfaceMsgType type;
    int index = 0;
    memcpy(&type, msgInfo, sizeof(type));
    index += sizeof(type);
    UInt32 size;
    memcpy(&size, msgInfo + index, sizeof(size));
    index += sizeof(size);
    char* outStr = NULL;
    if (size > 0)
    {
        outStr = msgInfo + index;
    }
    char* extraOptions = NULL;
    int extraOptLen = 0;
    if (index + size < msgInfoLen)
    {
        memcpy(&extraOptLen, msgInfo + index, sizeof(extraOptLen));
        index += sizeof(extraOptLen);
        extraOptions = msgInfo + index; 
    }

    switch (type)
    {
        case SEND_MESG:
        {
            MultiGuiIntf_Send(outStr, size);
            break;
        }
        case SEND_GUI_JOIN_EVENTS:
        {
            Int64 connectionId = *(Int64*)extraOptions;
            MultiGuiIntf_Send(outStr, size, connectionId);
            break;
        }
        case REQUEST_OLDER_EVENTS:
        {
            Int64 connectionId = *(Int64*)outStr;
#ifdef AUTO_IPNE_INTERFACE
            if (partition->partitionId == 0)
            {
                MultiGuiIntf_SendMapping(partition, connectionId);
            }
#endif
            MultiGuiIntf_SendNodeInfo(partition, connectionId);
            break;
        }
    }
    MESSAGE_Free(node, msg);
}


void MultiGuiIntf_Finalize (EXTERNAL_Interface *iface) {
    // Close socket connection to GUI.
    if (DEBUG) {
        printf ("Finalizing - performing disconnect\n");
        fflush(stdout);
    }

    std::string replyText = " ";
    MultiGUI_SendReply(GUI_CreateReply(GUI_FINISHED, &replyText));
}

/// \brief Request for Older Events on new connections
///
/// \param iface  pointer to master simulation interface
/// \param connectionId  socket descriptor for new connection
void MultiGuiIntf_RequestForOlderEvents(EXTERNAL_Interface *iface, Int64 connectionId)
{
    PartitionData* partitionData = iface->partition;
    int numPart = partitionData->getNumPartitions();
    MultiGuiInterfaceMsgType type = REQUEST_OLDER_EVENTS;
    UInt32 size = sizeof(connectionId);
    unsigned int newSize = sizeof(type) + sizeof(size) + sizeof(connectionId);
    char* newBuffer = (char*)MEM_malloc(newSize);
    int index = 0;
    memcpy(newBuffer, &type, sizeof(type));
    index += sizeof(type);
    memcpy(newBuffer + index, &size, sizeof(size));
    index += sizeof(size);
    memcpy(newBuffer + index, &connectionId, sizeof(connectionId));
    for (int i = 0; i < numPart; i++)
    {
        Message* msg = MESSAGE_Alloc(partitionData->firstNode,
            EXTERNAL_LAYER,
            EXTERNAL_MULTI_GUI,
            0);
        MESSAGE_AddInfo(partitionData->firstNode, msg, newSize);
        memcpy(MESSAGE_ReturnInfo(msg), newBuffer, newSize);
        EXTERNAL_MESSAGE_RemoteSend(partitionData,
            i,
            msg,
            0,
            EXTERNAL_SCHEDULE_SAFE);
    }
    MEM_free(newBuffer);
}

#ifdef AUTO_IPNE_INTERFACE
/// \brief Sends all mapping information to newlly joined GUI
///
/// \param partition  pointer to PartitionData of the current partition
/// \param connectionId  socket descriptor for new connection
void MultiGuiIntf_SendMapping(PartitionData* partition, Int64 connectionId)
{
    int numPart = partition->getNumPartitions();
    EXTERNAL_Interface *ipne = NULL;
    AutoIPNEData *ipneData = NULL;
    AutoIPNE_NodeMapping* mapping = NULL;
    EXTERNAL_Mapping *r = NULL;

    // Get IPNE external interface
    ipne = EXTERNAL_GetInterfaceByName(
        &partition->interfaceList,
        "IPNE");
    assert(ipne != NULL);

    for (int i = 0; i < EXTERNAL_MAPPING_TABLE_SIZE; i++)
    {
        r = ipne->table[i];
        // Look through the list
        while (r != NULL)
        {
            // Using mac-address as key to get mapping
            if (r->keySize == 10)
            {
                mapping = (AutoIPNE_NodeMapping*)r->val;
                char virtualAddrStr[MAX_STRING_LENGTH] = { 0 };
                char physicalAddrStr[MAX_STRING_LENGTH] = { 0 };
                Address virtualAddress = mapping->virtualAddress;
                Address physicalAddress = mapping->physicalAddress;
                if (physicalAddress.networkType == NETWORK_IPV4)
                {
                    // ntohl is being done here to undo the byte ordering change
                    // done by htonl called in starting of function for ipv4
                    virtualAddress.interfaceAddr.ipv4
                        = ntohl(virtualAddress.interfaceAddr.ipv4);
                    physicalAddress.interfaceAddr.ipv4
                        = ntohl(physicalAddress.interfaceAddr.ipv4);
                }
                // Get the node pointer (from virtual address)
                int nodeId = MAPPING_GetNodeIdFromInterfaceAddress(
                    ipne->partition->firstNode,
                    virtualAddress);
                if (nodeId == INVALID_ADDRESS)
                {
                    char msg[MAX_STRING_LENGTH] = { 0 };
                    sprintf(msg, "Invalid virtual node address: ipv6\n");
                    ERROR_ReportWarning(msg);
                }
                Node* virtualNodePtr
                    = MAPPING_GetNodePtrFromHash(ipne->partition->nodeIdHash,
                                                 nodeId);
                if (virtualNodePtr == NULL)
                {
                    virtualNodePtr = MAPPING_GetNodePtrFromHash(
                        ipne->partition->remoteNodeIdHash,
                        nodeId);
                    if (virtualNodePtr == NULL)
                    {
                        char msg[MAX_STRING_LENGTH] = { 0 };
                        sprintf(msg,
                            "Invalid virtual node address:"
                            " ipv6 or nodeId: %d\n",
                            nodeId);
                        ERROR_ReportWarning(msg);
                    }
                }
                IO_ConvertIpAddressToString(&virtualAddress,
                                            virtualAddrStr);
                IO_ConvertIpAddressToString(&physicalAddress,
                                            physicalAddrStr);

                clocktype time = ipne->partition->getGlobalTime();

                GuiReply reply, replyMapping;
                char timeString[MAX_CLOCK_STRING_LENGTH] = { 0 };
                char replyBuff[GUI_MAX_COMMAND_LENGTH] = { 0 };

                ctoa(time, timeString);

                reply.type = GUI_ANIMATION_COMMAND;
                sprintf(replyBuff, "%d %u %d %s",
                    GUI_SET_EXTERNAL_NODE,
                    virtualNodePtr->nodeId,
                    0,
                    timeString);
                reply.args.append(replyBuff);
                MultiGUI_SendReply(reply, connectionId);

                replyMapping.type = GUI_ANIMATION_COMMAND;
                sprintf(replyBuff, "%d %u %s %s %s",
                    GUI_SET_EXTERNAL_NODE_MAPPING,
                    virtualNodePtr->nodeId,
                    virtualAddrStr,
                    physicalAddrStr,
                    timeString);

                replyMapping.args.append(replyBuff);
                MultiGUI_SendReply(replyMapping, connectionId);
            }
            r = r->next;
        }
    }
}
#endif

/// \brief Sends node status (enabled or disabled and node position) for the nodes in the partition
///
/// \param partition  pointer to PartitionData of the current partition
/// \param connectionId  socket descriptor for new connection
void MultiGuiIntf_SendNodeInfo(PartitionData* partition, Int64 connectionId)
{
    Node* node = partition->firstNode;
    clocktype time = partition->getGlobalTime();
    char timeString[MAX_CLOCK_STRING_LENGTH] = { 0 };
    char replyBuff[GUI_MAX_COMMAND_LENGTH] = { 0 };

    ctoa(time, timeString);
    while (node)
    {
        if (node->mobilityData->mobilityType != NO_MOBILITY)
        {
            // Sending node position
            GuiReply replyPosition;
            replyPosition.type = GUI_ANIMATION_COMMAND;
            sprintf(replyBuff, "%d %u %.15f %.15f %.15f %s",
                GUI_MOVE_NODE, node->nodeId,
                node->mobilityData->current->position.common.c1,
                node->mobilityData->current->position.common.c2,
                node->mobilityData->current->position.common.c3,
                timeString);
            replyPosition.args.append(replyBuff);
            MultiGUI_SendReply(replyPosition, connectionId);
        }

        // Sending GUI_DEACTIVATE_INTERFACE command for every interface
        for (int i = 0; i < node->numberInterfaces; i++)
        {
            if (!NetworkIpInterfaceIsEnabled(node, i))
            {
                GuiReply replyInterfaceInfo;
                char replyBuff[GUI_MAX_COMMAND_LENGTH] = { 0 };

                replyInterfaceInfo.type = GUI_ANIMATION_COMMAND;
                sprintf(replyBuff,
                        "%d %d %d %s",
                        GUI_DEACTIVATE_INTERFACE,
                        node->nodeId,
                        i,
                        0);
                replyInterfaceInfo.args.append(replyBuff);
                MultiGUI_SendReply(replyInterfaceInfo, connectionId);
            }
        }
        node = node->nextNodeData;
    }
}
