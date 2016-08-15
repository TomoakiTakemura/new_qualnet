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

/// \defgroup Package_MEMORY MEMORY

/// \file
/// \ingroup Package_MEMORY
/// This file describes the memory management
/// data structures and functions.

#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/// Defines the parameters collected by the memory system.
/// Restricted to kernel use.
struct MemoryUsageData
{
    UInt32 forPeakUsage;
    UInt32 totalAllocated;
    UInt32 totalFreed;
    UInt32 peakUsage;
    UInt32 partitionId;
};

extern UInt32 totalAllocatedMemory;
extern UInt32 totalFreedMemory;
extern UInt32 totalPeakUsage;

/// Creates partition-specific space for collecting memory usage
/// statistics.  This is used in threaded versions of QualNet,
/// but not in distributed versions, currently.
///
void MEM_CreateThreadData();

/// Sets the partition-specific memory data for this partition.
///
/// \param data  the data
void MEM_InitializeThreadData(MemoryUsageData* data);

/// Prints the partition-specific memory data.
///
void MEM_PrintThreadData();

/// Prints out the total memory used by this partition.
///
/// \param partitionId  the partition number
/// \param totalAllocatedMemory  sum of all MEM_malloc calls
/// \param totalFreedMemory  sum of all MEM_free calls
/// \param totalPeakUsage  peak usage of allocated memory
void MEM_ReportPartitionUsage(int    partitionId,
                              UInt32 totalAllocatedMemory,
                              UInt32 totalFreedMemory,
                              UInt32 totalPeakUsage);

/// Prints out the total memory usage statistics for the
/// simulation.  In a parallel run, the peak usage is the sum
/// of the partition's peak usage and might not be precisely
/// accurate.
///
/// \param totalAllocatedMemory  sum of all MEM_malloc calls
/// \param totalFreedMemory  sum of all MEM_free calls
/// \param totalPeakUsage  peak usage of allocated memory
void MEM_ReportTotalUsage(UInt32 totalAllocatedMemory,
                          UInt32 totalFreedMemory,
                          UInt32 totalPeakUsage);

#endif // MEMORY_H
