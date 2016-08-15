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

/// \defgroup Package_CLOCK CLOCK

/// \file
/// \ingroup Package_CLOCK
/// This file describes data structures and functions used for time-related operations.

#ifndef QUALNET_CLOCK_H
#define QUALNET_CLOCK_H

#include "types.h"
#include "main.h"

/// \brief Time value
///
/// This type contains a time value, represented as a count of nanoseconds.
/// This is either a time interval, or represents the amount of time since
/// the start of simulation.
///
/// Immedate values of this type should usually be declared using one of
/// the unit macros defined in this header file; for example:
/// \code
/// clocktype defaultTimeout = 3 * SECOND;
/// \endcode
typedef Int64 clocktype;

/// CLOCKTYPE_MAX is the maximum value of clocktype.
/// This value can be anything as long as it is less than or equal to the
/// maximum value of the type which is typedefed to clocktype.
/// Users can simulate the model up to CLOCKTYPE_MAX - 1.

#define CLOCKTYPE_MAX TYPES_ToInt64(0x7fffffffffffffff)

/// like sprintf, prints a clocktype to a string
/// \sa TIME_PrintClockInSecond
#define ctoa(clock, string) CHARTYPE_sprintf( \
                        string, \
                        CHARTYPE_Cast("%" TYPES_64BITFMT "d"), \
                        clock)

/// like atoi or atof, converts a string to a clocktype
/// \sa TIME_ConvertToClock
#define atoc(string, clock) CHARTYPE_sscanf( \
                          string,\
                          CHARTYPE_Cast("%" TYPES_64BITFMT "d"), \
                          clock)

// Units of time defined as units of clocktype.
//   1 ns = 1 unit of clocktype.


/// clocktype value representing 1 nanosecond
#define NANO_SECOND              ((clocktype) 1)

/// clocktype value representing 1 microsecond
#define MICRO_SECOND             (1000 * NANO_SECOND)

/// clocktype value representing 1 millisecond
#define MILLI_SECOND             (1000 * MICRO_SECOND)

/// clocktype value representing 1 second
#define SECOND                   (1000 * MILLI_SECOND)

/// clocktype value representing 1 minute
#define MINUTE                   (60 * SECOND)

/// clocktype value representing 1 hour
#define HOUR                     (60 * MINUTE)

/// clocktype value representing 1 day
#define DAY                      (24 * HOUR)

/// Used to prioritize a process
#define PROCESS_IMMEDIATELY ((clocktype) 0)

/// To get the simulation start time of a node
#define getSimStartTime(node) (*(node->startTime))

/// Read the string in \p buf and provide the corresponding
/// clocktype value for the string using the following conversions:
/// * NS - nano-seconds
/// * US - micro-seconds
/// * MS - milli-seconds
/// * S  - seconds (default if no specification)
/// * M  - minutes
/// * H  - hours
/// * D  - days
///
/// \param buf  The time string
///
/// \return Time in clocktype
clocktype
TIME_ConvertToClock(const char *buf);

/// Print a clocktype value in second. The result is copied
/// to \p stringInSecond
///
/// \param clock  Time in clocktype
/// \param stringInSecond  string containing time in seconds
void
TIME_PrintClockInSecond(clocktype clock, char stringInSecond[]);

/// Print a clocktype value in second. The result is copied
/// to \p stringInSecond
///
/// \param clock  Time in clocktype
/// \param stringInSecond  string containing time in seconds
/// \param node  Input node
void 
TIME_PrintClockInSecond(clocktype clock, char stringInSecond[], Node* node);

/// Print a clocktype value in second. The result is copied
/// to \p stringInSecond
///
/// \param clock  Time in clocktype
/// \param stringInSecond  string containing time in seconds
/// \param partition  Input partition
void 
TIME_PrintClockInSecond(clocktype clock, char stringInSecond[], PartitionData* partition);


/// Return the maximum simulation clock
///
/// \param node  Input node
///
/// \return Returns maximum simulation time
clocktype 
TIME_ReturnMaxSimClock(Node *node);

/// Return the simulation start clock
///
/// \param node  Input node
///
/// \return Returns simulation start time
clocktype 
TIME_ReturnStartSimClock(Node *node);

#endif
