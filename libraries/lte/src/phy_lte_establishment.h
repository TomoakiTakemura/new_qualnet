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

#ifndef _PHY_LTE_ESTABLISHMENT_H_
#define _PHY_LTE_ESTABLISHMENT_H_

#include <list>
#include <map>
#include "layer2_lte_establishment.h"

/// Handle signal arrival from the channel
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param channelIndex  Index of the channel receiving signal from
/// \param propRxInfo  Propagation information
///
/// \return some message info are appended
BOOL PhyLteSignalArrivalFromChannelInEstablishment(
                                    Node* node,
                                    int phyIndex,
                                    int channelIndex,
                                    PropRxInfo* propRxInfo);

/// Handle signal end from a channel
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param channelIndex  Index of the channel receiving signal from
/// \param propRxInfo  Propagation information
///
void PhyLteSignalEndFromChannelInEstablishment(
                                Node* node,
                                int phyIndex,
                                int channelIndex,
                                PropRxInfo* propRxInfo);

/// Start transmitting a frame
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param rxmsg  Message associated with start transmitting event
///
void PhyLteStartTransmittingSignalInEstablishment(
                                Node* node,
                                int phyIndex,
                                Message* msg);

/// End of the transmission
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param msg  Tx End event
///
void PhyLteTransmissionEndInEstablishment(
                                Node* node,
                                int phyIndex,
                                Message* msg);

/// Set a NonServingCellMeasurementInterval Timer
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteSetNonServingCellMeasurementIntervalTimer(
                                Node* node,
                                int phyIndex);


/// Set a NonServingCellMeasurementPeriod Timer
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteSetNonServingCellMeasurementPeriodTimer(
                                Node* node,
                                int phyIndex,
                                BOOL isForHoMeasurement);

/// End of the measurement interval
// NOTE       :: Note that this function is never called
/// if LTE_LIB_USE_ONOFF is not defined.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteNonServingCellMeasurementIntervalExpired(
                                Node* node,
                                int phyIndex);

/// Callback of end of the measurement period
// NOTE       :: Note that this function is called only one time
/// if LTE_LIB_USE_ONOFF is not defined.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteNonServingCellMeasurementPeriodExpired(
                                Node* node,
                                int phyIndex);

/// Get whether the PHY state is a stationary state.
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
/// \return PHY state is a stationary state
bool PhyLteIsInStationaryState(Node* node,
                               int phyIndex);


/// Sending RA Grant
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///    + ueRnti                    : Rnti of UE which receives RA Grant
///
void PhyLteRaGrantTransmissionIndication(
                                Node* node,
                                int phyIndex,
                                LteRnti ueRnti);


/// End of the transmission
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param useMacLayerSpecifiedDelay  Use MAC layer specified delay or not
/// \param initDelayUntilAirborne  Delay until airborne
/// \param preambleInfoFromMac  Random Access Preamble Info
///
void PhyLteRaPreambleTransmissionIndication(
                                Node* node,
                                int phyIndex,
                                BOOL useMacLayerSpecifiedDelay,
                                clocktype initDelayUntilAirborne,
                                const LteRaPreamble* preambleInfoFromMac);

/// RA grant waiting timer time out notification from MAC Layer
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteRaGrantWaitingTimerTimeoutNotification(
                                Node* node,
                                int phyIndex);

/// Pass the mac config to MAC Layer during establishment period
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
/// \return Pointer of LteMacConfig
LteMacConfig* PhyLteGetMacConfigInEstablishment(
                                Node* node,
                                int phyIndex);

/// Start cell search
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteStartCellSearch(Node* node,
                           int phyIndex);



// FUNCTION   :: PhyLteIFPHConfigureMeasConfig
// LAYER      :: PHY
// PURPOSE    :: IF for RRC to setup intra-freq meas config
// PARAMETERS ::
// + node                 : Node* : Pointer to node.
// + phyIndex             : int   : Index of the PHY
// + intervalSubframeNum  : int   : interval of measurement
// + offsetSubframe       : int   : offset of measurement subframe
// + filterCoefRSRP       : int   : filter Coefficient RSRP
// + filterCoefRSRQ       : int   : filter Coefficient RSRQ
// + gapInterval          : clocktype : non Serving Cell Measurement Interval
// RETURN     :: void  : NULL
void PhyLteIFPHConfigureMeasConfig(
                            Node *node,
                            int phyIndex,
                            int intervalSubframeNum,
                            int offsetSubframe,
                            int filterCoefRSRP,
                            int filterCoefRSRQ,
                            clocktype gapInterval);

/// IF for RRC to start intra-freq measurement
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteIFPHStartMeasIntraFreq(
                            Node *node,
                            int phyIndex);

/// IF for RRC to stop intra-freq measurement
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteIFPHStopMeasIntraFreq(
                            Node *node,
                            int phyIndex);

/// IF for RRC to start inter-freq measurement
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteIFPHStartMeasInterFreq(
                            Node *node,
                            int phyIndex);

/// IF for RRC to stop inter-freq measurement
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
///
void PhyLteIFPHStopMeasInterFreq(
                            Node *node,
                            int phyIndex);

/// clear information for H.O. execution
///
/// \param node  Pointer to node.
/// \param phyIndex  Index of the PHY
/// \param selectedRntieNB  target eNB's RNTI
/// \param selectedRrcConfig  rrcConfig to connect to target
///
void PhyLtePrepareForHandoverExecution(
    Node* node,
    int phyIndex,
    const LteRnti& selectedRntieNB,
    const LteRrcConfig& selectedRrcConfig);



#endif /* _PHY_LTE_ESTABLISHMENT_H_ */

