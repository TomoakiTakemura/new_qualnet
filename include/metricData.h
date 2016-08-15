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

#ifndef METRIC_DATA_H
#define METRIC_DATA_H
//--------------------------------------------------------
// data structures for use in managing dynamic statistics
//--------------------------------------------------------

/// Class to identify a specific dynamic statistic.
struct MetricData {
    int  metricID;
    int  metricLayerID;
    int  metricDataType;
    char metricName[MAX_STRING_LENGTH];
};

/// Contains a list of the metrics collected at a layer of the
/// protocol stack.
struct MetricLayerData {
    MetricData* metricList;
    int         numberAllocated;
    int         numberUsed;
};

#endif
