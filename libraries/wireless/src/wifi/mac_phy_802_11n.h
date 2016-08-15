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

#ifndef mac_phy_802_11n
#define mac_phy_802_11n

// Total possible number of channel bandwidths for 802_11n
#define NUM_CH_BW_802_11N   2

// Total possible number of channel bandwidths for 802_11ac
#define NUM_CH_BW_802_11AC   4

#include "proc-process.h"
#include "proc-datamodel.h"
#include "app_util.h"

using namespace Proc;
namespace Proc
{
    class DataModel;
}


/// \brief Main Stats Controller
///
/// It manages the process to update the
/// statistics for PHY and MAC models
class StatsController : public Process
{
    /// boolean to detect whether the phy send stats are being collected
    bool m_phySendStat;
    /// boolean to detect whether the phy receive stats are being collected
    bool m_phyRecvStat;
    /// boolean to detect whether the mac send stats are being collected
    bool m_macSendStat;
    /// boolean to detect whether the mac receive stats are being collected
    bool m_macRecvStat;
    /// boolean to detect whether the nav stats are being collected
    bool m_navStat;
    /// boolean to detect whether the ifs stats are being collected
    bool m_ifsStat;
    /// boolean to detect whether the process is enabled
    bool m_enabled;

    std::string m_difs;
    std::string m_sifs;

public:
    /*!
    * \brief StatsController Constructor
    */
    StatsController(DataModel& dm)
        : Process(dm)
    {
        m_phySendStat = false;
        m_phyRecvStat = false;
        m_macSendStat = false;
        m_macRecvStat = false;
        m_ifsStat = false;
        m_navStat = false;
        m_enabled = true;
    }
    void setDifs(const std::string& difs)
    {
        m_difs = difs;
    }

    void setSifs(const std::string& sifs)
    {
        m_sifs = sifs;
    }
    const std::string& difs()
    {
        return m_difs;
    }
    const std::string& sifs()
    {
        return m_sifs;
    }

    /*!
    * \brief To get dm
    */
    DataModel& getDM()
    {
        return dm();
    }
    /// \brief Implementation of initialize API
    ///
    /// This function initializes the stats controller
    ///
    void initialize()
    {
        DataModel& dm = getDM();
        std::string tokenStr = dm.readParam("DOT11-DATABASE-CONTROL");

        if (tokenStr.empty())
        {
            return;
        }
        char* inputString = new char[tokenStr.length() +1];
        strcpy(inputString, tokenStr.c_str());

        char* next = NULL;
        char* token = NULL;
        char delims[] = "{,} \n\t";
        char iotoken[MAX_STRING_LENGTH];

        IO_TrimLeft(inputString);
        IO_TrimRight(inputString);
        TruncateSpaces(inputString);
        IO_ConvertStringToUpperCase(inputString);

        token = IO_GetDelimitedToken(iotoken, inputString, delims, &next);

        while (token != NULL)
        {
            if (strcmp(token, "ALL") == 0)
            {
                m_phySendStat = true;
                m_phyRecvStat = true;
                m_macSendStat = true;
                m_macRecvStat = true;
                m_ifsStat = true;
                m_navStat = true;
                break;
            }
            if (strcmp(token, "MAC_RECEIVED") == 0)
            {
                m_macRecvStat = true;
            }
            else if (strcmp(token, "PHY_RECEIVED") == 0)
            {
                m_phyRecvStat = true;
            }
            else if (strcmp(token, "PHY_SENT") == 0)
            {
                m_phySendStat = true;
            }
            else if (strcmp(token, "MAC_SENT") == 0)
            {
                m_macSendStat = true;
            }
            else if (strcmp(token, "NAV") == 0)
            {
                m_navStat = true;
            }
            else if (strcmp(token, "IFS") == 0)
            {
                m_ifsStat = true;
            }
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
        }

    }
    /*!
    * \brief To finalize the process
    */
    void finalize() { ; }

    /*!
    * \brief To enable the process
    */
    void enable() { m_enabled = true; }
    /*!
    * \brief To disable the process
    */
    void disable() { m_enabled = false; }
    /*!
    * \brief To reset the process
    */
    void reset() { disable(); }
    /*!
    * \brief return the enable state of the process
    */
    bool enabled() { return true; }

    void runTimeStat() { ; }

    /// \brief Implementation of createTableSchema API
    ///
    /// This function creates the table schema
    ///
    void createTableSchema()
    {
        DataModel& dmodel = getDM();
        if (m_phySendStat || m_phyRecvStat)
        {
            dmodel.cs("WiFiEvents")("Size", "integer")
                                    ("OverheadSize","integer")
                                    ("SignalPower", "double")
                                    ("Sinr", "double")
                                    ("RSSI", "double")
                                    ("RSNI", "double")
                                    ("Pathloss", "double");

        }
        if (m_macSendStat || m_macRecvStat)
        {
            dmodel.cs("WiFiEvents")("SrcAddress", "varchar(256)")
                                    ("DstAddress", "varchar(256)")
                                    ("Size", "integer")
                                    ("OverheadSize","integer");
        }
        if (m_navStat)
        {
            dmodel.cs("WiFiEvents")("NavEndTime", "clocktype");
        }
        if (m_ifsStat)
        {
            dmodel.cs("WiFiEvents")("IfsState", "varchar(256)");
        }
        dmodel.initialized();
    }


    /// \brief Implementation of updateStat API
    ///
    /// This function update the stats for PHY layer
    ///
    /// \param eventName Name of the event type
    /// \param size Size of the msg
    /// \param overheadSize Size of the header
    /// \param sinr Value of SINR
    /// \param rssi Value of RSSI
    /// \param rsni Value of RSNI
    /// \param pathloss Value of pathloss
    void updateStat(const std::string& eventName,
                    int size,
                    int overheadSize,
                    double signalPower,
                    double sinr = 0,
                    double rssi = 0,
                    double rsni = 0,
                    double pathloss = 0)
    {
        if (!m_enabled)
        {
            return;
        }
        DataModel& dm = getDM();

        if (eventName == "PhySent" && m_phySendStat)
        {
            dm.ds(eventName, now())
                 ("NodeID", nodeId())
                 ("InterfaceIndex", ifidx())
                 ("Size", size)
                 ("OverheadSize", overheadSize)
                 ("SignalPower", signalPower)();
        }
        else if (eventName == "PhyReceived" && m_phyRecvStat)
        {
            dm.ds(eventName, now())
                 ("NodeID", nodeId())
                 ("InterfaceIndex", ifidx())
                 ("Size", size)
                 ( "OverheadSize", overheadSize)
                 ("SignalPower", signalPower)
                 ("Sinr", sinr)
                 ("RSSI", rssi)
                 ("RSNI", rsni)
                 ("Pathloss", pathloss)();
        }

    }

    /// \brief Implementation of updateStat API
    ///
    /// This function update the stats for MAC layer
    ///
    /// \param eventName Name of the event type
    /// \param srcAddress Address of the source node
    /// \param size Size of the msg
    /// \param overheadSize Size of the header
    void updateStat(const std::string& eventName,
                    int size,
                    int overheadSize,
                    const std::string& address)
    {
        if (!m_enabled)
        {
            return;
        }
        DataModel& dm = getDM();

        if (eventName == "MacSent" && m_macSendStat)
        {
            dm.ds(eventName, now())
                 ("NodeID", nodeId())
                 ("InterfaceIndex", ifidx())
                 ("DstAddress", address)
                 ("Size", size)
                 ("OverheadSize", overheadSize)();
        }
        else if (eventName == "MacReceived" && m_macRecvStat)
        {
            dm.ds(eventName, now())
                 ("NodeID", nodeId())
                 ("InterfaceIndex", ifidx())
                 ("SrcAddress", address)
                 ("Size", size)
                 ("OverheadSize", overheadSize)();
        }

    }


    /// \brief Implementation of updateStat API
    ///
    /// This function update the stats for NAV management
    ///
    /// \param eventName Name of the event type
    /// \param navTime Value of nav end time
    void updateStat(const std::string& eventName, clocktype navEndTime)
    {
        if (!(m_enabled && m_navStat))
        {
            return;
        }
        DataModel& dm = getDM();

        if (eventName == "NavEnd")
        {
            dm.ds(eventName, now())
                 ("NodeID", nodeId())
                 ("InterfaceIndex", ifidx());
        }
        else
        {
            dm.ds(eventName, now())
                 ("NodeID", nodeId())
                 ("InterfaceIndex", ifidx())
                 ("NavEndTime", navEndTime)();
        }
    }
    void updateStat(const std::string& eventName,const std::string& ifs)
    {
        if (!(m_enabled && m_ifsStat))
        {
            return;
        }
        DataModel& dm = getDM();

        dm.ds(eventName, now())
             ("NodeID", nodeId())
             ("InterfaceIndex", ifidx())
             ("IfsState", ifs)();
    }
};

///** Station capability mode
// *  The enum types for station capabilities
// */
enum STA_Capability_Mode
{
    k_Dot11a,
    k_Dot11b,
    k_Dot11n = 19,
    k_Dot11ac
};

///** Mode
// *  The enum types for 802.11n HT operation modes and PPDU formats
// */
enum Mode {
    MODE_NON_HT,         // NON HT
    MODE_HT_MF,          // mixed mode
    MODE_HT_GF,          // green field mode
    MODE_VHT,           // very high throughput mode
};

///** ChBandwidth
//* The enum types for 802.11n channel bandwidth
//*/
enum ChBandwidth {
    CHBWDTH_10MHZ = 0,
    CHBWDTH_20MHZ,
    CHBWDTH_40MHZ,
    CHBWDTH_80MHZ,
    CHBWDTH_160MHZ,
};

///** GI
//* The enum types for 802.11n guard interval
//*/
enum GI {
    GI_LONG,             // Default 800ns GI
    GI_SHORT,            // 400ns GI
    GI_NUMS              // Total number of guard intervals
};

//** Non_HT_Modulation
//* The enum types for 802.11 modulation types
//*/
enum Non_HT_Modulation
{
    OFDM,
    NON_HT_DUP_OFDM
};

//** Dynamic_Bwdth_Non_HT
//* The enum types for 802.11 bandwidth modes
//*/
enum Dynamic_Bwdth_Non_HT
{
    STATIC,
    DYNAMIC
};

//** RfChainMode
//* The enum types for number of antennas
//*/
enum RfChainMode
{
    k_Single_Rf_Chain = 0,
    k_All_Rf_Chain = 1
};

//** SmMode
//* The enum types for SMPS mode of operation
//*/
enum SmMode
{
    k_Static = 0,
    k_Dynamic = 1,
    k_Reserved = 2,
    k_Disabled = 3
};


///**MAC_PHY_TxRxVector
// * MAC_PHY_TxRxVector defines per-packet transmit parameters supplied by 802.11 MAC
// */
typedef struct MAC_PHY_TxRxVector{
    Mode                format;                 // Transmit format
    Non_HT_Modulation   nonHTMod;
    ChBandwidth         chBwdthNonHt;
    Dynamic_Bwdth_Non_HT dBwdthNonHT;
    ChBandwidth         chBwdth;                // Channel bandwidth
    size_t              length;                 // PSDU length, which may not be the same
                                                // as packetSize as padding may be added.
    BOOL                sounding;               // Whether a sounding packet
    BOOL                containAMPDU;           // Whether contains a A-MPDU
    GI                  gi;                     // 800ns or 400ns GI
    unsigned char       mcs;                    // MCS index
    unsigned char       phyType;                // Type of PHY
    unsigned char       numEss;  // Number of extension spatial streams
    // For future use
    // BOOL                 txop_ps_not_allowed;
    // constuctor
    MAC_PHY_TxRxVector()
    {
        format = MODE_NON_HT;
        nonHTMod = OFDM;
        chBwdthNonHt = CHBWDTH_20MHZ;
        dBwdthNonHT = STATIC;
        chBwdth = CHBWDTH_20MHZ;
        length = 0;
        sounding = FALSE;
        containAMPDU = FALSE;
        gi = GI_LONG;
        mcs = 0;
        phyType = 0;
        numEss = 0;
        // For future use
        // txop_ps_not_allowed = TRUE;
    }
}MAC_PHY_TxRxVector;

#endif
