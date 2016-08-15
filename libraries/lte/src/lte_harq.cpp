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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "api.h"
#include "antenna.h"
#include "main.h"
#include "clock.h"
#include "random.h"
#include "partition.h"
#include "propagation.h"
#include "message.h"
#include "phy.h"

#include "phy_lte.h"
#include "phy_lte_establishment.h"
#include "lte_harq.h"
#include "lte_common.h"

#ifdef LTE_LIB_LOG
#include "log_lte.h"
#endif // LTE_LIB_LOG

#ifdef LTE_LIB_LOG
void LteLibLogRBHistory(UInt8* history, std::string* str)
{
    std::stringstream ss;
    for (int i = 0; i < LTE_MAX_NUM_RB; i++)
    {
        ss << (unsigned short)history[i];
    }
    *str = ss.str();
}
void LteLibLogAppendHistory(HarqPhyLteReceiveBuffer* _receiveBuffer, std::string* str)
{
    std::stringstream ss;

    for (int i = 0; i < 4; i++)
    {
        if (i < _receiveBuffer->_mcsHistory->size())
        {
            ss << (unsigned short)(_receiveBuffer->_mcsHistory->at(i));
            ss << ",";
        }
        else {
            ss << "-,";
        }
    }
    for (int i = 0; i < 4; i++)
    {
        if (i < _receiveBuffer->_rvidxHistory->size())
        {
            ss << _receiveBuffer->_rvidxHistory->at(i);
            ss << ",";
        }
        else {
            ss << "-,";
        }
    }
    for (int i = 0; i < 4; i++)
    {
        if (i < _receiveBuffer->_sinrHistory->size())
        {
            std::vector<double>::iterator ite;
            for (ite = _receiveBuffer->_sinrHistory->at(i).begin();
                ite != _receiveBuffer->_sinrHistory->at(i).end();
                ite++)
            {
                ss << *ite;
                ss << "-";
            }
            ss << ",";
        }
        else {
            ss << "-,";
        }
    }
    for (int i = 0; i < 4; i++)
    {
        if (i < _receiveBuffer->_rbAllocHistory->size())
        {
            std::vector<int>::iterator ite;
            for (ite = _receiveBuffer->_rbAllocHistory->at(i).begin();
                ite != _receiveBuffer->_rbAllocHistory->at(i).end();
                ite++)
            {
                ss << *ite;
            }
            ss << ",";
        }
        else {
            ss << "-,";
        }
    }
    *str = ss.str();
    str->erase(--str->end());

}

void LteLibLogSinr(std::vector<double>* sinrHistory, std::string* str)
{
    std::stringstream ss;
    std::vector<double>::iterator ite;
    for (ite = sinrHistory->begin();
        ite != sinrHistory->end();
        ite++)
    {
        ss << *ite;
        ss << "-";
    }
    *str = ss.str();
    str->erase(--str->end());

}
void LteLibLogRBGroupList(Node* node, int interfaceIndex,
    LteDlSchedulingResultInfo* dlSchedulingResult,std::string* str)
{
    std::vector<int> rbgIndex;
    int phyIndex = LteGetPhyIndexFromMacInterfaceIndex(node, interfaceIndex);
    int numRb = PhyLteGetNumResourceBlocks(node, phyIndex);
    int rbGroupSize = PhyLteGetRbGroupsSize(node, phyIndex, numRb);

    rbgIndex.clear();

    for (int i = 0; i < LTE_MAX_NUM_RB; i+=rbGroupSize)
    {
        if (dlSchedulingResult->allocatedRb[i] == 1)
        {
            rbgIndex.push_back(i / rbGroupSize);
        }
    }

    std::stringstream ss;
    for (int i = 0; i < rbgIndex.size(); i++)
    {
        ss << rbgIndex[i];
        ss << "-";
    }
    *str = ss.str();
    if (str->length() > 0)
    {
        str->erase(str->end()-1);
    }
}

#endif

////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////


// Multiplexing from MAC SDU to MAC PDU
//
// \param node  Pointer to node.
// \param interfaceIndex  Interface index
// \param oppositeRnti  Opposite node RNTI
// \param bearerId  Bearer ID [0, *]
// \param tbSize  Transport Block Size
// \param sduList  List of MAC SDUs
// \param macPdu  Buffer to the pointer of created MAC PDU
// \param withoutPaddingByte  Buffer to the bytes excluding padding bits
//
void MacLteDoMultiplexing(
    Node* node, int interfaceIndex, const LteRnti& oppositeRnti,
    int bearerId, int tbSize,
    std::list < Message* > &sduList, Message** macPdu,
    UInt32* withoutPaddingByte);

// Add BSR
//
// \param node  Pointer to node.
// \param interfaceIndex  Interface index
// \param msg  MAC PDU
// \param tbIndex  TB Index
// \param dlSchedulingResult  DL scheduling result
// \param txProcesses  Dl HARQ Tx Process
//
void MacLteAddDciForDl(
    Node* node, int interfaceIndex, Message* msg,
    int tbIndex,
    LteDlSchedulingResultInfo* dlSchedulingResult,
    MacLteDlHarqTxProcess* txProcesses);

// Calculate Transport Block Size
//
// \param node  Pointer to node.
// \param interfaceIndex  Interface index
// \param mcsIndex  MCS Index
// \param numRB  Number of resource blocks
//
// \return Transport Block Size[bit]
//
int MacLteCalculateTbs(
    Node* node, int interfaceIndex, UInt8 mcsIndex, int numRB);

// Get number of Resource Blocks for DL
//
// \param dlSchedulingResult  DL scheduling result
//
// \return number of RBs
//
int MacLteGetNumberOfRbForDl(
    LteDlSchedulingResultInfo* dlSchedulingResult);

// Get redundancy version from mcs
//
// \param mcs  MCS index
//
// \return redundancy version
//
UInt8 GetCurrentIrvFromMcs(UInt8 mcs);

// Get redundancy version from Irv
//
// \param currentIrv  Irv
//
// \return Redundancy version
//
UInt8 GetRedundancyVersionFromCurrentIrv(UInt8 currentIrv);


// Initialize receive buffer
//
HarqPhyLteReceiveBuffer::HarqPhyLteReceiveBuffer(Node* node)
    : _node(node)
{
    _mcsHistory = new std::vector<UInt8>();
    _rvidxHistory = new std::vector<int>();
    _sinrHistory = new std::vector< std::vector<double> >();
    _rbAllocHistory = new std::vector< std::vector<int> >();
    _tb = NULL;
}

// Finalize receive buffer
//
HarqPhyLteReceiveBuffer::~HarqPhyLteReceiveBuffer()
{
    if (_tb != NULL)
    {
        MESSAGE_Free(_node, _tb);
    }
    delete _mcsHistory;
    delete _rvidxHistory;
    delete _sinrHistory;
    delete _rbAllocHistory;
}

// Clear receive buffer
//
// \param node  Pointer to node.
//
void HarqPhyLteReceiveBuffer::Clear()
{
    // clear buffer
    if (_mcsHistory) {
        _mcsHistory->clear();
    }
    if (_rbAllocHistory) {
        _rbAllocHistory->clear();
    }
    if (_rvidxHistory) {
        _rvidxHistory->clear();
    }
    if (_sinrHistory) {
        _sinrHistory->clear();
    }
    if (_tb != NULL)
    {
        MESSAGE_Free(_node, _tb);
        _tb = NULL;
    }
}

// Constructor
//
// \param node  Node structure
//
MacLteDlHarqTxProcessTbStatus::MacLteDlHarqTxProcessTbStatus(Node* node)
    : _node(node), _isValid(FALSE), _harqFeedback(NACKED),
      _NDI(FALSE), _tb(NULL)
{
}

// Destructor
//
MacLteDlHarqTxProcessTbStatus::~MacLteDlHarqTxProcessTbStatus()
{
    if (_tb != NULL)
    {
        MESSAGE_Free(_node, _tb);
        _tb = NULL;
    }
}

// Constructor of DL HARQ Tx Process
//
// \param node  Node structure
//
MacLteDlHarqTxProcess::MacLteDlHarqTxProcess(Node* node)
{
    for (int i = 0; i < DL_NUM_TB_PER_HARQ_PROCESS; ++i)
    {
        _tbStatus[i] = new MacLteDlHarqTxProcessTbStatus(node);
    }

    _schAllocHistory = new std::vector<LteDlSchedulingResultInfo>();
}

// Destructor of DL HARQ Tx Process
//
MacLteDlHarqTxProcess::~MacLteDlHarqTxProcess()
{
    for (int i = 0; i < DL_NUM_TB_PER_HARQ_PROCESS; ++i)
    {
        delete _tbStatus[i];
    }

    delete _schAllocHistory;
}

// Constructor of DL HARQ Tx Entity
//
// \param node  Node structure
// \param interfaceIndex  MAC interface index
// \param oppositeRnti  UE RNTI
//
MacLteDlHarqTxEntity::MacLteDlHarqTxEntity(
        Node* node, int interfaceIndex, const LteRnti& oppositeRnti)
    : _node(node), _interfaceIndex(interfaceIndex), _oppositeRnti(oppositeRnti)
{
    for (int i = 0; i < DL_HARQ_NUM_HARQ_PROCESS; i++)
    {
        _harqProcess[i] = new MacLteDlHarqTxProcess(node);
    }
}

// Finalize entity
//
// \param node  Pointer to node.
// \param macInterfaceIndex  Interface index
// \param txNodeId  Tx node ID
//
MacLteDlHarqTxEntity::~MacLteDlHarqTxEntity()
{
    for (int i = 0; i < DL_HARQ_NUM_HARQ_PROCESS; i++)
    {
        delete _harqProcess[i];
    }
#ifdef LTE_LIB_LOG
    lte::LteLog::DebugFormat(_node, _interfaceIndex,
        "DELETE_DL_TX_ENTITY",
        "%d",_oppositeRnti.nodeId);
#endif

}

// Process HARQ feedback message
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param rxMsg  Received message
// \param feedback  Feedback message
//
void MacLteDlHarqTxEntity::OnReceveHarqFeedback(Node* node,
                             int phyIndex,
                             Message* rxMsg,
                             PhyLteDlHarqFeedback* feedback,
                             LteRnti* txRnti)
{
    for (int i = 0; i < DL_NUM_TB_PER_HARQ_PROCESS; ++i)
    {
        MacLteDlHarqTxProcessTbStatus* tbStatus = _harqProcess[feedback->_processid]->_tbStatus[i];
        // set feedback message
        if (tbStatus->_isValid == TRUE)
        {
            ERROR_Assert( !(tbStatus->_harqFeedback == ACKED &&
                feedback->_ackNack[i] == NACKED), "nack received after acked");

            tbStatus->_harqFeedback = feedback->_ackNack[i];
        }
    }

#ifdef LTE_LIB_LOG
    lte::LteLog::DebugFormat(node, phyIndex,
        "RCV_HARQFB",
        "%d,%d,%d,%d",
        txRnti->nodeId,feedback->_processid,feedback->_ackNack[0],feedback->_ackNack[1]);
#endif
}

// start signal transmission notification
//
// \param dlSchedulingResult  scheduling result
// \param listMsg  message data
// \param curMsg  message data
//
void MacLteDlHarqTxEntity::OnStartSignalTransmission(
    LteDlSchedulingResultInfo* dlSchedulingResult,
    Message** listMsg,Message** curMsg )
{
#ifdef LTE_LIB_LOG
    std::string rbglist;
    LteLibLogRBGroupList(_node, _interfaceIndex, dlSchedulingResult, &rbglist);
    lte::LteLog::DebugFormat(_node, _interfaceIndex,
        "DL_SCHED_INFO",
        "%d,%d,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d",
        dlSchedulingResult->rnti.nodeId,dlSchedulingResult->harqProcessId,
        LteGetTxSchemeString(dlSchedulingResult->txScheme),rbglist.c_str(),
        dlSchedulingResult->numTransportBlock,
        dlSchedulingResult->mcsIndex[0],dlSchedulingResult->mcsIndex[1],
        dlSchedulingResult->harqProcessId,
        dlSchedulingResult->isNewData[0],dlSchedulingResult->isNewData[1],
        dlSchedulingResult->rvidx[0],dlSchedulingResult->rvidx[1]);
#endif

    UInt32 sumTbs = 0;

    LteMacData* macData = LteLayer2GetLteMacData(_node, _interfaceIndex);

    LteRnti oppositeRnti = dlSchedulingResult->rnti;
    int numTB = dlSchedulingResult->numTransportBlock;
    int numRB = MacLteGetNumberOfRbForDl(dlSchedulingResult);

    bool dciAdded = false;
    int tbIndex = 0;
    Message* dciAddMsg = NULL;
    BOOL isTb[2] = {FALSE,FALSE};

    if (dlSchedulingResult->txScheme == TX_SCHEME_OL_SPATIAL_MULTI)
    {
        ERROR_Assert( dlSchedulingResult->isNewData[0] == dlSchedulingResult->isNewData[1],
            "OLSM TB new data scheduling is wrong.");
    }

    for (int i = 0; i < numTB; i++)
    {
        MacLteDlHarqTxProcessTbStatus* rfTbStatus = _harqProcess[dlSchedulingResult->harqProcessId]->_tbStatus[i];
        Message* macPdu = NULL;

        if (dlSchedulingResult->isNewData[i] == TRUE)
        {
            // get MAC PDU from RLC
            UInt8 mcsIndex =  dlSchedulingResult->mcsIndex[i];
            LteSchedulingResultDequeueInfo* dequeueInfo =
                &(dlSchedulingResult->dequeueInfo[i]);
            int dequeueSizeByte = dequeueInfo->dequeueSizeByte;

            if (dequeueSizeByte == 0)
            {
#ifdef LTE_LIB_LOG
                lte::LteLog::DebugFormat(_node, _interfaceIndex,
                    "TB_RETRIEVE",
                    "%d,%d,%d,%d,%d",dlSchedulingResult->rnti.nodeId,dlSchedulingResult->harqProcessId,i,0,0);
#endif
                continue;
            }

            std::list < Message* > sduList;

            // Dequeue from RLC
            LteRlcDeliverPduToMac(
                    _node, _interfaceIndex,
                oppositeRnti,
                LTE_DEFAULT_BEARER_ID,
                dequeueSizeByte,
                &sduList);

            if (sduList.size() < 1)
            {
#ifdef LTE_LIB_LOG
                lte::LteLog::DebugFormat(_node, _interfaceIndex,
                    "TB_RETRIEVE",
                    "%d,%d,%d,%d,%d",dlSchedulingResult->rnti.nodeId,dlSchedulingResult->harqProcessId,i,0,0);
#endif
                continue;
            }
            macData->statData.numberOfSduFromUpperLayer +=
                sduList.size();

            // Calculate Transport Block Size
            int tbSize = MacLteCalculateTbs(
                    _node, _interfaceIndex, mcsIndex, numRB);
            sumTbs += tbSize;

            isTb[i] = TRUE;
#ifdef LTE_LIB_LOG
            lte::LteLog::DebugFormat(_node, _interfaceIndex,
                "TB_RETRIEVE",
                "%d,%d,%d,%d,%d",dlSchedulingResult->rnti.nodeId,dlSchedulingResult->harqProcessId,i,1,tbSize);
#endif

            // Do multiplexing from MAC SDUs to MAC PDU
            UInt32 withoutPaddingByte = 0;
            MacLteDoMultiplexing(
                    _node, _interfaceIndex, oppositeRnti,
                LTE_DEFAULT_BEARER_ID,
                tbSize, sduList, &macPdu,
                &withoutPaddingByte);

            macData->statData.numberOfPduToLowerLayer++;

            ERROR_Assert( macPdu, "MAC PDU is not created.");

            // clear buffer
            _harqProcess[dlSchedulingResult->harqProcessId]->_schAllocHistory->clear();

            if (rfTbStatus->_tb != NULL)
            {
                MESSAGE_Free(_node, rfTbStatus->_tb);
            }
            rfTbStatus->_isValid = FALSE;
            rfTbStatus->_tb = NULL;
            rfTbStatus->_harqFeedback = NACKED;

            rfTbStatus->_NDI = !rfTbStatus->_NDI; // toggle
            rfTbStatus->_tb = MESSAGE_Duplicate(_node, macPdu);
        }
        else
        {
            // retry message
            if (rfTbStatus->_tb == NULL)
            {
                continue;
            }
            macPdu = MESSAGE_Duplicate(_node, rfTbStatus->_tb);
        }

        if (macPdu == NULL)
        {
            return;
        }


        if (dciAdded == false)
        {
            dciAdded = true;
            tbIndex = i; // for tb2cwSwapFlag
            dciAddMsg = macPdu;
        }

        // Add Destination Info
        MacLteAddDestinationInfo(
            _node, _interfaceIndex, macPdu, oppositeRnti);


        // Add send message list
        if (*listMsg == NULL)
        {
            *listMsg = macPdu;
            *curMsg = macPdu;
        }
        else
        {
            (*curMsg)->next = macPdu;
            *curMsg = macPdu;
        }
        if (_node->macData[_interfaceIndex]->macStats)
        {
#ifdef ADDON_DB
            LteStatsDbSduPduInfo* macPduInfo =
                (LteStatsDbSduPduInfo*)MESSAGE_ReturnInfo(
                              macPdu,
                             (UInt16)INFO_TYPE_LteStatsDbSduPduInfo);
            _node->macData[_interfaceIndex]->stats->
                                        AddFrameSentDataPoints(
                                                _node,
                                                macPdu,
                                                STAT_Unicast,
                                                macPduInfo->ctrlSize,
                                                macPduInfo->dataSize,
                                                _interfaceIndex,
                                                oppositeRnti.nodeId);
#endif
        }
    }

    if (dciAddMsg != NULL)
    {
         MacLteAddDciForDl(
           _node, _interfaceIndex, dciAddMsg, tbIndex, dlSchedulingResult,
           _harqProcess[dlSchedulingResult->harqProcessId]);
    }

    if (dlSchedulingResult->isNewData[0] == TRUE)
    {
        // keep which TB is using
        if (dlSchedulingResult->txScheme == TX_SCHEME_OL_SPATIAL_MULTI)
        {
            _harqProcess[dlSchedulingResult->harqProcessId]->_tbStatus[0]->_isValid = isTb[0];
            _harqProcess[dlSchedulingResult->harqProcessId]->_tbStatus[1]->_isValid = isTb[1];
        }
        else
        {
            if (isTb[0] == TRUE)
            {
                _harqProcess[dlSchedulingResult->harqProcessId]->_tbStatus[0]->_isValid = TRUE;
                _harqProcess[dlSchedulingResult->harqProcessId]->_tbStatus[1]->_isValid = FALSE;
            }
            else
            {
                return;
            }
        }
    }

    _harqProcess[dlSchedulingResult->harqProcessId]->_schAllocHistory->push_back(*dlSchedulingResult);

#ifdef LTE_LIB_LOG
    MacLteDlHarqTxProcessTbStatus* logTb1 = _harqProcess[dlSchedulingResult->harqProcessId]->_tbStatus[0];
    MacLteDlHarqTxProcessTbStatus* logTb2 = _harqProcess[dlSchedulingResult->harqProcessId]->_tbStatus[1];
    LteDciFormat1* logDci1Info = NULL;
    LteDciFormat2a* logDci2aInfo = NULL;
    // DCI format-1?
    if (dciAddMsg != NULL)
    {
        logDci1Info =
            (LteDciFormat1*)MESSAGE_ReturnInfo(
                                dciAddMsg, (unsigned short)INFO_TYPE_LteDci1Info);
    }

    // DCI format-2a?
    if (logDci1Info == NULL && dciAddMsg != NULL)
    {
        logDci2aInfo =
            (LteDciFormat2a*)MESSAGE_ReturnInfo(
                                dciAddMsg, (unsigned short)INFO_TYPE_LteDci2aInfo);
    }
    if (logDci1Info == NULL && logDci2aInfo == NULL && dciAddMsg != NULL)
    {
        logDci2aInfo =
            (LteDciFormat2a*)MESSAGE_ReturnInfo(
                                dciAddMsg, (unsigned short)INFO_TYPE_LteDci2aInfo);
    }
    int tbSize1 = 0;
    int tbSize2 = 0;
    if (logDci1Info != NULL)
    {
        // Calculate Transport Block Size
        tbSize1 = MESSAGE_ReturnPacketSize(dciAddMsg);
        std::string str;
        LteLibLogRBHistory(logDci1Info->usedRB_list, &str);
        lte::LteLog::DebugFormat(_node, _interfaceIndex,
            "SND_TB",
            "%d,%s,%d,1,%d,%s,%d,%d,%d,%d,-,-,-,-",oppositeRnti.nodeId,LteGetTxSchemeString(dlSchedulingResult->txScheme),numTB,
            logDci1Info->harqProcessId,str.c_str(),
            logDci1Info->ndi,logDci1Info->mcsID,logDci1Info->redundancyVersion,tbSize1);
    }
    if (logDci2aInfo != NULL)
    {
        // Calculate Transport Block Size
        if (logTb1->_isValid == TRUE)
        {
            tbSize1 = MESSAGE_ReturnPacketSize(dciAddMsg);
        }
        if (logTb2->_isValid == TRUE)
        {
            if (logDci2aInfo->tb2cwSwapFlag == FALSE)
            {
                tbSize2 = MESSAGE_ReturnPacketSize(dciAddMsg->next);
            }
            else {
                tbSize2 = MESSAGE_ReturnPacketSize(dciAddMsg);
            }
        }
        std::string str;
        LteLibLogRBHistory(logDci2aInfo->usedRB_list, &str);
        lte::LteLog::DebugFormat(_node, _interfaceIndex,
            "SND_TB",
            "%d,%s,%d,2A,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d",oppositeRnti.nodeId,LteGetTxSchemeString(dlSchedulingResult->txScheme),numTB,
            logDci2aInfo->harqProcessId,str.c_str(),
            logDci2aInfo->forTB[0].ndi,logDci2aInfo->forTB[0].mcsID,logDci2aInfo->forTB[0].redundancyVersion,tbSize1,
            logDci2aInfo->forTB[1].ndi,logDci2aInfo->forTB[1].mcsID,logDci2aInfo->forTB[1].redundancyVersion,tbSize2);
    }
    lte::LteLog::DebugFormat(_node, _interfaceIndex,
        "DMP_DL_TX_PROC",
        "%d,%d,%d,%d,%d,%d,%d,%d",oppositeRnti.nodeId,dlSchedulingResult->harqProcessId,
        tbSize1,logTb1->_NDI,logTb1->_harqFeedback,
        tbSize2,logTb2->_NDI,logTb2->_harqFeedback);
#endif
}

// Terminate retransmission for all of the process.
// Any information used by scheduler is altered so that
// scheduler does not schedule any HARQ retransmission.
//
void MacLteDlHarqTxEntity::TerminateRetransmission()
{
    for (int i = 0; i < DL_HARQ_NUM_HARQ_PROCESS; ++i)
    {
        _harqProcess[i]->_schAllocHistory->clear();
    }
}

// Pre-process of start transmission signal notification
// Scheduling state change ACKED if there is no response from UE.
//
// \param node  Pointer to node.
// \param interfaceIndex  Index of the MAC
//
void MacLteUlHarqRxEntity::OnBeforeStartSignalTransmission()
{

    UInt64 ttiNum =  MacLteGetTtiNumber(_node, _interfaceIndex);

    // tti / 8
    int processId = (ttiNum) % UL_HARQ_NUM_HARQ_PROCESS;

    int txCount = _harqProcess[processId]->_schAllocHistory->size();
    int rxCount = _harqProcess[processId]->_tbStatus[0]->_receiveBuffer->_mcsHistory->size();

   //  ERROR_Assert(txCount >= rxCount, "non-adaptive message received");

    if (_harqProcess[processId]->_tbStatus[0]->_harqFeedback == NACKED
        && txCount != rxCount)
    {
#ifdef LTE_LIB_LOG
        lte::LteLog::WarnFormat(_node, _interfaceIndex,
                LTE_STRING_LAYER_TYPE_MAC,
                "TX_RX_COUNT_DIFFER,"
                "%d,%d,processId=,%d,tx=,%d,rx=,%d",
                _oppositeRnti.nodeId, _oppositeRnti.interfaceIndex,
                processId,txCount, rxCount);

#endif
        _harqProcess[processId]->_tbStatus[0]->_harqFeedback = ACKED;
    }
}



// Start signal transmission notification
//
// \param ulSchedulingResult  scheduling result
// \param msg  message data.add dci0format
//
void MacLteUlHarqRxEntity::OnStartSignalTransmission(
        LteUlSchedulingResultInfo* ulSchedulingResult,
        Message* msg)
{
    UInt32 sumTbs = 0;

    LteRnti oppositeRnti = ulSchedulingResult->rnti;

    UInt64 ttiNum =  MacLteGetTtiNumber(_node, _interfaceIndex);

    // tti / 8
    int processId = (ttiNum) % UL_HARQ_NUM_HARQ_PROCESS;

    MacLteUlHarqRxProcessTbStatus* rfTbStatus = _harqProcess[processId]->_tbStatus[0];
#ifdef LTE_LIB_LOG
    lte::LteLog::DebugFormat(_node, _interfaceIndex,
        "UL_SCHED_INFO",
        "%d,%d,%d,%d,%d,%d",ulSchedulingResult->rnti.nodeId,processId,
        ulSchedulingResult->startResourceBlock,ulSchedulingResult->numResourceBlocks,
        ulSchedulingResult->mcsIndex,ulSchedulingResult->di.isNewData);
#endif
    if (ulSchedulingResult->di.isNewData == TRUE)
    {
        // clear buffer
        _harqProcess[processId]->_schAllocHistory->clear();
        rfTbStatus->_receiveBuffer->Clear();
        rfTbStatus->_harqFeedback = NACKED;
        rfTbStatus->_NDI = !rfTbStatus->_NDI; // toggle
    }

    // create dci0format
    LteDciFormat0 dci;
    dci.resAllocHeader = 0; // Fixed value
    dci.usedRB_length = (UInt8)ulSchedulingResult->numResourceBlocks;
    dci.usedRB_start = ulSchedulingResult->startResourceBlock;
    dci.freqHopFlag = 0; // Fixed value
    dci.mcsID = ulSchedulingResult->mcsIndex;
    dci.ndi = rfTbStatus->_NDI;

    char* info = MESSAGE_AddInfo(
                _node, msg, sizeof(LteDciFormat0), INFO_TYPE_LteDci0Info);
    memcpy(info, &dci, sizeof(LteDciFormat0));

    _harqProcess[processId]->_schAllocHistory->push_back(*ulSchedulingResult);
#ifdef LTE_LIB_LOG
    lte::LteLog::DebugFormat(_node, _interfaceIndex,
        "SND_DCI0",
        "%d,%d,%d,%d",ulSchedulingResult->rnti.nodeId,processId,dci.ndi,dci.mcsID);
#endif
}

// Terminate retransmission for all of the process.
// Note that reception contexts are remain unchanged.
//
void MacLteUlHarqRxEntity::TerminateRetransmission()
{
    for (int i = 0; i < UL_HARQ_NUM_HARQ_PROCESS; ++i)
    {
        _harqProcess[i]->_schAllocHistory->clear();
    }
}

// Constructor
//
// \param node  Node* : Pointer to node.
//
MacLteDlHarqRxProcessTbStatus::MacLteDlHarqRxProcessTbStatus(Node* node)
    : _node(node)
{
    _receiveBuffer = new HarqPhyLteReceiveBuffer(node);

    Clear();
}

// Clear TB status
//
void MacLteDlHarqRxProcessTbStatus::Clear()
{
    _isValid = FALSE;
    _NDI = -1;
    _harqFeedback = NACKED;
    _receiveBuffer->Clear();
}

// Destructor
//
MacLteDlHarqRxProcessTbStatus::~MacLteDlHarqRxProcessTbStatus()
{
    delete _receiveBuffer;
}

// Constructor
//
// \param node  Pointer to node.
//
MacLteDlHarqRxProcess::MacLteDlHarqRxProcess(Node* node)
{
    for (int i = 0; i < DL_NUM_TB_PER_HARQ_PROCESS; ++i)
    {
        _tbStatus[i] = new MacLteDlHarqRxProcessTbStatus(node);
    }
}

// Clear DL HARQ Rx Process
//
void MacLteDlHarqRxProcess::Clear()
{
    for (int i = 0; i < DL_NUM_TB_PER_HARQ_PROCESS; ++i)
    {
        _tbStatus[i]->Clear();
    }
}

// Destructor
//
MacLteDlHarqRxProcess::~MacLteDlHarqRxProcess()
{
    for (int i = 0; i < DL_NUM_TB_PER_HARQ_PROCESS; ++i)
    {
        delete _tbStatus[i];
    }
}


// Initialize entity
//
// \param node  Pointer to node.
// \param interfaceIndex  Index of the MAC
//
MacLteDlHarqRxEntity::MacLteDlHarqRxEntity(Node* node,
                         int interfaceIndex)
    : _node(node), _interfaceIndex(interfaceIndex)
{
    for (int i = 0; i < DL_HARQ_NUM_HARQ_PROCESS; ++i)
    {
        _harqProcess[i] = new MacLteDlHarqRxProcess(node);
    }
    _dlFeedbackMessageQueue = new std::map<UInt64, PhyLteDlHarqFeedback>();

#ifdef LTE_LIB_LOG
    lte::LteLog::DebugFormat(node, interfaceIndex,
        "RESET_DL_RX_ENTITY",
        "");
#endif
}

// Clear receive buffer
//
void MacLteDlHarqRxEntity::Clear()
{
    for (int i = 0; i < DL_HARQ_NUM_HARQ_PROCESS; i++)
    {
        _harqProcess[i]->Clear();
    }
    _dlFeedbackMessageQueue->clear();

#ifdef LTE_LIB_LOG
    lte::LteLog::DebugFormat(_node, _interfaceIndex,
        "RESET_DL_RX_ENTITY",
        "");
#endif
}

// Finalize entity
//
// \param node  Pointer to node.
//
MacLteDlHarqRxEntity::~MacLteDlHarqRxEntity()
{
    for (int i = 0; i < DL_HARQ_NUM_HARQ_PROCESS; ++i)
    {
        delete _harqProcess[i];
    }

    delete _dlFeedbackMessageQueue;
}

// Get process ID and numTB from received message
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param rxMsg  Received message
// \param processId  Storage to process ID
// \param numTB  Storage to number of TBs
//
void MacLteDlHarqRxEntity::GetDlHarqRxProcessInfo(Node* node,
                         int phyIndex,
                         Message* rxMsg,
                         int* processId,
                         int* numTB)
{
    // DCI format-1
    LteDciFormat1* dci1 =
        (LteDciFormat1*)MESSAGE_ReturnInfo(
                    rxMsg, (unsigned short)INFO_TYPE_LteDci1Info);

    // DCI format2a
    LteDciFormat2a* dci2a =
        (LteDciFormat2a*)MESSAGE_ReturnInfo(
                    rxMsg,(unsigned short)INFO_TYPE_LteDci2aInfo);

    // get processId
    // Judge transmission scheme used
    LteTxScheme txScheme = PhyLteJudgeTxScheme(node, phyIndex, rxMsg);
    if (txScheme == TX_SCHEME_SINGLE_ANTENNA
        || txScheme == TX_SCHEME_DIVERSITY)
    {
        *processId = dci1->harqProcessId;
        *numTB = 1;
    }
    else
    {
        *processId = dci2a->harqProcessId;
        *numTB = 2;
    }
}

// Process arrival of transport block
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param rxMsg  Received message
// \param txRnti  RNTI of transmitter node
//
void MacLteDlHarqRxEntity::OnSignalArrival(Node* node,
                                  int phyIndex,
                                  Message* rxMsg,
                                  LteRnti* txRnti)
{
    ERROR_Assert(rxMsg != NULL, "rxMsg is NULL");
    // DCI format-1
    LteDciFormat1* dci1 =
        (LteDciFormat1*)MESSAGE_ReturnInfo(
                    rxMsg, (unsigned short)INFO_TYPE_LteDci1Info);

    // DCI format2a
    LteDciFormat2a* dci2a =
        (LteDciFormat2a*)MESSAGE_ReturnInfo(
                    rxMsg,(unsigned short)INFO_TYPE_LteDci2aInfo);

    LteTxScheme txScheme = PhyLteJudgeTxScheme(node, phyIndex, rxMsg);

    // get processId
    // Judge transmission scheme used
    int numTB = 0;
    int processId = 0;
    GetDlHarqRxProcessInfo(node, phyIndex, rxMsg, &processId, &numTB);

    int interfaceIndex =
        LteGetMacInterfaceIndexFromPhyIndex(node, phyIndex);
    LteMacData* macData = LteLayer2GetLteMacData(node, interfaceIndex);

    MacLteDlHarqRxProcess* dlRxProc = _harqProcess[processId];
    if (txScheme == TX_SCHEME_OL_SPATIAL_MULTI)
    {
        if (dci2a->tb2cwSwapFlag == true)
        {
            dlRxProc->_tbStatus[0]->_isValid = FALSE;
            dlRxProc->_tbStatus[1]->_isValid = TRUE;
        }
        else
        {
            if (PhyLteCheckDlTransportBlockInfo(rxMsg->next) == TRUE)
            {
                dlRxProc->_tbStatus[0]->_isValid = TRUE;
                dlRxProc->_tbStatus[1]->_isValid = TRUE;
            }
            else
            {
                dlRxProc->_tbStatus[0]->_isValid = TRUE;
                dlRxProc->_tbStatus[1]->_isValid = FALSE;
            }
        }
    }
    else
    {
        dlRxProc->_tbStatus[0]->_isValid = TRUE;
        dlRxProc->_tbStatus[1]->_isValid = FALSE;
    }

    // isnewData
    Message* rfMsg = rxMsg;
    for (int i=0; i < numTB; i++)
    {
        int tbIndex = i;
        if (txScheme == TX_SCHEME_OL_SPATIAL_MULTI)
        {
            tbIndex = (dci2a->tb2cwSwapFlag ? 1 - i : i);
        }
        BOOL isNewData = FALSE;
        MacLteDlHarqRxProcessTbStatus* rfTbStatus = dlRxProc->_tbStatus[tbIndex];

        if (rfTbStatus->_isValid == FALSE)
        {
            continue;
        }

        if (txScheme == TX_SCHEME_SINGLE_ANTENNA
            || txScheme == TX_SCHEME_DIVERSITY)
        {
            if (rfTbStatus->_NDI != dci1->ndi)
            {
                isNewData = TRUE;
                rfTbStatus->_NDI = dci1->ndi;
            }
        }
        else
        {
            if (rfTbStatus->_NDI != dci2a->forTB[tbIndex].ndi)
            {
                isNewData = TRUE;
                rfTbStatus->_NDI = dci2a->forTB[tbIndex].ndi;
            }
        }

        if (isNewData == TRUE)
        {
            // new data
            // buffer clear
            rfTbStatus->_receiveBuffer->Clear();
            rfTbStatus->_harqFeedback = NACKED;

            if ((tbIndex == 0) ||
                (tbIndex == 1 && PhyLteCheckDlTransportBlockInfo(rfMsg) == TRUE))
            {
                rfTbStatus->_receiveBuffer->_tb = MESSAGE_Duplicate(node, rfMsg);
            }
            else {
                rfTbStatus->_receiveBuffer->_tb = NULL;
            }

            macData->statData.numberOfFirstReceiveTb++;

#ifdef LTE_LIB_LOG
            lte::LteLog::DebugFormat(node, phyIndex,
                "DLHARQRX",
                "%d,%d,%d,1",
                txRnti->nodeId,processId,tbIndex);
#endif
        }
        else{
#ifdef LTE_LIB_LOG
            lte::LteLog::DebugFormat(node, phyIndex,
                "DLHARQRX",
                "%d,%d,%d,0",
                txRnti->nodeId,processId,tbIndex);
#endif
        }

        // add mcsHistory, rvidxHistory, rbAllocHistory
        if (txScheme == TX_SCHEME_SINGLE_ANTENNA
            || txScheme == TX_SCHEME_DIVERSITY)
        {
            rfTbStatus->_receiveBuffer->_mcsHistory->push_back(dci1->mcsID);
            rfTbStatus->_receiveBuffer->_rvidxHistory->push_back(dci1->redundancyVersion);

            std::vector<int> tbAllocHist;
            tbAllocHist.insert(tbAllocHist.begin(), dci1->usedRB_list, dci1->usedRB_list+LTE_MAX_NUM_RB);
            rfTbStatus->_receiveBuffer->_rbAllocHistory->push_back(tbAllocHist);
        }
        else
        {
            rfTbStatus->_receiveBuffer->_mcsHistory->push_back(dci2a->forTB[tbIndex].mcsID);
            rfTbStatus->_receiveBuffer->_rvidxHistory->push_back(dci2a->forTB[tbIndex].redundancyVersion);

            std::vector<int> tbAllocHist;
            tbAllocHist.insert(tbAllocHist.begin(), dci2a->usedRB_list, dci2a->usedRB_list+LTE_MAX_NUM_RB);
            rfTbStatus->_receiveBuffer->_rbAllocHistory->push_back(tbAllocHist);
        }

        macData->statData.numberOfReceiveTb++;
        rfMsg = rxMsg->next;
    }
#ifdef LTE_LIB_LOG
    if (dci1 != NULL)
    {
        std::string str;
        LteLibLogRBHistory(dci1->usedRB_list, &str);
        lte::LteLog::DebugFormat(node, phyIndex,
            "RCV_TB_A",
            "%d,%s,%d,1,%d,%s,%d,%d,%d,%d,-,-,-,-",
            txRnti->nodeId,LteGetTxSchemeString(txScheme),numTB,
            dci1->harqProcessId,str.c_str(),dci1->ndi,dci1->mcsID,dci1->redundancyVersion,MESSAGE_ReturnPacketSize(rxMsg));
    }

    if (dci2a != NULL)
    {
        int logTBS = 0;
        if (PhyLteCheckDlTransportBlockInfo(rxMsg->next) == TRUE) {
            logTBS = MESSAGE_ReturnPacketSize(rxMsg->next);
        }
        std::string str;
        LteLibLogRBHistory(dci2a->usedRB_list, &str);
        lte::LteLog::DebugFormat(node, phyIndex,
            "RCV_TB_A",
            "%d,%s,%d,2A,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d",
            txRnti->nodeId,LteGetTxSchemeString(txScheme),numTB,
            dci2a->harqProcessId,str.c_str(),
            dci2a->forTB[0].ndi,dci2a->forTB[0].mcsID,dci2a->forTB[0].redundancyVersion,MESSAGE_ReturnPacketSize(rxMsg),
            dci2a->forTB[1].ndi,dci2a->forTB[1].mcsID,dci2a->forTB[1].redundancyVersion,logTBS);
    }

    MacLteDlHarqRxProcessTbStatus* logTb1 = _harqProcess[processId]->_tbStatus[0];
    MacLteDlHarqRxProcessTbStatus* logTb2 = _harqProcess[processId]->_tbStatus[1];

    std::string str1;
    LteLibLogAppendHistory(logTb1->_receiveBuffer, &str1);
    std::string str2;
    LteLibLogAppendHistory(logTb2->_receiveBuffer, &str2);
    int tbSize1 = 0;
    int tbSize2 = 0;
    if (logTb1->_receiveBuffer->_tb != NULL)
    {
        tbSize1 = MESSAGE_ReturnPacketSize(logTb1->_receiveBuffer->_tb);
    }
    if (logTb2->_receiveBuffer->_tb != NULL)
    {
        tbSize2 = MESSAGE_ReturnPacketSize(logTb2->_receiveBuffer->_tb);
    }
    lte::LteLog::DebugFormat(node, phyIndex,
        "DMP_DL_RX_PROC_A",
        "%d,%d,%d,%d,%d,%s,%d,%d,%d,%s",txRnti->nodeId,processId,
        logTb1->_NDI,logTb1->_harqFeedback,tbSize1,str1.c_str(),
        logTb2->_NDI,logTb2->_harqFeedback,tbSize2,str2.c_str());


#endif
}

// sinr notification
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param rxMsg  Received message
// \param sinr  sinr
// \param txRnti  txRnti
//
void MacLteDlHarqRxEntity::OnSinrNotification(Node* node,
                                  int phyIndex,
                                  Message* rxMsg,
                                  int numCodeWords,
                                  bool tb2cwSwapFlag,
                                  std::vector< std::vector<double> >* sinr,
                                  LteRnti* txRnti)
{
    int numTB = 0;
    int processId = 0;
    GetDlHarqRxProcessInfo(node, phyIndex, rxMsg, &processId, &numTB);

    for (int i = 0; i < numCodeWords; i++)
    {
        int tbIndex = (tb2cwSwapFlag ? 1 - i : i);
        _harqProcess[processId]->_tbStatus[tbIndex]->_receiveBuffer->_sinrHistory->push_back(sinr->at(tbIndex));
#ifdef LTE_LIB_LOG
        std::string str;
        LteLibLogSinr(&sinr->at(tbIndex),&str);
        lte::LteLog::DebugFormat(node, phyIndex,
            "DLSINR",
            "%d,%d,%d,%s",txRnti->nodeId,processId,tbIndex,str.c_str());
#endif
    }
}

// signal end
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param rxMsg  Received message
// \param srcRnti  srcRnti
//
void MacLteDlHarqRxEntity::OnSignalEnd(Node* node,
                                  int phyIndex,
                                  Message* rxMsg,
                                  LteRnti* srcRnti)
{
    int interfaceIndex =
          LteGetMacInterfaceIndexFromPhyIndex(node, phyIndex);

    LteTxScheme txScheme = PhyLteJudgeTxScheme(node, phyIndex, rxMsg);

    int processId = 0;
    int numTB = 0;
    GetDlHarqRxProcessInfo(node, phyIndex, rxMsg, &processId, &numTB);

    PhyData* thisPhy   = node->phyData[phyIndex];
    PhyDataLte* phyLte = (PhyDataLte*)thisPhy->phyVar;

    PhyLteDlHarqFeedback feedback;
    feedback._ackNack[0] = ACKED;
    feedback._ackNack[1] = ACKED;
    feedback._processid = processId;

    Message* rfMsg = rxMsg;
    for (int i = 0; i < numTB; i++)
    {
        int tbIndex = i;
        if (txScheme == TX_SCHEME_OL_SPATIAL_MULTI)
        {
            // DCI format2a
            LteDciFormat2a* dci2a =
                (LteDciFormat2a*)MESSAGE_ReturnInfo(
                            rxMsg,(unsigned short)INFO_TYPE_LteDci2aInfo);

            tbIndex = (dci2a->tb2cwSwapFlag ? 1 - i : i);
        }


        if (_harqProcess[processId]->_tbStatus[tbIndex]->_isValid == FALSE)
        {
            feedback._ackNack[tbIndex] = ACKED;
            _harqProcess[processId]->_tbStatus[tbIndex]->_harqFeedback = ACKED;
        }
        else if (_harqProcess[processId]->_tbStatus[tbIndex]->_harqFeedback == NACKED)
        {
            double bler, refSinr_dB; // For log
            BOOL isError = PhyLteEvaluateError(node, phyIndex,
                    TRUE, _harqProcess[processId]->_tbStatus[tbIndex]->_receiveBuffer,
                    &bler, &refSinr_dB);


            int transportBlockSize = PhyLteGetDlTxBlockSize(node, phyIndex,
                    _harqProcess[processId]->_tbStatus[tbIndex]->_receiveBuffer->
                        _mcsHistory->at(0),
                    _harqProcess[processId]->_tbStatus[tbIndex]->_receiveBuffer->
                        _sinrHistory->at(0).size());

            if (isError == FALSE)
            {
                // no error
                // send message to mac
                if (( tbIndex==0 && rfMsg != NULL) ||
                    ( tbIndex==1 && PhyLteCheckDlTransportBlockInfo(rfMsg) == TRUE))
                {
                    MESSAGE_AppendInfo(node,
                       rfMsg,
                       sizeof(PhyLteTxInfo),
                       (unsigned short)INFO_TYPE_LtePhyLtePhyToMacInfo);

                    PhyLtePhyToMacInfo* info =
                        (PhyLtePhyToMacInfo*)MESSAGE_ReturnInfo(
                                            rfMsg,
                                            (unsigned short)INFO_TYPE_LtePhyLtePhyToMacInfo);

                    info->isError = FALSE;
                    info->srcRnti = *srcRnti;

                    Message* phy2mac = MESSAGE_Duplicate(node, rfMsg);
#ifdef LTE_LIB_LOG
                    lte::LteLog::DebugFormat(node, phyIndex,
                        "FWD_TB",
                        "%d,%d,%d",srcRnti->nodeId,processId, transportBlockSize);
#endif

                    MESSAGE_SetInstanceId(phy2mac, (short) phyIndex);

                    MAC_ReceivePacketFromPhy(
                        node,
                        interfaceIndex,
                        phy2mac);

                    phyLte->stats.totalRxTbsToMac++;

                    phyLte->stats.totalBitsToMac += transportBlockSize;
                }
                // ACK
                feedback._ackNack[tbIndex] = ACKED;
                _harqProcess[processId]->_tbStatus[tbIndex]->_harqFeedback = ACKED;
            }
            else
            {
                // send NACK after 4subframe
                feedback._ackNack[tbIndex] = NACKED;
                _harqProcess[processId]->_tbStatus[tbIndex]->_harqFeedback = NACKED;

                phyLte->stats.totalRxTbsWithErrors++;
            }

#ifdef LTE_LIB_LOG
#ifdef LTE_LIB_VALIDATION_LOG

            int numTx = _harqProcess[processId]->_tbStatus[tbIndex]->_receiveBuffer->_mcsHistory->size();
            int firstMcs = _harqProcess[processId]->_tbStatus[tbIndex]->_receiveBuffer->_mcsHistory->at(0);

            // Aggregate HARQ related metrics for each opposite nodes.
            if (node->getNodeTime() >=
                    lte::LteLog::getValidationStatOffset()*SECOND)
            {
               // Aggregate metrics on 1st-transmission
               if (numTx == 1)
               {
                   // Average MCS on 1st transmission
                   (*phyLte->avgMcs)[*srcRnti].add(firstMcs);

                   // BLER of 1st transmission
                   if (isError == FALSE)
                   {
                       (*phyLte->bler)[*srcRnti].add(0.0);
                   }else
                   {
                       (*phyLte->bler)[*srcRnti].add(1.0);
                   }

                   // Average BLER calculated by L2S mapping model on 1st transmission
                   (*phyLte->blerLookup)[*srcRnti].add(bler);

                   const std::vector<double>& sinrdB_Hist =
                           _harqProcess[processId]->_tbStatus[tbIndex]->_receiveBuffer->
                           _sinrHistory->at(0);

                   double avgSinr_nondB = 0.0;
                   for (int ii = 0; ii < sinrdB_Hist.size(); ++ii)
                       avgSinr_nondB += NON_DB(sinrdB_Hist[ii])/sinrdB_Hist.size();

                   // Average SINR on 1st transmission
                   (*phyLte->avgReceiveSinr)[*srcRnti].add(IN_DB(avgSinr_nondB));

                   // Average reference SINR on 1st transmission
                   (*phyLte->avgEffectiveSinr)[*srcRnti].add(refSinr_dB);

                   lte::LteLog::InfoFormat(
                        node, node->phyData[phyIndex]->macInterfaceIndex,
                            LTE_STRING_LAYER_TYPE_PHY,
                            "ReceiveSinr,%d,%e",
                            srcRnti->nodeId,
                            IN_DB(avgSinr_nondB));
               }

               // Aggregate number of HARQ transmissions conducted.
               // When TB has no error or reaches maximum HARQ retransmission,
               // number of HARQ retransmission is fixed.
               LteMacConfig* macConfig = GetLteMacConfig(node, interfaceIndex);
               if (isError == FALSE || numTx == macConfig->maxHarqTx)
               {
                   // Have no error or end of HARQ retransmission
                   (*phyLte->avgHarqRetx)[*srcRnti].add(numTx);
               }

               if (isError == FALSE)
               {
                   (*phyLte->totalReceivedBits)[*srcRnti].add(transportBlockSize);
               }
            }
#endif
#endif

#ifdef LTE_LIB_LOG
            lte::LteLog::DebugFormat(node, phyIndex,
                "ERR_EVAL",
                "%d,%d,%d,%d",srcRnti->nodeId,processId,tbIndex,isError);
#endif
        }
        else
        {
            feedback._ackNack[tbIndex] = ACKED;
            _harqProcess[processId]->_tbStatus[tbIndex]->_harqFeedback = ACKED;
#ifdef LTE_LIB_LOG
            lte::LteLog::DebugFormat(node, phyIndex,
                "ERR_EVAL",
                "%d,%d,%d,%d",srcRnti->nodeId,processId,tbIndex,-1);
#endif
        }
        rfMsg = rxMsg->next;
    }

    // insert queue
    UInt64 tti = MacLteGetTtiNumber(node, phyIndex);
    _dlFeedbackMessageQueue->insert(
        std::pair<UInt64,PhyLteDlHarqFeedback>(tti, feedback));

#ifdef LTE_LIB_LOG
    MacLteDlHarqRxProcessTbStatus* logTb1 = _harqProcess[processId]->_tbStatus[0];
    MacLteDlHarqRxProcessTbStatus* logTb2 = _harqProcess[processId]->_tbStatus[1];

    std::string str1;
    LteLibLogAppendHistory(logTb1->_receiveBuffer, &str1);
    std::string str2;
    LteLibLogAppendHistory(logTb2->_receiveBuffer, &str2);
    int tbSize1 = 0;
    int tbSize2 = 0;
    if (logTb1->_receiveBuffer->_tb != NULL)
    {
        tbSize1 = MESSAGE_ReturnPacketSize(logTb1->_receiveBuffer->_tb);
    }
    if (logTb2->_receiveBuffer->_tb != NULL)
    {
        tbSize2 = MESSAGE_ReturnPacketSize(logTb2->_receiveBuffer->_tb);
    }
   lte::LteLog::DebugFormat(node, phyIndex,
        "DMP_DL_RX_PROC_E",
        "%d,%d,%d,%d,%d,%s,%d,%d,%d,%s",srcRnti->nodeId,processId,
        logTb1->_NDI,logTb1->_harqFeedback,tbSize1,str1.c_str(),
        logTb2->_NDI,logTb2->_harqFeedback,tbSize2,str2.c_str());
#endif
}

// Check the feedback message info.
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY.
// \param msg  Pointer to message.
//
// \return TRUE found, FALSE not found.
//
BOOL MacLteIsDlFeedbackMessageInfo(Node* node, int phyIndex, Message* msg)
{
    PhyLteDlHarqFeedback* info =
        (PhyLteDlHarqFeedback*)MESSAGE_ReturnInfo(
                msg,
                (unsigned short)INFO_TYPE_LteHarqFeedback);
    if (info != NULL)
    {
        return TRUE;
    }

    return FALSE;
}

// create feedback message
//
// \param node Pointer to node.
// \param phyIndex  Index of the PHY
// \param msg  created message
// \param feedbackMessage  feedback message
//
void MacLteDlHarqRxEntity::DlHarqAddFeedbackMessage(Node* node,
                                  int interfaceIndex,
                                  Message* msg,
                                  PhyLteDlHarqFeedback* feedbackMessage)
{
    // add feedback message
    PhyLteDlHarqFeedback* info =
        (PhyLteDlHarqFeedback*)MESSAGE_AddInfo(
        node, msg, sizeof(PhyLteDlHarqFeedback), INFO_TYPE_LteHarqFeedback);

    memcpy(info, feedbackMessage, sizeof(PhyLteDlHarqFeedback));
}

// get feedback message
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param rxMsg  Received message
//
// \return  FALSE is no feedback message.
//
BOOL MacLteDlHarqRxEntity::GetHarqFeedbackMessage(Node* node,
                                  int phyIndex,
                                  Message* msg,
                                  const LteRnti* rnti)
{
    BOOL ret = FALSE;

    int interfaceIndex =
        LteGetMacInterfaceIndexFromPhyIndex(node, phyIndex);

    UInt64 nowTti = MacLteGetTtiNumber(node, interfaceIndex) + 1;

    // now TTI - 4 is sentdata
    UInt64 rxTtiNumber = (nowTti - PHY_LTE_DCI_RECEPTION_DELAY);

    std::map<UInt64, PhyLteDlHarqFeedback>* queue =
        _dlFeedbackMessageQueue;

    std::map<UInt64, PhyLteDlHarqFeedback>::iterator ite;

    ite = queue->begin();
    while (ite != queue->end())
    {
        if (ite->first < rxTtiNumber){
            queue->erase(ite++);
        }
        else {
            ite++;
        }
    }

    ite = queue->find(rxTtiNumber);

    if (ite !=queue->end())
    {
        // create feedback message
        DlHarqAddFeedbackMessage(node, phyIndex, msg, &ite->second);

#ifdef LTE_LIB_LOG
        lte::LteLog::DebugFormat(node, phyIndex,
            "SND_HARQFB",
            "%d,%d,%d,%d",rnti->nodeId,ite->second._processid,ite->second._ackNack[0],ite->second._ackNack[1]);
#endif

        // delete this queue
        queue->erase(ite);

        ret = TRUE;
    }
    else
    {
        // no feedback message
        ret = FALSE;
    }

    return ret;
}



///////////// up link ///////

// Constructor
//
// \param node  Pointer to node.
//
MacLteUlHarqTxProcessTbStatus::MacLteUlHarqTxProcessTbStatus(Node* node)
    : _node(node), _tb(NULL)
{
    Clear();
}

// Clear TB status
//
void MacLteUlHarqTxProcessTbStatus::Clear()
{
    if (_tb != NULL)
    {
        MESSAGE_Free(_node, _tb);
        _tb = NULL;
    }
    _NDI = -1;
    _currentTbNb = 0;
    _currentirv = 0;
    _iniMcs = -1;
    _lastValidMcs = -1;
    _harqFeedback = NACKED;
}

// Destructor
//
MacLteUlHarqTxProcessTbStatus::~MacLteUlHarqTxProcessTbStatus()
{
    if (_tb != NULL)
    {
        MESSAGE_Free(_node, _tb);
        _tb = NULL;
    }
}

// Constructor of UL HARQ Tx Process
//
// \param node  Pointer to node.
//
MacLteUlHarqTxProcess::MacLteUlHarqTxProcess(Node* node)
{
    for (int i = 0; i < UL_NUM_TB_PER_HARQ_PROCESS; ++i)
    {
        _tbStatus[i] = new MacLteUlHarqTxProcessTbStatus(node);
    }
}

// Clear UL HARQ Tx Process
//
void MacLteUlHarqTxProcess::Clear()
{
    for (int i = 0; i < UL_NUM_TB_PER_HARQ_PROCESS; ++i)
    {
        _tbStatus[i]->Clear();
    }
}

// Destructor of UL HARQ Tx Process
//
MacLteUlHarqTxProcess::~MacLteUlHarqTxProcess()
{
    for (int i = 0; i < UL_NUM_TB_PER_HARQ_PROCESS; ++i)
    {
        delete _tbStatus[i];
    }
}

// Constructor of UL HARQ Tx Entity
//
// \param node  Pointer to node.
// \param interfaceIndex  Index of the PHY
//
MacLteUlHarqTxEntity::MacLteUlHarqTxEntity(Node* node,
                         int interfaceIndex)
    : _node(node), _interfaceIndex(interfaceIndex)
{
    for (int i = 0; i < UL_HARQ_NUM_HARQ_PROCESS; i++)
    {
        _harqProcess[i] = new MacLteUlHarqTxProcess(node);
    }

    Clear();
}

// Clear UL HARQ Tx Entity
//
void MacLteUlHarqTxEntity::Clear()
{
    for (int i = 0; i < UL_HARQ_NUM_HARQ_PROCESS; i++)
    {
        _harqProcess[i]->Clear();
    }
#ifdef LTE_LIB_LOG
    lte::LteLog::DebugFormat(_node, _interfaceIndex,
        "RESET_UL_TX_ENTITY",
        "");
#endif
}

// Destructor
//
MacLteUlHarqTxEntity::~MacLteUlHarqTxEntity()
{
    for (int i = 0; i < UL_HARQ_NUM_HARQ_PROCESS; i++)
    {
        delete _harqProcess[i];
    }
}

// receive feedback
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param rxMsg  Received message
// \param feedback  feedback message
// \param txRnti  txRnti
//
void MacLteUlHarqTxEntity::OnReceiveHarqFeedback(Node* node,
                             int phyIndex,
                             Message* rxMsg,
                             PhyLteUlHarqFeedback* feedback,
                             LteRnti* txRnti)
{
#ifdef LTE_LIB_LOG
    int interfaceIndex =
        LteGetMacInterfaceIndexFromPhyIndex(node, phyIndex);
    lte::LteLog::DebugFormat(node, interfaceIndex,
        "RCV_HARQFB",
        "%d,%d,%d",txRnti->nodeId,feedback->_processid,feedback->_ackNack[0]);
#endif
    MacLteUlHarqTxProcessTbStatus* tbStatus = _harqProcess[feedback->_processid]->_tbStatus[0];

    ERROR_Assert( !(tbStatus[0]._harqFeedback == ACKED &&
        feedback->_ackNack[0] == NACKED), "nack received after acked");

    // set feedback message
    tbStatus[0]._harqFeedback = feedback->_ackNack[0];

}


// manage TB and NDI
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param interfaceIndex  Index of the MAC
// \param oppositeRnti  oppositeRnti
// \param schedulingInfo  schedulingInfo
// \param macPdu  create message
//
void MacLteUlHarqTxEntity::GetMacPdu(Node* node,
                           int phyIndex,
                           int interfaceIndex,
                           LteRnti* oppositeRnti,
                           LteUlSchedulingResultInfo* schedulingInfo,
                           Message** macPdu)
{
    LteMacData* macData = LteLayer2GetLteMacData(node, interfaceIndex);
    std::list < Message* > sduList;
    // Dequeue from RLC
    LteRlcDeliverPduToMac(
            node, interfaceIndex,
            *oppositeRnti,
            LTE_DEFAULT_BEARER_ID,
            schedulingInfo->dequeueInfo.dequeueSizeByte,
            &sduList);

    if (sduList.size() < 1)
    {
        return;
    }
    macData->statData.numberOfSduFromUpperLayer += sduList.size();

    // Calculate Transport Block Size
    int tbSize = MacLteCalculateTbs(node, interfaceIndex, schedulingInfo->mcsIndex, schedulingInfo->numResourceBlocks);


    // Do multiplexing from MAC SDUs to MAC PDU
    *macPdu = NULL;
    UInt32 withoutPaddingByte = 0;
    MacLteDoMultiplexing(
        node, interfaceIndex, *oppositeRnti,
        LTE_DEFAULT_BEARER_ID,
        tbSize, sduList, macPdu, &withoutPaddingByte);
    macData->statData.numberOfPduToLowerLayer++;
}


// Notification of signal transmission
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param ulSchedulingResult  schedule result
// \param listMsg  send message
// \param nextMsg  send message
// \param numUlResourceBlocks  numUlResourceBlocks
//
// \return TRUE is create message.FALSE no message.
//
BOOL MacLteUlHarqTxEntity::OnSignalTransmission(Node* node,
                             int phyIndex,
                             std::vector < LteUlSchedulingResultInfo >* ulSchedulingResult,
                             Message** listMsg,Message** nextMsg,
                             UInt32* numUlResourceBlocks)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataLte* phyLte = (PhyDataLte*)thisPhy->phyVar;
    int interfaceIndex =
        LteGetMacInterfaceIndexFromPhyIndex(node, phyIndex);
    LteMacConfig* macConfig = GetLteMacConfig(node, interfaceIndex);
    LteMacData* macData = LteLayer2GetLteMacData(node, interfaceIndex);

    UInt64 ttiNum =  MacLteGetTtiNumber(node, interfaceIndex);

    // ( tti + 4 + 1) % 8
    int processId =  (ttiNum + PHY_LTE_DCI_RECEPTION_DELAY + 1) % UL_HARQ_NUM_HARQ_PROCESS;

    BOOL isNewTx = FALSE;
    BOOL isAdaptiveReTx = FALSE;
    BOOL isNonAdaptiveReTx = FALSE;
    BOOL messageSendFlag = FALSE;

    MacLteUlHarqTxProcessTbStatus* tbStatus = _harqProcess[processId]->_tbStatus[0];
    Message* macPdu = NULL;

    // Ret
    // Retrieve RNTI of serving eNB
    ListLteRnti tempList;
    Layer3LteGetSchedulableListSortedByConnectedTime(node, interfaceIndex,
        &tempList);
    const ListLteRnti* listLteRnti = &tempList;

    ERROR_Assert(listLteRnti->size() < 2, "Invalid connected node list");

    if (listLteRnti->size() == 0)
    {
        return FALSE;
    }

    LteRnti servingEnbRnti = listLteRnti->front();
    LteUlSchedulingResultInfo* schedulingInfo = NULL;

    if (ulSchedulingResult->size() > 0)
    {
        schedulingInfo = &ulSchedulingResult->at(0);

        // dciformat0 exists
        if (schedulingInfo->di.ndi != tbStatus->_NDI
            || tbStatus->_tb == NULL)
        {
            // new transmission
            isNewTx = TRUE;
            tbStatus->_NDI = schedulingInfo->di.ndi;
            if (tbStatus->_tb != NULL)
            {
                MESSAGE_Free(node, tbStatus->_tb);
            }
            tbStatus->_tb = NULL;

            ERROR_Assert(schedulingInfo->mcsIndex < 29, "mcs error.");

            tbStatus->_iniMcs = schedulingInfo->mcsIndex;
        }
        else
        {
            // adaptive re-transmission
            isAdaptiveReTx = TRUE;
        }

        if (schedulingInfo->mcsIndex <= 28)
        {
            tbStatus->_lastValidMcs = schedulingInfo->mcsIndex;
        }

        tbStatus->lastRbAlloc.first = schedulingInfo->startResourceBlock;
        tbStatus->lastRbAlloc.second = schedulingInfo->numResourceBlocks;
    }
    else if (tbStatus->_tb != NULL)
    {
        // non adaprive re-transmission
        isNonAdaptiveReTx = TRUE;
    }
    else
    {
        // no message
        return FALSE;
    }

    // set status
    if (isNewTx == TRUE)
    {

        tbStatus->_currentTbNb = 0;
        tbStatus->_currentirv = 0;
        tbStatus->_harqFeedback = NACKED;

        // get dequeueinfo from UE scheduler
        // create macPDU
        GetMacPdu(node, phyIndex,interfaceIndex,
            &servingEnbRnti, schedulingInfo, &macPdu);


        if (macPdu != NULL)
        {
            tbStatus->_tb = MESSAGE_Duplicate(node, macPdu);
            messageSendFlag = TRUE;

            macData->statData.numberOfFirstSendTb++;
#ifdef LTE_LIB_LOG
            int tbSize = MESSAGE_ReturnPacketSize(macPdu);
            lte::LteLog::DebugFormat(node, interfaceIndex,
                "TB_RETRIEVE",
                "%d,%d,%d,%d",servingEnbRnti.nodeId,processId,1,tbSize);
#endif
        }
        else
        {
            //return FALSE;
#ifdef LTE_LIB_LOG
            lte::LteLog::DebugFormat(node, interfaceIndex,
                "TB_RETRIEVE",
                "%d,%d,%d,%d",servingEnbRnti.nodeId,processId,0,0);
#endif
        }
    }
    else if (isAdaptiveReTx == TRUE)
    {
        ERROR_Assert(tbStatus->_tb != NULL, "tbStatus->_tb is failed");
        macPdu = MESSAGE_Duplicate(node, tbStatus->_tb);
        ERROR_Assert(macPdu != NULL, "MESSAGE_AddInfo is failed");

        tbStatus->_currentTbNb += 1;
        tbStatus->_currentirv = GetCurrentIrvFromMcs(schedulingInfo->mcsIndex);
        tbStatus->_harqFeedback = NACKED;

        messageSendFlag = TRUE;
    }
    else if (isNonAdaptiveReTx == TRUE)
    {
        // Non-adaptive re-tx does not send message in current version
        if (tbStatus->_harqFeedback == ACKED)
        {
            return FALSE;
        }
        else {
#ifdef LTE_LIB_LOG
            lte::LteLog::WarnFormat(node, interfaceIndex,
                "NON_ADAPTIVE_RETX",
                "%d,%d",servingEnbRnti.nodeId,processId,0,0);
#endif
        }
    }

    if (messageSendFlag == TRUE)
    {
        // create UL TB TxInfo
        LteUlTbTxInfo txInfo;
        txInfo.mcsID = tbStatus->_lastValidMcs;
        txInfo.usedRB_start = tbStatus->lastRbAlloc.first;
        txInfo.usedRB_length = tbStatus->lastRbAlloc.second;
        txInfo.redundancyVersion = GetRedundancyVersionFromCurrentIrv(tbStatus->_currentirv);
        txInfo.isNewTx = (isNewTx == TRUE) ? 1 : 0;
        txInfo.harqProcessId = processId;

        char* info = MESSAGE_AddInfo(
                    node, macPdu, sizeof(LteUlTbTxInfo), INFO_TYPE_LteUlTbTxInfo);
        ERROR_Assert(info != NULL, "MESSAGE_AddInfo is failed");

        memcpy(info, &txInfo, sizeof(LteUlTbTxInfo));

        // Add Destination Info
        MacLteAddDestinationInfo(node, interfaceIndex, macPdu, servingEnbRnti);
        tbStatus->_currentirv = (tbStatus->_currentirv + 1) % 4;

       // Add send message list
        if (*listMsg == NULL)
        {
            *listMsg = macPdu;
            *nextMsg = (*listMsg)->next;
        }
        else
        {
            *nextMsg = macPdu;
            *nextMsg = (*nextMsg)->next;
        }

#ifdef LTE_LIB_LOG
        int adaptive_flag = -1;
        if (isNewTx == TRUE) {
            adaptive_flag = 0;
        }
        else if (isAdaptiveReTx == TRUE){
            adaptive_flag = 1;
        }
        else if (isNonAdaptiveReTx == TRUE) {
            adaptive_flag = 2;
        }
        lte::LteLog::DebugFormat(node, interfaceIndex,
            "SND_TB",
            "%d,%d,%d,%d,%d,%d,%d,%d",
            servingEnbRnti.nodeId,processId,adaptive_flag,
            txInfo.usedRB_start,txInfo.usedRB_length,txInfo.mcsID,txInfo.isNewTx,txInfo.redundancyVersion);
#endif


        macData->statData.numberOfSendTb++;

        if (node->macData[interfaceIndex]->macStats)
        {
#ifdef ADDON_DB

            LteStatsDbSduPduInfo* macPduInfo =
                (LteStatsDbSduPduInfo*)MESSAGE_ReturnInfo(
                              macPdu,
                             (UInt16)INFO_TYPE_LteStatsDbSduPduInfo);
            if (macPduInfo != NULL) {
            node->macData[interfaceIndex]->stats->
                                        AddFrameSentDataPoints(
                                                node,
                                                macPdu,
                                                STAT_Unicast,
                                                macPduInfo->ctrlSize,
                                                macPduInfo->dataSize,
                                                interfaceIndex,
                                                servingEnbRnti.nodeId);
            }
#endif
        }

        *numUlResourceBlocks += txInfo.usedRB_length;
    } // messageSendFlag == TRUE

    if (tbStatus->_currentTbNb == (macConfig->maxHarqTx - 1))
    {
        // clear buffer
        if (tbStatus->_tb != NULL)
        {
            MESSAGE_Free(node, tbStatus->_tb);
        }
        tbStatus->_tb = NULL;
    }

#ifdef LTE_LIB_LOG
    int tbsize = 0;
    if (tbStatus->_tb != NULL)
    {
        tbsize = MESSAGE_ReturnPacketSize(tbStatus->_tb);
    }
    lte::LteLog::DebugFormat(node, interfaceIndex,
        "DMP_UL_TX_PROC",
        "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
        servingEnbRnti.nodeId,processId,
        tbsize,tbStatus->_NDI,tbStatus->_currentTbNb,tbStatus->_currentirv,tbStatus->_harqFeedback,tbStatus->_iniMcs,
        tbStatus->_lastValidMcs,tbStatus->lastRbAlloc.first,tbStatus->lastRbAlloc.second);
#endif

    return messageSendFlag;
}

// Constructor
//
// \param node  Node structure
//
MacLteUlHarqRxProcessTbStatus::MacLteUlHarqRxProcessTbStatus(Node* node)
    : _node(node)
{
    _receiveBuffer = new HarqPhyLteReceiveBuffer(node);
    _NDI = FALSE;
    _harqFeedback = ACKED;
}

// Destructor
//
// \param node
//
MacLteUlHarqRxProcessTbStatus::~MacLteUlHarqRxProcessTbStatus()
{
    delete _receiveBuffer;
}

// Constructor
//
// \param node  : Node structure
//
MacLteUlHarqRxProcess::MacLteUlHarqRxProcess(Node* node)
{
    _tbStatus[0] = new MacLteUlHarqRxProcessTbStatus(node);
    _schAllocHistory = new std::vector<LteUlSchedulingResultInfo>();
}

// Destructor
//
MacLteUlHarqRxProcess::~MacLteUlHarqRxProcess()
{
    delete _tbStatus[0];
    delete _schAllocHistory;
}

// Constructor of UL HARQ Rx Entity
//
// \param node  Node structure
// \param interfaceIndex  MAC interface index
// \param oppositeRnti  UE RNTI
//
MacLteUlHarqRxEntity::MacLteUlHarqRxEntity(
        Node* node, int interfaceIndex, const LteRnti& oppositeRnti)
    : _node(node), _interfaceIndex(interfaceIndex),
      _oppositeRnti(oppositeRnti), _ulFeedbackMessageQueue(NULL)
{
    for (int i = 0; i < UL_HARQ_NUM_HARQ_PROCESS; i++)
    {
        _harqProcess[i] = new MacLteUlHarqRxProcess(node);
    }
    _ulFeedbackMessageQueue = new std::map<UInt64, PhyLteUlHarqFeedback>();
}


// finalize
//
// \param node  Pointer to node.
//
MacLteUlHarqRxEntity::~MacLteUlHarqRxEntity()
{

    for (int i = 0; i < UL_HARQ_NUM_HARQ_PROCESS; i++)
    {
        delete _harqProcess[i];
    }
    delete _ulFeedbackMessageQueue;
#ifdef LTE_LIB_LOG
    lte::LteLog::DebugFormat(_node, _interfaceIndex,
        "DELETE_UL_RX_ENTITY",
        "%d",_oppositeRnti.nodeId);
#endif

}

// Get Irv from mcs
//
// \param mcs  MCS index
//
UInt8 GetCurrentIrvFromMcs(UInt8 mcs)
{
    UInt8 ret = 0;

    switch (mcs)
    {
    case 29:
        ret = 3;
        break;
    case 30:
        ret = 1;
        break;
    case 31:
        ret = 2;
        break;
    default:
        ret = 0;
        break;

    }
    return ret;
}

// Get redundancy version from Irv
//
// \param currentIrv  Irv
//
// \return Redundancy version
//
UInt8 GetRedundancyVersionFromCurrentIrv(UInt8 currentIrv)
{
    UInt8 ret = 0;

    switch (currentIrv)
    {
    case 1:
        ret = 2;
        break;
    case 2:
        ret = 3;
        break;
    case 3:
        ret = 1;
        break;
    default:
        ret = 0;
        break;

    }
    return ret;
}

// receive UL message
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param rxMsg  Received message
// \param ulTbTxInfo  UL TB TxInfo
// \param txRnti  txRnti
//
void MacLteUlHarqRxEntity::OnSignalArrival(
                        Node *node,
                        int phyIndex,
                        Message *rxMsg,
                        LteUlTbTxInfo* ulTbTxInfo,
                        LteRnti* txRnti)
{
    int interfaceIndex =
        LteGetMacInterfaceIndexFromPhyIndex(node, phyIndex);

    UInt64 ttiNum =  MacLteGetTtiNumber(node, interfaceIndex);

    int processId = ulTbTxInfo->harqProcessId; // get from txInfo

    MacLteUlHarqRxProcessTbStatus* tbStatus = _harqProcess[processId]->_tbStatus[0];
    // new data
    if (ulTbTxInfo->isNewTx == 1)
    {
        // buffer clear
        tbStatus->_receiveBuffer->Clear();
        tbStatus->_harqFeedback = NACKED;

        tbStatus->_receiveBuffer->_tb = MESSAGE_Duplicate(node, rxMsg);
    }

    tbStatus->_receiveBuffer->_mcsHistory->push_back(ulTbTxInfo->mcsID);
    tbStatus->_receiveBuffer->_rvidxHistory->push_back(ulTbTxInfo->redundancyVersion);

    std::vector<int> usedRB_list(PHY_LTE_MAX_NUM_RB, 0);
    for (UInt8 i = 0; i < ulTbTxInfo->usedRB_length; i ++)
    {
        usedRB_list[ulTbTxInfo->usedRB_start + i] = 1;
    }
    tbStatus->_receiveBuffer->_rbAllocHistory->push_back(usedRB_list);


#ifdef LTE_LIB_LOG
    lte::LteLog::DebugFormat(node, interfaceIndex,
        "RCV_TB_A",
        "%d,%d,%d,%d,%d,%d,%d",
        txRnti->nodeId,processId,
        ulTbTxInfo->usedRB_start,ulTbTxInfo->usedRB_length,ulTbTxInfo->mcsID,ulTbTxInfo->isNewTx,ulTbTxInfo->redundancyVersion);
#endif
#ifdef LTE_LIB_LOG
    int tbsize = 0;
    if (tbStatus->_receiveBuffer->_tb != NULL)
    {
        tbsize = MESSAGE_ReturnPacketSize(tbStatus->_receiveBuffer->_tb);
    }
    std::string str1;
    LteLibLogAppendHistory(tbStatus->_receiveBuffer, &str1);
    lte::LteLog::DebugFormat(node, interfaceIndex,
        "DMP_UL_RX_PROC_A",
        "%d,%d,%d,%d,%s",
        txRnti->nodeId,processId,
        tbStatus->_harqFeedback,tbsize,str1.c_str());
#endif
}

// sinr notification
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param rxMsg  Received message
// \param sinr  sinr
// \param txRnti  txRnti
//
void MacLteUlHarqRxEntity::OnSinrNotification(Node* node,
                                  int phyIndex,
                                  Message* rxMsg,
                                  std::vector< std::vector<double> >* sinr,
                                  LteRnti* txRnti)
{
    int interfaceIndex =
        LteGetMacInterfaceIndexFromPhyIndex(node, phyIndex);

    UInt64 ttiNum =  MacLteGetTtiNumber(node, interfaceIndex);

    int processId = (ttiNum + PHY_LTE_DCI_RECEPTION_DELAY) % UL_HARQ_NUM_HARQ_PROCESS;

    // TB = 1
    _harqProcess[processId]->_tbStatus[0]->_receiveBuffer->_sinrHistory->push_back(sinr->at(0));

#ifdef LTE_LIB_LOG
    std::string str;
    LteLibLogSinr(&sinr->at(0),&str);
    lte::LteLog::DebugFormat(node, phyIndex,
        "ULSINR",
        "%d,%d,%s",txRnti->nodeId,processId,str.c_str());
#endif
}

// UL Rx signal end
//
// \param node  Pointer to node.
// \param phyIndex  Index of the PHY
// \param rxMsg  Received message
// \param srcRnti  rnti
//
void MacLteUlHarqRxEntity::OnSignalEnd(Node* node,
                                  int phyIndex,
                                  Message* rxMsg,
                                  LteRnti* srcRnti)
{
    PhyData* thisPhy = node->phyData[phyIndex];
    PhyDataLte* phyLte = (PhyDataLte*)thisPhy->phyVar;

    int interfaceIndex =
        LteGetMacInterfaceIndexFromPhyIndex(node, phyIndex);

    UInt64 ttiNum =  MacLteGetTtiNumber(node, interfaceIndex);

    // UL TB Tx Info
    LteUlTbTxInfo* ulTbTxInfo =
        (LteUlTbTxInfo*)MESSAGE_ReturnInfo(
            rxMsg,
        (unsigned short)INFO_TYPE_LteUlTbTxInfo);

    int processId = ulTbTxInfo->harqProcessId; // get from txInfo

    PhyLteUlHarqFeedback feedback;
    feedback._processid = processId;

    if (_harqProcess[processId]->_tbStatus[0]->_harqFeedback == NACKED)
    {
        double bler, refSinr_dB; // For log
        BOOL isError = PhyLteEvaluateError(node, phyIndex,
                FALSE, _harqProcess[processId]->_tbStatus[0]->_receiveBuffer,
                &bler, &refSinr_dB);

        int transportBlockSize = PhyLteGetUlTxBlockSize(node, phyIndex,
                                _harqProcess[processId]->_tbStatus[0]->_receiveBuffer->
                                    _mcsHistory->at(0),
                                    _harqProcess[processId]->_tbStatus[0]->_receiveBuffer->
                                    _sinrHistory->at(0).size());
        if (isError == FALSE)
        {
            // no error
            // send message to mac
            MESSAGE_AppendInfo(node,
               rxMsg,
               sizeof(PhyLteTxInfo),
               (unsigned short)INFO_TYPE_LtePhyLtePhyToMacInfo);

            PhyLtePhyToMacInfo* info =
                (PhyLtePhyToMacInfo*)MESSAGE_ReturnInfo(
                                    rxMsg,
                                    (unsigned short)INFO_TYPE_LtePhyLtePhyToMacInfo);

            info->isError = FALSE;
            info->srcRnti = *srcRnti;
            Message* phy2mac = MESSAGE_Duplicate(node, rxMsg);

            MESSAGE_SetInstanceId(phy2mac, (short) phyIndex);

            MAC_ReceivePacketFromPhy(
                node,
                interfaceIndex,
                phy2mac);

            // ACK
            feedback._ackNack[0] = ACKED;
            _harqProcess[processId]->_tbStatus[0]->_harqFeedback = ACKED;

            phyLte->stats.totalRxTbsToMac++;

            phyLte->stats.totalBitsToMac += transportBlockSize;


#ifdef LTE_LIB_LOG
            lte::LteLog::DebugFormat(node, phyIndex,
                "FWD_TB",
                "%d,%d,%d",srcRnti->nodeId,processId,transportBlockSize);
#endif
        }
        else
        {
            // send NACK after 4subframe
            feedback._ackNack[0] = NACKED;

            phyLte->stats.totalRxTbsWithErrors++;
        }

#ifdef LTE_LIB_LOG
#ifdef LTE_LIB_VALIDATION_LOG

        int numTx = _harqProcess[processId]->_tbStatus[0]->_receiveBuffer->
                                            _mcsHistory->size();

        int firstMcs = _harqProcess[processId]->_tbStatus[0]->_receiveBuffer->
                                            _mcsHistory->at(0);

        // Aggregate HARQ related metrics for each opposite nodes.
        if (node->getNodeTime() >=
                lte::LteLog::getValidationStatOffset()*SECOND)
        {
            // Aggregate metrics on 1st-transmission
           if (numTx == 1)
           {
               // Average MCS on 1st transmission
               (*phyLte->avgMcs)[*srcRnti].add(firstMcs);

               // BLER of 1st transmission
               if (isError == FALSE)
               {
                   (*phyLte->bler)[*srcRnti].add(0.0);
               }else
               {
                   (*phyLte->bler)[*srcRnti].add(1.0);
               }

               // Average BLER calculated by L2S mapping model on 1st transmission
               (*phyLte->blerLookup)[*srcRnti].add(bler);

               const std::vector<double>& sinrdB_Hist =
                       _harqProcess[processId]->_tbStatus[0]->_receiveBuffer->
                       _sinrHistory->at(0);

               double avgSinr_nondB = 0.0;
               for (int ii = 0; ii < sinrdB_Hist.size(); ++ii)
                   avgSinr_nondB += NON_DB(sinrdB_Hist[ii])/sinrdB_Hist.size();

               // Average SINR on 1st transmission
               (*phyLte->avgReceiveSinr)[*srcRnti].add(IN_DB(avgSinr_nondB));

               // Average reference SINR on 1st transmission
               (*phyLte->avgEffectiveSinr)[*srcRnti].add(refSinr_dB);

               lte::LteLog::InfoFormat(
                    node, node->phyData[phyIndex]->macInterfaceIndex,
                        LTE_STRING_LAYER_TYPE_PHY,
                        "ReceiveSinr,%d,%e",
                        srcRnti->nodeId,
                        IN_DB(avgSinr_nondB));
           }

           // Aggregate number of HARQ transmissions conducted.
           // When TB has no error or reaches maximum HARQ retransmission,
           // number of HARQ retransmission is fixed.
           LteMacConfig* macConfig = GetLteMacConfig(node, interfaceIndex);
           if (isError == FALSE || numTx == macConfig->maxHarqTx)
           {
               (*phyLte->avgHarqRetx)[*srcRnti].add(numTx);
           }

           if (isError == FALSE)
           {
                (*phyLte->totalReceivedBits)[*srcRnti].add(transportBlockSize);
           }
        }
#endif
#endif



#ifdef LTE_LIB_LOG
        lte::LteLog::DebugFormat(node, phyIndex,
            "ERR_EVAL",
            "%d,%d,%d",srcRnti->nodeId,processId,isError);
#endif
    }
    else
    {
        feedback._ackNack[0] = ACKED;
#ifdef LTE_LIB_LOG
        lte::LteLog::DebugFormat(node, phyIndex,
            "ERR_EVAL",
            "%d,%d,%d",srcRnti->nodeId,processId,-1);
#endif
    }

    // insert queue
    _ulFeedbackMessageQueue->insert(
        std::pair<UInt64,PhyLteUlHarqFeedback>(ttiNum, feedback));

#ifdef LTE_LIB_LOG
    MacLteUlHarqRxProcessTbStatus* tbStatus = _harqProcess[processId]->_tbStatus[0];
    int tbsize = 0;
    if (tbStatus->_receiveBuffer->_tb != NULL)
    {
        tbsize = MESSAGE_ReturnPacketSize(tbStatus->_receiveBuffer->_tb);
    }
    std::string str1;
    LteLibLogAppendHistory(tbStatus->_receiveBuffer, &str1);
    lte::LteLog::DebugFormat(node, interfaceIndex,
        "DMP_UL_RX_PROC_E",
        "%d,%d,%d,%d,%s",
        srcRnti->nodeId,processId,
        tbStatus->_harqFeedback,tbsize,str1.c_str());
#endif
}

// create feedback message
//
// \param node  \param.
// \param phyIndex  Index of the PHY
// \param msg  created message
// \param feedbackMessage  feedback message
// \param rnti  const LteRnti* : rnti
//
void  MacLteUlHarqRxEntity::AddFeedbackMessage(Node* node,
                                  int phyIndex,
                                  Message** msg,
                                  PhyLteUlHarqFeedback* feedbackMessage,
                                  const LteRnti* rnti)
{
    // feedback message
    *msg = MESSAGE_Alloc(node, 0, 0, 0);

    PhyLteUlHarqFeedback* info =
        (PhyLteUlHarqFeedback*)MESSAGE_AddInfo(
        node, *msg, sizeof(PhyLteUlHarqFeedback), INFO_TYPE_LteHarqFeedback);

    memcpy(info, feedbackMessage, sizeof(PhyLteUlHarqFeedback));


    MacLteMsgDestinationInfo* dstInfo =
        (MacLteMsgDestinationInfo*)MESSAGE_AddInfo(
                                    node,
                                    *msg,
                                    sizeof(MacLteMsgDestinationInfo),
                                    (UInt16)INFO_TYPE_LteMacDestinationInfo);
    memcpy(&dstInfo->dstRnti, rnti, sizeof(LteRnti));
}


// Get a HARQ feedback message that should be processed at
//  current TTI.
//
// \param node         : Node* : Pointer to node.
// \param phyIndex     : int   : Index of the PHY
// \param rxMsg        : Message* : feedback message
// \param rnti         : const LteRnti* : rnti
//
void MacLteUlHarqRxEntity::GetFeedbackMessage(Node* node,
                                  int phyIndex,
                                  Message** msg,
                                  const LteRnti* rnti)
{

    int interfaceIndex =
        LteGetMacInterfaceIndexFromPhyIndex(node, phyIndex);

    UInt64 ttiNum = MacLteGetTtiNumber(node, interfaceIndex);

    int processId = (ttiNum) % UL_HARQ_NUM_HARQ_PROCESS;

    // now TTI - 4 is sentdata
    UInt64 rxTtiNumber = (ttiNum - PHY_LTE_DCI_RECEPTION_DELAY);

    std::map<UInt64, PhyLteUlHarqFeedback>* queue =
        _ulFeedbackMessageQueue;


    std::map<UInt64, PhyLteUlHarqFeedback>::iterator ite;

    ite = queue->begin();
    while (ite != queue->end())
    {
        if (ite->first < rxTtiNumber){
            queue->erase(ite++);
        }
        else {
            ite++;
        }
    }

    ite = queue->find(rxTtiNumber);

    if (ite !=queue->end())
    {
        // set message
        AddFeedbackMessage(node, phyIndex, msg, &ite->second, rnti);
#ifdef LTE_LIB_LOG
        lte::LteLog::DebugFormat(node, interfaceIndex,
            "SND_HARQFB",
            "%d,%d,%d",rnti->nodeId,processId,ite->second._ackNack[0]);
#endif
        // delete this queue
        queue->erase(ite);
    }
    else
    {
        // no feedback message
    }
}


// Get MCS index 29, 30 or 31 corresponding to the
// modulation order of specified MCS (0 - 28)
//
// \param mcs       : int : MCS
//
// \return MCS index 29, 30 or 31
//
int GetMcsFromMcsModulationOrder(int mcs)
{
    if (0 <= mcs && mcs <= 9)
    {
        // modulation order = 2
        return 29;
    }
    else if (10 <= mcs && mcs <= 16)
    {
        // modulation order = 4
        return 30;
    }
    else if (17 <= mcs && mcs <= 28)
    {
        // modulation order = 6
        return 31;
    }

    return mcs;
}

// get mcs from redendancy version
//
// \param rvidx  rvidx
// \param mcs  mcs
//
// \return  mcs
//
int GetMcsIndexFromRvidx(int rvidx, int mcs)
{
    if (rvidx == 1) {
        return 29;
    }
    else if (rvidx == 2) {
        return 30;
    }
    else if (rvidx == 3) {
        return 31;
    }
    return mcs;

}
