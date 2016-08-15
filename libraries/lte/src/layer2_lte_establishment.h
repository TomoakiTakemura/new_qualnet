#ifndef _LAYER2_LTE_ESTABLISHMENT_H_
#define _LAYER2_LTE_ESTABLISHMENT_H_

#include "layer2_lte_mac.h"

/// Random Access Preamble Info
typedef struct {
    int raPRACHMaskIndex; // ra-PRACH-MaskIndex [0-15]
                          // Phase1 use only
                          // MAC_LTE_DEFAULT_RA_PRACH_MASK_INDEX
    int raPreambleIndex; // ra-PreambleIndex [0-63]
    double preambleReceivedTargetPower;
    LteRnti ueRnti;
} LteRaPreamble;

/// Random Access Grant Info
typedef struct {
    LteRnti ueRnti;
} LteRaGrant;


////////////////////////////////////////////////////////////////////////////
// eNB - API for PHY
////////////////////////////////////////////////////////////////////////////
/// Notify RA Preamble from eNB's PHY Layer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param ueRnti  UE's RNTI
///
void MacLteNotifyReceivedRaPreamble(
    Node* node, int interfaceIndex, const LteRnti& ueRnti,
    BOOL isHandingoverRa);



/// Nortify RRCConnectionSetupComplete from eNB's PHY Layer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param enbRnti  eNB's RNTI (Destination)
/// \param ueRnti  UE's RNTI  (Source)
///
void RrcLteNotifyRrcConnectionSetupComplete(
    Node* node, int interfaceIndex,
    const LteRnti& enbRnti, const LteRnti& ueRnti);



/// Nortify RRCConnectionReconfComplete from eNB's PHY Layer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param enbRnti  eNB's RNTI (Destination)
/// \param ueRnti  UE's RNTI  (Source)
///
void RrcLteNotifyRrcConnectionReconfComplete(
    Node* node, int interfaceIndex,
    const LteRnti& enbRnti, const LteRnti& ueRnti);



////////////////////////////////////////////////////////////////////////////
// eNB - API for MAC
////////////////////////////////////////////////////////////////////////////
/// Notify network entry from UE
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param ueRnti  UE's RNTI
///
void RrcLteNotifyNetworkEntryFromUE(
    Node* node, int interfaceIndex, const LteRnti& ueRnti);

/// Process RA Grant waiting Timer expire event.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param msg  Event message
///
void MacLteProcessRaGrantWaitingTimerExpired(
    Node* node, UInt32 interfaceIndex, Message* msg);

/// Process RA Backoff waiting Timer expire event.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param msg  Event message
///
void MacLteProcessRaBackoffWaitingTimerExpired(
    Node* node, UInt32 interfaceIndex, Message* msg);

////////////////////////////////////////////////////////////////////////////
// UE - API for RRC
////////////////////////////////////////////////////////////////////////////
/// Random Access Start Point
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
///
void MacLteStartRandomAccess(Node* node, int interfaceIndex);

// RRC ConnectedNotification
/// Notify RRC_CONNECTED from UE's RRC Layer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param handingover  whether handingover
///
void MacLteNotifyRrcConnected(Node* node, int interfaceIndex,
                              BOOL handingover = FALSE);



////////////////////////////////////////////////////////////////////////////
// UE - API for MAC
////////////////////////////////////////////////////////////////////////////
/// Notify Random Access Results from MAC Layer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param enbRnti  eNB's RNTI
/// \param isSuccess      : BOOL    : TRUE  Failure
///
void RrcLteNotifyRandomAccessResults(
    Node* node, int interfaceIndex, const LteRnti& enbRnti, BOOL isSuccess);


////////////////////////////////////////////////////////////////////////////
// UE - API for PHY
////////////////////////////////////////////////////////////////////////////
/// Notify RA Grant from UE's PHY Layer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param raGrant  RA Grant Structure
///
void MacLteNotifyReceivedRaGrant(
    Node* node, int interfaceIndex, const LteRaGrant& raGrant);

/// Notify RA Grant from UE's PHY Layer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param isSuccess  RA Grant Structure
/// \param enbRnti  eNB's RNTI
///    (if isSuccess == FALSE, do not need enbRnti)
///
void RrcLteNotifyCellSelectionResults(
    Node* node, int interfaceIndex, BOOL isSuccess,
    const LteRnti& enbRnti = LTE_INVALID_RNTI);

// RRC Connection Setup Complete notification
/// Notify Detecting Better Cell from PHY Layer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param enbRnti  eNB's RNTI
///
void RrcLteNotifyDetectingBetterCell(
    Node* node, int interfaceIndex, const LteRnti& enbRnti);


/// random access for handover execution
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface Index
/// \param enbRnti  target eNB's RNTI
///
void RrcLteRandomAccessForHandoverExecution(
    Node* node, int interfaceIndex, const LteRnti& enbRnti);



#endif // _LAYER2_LTE_ESTABLISHMENT_H_
