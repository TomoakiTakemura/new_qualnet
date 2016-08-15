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


#ifndef PHY_DOT11A_H
#define PHY_DOT11A_H

#include "mac_dot11_channelmanager.h"
#include "phy_802_11.h"
#include "phy_802_11_manager.h"



namespace Dot11
{

namespace Qos
{

/*!
 * \brief 802.11a PHY class
 */
class Phy802_11a : public Phy802_11Manager
{
public:
    /*!
     * \brief Parameter to enable/disable Legacy 802.11b calculations
     */
    BOOL useLegacy802_11b;

    /*!
     * \brief Initializes 802_11a class
     */
    Phy802_11a(PhyData802_11* parentData,
          Node* node,
          int interfaceIndex,
          const NodeInput *nodeInput):Phy802_11Manager(MODE_NON_HT,
                                            parentData,
                                            new Dot11::Qos::ChannelController,
                                            node,
                                            interfaceIndex,
                                            nodeInput,
                                            k_Mode_Legacy)
    {
        if (parentData->thisPhy->phyModel == PHY802_11a)
        {
            initDefaultParams_a(nodeInput);
            readCfgParamsForBackwardCompatibility_a(nodeInput);
            readCfgParams_a(nodeInput);
        }

        else if (parentData->thisPhy->phyModel == PHY802_11b)
        {
            initDefaultParams_b(nodeInput);
            readCfgParamsForBackwardCompatibility_b(nodeInput);
            readCfgParams_b(nodeInput);
        }
        else if (parentData->thisPhy->phyModel == PHY802_11pCCH
            || parentData->thisPhy->phyModel == PHY802_11pSCH)
        {
            initDefaultParams_p();
            readCfgParams_p(nodeInput);
        }
        init(nodeInput);
    }


    ~Phy802_11a() { }

    /*!
     * \brief Initialization function for PHY 802_11a
     */
    void init(const NodeInput* nodeInput);

    /*!
     * \brief Initialization function for PHY 802_11a
     */
    void readBwdthParams(const NodeInput* nodeInput);

    /*!
     * \brief Finalization function for PHY 802_11a
     */
    void finalize(Node* node, int phyIndex);

    /*!
     * \brief To set user configurable parameters of 802.11a and 802.11b
     */
    void readCfgParams(const NodeInput *nodeInput);

    /*!
     * \brief To initialize 802.11a default parameters
     */
    void initDefaultParams_a(const NodeInput* nodeInput);
    
    /*!
     * \brief To set user configurable parameters of 802.11a
     */
    void readCfgParams_a(const NodeInput *nodeInput);

    /*!
     * \brief To set user configurable parameters of 802.11a for
     * backward compatibility.
     */
    void readCfgParamsForBackwardCompatibility_a(
                                          const NodeInput *nodeInput);
    
    /*!
     * \brief To initialize 802.11b default parameters
     */
    void initDefaultParams_b(const NodeInput* nodeInput);
    
    /*!
    * \brief To set user configurable parameters of 802.11b
    * for backward compatibility.
    */
    void readCfgParamsForBackwardCompatibility_b(
                                        const NodeInput *nodeInput);
    
    /*!
     * \brief To set user configurable parameters of 802.11b
     */
    void readCfgParams_b(const NodeInput *nodeInput);

    /*!
     * \brief Handles the signal arrival event
     */
    void signalArrival(Node* node,
                       int phyIndex,
                       int channelIndex,
                       PropRxInfo *propRxInfo);

    /*!
     * \brief Handles the signal end event
     */
    void signalEnd(Node* node,
                   int phyIndex,
                   int channelIndex,
                   PropRxInfo *propRxInfo);

    /*!
     * \brief Checks whether phy is able to sense signal present
     * in medium or not
     */
    BOOL carrierSensing(BOOL isSigEnd, int phyIndex);

    /*!
     * \brief Checks whether to process signal or not
     */
    BOOL processSignal(int phyIndex);

    BOOL processSignal(int phyIndex,
                       PropRxInfo* propRxInfo,
                       double rxPower_dBm)
    {
        ERROR_ReportError("Invalid API call");
        return 0;
    }

    /*!
     * \brief Returns default operational mode
     */
    Mode getDefaultMode()
    {
        ERROR_ReportError("Invalid API call");
        return MODE_HT_GF;
    }

    /*!
     * \brief Function for Clear channel assessment
     */
    ChBandwidth cca(ChBandwidth chBwdth)
    {
        ERROR_ReportError("Invalid API call");
        return CHBWDTH_20MHZ;
    }

    /*!
     * \brief Function to get sensitivity value based
     * on configured channel bandwidth
     */
    double getSensitivity_dBm()
    {
        ERROR_ReportError("Invalid API call");
        return 0.0;
    }

    /*!
     * \brief Function to get minimum sensitivity value based
     * on channel bandwidth
     */
    double getMinSensitivity_dBm(ChBandwidth chWidth)
    {
        return m_parentData->rxSensitivity_mW[chWidth];
    }
    /*!
     * \brief Function to initialize 802.11p default parameters
     */
    void initDefaultParams_p();

    /*!
     * \brief Function to set user configurable parameters of 802.11p
     */
    void readCfgParams_p(const NodeInput *nodeInput);

protected:
    /*!
     * \brief Function to check if signal is received with error or not
     */
    BOOL checkPacketError(double *sinrPtr,
                          PropRxInfo* propRxInfo,
                          int phyIndex);

    /*!
     * \brief Function to get the Rx model
     */
    PhyRxModel getRxModel()
    {
        PhyData802_11* phy802_11 = m_parentData;
        PhyRxModel rxModel = RX_802_11b;
        if (phy802_11->thisPhy->phyModel == PHY802_11pCCH
           || phy802_11->thisPhy->phyModel == PHY802_11pSCH)
         {
            // As 802.11p is using 802.11a ber tables
            return RX_802_11a;
         }
        if (phy802_11->thisPhy->phyModel == PHY802_11b)
        {
            rxModel = phy802_11->thisPhy->phyRxModel;
        }
        else
        {
            ERROR_Assert(phy802_11->thisPhy->phyModel == PHY802_11a,
                "Invalid phy type" );
            if (phy802_11->thisPhy->phyModel == PHY802_11a)
            {
                if (phy802_11->rxPhyType == k_Dot11b)
                {
                    rxModel = RX_802_11b;
                }
                else
                {
                    rxModel = RX_802_11a;
                }
            }
            else
            {
                rxModel =  phy802_11->thisPhy->phyRxModel;
            }
        }
        return rxModel;
    }

    /*!
     * \brief Get bit error rate in the received signal
     */
    double getBer(double sinr)
    {
        PhyData802_11* phy802_11 = m_parentData;
        double ber = 0;

        ber = PHY_BER(phy802_11->thisPhy,
                      this->rxDataRateType,
                      sinr,
                      getRxModel());
        return ber;
    }

};
}
}

#endif //PHY_DOT11A_H

