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

#ifndef _LTE_HARQ_H_
#define _LTE_HARQ_H_

#include "lte_common.h"

#define DL_HARQ_NUM_HARQ_PROCESS (8)
#define UL_HARQ_NUM_HARQ_PROCESS (8)
#define DL_NUM_TB_PER_HARQ_PROCESS (2)
#define UL_NUM_TB_PER_HARQ_PROCESS (1)

struct LteDlSchedulingResultInfo;
struct LteUlSchedulingResultInfo;

/// Array representing the order of redundancy
/// version transmitted for each HARQ transmission
static const int HARQ_RedundancyVersionArray[4] = {0,2,3,1};

/// PhyLteHarqFeedback
typedef enum {
    NACKED = 0,
    ACKED = 1
} PhyLteHarqFeedback;

/// PhyLteDlHarqFeedback
typedef struct
{
    int _processid;
    PhyLteHarqFeedback _ackNack[2];

} PhyLteDlHarqFeedback;

/// PhyLteUlHarqFeedback
typedef struct
{
    int _processid;
    PhyLteHarqFeedback _ackNack[1];

} PhyLteUlHarqFeedback;


/// HarqPhyLteReceiveBuffer
class HarqPhyLteReceiveBuffer
{
    Node* _node;
public:
    HarqPhyLteReceiveBuffer(Node* node);
    ~HarqPhyLteReceiveBuffer();

    void Clear();

    Message* _tb;
    std::vector<UInt8>* _mcsHistory;
    std::vector<int>* _rvidxHistory;
    std::vector< std::vector<double> >* _sinrHistory;
    std::vector< std::vector<int> >* _rbAllocHistory;

};

/// MacLteDlHarqTxProcessTbStatus
class MacLteDlHarqTxProcessTbStatus
{
    Node* _node;

public:

    MacLteDlHarqTxProcessTbStatus(Node* node);

    ~MacLteDlHarqTxProcessTbStatus();

    BOOL _isValid;
    Message* _tb;
    BOOL _NDI;
    PhyLteHarqFeedback _harqFeedback; // ACKED|NACKED
};


/// MacLteDlHarqTxProcess
class MacLteDlHarqTxProcess
{
public:
    MacLteDlHarqTxProcess(Node* node);
    ~MacLteDlHarqTxProcess();

    MacLteDlHarqTxProcessTbStatus* _tbStatus[DL_NUM_TB_PER_HARQ_PROCESS];
    std::vector<LteDlSchedulingResultInfo>* _schAllocHistory;
};


/// HARQ DL Tx entity
class MacLteDlHarqTxEntity
{
    Node* _node;
    int _interfaceIndex;
    LteRnti _oppositeRnti;

public:
    MacLteDlHarqTxProcess* _harqProcess[DL_HARQ_NUM_HARQ_PROCESS];

    MacLteDlHarqTxEntity(Node* node, int interfaceIndex, const LteRnti& oppositeRnti);
    ~MacLteDlHarqTxEntity();
    void OnReceveHarqFeedback(Node* node,
                            int phyIndex,
                            Message* rxMsg,
                            PhyLteDlHarqFeedback* feedback,
                            LteRnti* txRnti);
    void OnStartSignalTransmission(
                            LteDlSchedulingResultInfo* dlSchedulingResult,
                            Message** listMsg,Message** curMsg );

    void TerminateRetransmission();

};



/// MacLteDlHarqRxProcessTbStatus
class MacLteDlHarqRxProcessTbStatus
{
    Node* _node;
public:
    MacLteDlHarqRxProcessTbStatus(Node* node);

    void Clear();

    ~MacLteDlHarqRxProcessTbStatus();

    BOOL _isValid;
    int _NDI; // -1,0,1
    HarqPhyLteReceiveBuffer* _receiveBuffer;
    PhyLteHarqFeedback _harqFeedback;
};


/// MacLteDlHarqRxProcess
class MacLteDlHarqRxProcess
{
public:
    MacLteDlHarqRxProcessTbStatus* _tbStatus[DL_NUM_TB_PER_HARQ_PROCESS];

    MacLteDlHarqRxProcess(Node* node);

    void Clear();

    ~MacLteDlHarqRxProcess();
};

/// HARQ DL Tx entity
class MacLteDlHarqRxEntity
{
    Node* _node;
    int _interfaceIndex;

public:
    MacLteDlHarqRxProcess* _harqProcess[DL_HARQ_NUM_HARQ_PROCESS];

    // feedback message queue
    std::map<UInt64, PhyLteDlHarqFeedback>* _dlFeedbackMessageQueue;


    MacLteDlHarqRxEntity(Node* node,
                         int interfaceIndex);

    ~MacLteDlHarqRxEntity();

    void Clear();

    void OnSignalArrival(Node* node,
                         int phyIndex,
                         Message* rxMsg,
                         LteRnti* txRnti);

    void OnSinrNotification(Node* node,
                            int phyIndex,
                            Message* rxMsg,
                            int numCodeWords,
                            bool tb2cwSwapFlag,
                            std::vector< std::vector<double> >* sinr,
                            LteRnti* txRnti);

    void OnSignalEnd(Node* node,
                     int phyIndex,
                     Message* rxMsg,
                     LteRnti* srcRnti);

    BOOL GetHarqFeedbackMessage(Node* node,
                                int phyIndex,
                                Message* msg,
                                const LteRnti * rnti);


private:
    void DlHarqAddFeedbackMessage(Node* node,
                                  int interfaceIndex,
                                  Message* msg,
                                  PhyLteDlHarqFeedback* feedbackMessage);

    void GetDlHarqRxProcessInfo(Node* node,
                                 int phyIndex,
                                 Message* rxMsg,
                                 int* processId,
                                 int* numTB);
};



/// /HARQ UL Tx process tb status
class MacLteUlHarqTxProcessTbStatus
{
    Node* _node;
public:
    MacLteUlHarqTxProcessTbStatus(Node* node);

    void Clear();

    ~MacLteUlHarqTxProcessTbStatus();

    Message* _tb;
    int _NDI;
    int _currentTbNb;
    int _currentirv; // redundancy version.{0,2,3,1} ndex
    PhyLteHarqFeedback _harqFeedback;
    Int8 _iniMcs;
    Int8 _lastValidMcs;
    std::pair<UInt8,UInt8> lastRbAlloc;

};

// HARQ UL Tx process tb status
class MacLteUlHarqTxProcess
{
public:
    MacLteUlHarqTxProcessTbStatus* _tbStatus[UL_NUM_TB_PER_HARQ_PROCESS];

    MacLteUlHarqTxProcess(Node* node);

    void Clear();

    ~MacLteUlHarqTxProcess();
};

/// HARQ UL Tx entity
class MacLteUlHarqTxEntity
{
    Node* _node;
    int _interfaceIndex;

public:
    MacLteUlHarqTxProcess* _harqProcess[UL_HARQ_NUM_HARQ_PROCESS];

    MacLteUlHarqTxEntity(Node* node, int interfaceIndex);
    ~MacLteUlHarqTxEntity();

    void Clear();
    void OnReceiveHarqFeedback(Node* node,
                             int phyIndex,
                             Message* rxMsg,
                             PhyLteUlHarqFeedback* feedback,
                             LteRnti* txRnti);

    BOOL OnSignalTransmission(Node* node,
                             int phyIndex,
                             std::vector < LteUlSchedulingResultInfo >* ulSchedulingResult,
                             Message** listMsg,Message** nextMsg,
                             UInt32* numUlResourceBlocks);

private:
    void GetMacPdu(Node* node,
                   int phyIndex,
                   int interfaceIndex,
                   LteRnti* oppositeRnti,
                   LteUlSchedulingResultInfo* schedulingInfo,
                   Message** macPdu);
};

/// HARQ UL Rx process tb status
class MacLteUlHarqRxProcessTbStatus
{
    Node* _node;
public:
    MacLteUlHarqRxProcessTbStatus(Node* node);
    ~MacLteUlHarqRxProcessTbStatus();

    HarqPhyLteReceiveBuffer* _receiveBuffer;
    PhyLteHarqFeedback _harqFeedback;
    BOOL _NDI;
};

/// HARQ UL Rx process
class MacLteUlHarqRxProcess
{
public:
    MacLteUlHarqRxProcessTbStatus* _tbStatus[1];
    std::vector<LteUlSchedulingResultInfo>* _schAllocHistory;

    MacLteUlHarqRxProcess(Node* node);

    ~MacLteUlHarqRxProcess();

};

/// HARQ UL Rx Entity
class MacLteUlHarqRxEntity
{
    Node* _node;
    int _interfaceIndex;
    LteRnti _oppositeRnti;

public:
    MacLteUlHarqRxProcess* _harqProcess[UL_HARQ_NUM_HARQ_PROCESS];

    // feedback message queue
    std::map<UInt64, PhyLteUlHarqFeedback>* _ulFeedbackMessageQueue;

    MacLteUlHarqRxEntity(Node* node, int interfaceIndex, const LteRnti& oppositeRnti);
    ~MacLteUlHarqRxEntity();

    void OnSignalArrival(Node* node,
                                  int phyIndex,
                                  Message* rxMsg,
                                  LteUlTbTxInfo* ulTbTxInfo,
                                  LteRnti* txRnti);

    void OnSinrNotification(Node* node,
                                  int phyIndex,
                                  Message* rxMsg,
                                  std::vector< std::vector<double> >* sinr,
                                  LteRnti* txRnti);

    void OnSignalEnd(Node* node,
                                  int phyIndex,
                                  Message* rxMsg,
                                  LteRnti* srcRnti);

    void GetFeedbackMessage(Node* node,
                            int phyIndex,
                            Message** msg,
                            const LteRnti* rnti );
    void OnBeforeStartSignalTransmission();
    void OnStartSignalTransmission(
                            LteUlSchedulingResultInfo* ulSchedulingResult,
                            Message* msg);

    void TerminateRetransmission();

private:
    void AddFeedbackMessage(Node* node,
                                  int phyIndex,
                                  Message** msg,
                                  PhyLteUlHarqFeedback* feedbackMessage,
                                  const LteRnti* rnti);

};


/// /get mcs from modulation order
///
/// \param mcs  mcs
///
/// \return  mcs
///
int GetMcsFromMcsModulationOrder(int mcs);

/// get mcs from redendancy version
///
/// \param mcs  mcs
///
/// \return  mcs
///
int GetMcsIndexFromRvidx(int rvidx, int mcs);


#endif // _LTE_HARQ_H_
