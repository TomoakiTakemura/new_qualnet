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

#ifndef PHY_RXMODEL_LTE_H
#define PHY_RXMODEL_LTE_H

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <numeric>
#include <assert.h>
#include "api.h"
#include "lte_common.h"

class HarqPhyLteReceiveBuffer;

#define LTE_BLER_DIR "/data/modulation/lte/bler/"
#define LTE_PROF_DIR "/data/modulation/lte/prof/"
#define LTE_MIB_DIR  "/data/modulation/lte/mib/"
#define LTE_ECR_PROF_FILE_NAME "ecr_prof.csv"
#define LTE_CBS_PROF_FILE_NAME "cbs_prof.csv"
#define LTE_REPETITION_PROF_FILE_NAME "repetition_prof.csv"
#define LTE_BLER_FILE_NAME "%s_MCS%02d_NRB010.bler"
#define LOAD_BLER_FROM_LOCAL (0)

///////////////////////////////////////////////////////////////
// Structs
///////////////////////////////////////////////////////////////

/// SNR - MIB curve element
struct PhyLteMibEntry
{
    double snr_dB;
    double avgMib;
    int numCh;
    double* chMib;
};

/// Predicate for comparison of average MIB in PhyLteMibEntry
struct PhyLtePredAvgMib
{
    bool operator()(const PhyLteMibEntry& l, double rmib) const { return l.avgMib < rmib; }
    bool operator()(double lmib, const PhyLteMibEntry& r) const { return lmib  < r.avgMib; }
    bool operator()(const PhyLteMibEntry& l, const PhyLteMibEntry& r)  const { return l.avgMib < r.avgMib; }
};

/// Predicate for comparison of channel MIB in PhyLteMibEntry
struct PhyLtePredChMib
{
    int _chNo;
    PhyLtePredChMib(int chNo){ _chNo = chNo; }
    bool operator()(const PhyLteMibEntry& l, double rmib) const { return l.chMib[_chNo] < rmib; }
    bool operator()(double lmib, const PhyLteMibEntry& r) const { return lmib  < r.chMib[_chNo]; }
    bool operator()(const PhyLteMibEntry& l, const PhyLteMibEntry& r)  const { return l.chMib[_chNo] < r.chMib[_chNo]; }
};

/// Repetition factor data set element
struct RepetitionFactorElem
{
    int _mcsIndex;
    int _numRb;
    int _rvidx;
    std::vector<int> _s; // Set of numTx of bits
    double _repetitionFactor;

    std::string dump()
    {
        std::stringstream ss;
        ss << "mcsIndex," << _mcsIndex << ","
           << "numRb," << _numRb << ","
           << "rvidx," << _rvidx << ","
           << "s," << _s[0] << "," << _s[1] << "," << _s[2] << "," << _s[3] << ","
           << "rf," << _repetitionFactor;
        return ss.str();
    }
};

///////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////
//
/// SNR-MIB curve class
class PhyLteMibCurve
{
    std::vector<PhyLteMibEntry> _mibArray;
    std::vector<PhyLteMibEntry> _invMibArray;

public:
    PhyLteMibCurve(const char* fn, int modType)
    {
        loadMib(fn, modType);
    }

    ~PhyLteMibCurve();

    int getNumCh() const
    {
        if (_mibArray.size() == 0)
        {
            ERROR_ReportError("Failed to load MIB file");
        }
        return _mibArray[0].numCh;
    }

    double lookup(double snr_dB, int chNo) const;
    double ilookup(double mi, int chNo) const;

protected:
    void loadMib(const char* fn, int modType);

    inline double lininterp(double left, double right, double relp) const
    {
        return ((left) + ((right) - (left))*(relp));
    }

#ifdef LTE_LIB_LOG
    void Dump();
#endif

};

/// Managing SNR-MIB curves of QPSK, QAM16, QAM64
class PhyLteMibCurveDB
{
    PhyLteMibCurve* _qpsk;
    PhyLteMibCurve* _qam16;
    PhyLteMibCurve* _qam64;
public:
    static PhyLteMibCurveDB& getInstance();
    PhyLteMibCurve* getCurve(int modType);
private:
    PhyLteMibCurveDB();
    virtual ~PhyLteMibCurveDB();
};

/// Managing repetition factor data set
class PhyLteRepetitionFactorDB
{
    PhyLteRepetitionFactorDB();
    std::vector< std::vector< RepetitionFactorElem > > _repetitionFactorListDL;
    std::vector< std::vector< RepetitionFactorElem > > _repetitionFactorListUL;
public:
    static PhyLteRepetitionFactorDB& getInstance();
    virtual ~PhyLteRepetitionFactorDB();

    const std::vector<RepetitionFactorElem>&
    getRepetitionFactorProfile(bool isDl, int mcsIndex, int numRb, int txNo);
};

/// Managing ECR data set
class PhyLteEcrDB
{
    PhyLteEcrDB();
    std::vector< double > _ecrProfDL;
    std::vector< double > _ecrProfUL;
public:
    static PhyLteEcrDB& getInstance();
    virtual ~PhyLteEcrDB();

    void init();

    double
    getEcr(bool isDl, int mcsIndex, int numRb, int txNo);
};

/// Managing CBS(Code Block Segmentation) data set
class PhyLteCbsDB
{
    PhyLteCbsDB();
    std::vector<int> _cbsProfDL;
    std::vector<int> _cbsProfUL;
public:
    static PhyLteCbsDB& getInstance();
    virtual ~PhyLteCbsDB();

    void init();

    int getNumCodeBlocks(bool isDl, int mcsIndex, int numRb);
};

///////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////

/// Calculate combined MIB
///
/// \param snrs  Array of SNR
/// \param channels  Array of channel numbers
///
/// \return  Combined MIB value
///
double PhyLteCalcCombinedMIB(
    const PhyLteMibCurve& bpskMib, const PhyLteMibCurve& mibCurve,
    const std::vector<double>& snrs,
    const std::vector<int>& channels);

/// Lookup MIB value from snr[dB] for BPSK
/// using approximated function of SNR-MIB curve.
///
/// \param snr_nondB  SNR
///
/// \return  MIB value
///
double PhyLteRxModelCaclJ(double snr_nondB);

/// Lookup snr value from MIB for BPSK
/// using approximated function of SNR-MIB curve.
///
/// \param mib  MIB
///
/// \return  SNR (non dB)
///
double PhyLteRxModelCalcInvJ(double mib);

/// Calculate G function
///
/// \param v  Arguments of G function
/// \param _0  Start point of valid argument in v
///
/// \return  G(v)
double PhyLteRxModelCalcG(const std::vector<double>& v, int _0 = 0);

/// Calculation of MIB in system level simulation assumption
///
/// \param isDl   true if DL, otherwise UL
/// \param mcsIndex  MCS index ( 0 ... 28 )
/// \param numRb  Number of RB
/// \param mibCUrve  SNR-MIB curve
/// \param txNo  Transmission number ( 0:1st tx, 1: 1st retx, 2: 2nd retx ... )
/// \param snrdB_list List of list of SNRs
///
/// \return  MIB value
double PhyLteCalcAvgMibSL(
    bool isDl, int mcsIndex, int numRb,
    const PhyLteMibCurve& mibCurve,
    int txNo,
    const std::vector< std::vector<double> >& snrdB_list);

/// Load ECR profile from file
///
/// \param fn  Path for file
/// \param rf  Storage for loaded ECR
/// \return  MIB value
void LoadEcrProfile(const char* fn, std::vector< double >& rf);



/// Get number of code blocks in transport
/// block defined for specified MCS and #RB
///
/// \param isDL  true if DL, otherwise false
/// \param mcsIndex  MCS index
/// \param numRb  Number of RBs
/// \return  Number of code blocks
int PhyLteGetNumCodeBlocks(bool isDL, int mcsIndex, int numRb);

#endif

