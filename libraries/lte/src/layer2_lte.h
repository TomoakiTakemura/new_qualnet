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

#ifndef _LAYER2_LTE_H_
#define _LAYER2_LTE_H_

#include <list>

#include "lte_common.h"
#include "lte_rrc_config.h"
#include "layer2_lte_rlc.h"
#include "layer2_lte_pdcp.h"
#include "layer2_lte_sch.h"
#include "layer2_lte_mac.h"
#include "layer3_lte.h"

#ifdef LTE_LIB_LOG
#ifdef LTE_LIB_VALIDATION_LOG

#include "log_lte.h"

struct LteLayer2ValidationData {
    UInt32 rlcNumSduToTxQueue; // RLC : Number of SDU to Tx Queue
    UInt32 rlcNumSduToPdcp;    // RLC : Number of SDU to PDCP
    UInt64 rlcBitsToPdcp;      // RLC : Bits of SDU to PDCP

    LteLayer2ValidationData()
        : rlcBitsToPdcp(0)
        , rlcNumSduToTxQueue(0)
        , rlcNumSduToPdcp(0)
    {
    }
};
typedef std::pair < LteRnti, LteLayer2ValidationData* >
    PairLteLayer2ValidationData;
typedef std::map < LteRnti, LteLayer2ValidationData* >
    MapLteLayer2ValidationData;
#endif
#endif

///////////////////////////////////////////////////////////////
// prototype
///////////////////////////////////////////////////////////////
struct lte_rrc_config;
class LteLayer3Filtering;
struct struct_layer3_lte_str;

///////////////////////////////////////////////////////////////
// define
///////////////////////////////////////////////////////////////
#ifdef PARALLEL
  #define LTE_LAYER2_DEFAULT_USE_SPECIFIED_DELAY (TRUE)
  #define LTE_LAYER2_DEFAULT_DELAY_UNTIL_AIRBORN (100 * NANO_SECOND)
#else
  #define LTE_LAYER2_DEFAULT_USE_SPECIFIED_DELAY (FALSE)
  #define LTE_LAYER2_DEFAULT_DELAY_UNTIL_AIRBORN (0)
#endif // PARALLEL_LTE
#define LTE_LAYER2_DEFAULT_NUM_SF_PER_TTI (1)


///////////////////////////////////////////////////////////////
// typedef enum
///////////////////////////////////////////////////////////////
/// Delay will be incurred for Handleoff packet at CRNC
//const clocktype LTE_LAYER2_HANDOFFPKT_DELAY = 1 * MILLI_SECOND;

/// Layer2 sublayer type
typedef enum
{
    LTE_LAYER2_SUBLAYER_MAC,  // layer2 sublayer
    LTE_LAYER2_SUBLAYER_RLC,  // RLC sublayer
    LTE_LAYER2_SUBLAYER_PDCP, // PDCP sublayer
    LTE_LAYER2_SUBLAYER_SCH,  // SCH sublayer
    LTE_LAYER2_SUBLAYER_NONE  // default, for developing purpose only
} LteLayer2SublayerType;

///////////////////////////////////////////////////////////////
// typedef struct
///////////////////////////////////////////////////////////////

// STRUCT:: MacLteMsgDestinationInfo
// DESCRIPTION:: MacLteMsgDestinationInfo
typedef struct struct_mac_lte_msg_destination_info_str
{
    LteRnti dstRnti;
} MacLteMsgDestinationInfo;

/// layer2 data of LTE
typedef struct struct_layer2_data_lte
{
    MacData* myMacData;     // Mac Layer
    RandomSeed randSeed;    // Random Object
    LteRnti myRnti;         // Identifier

    // RRC Config, Layer3Filtering Module
    LteRrcConfig* rrcConfig;
    LteLayer3Filtering* layer3Filtering;

    // Sublayer, Scheduler, Layer3
    LteMacData* lteMacData; // layer2 layer lte struct pointer
    LteRlcData* lteRlcVar; // RLC sublayer struct pointer
    LtePdcpData* ltePdcpData; // PDCP sublayer struct pointer
    class LteScheduler* lteScheduler; // SCH sublayer struct pointer
    Layer3DataLte* lteLayer3Data;

    // configurable parameter
    LteStationType stationType;
    int numSubframePerTti; // 1,2,5,10    //MAC-LTE-NUM-SF-PER-TTI

#ifdef LTE_LIB_LOG
#ifdef LTE_LIB_VALIDATION_LOG
    MapLteLayer2ValidationData* validationData;
#endif
#endif

}Layer2DataLte;

#ifdef LTE_LIB_LOG
#ifdef LTE_LIB_VALIDATION_LOG
LteLayer2ValidationData* LteLayer2GetValidataionData(
    Node* node, int interfaceIndex, const LteRnti& oppositeRnti);
#endif
#endif


//--------------------------------------------------------------------------
//  Utility functions
//--------------------------------------------------------------------------
/// Get Layer2 data
///
///    + node:       pointer to the network node
///    + interfaceIndex: index of interface
///
/// \return Layer2DataLte
Layer2DataLte* LteLayer2GetLayer2DataLte(Node* node, int interfaceIndex);

/// Get PDCP data
///
///    + node:       pointer to the network node
///    + interfaceIndex: index of interface
///
/// \return LtePdcpData
LtePdcpData* LteLayer2GetLtePdcpData(Node* node, int interfaceIndex);

/// Get RLC data
///
///    + node:       pointer to the network node
///    + interfaceIndex: index of interface
///
/// \return LteRlcData
LteRlcData* LteLayer2GetLteRlcData(Node* node, int interfaceIndex);

/// Get MAC data
///
///    + node:       pointer to the network node
///    + interfaceIndex: index of interface
///
/// \return LteMacData
LteMacData* LteLayer2GetLteMacData(Node* node, int interfaceIndex);

/// Get Layer3 filtering data
///
///    + node:       pointer to the network node
///    + interfaceIndex: index of interface
///
/// \return LteLayer3Filtering
LteLayer3Filtering* LteLayer2GetLayer3Filtering(
    Node* node, int interfaceIndex);

/// Get Layer3 data
///
///    + node:       pointer to the network node
///    + interfaceIndex: index of interface
///
/// \return Layer3DataLte
Layer3DataLte* LteLayer2GetLayer3DataLte(Node* node, int interfaceIndex);

/// Get Scheduler data
///
///    + node:       pointer to the network node
///    + interfaceIndex: index of interface
///
/// \return LteScheduler
LteScheduler* LteLayer2GetScheduler(Node* node, int interfaceIndex);

/// Get station type
///
///    + node:       pointer to the network node
///    + interfaceIndex: index of interface
///
/// \return LteStationType
LteStationType LteLayer2GetStationType(Node* node, int interfaceIndex);

/// Get own RNTI
///
///    + node:       pointer to the network node
///    + interfaceIndex: index of interface
///
/// \return LteRnti
const LteRnti& LteLayer2GetRnti(Node* node, int interfaceIndex);

//--------------------------------------------------------------------------
//  Key API functions
//--------------------------------------------------------------------------

/// Called by upper layers to request
/// sending SDU
///
///    + node:       pointer to the network node
///    + interfaceIndex: index of interface
///    + lte:        Layer2 data
///
void  LteLayer2UpperLayerHasPacketToSend(
        Node* node,
        int interfaceIndex,
        Layer2DataLte* lte);

/// Called by lower layers to request
/// receiving PDU
///
///    + node:       pointer to the network node
///    + interfaceIndex: index of interface
///    + lte:        Layer2 data
///
void LteLayer2ReceivePacketFromPhy(Node* node,
                                   int interfaceIndex,
                                   Layer2DataLte* lte,
                                   Message* msg);

/// Pre-initialize LTE layer2 at a given interface.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param nodeInput  Pointer to node input.
///
void LteLayer2PreInit(
    Node* node, int interfaceIndex, const NodeInput* nodeInput);

/// Initialize LTE layer2 at a given interface.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param nodeInput  Pointer to node input.
///
void LteLayer2Init(Node* node,
                    UInt32 interfaceIndex,
                    const NodeInput* nodeInput);

/// Print stats and clear protocol variables.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
///
void LteLayer2Finalize(Node* node, UInt32 interfaceIndex);

/// Handle timers and layer messages.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param msg  Message for node to interpret.
///
void LteLayer2ProcessEvent(Node* node, UInt32 interfaceIndex, Message* msg);


/// get SN from PDCP header
///
/// \param msg  PDCP PDU
///
/// \return SN of this msg
UInt16 LteLayer2GetPdcpSnFromPdcpHeader(
        const Message* msg);



#ifdef ADDON_DB
//--------------------------------------------------------------------------
// FUNCTION   :: LteLayer2GetPacketProperty
// LAYER      :: MAC
// PURPOSE    :: Called by the Mac Events Stats DB
// PARAMETERS :: Node* node
//                   Pointer to the node
//               Message* msg
//                   Pointer to the input message
//               Int32 interfaceIndex
//                   Interface index on which packet received
//               MacDBEventType eventType
//                   Receives the eventType
//               StatsDBMacEventParam& macParam
//                   Input StatDb event parameter
//               BOOL& isMyFrame
//                   Set TRUE if msg is unicast
// RETURN     :: NONE
//--------------------------------------------------------------------------
void LteLayer2GetPacketProperty(Node* node,
                             Message* msg,
                             Int32 interfaceIndex,
                             MacDBEventType eventType,
                             StatsDBMacEventParam& macParam,
                             BOOL& isMyFrame);
#endif

#endif /* _LAYER2_LTE_H_ */
