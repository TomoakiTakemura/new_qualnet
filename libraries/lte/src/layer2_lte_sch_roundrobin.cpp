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

#include "layer2_lte_sch_roundrobin.h"
#include "lte_harq.h"
#ifdef LTE_LIB_LOG
#include "log_lte.h"
#endif

#define EFFECTIVE_SINR_BY_AVERAGE 1
#define LTE_BER_BASED 1

// Initialize Round Robin scheduler
//
// \param node  Node Pointer
// \param interfaceIndex  Interface index
// \param nodeInput  Pointer to node input
//
LteSchedulerENBRoundRobin::LteSchedulerENBRoundRobin(
    Node* node, int interfaceIndex, const NodeInput* nodeInput)
    : LteSchedulerENB(node, interfaceIndex, nodeInput)
{
    _dlNextAllocatedUe = _dlConnectedUeList.end();
    _ulNextAllocatedUe = _ulConnectedUeList.end();
}

// Finalize Round Robin scheduler
//
//
LteSchedulerENBRoundRobin::~LteSchedulerENBRoundRobin()
{
}

// Notify Round Robin scheduler of power on.
//
//
void LteSchedulerENBRoundRobin::notifyPowerOn()
{
    // Do nothing
}

// Notify Round Robin scheduler of power off.
//
//
void LteSchedulerENBRoundRobin::notifyPowerOff()
{
    // Do nothing
}

// Notify Round Robin scheduler of UE attach
//
// \param rnti  RNTI of newly attached UE
//
void LteSchedulerENBRoundRobin::notifyAttach(const LteRnti& rnti)
{
    // Do nothing
}

// Notify Round Robin scheduler of UE detach
//
// \param rnti  RNTI of detached UE
//
void LteSchedulerENBRoundRobin::notifyDetach(const LteRnti& rnti)
{
    // Do nothing
}

// Prepare for scheduling.
//
// \param ttiNumber  TTI number
//
void LteSchedulerENBRoundRobin::prepareForScheduleTti(UInt64 ttiNumber)
{
    LteScheduler::prepareForScheduleTti(ttiNumber);

#ifdef LTE_LIB_LOG
    debugOutputInterfaceCheckingLog();
#endif

    ListLteRnti tempList;
    Layer3LteGetSchedulableListSortedByConnectedTime(_node, _interfaceIndex,
        &tempList);
    const ListLteRnti* listLteRnti = &tempList;

    ListLteRnti::const_iterator start_dl =
                                    getNextAllocatedUe(true,  listLteRnti);
    ListLteRnti::const_iterator start_ul =
                                    getNextAllocatedUe(false, listLteRnti);

    _dlConnectedUeList.clear();
    _ulConnectedUeList.clear();

    _dlConnectedUeList.insert(
                _dlConnectedUeList.begin(), listLteRnti->begin(), start_dl);
    _dlConnectedUeList.insert(
                _dlConnectedUeList.begin(), start_dl, listLteRnti->end());


    _ulConnectedUeList.insert(
                _ulConnectedUeList.begin(), listLteRnti->begin(), start_ul);
    _ulConnectedUeList.insert(
                _ulConnectedUeList.begin(), start_ul, listLteRnti->end());

    _dlNextAllocatedUe = _dlConnectedUeList.begin();
    _ulNextAllocatedUe = _ulConnectedUeList.begin();


#ifdef LTE_LIB_LOG
    std::stringstream log;
    ListLteRnti::const_iterator c_it;
    char buf[MAX_STRING_LENGTH];

    // Downlink target UEs
    for (c_it = listLteRnti->begin(); c_it != listLteRnti->end(); ++c_it)
    {
        sprintf(buf,
                LTE_STRING_FORMAT_RNTI,
                c_it->nodeId,
                c_it->interfaceIndex);
        log << buf << ",";
    }
    lte::LteLog::DebugFormat(
        _node,
        _interfaceIndex,
        LTE_STRING_LAYER_TYPE_SCHEDULER,
        "%s,%s",
        LTE_STRING_CATEGORY_TYPE_CONNECTED_UE,
        log.str().c_str());

    // Connected UEs list for downlink
    log.str("");
    for (c_it = _dlConnectedUeList.begin();
        c_it != _dlConnectedUeList.end();
        ++c_it)
    {
        sprintf(buf,
                LTE_STRING_FORMAT_RNTI,
                c_it->nodeId,
                c_it->interfaceIndex);
        log << buf << ",";
    }
    lte::LteLog::DebugFormat(
        _node,
        _interfaceIndex,
        LTE_STRING_LAYER_TYPE_SCHEDULER,
        "%s,%s",
        LTE_STRING_CATEGORY_TYPE_CONNECTED_UE_DL,
        log.str().c_str());

    // Connected UEs list for uplink
    log.str("");
    for (c_it = _ulConnectedUeList.begin();
        c_it != _ulConnectedUeList.end();
        ++c_it)
    {
        sprintf(buf,
                LTE_STRING_FORMAT_RNTI,
                c_it->nodeId,
                c_it->interfaceIndex);
        log << buf << ",";
    }
    lte::LteLog::DebugFormat(
        _node,
        _interfaceIndex,
        LTE_STRING_LAYER_TYPE_SCHEDULER,
        "%s,%s",
        LTE_STRING_CATEGORY_TYPE_CONNECTED_UE_UL,
        log.str().c_str());
#endif // LTE_LIB_LOG
}


// Determine target UEs for scheduling
//
// \param downlink  Downlink scheduling or uplink
// \param targetUes     : std:  Buffer for the target UEs stored
// \param reTxTargetUes : std:  allocated re-tx target UEs
//
void LteSchedulerENBRoundRobin::determineTargetUesExcludeReTx(
    bool downlink,
    std::vector < LteRnti >* targetUes,
    std::vector < LteRnti >* reTxTargetUes)
{
    // Get list of RNTI of UEs in RRC_CONNECTED status
    ListLteRnti& connectedUeList =
            downlink ? _dlConnectedUeList : _ulConnectedUeList;

    ListLteRnti::const_iterator it;
    std::vector < LteRnti >::iterator reIt;
    for (it = connectedUeList.begin();
        it != connectedUeList.end();
        ++it)
    {
        reIt = std::find(reTxTargetUes->begin(), reTxTargetUes->end(), *it);
        bool isTargetUe = downlink ? dlIsTargetUe(*it) : ulIsTargetUe(*it);
        if (reIt == reTxTargetUes->end() && isTargetUe)
        {
            targetUes->push_back(*it);
        }
    }
}

// Get next allocated UE
//
// \param downlink  Downlink or not
// \param listLteRnti  List of all of the current
//    connected UEs
//
// \return Selected MCS index.
// In case no MCS index satisfies targetBler, return -1.
ListLteRnti::const_iterator LteSchedulerENBRoundRobin::getNextAllocatedUe(
    bool downlink,
    const ListLteRnti* listLteRnti)
{
    ListLteRnti::const_iterator nextAllocatedUe;
    ListLteRnti& _connectedUeList =
                        downlink ? _dlConnectedUeList : _ulConnectedUeList;

    if (downlink)
    {
        nextAllocatedUe = _dlNextAllocatedUe;

    }else
    {
        nextAllocatedUe = _ulNextAllocatedUe;
    }

    // If no UE is set as candidate of next allocated UE
    // Adopt head UE in connected UEs list
    if (nextAllocatedUe == _connectedUeList.end())
    {
        return listLteRnti->begin();
    }

    ListLteRnti::const_iterator start =
        find(listLteRnti->begin(), listLteRnti->end(), *nextAllocatedUe);

    // If next UE is disconnected, determine next candidate UE.

    ListLteRnti::const_iterator firstNextAllocatedUe = nextAllocatedUe;

    while (start == listLteRnti->end())
    {
        ++nextAllocatedUe;

        if (nextAllocatedUe == _connectedUeList.end())
        {
            nextAllocatedUe = _connectedUeList.begin();
        }

        if (nextAllocatedUe == firstNextAllocatedUe)
        {
            break;
        }

        start =
            find(listLteRnti->begin(), listLteRnti->end(), *nextAllocatedUe);

    }

    // start == listLteRnti->end() if no candidate UE is determined
    return start;

}

// Execute scheduling for downlink transmission
//
// \param schedulingResult  vector<LteDlSchedulingResultInfo>& :
//    List of scheduling result.
//
void LteSchedulerENBRoundRobin::scheduleDlTti(
    std::vector < LteDlSchedulingResultInfo > &schedulingResult)
{
    // Initialize resutls
    schedulingResult.clear();


    UInt64 ttiNum = MacLteGetTtiNumber(_node, _interfaceIndex);
    int processId = (ttiNum) % DL_HARQ_NUM_HARQ_PROCESS;

    // HARQ scheduling re-transmission UEs
    // ----------------------
    std::vector < LteDlSchedulingResultInfo > reTxschedulingResult; 
    std::vector < LteRnti > reTxTargetUes;
    scheduleDlReTransmission(reTxschedulingResult, reTxTargetUes);

    // Get common informations (Could be class member variables)
    int numRb = PhyLteGetNumResourceBlocks(_node, _phyIndex);

    // Determine target UEs
    // -----------------------

    std::vector < LteRnti > newTxTargetUes;

    determineTargetUesExcludeReTx(true, &newTxTargetUes, &reTxTargetUes);

#ifdef LTE_LIB_LOG
    {
        std::stringstream log;
        std::vector < LteRnti > ::const_iterator c_it;
        char buf[MAX_STRING_LENGTH];

        // Downlink target UEs
        for (c_it = newTxTargetUes.begin(); c_it != newTxTargetUes.end(); ++c_it)
        {
            log << STR_RNTI(buf, *c_it) << ",";
        }

        lte::LteLog::DebugFormat(
            _node,
            _interfaceIndex,
            LTE_STRING_LAYER_TYPE_SCHEDULER,
            "%s,%s",
            LTE_STRING_CATEGORY_TYPE_TARGET_UE_DL,
            log.str().c_str());
    }
#endif

    int newTxNumTargetUes = newTxTargetUes.size();

    // Exit scheduling if there is no target user
    if (newTxNumTargetUes == 0 && reTxschedulingResult.size() == 0)
    {
        return;
    }
    else if (newTxNumTargetUes == 0 && reTxschedulingResult.size() > 0)
    {
        // Merge HARQ Re-Tx scheduling
        schedulingResult.insert(schedulingResult.end(),reTxschedulingResult.begin(),reTxschedulingResult.end());
        return;
    }


    int rbGroupSize = PhyLteGetRbGroupsSize(_node, _phyIndex, numRb);

    int numberOfRbGroup = (int)ceil((double)numRb / rbGroupSize);

    int lastRbGroupSize = rbGroupSize;

    if (numRb % rbGroupSize != 0)
    {
        lastRbGroupSize = numRb % rbGroupSize;
    }


    // listup re-tx RB Group Index
    std::vector<int> reTxRbGroupIndex;
    std::vector<int> newTxAvailableRbGroupIndex;
    checkReTxRbGroupIndex(reTxschedulingResult, &reTxRbGroupIndex);
    checkUsableRbGroupIndex(numberOfRbGroup,&reTxRbGroupIndex,&newTxAvailableRbGroupIndex);
    int newTxAvailableRbGroupSize = newTxAvailableRbGroupIndex.size();


    // Number of allocated UEs
    int newTxNumAllocatedUes = MIN(numberOfRbGroup - (int)reTxRbGroupIndex.size(), newTxNumTargetUes);


    // Allocate array in which to set scheduling results
    // !! MAC layer must delete (free) !!
    // !! this dynamically allocated schedulingResult !!
    schedulingResult.resize(newTxNumAllocatedUes);

    // Initialize scheduling results
    for (int ueIndex = 0; ueIndex < newTxNumAllocatedUes; ++ueIndex)
    {
        schedulingResult[ueIndex].rnti = newTxTargetUes[ueIndex];

        for (int rbIndex = 0; rbIndex < LTE_MAX_NUM_RB; ++rbIndex)
        {
            schedulingResult[ueIndex].allocatedRb[rbIndex] = 0;
        }

        schedulingResult[ueIndex].numResourceBlocks = 0;

        schedulingResult[ueIndex].mcsIndex[0] = PHY_LTE_INVALID_MCS;
        schedulingResult[ueIndex].mcsIndex[1] = PHY_LTE_INVALID_MCS;

        schedulingResult[ueIndex].harqProcessId = processId;
        
        schedulingResult[ueIndex].isNewData[0] = TRUE;
        schedulingResult[ueIndex].isNewData[1] = TRUE;
        
        schedulingResult[ueIndex].rvidx[0] = 0;
        schedulingResult[ueIndex].rvidx[1] = 0;
    }

    // RB allocation
    // -----------------
    int nextAllocatedUeIndex = 0;

    std::vector < int > shuffledRbgs(newTxAvailableRbGroupSize);
    for (int idx = 0;
        idx < newTxAvailableRbGroupSize;
        ++idx)
    {
        shuffledRbgs[idx] = newTxAvailableRbGroupIndex[idx];
    }

    randomShuffle(shuffledRbgs);

    for (int shuffledRbGroupIndex = 0;
        shuffledRbGroupIndex < newTxAvailableRbGroupSize;
        ++shuffledRbGroupIndex)
    {
        int rbGroupIndex = shuffledRbgs[shuffledRbGroupIndex];

        int startRbIndex = rbGroupIndex * rbGroupSize;

        int numRbsInThisRbGroup =
            (rbGroupIndex < numberOfRbGroup - 1) ?
                rbGroupSize : lastRbGroupSize;

        for (int lRbIndex = 0; lRbIndex < numRbsInThisRbGroup; ++lRbIndex)
        {
            int rbIndex = startRbIndex + lRbIndex;
            schedulingResult[nextAllocatedUeIndex].allocatedRb[rbIndex] = 1;
        }

        schedulingResult[nextAllocatedUeIndex].numResourceBlocks +=
                                                        numRbsInThisRbGroup;

        nextAllocatedUeIndex = (nextAllocatedUeIndex + 1) % newTxNumAllocatedUes;
    }

    // Update the candidate of next allocated Ue (for next TTI)
    if (newTxNumAllocatedUes < newTxNumTargetUes)
    {
        // If number of allocated UE is less than number of target UEs
        // (Which means, UEs which have packet to sent
        // were not be allocated any RBs)
        // then, such UEs has to be allocated at first in next scheduling.

        _dlNextAllocatedUe = find(
            _dlConnectedUeList.begin(),
            _dlConnectedUeList.end(),
            newTxTargetUes[newTxNumAllocatedUes]);
    }else
    {
        // If not, all UEs are checked equally whether
        // it has packet to sent or not,
        // So next allocated UE is same as the next allocated UE of
        // current allocation.
        _dlNextAllocatedUe = find(
            _dlConnectedUeList.begin(),
            _dlConnectedUeList.end(),
            newTxTargetUes[nextAllocatedUeIndex]);
    }


    // Determine transmission scheme
    //--------------------------------
    // Transmission scheme and tarnamission mode is difference concept.
    //

    for (int ueIndex = 0; ueIndex < newTxNumAllocatedUes; ++ueIndex) {

        // Retrieve CQI fedback from UE
        PhyLteCqiReportInfo phyLteCqiReportInfo;

        bool ret = PhyLteGetCqiInfoFedbackFromUe(
            _node,
            _phyIndex,
            newTxTargetUes[ueIndex],
            &phyLteCqiReportInfo);

        ERROR_Assert(ret == true, "CQIs not found.");

        int numLayer;
        int numTransportBlocksToSend;

        //results[ueIndex].txScheme = determineTransmissiomScheme(
        //    phyLteCqiReportInfo.riInfo,
        //    &numLayer, &numTransportBlocksToSend);

        schedulingResult[ueIndex].txScheme =
                                        PhyLteDetermineTransmissiomScheme(
                                                _node,
                                                _phyIndex,
                                                phyLteCqiReportInfo.riInfo,
                                                &numLayer,
                                                &numTransportBlocksToSend);

        schedulingResult[ueIndex].numTransportBlock =
                                        numTransportBlocksToSend;

        // For phase 3 LTE library

        for (int tbIndex = 0;
            tbIndex < schedulingResult[ueIndex].numTransportBlock;
            ++tbIndex)
        {
            int mcsIndex = dlSelectMcs(newTxTargetUes[ueIndex], schedulingResult[ueIndex].allocatedRb, tbIndex);

            schedulingResult[ueIndex].mcsIndex[tbIndex] = (UInt8)mcsIndex;
        }
    }

    // Create dequeue information
    //----------------------------

    createDequeueInformation(schedulingResult);

    // Finally, Purge invalid scheduling results
    //-------------------------------------------
    purgeInvalidSchedulingResults(schedulingResult);


    // Merge HARQ Re-Tx scheduling
    schedulingResult.insert(schedulingResult.end(),reTxschedulingResult.begin(),reTxschedulingResult.end());


    // End of DL scheduling

#ifdef LTE_LIB_LOG
    checkSchedulingResult(schedulingResult);
#endif
}

// Execute scheduling for uplink transmission
//
// \param schedulingResult  vector<LteUlSchedulingResultInfo>& :
//    List of scheduling result
//
void LteSchedulerENBRoundRobin::scheduleUlTti(
    std::vector < LteUlSchedulingResultInfo > &schedulingResult)
{
    // Initialize resutls
    schedulingResult.clear();

    // HARQ scheduling re-transmission UEs
    // ----------------------
    std::vector < LteUlSchedulingResultInfo > reTxschedulingResult; 
    std::vector < LteRnti > reTxTargetUes;
    int reTxAllocRbSize = 0;
    scheduleUlReTransmission(reTxschedulingResult, reTxTargetUes, &reTxAllocRbSize);

    // Get common informations (Could be class member variables)
    int numRb = PhyLteGetNumResourceBlocks(_node, _phyIndex);

    int pucchOverhead =
        PhyLteGetUlCtrlChannelOverhead(_node, _phyIndex);

    // Number of available RBs
   int newTxNumAvailableRb = numRb - pucchOverhead - reTxAllocRbSize;

   if (newTxNumAvailableRb <= 0 && reTxAllocRbSize <= 0)
   {
       return;
   }

    // If pucchOverhead = 5, 3 RBs at the bottom of bandwidth and
    // 2 RBs at the top of all bandwidth are used for PUCCH.
    int puschRbOffset = (int)ceil((double)pucchOverhead / 2.0);

    // Determine target UEs
    // -----------------------

    std::vector < LteRnti > newTxTargetUes;
    determineTargetUesExcludeReTx(false, &newTxTargetUes, &reTxTargetUes);

    if (newTxNumAvailableRb <= 0)
    {
        newTxTargetUes.clear();
    }

#ifdef LTE_LIB_LOG
    {
        std::stringstream log;
        std::vector < LteRnti > ::const_iterator c_it;
        char buf[MAX_STRING_LENGTH];

        // Downlink target UEs
        for (c_it = newTxTargetUes.begin(); c_it != newTxTargetUes.end(); ++c_it)
        {
            log << STR_RNTI(buf, *c_it) << ",";
        }
        lte::LteLog::DebugFormat(
            _node,
            _interfaceIndex,
            LTE_STRING_LAYER_TYPE_SCHEDULER,
            "%s,%s",
            LTE_STRING_CATEGORY_TYPE_TARGET_UE_UL,
            log.str().c_str());
    }
#endif

    int newTxNumTargetUes = newTxTargetUes.size();

    // Exit scheduling if there is no target user
    if (newTxNumTargetUes == 0 && reTxTargetUes.size() == 0)
    {
        return;
    }

    // ----
    // Resource allocation for new data transmission
    // ----
    int newTxNumAllocatedUes = 0;
    if (newTxNumTargetUes > 0)
    {
        int rbGroupSizeL = (int)ceil((double)newTxNumAvailableRb / newTxNumTargetUes);
        int rbGroupSizeS = rbGroupSizeL - 1;

        int numberOfRbGroupS = newTxNumTargetUes * rbGroupSizeL - newTxNumAvailableRb;
        int numberOfRbGroupL = newTxNumTargetUes - numberOfRbGroupS;

        // Number of allocated UEs
        newTxNumAllocatedUes = numberOfRbGroupL +
            (rbGroupSizeS > 0 ? numberOfRbGroupS : 0);

        int numberOfRbGroup = newTxNumAllocatedUes;

        // Update the candidate of next allocated Ue (for next TTI)
        if (newTxNumAllocatedUes < newTxNumTargetUes)
        {
            // If number of allocated UE is less than number of target UEs
            // (Which means, An UE which have packet to sent
            // was not be allocated any RBs)
            // then, such UE has to be allocated at first in next scheduling.

            _ulNextAllocatedUe = find(
                _ulConnectedUeList.begin(),
                _ulConnectedUeList.end(),
                newTxTargetUes[newTxNumAllocatedUes]);
        }else
        {
            // If not, all UEs are checked equally whether
            // it has packet to sent or not,
            // So next allocated UE is same as that of current TTI

            _ulNextAllocatedUe = _ulConnectedUeList.begin();
        }


        // Allocate array in which to set scheduling results
        schedulingResult.resize(newTxNumAllocatedUes);

        // RB allocation
        // -----------------

        std::vector<int> rbgSizes(numberOfRbGroup);
        for (int rbGroupIndex = 0;
            rbGroupIndex < numberOfRbGroup;
            ++rbGroupIndex)
        {
            rbgSizes[rbGroupIndex] =
                    ( rbGroupIndex < numberOfRbGroupL ? rbGroupSizeL : rbGroupSizeS );
        }

        // RBG sizes are shuffled to avoid unfair allocation
        randomShuffle(rbgSizes);

        int ueIndex = 0;
        for (int rbGroupIndex = 0;
            rbGroupIndex < numberOfRbGroup;
            ++rbGroupIndex)
        {
            schedulingResult[ueIndex].rnti = newTxTargetUes[ ueIndex ];

            // Allocate resource blocks
            schedulingResult[ueIndex].startResourceBlock = 0; // default
            schedulingResult[ueIndex].di.isNewData = TRUE;
            schedulingResult[ueIndex].numResourceBlocks = rbgSizes[rbGroupIndex];
            ++ueIndex;
        }

    }

    // merge new scheduling and re-tx scheduling
    schedulingResult.insert(schedulingResult.end(),reTxschedulingResult.begin(),reTxschedulingResult.end());
    
    // shuffle new and re-tx RB Group
    int allTtargetUes = reTxTargetUes.size() + newTxNumAllocatedUes;
    std::vector <int> shuffledRbgs(allTtargetUes);
    for (int rbGroupIndex = 0;
        rbGroupIndex < allTtargetUes;
        ++rbGroupIndex)
    {
        shuffledRbgs[rbGroupIndex]  = rbGroupIndex;
    }

    randomShuffle(shuffledRbgs);
    
    int startRBNumber = 0;
    for (int shuffledRbGroupIndex = 0;
        shuffledRbGroupIndex < allTtargetUes;
        ++shuffledRbGroupIndex)
    {
        int rbGroupIndex = shuffledRbgs[shuffledRbGroupIndex];
        schedulingResult[rbGroupIndex].startResourceBlock = startRBNumber;
        startRBNumber += schedulingResult[rbGroupIndex].numResourceBlocks;      
    }

    // Add offset for PUSCH
    for (int ueIndex = 0; ueIndex < allTtargetUes; ++ueIndex)
    {
        schedulingResult[ueIndex].startResourceBlock += (UInt8)puschRbOffset;
    }

    // Determine MCS index
    //------------------------

    for (size_t ueIndex = 0; ueIndex < schedulingResult.size(); ++ueIndex)
    {
        if (schedulingResult[ueIndex].di.isNewData == TRUE)
        {

#ifdef LTE_BER_BASED

            // Select the maximum mcs which estimated BLER is
            // greater than or equal to target BLER.
            int mcsIndex = ulSelectMcs(schedulingResult[ueIndex].rnti,
                    schedulingResult[ueIndex].startResourceBlock,
                    schedulingResult[ueIndex].numResourceBlocks);


            schedulingResult[ueIndex].mcsIndex = (UInt8)mcsIndex;

#ifdef LTE_LIB_LOG
            PhyLteGetAggregator(_node, _phyIndex)->regist(\
                                            schedulingResult[ueIndex].rnti,
                                            lte::Aggregator::UL_MCS,
                                            mcsIndex);
#endif

#endif
        }
    }

    // Finally, Purge invalid scheduling results
    //-------------------------------------------
    purgeInvalidSchedulingResults(schedulingResult);

    //End UL scheduling

#ifdef LTE_LIB_LOG
    checkSchedulingResult(schedulingResult);
#endif
}

// Determine re-transmission target UEs for scheduling
//
// \param schedulingResult : std::vector<LteDlSchedulingResultInfo>& :
//    re-tx scheduling list
// \param reTxTargetUes    : std::vector<LteRnti>& :
//    re-tx target UEs stored
void LteSchedulerENBRoundRobin::scheduleDlReTransmission(
        std::vector < LteDlSchedulingResultInfo > &schedulingResult,
        std::vector < LteRnti > &reTxTargetUes)
{   
    UInt64 ttiNum = MacLteGetTtiNumber(_node, _interfaceIndex);
    int processId = (ttiNum) % DL_HARQ_NUM_HARQ_PROCESS;

    LteMacData* macData = LteLayer2GetLteMacData(_node, _interfaceIndex);
    LteMacConfig* macConfig = GetLteMacConfig(_node, _interfaceIndex);
    
    // get connected UE list
    std::vector < LteRnti > shuffleUes;
    shuffleUes.insert(shuffleUes.begin(), _dlConnectedUeList.begin(), _dlConnectedUeList.end());

    size_t numTargetUes = shuffleUes.size();

    // Exit scheduling if there is no target user
    if (numTargetUes == 0)
    {
        return;
    }

    // shuffle UE list
    randomShuffle(shuffleUes);

    // retransmission UE list
    std::vector< LteRnti > reTransmissionTargetUes;

    // Get common informations (Could be class member variables)
    int numRb = PhyLteGetNumResourceBlocks(_node, _phyIndex);

    int rbGroupSize = PhyLteGetRbGroupsSize(_node, _phyIndex, numRb);

    int numberOfRbGroup = (int)ceil((double)numRb / rbGroupSize);

    std::map<LteRnti, MacLteDlHarqTxEntity*>::const_iterator harqIte;
    std::vector<LteRnti>::iterator ite;
    for (ite = shuffleUes.begin(); ite != shuffleUes.end(); ite++)
    {
        if ((int)schedulingResult.size() > numberOfRbGroup)
        {
            break;
        }


        harqIte = macData->dlHarqTxEntity->find(*ite);
        ERROR_Assert(harqIte != macData->dlHarqTxEntity->end(),"there is no dlHarqTxEntity");

        const MacLteDlHarqTxProcess* txProcess = harqIte->second->_harqProcess[processId];

        int txCount = txProcess->_schAllocHistory->size(); 

        if (((txProcess->_tbStatus[0]->_isValid == TRUE &&
            txProcess->_tbStatus[0]->_harqFeedback == NACKED ) ||
                (txProcess->_tbStatus[1]->_isValid == TRUE &&
                txProcess->_tbStatus[1]->_harqFeedback == NACKED ))
            && txCount >= 1
            && txCount < macConfig->maxHarqTx)
        {
            // re-transmission
            LteDlSchedulingResultInfo result;
            result.rnti = txProcess->_schAllocHistory->at(0).rnti;
            result.txScheme = txProcess->_schAllocHistory->at(0).txScheme;
            memcpy(result.allocatedRb, txProcess->_schAllocHistory->at(0).allocatedRb,
                sizeof(UInt8) * LTE_MAX_NUM_RB);
            result.numTransportBlock = txProcess->_schAllocHistory->at(0).numTransportBlock;

            for (int i = 0; i < txProcess->_schAllocHistory->at(0).numTransportBlock; i++)
            {
                result.mcsIndex[i] = GetMcsFromMcsModulationOrder(txProcess->_schAllocHistory->at(0).mcsIndex[i]);
            }

            result.dequeueInfo[0].bearerId = 0;        // no use
            result.dequeueInfo[0].dequeueSizeByte = 0; // no use
            result.dequeueInfo[1].bearerId = 0;        // no use
            result.dequeueInfo[1].dequeueSizeByte = 0; // no use

            result.numResourceBlocks = txProcess->_schAllocHistory->at(0).numResourceBlocks;
            result.harqProcessId = processId;
            result.isNewData[0] = FALSE;
            result.isNewData[1] = FALSE;

            result.rvidx[0] = HARQ_RedundancyVersionArray[txCount % 4];
            result.rvidx[1] = HARQ_RedundancyVersionArray[txCount % 4];

            schedulingResult.push_back(result);
            reTxTargetUes.push_back(*ite);

        }
    }

}


// list up re-tx using RBGroup Index from allocated RB
//
// \param schedulingResult : std::vector<LteDlSchedulingResultInfo>& :
//    List of scheduling result.
// \param rbgIndex : std::vector<int>* :
//    allocated RBGroup index list
void LteSchedulerENBRoundRobin::checkReTxRbGroupIndex(
    std::vector < LteDlSchedulingResultInfo > &schedulingResult,
    std::vector<int> *rbgIndex)
{

    int numRb = PhyLteGetNumResourceBlocks(_node, _phyIndex);
    int rbGroupSize = PhyLteGetRbGroupsSize(_node, _phyIndex, numRb);

    rbgIndex->clear();

    std::vector < LteDlSchedulingResultInfo >::iterator ite;
    for (ite = schedulingResult.begin(); ite != schedulingResult.end(); ite++)
    {
        for (int i = 0; i < LTE_MAX_NUM_RB; i+=rbGroupSize)
        {
            if (ite->allocatedRb[i] == 1)
            {
                rbgIndex->push_back(i / rbGroupSize);               
            }
        }
    }

    return;
}

// list up usable RBGroup Index
//
// \param numberOfRbGroup int RBGroup size
// \param rbgIndex : std::vector<int>* : allocated RBGroup index list
// \param usableRbgIndex : std::vector<int>* : usable RBGroup index list
void LteSchedulerENBRoundRobin::checkUsableRbGroupIndex(
    int numberOfRbGroup,
    std::vector<int> *rbgIndex,
    std::vector<int> *usableRbgIndex)
{
    usableRbgIndex->clear();

    std::vector<int>::iterator ite;
    for (int i = 0; i < numberOfRbGroup; i++)
    {
        ite = std::find(rbgIndex->begin(), rbgIndex->end(),i);
        if (ite == rbgIndex->end())
        {
            usableRbgIndex->push_back(i);
        }
    }

    return;
} 


// Determine re-transmission target UEs for scheduling
//
// \param schedulingResult : std::vector < LteUlSchedulingResultInfo >& :
//    ul scheduling
// \param reTxTargetUes    : std::vector<LteRnti>& :
//    re-tx target UEs list
// \param allocRbgIndex    : std::vector<int>* :
//    allocate RB Group index list
// param allocRbSize int* allocate RB size
void LteSchedulerENBRoundRobin::scheduleUlReTransmission(
        std::vector < LteUlSchedulingResultInfo > &schedulingResult,
        std::vector < LteRnti > &reTxTargetUes,
        int *allocRbSize)
{   
    UInt64 ttiNum = MacLteGetTtiNumber(_node, _interfaceIndex);
    int processId = (ttiNum) % UL_HARQ_NUM_HARQ_PROCESS;

    LteMacData* macData = LteLayer2GetLteMacData(_node, _interfaceIndex);
    LteMacConfig* macConfig = GetLteMacConfig(_node, _interfaceIndex);
    
    // get connected UE list
    std::vector < LteRnti > shuffleUes;
    shuffleUes.insert(shuffleUes.begin(), _ulConnectedUeList.begin(), _ulConnectedUeList.end());

    size_t numTargetUes = shuffleUes.size();

    // Exit scheduling if there is no target user
    if (numTargetUes == 0)
    {
        return;
    }

    // shuffle UE list
    randomShuffle(shuffleUes);

    // retransmission UE list
    std::vector< LteRnti > reTransmissionTargetUes;

    // Get common informations (Could be class member variables)
    int numRb = PhyLteGetNumResourceBlocks(_node, _phyIndex);

    int pucchOverhead =
        PhyLteGetUlCtrlChannelOverhead(_node, _phyIndex);

    int numAvailableRb = numRb - pucchOverhead;

    // Number of allocated Rb
    *allocRbSize = 0;

    std::map<LteRnti, MacLteUlHarqRxEntity*>::const_iterator harqIte;
    std::vector<LteRnti>::iterator ite;
    for (ite = shuffleUes.begin(); ite != shuffleUes.end(); ite++)
    {
        if (*allocRbSize >= numAvailableRb)
        {
            break;
        }


        harqIte = macData->ulHarqRxEntity->find(*ite);
        ERROR_Assert(harqIte != macData->ulHarqRxEntity->end(),"there is no ulHarqRxEntity");

        const MacLteUlHarqRxProcess* rxProcess = harqIte->second->_harqProcess[processId];

        int txCount = rxProcess->_schAllocHistory->size(); 

        if (rxProcess->_tbStatus[0]->_harqFeedback == NACKED
            && txCount >= 1
            && txCount < macConfig->maxHarqTx)
        {
            // re-transmission
            LteUlSchedulingResultInfo result;
            result.rnti = rxProcess->_schAllocHistory->at(0).rnti;
            result.startResourceBlock = 0; // default
            result.numResourceBlocks =  rxProcess->_schAllocHistory->at(0).numResourceBlocks;
            
            int rvidx = HARQ_RedundancyVersionArray[txCount % 4];
            result.mcsIndex = GetMcsIndexFromRvidx(rvidx, rxProcess->_schAllocHistory->at(0).mcsIndex);

            result.dequeueInfo.bearerId = 0;        // no use
            result.dequeueInfo.dequeueSizeByte = 0; // no use
            
            result.di.isNewData = FALSE;

            schedulingResult.push_back(result);
            reTxTargetUes.push_back(*ite);

            *allocRbSize += result.numResourceBlocks;
        }
    }

}

