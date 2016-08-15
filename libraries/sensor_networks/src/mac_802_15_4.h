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

#ifndef MAC_802_15_4_H
#define MAC_802_15_4_H

#include "api.h"
#include "mac_802_15_4_cmn.h"
#include "csma_802_15_4.h"
#include "sscs_802_15_4.h"
#include "phy_802_15_4.h"
#include "mac_802_15_4_transac.h"

#include "mydef.h"
//#include "app_myAPI.h"

// --------------------------------------------------------------------------
// #define's
// --------------------------------------------------------------------------
/// # of slots contained in a superframe
const UInt8 aNumSuperframeSlots = 16;

/// # of symbols comprising a superframe slot of order 0
const UInt8 aBaseSlotDuration = 60;

//                      aBaseSlotDuration*aNumSuperframeSlots
// DESCRIPTION :: # of symbols comprising a superframe of order 0
const UInt16 aBaseSuperframeDuration
        = aBaseSlotDuration*aNumSuperframeSlots;

/// max value of the backoff exponent in the CSMA-CA algorithm
const UInt8 aMaxBE = 6;


/// # of superframes that a GTS descriptor exists in the beacon
/// frame of a PAN coordinator
const UInt8 aGTSDescPersistenceTime = 4;


/// # of of GTSs that can be allocated by PAN coordinator
const UInt8 aMaxNumGts = 7;

/// Maximum number of GTS slots that can be requested in 
/// a GTS request
const UInt8 aMaxNumGtsSlotsRequested = 15;

/// max # of octets added by the MAC sublayer to its payload
/// w/o security.
const UInt8 aMaxFrameOverhead = 25;

/// max # of symbols (or CAP symbols) to wait for a response
/// frame
const UInt16 aMaxFrameResponseTime  = 10000;

/// max # of retries allowed after a transmission failures
const UInt8 aMaxFrameRetries = 3;

/// max # of consecutive beacons the MAC sublayer can miss w/o
/// declaring a loss of synchronization
const UInt8 aMaxLostBeacons = 4;

/// max # of octets that can be transmitted in the MAC frame
/// payload field
const UInt8 aMaxMACFrameSize = aMaxPHYPacketSize - aMaxFrameOverhead;

/// max size of a frame, in octets, that can be followed by a
/// SIFS period
const UInt8 aMaxSIFSFrameSize = 18;

/// min # of symbols comprising the CAP
const UInt16 aMinCAPLength = 440;

/// min # of symbols comprising a LIFS period
const UInt8 aMinLIFSPeriod = 40;

/// min # of symbols comprising a SIFS period
const UInt8 aMinSIFSPeriod = 12;

/// max # of symbols a device shall wait for a response command
/// following a request command
const UInt16 aResponseWaitTime = 32 * aBaseSuperframeDuration;

/// # of symbols comprising the basic time period used by the
/// CSMA-CA algorithm
const UInt8 aUnitBackoffPeriod = 20;

/// Acknowledgement frame size in Octets
const UInt8 numPsduOctetsInAckFrame = 5;

/// Number of bits in an Octet
const UInt8 numBitsPerOctet = 8;

// maximum propagation delay
const clocktype max_pDelay = (100 * SECOND/200000000);

// String size
#define M802_15_4_STRING_SIZE          81

// Maximum GTS descriptor count
#define M802_15_4_GTS_DESC_COUNT       7

/// The maximum number of symbols to wait for an acknowledgment
/// frame to arrive following a transmitted data frame.
#define M802_15_4_ACKWAITDURATION                  650

/// Indicates whether a co-ordinator is allowing association.
#define M802_15_4_ASSOCIATIONPERMIT                FALSE

/// Indicates whether a device automatically sends data
/// request command if its address is listed in beacon frame.
#define M802_15_4_AUTOREQUEST                      TRUE

/// Indicates whether BLE is enable
#define M802_15_4_BATTLIFEEXT                      FALSE

/// In BLE mode, the number of backoff periods during which
/// the receiver is enabled after the IFS following a beacon.
#define M802_15_4_BATTLIFEEXTPERIODS               6

/// The contents of the beacon payload.
#define M802_15_4_BEACONPAYLOAD                    ""

/// Beacon payload length in octets.
#define M802_15_4_BEACONPAYLOADLENGTH              0

/// Specification of how often the coordinator transmits its
/// beacon.
#define M802_15_4_BEACONORDER                      15

/// The time that the device transmitted its last beacon
/// frame, in symbol periods.
#define M802_15_4_BEACONTXTIME                     0x000000

/// Beacon sequence number
#define M802_15_4_BSN                0

/// The 64-bit address of the coordinator through which the
/// device is associated.
#define M802_15_4_COORDEXTENDEDADDRESS             0xffff

/// The 16-bit address of the coordinator through which the
/// device is associated.
#define M802_15_4_COORDSHORTADDRESS                0xffff

/// Beacon sequence number
#define M802_15_4_DSN                0

/// Indicates whether PAN co-ordinator accepts GTS requests.
#define M802_15_4_GTSPERMIT                        TRUE

/// The maximum value of the backoff exponent, BE, in the
/// CSMA-CA algorithm.
#define M802_15_4_MAXCSMABACKOFFS                  4

/// The minimum value of the backoff exponent, BE, in the
/// CSMA-CA algorithm.
#define M802_15_4_MINBE                            3

/// The 16-bit identifier of the PAN on which the device is
/// operating.
#define M802_15_4_PANID                            0xffff

/// Indicates whether MAC sublayer is in a promiscuous mode
#define M802_15_4_PROMISCUOUSMODE                  FALSE

/// Indicates whether MAC is to enable its receiver during idle
/// periods.
#define M802_15_4_RXONWHENIDLE                     FALSE

/// The 16-bit address that the device uses to communicate
/// in the PAN.
#define M802_15_4_SHORTADDRESS                     0xffff

/// The length of the active portion of the outgoing
/// superframe, including the beacon frame.
#define M802_15_4_SUPERFRAMEORDER                  15

/// The maximum time (in unit periods) that a transaction is
/// stored by a coordinator and indicated in its beacon.
#define M802_15_4_TRANSACTIONPERSISTENCETIME       0x01f4

/// 
#define M802_15_4_ACLENTRYDESCRIPTORSET            NULL

/// 
#define M802_15_4_ACLENTRYDESCRIPTORSETSIZE        0x00

/// 
#define M802_15_4_DEFAULTSECURITY                  FALSE

/// 
#define M802_15_4_ACLDEFAULTSECURITYMATERIALLENGTH 0x15

/// 
#define M802_15_4_DEFAULTSECURITYMATERIAL          NULL

/// Default Position descriptor
#define M802_15_4_DEFAULTGTSPOSITION 255

/// 
#define M802_15_4_DEFAULTSECURITYSUITE             0x00

/// 
#define M802_15_4_SECURITYMODE                     0x00

/// 
#define M802_15_4_DEFAULT_DATARATE_INDEX            0x00

/// Default device capabilities
/// alterPANCoor = true
/// FFD = true
/// mainsPower = false
/// recvOnWhenIdle = false
/// secuCapable = false
/// alloShortAddr = true
#define M802_15_4_DEFAULTDEVCAP                    0xc1

#define M802_15_4DEFFRMCTRL_TYPE_BEACON      0x00
#define M802_15_4DEFFRMCTRL_TYPE_DATA        0x01
#define M802_15_4DEFFRMCTRL_TYPE_ACK         0x02
#define M802_15_4DEFFRMCTRL_TYPE_MACCMD      0x03

#define M802_15_4DEFFRMCTRL_ADDRMODENONE     0x01
#define M802_15_4DEFFRMCTRL_ADDRMODE16       0x02
#define M802_15_4DEFFRMCTRL_ADDRMODE64       0x03


// Broadcast Queue size in bytes
#define M802_15_4BROADCASTQUEUESIZE          10000


// --------------------------------------------------------------------------
// typedef's enums
// --------------------------------------------------------------------------

/// Timers used by MAC
typedef enum
{
    M802_15_4TXOVERTIMER,
    M802_15_4TXTIMER,
    M802_15_4EXTRACTTIMER,
    M802_15_4ASSORSPWAITTIMER,
    M802_15_4DATAWAITTIMER,
    M802_15_4RXENABLETIMER,
    M802_15_4SCANTIMER,
    M802_15_4BEACONTXTIMER,
    M802_15_4BEACONRXTIMER,
    M802_15_4BEACONSEARCHTIMER,
    M802_15_4TXCMDDATATIMER,
    M802_15_4BACKOFFBOUNDTIMER,
    M802_15_4ORPHANRSPTIMER,
    M802_15_4IFSTIMER,
    M802_15_4CHECKOUTGOING,


    M802_15_4BROASCASTTIMER,

    M802_15_4GTSTIMER,
    M802_15_4GTSDEALLOCATETIMER,
	//takemura CHANGE_BI
#ifdef CHANGE_BI
    M802_15_4VARBEACONRXTIMER
#endif
}M802_15_4TimerType;

// --------------------------------------------------------------------------
// typedef's struct
// --------------------------------------------------------------------------

typedef struct mac_802_15_4_taskPending
{
    // ----------------
    BOOL      mcps_data_request;
    UInt8     mcps_data_request_STEP;

    BOOL      mcps_broadcast_request;
    UInt8     mcps_broadcast_request_STEP;

    char      mcps_data_request_frFunc[M802_15_4_STRING_SIZE];
    UInt8     mcps_data_request_TxOptions;
    UInt8     mcps_data_request_Last_TxOptions;
    Message*  mcps_data_request_pendPkt;

    BOOL      mlme_associate_request;
    UInt8     mlme_associate_request_STEP;
    char      mlme_associate_request_frFunc[M802_15_4_STRING_SIZE];
    BOOL      mlme_associate_request_SecurityEnable;
    UInt8     mlme_associate_request_CoordAddrMode;
    Message*  mlme_associate_request_pendPkt;

    BOOL      mlme_associate_response;
    UInt8     mlme_associate_response_STEP;
    char      mlme_associate_response_frFunc[M802_15_4_STRING_SIZE];
    MACADDR   mlme_associate_response_DeviceAddress;
    Message*  mlme_associate_response_pendPkt;

    BOOL      mlme_disassociate_request;
    UInt8     mlme_disassociate_request_STEP;
    char      mlme_disassociate_request_frFunc[M802_15_4_STRING_SIZE];
    BOOL      mlme_disassociate_request_toCoor;
    Message*  mlme_disassociate_request_pendPkt;

    BOOL      mlme_orphan_response;
    UInt8     mlme_orphan_response_STEP;
    char      mlme_orphan_response_frFunc[M802_15_4_STRING_SIZE];
    MACADDR   mlme_orphan_response_OrphanAddress;

    BOOL      mlme_reset_request;
    UInt8     mlme_reset_request_STEP;
    char      mlme_reset_request_frFunc[M802_15_4_STRING_SIZE];
    BOOL      mlme_reset_request_SetDefaultPIB;

    BOOL      mlme_rx_enable_request;
    UInt8     mlme_rx_enable_request_STEP;
    char      mlme_rx_enable_request_frFunc[M802_15_4_STRING_SIZE];
    UInt32    mlme_rx_enable_request_RxOnTime;
    UInt32    mlme_rx_enable_request_RxOnDuration;
    clocktype mlme_rx_enable_request_currentTime;

    BOOL      mlme_scan_request;
    UInt8     mlme_scan_request_STEP;
    char      mlme_scan_request_frFunc[M802_15_4_STRING_SIZE];
    UInt8     mlme_scan_request_ScanType;
    UInt8     mlme_scan_request_orig_macBeaconOrder;
    UInt8     mlme_scan_request_orig_macBeaconOrder2;
    UInt8     mlme_scan_request_orig_macBeaconOrder3;
    UInt16    mlme_scan_request_orig_macPANId;
    UInt32    mlme_scan_request_ScanChannels;
    UInt8     mlme_scan_request_ScanDuration;
    UInt8     mlme_scan_request_CurrentChannel;
    UInt8     mlme_scan_request_ListNum;
    UInt8     mlme_scan_request_EnergyDetectList[27];
    M802_15_4PanEle mlme_scan_request_PANDescriptorList[27];

    BOOL      mlme_start_request;
    UInt8     mlme_start_request_STEP;
    char      mlme_start_request_frFunc[M802_15_4_STRING_SIZE];
    UInt8     mlme_start_request_BeaconOrder;
    UInt8     mlme_start_request_SuperframeOrder;
    BOOL      mlme_start_request_BatteryLifeExtension;
    BOOL      mlme_start_request_SecurityEnable;
    BOOL      mlme_start_request_PANCoordinator;
    UInt16    mlme_start_request_PANId;
    UInt8     mlme_start_request_LogicalChannel;

    BOOL      mlme_sync_request;
    UInt8     mlme_sync_request_STEP;
    char      mlme_sync_request_frFunc[M802_15_4_STRING_SIZE];
    UInt8     mlme_sync_request_numSearchRetry;
    BOOL      mlme_sync_request_tracking;

    BOOL      mlme_poll_request;
    UInt8     mlme_poll_request_STEP;
    char      mlme_poll_request_frFunc[M802_15_4_STRING_SIZE];
    UInt8     mlme_poll_request_CoordAddrMode;
    UInt16    mlme_poll_request_CoordPANId;
    MACADDR   mlme_poll_request_CoordAddress;
    BOOL      mlme_poll_request_SecurityEnable;
    BOOL      mlme_poll_request_autoRequest;
    BOOL      mlme_poll_request_pending;

    BOOL      mlme_gts_request;
    UInt8     mlme_gts_request_STEP;
    UInt8     gtsChracteristics;
    char      mlme_gts_request_frFunc[M802_15_4_STRING_SIZE];
    BOOL      mcps_gts_data_request;
    UInt8     mcps_gts_data_request_STEP;
    char      mlme_gts_data_request_frFunc[M802_15_4_STRING_SIZE];

    BOOL      CCA_csmaca;
    UInt8     CCA_csmaca_STEP;

    BOOL      RX_ON_csmaca;
    UInt8     RX_ON_csmaca_STEP;
}M802_15_4TaskPending;


/// Timer type for MAC 802.15.4
typedef struct mac_802_15_4_timer_str
{
    M802_15_4TimerType timerType;
}M802_15_4Timer;


/// Various transaction link operation
enum mac_802_15_4_TransacLink_Operation
{
    OPER_DELETE_TRANSAC = 1,
    OPER_PURGE_TRANSAC,
    OPER_CHECK_TRANSAC
};


/// Various device link operation
enum mac_802_15_4_DeviceLink_Operation
{
    OPER_DELETE_DEVICE_REFERENCE = 1,
    OPER_CHECK_DEVICE_REFERENCE,
    OPER_GET_DEVICE_REFERENCE
};

/// Steps while sending packets in GTS
enum mac_802_15_4_Gts_Operation
{
    GTS_INIT_STEP,
    GTS_PKT_SENT_STEP,
    GTS_ACK_STATUS_STEP,
    GTS_PKT_RETRY_STEP
};

/// Steps while sending GTS request
enum mac_802_15_4_Gts_Request_Operation
{
    GTS_REQUEST_INIT_STEP,
    GTS_REQUEST_CSMA_STATUS_STEP,
    GTS_REQUEST_PKT_SENT_STEP,
    GTS_REQUEST_ACK_STATUS_STEP
};

/// Steps while sending packets via indirect mode
enum mac_802_15_4_Indirect_Operation
{
    INDIRECT_INIT_STEP,
    INDIRECT_PKT_SENT_STEP,
    INDIRECT_TIMER_EXPIRE_STEP
};

/// Steps while sending packets via direct mode
enum mac_802_15_4_Direct_Operation
{
    DIRECT_INIT_STEP,
    DIRECT_CSMA_STATUS_STEP,
    DIRECT_PKT_SENT_STEP,
    DIRECT_ACK_STATUS_STEP
};

/// Steps while sending poll request to parent
enum mac_802_15_4_Poll_Operation
{
    POLL_INIT_STEP,
    POLL_CSMA_STATUS_STEP,
    POLL_PKT_SENT_STEP,
    POLL_ACK_STATUS_STEP,
    POLL_DATA_RCVD_STATUS_STEP
};

/// Steps while syncing with the parent
enum mac_802_15_4_Sync_Operation
{
    SYNC_INIT_STEP,
    SYNC_BEACON_RCVD_STATUS_STEP
};

/// Backoff states
enum mac_802_15_4_Backoff_Operation
{
    BACKOFF_RESET,
    BACKOFF_SUCCESSFUL,
    BACKOFF_FAILED,
    BACKOFF = 99,
};

/// Transmission options
enum mac_802_15_4_Txop
{
    TxOp_Acked = 0x01,
    TxOp_GTS = 0x02,
    TxOp_Indirect = 0x04,
    TxOp_SecEnabled = 0x08
};

/// Steps while sending association request to parent
enum mac_802_15_4_Association_Operation
{
    ASSOC_INIT_STEP,
    ASSOC_CSMA_STATUS_STEP,         // 1
    ASSOC_PKT_SENT_STEP,            // 2
    ASSOC_ACK_STATUS_STEP,          // 3
    ASSOC_DATAREQ_INIT_STEP,        // 4
    ASSOC_DATAREQ_CSMA_STATUS_STEP, // 5
    ASSOC_DATAREQ_PKT_SENT_STEP,    // 6
    ASSOC_DATAREQ_ACK_STATUS_STEP,  // 7
    ASSOC_RESPONSE_STATUS_STEP      // 8
};

/// Steps while performing ED scanning
enum mac_802_15_4_ED_Scan_Operation
{
    ED_SCAN_INIT_STEP,
    ED_SCAN_SET_CONFIRM_STATUS_STEP,  // 1
    ED_SCAN_TRX_STATE_STATUS_STEP,    // 2
    ED_SCAN_ED_CONFIRM_STATUS_STEP,   // 3
    ED_SCAN_CONFIRM_STEP              // 4
};

/// Steps while performing Active or Passive scanning
enum mac_802_15_4_Active_Passive_Scan_Operation
{
    SCAN_INIT_STEP,
    SCAN_CREATE_BEACON_OR_SET_TRX_STATE_STEP,   // 1
    ACTIVE_SCAN_CSMA_STATUS_STEP,               // 2
    ACTIVE_SCAN_SET_TRX_STATE_STEP,             // 3
    SCAN_TRX_STATE_STATUS_STEP,                 // 4
    SCAN_BEACON_RCVD_STATUS_STEP,               // 5
    SCAN_CHANNEL_POS_CHANGE_STEP,               // 6
    SCAN_CONFIRM_STEP                           // 7
};

/// Steps while performing Orphan scanning
enum mac_802_15_4_Orphan_Scan_Operation
{
    ORPHAN_SCAN_INIT_STEP,
    ORPHAN_SCAN_CREATE_BEACON_STEP,                 // 1
    ORPHAN_SCAN_CSMA_STATUS_STEP,                   // 2
    ORPHAN_SCAN_BEACON_SENT_STATUS_STEP,            // 3
    ORPHAN_SCAN_TRX_STATE_STATUS_STEP,              // 4
    ORPHAN_SCAN_COOR_REALIGNMENT_RECVD_STATUS_STEP, // 5
    ORPHAN_SCAN_CONFIRM_STEP                        // 6
};

/// Steps while starting a device
enum mac_802_15_4_Start_Operation
{
    START_INIT_STEP,
    START_CSMA_STATUS_STEP,                   // 1
    START_TRX_STATE_STATUS_STEP,              // 2
    START_BEACON_SENT_STATUS_STEP             // 3
};


/// PAN Address Information
typedef struct mac_802_15_4_panaddrinfo_str
{
    UInt16 panID;
    union
    {
        UInt16 addr_16;
        MACADDR addr_64; // using 48-bit instead of 64-bit
    };
}M802_15_4PanAddrInfo;

/// GTS descriptor
typedef struct mac_802_15_4_gtsdescriptor_str
{
    UInt16 devAddr16;
    UInt8 slotSpec;    // --(0123):    GTS starting slot
                // --(4567):    GTS length
}M802_15_4GTSDescriptor;

/// GTS fields
typedef struct mac_802_15_4_gtsfields_str
{
    UInt8 spec;        // GTS specification
                // --(012): GTS descriptor count
                // --(3456):    reserved
                // --(7):   GTS permit
    UInt8 dir;     // GTS directions
                // --(0123456): for up to 7 descriptors:
                // 1 = receive only (w.r.t. data transmission by the device)
                // 0 = transmit only (w.r.t. data transmission by the device)
                // --(7):   reserved
    M802_15_4GTSDescriptor list[M802_15_4_GTS_DESC_COUNT];
                 // GTS descriptor list
}M802_15_4GTSFields;

/// GTS specifications
typedef struct mac_802_15_4_gtsspec_str
{
    M802_15_4GTSFields fields;
    UInt8 count;       // GTS descriptor count
    BOOL permit;        // GTS permit
    BOOL recvOnly[M802_15_4_GTS_DESC_COUNT];   // reception only
    UInt8 slotStart[M802_15_4_GTS_DESC_COUNT];    // starting slot
    UInt8 length[M802_15_4_GTS_DESC_COUNT];   // length in slots
    Message *msg[M802_15_4_GTS_DESC_COUNT];  // messages pending to be sent
    clocktype endTime[M802_15_4_GTS_DESC_COUNT];
    Message *deAllocateTimer[M802_15_4_GTS_DESC_COUNT];
    UInt8 aGtsDescPersistanceCount[M802_15_4_GTS_DESC_COUNT];
    UInt16 expiraryCount[M802_15_4_GTS_DESC_COUNT];
    Queue* queue[M802_15_4_GTS_DESC_COUNT];
    UInt8 retryCount[M802_15_4_GTS_DESC_COUNT];
    short appPort[M802_15_4_GTS_DESC_COUNT];
}M802_15_4GTSSpec;

/// Pending Address field
typedef struct mac_802_15_4_pendaddrfield_str
{
    UInt8 spec;        // Pending address specification field
                // --(012): num of short addresses pending
                // --(3):   reserved
                // --(456): num of extended addresses pending
                // --(7):   reserved
    MACADDR addrList[7];    // pending address list
}M802_15_4PendAddrField;

/// Pending Address specification
typedef struct mac_802_15_4_pendaddrspec_str
{
    M802_15_4PendAddrField fields;
    UInt8 numShortAddr;    // num of short addresses pending
    UInt8 numExtendedAddr; // num of extended addresses pending
}M802_15_4PendAddrSpec;

/// Device capabilities
typedef struct mac_802_15_4_devcapability_str
{
    UInt8 cap;  // --(0):   alternate PAN coordinator
                // --(1):   device type (1=FFD,0=RFD)
                // --(2):   power source(1=mains powered,0=non mains powered)
                // --(3):   receiver on when idle
                // --(45):  reserved
                // --(6):   security capability
                // --(7):   allocate address)
    BOOL alterPANCoor;
    BOOL FFD;
    BOOL mainsPower;
    BOOL recvOnWhenIdle;
    BOOL secuCapable;
    BOOL alloShortAddr;
}M802_15_4DevCapability;

/// MAC Frame control
typedef struct mac_802_15_4_framectrl_str
{
    UInt16 frmCtrl; // (PSDU/MPDU) Frame Control (Figure 35)
                    // --leftmost bit numbered 0 and transmitted first
                    // --(012): Frame type (Table 65)
                    //       --(210)=000:       Beacon
                    //       --(210)=001:       Data
                    //       --(210)=010:       Ack.
                    //       --(210)=011:       MAC command
                    //       --(210)=others:    Reserved
                    // --(3):   Security enabled
                    // --(4):   Frame pending
                    // --(5):   Ack. req.
                    // --(6):   Intra PAN
                    // --(789): Reserved
                    // --(ab):  Dest. addressing mode (Table 66)
                    //       --(ba)=00: PAN ID and Addr. field not present
                    //       --(ba)=01: Reserved
                    //       --(ba)=10: 16-bit short address
                    //       --(ba)=11: 64-bit extended address
                    // --(cd):  Reserved
                    // --(ef):  Source addressing mode
    UInt8 frmType;
    BOOL secu;
    BOOL frmPending;
    BOOL ackReq;
    BOOL intraPan;
    UInt8 dstAddrMode;
    UInt8 srcAddrMode;
}M802_15_4FrameCtrl;

/// MAC Superframe specification
typedef struct mac_802_15_4_superframespec_str
{
    UInt16 superSpec;      // (MSDU) Superframe Specification (Figure 40)
                    // --(0123):    Beacon order
                    // --(4567):    Superframe order
                    // --(89ab):    Final CAP slot
                    // --(c):   Battery life extension
                    // --(d):   Reserved
                    // --(e):   PAN Coordinator
                    // --(f):   Association permit
    UInt8 BO;
    UInt32 BI;
    UInt8 SO;
    UInt32 SD;
    UInt32 sd;
    UInt8 FinCAP;
    BOOL BLE;
    BOOL PANCoor;
    BOOL AssoPmt;
}M802_15_4SuperframeSpec;


/// 802_15_4 Beacon Frame Header
typedef struct mac_802_15_4_beacon_frame_str
{
    // ---beacon frame (Figures 10,37)---
    UInt16     MSDU_SuperSpec;     // (MSDU) Superframe Specification
                        // --(0123):    Beacon order
                        // --(4567):    Superframe order
                        // --(89ab):    Final CAP slot
                        // --(c):   Battery life extension
                        // --(d):   Reserved
                        // --(e):   PAN Coordinator
                        // --(f):   Association permit
    M802_15_4GTSFields   MSDU_GTSFields;     // GTS Fields (Figure 38)
    M802_15_4PendAddrField  MSDU_PendAddrFields;    // (MSDU) Address Fields
                        // --(012): # of short addressing pending
                        // --(3):   Reserved
                        // --(456): # of extended addressing pending
                        // --(7):   Reserved
    // MSDU_BeaconPL;            // (MSDU) Beacon Payload
//     UInt16     MFR_FCS;        // (PSDU/MPDU) FCS
}M802_15_4BeaconFrame;

/// 802_15_4 Data Frame Header
typedef struct mac_802_15_4_data_frame_str
{
    // ---data frame
}M802_15_4DataFrame;

/// 802_15_4 Ack Frame Header
typedef struct mac_802_15_4_ack_frame_str
{
    // ---acknowledgement frame
}M802_15_4AckFrame;

/// 802_15_4 MAC command Frame Header
typedef struct mac_802_15_4_cmd_frame_str
{
    // ---MAC command frame
    UInt8      MSDU_CmdType;       // (MSDU) Command Type/Command frame
                                   // identifier
                        // --0x01:  Association request
                        // --0x02:  Association response
                        // --0x03:  Disassociation notification
                        // --0x04:  Data request
                        // --0x05:  PAN ID conflict notification
                        // --0x06:  Orphan notification
                        // --0x07:  Beacon request
                        // --0x08:  Coordinator realignment
                        // --0x09:  GTS request
                        // --0x0a-0xff: Reserved
    // MSDU_CmdPL;               // (MSDU) Command Payload
    UInt16     MFR_FCS;        // same as above
}M802_15_4CommandFrame;

/// 802_15_4 MAC header
typedef struct mac_802_15_4_header_str
{
    // ---MAC header---
    UInt16     MHR_FrmCtrl;
    UInt8      MHR_BDSN;
    M802_15_4PanAddrInfo MHR_DstAddrInfo;
    M802_15_4PanAddrInfo MHR_SrcAddrInfo;
    UInt16     MHR_FCS;
    UInt8      MSDU_CmdType;
    UInt8      msduHandle;
    BOOL       indirect;
    UInt8      phyCurrentChannel;

    /*    // ---MAC sublayer---
    UInt16     MFR_FCS;
    UInt16     MSDU_SuperSpec;
    M802_15_4GTSFields   MSDU_GTSFields;
    M802_15_4PendAddrField  MSDU_PendAddrFields;
    UInt8      MSDU_CmdType;
    UInt8      MSDU_PayloadLen;
    UInt16     pad;
    UInt8      MSDU_Payload[aMaxMACFrameSize];
    BOOL       SecurityUse;
    UInt8      ACLEntry;*/
}M802_15_4Header;


// --------------------------------------------------------------------------
//                       Layer structure
// --------------------------------------------------------------------------

/// Statistics of MAC802.15.4
typedef struct mac_802_15_4_stats_str
{
    // Data related
    UInt32 numDataPktSent;
    UInt32 numDataPktRecd;

    // Mgmt related
    UInt32 numDataReqSent;
    UInt32 numDataReqRecd;
    UInt32 numAssociateReqSent;
    UInt32 numAssociateReqRecd;
    UInt32 numAssociateResSent;
    UInt32 numAssociateResRecd;
    UInt32 numDisassociateReqSent;
    UInt32 numDisassociateReqRecd;
    UInt32 numOrphanIndRecd;
    UInt32 numOrphanResSent;
    UInt32 numBeaconsSent;
    UInt32 numBeaconsReqSent;
    UInt32 numBeaconsReqRecd;
    UInt32 numBeaconsRecd;
    UInt32 numPollReqSent;

    UInt32 numGtsAllocationReqSent;
    UInt32 numGtsDeAllocationReqSent;
    UInt32 numGtsReqRetried;
    UInt32 numGtsReqRecd;
    UInt32 numGtsDeAllocationReqRecd;
    UInt32 numDataPktsSentInGts;
    UInt32 numDataPktsQueuedForGts;
    UInt32 numDataPktsDeQueuedForGts;
    UInt32 numGtsReqIgnored;
    UInt32 numGtsExpired;
    UInt32 numGtsRequestsRejectedByPanCoord;
    UInt32 numGtsAllocationConfirmedByPanCoord;
    UInt32 numDataPktsQueuedForCap;
    UInt32 numDataPktsDeQueuedForCap;
    // common
    UInt32 numAckSent;
    UInt32 numAckRecd;
    UInt32 numPktDropped;
    UInt32 numDataPktDroppedNoAck;
    UInt32 numDataPktRetriedForNoAck;
    UInt32 numDataPktDroppedChannelAccessFailure;
}M802_15_4Statistics;


/// GTS request paramters.
typedef struct gts_request_parameters
{
    BOOL receiveOnly;
    BOOL allocationRequest;
    UInt8 numSlots;
    BOOL active;
}GtsRequestData;


/// Layer structure of MAC802.15.4
typedef struct mac_802_15_4_str
{
    MacData* myMacData;
    M802_15_4PIB mpib;
    PHY_PIB tmp_ppib;
    M802_15_4DevCapability capability;    // device capability

    // --- for beacon ---
    // (most are temp. variables which should be populated before being used)
    BOOL secuBeacon;
    M802_15_4SuperframeSpec sfSpec,sfSpec2,sfSpec3;  // superframe spec
    M802_15_4GTSSpec gtsSpec,gtsSpec2, gtsSpec3;      // GTS specification
    M802_15_4PendAddrSpec pendAddrSpec;      // pending address specification
    UInt8 beaconPeriods,beaconPeriods2;    // # of backoff periods it takes
                                           // to transmit the beacon
    M802_15_4PanEle panDes,panDes2;         // PAN descriptor
    Message* rxBeacon;           // the beacon packet just received
    clocktype  macBcnTxTime;           // the time last beacon sent (in symbol)
    clocktype  macBcnRxTime;   // the time last beacon received
                            //  from within PAN (in clocktype)
    clocktype macBcnRxLastTime;
    clocktype  macBcnOtherRxTime;  // the time last beacon received
                                // from outside PAN (in symbol)
    UInt8  macBeaconOrder2;
    UInt8  macSuperframeOrder2;
    UInt8  macBeaconOrder3;
    UInt8  macSuperframeOrder3;
    UInt8  macBeaconOrder_last;

    BOOL    oneMoreBeacon;
    UInt8  numLostBeacons;         // # of beacons lost in a row
    // ------------------
    UInt16 rt_myNodeID;

    UInt8 energyLevel;

    MACADDR aExtendedAddress;

    BOOL isPANCoor;         // is a PAN coordinator?
    BOOL isCoor;            // is a coordinator?
    BOOL isCoorInDeviceMode;
    BOOL isSyncLoss;        // has loss the synchronization with CO?

//     Phy802_15_4 *phy;
    void* csma;
    void* sscs;

    PLMEsetTrxState trx_state_req;      // tranceiver state required:
    BOOL inTransmission;        // in the middle of transmission
    BOOL beaconWaiting;     // it's about time to transmit beacon (suppress
                            // all other transmissions)
    Message* txBeacon;      // beacon packet to be transmitted (w/o using
                            // CSMA-CA)
    Message* txAck;          // ack. packet to be transmitted (no waiting)
    Message* txBcnCmd;       // beacon or command packet waiting for
        // transmission (using CSMA-CA) -- triggered by receiving a packet
    Message* txBcnCmd2;      // beacon or command packet waiting for
        // transmission (using CSMA-CA) -- triggered by upper layer
    Message* txData;        // data packet waiting for transmission
                            // (using    CSMA-CA)
    Message* txCsmaca;      // for which packet (txBcnCmd/txBcnCmd2/txData)
                            // is CSMA-CA performed
    Message* txPkt;         // packet (any type) currently being transmitted
    Message* rxData;        // data packet received (waiting for passing up)
    Message* rxCmd;         // command packet received (will be handled after
                            // the transmission of ack.)
    UInt32 txPkt_uid;      // for debug purpose
    clocktype rxDataTime;      // the time when data packet received by MAC
    BOOL waitBcnCmdAck;     // only used if (txBcnCmd): waiting for an ack
    BOOL waitBcnCmdAck2;    // only used if (txBcnCmd2): waiting for an ack
    BOOL waitDataAck;       // only used if (txData): waiting for an ack
    UInt8 backoffStatus;       // 0=no backoff yet;1=backoff
            // successful;2=backoff failed;99=in the middle of backoff
    UInt8 numDataRetry;         // # of retries (retransmissions) for data
                                // packet
    UInt8 numBcnCmdRetry;       // # of retries (retransmissions) for beacon
                                // or command packet
    UInt8 numBcnCmdRetry2;      // # of retries (retransmissions) for beacon
                                // or command packet

    // seed for generating random backoff
    RandomSeed seed;

    // Statistics
    M802_15_4Statistics stats;

    // Timers
    Message* txOverT;   // Transmission over timer
    Message* txT;       // Tx timer
    Message* extractT;     // Extraction timer
    Message* assoRspWaitT;     // Association resp timer
    Message* dataWaitT;       // Data wait
    Message* rxEnableT;       // Receive enable
    Message* scanT;               // Scan
    Message* bcnTxT;       // beacon transmission timer
    Message* bcnRxT;       // beacon reception timer
    Message* bcnSearchT;   // beacon search timer
    Message* txCmdDataT;
    Message* backoffBoundT;
    Message* orphanT;
    Message* IFST;   // Interframe space

    // MAC state
    M802_15_4TaskPending taskP;

    // for association and transaction
    M802_15_4DEVLINK* deviceLink1;
    M802_15_4DEVLINK* deviceLink2;
    M802_15_4TRANSLINK* transacLink1;
    M802_15_4TRANSLINK* transacLink2;

    BOOL isBroadCastPacket;
    BOOL isCalledAfterTransmittingBeacon;
    Queue* broadCastQueue;
    Message* broadcastT;
    BOOL isPollRequestPending;
    BOOL ifsTimerCalledAfterReceivingBeacon;

    BOOL sendGtsRequestToPancoord;
    BOOL sendGtsConfirmationPending;
    UInt8 sendGtsTrackCount;

    BOOL receiveGtsConfirmationPending;
    UInt8 receiveGtsTrackCount;

    UInt8 currentFinCap;
    Queue* capQueue;
    Message* txGts;
    UInt8 currentGtsPositionDesc;

    Message* gtsT;
    BOOL CheckPacketsForTransmission;
    GtsRequestData gtsRequestData;
    GtsRequestData gtsRequestPendingData;
    BOOL gtsRequestPending;
    BOOL gtsRequestExhausted;
    BOOL displayGtsStats;

    BOOL isDisassociationPending;

	//takemura
#ifdef CHANGE_BI
//for client
	MyApiBeaconData LastMyBeacon; 	//save MyApiBeaconData(Normal Beacon has thisData too)
    clocktype VarBcnRxTime;		//variable Beacon receive time
	clocktype NorBcnRxTime;		//normal Beacon receive time
    Message* rxMyBeacon;			//variable Beacon Message
    Message* VarBcnRxT;       		// variable beacon reception timer
	Message* NorBcnRxT;				//normal beacon reception timer
	int numVarBcnHandler;				//count received variable beacon during a normal BI
//for server
	UInt8 RecmBO;					//servers MQTTAPP set recommend BO. this value isnt change by MAC

#endif
}MacData802_15_4;


// --------------------------------------------------------------------------
// FUNCTION DECLARATIONS
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// API functions between MAC and PHY
// --------------------------------------------------------------------------

/// Primitive to report result of Data request
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param status  Status received from phy
///
void Mac802_15_4PD_DATA_confirm(Node* node,
                                Int32 interfaceIndex,
                                PhyStatusType status);

/// Primitive to report result of CCA request
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param status  Status received from phy
///
void Mac802_15_4PLME_CCA_confirm(Node* node,
                                 Int32 interfaceIndex,
                                 PhyStatusType status);

/// Primitive to report result of ED request
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param status  Status received from phy
/// \param EnergyLevel  Energy level
///
void Mac802_15_4PLME_ED_confirm(Node* node,
                                Int32 interfaceIndex,
                                PhyStatusType status,
                                UInt8 EnergyLevel);

/// Primitive to report result of GET request
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param status  Status received from phy
/// \param PIBAttribute  Attribute id
/// \param PIBAttributeValue  Attribute value
///
void Mac802_15_4PLME_GET_confirm(Node* node,
                                 Int32 interfaceIndex,
                                 PhyStatusType status,
                                 PPIBAenum PIBAttribute,
                                 PHY_PIB* PIBAttributeValue);

/// Primitive to report result of Set TRX request
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param status  Status received from phy
/// \param toParent  Communicating towards parent?
///
void Mac802_15_4PLME_SET_TRX_STATE_confirm(Node* node,
                                           Int32 interfaceIndex,
                                           PhyStatusType status,
                                           BOOL toParent = FALSE);

/// Primitive to report result of GET request
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param status  Status received from phy
/// \param PIBAttribute  Attribute id
///
void Mac802_15_4PLME_SET_confirm(Node* node,
                                 Int32 interfaceIndex,
                                 PhyStatusType status,
                                 PPIBAenum PIBAttribute);

// --------------------------------------------------------------------------
// API functions between MAC and SSCS
// --------------------------------------------------------------------------

/// Primitive to request data transfer from SSCS to peer entity
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param SrcAddrMode  Source address mode
/// \param SrcPANId  source PAN id
/// \param SrcAddr  Source address
/// \param DstAddrMode  Destination address mode
/// \param DstPANId  Destination PAN id
/// \param DstAddr  Destination Address
/// \param msduLength  MSDU length
/// \param msdu  MSDU
/// \param msduHandle  Handle associated with MSDU
/// \param TxOptions  Transfer options (3bits)
///    bit-1 = ack(1)/unack(0) tx
///    bit-2 = GTS(1)/CAP(0) tx
///    bit-3 = Indirect(1)/Direct(0) tx
///
void Mac802_15_4MCPS_DATA_request(Node* node,
                                  MacData802_15_4* M802_15_4,
                                  UInt8 SrcAddrMode,
                                  UInt16 SrcPANId,
                                  MACADDR SrcAddr,
                                  UInt8 DstAddrMode,
                                  UInt16 DstPANId,
                                  MACADDR DstAddr,
                                  Int32 msduLength,
                                  Message* msdu,
                                  UInt8 msduHandle,
                                  UInt8 TxOptions);

/// Primitive to indicate the transfer of a data SPDU to SSCS
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param SrcAddrMode  Source address mode
/// \param SrcPANId  source PAN id
/// \param SrcAddr  Source address
/// \param DstAddrMode  Destination address mode
/// \param DstPANId  Destination PAN id
/// \param DstAddr  Destination Address
/// \param msduLength  MSDU length
/// \param msdu  MSDU
/// \param mpduLinkQuality  LQI value measured during reception of
///    the MPDU
/// \param SecurityUse  whether security is used
/// \param ACLEntry  ACL entry
///
void Mac802_15_4MCPS_DATA_indication(Node* node,
                                     Int32 interfaceIndex,
                                     UInt8 SrcAddrMode,
                                     UInt16 SrcPANId,
                                     MACADDR SrcAddr,
                                     UInt8 DstAddrMode,
                                     UInt16 DstPANId,
                                     MACADDR DstAddr,
                                     Int32 msduLength,
                                     Message* msdu,
                                     UInt8 mpduLinkQuality,
                                     BOOL SecurityUse,
                                     UInt8 ACLEntry);

/// Primitive to request purging an MSDU from transaction queue
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param msduHandle  Handle associated with MSDU
///
void Mac802_15_4MCPS_PURGE_request(Node* node,
                                   Int32 interfaceIndex,
                                   UInt8 msduHandle);

/// Primitive to request an association with a coordinator
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param LogicalChannel  Channel on which attempt is to be done
/// \param CoordAddrMode  Coordinator address mode
/// \param CoordPANId  Coordinator PAN id
/// \param CoordAddress  Coordinator address
/// \param CapabilityInformation  capabilities of associating device
/// \param SecurityEnable  Whether enable security or not
///
void Mac802_15_4MLME_ASSOCIATE_request(Node* node,
                                       Int32 interfaceIndex,
                                       UInt8 LogicalChannel,
                                       UInt8 CoordAddrMode,
                                       UInt16 CoordPANId,
                                       MACADDR CoordAddress,
                                       UInt8 CapabilityInformation,
                                       BOOL SecurityEnable);

/// Primitive to initiate a response from SSCS sublayer
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param DeviceAddress  Address of device requesting assoc
/// \param AssocShortAddress  Short address allocated by coord
/// \param status  Status of association attempt
/// \param SecurityEnable  Whether enabled security or not
///
void Mac802_15_4MLME_ASSOCIATE_response(Node* node,
                                        Int32 interfaceIndex,
                                        MACADDR DeviceAddress,
                                        UInt16 AssocShortAddress,
                                        M802_15_4_enum status,
                                        BOOL SecurityEnable);

/// Primitive to indicate coordinator intent to leave
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param DeviceAddress  Add of device requesting dis-assoc
/// \param DisassociateReason  Reason for disassociation
/// \param SecurityEnable  Whether enabled security or not
///
void Mac802_15_4MLME_DISASSOCIATE_request(Node* node,
                                          Int32 interfaceIndex,
                                          MACADDR DeviceAddress,
                                          UInt8 DisassociateReason,
                                          BOOL SecurityEnable);

/// Primitive to indicate reception of disassociation command
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param DeviceAddress  Add of device requesting dis-assoc
/// \param DisassociateReason  Reason for disassociation
/// \param SecurityUse  Whether enabled security or not
/// \param ACLEntry  ACL entry
///
void Mac802_15_4MLME_DISASSOCIATE_indication(Node* node,
                                             Int32 interfaceIndex,
                                             MACADDR DeviceAddress,
                                             UInt8 DisassociateReason,
                                             BOOL SecurityUse,
                                             UInt8 ACLEntry);

/// Primitive to request info about PIB attribute
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param PIBAttribute  PIB attribute id
///
void Mac802_15_4MLME_GET_request(Node* node,
                                 Int32 interfaceIndex,
                                 M802_15_4_PIBA_enum PIBAttribute);

/// Primitive to request to the PAN coordinator to allocate a
/// new GTS or to deallocate an existing GTS
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param GTSCharacteristics  characteristics of GTS req
/// \param SecurityEnable  Whether enabled security or not
///
void Mac802_15_4MLME_GTS_request(Node* node,
                                 Int32 interfaceIndex,
                                 UInt8 GTSCharacteristics,
                                 BOOL SecurityEnable,
                                 PhyStatusType status);

/// Primitive to report the result of a GTS req
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param GTSCharacteristics  characteristics of GTS req
/// \param status  status of GTS req
///
void Mac802_15_4MLME_GTS_confirm(Node* node,
                                 Int32 interfaceIndex,
                                 UInt8 GTSCharacteristics,
                                 M802_15_4_enum status);

/// Primitive to indicates that a GTS has been allocated or that
/// a previously allocated GTS has been deallocated.
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param DevAddress  Short address of device
/// \param GTSCharacteristics  characteristics of GTS req
/// \param SecurityUse  Whether enabled security or not
/// \param ACLEntry  ACL entry
///
void Mac802_15_4MLME_GTS_indication(Node* node,
                                    Int32 interfaceIndex,
                                    UInt16 DevAddress,
                                    UInt8 GTSCharacteristics,
                                    BOOL SecurityUse,
                                    UInt8 ACLEntry);

/// Primitive to respond to the MLME-ORPHAN.indication primitive
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param OrphanAddress  Address of orphan device
/// \param ShortAddress  Short address of device
/// \param AssociatedMember  Associated or not
/// \param SecurityEnable  Whether enabled security or not
///
void Mac802_15_4MLME_ORPHAN_response(Node* node,
                                     Int32 interfaceIndex,
                                     MACADDR OrphanAddress,
                                     UInt16 ShortAddress,
                                     BOOL AssociatedMember,
                                     BOOL SecurityEnable);

/// Primitive to request that the MLME performs a reset
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param SetDefaultPIB  Whether to reset PIB to default
///
void Mac802_15_4MLME_RESET_request(Node* node,
                                   Int32 interfaceIndex,
                                   BOOL SetDefaultPIB);

/// Primitive to request that the receiver is either enabled for
/// a finite period of time or disabled
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param DeferPermit  If defer till next superframe permitted
/// \param RxOnTime  No. of symbols from start of superframe
///    after which receiver is enabled
/// \param RxOnDuration  No. of symbols for which receiver is to be
///    enabled
///
void Mac802_15_4MLME_RX_ENABLE_request(Node* node,
                                       Int32 interfaceIndex,
                                       BOOL DeferPermit,
                                       UInt32 RxOnTime,
                                       UInt32 RxOnDuration);

/// Primitive to initiate a channel scan over a given list of
/// channels
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param ScanType  Type of scan (0-ED/Active/Passive/3-Orphan)
/// \param ScanChannels  Channels to be scanned
/// \param ScanDuration  Duration of scan, ignored for orphan scan
///
void Mac802_15_4MLME_SCAN_request(Node* node,
                                  Int32 interfaceIndex,
                                  UInt8 ScanType,
                                  UInt32 ScanChannels,
                                  UInt8 ScanDuration);

/// Primitive to set PIB attribute
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param PIBAttribute  PIB attribute id
/// \param PIBAttributeValue  Attribute value
///
void Mac802_15_4MLME_SET_request(Node* node,
                                 Int32 interfaceIndex,
                                 M802_15_4_PIBA_enum PIBAttribute,
                                 M802_15_4PIB* PIBAttributeValue);

/// Primitive to allow the PAN coordinator to initiate a new PAN
/// or to begin using a new superframe configuration
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param PANId  PAN id
/// \param LogicalChannel  Logical channel
/// \param BeaconOrder  How often a beacon is tx'd.
/// \param SuperframeOrder  Length of active portion of s-frame
/// \param PANCoordinator  If device is PAN coordinator
/// \param BatteryLifeExtension  for battery saving mode
/// \param CoordRealignment  If coordinator realignment command
///    needs to be sent prior to changing superframe config
/// \param SecurityEnable  If security is enabled
///
void Mac802_15_4MLME_START_request(Node* node,
                                   Int32 interfaceIndex,
                                   UInt16 PANId,
                                   UInt8 LogicalChannel,
                                   UInt8 BeaconOrder,
                                   UInt8 SuperframeOrder,
                                   BOOL PANCoordinator,
                                   BOOL BatteryLifeExtension,
                                   BOOL CoordRealignment,
                                   BOOL SecurityEnable);

/// Primitive to request synchronize with the coordinator
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param LogicalChannel  Logical channel
/// \param TrackBeacon  Whether to synchronize with all future beacons
///
void Mac802_15_4MLME_SYNC_request(Node* node,
                                  Int32 interfaceIndex,
                                  UInt8 LogicalChannel,
                                  BOOL TrackBeacon);

/// Primitive to prompt device to request data from coordinator
///
/// \param node  Node receiving call
/// \param interfaceIndex  Interface index
/// \param CoordAddrMode  Coordinator address mode
/// \param CoordPANId  Coordinator PAN id
/// \param CoordAddress  Coordinator address
/// \param SecurityEnable  Whether enable security or not
///
void Mac802_15_4MLME_POLL_request(Node* node,
                                  Int32 interfaceIndex,
                                  UInt8 CoordAddrMode,
                                  UInt16 CoordPANId,
                                  MACADDR CoordAddress,
                                  BOOL SecurityEnable);


Message* Mac802_15_4SetTimer(Node* node,
                             MacData802_15_4* mac802_15_4,
                             M802_15_4TimerType timerType,
                             clocktype delay,
                             Message* msg);

// FUNCTION     Mac802_15_4Init
// PURPOSE      Initialization function for 802.15.4 protocol of MAC layer
/// PARAMETERS   Node* node
/// Node being initialized.
/// NodeInput* nodeInput
/// Structure containing contents of input file.
/// SubnetMemberData* subnetList
/// Number of nodes in subnet.
/// Int32 nodesInSubnet
/// Number of nodes in subnet.
/// Int32 subnetListIndex
/// NodeAddress subnetAddress
/// Subnet address.
/// RETURN       None
/// NOTES        None
void Mac802_15_4Init(Node* node,
                     const NodeInput* nodeInput,
                     Int32 interfaceIndex,
                     NodeAddress subnetAddress,
                     SubnetMemberData* subnetList,
                     Int32 nodesInSubnet);

// FUNCTION     Mac802_15_4Layer
// PURPOSE      Models the behaviour of the MAC layer with the 802.15.4
/// protocol on receiving the message enclosed in msgHdr
/// PARAMETERS   Node *node
/// Node which received the message.
/// Int32 interfaceIndex
/// Interface index.
/// Message* msg
/// Message received by the layer.
/// RETURN       None
/// NOTES        None
void Mac802_15_4Layer(Node* node, Int32 interfaceIndex, Message* msg);


// FUNCTION     Mac802_15_4Finalize
// PURPOSE      Called at the end of simulation to collect the results of
/// the simulation of 802.15.4 protocol of the MAC Layer.
/// PARAMETERS   Node* node
/// Node which received the message.
/// Int32 interfaceIndex
/// Interface index.
/// RETURN       None
/// NOTES        None
void Mac802_15_4Finalize(Node* node, Int32 interfaceIndex);


// NAME         Mac802_15_4NetworkLayerHasPacketToSend
// PURPOSE      To notify 802.15.4 that network has something to send
/// PARAMETERS   Node* node
/// Node which received the message.
/// MacData_802_15_4* M802_15_4
/// 802.15.4 data structure
/// RETURN       None
/// NOTES        None
void Mac802_15_4NetworkLayerHasPacketToSend(Node* node,
                                            MacData802_15_4* M802_15_4);


// NAME         Mac802_15_4ReceivePacketFromPhy
// PURPOSE      To recieve packet from the physical layer
/// PARAMETERS   Node* node
/// Node which received the message.
/// MacData_802_15_4* M802_15_4
/// 802.15.4 data structure
/// Message* msg
/// Message received by the layer.
/// RETURN       None.
/// NOTES        None
void Mac802_15_4ReceivePacketFromPhy(Node* node,
                                     MacData802_15_4* M802_15_4,
                                     Message* msg);


// NAME         Mac802_15_4ReceivePhyStatusChangeNotification
// PURPOSE      Receive notification of status change in physical layer
/// PARAMETERS   Node* node
/// Node which received the message.
/// MacData_802_15_4* M802_15_4
/// 802.15.4 data structure.
/// PhyStatusType oldPhyStatus
/// The old physical status.
/// PhyStatusType newPhyStatus
/// The new physical status.
/// clocktype receiveDuration
/// The receiving duration.
/// RETURN       None.
/// NOTES        None
void Mac802_15_4ReceivePhyStatusChangeNotification(
    Node* node,
    MacData802_15_4* M802_15_4,
    PhyStatusType oldPhyStatus,
    PhyStatusType newPhyStatus,
    clocktype receiveDuration);



// --------------------------------------------------------------------------
// STATIC FUNCTIONS
// --------------------------------------------------------------------------

/// This function is called for parsing Frame Control fields.
///
/// \param frameCtrl  Pointer to Frame Control
///
inline
void Mac802_15_4FrameCtrlParse(M802_15_4FrameCtrl* frameCtrl)
{
    frameCtrl->frmType = (UInt8)((frameCtrl->frmCtrl & 0xe000) >> 13);
    // taking deviation from coding guidelines and using ternary operator
    frameCtrl->secu = ((frameCtrl->frmCtrl & 0x1000) == 0)?FALSE:TRUE;
    frameCtrl->frmPending = ((frameCtrl->frmCtrl & 0x0800) == 0)?FALSE:TRUE;
    frameCtrl->ackReq = ((frameCtrl->frmCtrl & 0x0400) == 0)?FALSE:TRUE;
    frameCtrl->intraPan = ((frameCtrl->frmCtrl & 0x0200) == 0)?FALSE:TRUE;
    frameCtrl->dstAddrMode = (UInt8)((frameCtrl->frmCtrl & 0x0030) >> 4);
    frameCtrl->srcAddrMode = (UInt8)(frameCtrl->frmCtrl & 0x0003);
}

/// This function is called for setting Frame Type.
///
/// \param frameCtrl  Pointer to Frame Control
/// \param frmType  Frame type
///
inline
void Mac802_15_4FrameCtrlSetFrmType(M802_15_4FrameCtrl* frameCtrl,
                                    UInt8 frmType)
{
    frameCtrl->frmType = frmType;
    frameCtrl->frmCtrl = (frameCtrl->frmCtrl & 0x1fff) + (frmType << 13);
}

/// TThis function is called for setting security.
///
/// \param frameCtrl  Pointer to Frame Control
/// \param sc  Security enabled or disabled
///
inline
void Mac802_15_4FrameCtrlSetSecu(M802_15_4FrameCtrl* frameCtrl, BOOL sc)
{
    frameCtrl->secu = sc;
    frameCtrl->frmCtrl = (frameCtrl->frmCtrl & 0xefff);
    if (sc == TRUE)
    {
        frameCtrl->frmCtrl += 0x1000;
    }
}

/// This function is called for setting Pending flag.
///
/// \param frameCtrl  Pointer to Frame Control
/// \param pending  Pending true or not
///
inline
void Mac802_15_4FrameCtrlSetFrmPending(M802_15_4FrameCtrl* frameCtrl,
                                       BOOL pending)
{
    frameCtrl->frmPending = pending;
    frameCtrl->frmCtrl = (frameCtrl->frmCtrl & 0xf7ff);
    if (pending == TRUE)
    {
        frameCtrl->frmCtrl += 0x0800;
    }
}

/// This function is called for setting ACK flag.
///
/// \param frameCtrl  Pointer to Frame Control
/// \param ack  ACK true or not
///
inline
void Mac802_15_4FrameCtrlSetAckReq(M802_15_4FrameCtrl* frameCtrl, BOOL ack)
{
    frameCtrl->ackReq = ack;
    frameCtrl->frmCtrl = (frameCtrl->frmCtrl & 0xfbff);
    if (ack == TRUE)
    {
        frameCtrl->frmCtrl += 0x0400;
    }
}

/// This function is called for setting IntraPAN flag.
///
/// \param frameCtrl  Pointer to Frame Control
/// \param intrapan  Intra-PAN true or not
///
inline
void Mac802_15_4FrameCtrlSetIntraPan(M802_15_4FrameCtrl* frameCtrl,
                                     BOOL intrapan)
{
    frameCtrl->intraPan = intrapan;
    frameCtrl->frmCtrl = (frameCtrl->frmCtrl & 0xfdff);
    if (intrapan)
    {
        frameCtrl->frmCtrl += 0x0200;
    }
}

/// This function is called for setting destination addr mode.
///
/// \param frameCtrl  Pointer to Frame Control
/// \param dstmode  Destination mode
///
inline
void Mac802_15_4FrameCtrlSetDstAddrMode(M802_15_4FrameCtrl* frameCtrl,
                                        UInt8 dstmode)
{
    frameCtrl->dstAddrMode = dstmode;
    frameCtrl->frmCtrl = (frameCtrl->frmCtrl & 0xffcf) + (dstmode << 4);
}

/// This function is called for setting source address mode.
///
/// \param frameCtrl  Pointer to Frame Control
/// \param srcmode  Source mode
///
inline
void Mac802_15_4FrameCtrlSetSrcAddrMode(M802_15_4FrameCtrl* frameCtrl,
                                        UInt8 srcmode)
{
    frameCtrl->srcAddrMode = srcmode;
    frameCtrl->frmCtrl = (frameCtrl->frmCtrl & 0xfffc) + srcmode;
}

/// This function is called for parsing Super Frame fields.
///
/// \param frameCtrl  Pointer to Super Frame Spec
///
inline
void Mac802_15_4SuperFrameParse(M802_15_4SuperframeSpec* superSpec)
{
    superSpec->BO = (UInt8)((superSpec->superSpec & 0xf000) >> 12);
    superSpec->BI = aBaseSuperframeDuration * (1 << superSpec->BO);
    superSpec->SO = (UInt8)((superSpec->superSpec & 0x0f00) >> 8);
    superSpec->SD = aBaseSuperframeDuration * (1 << superSpec->SO);// duration
    superSpec->sd = aBaseSlotDuration * (1 << superSpec->SO);
    superSpec->FinCAP = (UInt8)((superSpec->superSpec & 0x00f0) >> 4);
    superSpec->BLE = ((superSpec->superSpec & 0x0008) == 0)?FALSE:TRUE;
    superSpec->PANCoor = ((superSpec->superSpec & 0x0002) == 0)?FALSE:TRUE;
    superSpec->AssoPmt = ((superSpec->superSpec & 0x0001) == 0)?FALSE:TRUE;
}

/// This function is called for setting BO.
///
/// \param frameCtrl  Pointer to Super Frame Spec
/// \param bo  BO value
///
inline
void Mac802_15_4SuperFrameSetBO(M802_15_4SuperframeSpec* superSpec,
                                UInt8 bo)
{
    superSpec->BO = bo;
    superSpec->BI = aBaseSuperframeDuration * (1 << superSpec->BO);
    superSpec->superSpec = (superSpec->superSpec & 0x0fff) + (bo << 12);
}

/// This function is called for setting SO.
///
/// \param frameCtrl  Pointer to Super Frame Spec
/// \param so  SO value
///
inline
void Mac802_15_4SuperFrameSetSO(M802_15_4SuperframeSpec* superSpec,
                                UInt8 so)
{
    superSpec->SO = so;
    superSpec->SD = aBaseSuperframeDuration * (1 << superSpec->SO);
    superSpec->sd = aBaseSlotDuration * (1 << superSpec->SO);
    superSpec->superSpec = (superSpec->superSpec & 0xf0ff) + (so << 8);
}

/// This function is called for setting CAP.
///
/// \param frameCtrl  Pointer to Super Frame Spec
/// \param fincap  fincap value
///
inline
void Mac802_15_4SuperFrameSetFinCAP(M802_15_4SuperframeSpec* superSpec,
                                    UInt8 fincap)
{
    superSpec->FinCAP = fincap;
    superSpec->superSpec = (superSpec->superSpec & 0xff0f) + (fincap << 4);
}

/// This function is called for setting BLE.
///
/// \param frameCtrl  Pointer to Super Frame Spec
/// \param ble  BLE value
///
inline
void Mac802_15_4SuperFrameSetBLE(M802_15_4SuperframeSpec* superSpec,
                                 BOOL ble)
{
    superSpec->BLE = ble;
    superSpec->superSpec = (superSpec->superSpec & 0xfff7);
    if (ble == TRUE)
    {
        superSpec->superSpec += 8;
    }
}

/// This function is called for setting whether PAN coordinator.
///
/// \param frameCtrl  Pointer to Super Frame Spec
/// \param pancoor  PAN Coordinator
///
inline
void Mac802_15_4SuperFrameSetPANCoor(M802_15_4SuperframeSpec* superSpec,
                                     BOOL pancoor)
{
    superSpec->PANCoor = pancoor;
    superSpec->superSpec = (superSpec->superSpec & 0xfffd);
    if (pancoor == TRUE)
    {
        superSpec->superSpec += 2;
    }
}

/// This function is called for setting whether Association is
/// permitted
///
/// \param frameCtrl  Pointer to Super Frame Spec
/// \param assopmt  Association permitted
///
inline
void Mac802_15_4SuperFrameSetAssoPmt(M802_15_4SuperframeSpec* superSpec,
                                     BOOL assopmt)
{
    superSpec->AssoPmt = assopmt;
    superSpec->superSpec = (superSpec->superSpec & 0xfffe);
    if (assopmt == TRUE)
    {
        superSpec->superSpec += 1;
    }
}


/// This function is called for parsing GTS specification
///
/// \param gtsSpec  Pointer to GTS Spec
///
inline
void Mac802_15_4GTSSpecParse(M802_15_4GTSSpec* gtsSpec)
{
    Int32 i = 0;
    for (i = 0; i < gtsSpec->count; i++)
    {
        // reset the existing gts info
        gtsSpec->recvOnly[i] = 0;
        gtsSpec->slotStart[i] = 0;
        gtsSpec->length[i] = 0;
    }
    gtsSpec->count = (gtsSpec->fields.spec & 0xe0) >> 5;
    gtsSpec->permit = ((gtsSpec->fields.spec & 0x01) != 0)?TRUE:FALSE;
    for (Int32 i=0; i < gtsSpec->count; i++)
    {
        gtsSpec->recvOnly[i] = ((gtsSpec->fields.dir & (1<<(7-i))) != 0);
        gtsSpec->slotStart[i]
            = (gtsSpec->fields.list[i].slotSpec & 0xf0) >> 4;
        gtsSpec->length[i] = (gtsSpec->fields.list[i].slotSpec & 0x0f);
    }
}

/// This function is called for setting GTS descriptor count
///
/// \param gtsSpec  Pointer to GTS Spec
/// \param cnt  Count
///
inline
void Mac802_15_4GTSSpecSetCount(M802_15_4GTSSpec* gtsSpec,
                                UInt8 cnt)
{
    gtsSpec->count = cnt;
    gtsSpec->fields.spec = (gtsSpec->fields.spec & 0x1f) + (cnt << 5);
}


/// This function is called for setting GTS permit
///
/// \param gtsSpec  Pointer to GTS Spec
/// \param pmt  Permit
///
inline
void Mac802_15_4GTSSpecSetPermit(M802_15_4GTSSpec* gtsSpec,
                                 BOOL pmt)
{
    gtsSpec->permit = pmt;
    gtsSpec->fields.spec = (gtsSpec->fields.spec & 0xfe);
    if (pmt == TRUE)
    {
        gtsSpec->fields.spec += 1;
    }
}

/// This function is called for setting receive only on ith slot
///
/// \param gtsSpec  Pointer to GTS Spec
/// \param ith  ith slot
/// \param rvonly  Receive only?
///
inline
void Mac802_15_4GTSSpecSetRecvOnly(M802_15_4GTSSpec* gtsSpec,
                                   UInt8 ith,
                                   BOOL rvonly)
{
    gtsSpec->recvOnly[ith] = rvonly;
    gtsSpec->fields.dir = gtsSpec->fields.dir & ((1<<(7-ith))^0xff);
    if (rvonly == TRUE)
    {
        gtsSpec->fields.dir += (1<<(7-ith));
    }
}

/// This function is called for setting slot start on ith slot
///
/// \param gtsSpec  Pointer to GTS Spec
/// \param ith  ith slot
/// \param st  Start
///
inline
void Mac802_15_4GTSSpecSetSlotStart(M802_15_4GTSSpec* gtsSpec,
                                    UInt8 ith,
                                    UInt8 st)
{
    gtsSpec->slotStart[ith] = st;
    gtsSpec->fields.list[ith].slotSpec =
            (gtsSpec->fields.list[ith].slotSpec & 0x0f) + (st << 4);
}

/// This function is called for setting slot length on ith slot
///
/// \param gtsSpec  Pointer to GTS Spec
/// \param ith  ith slot
/// \param len  Length
///
inline
void Mac802_15_4GTSSpecSetLength(M802_15_4GTSSpec* gtsSpec,
                                 UInt8 ith,
                                 UInt8 len)
{
    gtsSpec->length[ith] = len;
    gtsSpec->fields.list[ith].slotSpec
        = (gtsSpec->fields.list[ith].slotSpec & 0xf0) + len;
}

/// This function is called for getting GTS size
///
/// \param gtsSpec  Pointer to GTS Spec
///
/// \return GTS size
inline
Int32 Mac802_15_4GTSSpecSize(M802_15_4GTSSpec* gtsSpec)
{
    gtsSpec->count = (gtsSpec->fields.spec & 0xe0) >> 5;
    return 1 + 1 + 3 * gtsSpec->count;
}

/// Add new short address to pending list
///
/// \param pendAddSpec  Pointer to Pending Addr
/// \param sa  Short Address
///
/// \return Number of Short and extended addresses in list
inline
UInt8 Mac802_15_4PendAddSpecAddShortAddr(M802_15_4PendAddrSpec* pendAddSpec,
                                         MACADDR sa)
{
    Int32 i = 0;
    if (pendAddSpec->numShortAddr + pendAddSpec->numExtendedAddr >= 7)
    {
        return pendAddSpec->numShortAddr + pendAddSpec->numExtendedAddr;
    }

    // only unique address added
    for (i = 0; i < pendAddSpec->numShortAddr; i++)
    {
        if (pendAddSpec->fields.addrList[i] == sa)
        {
            return pendAddSpec->numShortAddr + pendAddSpec->numExtendedAddr;
        }
    }

    pendAddSpec->fields.addrList[pendAddSpec->numShortAddr] = sa;
    pendAddSpec->numShortAddr++;
    return pendAddSpec->numShortAddr + pendAddSpec->numExtendedAddr;
}

/// Add new extended address to pending list
///
/// \param pendAddSpec  Pointer to Pending Addr
/// \param ea  Extended Address
///
/// \return Number of Short and extended addresses in list
inline
UInt8 Mac802_15_4PendAddSpecAddExtendedAddr(
    M802_15_4PendAddrSpec* pendAddSpec,
    MACADDR ea)
{
    Int32 i = 0;

    if (pendAddSpec->numShortAddr + pendAddSpec->numExtendedAddr >= 7)
    {
        return pendAddSpec->numShortAddr + pendAddSpec->numExtendedAddr;
    }
        // only unique address added
    for (i = 6;i > 6 - pendAddSpec->numExtendedAddr; i--)
    {
        if (pendAddSpec->fields.addrList[i] == ea)
        {
            return pendAddSpec->numShortAddr + pendAddSpec->numExtendedAddr;
        }
    }
        // save the extended address in reverse order
    pendAddSpec->fields.addrList[6 - pendAddSpec->numExtendedAddr] = ea;
    pendAddSpec->numExtendedAddr++;
    return pendAddSpec->numShortAddr + pendAddSpec->numExtendedAddr;
}

/// Realign Pending Address Specification
///
/// \param pendAddSpec  Pointer to Pending Addr
///
inline
void Mac802_15_4PendAddSpecFormat(M802_15_4PendAddrSpec* pendAddSpec)
{
    Int32 i = 0;
    MACADDR tmpAddr;

    // restore the order of extended addresses
    for (i = 0; i < pendAddSpec->numExtendedAddr; i++)
    {
        if ((7 - pendAddSpec->numExtendedAddr + i) < (6 - i))
        {
            tmpAddr
                = pendAddSpec->fields.addrList[7 -
                                        pendAddSpec->numExtendedAddr + i];
            pendAddSpec->fields.addrList[7 - pendAddSpec->numExtendedAddr + i]
                = pendAddSpec->fields.addrList[6 - i];
            pendAddSpec->fields.addrList[6 - i] = tmpAddr;
        }
    }

    // attach the extended addresses to short addresses
    for (i = 0; i < pendAddSpec->numExtendedAddr; i++)
    {
        pendAddSpec->fields.addrList[pendAddSpec->numShortAddr + i]
        = pendAddSpec->fields.addrList[7 - pendAddSpec->numExtendedAddr + i];
    }

    // update address specification
    pendAddSpec->fields.spec
        = ((pendAddSpec->numShortAddr) << 5)
            + (pendAddSpec->numExtendedAddr << 1);
}

/// Parsing the Pending address spec
///
/// \param pendAddSpec  Pointer to Pending Addr
///
inline
void Mac802_15_4PendAddSpecParse(M802_15_4PendAddrSpec* pendAddSpec)
{
    pendAddSpec->numShortAddr = (pendAddSpec->fields.spec & 0xe0) >> 5;
    pendAddSpec->numExtendedAddr = (pendAddSpec->fields.spec & 0x0e) >> 1;
}

#if 0
/// Add Pending Address spec size
///
/// \param pendAddSpec  Pointer to Pending Addr
inline
Int32 Mac802_15_4PendAddSpecSize(M802_15_4PendAddrSpec* pendAddSpec)
{
    Mac802_15_4PendAddSpecParse(pendAddSpec);
    return 1 + pendAddSpec->numShortAddr * 2
                + pendAddSpec->numExtendedAddr * 8;
}
#endif

/// Parses Device capabilities
///
/// \param devCap  Pointer to Pending Addr
///
inline
void Mac802_15_4DevCapParse(M802_15_4DevCapability* devCap)
{
    devCap->alterPANCoor = ((devCap->cap & 0x80) != 0)?TRUE:FALSE;
    devCap->FFD = ((devCap->cap & 0x40) != 0)?TRUE:FALSE;
    devCap->mainsPower = ((devCap->cap & 0x20) != 0)?TRUE:FALSE;
    devCap->recvOnWhenIdle = ((devCap->cap & 0x10) != 0)?TRUE:FALSE;
    devCap->secuCapable = ((devCap->cap & 0x02) != 0)?TRUE:FALSE;
    devCap->alloShortAddr = ((devCap->cap & 0x01) != 0)?TRUE:FALSE;
}

#if 0
/// Sets whether alternate PAN coordinator
///
/// \param devCap  Pointer to Pending Addr
/// \param alterPC  Alternate PAN Coordinator
///
inline
void Mac802_15_4DevCapSetAlterPANCoor(M802_15_4DevCapability* devCap,
                                      BOOL alterPC)
{
    devCap->alterPANCoor = alterPC;
    devCap->cap = (devCap->cap & 0x7f);
    if (alterPC == TRUE)
    {
        devCap->cap += 0x80;
    }
}

/// Sets whether Full function device
///
/// \param devCap  Pointer to Pending Addr
/// \param ffd  FFD
///
inline
void Mac802_15_4DevCapSetFFD(M802_15_4DevCapability* devCap, BOOL ffd)
{
    devCap->FFD = ffd;
    devCap->cap = (devCap->cap & 0xbf);
    if (ffd == TRUE)
    {
        devCap->cap += 0x40;
    }
}

/// Sets whether on main power (or battery)
///
/// \param devCap  Pointer to Pending Addr
/// \param mainsPowered  Main power
///
inline
void Mac802_15_4DevCapSetMainPower(M802_15_4DevCapability* devCap,
                                   BOOL mainsPowered)
{
    devCap->mainsPower = mainsPowered;
    devCap->cap = (devCap->cap & 0xdf);
    if (mainsPowered == TRUE)
    {
        devCap->cap += 0x20;
    }
}

/// Sets whether device receives on idle
///
/// \param devCap  Pointer to Pending Addr
/// \param onIdle  Recv on idle
///
inline
void Mac802_15_4DevCapSetRecvOnWhenIdle(M802_15_4DevCapability* devCap,
                                        BOOL onIdle)
{
    devCap->recvOnWhenIdle = onIdle;
    devCap->cap = (devCap->cap & 0xef);
    if (onIdle == TRUE)
    {
        devCap->cap += 0x10;
    }
}

/// Sets whether security capabilities are enabled
///
/// \param devCap  Pointer to Pending Addr
/// \param securityEnable  Security capability
///
inline
void Mac802_15_4DevCapSetSecuCapable(M802_15_4DevCapability* devCap,
                                     BOOL securityEnable)
{
    devCap->secuCapable = securityEnable;
    devCap->cap = (devCap->cap & 0xfd);
    if (securityEnable == TRUE)
    {
        devCap->cap += 0x02;
    }
}

/// Sets whether coordinator should allocate short address
///
/// \param devCap  Pointer to Pending Addr
/// \param alloc  Allocate short address
///
inline
void Mac802_15_4DevCapSetAlloShortAddr(M802_15_4DevCapability* devCap,
                                       BOOL alloc)
{
    devCap->alloShortAddr = alloc;
    devCap->cap = (devCap->cap & 0xfe);
    if (alloc == TRUE)
    {
        devCap->cap += 0x01;
    }
}
#endif
inline
clocktype Mac802_15_4TrxTime(Node* node, Int32 interfaceIndex, Message* msg)
{
    clocktype trxTime = 0;
    trxTime = PHY_GetTransmissionDuration(
                                node,
                                interfaceIndex,
                                M802_15_4_DEFAULT_DATARATE_INDEX,
                                MESSAGE_ReturnPacketSize(msg));
    return trxTime;
}

/// Convert IPv4 addtess to Hardware address
///
/// \param node  Pointer to Node structure
/// \param ipv4Address  IPv4 address
/// \param macAddr  Pointer to Hardware address structure
void MAC802_15_4IPv4AddressToHWAddress(Node* node,
                                       NodeAddress ipv4Address,
                                       MacHWAddress* macAddr);

void Mac802_15_4Dispatch(Node* node,
                         Int32 interfaceIndex,
                         PhyStatusType status,
                         const char* frFunc,
                         PLMEsetTrxState req_state,
                         M802_15_4_enum mStatus);

#endif /*MAC_802_15_4*/
