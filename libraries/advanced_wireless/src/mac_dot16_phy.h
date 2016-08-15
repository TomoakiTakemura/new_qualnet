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

#ifndef MAC_DOT16_PHY_H
#define MAC_DOT16_PHY_H

//
// This is the header file of the 802.16 PHY specific part
//

/// Statistics of the 802.16 PHY sublayer
typedef struct
{
    int numFramesSent;    // # of MAC frames sent
    int numFramesRcvd;    // # of MAC frames received
} MacDot16PhyStats;

/// Turn around time of the wireles radio
#define DOT16_PHY_RADIO_TURNAROUND_TIME    (100 * NANO_SECOND)

//--------------------------------------------------------------------------
// OFDMA PHY related constants, enums, and structures
//--------------------------------------------------------------------------

// DESCRPTION  :: Number of OFDM symbols per physical slot on donwlink.
#define DOT16_OFDMA_NUM_SYMBOLS_PER_DL_PS    2

// DESCRPTION  :: Number of OFDM symbols per physical slot on uplink.
#define DOT16_OFDMA_NUM_SYMBOLS_PER_UL_PS    3

// DESCRPTION  :: 802.16 OFDMA support 16 DIUC currently
#define DOT16_OFDMA_NUM_DIUC    16

// DESCRPTION  :: 802.16 OFDMA support 16 UIUC currently
#define DOT16_OFDMA_NUM_UIUC    16

/// 802.16 OFDMA support 256 ranging code
#define DOT16_OFDMA_NUM_CDMA_CODE    256

/// The first N codes starting from S (subgroup number) to
/// S + N-1 for initial ranging
#define DOT16_OFDMA_DEFAULT_NUM_INITIAL_RANGING_CODE 16

/// The first N codes starting from S + N (subgroup number)
/// to S + N + M -1 for periodic ranging
#define DOT16_OFDMA_DEFAULT_NUM_PERIODIC_RANGING_CODE 16

/// The first N codes starting from S + M + N
/// (subgroup number) to S + M+ N + L -1 for initial ranging
#define DOT16_OFDMA_DEFAULT_NUM_BANDWIDTH_RQUEST_CODE 16

/// Types of OFDMA CDMA codes
typedef enum
{
    DOT16_OFDMA_INITIAL_RANGING_CODE    =    0,
    DOT16_OFDMA_PERIODIC_RANGING_CODE   =    1,
    DOT16_OFDMA_BANDWIDTH_REQUEST_CODE  =    2
}MacDot16OfdmaCdmaCodeType;

/// 802.16 OFDMA supports 9 frame durations currently.
#define DOT16_OFDMA_NUM_FRAME_DURATION    9

/// Preable of the OFDMA PHY is 1 OFDMA symbol
#define DOT16_OFDMA_PREAMBLE_SYMBOL_LENGTH    1

/// PHY syncrhonization field of OFDMA PHY
/// To eliminate by alignment problem, the order of fields
/// is slightly adjusted, which won't affect sim results.
typedef struct
{
    unsigned char preamble;
    unsigned char usedSubchannelMap: 6,
                  repCodingIndication: 2;
    unsigned char rangeChangeIndication: 1,
                  codingIndication: 3,
                  reserved: 4;
    unsigned char dlMapLength;
} MacDot16PhyOfdmaFch;

/// PHY syncrhonization field of OFDMA PHY
typedef struct
{
    unsigned int durationCode:8,    // frame duration code
                 frameNumber:24;    // frame number, modulo 2^24
} MacDot16PhyOfdmaSyncField;

/// Data structure of OFMDA DL-MAP_IE
typedef struct
{
    unsigned char diuc: 4, // DIUC used in the DL burst
              padding: 4;
    unsigned int ofdmaSymbolOffset: 8, // OFDMA symbol offset
                  subchannelOffset: 6, // sunchannel offset
                          boosting: 3, // power boost, 000: normal
                   numOfdmaSymbols: 7, // number of OFDMA  symbols
                    numSubchannels: 6, // number of subchannels
               repCodingIndication: 2; // repeatition code


} MacDot16PhyOfdmaDlMapIE;

/// Data structure of OFMDA UL-MAP_IE
/// This is only for normal UL-MAP_IE whose UIUC
/// is not one of 12, 14, and 15
typedef struct
{
    unsigned int cid: 16,                 // CID that the IE assigned to
                 uiuc: 4,                 // UIUC used for the burst
                 duration: 10,            // duration of the burst
                 repCodingIndication: 2;  // Indicates the repetition code
} MacDot16PhyOfdmaUlMapIE;

typedef struct
{
    Dot16CIDType cid;                 // CID that the IE assigned to
    unsigned char uiuc: 4,                 // UIUC used for the burst
                  paddingNibble: 4;

    unsigned int ofmaSynbolOffset:8,      // Ofdma synbol number
                 subchannelOffset:7,      // OFDMA symbol subchannel
                 noOfOfdmaSymbols:7,      // number of OFDMA symbols
                 noOfSubchannels:7,          // number of subchannels
                 rangingMethod:2,          // the ranging method type
                 rangingIndicator: 1;       // Dedicated ranging indicator
} MacDot16PhyOfdmaUlMapIEuiuc12;

typedef struct
{
    Dot16CIDType cid;                 // CID that the IE assigned to
    unsigned short int uiuc: 4,                 // UIUC used for the burst
                       duration: 6,             // duration of the burst
                       uiuc_Transmission: 4,    // UIUC for transmission
                       repCodingIndication: 2;  // Indicates the repetition code
    unsigned char frameNumber: 4, // LSBs of relevant frame number
                  paddingNibble: 4;
    unsigned char rangingCode; // Ranging Code 8 bits
    unsigned char rangingSymbol; // Ranging Symbol 8 bits
    unsigned char rangingSubchannel: 7, // Ranging subchannel 7 bits
                  bwRequest: 1; // BW request mandatory 1 bit 1= yes, 0= no
} MacDot16PhyOfdmaUlMapIEuiuc14;

typedef union
{
    MacDot16PhyOfdmaUlMapIE ulMapIE;
    MacDot16PhyOfdmaUlMapIEuiuc12 ulMapIE12;
    MacDot16PhyOfdmaUlMapIEuiuc14 ulMapIE14;
} MacDot16GenericPhyOfdmaUlMapIE;
/// Data structure of Dot16 OFDMA PHY sublayer
typedef struct struct_mac_dot16_phy_ofdma_str
{
    int frameNumber;    // frame number
    int numSubchannels; // # of sub-channels

    // frame duration code
    clocktype frameDurationList[DOT16_OFDMA_NUM_FRAME_DURATION];
    unsigned char frameDurationCode;

    MacDot16PhyStats stats;
} MacDot16PhyOfdma;

//--------------------------------------------------------------------------
//  API functions
//--------------------------------------------------------------------------
/// Add the PHY synchronization field
///
/// \param node  Pointer to node
/// \param dot16  Pointer to 802.16 data structure
/// \param macFrame  Pointer to the MAC frame
/// \param startIndex  starting position in MAC frame
/// \param frameLengh  total length of the DL part of the frame
///
/// \return Number of bytes added
int MacDot16PhyAddPhySyncField(Node* node,
                               MacDataDot16* dot16,
                               unsigned char* macFrame,
                               int startIndex,
                               int frameLength);


/// Get frame number from PHY synchronization field
///
/// \param node  Pointer to node
/// \param dot16  Pointer to 802.16 data structure
/// \param phySyncField  Pointer to PHY sync field
///
/// \return return frame number in PHY sync field
int MacDot16PhyGetFrameNumber(Node* node,
                                      MacDataDot16* dot16,
                                      unsigned char* macFrame);

/// Get frame duration from PHY synchronization field
///
/// \param node  Pointer to node
/// \param dot16  Pointer to 802.16 data structure
/// \param phySyncField  Pointer to PHY sync field
///
/// \return return frame duration in PHY sync field
clocktype MacDot16PhyGetFrameDuration(Node* node,
                                MacDataDot16* dot16,
                                unsigned char* macFrame);

/// Set and validate the frame duration according to PHY type
///
/// \param node  Pointer to node
/// \param dot16  Pointer to 802.16 data structure
/// \param frameDuration  Duration of the frame
///
void MacDot16PhySetFrameDuration(Node* node,
                                 MacDataDot16* dot16,
                                 clocktype frameDuration);
/// Add DL MAP IE for one  burst
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 data struct
/// \param macFrame  Pointer to the MAC frame
/// \param dlMapIEIndex  Index of the DL MAP IE in DL MAP
///    + cid       :                : CID for this  burst
///    + diuc      :                : diuc used in the busrt
/// \param burstInfo  busrt info
///
/// \return Number of bytes added
int MacDot16PhyAddDlMapIE(Node* node,
                          MacDataDot16* dot16,
                          unsigned char* macFrame,
                          int startIndex,
                          Dot16CIDType cid,
                          unsigned char diuc,
                          Dot16BurstInfo burstInfo);

/// Get one DL-MAP_IE
///
/// \param node  Pointer to node
/// \param dot16  Pointer to 802.16 data structure
/// \param macFrame  Pointer to the MAC frame
/// \param startIndex  starting position in MAC frame
/// \param cid  CID that the IE assigned to
/// \param diuc  DIUC value
/// \param burstInfo  Pointer to the burst Info
///
/// \return Number of bytes processed
int MacDot16PhyGetDlMapIE(Node* node,
                          MacDataDot16* dot16,
                          unsigned char* macFrame,
                          int startIndex,
                          Dot16CIDType* cid,
                          unsigned char* diuc,
                          Dot16BurstInfo* burstInfo);

/// Add one UL-MAP_IE
///
/// \param node  Pointer to node
/// \param dot16  Pointer to 802.16 data structure
/// \param macFrame  Pointer to the MAC frame
/// \param startIndex  starting position in MAC frame
/// \param frameLengh  total length of the DL part of the frame
/// \param cid  CID that the IE assigned to
/// \param uiuc  UIUC value
/// \param duration  Duration of the burst
///
/// \return Number of bytes added
int MacDot16PhyAddUlMapIE(Node* node,
                          MacDataDot16* dot16,
                          unsigned char* macFrame,
                          int startIndex,
                          int frameLength,
                          Dot16CIDType cid,
                          unsigned char uiuc,
                          int duration,
                          unsigned char flag
                          = DOT16_CDMA_INITIAL_RANGING_OVER_2SYMBOL);

/// Get one UL-MAP_IE
///
/// \param node  Pointer to node
/// \param dot16  Pointer to 802.16 data structure
/// \param macFrame  Pointer to the MAC frame
/// \param startIndex  starting position in MAC frame
/// \param cid  CID that the IE assigned to
/// \param uiuc  UIUC value
/// \param duration  Duration of the burst
///
/// \return Number of bytes processed
int MacDot16PhyGetUlMapIE(Node* node,
                          MacDataDot16* dot16,
                          unsigned char* macFrame,
                          int startIndex,
                          Dot16CIDType* cid,
                          unsigned char* uiuc,
                          clocktype* duration,
                          MacDot16GenericPhyOfdmaUlMapIE* cdmaInfo);

/// Transmit a MAC frame
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param msg  Packet received from PHY
/// \param dlMapLength  Length of the DL-MAP
///
void MacDot16PhyTransmitMacFrame(Node* node,
                                 MacDataDot16* dot16,
                                 Message* msg,
                                 unsigned char dlMapLength,
                                 clocktype duration);

/// Transmit a Uplink burst
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param msg  Packet received from PHY
/// \param channelIndex  Channel for transmission
/// \param delayInPs  Delay in PS for transmitting the
///    burst
///
void MacDot16PhyTransmitUlBurst(Node* node,
                                MacDataDot16* dot16,
                                Message* msg,
                                int channelIndex,
                                int delayInPs);


/// Initialize 802.16 PHY sublayer
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
/// \param nodeInput  Pointer to node input.
///
void MacDot16PhyInit(Node* node,
                     int interfaceIndex,
                     const NodeInput* nodeInput);

/// Print stats and clear protocol variables.
///
/// \param node  Pointer to node.
/// \param interfaceIndex  Interface index
///
void MacDot16PhyFinalize(Node *node, int interfaceIndex);

/// Receive a packet from physical layer.
/// The PHY sublayer will first handle it
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param msg  Packet received from PHY
///
void MacDot16PhyReceivePacketFromPhy(Node* node,
                                     MacDataDot16* dot16,
                                     Message* msg);

/// Get the additional overhead that PHY adds to the beginning
/// of each MAC frame in terms of number of bytes
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
///
/// \return PHY overhead in terms of number of bytes
int MacDot16PhyGetFrameOverhead(Node* node,
                                MacDataDot16* dot16);

/// Get the duration of a physical slot.
/// The physical slot duration is different for
/// different 802.16 PHY models. It may be different for
/// DL and UL.
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param subframeType  Indicate DL or UL
///
/// \return Duration of a physical slot.
clocktype MacDot16PhyGetPsDuration(Node* node,
                                   MacDataDot16* dot16,
                                   MacDot16SubframeType subframeType);

/// Get the number of Bits per PS given the burst profile
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param burstProfile  Point to burst profile
/// \param subframeType  Indicate DL or UL
///
/// \return # of bits per Ps
int MacDot16PhyBitsPerPs(Node* node,
                         MacDataDot16* dot16,
                         void* burstProfile,
                         MacDot16SubframeType subframeType);

/// Get the number of physical slots needed to transmit
/// the given number of bytes
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
/// \param size  Size in bytes
/// \param burstProfile  Point to the burst profile
/// \param subframeType  Indicate DL or UL
///
/// \return # of physical slots needed
int MacDot16PhyBytesToPs(Node* node,
                         MacDataDot16* dot16,
                         int size,
                         void* burstProfile,
                         MacDot16SubframeType subframeType);

/// Return # of OFDM symbols per PS
///
/// \param subframeType  Indicate DL or UL
///
/// \return # of OFMD symbols
inline
int MacDot16PhySymbolsPerPs(MacDot16SubframeType subframeType)
{
    if (subframeType == DOT16_DL)
    {
        return DOT16_OFDMA_NUM_SYMBOLS_PER_DL_PS;
    }
    else
    {
        return DOT16_OFDMA_NUM_SYMBOLS_PER_UL_PS;
    }
}

/// Get number of subchannels supported by PHY layer
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
///
/// \return Number of subcahnnels supported
int MacDot16PhyGetNumSubchannels(Node* node,
                                 MacDataDot16* dot16,
                                 MacDot16SubframeType subChannelType
                                 = DOT16_DL);

/// Get uplink number of subchannels supported by PHY layer
///
/// \param node  Pointer to node.
/// \param dot16  Pointer to DOT16 structure
///
/// \return Number of subcahnnels supported
int MacDot16PhyGetUplinkNumSubchannels(Node* node, MacDataDot16* dot16);

#endif // MAC_DOT16_PHY_H
