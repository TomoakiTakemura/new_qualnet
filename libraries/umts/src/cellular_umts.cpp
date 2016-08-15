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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include <list>
#include <iomanip>

#include "api.h"

#include "cellular.h"
#include "cellular_umts.h"
#include "cellular_layer3.h"
#include "layer3_umts.h"
#include "layer2_umts.h"
#include "mac_cellular.h"
#include "phy_cellular.h"
#include "phy_umts.h"
#include "umts_constants.h"

#define DEBUG_POWER 0

//
// Cross layer utility functions and macros
//

// Get the node type of the UMTS device.
//
// \param node  Pointer to node.
//
// \return Type of the node, ie UE, NodeB, etc
CellularNodeType UmtsGetNodeType(Node *node)
{
    CellularLayer3Data *cellularData;
    UmtsLayer3Data *umtsL3 = NULL;

    cellularData = (CellularLayer3Data *)
                   node->networkData.cellularLayer3Var;
    ERROR_Assert(cellularData != NULL, "UMTS: Memory error!");
    umtsL3 = (UmtsLayer3Data *) cellularData->umtsL3;
    ERROR_Assert(umtsL3 != NULL, "UMTS:Memory error!");

    return umtsL3->nodeType;
}

// Is the device a UE node?
//
// \param node  Pointer to node.
//
// \return TRUE, if UE, FALSE elsewise
BOOL UmtsIsUe(Node *node)
{
    return (UmtsGetNodeType(node) == CELLULAR_UE);
}

// Is the device a NodeB node?
//
// \param node  Pointer to node.
//
// \return TRUE, if NodeB, FALSE elsewise
BOOL UmtsIsNodeB(Node *node)
{
    return (UmtsGetNodeType(node) == CELLULAR_NODEB);
}

// Is the device a RNC node?
//
// \param node  Pointer to node.
//
// \return TRUE, if RNC, FALSE elsewise
BOOL UmtsIsRnc(Node *node)
{
    return (UmtsGetNodeType(node) == CELLULAR_RNC);
}

// Is the device a GGSN node?
//
// \param node  Pointer to node.
//
// \return TRUE, if GGSN, FALSE elsewise
BOOL UmtsIsGgsn(Node *node)
{
    return (UmtsGetNodeType(node) == CELLULAR_GGSN);
}

// FUNCTION   :: UmtsSerializeMessage
// LAYER      :: MAC
// PURPOSE    :: Dump a single message into a buffer so that the orignal
//               message can be recovered from the buffer
// PARAMETERS ::
// + node      : Node*    : Pointer to node.
// + msg       : Message* : Pointer to a messages
// + buffer    : string&   : The string buffer the message will be 
//                          serialzed into (append to the end)
// RETURN     :: void     : NULL
void UmtsSerializeMessage(Node* node,
                          Message* msg,
                          std::string& buffer)
{
    MESSAGE_Serialize(node, msg, buffer);
}

// FUNCTION   :: UmtsUnSerializeMessage
// LAYER      :: MAC
// PURPOSE    :: recover the orignal message from the buffer
// PARAMETERS ::
// + node      : Node*    : Pointer to node.
// + buffer    : const char* : The string buffer containing the message
//                          was serialzed into
// + bufIndex  : int&  : the start position in the buffer pointing
//                          to the message updated to the end of 
//                          the message after the unserialization.
// RETURN     :: Message* : Message pointer to be recovered
Message* UmtsUnSerializeMessage(Node* node,
                                const char* buffer,
                                int& bufIndex)
{
    return MESSAGE_Unserialize(node->partitionData, buffer, bufIndex);
}

// FUNCTION   :: UmtsSerializeMessageList
// LAYER      :: MAC
// PURPOSE    :: Dump a list of message into a buffer so that the orignal
//               messages can be recovered from the buffer
// PARAMETERS ::
// + node      : Node*    : Pointer to node.
// + msg       : Message* : Pointer to a message list
// + buffer    : string&  : The string buffer the messages will be 
//                          serialzed into (append to the end)
// RETURN     :: int      : number of messages in the list
int UmtsSerializeMessageList(Node* node,
                             Message* msgList,
                             std::string& buffer)
{
    return MESSAGE_SerializeMsgList(node, msgList, buffer);
}

// FUNCTION   :: UmtsUnSerializeMessageList
// LAYER      :: MAC
// PURPOSE    :: recover the orignal message from the buffer
// PARAMETERS ::
// + node      : Node*    : Pointer to node.
// + buffer    : const char* : The string buffer containing the message 
//                             list serialzed into
// + bufIndex  : int&  : the start position in the buffer pointing to 
//                       the message list
//                       updated to the end of the message list after the 
//                       unserialization.
// + numMsgs   : int   : Number of messages in the list
// RETURN     :: Message* : Pointer to the message list to be recovered
Message* UmtsUnSerializeMessageList(Node* node, 
                                    const char* buffer,
                                    int& bufIndex,
                                    int numMsgs) 
{
    return MESSAGE_UnserializeMsgList(node->partitionData, buffer, bufIndex, numMsgs);
}

// FUNCTION   :: UmtsPackMessage
// LAYER      :: MAC
// PURPOSE    :: Pack a list of messages to be one message structure
// PARAMETERS ::
// + node      : Node*    : Pointer to node.
// + msgList   : Message* : Pointer to a list of messages
// + origProtocol: TraceProtocolType : Protocol allocating this packet
// RETURN     :: Message* : The super msg contains a list of msgs as payload
Message* UmtsPackMessage(Node* node,
                         Message* msgList,
                         TraceProtocolType origProtocol)
{
    int actSize;
    return MESSAGE_PackMessage(node, msgList, origProtocol, &actSize);
}

// Unpack a message to the original list of messages
//
// \param node  Pointer to node.
// \param msg  Pointer to the supper msg contains list of msgs
// \param copyInfo  Whether copy info from old msg to first msg
// \param freeOld  Whether the original message should be freed
//
// \return A list of messages unpacked from original msg
Message* UmtsUnpackMessage(Node* node,
                           Message* msg,
                           BOOL copyInfo,
                           BOOL freeOld)
{
    return MESSAGE_UnpackMessage(node, msg, copyInfo, freeOld);
}

// FUNCTION   :: UmtsPackMessage
// LAYER      :: MAC
// PURPOSE    :: Pack a list of messages to be one message structure
// PARAMETERS ::
// + node      : Node*    : Pointer to node.
// + msgList   : list<Message*> : a list of messages
// + origProtocol: TraceProtocolType : Protocol allocating this packet
// RETURN     :: Message* : The super msg contains a list of msgs as payload
Message* UmtsPackMessage(Node* node,
                         std::list<Message*>& msgList,
                         TraceProtocolType origProtocol)
{
    std::string buffer;
    buffer.reserve(5000);

    if (msgList.empty())
    {
        return NULL;
    }

    std::list<Message*>::iterator itPos;
    for (itPos = msgList.begin(); itPos != msgList.end(); ++itPos)
    {
        Message* nextMsg = *itPos;

        UmtsSerializeMessage(node, nextMsg, buffer);

        // free the orignal message
        MESSAGE_Free(node, nextMsg);
    }
    msgList.clear();

    Message* newMsg = MESSAGE_Alloc(node, 0, 0, 0);
    MESSAGE_PacketAlloc(node, newMsg, (int)buffer.size(), origProtocol);
    memcpy(MESSAGE_ReturnPacket(newMsg),
           buffer.data(),
           buffer.size());
    return newMsg;
}

// Unpack a message to the original list of messages
//
// \param node  Pointer to node.
// \param msg  Pointer to the supper msg containing list of msgs
// \param copyInfo  Whether copy info from old msg to first msg
// \param freeOld  Whether the original message should be freed
//    + msgList   : list<Message*> : an empty stl list to be used to 
//    contain unpacked messages
//
void UmtsUnpackMessage(Node* node,
                       Message* msg,
                       BOOL copyInfo,
                       BOOL freeOld,
                       std::list<Message*>& msgList)
{
    msgList.clear();

    Message* firstMsg = NULL;
    Message* newMsg;

    char* payload;
    int payloadSize;
    int msgSize;

    payload = MESSAGE_ReturnPacket(msg);
    payloadSize = MESSAGE_ReturnPacketSize(msg);
    msgSize = 0;

    while (msgSize < payloadSize)
    {
        newMsg = UmtsUnSerializeMessage(node,
                                        payload,
                                        msgSize);

        // add new msg to msgList
        if (firstMsg == NULL)
        {
            firstMsg = newMsg;
        }
        msgList.push_back(newMsg);
    }

    if (copyInfo)
    {
        // copy over the info field
        MESSAGE_InfoAlloc(node, firstMsg, MESSAGE_ReturnInfoSize(msg));
        memcpy(MESSAGE_ReturnInfo(firstMsg),
               MESSAGE_ReturnInfo(msg),
               MESSAGE_ReturnInfoSize(msg));
    }

    // free original message
    if (freeOld)
    {
        MESSAGE_Free(node, msg);
    }
}

// FUNCTION   :: UmtsDuplicateMessageList
// LAYER      :: ANY
// PURPOSE    :: Duplicate a list of message
// PARAMETERS ::
// + node      : Node*    : Pointer to node.
// + msgList   : Message* : Pointer to a list of messages
// RETURN     :: Message* : The duplicated message list header
Message* UmtsDuplicateMessageList(
    Node* node,
    Message* msgList)
{
    Message* dupMsgHead = NULL;
    Message* nextMsg = msgList;
    Message* prevDupMsg = NULL;
    while (nextMsg)
    {
        Message* dupMsg = MESSAGE_Duplicate(node, nextMsg);
        if (!dupMsgHead)
        {
            dupMsgHead = dupMsg;
        }
        else
        {
            prevDupMsg->next = dupMsg;
        }
        prevDupMsg = dupMsg;
        nextMsg = nextMsg->next;
    }
    return dupMsgHead;
}

// Print part of message information for
// debugging purpose
//
//    +os:            output stream class
//    +msg:           Message to be printed
//
// \return ostream
std::ostream& UmtsPrintMessage(std::ostream& os, Message* msg)
{
    int i;

    os << "Printing message: layer: "<< msg->layerType
        << " protocol: " << msg->protocolType
        << " instance: " << msg->instanceId
        << " event: " << msg->eventType << std::endl;

#if 0
    int j;
    for (i = 0; i < 1 /*MAX_INFO_FIELDS*/; i ++)
    {
        os << "infoArray[" << i << "].infoType: \t"
            << msg->infoArray[i].infoType <<"\t";
        os << "infoArray[" << i << "].infoSize: \t"
            << msg->infoArray[i].infoSize <<"\t";
        os << "infoArray[" << i << "].info: \t";
        for (j = 0; j < msg->infoArray[i].infoSize; j++)
        {
            os << (int)msg->infoArray[i].info[j];
        }
        os << std::endl;
    }
#endif // 0
    os << "real packetSize: \t" <<  msg->packetSize
        << "\tpayloadSize: "<< msg->payloadSize << std::endl;
    {
        os << "packet: \t";
        for (i = 0; i < msg->packetSize; i++)
            os << (int) msg->packet[i];
        os << std::endl;
    }

#if 0
    os << "payload*: \t" << msg->payload << std::endl;
#endif

    os << "virtualPayloadSize: \t" << msg->virtualPayloadSize << "\t";
    os << "numberOfHeaders: \t" << msg->numberOfHeaders << std::endl;
    for (i = 0; i < msg->numberOfHeaders /*MAX_HEADERS*/; i++)
    {
        os << "headerProtocols[" << i <<"]: \t"
           << msg->headerProtocols[i] << "\t\t";
        os << "headerSizes[" << i << "]: \t"
           << msg->headerSizes[i] << std::endl;
    }
    os << std::endl;

    return os;
}


// Link messages contained in a STL list into 
// message linked by next field
//
//    + node:       pointer to the network node
//    + msgList:    A STL list containing the pdus to be sent
//
// \return The first message in the message list, 
// the rest messages are linked
// via field next
Message* UmtsLinkMessages(
    Node* node,
    std::list<Message*>& msgList)
{
    Message* ptrMsgList = NULL;
    Message* prevMsg = NULL;

    std::list<Message*>::iterator it;
    it = msgList.begin();
    while (it != msgList.end())
    {
        if (prevMsg == NULL)
        {
            ptrMsgList = *it;
        }
        else
        {
            prevMsg->next = *it;
        }
        prevMsg = *it;
        prevMsg->next = NULL;
        ++ it;
    }
    return ptrMsgList;
}

// unlink a list of Messages into an STL list
//
//    + node:       pointer to the network node
//    + msgHead:    the pointer to the first msg in the linked message list
//    + msgList:    An empty STL list to be used to contain messages
//
void UmtsUnLinkMessages(
    Node* node,
    Message* msgHead,
    std::list<Message*>& msgList)
{
    msgList.clear();
    Message* msg = msgHead;
    while (msg)
    {
        Message* nextMsg = msg->next;
        msg->next = NULL;
        msgList.push_back(msg);
        msg = nextMsg;
    }
}


// return Duplex mode of Ue/NodeB of the specified interface
//
// \param node  Pointer to node.
// \param interfaceIndex  interface index
//
// \return duplex mode of the interface
UmtsDuplexMode UmtsGetDuplexMode(Node*node, int interfaceIndex)
{
    MacCellularData* macCellularData = (MacCellularData*)
                      node->macData[interfaceIndex]->macVar;

    CellularUmtsLayer2Data* layer2Data =
        macCellularData->cellularUmtsL2Data;

    UmtsMacData* umtsMacData = (UmtsMacData*)layer2Data->umtsMacData;

    return  umtsMacData->duplexMode;
}

// return the active ul channel index of the intf at nodeB
//
// \param node  Pointer to node.
// \param interfaceIndex  interface index
//
// \return channel Index of the nodeB
int UmtsGetUlChIndex(Node*node, int interfaceIndex)
{
     MacCellularData* macCellularData = (MacCellularData*)
                      node->macData[interfaceIndex]->macVar;

    CellularUmtsLayer2Data* layer2Data =
        macCellularData->cellularUmtsL2Data;

    UmtsMacData* umtsMacData = (UmtsMacData*)layer2Data->umtsMacData;

    return  umtsMacData->currentULChannel;
}

// return the active ul channel index of the intf at nodeB
//
// \param node  Pointer to node.
// \param interfaceIndex  interface index
// \param channelIndex  channel index to be set
//
// \return cell Id of the nodeB
void UmtsSetUlChIndex(Node*node, int interfaceIndex, int channelIndex)
{
     MacCellularData* macCellularData = (MacCellularData*)
                      node->macData[interfaceIndex]->macVar;

    CellularUmtsLayer2Data* layer2Data =
        macCellularData->cellularUmtsL2Data;

    UmtsMacData* umtsMacData = (UmtsMacData*)layer2Data->umtsMacData;

    umtsMacData->currentULChannel = channelIndex;
}


// return the cell id of the interface at nodeB
//
// \param node  Pointer to node.
// \param interfaceIndex  interface index
//
// \return cell Id of the nodeB
UInt32 UmtsNodeBGetCellId(Node*node, int interfaceIndex)
{
    int phyIndex = node->macData[interfaceIndex]->phyNumber;
    PhyCellularData* phyCellular =
        (PhyCellularData*)node->phyData[phyIndex]->phyVar;
    PhyUmtsData* phyUmts = (PhyUmtsData *)phyCellular->phyUmtsData;

    PhyUmtsNodeBData* phyUmtsNodeb =
        (PhyUmtsNodeBData*)phyUmts->phyDataNodeB;

    return  (UInt32)(phyUmtsNodeb->pscCodeIndex);
}

// Set the spread factor for the interface
//
// \param node  Pointer to node.
// \param interfaceIndex  interface index
// \param sf  spread factor to be set
//
// \return cell Id of the nodeB
void UmtsUeSetSpreadFactorInUse(Node* node,
                                int interfaceIndex,
                                UmtsSpreadFactor sf)
{
    int phyIndex = node->macData[interfaceIndex]->phyNumber;
    std::list <PhyUmtsChannelData*>* phyChList;
    PhyCellularData* phyCellular =
        (PhyCellularData*)node->phyData[phyIndex]->phyVar;
    PhyUmtsData* phyUmts = (PhyUmtsData *)phyCellular->phyUmtsData;
    UmtsSpreadFactor oldSf;
    double curTxPow;
    double origGainFactor = 0;
    double newGainFactor = 0;

    PhyUmtsUeData* phyUmtsUe =
        (PhyUmtsUeData*)phyUmts->phyDataUe;
    phyChList = phyUmtsUe->phyChList;
    oldSf = phyUmtsUe->dpchSfInUse;
    phyUmtsUe->dpchSfInUse = sf;

    if (phyUmtsUe->ulDediPhChConfig.numDPDCH == 1)
    {
        // change the first dpdch 's SF tp sf
        std::list<PhyUmtsChannelData*>::iterator phChIter;
        for (phChIter = phyChList->begin();
             phChIter != phyChList->end();
             ++phChIter)
        {
            // only DPDCH needs to change SF
            if ((*phChIter)->phChType == UMTS_PHYSICAL_DPDCH)
            {
                // Calculate the gain factor for the old sf
                origGainFactor +=
                    UmtsPhyCalculateGainFactor((*phChIter)->spreadFactor);
                (*phChIter)->spreadFactor = sf;

                // For UE, when only 1 DPDCH is used, the channel code
                // index is the SF/4, for example, Sf =256, 
                // the code index is 64
                (*phChIter)->chCodeIndex = sf / 4;

                // Calculate the gain factor for the new sf
                newGainFactor +=
                    UmtsPhyCalculateGainFactor((*phChIter)->spreadFactor);
            }
            else
            {
                // Calculate the gain factor for the old sf
                origGainFactor +=
                    UmtsPhyCalculateGainFactor((*phChIter)->spreadFactor);
                
                // Calculate the gain factor for the new sfnewGainFactor +=
                    UmtsPhyCalculateGainFactor((*phChIter)->spreadFactor);
            }
        }
    }

    // adjust the Tx Power
    // set the new transmission power
    if (sf != oldSf)
    {
        PhyUmtsGetTransmitPower(node, phyIndex, &curTxPow);

        curTxPow = IN_DB(curTxPow);

        PhyUmtsSetTransmitPower(
            node,
            phyIndex,
            NON_DB(curTxPow +
                   10 * log10((newGainFactor * 
                               oldSf * 
                               UmtsPhyCalculateGainFactor(oldSf)) /
                               (origGainFactor * 
                               sf * 
                               UmtsPhyCalculateGainFactor(sf)))));
        if (DEBUG_POWER)
        {
            printf("node %d: set Tx power from %f to %f\n",
                node->nodeId, curTxPow,
                curTxPow +
                10 * log10((newGainFactor * 
                            oldSf * 
                            UmtsPhyCalculateGainFactor(oldSf)) /
                            (origGainFactor * 
                            sf * 
                            UmtsPhyCalculateGainFactor(sf))));
        }
    }
}

// Get the spread factor in use for the interface
//
// \param node  Pointer to node.
// \param interfaceIndex  interface index
//
// \return spread factor in use
UmtsSpreadFactor UmtsUeGetSpreadFactorInUse(Node* node, int interfaceIndex)
{
    int phyIndex = node->macData[interfaceIndex]->phyNumber;
    PhyCellularData* phyCellular =
        (PhyCellularData*)node->phyData[phyIndex]->phyVar;
    PhyUmtsData* phyUmts = (PhyUmtsData *)phyCellular->phyUmtsData;

    PhyUmtsUeData* phyUmtsUe =
        (PhyUmtsUeData*)phyUmts->phyDataUe;

    return  phyUmtsUe->dpchSfInUse;
}

// Get the number of active DPDCH in use for the interface
//
// \param node  Pointer to node.
// \param interfaceIndex  interface index
//
// \return number of active DPDCH in use
int UmtsUeGetActiveDpdchNum(Node* node, int interfaceIndex)
{
    int phyIndex = node->macData[interfaceIndex]->phyNumber;
    PhyCellularData* phyCellular =
        (PhyCellularData*)node->phyData[phyIndex]->phyVar;
    PhyUmtsData* phyUmts = (PhyUmtsData *)phyCellular->phyUmtsData;

    PhyUmtsUeData* phyUmtsUe =
        (PhyUmtsUeData*)phyUmts->phyDataUe;

    std::list<PhyUmtsChannelData*>::iterator it;
    int i = 0;
    for (it = phyUmtsUe->phyChList->begin();
        it != phyUmtsUe->phyChList->end();
        it ++)
    {
        if ((*it)->phChType == UMTS_PHYSICAL_DPDCH)
        {
            i ++;
        }
    }

    return i;
}

// Get the HSDPA capability for the interface
//
// \param node  Pointer to node.
// \param interfaceIndex  interface index
//
// \return HSDPA enabled or not
BOOL UmtsLayer3GetHsdpaCapability(Node* node, int interfaceIndex)
{
    int phyIndex = node->macData[interfaceIndex]->phyNumber;
    PhyCellularData* phyCellular =
        (PhyCellularData*)node->phyData[phyIndex]->phyVar;
    PhyUmtsData* phyUmts = (PhyUmtsData *)phyCellular->phyUmtsData;

    return phyUmts->hspdaEnabled;
}

// Get the data rate for the associated SF and DL Ph Ch Type
//
// \param sFactor  Spread factor
// \param slotFormatIndex  slot format index
// \param phChType  Phy Ch type
//
// \return data rate can be support by the channel with the SF
int UmtsLayer3GetDlPhChDataBitRate(UmtsSpreadFactor sFactor,
                                   signed char* slotFormatIndex,
                                   UmtsPhysicalChannelType phChType)
{
    if (*slotFormatIndex == -1)
    {
        // invalid one then need to use default one
        switch(sFactor)
        {
            case UMTS_SF_512:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_DL_DPCH_SLOT_FORMAT_SF_512;
                 break;
            }
            case UMTS_SF_256:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_DL_DPCH_SLOT_FORMAT_SF_256;
                 break;
            }
            case UMTS_SF_128:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_DL_DPCH_SLOT_FORMAT_SF_128;
                 break;
            }
            case UMTS_SF_64:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_DL_DPCH_SLOT_FORMAT_SF_64;
                 break;
            }
            case UMTS_SF_32:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_DL_DPCH_SLOT_FORMAT_SF_32;
                 break;
            }
            case UMTS_SF_16:
            {
                 if (phChType == UMTS_PHYSICAL_DPDCH)
                 {
                    *slotFormatIndex = 
                        (char)UMTS_DEFAULT_DL_DPCH_SLOT_FORMAT_SF_16;
                 }
                 else if (phChType == UMTS_PHYSICAL_HSPDSCH)
                 {
                    *slotFormatIndex = 
                        (char)UMTS_DEFAULT_HSDSCH_SLOT_FORMAT;
                 }

                 break;
            }
            case UMTS_SF_8:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_DL_DPCH_SLOT_FORMAT_SF_8;
                 break;
            }
            case UMTS_SF_4:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_DL_DPCH_SLOT_FORMAT_SF_4;
                 break;
            }
            default:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_DL_DPCH_SLOT_FORMAT_SF_256;
                 break;
            }
        }
    }
    else
    {
        // use the indicated slotFormat
    }

    if (phChType == UMTS_PHYSICAL_DPDCH && *slotFormatIndex != -1)
    {
        return (umtsDlDpchFormat[*slotFormatIndex].numData1 +
                umtsDlDpchFormat[*slotFormatIndex].numData2) *
                1 * SECOND / UMTS_SLOT_DURATION_384;
    }
    else if (phChType == UMTS_PHYSICAL_HSPDSCH && *slotFormatIndex != -1)
    {
        return umtsHsdschFormat[*slotFormatIndex].numData *
                1 * SECOND / UMTS_SLOT_DURATION_384;
    }
    else
    {
        return 0;
    }

}

// Get the data rate for the associated SF and UL Ph Ch Type
//
// \param sFactor  Spread factor
// \param slotFormatIndex  slot format index
// \param phChType  Phy Ch type
//
// \return data rate can be support by the channel with the SF
int UmtsLayer3GetUlPhChDataBitRate(UmtsSpreadFactor sFactor,
                                   signed char* slotFormatIndex,
                                   UmtsPhysicalChannelType phChType)
{
    if (*slotFormatIndex == -1)
    {
        // invalid one then need to use default one
        switch(sFactor)
        {
            case UMTS_SF_256:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_UL_DPDCH_SLOT_FORMAT_SF_256;
                 break;
            }
            case UMTS_SF_128:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_UL_DPDCH_SLOT_FORMAT_SF_128;
                 break;
            }
            case UMTS_SF_64:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_UL_DPDCH_SLOT_FORMAT_SF_64;
                 break;
            }
            case UMTS_SF_32:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_UL_DPDCH_SLOT_FORMAT_SF_32;
                 break;
            }
            case UMTS_SF_16:
            {
                 if (phChType == UMTS_PHYSICAL_DPDCH)
                 {
                    *slotFormatIndex = 
                        (char)UMTS_DEFAULT_UL_DPDCH_SLOT_FORMAT_SF_16;
                 }

                 break;
            }
            case UMTS_SF_8:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_UL_DPDCH_SLOT_FORMAT_SF_8;
                 break;
            }
            case UMTS_SF_4:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_UL_DPDCH_SLOT_FORMAT_SF_4;
                 break;
            }
            default:
            {
                 *slotFormatIndex = 
                     (char)UMTS_DEFAULT_UL_DPDCH_SLOT_FORMAT_SF_256;
                 break;
            }
        }
    }
    else
    {
        // use the indicated slotFormat
    }

    if (phChType == UMTS_PHYSICAL_DPDCH && *slotFormatIndex != -1)
    {
        return (umtsUlDpdchFormat[*slotFormatIndex].numData) *
            1 * SECOND / UMTS_SLOT_DURATION_384;
    }
    else
    {
        return 0;
    }
}

// Get the best SF for the specified rate requirement
//
//    + rateReq  : int : Data rate requirement 
// \param sFactor  Spread factor
//
// \return whether a SF can be used to support this rate
BOOL UmtsGetBestUlSpreadFactorForRate(int rateReq, UmtsSpreadFactor* sf)
{
    BOOL found = TRUE;
    signed char slotFormat = -1;
    if (UmtsLayer3GetUlPhChDataBitRate(
        UMTS_SF_256, &slotFormat) > rateReq)
    {
        *sf = UMTS_SF_256;
    }
    else if (UmtsLayer3GetUlPhChDataBitRate(
        UMTS_SF_128, &slotFormat) > rateReq)
    {
        *sf = UMTS_SF_128;
    }
    else if (UmtsLayer3GetUlPhChDataBitRate(
        UMTS_SF_64, &slotFormat) > rateReq)
    {
        *sf = UMTS_SF_64;
    }
    else if (UmtsLayer3GetUlPhChDataBitRate(
        UMTS_SF_32, &slotFormat) > rateReq)
    {
        *sf = UMTS_SF_32;
    }
    else if (UmtsLayer3GetUlPhChDataBitRate(
        UMTS_SF_16, &slotFormat) > rateReq)
    {
        *sf = UMTS_SF_16;
    }
    else if (UmtsLayer3GetUlPhChDataBitRate(
        UMTS_SF_8, &slotFormat) > rateReq)
    {
        *sf = UMTS_SF_8;
    }
    else if (UmtsLayer3GetUlPhChDataBitRate(
        UMTS_SF_4, &slotFormat) > rateReq)
    {
        *sf = UMTS_SF_4;
    }
    else
    {
        found = FALSE;
    }

    return found;
}

// get the default TTI of a transport channel from its type
//
//    trChType    : UmtsTransportChannelType  : transport channel type
//    format      : UmtsTransportFormat*      : transport format to be used
//    qosInfo     : const UmtsRABServiceInfo* : QoS Info 
//
// \return Whether a transport format can be supported
BOOL UmtsSelectDefaultTransportFormat(UmtsTransportChannelType  trChType,
                                      UmtsTransportFormat* format,
                                      const UmtsRABServiceInfo* qosInfo)
{
    BOOL found = TRUE;

    // currently FDD 3.84Mcps is  the only option
    const UmtsTransportFormat* defaultFormat = NULL;

    if (trChType == UMTS_TRANSPORT_BCH)
    {
        defaultFormat = &umtsDefaultBchTransFormat;
    }
    else if (trChType == UMTS_TRANSPORT_PCH)
    {
        defaultFormat = &umtsDefaultPchTransFormat;
    }
    else if (trChType == UMTS_TRANSPORT_FACH)
    {
        defaultFormat = &umtsDefaultFachTransFormat;
    }
    else if (trChType == UMTS_TRANSPORT_RACH)
    {
        defaultFormat = &umtsDefaultRachTransFormat;
    }
    else if (trChType == UMTS_TRANSPORT_DCH && !qosInfo)
    {
        defaultFormat = &umtsDefaultSigDchTransFormat_3400;
    }
#if 0
    // for background and interactive, defualt is used
    else if (trChType == UMTS_TRANSPORT_DCH && qosInfo &&
        (*qosInfo).trafficClass == UMTS_QOS_CLASS_BACKGROUND)
    {
        defaultFormat = &umtsDefaultDataDchTransFormatBackground;
    }
    else if (trChType == UMTS_TRANSPORT_DCH && qosInfo &&
        (*qosInfo).trafficClass >= UMTS_QOS_CLASS_INTERACTIVE_3 &&
        ((*qosInfo).trafficClass < UMTS_QOS_CLASS_BACKGROUND))
    {
        defaultFormat = &umtsDefaultDataDchTransFormatInteractive;
    }
#endif
    else if (trChType == UMTS_TRANSPORT_DCH && qosInfo &&
        (*qosInfo).maxBitRate <= 28800 &&
        (*qosInfo).trafficClass == UMTS_QOS_CLASS_CONVERSATIONAL)
    {
        defaultFormat = &umtsDefaultDataDchTransFormatConv_28800;
    }
    else if (trChType == UMTS_TRANSPORT_DCH && qosInfo &&
        (*qosInfo).maxBitRate > 28200 &&
        (*qosInfo).maxBitRate <= 32000 &&
        (*qosInfo).trafficClass == UMTS_QOS_CLASS_CONVERSATIONAL)
    {
        defaultFormat = &umtsDefaultDataDchTransFormatConv_32000;
    }
    else if (trChType == UMTS_TRANSPORT_DCH && qosInfo &&
        (*qosInfo).maxBitRate > 32000 &&
        (*qosInfo).maxBitRate <= 64000 &&
        (*qosInfo).trafficClass == UMTS_QOS_CLASS_CONVERSATIONAL)
    {
        defaultFormat = &umtsDefaultDataDchTransFormatConv_64000;
    }
    else if (trChType == UMTS_TRANSPORT_DCH && qosInfo &&
        (*qosInfo).maxBitRate > 64000 &&
        (*qosInfo).maxBitRate <= 128000 &&
        (*qosInfo).trafficClass == UMTS_QOS_CLASS_CONVERSATIONAL)
    {
        defaultFormat = &umtsDefaultDataDchTransFormatConv_128000;
    }
    else if (trChType == UMTS_TRANSPORT_DCH && qosInfo &&
        (*qosInfo).maxBitRate <= 14400 &&
        (*qosInfo).trafficClass == UMTS_QOS_CLASS_STREAMING)
    {
        defaultFormat = &umtsDefaultDataDchTransFormatStream_14400;
    }
    else if (trChType == UMTS_TRANSPORT_DCH && qosInfo &&
        (*qosInfo).maxBitRate > 14400 &&
        (*qosInfo).maxBitRate <= 28800 &&
        (*qosInfo).trafficClass == UMTS_QOS_CLASS_STREAMING)
    {
        defaultFormat = &umtsDefaultDataDchTransFormatStream_28800;
    }
    else if (trChType == UMTS_TRANSPORT_DCH && qosInfo &&
        (*qosInfo).maxBitRate > 28800 &&
        (*qosInfo).maxBitRate <= 57600 &&
        (*qosInfo).trafficClass == UMTS_QOS_CLASS_STREAMING)
    {
        defaultFormat = &umtsDefaultDataDchTransFormatStream_57600;
    }
    else if (trChType == UMTS_TRANSPORT_DCH && qosInfo &&
        (*qosInfo).maxBitRate > 57600 &&
        (*qosInfo).maxBitRate <= 115200 &&
        (*qosInfo).trafficClass == UMTS_QOS_CLASS_STREAMING)
    {
        defaultFormat = &umtsDefaultDataDchTransFormatStream_115200;
    }
    else if (trChType == UMTS_TRANSPORT_HSDSCH)
    {
        defaultFormat = &umtsDefaultDataHsdschTransFormat;
    }
    else
    {
        found = FALSE;
    }

    if (found && defaultFormat)
    {
        memcpy((void*)format,
           (void*)defaultFormat,
           sizeof(UmtsTransportFormat));
    }

    return found;
}

// Calculate the gain factor.
//
// \param spreadFactor  spread factor.
//
// \return gain factor
double UmtsPhyCalculateGainFactor( UmtsSpreadFactor spreadFactor)
{
    double gainFactor = 0.0;

    if (spreadFactor == UMTS_SF_1) {
        gainFactor = PHY_UMTS_GAIN_FACTOR_1;
    }
    else if (spreadFactor == UMTS_SF_2) {
        gainFactor = PHY_UMTS_GAIN_FACTOR_2;
    }
    else if (spreadFactor == UMTS_SF_4) {
        gainFactor = PHY_UMTS_GAIN_FACTOR_4;
    }
    else if (spreadFactor == UMTS_SF_8) {
        gainFactor = PHY_UMTS_GAIN_FACTOR_8;
    }
    else if (spreadFactor == UMTS_SF_16) {
        gainFactor = PHY_UMTS_GAIN_FACTOR_16;
    }
    else if (spreadFactor == UMTS_SF_32) {
        gainFactor = PHY_UMTS_GAIN_FACTOR_32;
    }
    else if (spreadFactor == UMTS_SF_64) {
        gainFactor = PHY_UMTS_GAIN_FACTOR_64;
    }
    else if (spreadFactor == UMTS_SF_128) {
        gainFactor = PHY_UMTS_GAIN_FACTOR_128;
    }
    else if (spreadFactor == UMTS_SF_256) {
        gainFactor = PHY_UMTS_GAIN_FACTOR_256;
    }
    else if (spreadFactor == UMTS_SF_512) {
        gainFactor = PHY_UMTS_GAIN_FACTOR_512;
    }
    else {
        ERROR_ReportError("Invalid Spread Factor");
    }

    return gainFactor;
}

// Calculate the total Pakcet size in a messagge list.
//
// \param msgList  :  pointer to the message list
//
// \return total packet size of teh message list
int UmtsGetTotalPktSizeFromMsgList(Message* msgList)
{
    int pktSize = 0;
    Message* currentMsg = msgList;
    while (currentMsg)
    {
        pktSize += MESSAGE_ReturnPacketSize(currentMsg);
        currentMsg = currentMsg->next;
    }
    return pktSize;
}

