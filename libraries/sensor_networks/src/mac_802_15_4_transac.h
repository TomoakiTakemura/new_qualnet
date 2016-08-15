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

#ifndef MAC_802_15_4_TRANSAC_H
#define MAC_802_15_4_TRANSAC_H

#include "mac_802_15_4.h"

/// The maximum number of transactions that are allowed
#define M802_15_4_MAXNUMTRANSACTIONS  70

#define M802_15_4_MAXTIMESTANSACTIONEXPIRED 5

//--------------------------------------------------------------------------
// typedefs
//--------------------------------------------------------------------------



//--------------------------------------------------------------------------
// DEVICE LINK FUNCTIONS
//--------------------------------------------------------------------------

/// To initialize device link structure.
///
/// \param devLink  Device Link structure
/// \param addr  MAC address
/// \param cap  Device capabilities
///
inline
void Mac802_15_4DevLinkInit(M802_15_4DEVLINK** devLink,
                            MACADDR addr,
                            UInt8 cap)
{
    *devLink = (M802_15_4DEVLINK*) MEM_malloc(sizeof(M802_15_4DEVLINK));
    (*devLink)->addr64 = addr;
    (*devLink)->addr16 = (UInt16)addr;
    (*devLink)->capability = cap;
    (*devLink)->numTimesTrnsExpired = 0;
    (*devLink)->last = NULL;
    (*devLink)->next = NULL;
};


/// To initialize device link structure.
///
/// \param devLink  Device Link structure
/// \param addr  MAC address
/// \param cap  Device capabilities
///
/// \return Int32
Int32 Mac802_15_4AddDeviceLink(M802_15_4DEVLINK** deviceLink1,
                               M802_15_4DEVLINK** deviceLink2,
                               MACADDR addr,
                               UInt8 cap);


/// 
///
/// \param oper  Operation
/// \param deviceLink1  Device Link structure
/// \param deviceLink2  Device Link structure
/// \param addr  MAC address
/// \param device ...  Double pointer to the
///    device link structure
///
/// \return Int32
Int32 Mac802_15_4UpdateDeviceLink(Int32 oper,
                                  M802_15_4DEVLINK** deviceLink1,
                                  M802_15_4DEVLINK** deviceLink2,
                                  MACADDR addr,
                                  M802_15_4DEVLINK** device = NULL);


/// 
///
/// \param deviceLink1  Device Link structure
///
/// \return Int32
Int32 Mac802_15_4NumberDeviceLink(M802_15_4DEVLINK** deviceLink1);

/// 
///
/// \param deviceLink1  Device Link structure
/// \param deviceLink2  Device Link structure
/// \param addr  MAC address
/// \param cap  Device capabilities
///
/// \return Int32
Int32 Mac802_15_4ChkAddDeviceLink(M802_15_4DEVLINK** deviceLink1,
                                  M802_15_4DEVLINK** deviceLink2,
                                  MACADDR addr,
                                  UInt8 cap);

/// 
///
/// \param deviceLink1  Device Link structure
/// \param deviceLink2  Device Link structure
///
void Mac802_15_4EmptyDeviceLink(M802_15_4DEVLINK** deviceLink1,
                                M802_15_4DEVLINK** deviceLink2);

/// 
///
/// \param deviceLink1  Device Link structure
/// \param coorAddr  MAC address of Coordinator
///
void Mac802_15_4DumpDeviceLink(M802_15_4DEVLINK* deviceLink1,
                               MACADDR coorAddr);


//--------------------------------------------------------------------------
// TRANSACTION LINK FUNCTIONS
//--------------------------------------------------------------------------


/// To initialize transaction link structure.
///
/// \param transLink  Transaction Link structure
///    + pendAM     : UInt8                 :
/// \param pendAddr  MAC address
/// \param p  Message
///    + msduH      : UInt8                 :
///    + kpTime     : clocktype             :
///
inline
void Mac802_15_4TransLink(Node* node,
                          M802_15_4TRANSLINK** translnk,
                          UInt8 pendAM,
                          MACADDR pendAddr,
                          Message* p,
                          UInt8 msduH,
                          clocktype kpTime)
{
    *translnk = (M802_15_4TRANSLINK*) MEM_malloc(sizeof(M802_15_4TRANSLINK));

    M802_15_4TRANSLINK* transLink = *translnk;
    transLink->pendAddrMode = pendAM;
    transLink->pendAddr64 = pendAddr;
    transLink->pkt = p;
    transLink->msduHandle = msduH;
    transLink->expTime = node->getNodeTime() + kpTime;
    transLink->last = NULL;
    transLink->next = NULL;
};

/// To initialize transaction link structure.
///
/// \param node  Node pointer
/// \param transacLink1  Transaction Link structure
/// \param transacLink2  Transaction Link structure
///
void Mac802_15_4PurgeTransacLink(Node* node,
                                 M802_15_4TRANSLINK** transacLink1,
                                 M802_15_4TRANSLINK** transacLink2);

/// 
///
/// \param transacLink1  Transaction Link structure
/// \param transacLink2  Transaction Link structure
///    + pendAM         : UInt8                  :
/// \param pendAddr  MAC address
/// \param p  Message
///    + msduH          : UInt8                  :
///    + kpTime         : clocktype              :
///
/// \return Int32
Int32 Mac802_15_4AddTransacLink(Node* node,
                                M802_15_4TRANSLINK **transacLink1,
                                M802_15_4TRANSLINK **transacLink2,
                                UInt8 pendAM,
                                MACADDR pendAddr,
                                Message* p,
                                UInt8 msduH,
                                clocktype kpTime);

/// 
///
/// \param transacLink1  Transaction Link structure
///    + pendAM         : UInt8                  :
/// \param pendAddr  MAC address
///
/// \return Message*
Message* Mac802_15_4GetPktFrTransacLink(M802_15_4TRANSLINK** transacLink1,
                                        UInt8 pendAM,
                                        MACADDR pendAddr);

/// 
///
/// \param node  Node pointer
/// \param oper  Operation
/// \param transacLink1  Transaction Link structure
/// \param transacLink2  Transaction Link structure
///    + pendAM         : UInt8                  :
/// \param pendAddr  MAC address
///
/// \return Int32
Int32 Mac802_15_4UpdateTransacLink(Node* node,
                                   Int32 oper,
                                   M802_15_4TRANSLINK** transacLink1,
                                   M802_15_4TRANSLINK** transacLink2,
                                   UInt8 pendAM,
                                   MACADDR pendAddr);

/// 
///
/// \param node  Node pointer
/// \param oper  Operation
/// \param transacLink1  Transaction Link structure
/// \param transacLink2  Transaction Link structure
/// \param p  Message
///    + msduH          : UInt8                  :
///
/// \return Int32
Int32 Mac802_15_4UpdateTransacLinkByPktOrHandle(Node* node,
                                                Int32 oper,
                                                M802_15_4TRANSLINK** transacLink1,
                                                M802_15_4TRANSLINK** transacLink2,
                                                Message* pkt,
                                                UInt8 msduH);

/// 
///
/// \param node  Node pointer
/// \param transacLink1  Transaction Link structure
/// \param transacLink2  Transaction Link structure
///
/// \return Int32
Int32 Mac802_15_4NumberTransacLink(Node* node,
                                   M802_15_4TRANSLINK** transacLink1,
                                   M802_15_4TRANSLINK** transacLink2);

/// 
///
/// \param node  Node pointer
/// \param transacLink1  Transaction Link structure
/// \param transacLink2  Transaction Link structure
///    + pendAM         : UInt8                  :
/// \param pendAddr  MAC address
/// \param p  Message
///    + msduH          : UInt8                  :
///    + kpTime         : clocktype                 :
///
/// \return Int32
Int32 Mac802_15_4ChkAddTransacLink(Node* node,
                                   M802_15_4TRANSLINK** transacLink1,
                                   M802_15_4TRANSLINK** transacLink2,
                                   UInt8 pendAM,
                                   MACADDR pendAddr,
                                   Message* p,
                                   UInt8 msduH,
                                   clocktype kpTime);

/// 
///
/// \param node  Node pointer
/// \param transacLink1  Transaction Link structure
/// \param transacLink2  Transaction Link structure
///
void Mac802_15_4EmptyTransacLink(Node* node,
                                 M802_15_4TRANSLINK** transacLink1,
                                 M802_15_4TRANSLINK** transacLink2);

/// 
///
/// \param transacLink1  Transaction Link structure
/// \param coorAddr  MAC address of Coordinator
///
void Mac802_15_4DumpTransacLink(M802_15_4TRANSLINK* transacLink1,
                                MACADDR coorAddr);

#endif /*MAC_802_15_4_TRANSAC_H*/
