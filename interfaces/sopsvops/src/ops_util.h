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
// use in compliance with the license agreement as part of the Qualnet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

#ifndef _OPS_UTIL_H_
#define _OPS_UTIL_H_

// Object Property Store Utility Functions

#include "types.h"
#include <sstream>
#include <string>
#include <vector>

/// Gets a wall clock system time
/// \return Clock ticks as scaled by CLOCKS_PER_SEC. 
/// Windows ticks are mlliseconds.
/// Linux ticks in this implementation are microseconds.
#ifdef _WIN32

// Windows doesn't have gettimeofday, but the clock() function in time.h
// can be used for getting the time for checking real time expiries.
#include <time.h>
#define OPS_GetRealtime clock
#define realtime_t clock_t

#else

// On Linux the clock() function reports the program user time, not the 
// wall click time. THe gettimeofday function is the desired choice.
#include <sys/time.h>
typedef Int64 realtime_t;
#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000L
#endif
realtime_t OPS_GetRealtime();

#endif 

/// Windows uses Sleep that takes milliseconds.
/// Unix has sleep that takes seconds and usleep that 
/// takes microseconds. Actual sleep times are approximate
/// and used the timesharing clock tick as the basic unit.
/// 
/// This function enables the code to simply request
/// to sleep for some milliseconds independent of the
/// platform.
///
/// \param milliseconds  number of milliseconds to sleep
void OPS_Sleep(unsigned int milliseconds);

/// Splits a string into a set of strings on a delimiter 
/// character.
///
/// \param str  string to be split
/// \param out      :  output array of one or more strings
/// \param delim  character to split the string
void OPS_SplitString(
    const std::string& str, std::vector<std::string>& out, const std::string& delim = ",");

/// Return a segment of a string or empty string if not found
/// getSegment("/abc/def/ghi", 1) returns "def"
///
/// \param str  string to be split
/// \param index    :  Segment to return

// 
std::string OPS_GetSegment(const std::string& str, int index);

///**
// FUNCTION   :: OPS_ToNumber
// PURPOSE    :: Converts a string into a double value. If the string is
//               non-numeric the value returned is 0.0.
// PARAMETERS ::
// + numstr    : string     : string containing a number
// RETURNS    :: zero if the string contains non-numeric characters,
//               else, the number represented by the string.
double OPS_ToNumber(const std::string& numstr);

/// Compares a key string to a second string that may contain 
/// the wildcard character, '*', within it between slashes.
/// It returns true if the string matches the wildcard string.
///
/// \param key  a key value to be compared to the wildcard
/// \param wild  the optionally wildcarded key value to match
///
/// \return true for a match, else false.
/// 
/// For example:
/// > wildcardMatch("/node/1/waypoint/speed", "/node/*/waypoint/speed")
/// returns true.
/// > wildcardMatch("/node/1/waypoint/speed", "/node*/waypoint/speed")
/// returns false
/// > wildcardMatch("/node/1/waypoint/speed", "/node/*/speed")
/// returns false
bool OPS_WildcardMatch(const char *key, const char *wild);
bool OPS_WildcardMatch(const std::string& key, const std::string& wild);

#endif // _OPS_UTIL_H_
