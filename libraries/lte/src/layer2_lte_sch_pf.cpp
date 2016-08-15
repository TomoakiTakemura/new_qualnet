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

#include "layer2_lte_sch_pf.h"
#ifdef LTE_LIB_LOG
#include "log_lte.h"
#endif

#define EFFECTIVE_SINR_BY_AVERAGE 1
#define LTE_BER_BASED 1

#define PfHogeFormat Debug2Format

// Initialize PF scheduler
//
// \param node  Node Pointer
// \param interfaceIndex  Interface index
// \param nodeInput  Pointer to node input
//
LteSchedulerENBPf::LteSchedulerENBPf(
    Node* node, int interfaceIndex, const NodeInput* nodeInput)
    : LteSchedulerENB(node, interfaceIndex, nodeInput)
{
    _filteringModule = NULL;

    initConfigurableParameters();
}

// Finalize PF scheduler
//
//
LteSchedulerENBPf::~LteSchedulerENBPf()
{
}

// Initialize configurable parameter
//
//
void LteSchedulerENBPf::initConfigurableParameters()
{
    // Call parent class's same function
    // LteSchedulerENB::initConfigurableParameters();

    // Load parameters for PF scheduler

    BOOL wasFound;
    int retInt;
    double retDouble;
    char errBuf[MAX_STRING_LENGTH] = {0};

    ////////////////////////////////////////////////////////////////
    // Read filter coefficient for average throughput
    ////////////////////////////////////////////////////////////////

    IO_ReadDouble(_node,
        _node->nodeId,
        _interfaceIndex,
        _nodeInput,
        "MAC-LTE-PF-FILTER-COEFFICIENT",
        &wasFound,
        &retDouble);

    if (wasFound == TRUE)
    {

        if (retDouble >= 0.0)
        {
            _pfFilterCoefficient = retDouble;
        }
        else
        {
            sprintf(errBuf,
                "Invalid PF filter coefficient %e. "
                "Should be greater than or equal to 0.0.", retDouble);

            ERROR_Assert(FALSE,errBuf);
        }
    }
    else
    {
        char warnBuf[MAX_STRING_LENGTH];
        sprintf(warnBuf,
            "Filter coefficient for PF scheduler should be set."
            "Default value %f is used.",
            (SCH_LTE_DEFAULT_PF_FILTER_COEFFICIENT));
        ERROR_ReportWarning(warnBuf);

        // default value
        _pfFilterCoefficient = SCH_LTE_DEFAULT_PF_FILTER_COEFFICIENT;
    }

    ////////////////////////////////////////////////////////////////
    // Read uplink RB allocation unit
    ////////////////////////////////////////////////////////////////

    IO_ReadInt(_node,
        _node->nodeId,
        _interfaceIndex,
        _nodeInput,
        "MAC-LTE-PF-UL-RB-ALLOCATION-UNIT",
        &wasFound,
        &retInt);

    if (wasFound == TRUE)
    {
        if (retInt >= 1 && retInt < PHY_LTE_MAX_NUM_RB)
        {
            _ulRbAllocationUnit = retInt;
        }
        else
        {
            sprintf(errBuf,
                "Invalid PF UL RB allocation unit %d [RB]. "
                "Should be in the range of [1, %d]",
                retInt, PHY_LTE_MAX_NUM_RB - 1);

            ERROR_Assert(FALSE,errBuf);
        }
    }
    else
    {
        char warnBuf[MAX_STRING_LENGTH];
        sprintf(warnBuf,
            "RB allocation unit for PF uplink scheduling should be set."
            "Default value %d is used.",
            (SCH_LTE_DEFAULT_PF_UL_RB_ALLOCATION_UNIT));
        ERROR_ReportWarning(warnBuf);

        _ulRbAllocationUnit = SCH_LTE_DEFAULT_PF_UL_RB_ALLOCATION_UNIT;
    }
}

// Notify PF scheduler of power on.
//
//
void LteSchedulerENBPf::notifyPowerOn()
{
    // Do nothing
}

// Notify PF scheduler of power off.
//
//
void LteSchedulerENBPf::notifyPowerOff()
{
    // Do nothing
}

// Notify PF scheduler of UE attach
//
// \param rnti  RNTI of newly attached UE
//
void LteSchedulerENBPf::notifyAttach(const LteRnti& rnti)
{
    // Do nothing
    _filteringModule =
        LteLayer2GetLayer3Filtering(_node, _interfaceIndex);

    double avgThroughput;
    BOOL found;

    // Set DL initial average throughput
    found = _filteringModule->get(
        rnti, LTE_LIB_PF_AVERAGE_THROUGHPUT_DL,&avgThroughput);

    ERROR_Assert(found == FALSE, "Filtering entry exists.");

    _filteringModule->update(
        rnti, LTE_LIB_PF_AVERAGE_THROUGHPUT_DL,
        SCH_LTE_PF_INITIAL_AVERAGE_THROUGHPUT,
        _pfFilterCoefficient);

    // Set UL initial average throughput

    found = _filteringModule->get(
        rnti, LTE_LIB_PF_AVERAGE_THROUGHPUT_UL,&avgThroughput);

    ERROR_Assert(found == FALSE, "Filtering entry exists.");

    _filteringModule->update(
        rnti, LTE_LIB_PF_AVERAGE_THROUGHPUT_UL,
        SCH_LTE_PF_INITIAL_AVERAGE_THROUGHPUT,
        _pfFilterCoefficient);
}

// Notify PF scheduler of UE detach
//
// \param rnti  RNTI of detached UE
//
void LteSchedulerENBPf::notifyDetach(const LteRnti& rnti)
{
    _filteringModule =
        LteLayer2GetLayer3Filtering(_node, _interfaceIndex);

    // Set DL initial average throughput
    _filteringModule->remove(
        rnti, LTE_LIB_PF_AVERAGE_THROUGHPUT_DL);

    // Set UL initial average throughput
    _filteringModule->remove(
        rnti, LTE_LIB_PF_AVERAGE_THROUGHPUT_UL);
}

// Prepare for scheduling.
//
// \param ttiNumber  TTI number
//
void LteSchedulerENBPf::prepareForScheduleTti(UInt64 ttiNumber)
{
    LteScheduler::prepareForScheduleTti(ttiNumber);

#ifdef LTE_LIB_LOG
    debugOutputInterfaceCheckingLog();
#endif

#ifdef LTE_LIB_LOG
    std::stringstream log;
    ListLteRnti::const_iterator c_it;
    char buf[MAX_STRING_LENGTH];

    ListLteRnti tempList;
    Layer3LteGetSchedulableListSortedByConnectedTime(_node, _interfaceIndex,
        &tempList);
    const ListLteRnti* listLteRnti = &tempList;

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

#endif // LTE_LIB_LOG

#ifdef LTE_LIB_LOG
    debugOutputRbgAverageThroughputDl();
#endif

#ifdef LTE_LIB_LOG
    debugOutputRbgAverageThroughputUl();
#endif

}

// Determine target UEs for scheduling
//
// \param downlink  Downlink scheduling or uplink
// \param targetUes     : std:  Buffer for the target UEs stored
//
void LteSchedulerENBPf::determineTargetUesExcludeReTx(
    bool downlink,
    std::vector < LteRnti >* targetUes,
    std::vector < LteRnti >* reTxTargetUes)
{
    // Get list of RNTI of UEs in RRC_CONNECTED status

    ListLteRnti tempList;
    Layer3LteGetSchedulableListSortedByConnectedTime(_node, _interfaceIndex,
        &tempList);
    const ListLteRnti* listLteRnti = &tempList;
    std::vector < LteRnti >::iterator reIt;

    ListLteRnti::const_iterator it;
    for (it = listLteRnti->begin(); it != listLteRnti->end(); ++it)
    {
        reIt = std::find(reTxTargetUes->begin(), reTxTargetUes->end(), *it);
        bool isTargetUe = downlink ? dlIsTargetUe(*it) : ulIsTargetUe(*it);
        if (reIt == reTxTargetUes->end() && isTargetUe)
        {
            targetUes->push_back(*it);
        }
    }
}

// Calculate average throughput
//
// \param rnti  RNTI of UE
// \param allocatedBits : int  vector<LteRnti>* :
//    Allocated bits for indicated UE in this TTI.
// \param isDl  Downlink or not.
//
// \return Average throughput [bps]
double LteSchedulerENBPf::getAverageThroughput(
    const LteRnti& rnti, int allocatedBits, bool isDl)
{
    LteMeasurementType measurementType
        = isDl ?
            LTE_LIB_PF_AVERAGE_THROUGHPUT_DL :
            LTE_LIB_PF_AVERAGE_THROUGHPUT_UL ;

    double averageThroughput;
    BOOL found =
        _filteringModule->get(
            rnti,
            measurementType,
            &averageThroughput);

    ERROR_Assert(found == TRUE, "Filtered throughput not found");

    double filteringAlpha = 1.0 / pow(2.0, _pfFilterCoefficient / 4.0);
    double instantThroughputAllocated =
            allocatedBits /
            (MacLteGetTtiLength(_node, _interfaceIndex) / (double)SECOND);

    averageThroughput =
            (1.0 - filteringAlpha) * averageThroughput
            + filteringAlpha * instantThroughputAllocated;

    ERROR_Assert(averageThroughput > 0.0, "Invalid average throughput");

    return averageThroughput;
}

// Execute scheduling for downlink transmission
//
// \param schedulingResult  vector<LteDlSchedulingResultInfo>& :
//    List of scheduling result.
//
void LteSchedulerENBPf::scheduleDlTti(
    std::vector < LteDlSchedulingResultInfo > &schedulingResult)
{
    // Initialize resutls
    schedulingResult.clear();

    // calc processid from tti
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
        updateAverageThroughputDl(NULL);
        return;
    }
    else if (newTxNumTargetUes == 0 && reTxschedulingResult.size() > 0)
    {
        // Merge HARQ Re-Tx scheduling
        schedulingResult.insert(schedulingResult.end(),reTxschedulingResult.begin(),reTxschedulingResult.end());
        updateAverageThroughputDl(NULL);
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


#ifdef LTE_LIB_LOG
    if (newTxNumTargetUes > 0 && reTxschedulingResult.size() > 0)
    {
        lte::LteLog::DebugFormat(
            _node,
            _interfaceIndex,
            "HARQ:DEBUG new and re-tx",
            "%d,%d",
            newTxNumTargetUes,reTxschedulingResult.size());

    }
#endif


    // Determine transmission scheme
    //--------------------------------
    // Transmission scheme and tarnamission mode is difference concept.
    //

    // Scheduling results for temporary use
    std::vector < LteDlSchedulingResultInfo > tmpResults(newTxNumTargetUes);

   for (int ueIndex = 0; ueIndex < newTxNumTargetUes; ++ueIndex)
    {
        tmpResults[ueIndex].rnti = newTxTargetUes[ueIndex];

        for (int rbIndex = 0; rbIndex < LTE_MAX_NUM_RB; ++rbIndex)
        {
            tmpResults[ueIndex].allocatedRb[rbIndex] = 0;
        }

        tmpResults[ueIndex].numResourceBlocks = 0;

        tmpResults[ueIndex].mcsIndex[0] = PHY_LTE_INVALID_MCS;
        tmpResults[ueIndex].mcsIndex[1] = PHY_LTE_INVALID_MCS;
        
        tmpResults[ueIndex].harqProcessId = processId;
        
        tmpResults[ueIndex].isNewData[0] = TRUE;
        tmpResults[ueIndex].isNewData[1] = TRUE;
        
        tmpResults[ueIndex].rvidx[0] = 0;
        tmpResults[ueIndex].rvidx[1] = 0;
    }

    for (int ueIndex = 0; ueIndex < newTxNumTargetUes; ++ueIndex){

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

        tmpResults[ueIndex].txScheme = PhyLteDetermineTransmissiomScheme(
            _node, _phyIndex,
            phyLteCqiReportInfo.riInfo,
            &numLayer, &numTransportBlocksToSend);

        tmpResults[ueIndex].numTransportBlock = numTransportBlocksToSend;
    }

    // Rbg allocation
    // ------------------------

    std::vector < std::vector < double > > pfMetric(newTxNumTargetUes);
    std::vector < std::vector < int > > allocatedBitsIf(newTxNumTargetUes);

    for (int ueIndex = 0; ueIndex < newTxNumTargetUes; ++ueIndex)
    {
        pfMetric[ueIndex].resize(numberOfRbGroup);
        allocatedBitsIf[ueIndex].resize(numberOfRbGroup);
    }

    // Container for allocated for each UEs
    std::vector < int > allocatedBits(newTxNumTargetUes, 0);

    // Sorted Pf metric values
    PfMetricContainer sortedPfMetric(newTxNumTargetUes * newTxAvailableRbGroupSize);

    for (int ueIndex = 0; ueIndex < newTxNumTargetUes; ++ueIndex)
    {
        for (int i = 0; i < newTxAvailableRbGroupSize; ++i)
        {
            // Create PfMetric entries
            pfMetricEntry obj;

            obj._metricValue = &pfMetric[ueIndex][newTxAvailableRbGroupIndex[i]];
            obj._ueIndex = ueIndex;
            obj._rbgIndex = newTxAvailableRbGroupIndex[i];
#if SCH_LTE_ENABLE_PF_RANDOM_SORT
            obj._rand = RANDOM_erand(randomSeed);
#endif
            sortedPfMetric[ueIndex * newTxAvailableRbGroupSize + i] = obj;
        }
    }

    calculatePfMetricsDl(
            -1, // Calculate for all of the UEs
            sortedPfMetric, allocatedBitsIf, tmpResults,
            allocatedBits, newTxTargetUes,
            numberOfRbGroup, rbGroupSize, lastRbGroupSize);

    // sort Pf Metric values
    std::sort(
        sortedPfMetric.begin(), sortedPfMetric.end(), pfMetricGreaterPtr());

    // For first release, only wideband CQI is supported.
    // RBG allocation becomes easier.
    std::vector < bool > allocatedRbgs(numberOfRbGroup, false);

#ifdef LTE_LIB_LOG
    int logAllocationCounter = 0;
#endif

    while (!sortedPfMetric.empty())
    {
        int allocatedUeIndex  = sortedPfMetric.front()._ueIndex;
        int allocatedRbgIndex = sortedPfMetric.front()._rbgIndex;

        int thisRbGroupSize = (allocatedRbgIndex == numberOfRbGroup - 1) ?
            lastRbGroupSize : rbGroupSize;


        if (pfMetric[allocatedUeIndex][allocatedRbgIndex] < 0.0)
        {
#ifdef LTE_LIB_LOG
            ERROR_ReportWarning("All the RBGs are not allocated.");
#endif
            break; // Stop allocating RBGs
        }

        int startRbIndex = allocatedRbgIndex * rbGroupSize;

        for (int lRbIndex = 0; lRbIndex < thisRbGroupSize; ++lRbIndex)
        {
            int rbIndex = startRbIndex + lRbIndex;
            tmpResults[allocatedUeIndex].allocatedRb[rbIndex] = 1;
        }

        tmpResults[allocatedUeIndex].numResourceBlocks += thisRbGroupSize;

        allocatedBits[allocatedUeIndex] =
                allocatedBitsIf[allocatedUeIndex][allocatedRbgIndex];

#ifdef LTE_LIB_LOG
        std::stringstream ss_rnti;
        std::stringstream ss_metric;
        std::stringstream ss_rbg;
        char buf[MAX_STRING_LENGTH];

        PfMetricContainer::const_iterator log_s_it;
        log_s_it = sortedPfMetric.begin();
        for (; log_s_it != sortedPfMetric.end(); ++log_s_it)
        {
            ss_rnti << STR_RNTI(buf, newTxTargetUes[log_s_it->_ueIndex]) << ",";
            ss_rbg << log_s_it->_rbgIndex << ",";
            ss_metric << *(log_s_it->_metricValue) << ",";
        }

        lte::LteLog::PfHogeFormat(
            _node,
            _interfaceIndex,
            LTE_STRING_LAYER_TYPE_SCHEDULER,
            "%s,cnt=,%d,rnti=,%s",
            "PfRbgAllocDl",
            logAllocationCounter,
            ss_rnti.str().c_str());

        lte::LteLog::PfHogeFormat(
            _node,
            _interfaceIndex,
            LTE_STRING_LAYER_TYPE_SCHEDULER,
            "%s,cnt=,%d,rbgIndex=,%s",
            "PfRbgAllocDl",
            logAllocationCounter,
            ss_rbg.str().c_str());

        lte::LteLog::PfHogeFormat(
            _node,
            _interfaceIndex,
            LTE_STRING_LAYER_TYPE_SCHEDULER,
            "%s,cnt=,%d,"
            "allocdPfMetric=,%e,allocdRbgIndex=,%d,allocdRnti=,%s,"
            "pfMetric=,%s",
            "PfRbgAllocDl",
            logAllocationCounter,
            *(sortedPfMetric.front()._metricValue),
            sortedPfMetric.front()._rbgIndex,
            STR_RNTI(buf, newTxTargetUes[allocatedUeIndex]),
            ss_metric.str().c_str());

        ++logAllocationCounter;
#endif

        // Purge elements in sortedPfMetric
        PfMetricContainer::iterator remaning_it =
            std::remove_if (
                sortedPfMetric.begin(), sortedPfMetric.end(),
                predRbgIndex(allocatedRbgIndex));

        sortedPfMetric.erase(remaning_it, sortedPfMetric.end());

        calculatePfMetricsDl(
            allocatedUeIndex, // Re-calculate PF metric for the selected UE
            sortedPfMetric,
            allocatedBitsIf,
            tmpResults,
            allocatedBits,
            newTxTargetUes,
            numberOfRbGroup,
            rbGroupSize,
            lastRbGroupSize);

        // Resort Pf Metric values
        std::sort(sortedPfMetric.begin(),
                  sortedPfMetric.end(),
                  pfMetricGreaterPtr());

    }


    // Number of UEs for which at least one RB is allocated in this TTI
    int numAllocatedUes = 0;

    for (int ueIndex = 0; ueIndex < newTxNumTargetUes; ++ueIndex){
        if (tmpResults[ueIndex].numResourceBlocks > 0)
        {
            ++numAllocatedUes;
        }
    }

    if (numAllocatedUes == 0)
    {
        // Merge HARQ Re-Tx scheduling
        schedulingResult.insert(schedulingResult.end(),reTxschedulingResult.begin(),reTxschedulingResult.end());
        updateAverageThroughputDl(NULL);
        return;
    }

    // Allocate array in which to set scheduling results
    schedulingResult.resize(numAllocatedUes);
    LteDlSchedulingResultInfo* results = &schedulingResult[0];

    int allocatedUeIndex = 0;
    for (int ueIndex = 0; ueIndex < newTxNumTargetUes; ++ueIndex){
        if (tmpResults[ueIndex].numResourceBlocks > 0)
        {
            // Copy LteDlSchedulingResultInfo of allocated users
            results[allocatedUeIndex] = tmpResults[ueIndex];
            ++allocatedUeIndex;
        }
    }

    // Determine MCS index
    //------------------------
    for (int allocatedUeIndex = 0;
        allocatedUeIndex < numAllocatedUes;
        ++allocatedUeIndex)
    {
        for (int tbIndex = 0;
            tbIndex < results[allocatedUeIndex].numTransportBlock;
            ++tbIndex)
        {
            int mcsIndex = dlSelectMcs(
                    results[allocatedUeIndex].rnti,
                    results[allocatedUeIndex].allocatedRb,
                    tbIndex);

            results[allocatedUeIndex].mcsIndex[tbIndex] = (UInt8)mcsIndex;
        }
    }

    // Create dequeue information
    //----------------------------

    createDequeueInformation(schedulingResult);

    // Finally, Purge invalid scheduling results
    //-------------------------------------------
    purgeInvalidSchedulingResults(schedulingResult);

    // Update average throughput for all UEs in the connected UE list
    //----------------------------------------------------------------
    updateAverageThroughputDl(&schedulingResult);


    // Merge HARQ Re-Tx scheduling and new scheduling
    schedulingResult.insert(schedulingResult.end(),reTxschedulingResult.begin(),reTxschedulingResult.end());


#ifdef LTE_LIB_LOG
    checkSchedulingResult(schedulingResult);
#endif
    // End of DL scheduling
}

// Execute scheduling for uplink transmission
//
// \param schedulingResult  vector<LteDlSchedulingResultInfo>& :
//    List of scheduling result.
//
void LteSchedulerENBPf::scheduleUlTti(
    std::vector < LteUlSchedulingResultInfo > &schedulingResult)
{
    // Initialize resutls
    schedulingResult.clear();


    // HARQ scheduling re-transmission UEs
    // ----------------------
    std::vector < LteUlSchedulingResultInfo > reTxschedulingResult; 
    std::vector < LteRnti > reTxTargetUes;
    std::vector<int> reTxRbGroupIndex;
    scheduleUlReTransmission(reTxschedulingResult, reTxTargetUes, &reTxRbGroupIndex);
    int reTxAllocRbSize = reTxRbGroupIndex.size();

    // Get common informations (Could be class member variables)
    int numRb = PhyLteGetNumResourceBlocks(_node, _phyIndex);

    int pucchOverhead =
        PhyLteGetUlCtrlChannelOverhead(_node, _phyIndex);

    // Number of available RBs
   //int numAvailableRb = numRb - pucchOverhead;
   int numAvailableRb = numRb - pucchOverhead - reTxAllocRbSize;

   if (numAvailableRb <= 0 && reTxAllocRbSize <= 0)
   {
       updateAverageThroughputUl(NULL);
       return;
   }
   else if (numAvailableRb <= 0 && reTxAllocRbSize > 0)
   {
       // Merge HARQ Re-Tx scheduling
       schedulingResult.insert(schedulingResult.end(),reTxschedulingResult.begin(),reTxschedulingResult.end());
       updateAverageThroughputUl(NULL);
       return;
   }

   // Determine target UEs
    // -----------------------

    std::vector < LteRnti > nweTxTargetUes;
    determineTargetUesExcludeReTx(false, &nweTxTargetUes, &reTxTargetUes);

#ifdef LTE_LIB_LOG
    {
        std::stringstream log;
        std::vector < LteRnti > ::const_iterator c_it;
        char buf[MAX_STRING_LENGTH];

        // Downlink target UEs
        for (c_it = nweTxTargetUes.begin(); c_it != nweTxTargetUes.end(); ++c_it)
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

    int newTxNumTargetUes = nweTxTargetUes.size();

    // Exit scheduling if there is no target user
    if (newTxNumTargetUes == 0 && reTxAllocRbSize == 0)
    {
        updateAverageThroughputUl(NULL);
        return;
    }
   else if (newTxNumTargetUes == 0 && reTxTargetUes.size() > 0)
   {
       // Merge HARQ Re-Tx scheduling
       schedulingResult.insert(schedulingResult.end(),reTxschedulingResult.begin(),reTxschedulingResult.end());
       updateAverageThroughputUl(NULL);
#ifdef LTE_LIB_LOG
        // Output scheduling result logs
        debugOutputSchedulingResultLog(schedulingResult);
#endif
       return;
   }

    int rbGroupSize = _ulRbAllocationUnit;

    int numberOfRbGroup = (int)ceil((double)numRb / rbGroupSize);

    int lastRbGroupSize = rbGroupSize;

    if (numRb % rbGroupSize != 0)
    {
        lastRbGroupSize = numRb % rbGroupSize;
    }

    // listup re-tx RB Group Index
    std::vector<int> newTxAvailableRbGroupIndex;
    checkUsableRbGroupIndex(numberOfRbGroup,&reTxRbGroupIndex,&newTxAvailableRbGroupIndex);
    int newTxAvailableRbGroupSize = newTxAvailableRbGroupIndex.size();

#ifdef LTE_LIB_LOG
    if (newTxNumTargetUes > 0 && reTxschedulingResult.size() > 0)
    {
        lte::LteLog::DebugFormat(
            _node,
            _interfaceIndex,
            "HARQ:DEBUG UPLINK new and re-tx",
            "%d,%d",
            newTxNumTargetUes,reTxschedulingResult.size());

    }
#endif


    // Rbg allocation
    //---------------

    // Note that order of subscript of pfMetric is
    // contrary to that of DL pfMetric
    std::vector < std::vector < double > > pfMetric(newTxNumTargetUes);
    std::vector < std::vector < int > > allocatedBitsIf(newTxNumTargetUes);

    for (int ueIndex = 0; ueIndex < newTxNumTargetUes; ++ueIndex)
    {
        pfMetric[ueIndex].resize(numberOfRbGroup);
        allocatedBitsIf[ueIndex].resize(numberOfRbGroup);
    }

    // Sorted Pf metric values
    PfMetricContainer sortedPfMetric(newTxNumTargetUes * newTxAvailableRbGroupSize);
    PfMetricContainer::iterator s_it;

    // RBG allocation
    std::vector < AllocatedRbgRange > allocatedRbg(newTxNumTargetUes);
    std::vector < int > allocatedBits(newTxNumTargetUes, 0);

    std::vector < bool > isRbgAllocated(numberOfRbGroup, false);

    for (int ueIndex = 0; ueIndex < newTxNumTargetUes; ++ueIndex)
    {
        for (int rbgIndex = 0; rbgIndex < newTxAvailableRbGroupSize; ++rbgIndex)
        {
            // Create PfMetric entries
            pfMetricEntry obj;

            obj._metricValue = &pfMetric[ueIndex][newTxAvailableRbGroupIndex[rbgIndex]];
            obj._ueIndex = ueIndex;
            obj._rbgIndex = newTxAvailableRbGroupIndex[rbgIndex];
#if SCH_LTE_ENABLE_PF_RANDOM_SORT
            obj._rand = RANDOM_erand(randomSeed);
#endif
            sortedPfMetric[ueIndex * newTxAvailableRbGroupSize + rbgIndex] = obj;
        }
    }

    // Calculate 1st pf metrics
    calculatePfMetricsUl(
        -1, // Calculate PF metric for all of the UEs
        sortedPfMetric,
        allocatedBitsIf,
        allocatedRbg, allocatedBits,
        nweTxTargetUes,
        numberOfRbGroup, rbGroupSize, lastRbGroupSize);

    // sort Pf Metric values
    std::sort(
        sortedPfMetric.begin(), sortedPfMetric.end(), pfMetricGreaterPtr());

#ifdef LTE_LIB_LOG
    int logAllocationCounter = 0;
#endif

    s_it = sortedPfMetric.begin();
    for (; s_it != sortedPfMetric.end();)
    {
        if (*(s_it->_metricValue) < 0.0)
        {
#ifdef LTE_LIB_LOG
            ERROR_ReportWarning("All the RBGs are not allocated (UL).");
#endif
            break; // Stop allocating RBGs
        }

        if (allocatedRbg[s_it->_ueIndex].add(s_it->_rbgIndex) != 0)
        {
            // _rgbIndex can be allocated to _ueIndex
            int allocatedUeIndex = s_it->_ueIndex;
            int allocatedRbgIndex = s_it->_rbgIndex;

            // Allocate RBG g for UE k
            isRbgAllocated[allocatedRbgIndex] = true;

#ifdef LTE_LIB_LOG
            std::stringstream ss_rnti;
            std::stringstream ss_rbg;
            std::stringstream ss_metric;

            PfMetricContainer::const_iterator log_s_it;
            log_s_it = sortedPfMetric.begin();
            char buf[MAX_STRING_LENGTH];
            for (; log_s_it != sortedPfMetric.end(); ++log_s_it)
            {
                ss_rnti << STR_RNTI(buf, nweTxTargetUes[log_s_it->_ueIndex])
                        << ",";
                ss_rbg << log_s_it->_rbgIndex << ",";
                ss_metric << *(log_s_it->_metricValue) << ",";
            }

            lte::LteLog::PfHogeFormat(
                _node,
                _interfaceIndex,
                LTE_STRING_LAYER_TYPE_SCHEDULER,
                "%s,cnt=,%d,rnti=,%s",
                "PfRbgAllocUl",
                logAllocationCounter,
                ss_rnti.str().c_str());

            lte::LteLog::PfHogeFormat(
                _node,
                _interfaceIndex,
                LTE_STRING_LAYER_TYPE_SCHEDULER,
                "%s,cnt=,%d,rbgIndex=,%s",
                "PfRbgAllocUl",
                logAllocationCounter,
                ss_rbg.str().c_str());

            lte::LteLog::PfHogeFormat(
                _node,
                _interfaceIndex,
                LTE_STRING_LAYER_TYPE_SCHEDULER,
                "%s,cnt=,%d,"
                "allocdPfMetric=,%e,allocdRbgIndex=,%d,allocdRnti=,%s,"
                "pfMetric=,%s",
                "PfRbgAllocUl",
                logAllocationCounter,
                *(s_it->_metricValue),
                s_it->_rbgIndex,
                STR_RNTI(buf, nweTxTargetUes[s_it->_ueIndex]),
                ss_metric.str().c_str());

            ++logAllocationCounter;
#endif

            // Update allocated bits
            allocatedBits[allocatedUeIndex] =
                allocatedBitsIf[allocatedUeIndex][allocatedRbgIndex];

            // Purge elements in sortedPfMetric
            PfMetricContainer::iterator remaning_it =
                std::remove_if (
                    sortedPfMetric.begin(), sortedPfMetric.end(),
                    predRbgIndex(allocatedRbgIndex));

            sortedPfMetric.erase(remaning_it, sortedPfMetric.end());

            // Recalculate pf metrics
            calculatePfMetricsUl(
                allocatedUeIndex,
                sortedPfMetric,
                allocatedBitsIf,
                allocatedRbg, allocatedBits,
                nweTxTargetUes,
                numberOfRbGroup, rbGroupSize, lastRbGroupSize);

            // Sort pf Metric value
            std::sort(sortedPfMetric.begin(),
                      sortedPfMetric.end(),
                      pfMetricGreaterPtr());

            // Return to head
            s_it = sortedPfMetric.begin();
        }else
        {
            // _rbgIndex is not an adjacent rbg for UE _ueIndex
            ++s_it;
        }
    }

    // Count number of Ues allocated
    int numAllocatedUes = 0;
    for (int ueIndex = 0; ueIndex < newTxNumTargetUes; ++ueIndex)
    {
        if (allocatedRbg[ueIndex].getNumAllocatedRbgs() > 0)
        {
            ++numAllocatedUes;
        }
    }

    if (numAllocatedUes == 0)
    {
        // Merge HARQ Re-Tx scheduling
        schedulingResult.insert(schedulingResult.end(),reTxschedulingResult.begin(),reTxschedulingResult.end());
        updateAverageThroughputUl(NULL);
#ifdef LTE_LIB_LOG
        // Output scheduling result logs
        debugOutputSchedulingResultLog(schedulingResult);
#endif
        return;
    }

    // Set resource block allocation info to scheduling results
    schedulingResult.resize(numAllocatedUes);

    int allocatedUeIndex = 0;
    for (int ueIndex = 0; ueIndex < newTxNumTargetUes; ++ueIndex)
    {
        if (allocatedRbg[ueIndex].getNumAllocatedRbgs() > 0)
        {
            schedulingResult[allocatedUeIndex].rnti = nweTxTargetUes[ueIndex];
            schedulingResult[allocatedUeIndex].startResourceBlock =
                (UInt8)(allocatedRbg[ueIndex].lower * rbGroupSize);
            
            schedulingResult[allocatedUeIndex].di.isNewData = TRUE;


            // Count number of resource blocks to allocate
            schedulingResult[allocatedUeIndex].numResourceBlocks =
                    getNumAllocatedRbs(
                            allocatedRbg[ueIndex],
                            numberOfRbGroup,
                            rbGroupSize,
                            lastRbGroupSize);

            ++allocatedUeIndex;
        }
    }

    // Determine MCS index
    //------------------------

    for (int allocatedUeIndex = 0;
        allocatedUeIndex < numAllocatedUes;
        ++allocatedUeIndex)
    {

        int mcsIndex = ulSelectMcs(
                schedulingResult[allocatedUeIndex].rnti,
                schedulingResult[allocatedUeIndex].startResourceBlock,
                schedulingResult[allocatedUeIndex].numResourceBlocks);

        schedulingResult[allocatedUeIndex].mcsIndex = (UInt8)mcsIndex;
    }

#ifdef LTE_LIB_LOG
    // Output scheduling result logs
    debugOutputSchedulingResultLog(schedulingResult);
#endif

    // Finally, Purge invalid scheduling results
    //-------------------------------------------
    purgeInvalidSchedulingResults(schedulingResult);

    // Update average throughput for all UEs in the connected UE list
    //----------------------------------------------------------------
    updateAverageThroughputUl(&schedulingResult);


    // Merge HARQ Re-Tx scheduling and new scheduling
    schedulingResult.insert(schedulingResult.end(),reTxschedulingResult.begin(),reTxschedulingResult.end());


    // End of UL scheduling

#ifdef LTE_LIB_LOG
    checkSchedulingResult(schedulingResult);
#endif
}

// Calculate PF metric value for UL scheduling
//
// \param targetUeIndex  Target UE index
// \param sortedPfMetric  List of sorted PF metric values
// \param allocatdBitsIf  : std:  vector<int> >& :
//    Allocated bits for each UEs assuming remaining
//    RBGs are allocated to them.
// \param allocatedRbg  vector<AllocatedRbgRange>& :
//    List of RBG allocation information for each UE
// \param allocatedBits  vector<int>& :
//    Allocated bits for each UEs
// \param targetUes  vector<LteRnti>& :
//    List of RNTIs of target UEs
// \param numberOfRbGroup  Number of RBG
// \param rbGroupSize  Size of RBG
// \param lastRbGroupSize  Size of last RBG.
//
void LteSchedulerENBPf::calculatePfMetricsUl(
    int targetUeIndex,
    PfMetricContainer& sortedPfMetric,
    std::vector < std::vector < int > > &allocatedBitsIf,
    const std::vector < AllocatedRbgRange > &allocatedRbg,
    const std::vector < int > &allocatedBits,
    const std::vector < LteRnti > &targetUes,
    int numberOfRbGroup, int rbGroupSize, int lastRbGroupSize)
{
    PfMetricContainer::iterator it;
    it = sortedPfMetric.begin();
    for (; it != sortedPfMetric.end(); ++it)
    {
        int ueIndex = it->_ueIndex;

        // If targetUeIndex is specified, calculate PF metric only for the UE
        if (targetUeIndex >= 0 && targetUeIndex != ueIndex)
        {
            continue;
        }

        int rbgIndex = it->_rbgIndex;

        int thisRbGroupSize = (rbgIndex == numberOfRbGroup - 1) ?
            lastRbGroupSize : rbGroupSize;

#if SCH_LTE_ENABLE_SIMPLE_PF_METRIC
        int pfMetricNumRbs = thisRbGroupSize;
#else
        int numAllocatedRbs =
            getNumAllocatedRbs(
                    allocatedRbg[ueIndex],
                    numberOfRbGroup,
                    rbGroupSize,
                    lastRbGroupSize);

        int pfMetricNumRbs =
            numAllocatedRbs + thisRbGroupSize;
#endif

        int side = allocatedRbg[ueIndex].isAdjacent(rbgIndex);

        // If rgbIndex is not adjacent rbg of currently allocated rbgs,
        // Pf metric value is set to 0.0, expected number of allocated
        // bits is equal to the current one.
        if (side == 0)
        {
            *it->_metricValue = 0.0;
            allocatedBitsIf[ueIndex][rbgIndex] = allocatedBits[ueIndex];
            continue;
        }

        int startRbgIndex;

        if (side == -1)
        {
            startRbgIndex = rbgIndex;
        }else
        {
            ERROR_Assert(side == 1, "Invalid adjacent index");
            startRbgIndex = allocatedRbg[ueIndex].lower;
        }

        int mcsIndexIf = ulSelectMcs(targetUes[ueIndex], startRbgIndex, pfMetricNumRbs);

        int transportBlockSizeIf = PhyLteGetUlTxBlockSize(
            _node,
            _phyIndex,
            mcsIndexIf,
            pfMetricNumRbs);

        allocatedBitsIf[ueIndex][rbgIndex] = transportBlockSizeIf;

#if SCH_LTE_ENABLE_SIMPLE_PF_METRIC
        double instantThroughput =
            allocatedBitsIf[ueIndex][rbgIndex] /
            (MacLteGetTtiLength(_node, _interfaceIndex) / (double)SECOND);
#else
        double instantThroughput =
            (allocatedBitsIf[ueIndex][rbgIndex] - allocatedBits[ueIndex]) /
            (MacLteGetTtiLength(_node, _interfaceIndex) / (double)SECOND);
#endif

        double averageThroughput =
                getAverageThroughput(
                        targetUes[ueIndex],allocatedBits[ueIndex],false);

        *it->_metricValue = instantThroughput / averageThroughput;
    }

}

// Calculate PF metric value for DL scheduling
//
// \param targetUeIndex  Target UE index
// \param sortedPfMetric  List of sorted PF metric values
// \param allocatdBitsIf   : std:  vector<int> >& :
//    Allocated bits for each UEs assuming remaining
//    RBGs are allocated to them.
// \param tmpResults  vector<LteDlSchedulingResultInfo>& :
//    List of temporary resource allocation information
//    for each UE.
// \param estimatedSinr_dB : const std:  vector<double> >& :
//    List of estimated SINR for each TB and UE.
// \param allocatedBits  vector<int>& :
//    Allocated bits for each UEs
// \param targetUes  vector<LteRnti>& :
//    List of RNTIs of target UEs
// \param numberOfRbGroup  Number of RBG
// \param rbGroupSize  Size of RBG
// \param lastRbGroupSize  Size of last RBG.
//
void LteSchedulerENBPf::calculatePfMetricsDl(
    int targetUeIndex,
    PfMetricContainer& sortedPfMetric,
    std::vector < std::vector < int > > &allocatedBitsIf,
    std::vector < LteDlSchedulingResultInfo > &tmpResults,
    const std::vector < int > &allocatedBits,
    const std::vector < LteRnti > &targetUes,
    int numberOfRbGroup, int rbGroupSize, int lastRbGroupSize)
{
    PfMetricContainer::iterator it;
    it = sortedPfMetric.begin();
    for (; it != sortedPfMetric.end(); ++it)
    {
        int ueIndex = it->_ueIndex;

        // If targetUeIndex is specified, calculate PF metric only for the UE
        if (targetUeIndex >= 0 && targetUeIndex != ueIndex)
        {
            continue;
        }

        int rbgIndex = it->_rbgIndex;

        int thisRbGroupSize = (rbgIndex == numberOfRbGroup - 1) ?
            lastRbGroupSize : rbGroupSize;

        allocatedBitsIf[ueIndex][rbgIndex] = 0;


        // Add for phase 3
        UInt8 rbBitmapIf[LTE_MAX_NUM_RB];
        memcpy(&rbBitmapIf[0], &(tmpResults[ueIndex].allocatedRb[0]), sizeof(rbBitmapIf));
        int startRbIndex = rbgIndex * rbGroupSize;
        for (int lRbIndex = 0; lRbIndex < thisRbGroupSize; ++lRbIndex)
        {
            int rbIndex = startRbIndex + lRbIndex;
            rbBitmapIf[rbIndex] = 1;
        }
        // End - Add for phase 3

        for (int tbIndex = 0;
            tbIndex < tmpResults[ueIndex].numTransportBlock;
            ++tbIndex)
        {
            // Add SINR estimation code when
            // subband CQI feedback is supported.
            // estimatedSinr_dB[ueInex][tbIndex] = AverageSinrOfAllocatedRbs

#if SCH_LTE_ENABLE_SIMPLE_PF_METRIC
            int pfMetricNumRbs = thisRbGroupSize;
#else
            int pfMetricNumRbs =
                    tmpResults[ueIndex].numResourceBlocks + thisRbGroupSize;
#endif
            int mcsIndexIf = dlSelectMcs(targetUes[ueIndex], rbBitmapIf, tbIndex);

            int transportBlockSizeIf = PhyLteGetDlTxBlockSize(
                _node,
                _phyIndex,
                mcsIndexIf,
                pfMetricNumRbs);

            allocatedBitsIf[ueIndex][rbgIndex] += transportBlockSizeIf;
        }

#if SCH_LTE_ENABLE_SIMPLE_PF_METRIC
        double instantThroughput =
            allocatedBitsIf[ueIndex][rbgIndex] /
                (MacLteGetTtiLength(_node, _interfaceIndex) /
                (double)SECOND);
#else
        double instantThroughput =
            (allocatedBitsIf[ueIndex][rbgIndex] - allocatedBits[ueIndex]) /
                (MacLteGetTtiLength(_node, _interfaceIndex) /
                (double)SECOND);
#endif

        double averageThroughput = getAverageThroughput(
                targetUes[ueIndex],
                allocatedBits[ueIndex],
                true);

        *it->_metricValue = instantThroughput / averageThroughput;
    }
}

// Get number of allocated RBs
//
// \param arr  Instance of RBG range class.
// \param rbGroupSize  Size of RBG
// \param lastRbGroupSize  Size of last RBG.
//
int LteSchedulerENBPf::getNumAllocatedRbs(
    const AllocatedRbgRange& arr,
    int numberOfRbGroup,
    int rbGroupSize,
    int lastRbGroupSize)
{
    int numAllocatedRbs = 0;

    if (arr.getNumAllocatedRbgs() > 0)
    {
        if (arr.upper == (numberOfRbGroup - 1))
        {
            numAllocatedRbs =
                    (arr.getNumAllocatedRbgs() - 1) * rbGroupSize +
                    lastRbGroupSize;
        }else
        {
            numAllocatedRbs =
                    arr.getNumAllocatedRbgs() * rbGroupSize;
        }
    }

    return numAllocatedRbs;
}

// Update average throughput for DL scheduling
//
// \param pSchedulingResult  vector<LteDlSchedulingResultInfo>* :
//    List of DL scheduling results
//
void LteSchedulerENBPf::updateAverageThroughputDl(
        const std::vector < LteDlSchedulingResultInfo > *pSchedulingResult)
{
    // Update average throughput
    // Note that average throughput for all of the UEs in the connected
    // UE list shall be updated even if no resources allocated.

    ListLteRnti tempList;
    Layer3LteGetSchedulableListSortedByConnectedTime(_node, _interfaceIndex,
        &tempList);
    const ListLteRnti* listLteRnti = &tempList;

    ListLteRnti::const_iterator it;

    if (pSchedulingResult == NULL)
    {
        for (it = listLteRnti->begin(); it != listLteRnti->end(); ++it)
        {
            _filteringModule->update(
                *it, LTE_LIB_PF_AVERAGE_THROUGHPUT_DL,
                0.0,
                _pfFilterCoefficient);
        }

        return;
    }

    const std::vector < LteDlSchedulingResultInfo > &schedulingResult
        = *pSchedulingResult;

    std::map < LteRnti, const LteDlSchedulingResultInfo* > allocatedUes;

    for (size_t i = 0; i < schedulingResult.size(); ++i)
    {
        allocatedUes.insert(
            std::pair < LteRnti, const LteDlSchedulingResultInfo* > (
                schedulingResult[i].rnti,&schedulingResult[i]));
    }

    for (it = listLteRnti->begin(); it != listLteRnti->end(); ++it)
    {
        double instantThroughput;
        std::map < LteRnti, const LteDlSchedulingResultInfo* >
            ::const_iterator allocated_itr = allocatedUes.find(*it);
        if (allocated_itr == allocatedUes.end())
        {
            // Not allocated
            instantThroughput = 0.0;
        }else
        {
            // allocated
            const LteDlSchedulingResultInfo* result = allocated_itr->second;

            int allocatedBits = 0;

            for (int tbIndex = 0;
                tbIndex < result->numTransportBlock;
                ++tbIndex)
            {
                int transportBlockSize = PhyLteGetDlTxBlockSize(
                    _node,
                    _phyIndex,
                    result->mcsIndex[tbIndex],
                    result->numResourceBlocks);

                allocatedBits += transportBlockSize;
            }

            instantThroughput = allocatedBits /
                                (MacLteGetTtiLength(_node, _interfaceIndex) /
                                (double)SECOND);

        }

        _filteringModule->update(
            *it, LTE_LIB_PF_AVERAGE_THROUGHPUT_DL,
            instantThroughput,
            _pfFilterCoefficient);
    }
}

// Update average throughput for UL scheduling
//
// \param pSchedulingResult  vector<LteUlSchedulingResultInfo>* :
//    List of UL scheduling results
//
void LteSchedulerENBPf::updateAverageThroughputUl(
        const std::vector < LteUlSchedulingResultInfo >* pSchedulingResult)
{
    // Update average throughput
    // Note that average throughput for all of the UEs in the connected
    // UE list shall be updated even if no resources allocated.

    ListLteRnti tempList;
    Layer3LteGetSchedulableListSortedByConnectedTime(_node, _interfaceIndex,
        &tempList);
    const ListLteRnti* listLteRnti = &tempList;

    ListLteRnti::const_iterator it;

    if (pSchedulingResult == NULL)
    {
        for (it = listLteRnti->begin(); it != listLteRnti->end(); ++it)
        {
            _filteringModule->update(
                *it, LTE_LIB_PF_AVERAGE_THROUGHPUT_UL,
                0.0,
                _pfFilterCoefficient);
        }

        return;
    }

    const std::vector < LteUlSchedulingResultInfo > &schedulingResult
        = *pSchedulingResult;

    std::map < LteRnti, const LteUlSchedulingResultInfo* > allocatedUes;

    for (size_t i = 0; i < schedulingResult.size(); ++i)
    {
        allocatedUes.insert(
            std::pair < LteRnti, const LteUlSchedulingResultInfo* > (
                schedulingResult[i].rnti,&schedulingResult[i]));
    }

    for (it = listLteRnti->begin(); it != listLteRnti->end(); ++it)
    {
        double instantThroughput;
        std::map < LteRnti, const LteUlSchedulingResultInfo* > ::const_iterator
            allocated_itr = allocatedUes.find(*it);
        if (allocated_itr == allocatedUes.end())
        {
            // Not allocated
            instantThroughput = 0.0;
        }else
        {
            // allocated
            const LteUlSchedulingResultInfo* result = allocated_itr->second;

            int transportBlockSize = PhyLteGetUlTxBlockSize(
                _node,
                _phyIndex,
                result->mcsIndex,
                result->numResourceBlocks);

            instantThroughput = transportBlockSize /
                                (MacLteGetTtiLength(_node, _interfaceIndex) /
                                (double)SECOND);

        }

        _filteringModule->update(
            *it, LTE_LIB_PF_AVERAGE_THROUGHPUT_UL,
            instantThroughput,
            _pfFilterCoefficient);
    }
}

// Determine re-transmission target UEs for scheduling
//
// \param schedulingResult : std::vector<LteDlSchedulingResultInfo>& :
//    re-tx scheduling list
// \param reTxTargetUes : std::vector<LteRnti>& :
//    re-tx target UEs stored
void LteSchedulerENBPf::scheduleDlReTransmission(
        std::vector < LteDlSchedulingResultInfo > &schedulingResult,
        std::vector < LteRnti > &reTxTargetUes)
{   
    UInt64 ttiNum = MacLteGetTtiNumber(_node, _interfaceIndex);
    int processId = (ttiNum) % DL_HARQ_NUM_HARQ_PROCESS;

    LteMacData* macData = LteLayer2GetLteMacData(_node, _interfaceIndex);
    LteMacConfig* macConfig = GetLteMacConfig(_node, _interfaceIndex);
    
    // get connected UE list
    ListLteRnti tempList;
    Layer3LteGetSchedulableListSortedByConnectedTime(_node, _interfaceIndex,
        &tempList);

    std::vector < LteRnti > shuffleUes(tempList.begin(),tempList.end());

    // shuffle UE list
    randomShuffle(shuffleUes);

    int numTargetUes = shuffleUes.size();

    // Exit scheduling if there is no target user
    if (numTargetUes == 0)
    {
        return;
    }

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
void LteSchedulerENBPf::checkReTxRbGroupIndex(
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
// \param numberOfRbGroup RBGroup size
// \param rbgIndex : std::vector<int>* :
//    allocated RBGroup index list
// \param usableRbgIndex : std::vector<int>* :
//    usable RBGroup index list
void LteSchedulerENBPf::checkUsableRbGroupIndex(
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
// RETURN     :: void : NULL
// **/
void LteSchedulerENBPf::scheduleUlReTransmission(
        std::vector < LteUlSchedulingResultInfo > &schedulingResult,
        std::vector < LteRnti > &reTxTargetUes,
        std::vector<int> *allocRbgIndex)
{   
    allocRbgIndex->clear();
    UInt64 ttiNum = MacLteGetTtiNumber(_node, _interfaceIndex);
    int processId = (ttiNum) % UL_HARQ_NUM_HARQ_PROCESS;

    LteMacData* macData = LteLayer2GetLteMacData(_node, _interfaceIndex);
    LteMacConfig* macConfig = GetLteMacConfig(_node, _interfaceIndex);
    
    // get connected UE list
    ListLteRnti tempList;
    Layer3LteGetSchedulableListSortedByConnectedTime(_node, _interfaceIndex,
        &tempList);

    std::vector < LteRnti > shuffleUes(tempList.begin(),tempList.end());

    int numTargetUes = shuffleUes.size();

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

    int rbGroupSize = _ulRbAllocationUnit;

    int numberOfRbGroup = (int)ceil((double)numRb / rbGroupSize);

    std::map<LteRnti, MacLteUlHarqRxEntity*>::const_iterator harqIte;
    std::vector<LteRnti>::iterator ite;
    for (ite = shuffleUes.begin(); ite != shuffleUes.end(); ite++)
    {
        if ((int)allocRbgIndex->size() >= numberOfRbGroup)
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

            int startRB = rxProcess->_schAllocHistory->at(0).startResourceBlock;
            int numAllocRB = rxProcess->_schAllocHistory->at(0).numResourceBlocks;
            int startRBGIdx = startRB / rbGroupSize;
            int usedNumRBGroup = (int)ceil((double)numAllocRB / rbGroupSize);

            for (int i = startRBGIdx; i < (startRBGIdx + usedNumRBGroup); ++i)
            {
                allocRbgIndex->push_back(i);
            }

            // re-transmission
            LteUlSchedulingResultInfo result;
            result.rnti = rxProcess->_schAllocHistory->at(0).rnti;
            result.startResourceBlock = rxProcess->_schAllocHistory->at(0).startResourceBlock;
            result.numResourceBlocks =  rxProcess->_schAllocHistory->at(0).numResourceBlocks;
            
            int rvidx = HARQ_RedundancyVersionArray[txCount % 4];
            result.mcsIndex = GetMcsIndexFromRvidx(rvidx, rxProcess->_schAllocHistory->at(0).mcsIndex);

            result.dequeueInfo.bearerId = 0;        // no use
            result.dequeueInfo.dequeueSizeByte = 0; // no use

            result.di.isNewData = FALSE;

            schedulingResult.push_back(result);
            reTxTargetUes.push_back(*ite);
        }
    }
}



#ifdef LTE_LIB_LOG

void LteSchedulerENBPf::debugOutputRbgAverageThroughputDl()
{
    ListLteRnti tempList;
    Layer3LteGetSchedulableListSortedByConnectedTime(_node, _interfaceIndex,
        &tempList);
    const ListLteRnti* listLteRnti = &tempList;

    ListLteRnti::const_iterator it;

    for (it = listLteRnti->begin(); it != listLteRnti->end(); ++it)
    {
        double averageThroughput;
        char buf[MAX_STRING_LENGTH];
        _filteringModule->get(
            *it, LTE_LIB_PF_AVERAGE_THROUGHPUT_DL, &averageThroughput);

        std::stringstream ss;
        ss << "rnti=," << STR_RNTI(buf, *it) << ","
           << "avgThroughput=," << averageThroughput << ",";

        lte::LteLog::PfHogeFormat(
            _node,
            _interfaceIndex,
            LTE_STRING_LAYER_TYPE_SCHEDULER,
            "%s,%s",
            "PfAvgThroughtputDl",
            ss.str().c_str());
    }
}

void LteSchedulerENBPf::debugOutputRbgAverageThroughputUl()
{
    ListLteRnti tempList;
    Layer3LteGetSchedulableListSortedByConnectedTime(_node, _interfaceIndex,
        &tempList);
    const ListLteRnti* listLteRnti = &tempList;

    ListLteRnti::const_iterator it;

    for (it = listLteRnti->begin(); it != listLteRnti->end(); ++it)
    {
        double averageThroughput;
        char buf[MAX_STRING_LENGTH];
        _filteringModule->get(
            *it, LTE_LIB_PF_AVERAGE_THROUGHPUT_UL, &averageThroughput);

        std::stringstream ss;
        ss << "rnti=," << STR_RNTI(buf, *it) << ","
           << "avgThroughput=," << averageThroughput << ",";

        lte::LteLog::PfHogeFormat(
            _node,
            _interfaceIndex,
            LTE_STRING_LAYER_TYPE_SCHEDULER,
            "%s,%s",
            "PfAvgThroughtputUl",
            ss.str().c_str());
    }
}

#endif // LTE_LIB_LOG

