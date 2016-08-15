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

#ifndef _SLIDING_WIN_H
#define _SLIDING_WIN_H

#include "clock.h"

/// \defgroup Package_SLIDING-WINDOW SLIDING-WINDOW

/// \file
/// \ingroup Package_SLIDING-WINDOW
/// This file describes data structures and functions to
/// implement a sliding window.

/// sliding time window averager structure
typedef struct {
        double*     pSlot;      // time slots for data measurement
        int         nSlot;      // # of slots
        clocktype   sSize;      // size for each slot
        clocktype   tmBase;     // base time
        clocktype   tmStart;    // time of the first data
        double      total;      // totally cumulative data amount
        double      weight;     // weight factor between -1 and 1
} MsTmWin;

/// initialize time sliding window with the given parameters
///
/// \param pWin  pointer to the time sliding window
/// \param sSize  sliding window slot size
/// \param nSlot  sliding window number of slots
/// \param weight  weight for average computation
/// \param theTime  the current time
void MsTmWinInit(MsTmWin* pWin, clocktype sSize, int nSlot, double weight,
                 clocktype theTime);

// API       :: MsTmWinClear
// PURPOSE   :: clears time sliding window
// PARAMETERS ::
// + pWin   : MsTmWin*  : pointer to the time sliding window
// RETURN    :: void :
void MsTmWinClear(MsTmWin* pWin);

/// resets time sliding window with the given parameters
///
/// \param pWin  pointer to the time sliding window
/// \param sSize  sliding window slot size
/// \param nSlot  sliding window number of slots
/// \param weight  weight for average computation
/// \param theTime  the current time
void MsTmWinReset(MsTmWin* pWin, clocktype sSize, int nSlot, double weight,
                  clocktype theTime);

/// updates time sliding window with the given new data
///
/// \param pWin  pointer to the time sliding window
/// \param data  new data
/// \param theTime  the current time
void MsTmWinNewData(MsTmWin* pWin, double data, clocktype theTime);

/// returns the window size
///
/// \param pWin  pointer to the time sliding window
/// \param theTime  the current time
///
/// \return the window size based on the current time
clocktype MsTmWinWinSize(MsTmWin* pWin, clocktype theTime);

/// computes the data sum of the window
///
/// \param pWin  pointer to the time sliding window
/// \param theTime  the current time
///
/// \return the data sum of the window
double MsTmWinSum(MsTmWin* pWin, clocktype theTime);

/// computes the data average of the window
///
/// \param pWin  pointer to the time sliding window
/// \param theTime  the current time
///
/// \return the data average of the window
double MsTmWinAvg(MsTmWin* pWin, clocktype theTime);

/// computes the total data sum
///
/// \param pWin  pointer to the time sliding window
/// \param theTime  the current time
///
/// \return the total data sum
double MsTmWinTotalSum(MsTmWin* pWin, clocktype theTime);

/// computes the total data average
///
/// \param pWin  pointer to the time sliding window
/// \param theTime  the current time
///
/// \return the total data average
double MsTmWinTotalAvg(MsTmWin* pWin, clocktype theTime);

#endif  // _SLIDING_WIN_H

