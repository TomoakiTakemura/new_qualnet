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


/// \defgroup Package_USER USER

/// \file
/// \ingroup Package_USER
/// This file describes data structures and functions used
/// by the User Layer.

#ifndef _USER_H_
#define _USER_H_

#include "random.h"

#ifdef CELLULAR_LIB
#include "user_profile_parser.h"
#include "user_trafficpattern_parser.h"
#endif // CELLULAR_LIB


/// Delay from a cellphone is powered on until it can
/// start working.
#define USER_PHONE_STARTUP_DELAY    (5 * SECOND)

/// The step value that the user dissatisfaction degree is
/// increased each time.
#define USER_INCREASE_DISSATISFACTION    (0.1)

/// The step value that the user dissatisfaction degree is
/// decreased each time.
#define USER_DECREASE_DISSATISFACTION    (-0.1)

/// Defines the default user status start time
#define USER_DEFAULT_STATUS_START_TIME    (10 * SECOND)

/// Status of an user application session.
enum UserApplicationStatus
{
    USER_CALL_DROPPED,
    USER_CALL_REJECTED,
    USER_CALL_COMPLETED
};


/// Data structure stores information of one user
/// application session.
struct UserAppInfo
{

#if defined(CELLULAR_LIB)
    PatternAppData *appData; //points to own instance of data, we need our
                             //own copy of distributions that won't change
    TrafficPatternBeh *patternBeh; //points to global for probability info
                                   //DO NOT accidently delete this when
                                   //freeing memory
#endif // UNIQUE

#if defined(CELLULAR_LIB) || defined(UMTS_LIB)
    int appId; //application ID
    int numRetries;
    BOOL retry; //set to false if app lasts longer than pattern duration
#endif // SHARED

    UserAppInfo* next;
};


/// Data structure stores statuses of a user
struct UserStatus
{
    char name[MAX_STRING_LENGTH]; //name of user status
    clocktype startTime; //start time of status

    UserStatus* next;
};


/// Data structure stores information of a user
struct UserData
{
    RandomSeed seed;

#if defined(CELLULAR_LIB)
    TrafficBehData *trafficBehaviorList; //list of traffic patterns
    TrafficPatternBeh *appList; //list of applications for this pattern
#endif // UNIQUE

#if defined(CELLULAR_LIB) || defined(UMTS_LIB)
    BOOL enabled; //is user layer enabled for app creation?
    BOOL phoneOn;

    int age;
    int sex;
    double dissatisfactionDegree;

    clocktype behaviorDuration; //duration for this pattern

    //details for current pattern
    int maxNumApps; //maximum number of apps that can be active
    RandomDistribution<clocktype> *arrivalInterval; //calculates when next app arrives
    UserAppInfo *currentApps; //list of active apps, needed for retry
    UserStatus *statusList; //list of user status start times

    int numActiveApps;
    //statistics
    double avgDissatisfactionDegree;
    int totalAppsGenerated;
    int totalAppsSuccessfullyFinished;
    int totalAppsRejected;
    int totalAppsDropped;
    int totalRetries;
    double avgRetriesPerApp;
#endif // SHARED
};


/// Reaction to the status change of an application session
///
/// \param node  Pointer to node.
/// \param appStatus  New status of the app session
///
void USER_HandleCallUpdate(Node *node, int appId,
                           UserApplicationStatus appStatus);

/// Handle messages and events for user layer
///
/// \param node  Pointer to node.
/// \param msg  The event
///
void USER_HandleUserLayerEvent(Node *node, Message *msg);

/// Set a user's traffic pattern based on its profile.
///
/// \param node  Pointer to node.
///
void USER_SetTrafficPattern(Node *node);

/// Schedule an application arrival time.
///
/// \param node  Pointer to node.
///
void USER_SetApplicationArrival(Node *node);

#endif /* _USER_H_ */
