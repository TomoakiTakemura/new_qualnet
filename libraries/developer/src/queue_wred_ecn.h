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


// PROTOCOL :: Queue-Scheduler
//
// SUMMARY  :: WRED is a differentiated dropping algorithm. It is the
/// variant of RED, called weighted RED. It uses three RED algorithms - one
/// destined for packets marked "green", the other for the packets marked
/// "yellow" and the last one for the packets marked "red". It drops "red"
/// packets more aggressively than dropping the "yellow" packets. Again it
/// drops "yellow" packets more aggressively than dropping the "green"
/// packets. When congestion occur it first drops "red" packets, and if
/// the congestion persists how and when it drops all "yellow" or  "green"
/// packets depends on the RED parameter specification for each color.
/// 
// LAYER ::
/// 
// STATISTICS:: Same as wred queue
/// 
// CONFIG_PARAM :: Same as wred queue
/// 
// VALIDATION :: N.A.
/// 
// IMPLEMENTED_FEATURES :: It drops "red" packets more aggressively than
/// dropping the "yellow" packets. Again it drops "yellow" packets more
/// aggressively than dropping the "green" packets. When congestion occur
/// it first drops "red" packets, and if the congestion persists how and
/// when it drops all "yellow" or  "green" packets depends on the RED
/// parameter specification for each color.
/// 
// OMITTED_FEATURES ::
/// 
// ASSUMPTIONS :: WRED is a dropping as well as queuing algorithm for
/// Assured forwarding Services. As mentioned above it use three RED
/// algorithm - one for the packets marked Green and the other for the packet
/// marked Yellow and the third for the packet marked Red. This marking is
/// done in the DSCP field in the IP header by the "marker (profile meter)"
/// at the edge of a DS-domain. In DiffServ there are two types of service
/// Assured service
/// Premium service
/// The building blocks of assured service include a traffic marker at the
/// edge of the domain, and a differentiated dropping algorithm in the
/// network interior. Assured forwarding is a group of 12 PHBs (4 classes X 3
/// levels of drop prec.). The corresponding DSCP fields are:
/// AF Class 1: 001dd0
/// AF Class 2: 010dd0
/// AF Class 3: 011dd0
/// AF Class 4: 100dd0
/// Where the drop level specifications ( dd ) are as below:
/// Level Low Drop : 01 (Green)
/// Level Medium Drop : 10 (Yellow)
/// Level High Drop : 11 (Red)
/// Within each AF class, RED- like buffer management.
/// 
// STANDARD ::
/// + www.sce.carleton.ca/faculty/lambadaris/recent-papers/162.pdf
/// + http://www.cisco.com/warp/public/732/Tech/red/
/// 
// RELATED :: N.A.


#ifndef QUEUE_WRED_ECN_H
#define QUEUE_WRED_ECN_H

#include "queue_red_ecn.h"
#include <map>

#define WRED_DEBUG 0

/// Denotes the number of phbs used in MRED

#define MRED_NUM_PHB 3

/// Denotes the default number of green packets in the queue
/// that represents the lower bound at which green packets
/// can be randomly dropped.

#define DEFAULT_GREEN_PROFILE_MIN_THRESHOLD        10

/// Denotes the default number of green profile packets
/// in the queue that represents the upper bound at which
/// green packets can be randomly dropped.

#define DEFAULT_GREEN_PROFILE_MAX_THRESHOLD        20

/// Denotes the default number of yellow packets in the queue
/// that represents the lower bound at which yellow packets
/// can be randomly dropped.

#define DEFAULT_YELLOW_PROFILE_MIN_THRESHOLD       5

/// Denotes the default number of yellow profile packets
/// in the queue that represents the upper bound at which
/// yellow packets can be randomly dropped.

#define DEFAULT_YELLOW_PROFILE_MAX_THRESHOLD       10

/// Denotes the default number of red packets in the queue
/// that represents the lower bound at which red packets
/// can be randomly dropped.

#define DEFAULT_RED_PROFILE_MIN_THRESHOLD          2

/// Denotes the default number of red profile packets
/// in the queue that represents the upper bound at which
/// red packets can be randomly dropped.

#define DEFAULT_RED_PROFILE_MAX_THRESHOLD          5

/// TOS fifth bit

#define DSCP_FIFTH_BIT  2

/// TOS fourth bit

#define DSCP_FOURTH_BIT 4

/// Green Profile packet

#define GREEN_PROFILE 0

/// Yellow Profile packet

#define YELLOW_PROFILE 1

/// Red Profile packet

#define RED_PROFILE 2

//                (&(mred->phbParams[GREEN_PROFILE]))
// DESCRIPTION :: PHB profile for green packet

#define GREEN_PROFILE_PHB(mred) (&(mred->phbParams[GREEN_PROFILE]))

//                (&(mred->phbParams[YELLOW_PROFILE]))
// DESCRIPTION :: PHB profile for yellow packet

#define YELLOW_PROFILE_PHB(mred) (&(mred->phbParams[YELLOW_PROFILE]))

//                (&(mred->phbParams[RED_PROFILE]))
// DESCRIPTION :: PHB profile for red packet

#define RED_PROFILE_PHB(mred) (&(mred->phbParams[RED_PROFILE]))


/// This class is derived from RedQueue to Simulate RED
/// with ECN support.

class EcnWredQueue:public EcnRedQueue
{
    private:
        std::map<UInt64, int> queue_profile;

    protected:
      int MredReturnPacketProfile(const int dscp);
      PhbParameters* MredReturnPhbForDs(const RedEcnParameters* mred,
                                        const int dscp);

    public:
      EcnWredQueue(Node* node,
                    const char queueTypeString[],
                    const int queueSize,
                    const int interfaceIndex,
                    const int queueNumber,
                    const int infoFieldSize = 0,
                    const bool enableQueueStat = false,
                    const bool showQueueInGui = false,
                    const clocktype currentTime = 0,
                    const RedEcnParameters* configInfo = NULL
#ifdef ADDON_DB
                    , const char* queuePosition = NULL
                    , const bool isFromNetworkLayer = false
#endif
                    , const clocktype maxPktAge = CLOCKTYPE_MAX
                    );

        EcnWredQueue(Node* node,
                    const char queueTypeString[],
                    const int queueSize,
                    const int interfaceIndex,
                    const int queueNumber,
                    const bool fromDerived,
                    const int infoFieldSize = 0,
                    const bool enableQueueStat = false,
                    const bool showQueueInGui = false,
                    const clocktype currentTime = 0
#ifdef ADDON_DB
                    , const char* queuePosition = NULL
                    , const bool isFromNetworkLayer = false
#endif
                    , const clocktype maxPktAge = CLOCKTYPE_MAX
                    );

      virtual void insert(Message* msg,
                          const void* infoField,
                          BOOL* QueueIsFull,
                          const clocktype currentTime,
                          const double serviceTag = 0.0);

      virtual void insert(Message* msg,
                          const void* infoField,
                          BOOL* QueueIsFull,
                          const clocktype currentTime,
                          TosType* tos,
                          const double serviceTag = 0.0);

      virtual BOOL retrieve(Message** msg,
                            const int index,
                            const QueueOperation operation,
                            const clocktype currentTime,
                            double* serviceTag = NULL);

      virtual void finalize(Node *node,
                            const char *layer,
                            const int interfaceIndex,
                            const int instanceId,
                            const char *invokingProtocol = "IP",
                            const char* splStatStr = NULL);
};


// -------------------------------------------------------------------------
// Read and configure WRED
// -------------------------------------------------------------------------

/// This function reads Three color, ECN enabled WRED
/// configuration parameters from QualNet .config file.
///
/// \param node  Pointer to Node structure
/// \param interfaceIndex  Interface Index
/// \param nodeInput  Pointer to NodeInput
/// \param enableQueueStat  Tag for enabling Queue Statistics
/// \param queueIndex  Queue Index
/// \param wred  Pointer of pointer to RedEcnParameters
///    Structure that keeps all configurable
///    entries for Three color, ECN enabled WRED
///    Queue.
///

void ReadWred_EcnConfigurationThParameters(
    Node* node,
    int interfaceIndex,
    const NodeInput *nodeInput,
    BOOL enableQueueStat,
    int queueIndex,
    RedEcnParameters** wred);



#endif // QUEUE_WRED_ECN_H
