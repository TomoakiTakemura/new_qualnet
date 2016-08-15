#ifndef _EPC_LTE_APP_H_
#define _EPC_LTE_APP_H_

#include <map>
#include "message.h"
#include "fileio.h"
#include "lte_common.h"
#include "layer3_lte.h"

#define EPC_LTE_APP_CAT_EPC_APP    "EpcApp"
#define EPC_LTE_APP_CAT_EPC_PROCESS     "EpcProcess"
#define EPC_LTE_APP_DELAY               0

/// argument for AttachUE
typedef struct struct_epc_lte_app_message_argument_attach_ue
{
    LteRnti ueRnti;
    LteRnti enbRnti;
} EpcLteAppMessageArgument_AttachUE;

/// argument for DetachUE
typedef struct struct_epc_lte_app_message_argument_detach_ue
{
    LteRnti ueRnti;
    LteRnti enbRnti;
} EpcLteAppMessageArgument_DetachUE;

/// argument for HoReq
typedef struct struct_epc_lte_app_message_argument_ho_req
{
    HandoverParticipator hoParticipator;
} EpcLteAppMessageArgument_HoReq;

/// argument for HoReqAck
typedef struct struct_epc_lte_app_message_argument_ho_req_ack
{
    HandoverParticipator hoParticipator;
    RrcConnectionReconfiguration reconf;    // including moblityControlInfo
} EpcLteAppMessageArgument_HoReqAck;

typedef struct struct_epc_lte_app_pair_bearerid_snstatus
{
    int bearerId;
    PdcpLteSnStatusTransferItem snStatusTransferItem;
} EpcLteAppPairBearerIdSnStatus;

/// argument for HoReqAck
typedef struct struct_epc_lte_app_message_argument_sn_status_transfer
{
    HandoverParticipator hoParticipator;
    int itemNum;
    EpcLteAppPairBearerIdSnStatus items[1];
} EpcLteAppMessageArgument_SnStatusTransfer;

/// argument for HoReqAck
typedef struct struct_epc_lte_app_message_argument_data_forwarding
{
    HandoverParticipator hoParticipator;
    int bearerId;
    Message* message;
} EpcLteAppMessageArgument_DataForwarding;

/// argument for HoReq
typedef struct struct_epc_lte_app_message_argument_path_switch_req
{
    HandoverParticipator hoParticipator;
} EpcLteAppMessageArgument_PathSwitchRequest;

/// argument for HoReqAck
typedef struct struct_epc_lte_app_message_argument_path_switch_req_ack
{
    HandoverParticipator hoParticipator;
    BOOL result;
} EpcLteAppMessageArgument_PathSwitchRequestAck;

/// argument for HoReq
typedef struct struct_epc_lte_app_message_argument_end_marker
{
    HandoverParticipator hoParticipator;
} EpcLteAppMessageArgument_EndMarker;

/// argument for HoReq
typedef struct struct_epc_lte_app_message_argument_ue_context_release
{
    HandoverParticipator hoParticipator;
} EpcLteAppMessageArgument_UeContextRelease;

/// argument for HoReq
typedef struct struct_epc_lte_app_message_argument_ho_preparation_failure
{
    HandoverParticipator hoParticipator;
} EpcLteAppMessageArgument_HoPreparationFailure;

/// common container for EPC message
typedef struct struct_epc_lte_app_message_container
{
    LteRnti src;
    LteRnti dst;
    EpcMessageType type;
    Int32 length;
    char value[1];
} EpcLteAppMessageContainer;

/// process event for EPC
///
/// \param node  Pointer to node.
/// \param msg  Message for node to interpret.
///
void
EpcLteAppProcessEvent(Node *node, Message *msg);

/// common sender called by sender of each API
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param src  source of message
/// \param dst  dest of message
/// \param type  message type
/// \param payloadSize  payload size
/// \param payload  pointer to payload
/// \param virtualPacketSize  virtual size of packet
///    when -1 specified, actual size used
///
void
EpcLteAppSend(Node* node,
              int interfaceIndex,
              const LteRnti& src,
              const LteRnti& dst,
              EpcMessageType type,
              int payloadSize,
              char *payload,
              int virtualPacketSize = -1);

/// send to UDP layer
///
///
void
EpcLteAppCommitToUdp(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    int outgoingInterface,
    char *payload,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    int virtualPacketSize);

/// API to send message AttachUE
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param src  UE
/// \param dst  eNB
///
void
EpcLteAppSend_AttachUE(Node* node,
                       int interfaceIndex,
                       const LteRnti& ueRnti,
                       const LteRnti& enbRnti);

/// API to send message DetachUE
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param src  UE
/// \param dst  eNB
///
void
EpcLteAppSend_DetachUE(Node* node,
                       int interfaceIndex,
                       const LteRnti& ueRnti,
                       const LteRnti& enbRnti);

/// API to send message HandoverRequest
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///
void EpcLteAppSend_HandoverRequest(
                    Node *node,
                    UInt32 interfaceIndex,
                    const HandoverParticipator& hoParticipator);

/// API to send message HandoverRequestAck
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
/// \param reconf  reconfiguration
///
void EpcLteAppSend_HandoverRequestAck(
                    Node *node,
                    UInt32 interfaceIndex,
                    const HandoverParticipator& hoParticipator,
                    const RrcConnectionReconfiguration& reconf);

/// API to send message SnStatusTransfer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
/// \param snStatusTransferItem  map<int, PdcpLteSnStatusTransferItem>& :
///
void EpcLteAppSend_SnStatusTransfer(
    Node *node,
    UInt32 interfaceIndex,
    const HandoverParticipator& hoParticipator,
    std::map<int, PdcpLteSnStatusTransferItem>& snStatusTransferItem);

/// API to send message DataForwarding
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
/// \param bearerId  bearerId
/// \param forwardingMsg  message
///
void EpcLteAppSend_DataForwarding(
                            Node* node,
                            int interfaceIndex,
                            const HandoverParticipator& hoParticipator,
                            int bearerId,
                            Message* forwardingMsg);

/// API to send message PathSwitchRequest
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///
void EpcLteAppSend_PathSwitchRequest(
    Node* node,
    int interfaceIndex,
    const HandoverParticipator&hoParticipator);

/// API to send message PathSwitchRequestAck
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
/// \param result  path switch result
///
void EpcLteAppSend_PathSwitchRequestAck(
    Node* node,
    int interfaceIndex,
    const HandoverParticipator&hoParticipator,
    BOOL result);

/// API to send message EndMarker
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param src  source of message
/// \param dst  dest of message
/// \param hoParticipator  participators of H.O.
///
void EpcLteAppSend_EndMarker(
                    Node *node,
                    UInt32 interfaceIndex,
                    const LteRnti& src,
                    const LteRnti& dst,
                    const HandoverParticipator& hoParticipator);

/// API to send message UeContextRelease
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///
void EpcLteAppSend_UeContextRelease(
    Node* node,
    int interfaceIndex,
    const HandoverParticipator&hoParticipator);

/// API to send message HoPreparationFailure
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param hoParticipator  participators of H.O.
///
void EpcLteAppSend_HoPreparationFailure(
    Node* node,
    int interfaceIndex,
    const HandoverParticipator&hoParticipator);

/// API to send message HandoverRequestAck
///
/// \param node  Pointer to node.
/// \param targetNodeId  Node ID
///
/// \return address of specified node on EPC subnet
NodeAddress EpcLteAppGetNodeAddressOnEpcSubnet(
    Node* node, NodeAddress targetNodeId);

/// API to send message HandoverRequestAck
///
/// \param node  Pointer to node.
/// \param targetNodeId  Node ID
///
/// \return result
BOOL EpcLteAppIsNodeOnTheSameEpcSubnet(
    Node* node, NodeAddress targetNodeId);

#endif  // _EPC_LTE_APP_H_
