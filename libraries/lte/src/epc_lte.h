#ifndef _EPC_LTE_H_
#define _EPC_LTE_H_

#include "node.h"
#include "epc_lte_common.h"

/// get EPC data
///
/// \param node  Pointer to node.
///
/// \return EPC data on specified node
EpcData*
EpcLteGetEpcData(Node* node);

/// initialize EPC
///
/// \param node  Pointer to node.
/// \param interfaceIndex  interface index
/// \param sgwmmeNodeId  node ID of SGW/MME
/// \param sgwmmeInterfaceIndex  interface index of SGW/MME
/// \param tagetNodeId  Node ID
///
void
EpcLteInit(Node* node, int interfaceIndex,
                NodeAddress sgwmmeNodeId, int sgwmmeInterfaceIndex);

/// Finalize EPC
///
/// \param node  Pointer to node.
///
void
EpcLteFinalize(Node* node);

/// process for message AttachUE
///
/// \param node  Pointer to node.
/// \param ueRnti  UE
/// \param enbRnti  eNodeB
///
void
EpcLteProcessAttachUE(Node* node,
                      const LteRnti& ueRnti, const LteRnti& enbRnti);
/// process for message DetachUE
///
/// \param node  Pointer to node.
/// \param ueRnti  UE
/// \param enbRnti  eNodeB
///
void
EpcLteProcessDetachUE(Node* node,
                      const LteRnti& ueRnti, const LteRnti& enbRnti);

/// process for message DetachUE
///
/// \param node  Pointer to node.
/// \param hoParticipator  participators of H.O.
///
void
EpcLteProcessPathSwitchRequest(
    Node* node,
    const HandoverParticipator& hoParticipator);

/// switch DL path
///
/// \param node  Pointer to node.
/// \param hoParticipator  participators of H.O.
///
/// \return result
BOOL
EpcLteSwitchDLPath(
    Node* node,
    const HandoverParticipator& hoParticipator);

/// send path switch request ack
///
/// \param node  Pointer to node.
/// \param hoParticipator  participators of H.O.
/// \param result  path switch result
///
void
EpcLteSendPathSwitchRequestAck(
    Node* node,
    const HandoverParticipator& hoParticipator,
    BOOL result);

#endif    // _EPC_LTE_H_
