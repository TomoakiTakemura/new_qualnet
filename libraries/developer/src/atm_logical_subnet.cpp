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
//
// PROTOCOL     :: Atm_Logical_Subnet
// REFERENCES   ::
// RFC: 2225 for Classical IP and ARP over ATM
// RFC: 2684 for Multi-protocol Encapsulation over
// ATM Adaptation Layer 5
// ATM Forum Addressing Specification:
// Reference Guide AF-RA-0106.000
// Assumtion :: 1. Two End System not connected by a Atm Link, There must
// have some Atm Switches.
// 2. Point to point wire link.
// 3. Asymmetric connection.
// 4. Generic Flow Control is omitted.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "api.h"
#include "partition.h"
#include "atm_layer2.h"
#include "atm_logical_subnet.h"


#define DEBUG 0

// Read the ATM logical subnet and node Id.
//
// \param node  pointer to node
// \param str  struture pointer
// \param nodeList  node List pointer
// \param subnetAddress  logical subnet address pointer
// \param nodePos  position of node
//
// \return total number of member.

static
int AtmReadLogicalSubnetAndNodeId(
    Node* node,
    char* str,
    int** nodeList,
    NodeAddress* subnetAddress,
    int* nodePos)
{
    char subnetAddressString[MAX_ADDRESS_STRING_LENGTH];
    int numHostBits = 0;
    int maxHosts = 0;
    NodeAddress nodeId;
    NodeAddress maxNodeId;
    char nodeIdList[MAX_STRING_LENGTH];
    char *token;
    char *next;
    char iotoken[MAX_STRING_LENGTH];
    char delims[] = "{,} \n\t";
    char *endOfList;
    unsigned int addCount = 0;
    char* p = str;

    sscanf(p, "%s\n", subnetAddressString);

    IO_ParseNetworkAddress(
        subnetAddressString,
        subnetAddress,
        &numHostBits);

    maxHosts = ((int) pow(2.0, numHostBits)) - 2;
    (*nodeList) = (int*) MEM_malloc(sizeof(int) * maxHosts);

    if (DEBUG)
    {
        printf("for subnet %s, host bits = %d, max hosts = %d\n",
               subnetAddressString, numHostBits, maxHosts);
    }

    p = strchr(p, '{');

    if (p == NULL)
    {
        char errorString[MAX_STRING_LENGTH];
        sprintf(errorString,
                "Could not find '{' character:\n  %s\n", str);
        ERROR_ReportError(errorString);
    }

    // mark the end of the list
    endOfList = strchr(p, '}');

    if (endOfList == NULL)
    {
        char errorString[MAX_STRING_LENGTH];
        sprintf(errorString,
                "Could not find '}' character:\n  %s\n", str);
        ERROR_ReportError(errorString);
    }

    endOfList[0] = '\0';

    strcpy(nodeIdList, p);

    token = IO_GetDelimitedToken(iotoken, nodeIdList, delims, &next);

    if (!token)
    {
        char errorString[MAX_STRING_LENGTH];
        sprintf(errorString,
                "Can't find nodeId list, e.g. { 1, 2, .. }:\n  %s\n", str);
        ERROR_ReportError(errorString);
    }

    while (1)
    {
        nodeId = atoi(token);
        (*nodeList)[addCount] = nodeId;
        addCount++;

        if (node->nodeId == nodeId)
        {
            *nodePos = addCount;
        }

        if (DEBUG)
        {
            printf("Read the node=%d and addCount=%d\n",
                nodeId, addCount);
        }

        if (addCount > (unsigned)maxHosts)
        {
            char errorStr[MAX_STRING_LENGTH];
            sprintf(errorStr, "Too many nodes in subnet %s",
                    subnetAddressString);
            ERROR_ReportError(errorStr);
        }

        token = IO_GetDelimitedToken(iotoken, next, delims, &next);
        if (!token)
        {
            break;
        }

        if (strncmp("thru", token, 4) == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            maxNodeId = atoi(token);

            nodeId++;
            (*nodeList)[addCount] = nodeId;
            addCount++;

            if (node->nodeId == nodeId)
            {
                *nodePos = addCount;
            }

            while (nodeId < maxNodeId)
            {
                printf("Read the node=%d and "
                    "addCount=%d\n",nodeId, addCount);
                if (addCount > (unsigned)maxHosts)
                {
                    char errorStr[MAX_STRING_LENGTH];
                    sprintf(errorStr, "Too many nodes in subnet %s",
                            subnetAddressString);
                    ERROR_ReportError(errorStr);
                }
                nodeId++;
                (*nodeList)[addCount] = nodeId;
                addCount++;

                if (node->nodeId == nodeId)
                {
                    *nodePos = addCount;
                }
            }

            token = IO_GetDelimitedToken(iotoken, next, delims, &next);

            if (!token)
            {
                break;
            }
        }
    }//while//
    return addCount;
}

// Read the ATM logical subnet at EndNode.
//
// \param node  pointer to node
// \param nodeInput  node Input
// \param interfaceIndex  associated InterfaceIndex
//
// \return pointer to logical subnet structure

LogicalSubnet* AtmReadLogicalSubnetAtEndNode(
    Node* node,
    const NodeInput* nodeInput,
    int interfaceIndex)
{
    char errorStr[MAX_STRING_LENGTH];
    char subnetAddStr[MAX_ADDRESS_STRING_LENGTH];
    NodeAddress subnetAddress = 0;
    int nodePos = -1;
    int* nodeList = NULL;
    BOOL wasFound = FALSE;
    char str[MAX_STRING_LENGTH];
    int i;
    int nodeNum = 0;
    LogicalSubnet* logicalSubnet =
        (LogicalSubnet*) MEM_malloc(sizeof(LogicalSubnet));

    const AddressMapType *map = node->partitionData->addressMapPtr;

    IO_ReadLine(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "ATM-LOGICAL-SUBNET",
        &wasFound,
        str);

    if (!wasFound)
    {
        printf(" In Node %u \n", node->nodeId);
        sprintf(errorStr, "LOGICAL-ATM-SUBNET: need to specify \n");
        ERROR_ReportError(errorStr);
    }

    nodeNum = AtmReadLogicalSubnetAndNodeId(node,
        str,
        &nodeList,
        &subnetAddress,
        &nodePos);

    sscanf(str, "%s\n", subnetAddStr);

    if (!nodeNum)
    {
        sprintf(errorStr, "LOGICAL-ATM-SUBNET: %s has empty node list",
                                                            subnetAddStr);
        ERROR_ReportError(errorStr);
    }

    if (nodePos == -1)
    {
        sprintf(errorStr, "LOGICAL-ATM-SUBNET: %s omitted this nodeId %d",
            subnetAddStr, node->nodeId);
        ERROR_ReportError(errorStr);
    }

    for (i = 0; i < map->numSubnets; i++)
    {
        if (map->subnetList[i].networkType == NETWORK_IPV4)
        {
            if (map->subnetList[i].list_subnet_addr == subnetAddress)
            {
                sprintf(errorStr, "SUBNET %s is "
                    "already exist", subnetAddStr);
                ERROR_ReportError(errorStr);
            }
        }
    }

    logicalSubnet->numMember = nodeNum;
    logicalSubnet->member =
        (MemberData*) MEM_malloc(sizeof(MemberData) * (nodeNum));
    logicalSubnet->ipAddress = subnetAddress + nodePos;
    logicalSubnet->subnetMask = subnetAddress;

    if (DEBUG)
    {
        printf("Number of NodeId=%d:-\n", nodeNum);

        for (i = 0; i < nodeNum; i++)
        {
            printf("\tNODE=%d\n", nodeList[i]);
        }
    }

    for (i = 0; i < nodeNum; i++)
    {
        (logicalSubnet->member)[i].nodeId = nodeList[i];
        (logicalSubnet->member)[i].relativeIndex = 0;
    }

    if (DEBUG)
    {
        printf("Logical Subnet for NodeId=%d:-\n", nodeNum);
        printf("\tnumMember=%d", logicalSubnet->numMember);
        printf("\tipAddress=%x", logicalSubnet->ipAddress);
        printf("\tsubnetMask=%x\n", logicalSubnet->subnetMask);


        for (i = 0; i < logicalSubnet->numMember; i++)
        {
            printf("\t\tMemberNode=%d\tIndex=%d\n",
                (logicalSubnet->member)[i].nodeId,
                (logicalSubnet->member)[i].relativeIndex);
        }
    }
    return logicalSubnet;
}

// Atm Get Logical IPAddress From NodeId
//
// \param node  pointer to node
// \param nodeId  member node Id
// \param infInx  associated InterfaceIndex
//
// \return Logical Ip address of member node

NodeAddress AtmGetLogicalIPAddressFromNodeId(
    Node* node,
    NodeAddress nodeId,
    int infInx)
{
    NodeAddress atmLogicalIpAdd = 0;
    char errorStr[MAX_STRING_LENGTH];

    Node* findNode = MAPPING_GetNodePtrFromHash(
                        node->partitionData->nodeIdHash,
                        nodeId);

    if (findNode == NULL) {
        findNode =
            MAPPING_GetNodePtrFromHash(
            node->partitionData->remoteNodeIdHash,
            nodeId);
        assert(findNode != NULL);
    }

    AtmLayer2Data* atmData = Atm_Layer2GetAtmData(findNode, infInx);

    if (atmData->nodeType == ATM_LAYER2_ENDSYSTEM)
    {
        //Find the logical IP address for a specific interface.
        atmLogicalIpAdd = atmData->logicalSubnet->ipAddress;
    }
    else
    {
        sprintf(errorStr,
            "LOGICAL-ATM-SUBNET: Node %d is not END-SYSTEM", nodeId);
        ERROR_ReportError(errorStr);
    }
    return atmLogicalIpAdd;
}


// Atm Get Logical subnet From NodeId
//
// \param node  pointer to node
// \param nodeId  member node Id
// \param infInx  associated InterfaceIndex
//
// \return LogicalSubnet structure of that node

const LogicalSubnet* AtmGetLogicalSubnetFromNodeId(
    Node* node,
    NodeAddress nodeId,
    int infInx)
{
    char errorStr[MAX_STRING_LENGTH];

    Node* findNode = MAPPING_GetNodePtrFromHash(
                        node->partitionData->nodeIdHash,
                        nodeId);

    if (findNode == NULL) {
        findNode =
            MAPPING_GetNodePtrFromHash(
            node->partitionData->remoteNodeIdHash,
            nodeId);
        assert(findNode != NULL);
    }

    AtmLayer2Data* atmData = Atm_Layer2GetAtmData(findNode, infInx);

    if (atmData->nodeType != ATM_LAYER2_ENDSYSTEM)
    {
        sprintf(errorStr,
            "LOGICAL-ATM-SUBNET: Node %d is not END-SYSTEM", nodeId);
        ERROR_ReportError(errorStr);
    }

    //Find the logical IP address for a specific interface.
    return atmData->logicalSubnet;
}
