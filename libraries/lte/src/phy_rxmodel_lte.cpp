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

#include "phy_rxmodel_lte.h"
#include "product_info.h"

#ifdef LTE_LIB_LOG
#include "log_lte.h"
#endif

#define PHY_LTE_APPROX_J_FUNC_COEFF (0.7585775750291838) // 10^(-0.12)


//  Generate all the 'dim'-dimensional array of integers,
//  where each integer element j is 0 <= j < numOpt.
class SequenceGenerator
{
    std::vector<UInt64> _buf;
    UInt64 _numOpt;
    UInt64 _v;
    UInt64 _end;
public:

    SequenceGenerator(UInt64 numOpt, UInt64 dim);

    bool eos() const;

    SequenceGenerator& operator++();

    UInt64 operator[](size_t i) const;
};


// Constructor of SequenceGenerator
//
// \param numOpt  Number of options each sequence element takes.
// \param dim  Dimension of sequence
SequenceGenerator::SequenceGenerator(UInt64 numOpt, UInt64 dim)
{
    _buf.resize((size_t)dim, 0);
    _numOpt = numOpt;
    _v = 0;
    _end = 1;
    for (UInt64 i = 0; i < dim; ++i)
        _end *= numOpt;
}

// Get whether all the sequences are generated
//
// \return  true if all the sequences are generated, otherwise false.
//
bool SequenceGenerator::eos() const
{
    return _v >= _end;
}

// Generate next sequence
//
// \return         Reference to myself
//
SequenceGenerator& SequenceGenerator::operator++()
{
    _v++;
    UInt64 i = _v;
    UInt64 dim = _buf.size();

    for (UInt64 j = dim-1; j >= 0; --j)
    {
        UInt64 s = i / _numOpt;
        _buf[(size_t)j] = i - _numOpt * s;
        i = s;
        if (j == 0)
            break;
    }

    return *this;
}

// Get i-th element of current sequence
//
// \param  index of the sequence
//
// \return i-th element of current sequence
//
UInt64 SequenceGenerator::operator[](size_t i) const
{
    return _buf[i];
}

// Calculate combined MIB
//
// \param snrs  Array of SNR
// \param channels  Array of channel numbers
//
// return Combined MIB value
//
double PhyLteCalcCombinedMIB(
    const PhyLteMibCurve& bpskMib, const PhyLteMibCurve& mibCurve,
    const std::vector<double>& snrs,
    const std::vector<int>& channels)
{
    double sum_snr_nondB_bpsk = 0.0;
    for (size_t i = 0; i < snrs.size(); ++i)
    {
        double snr = snrs[i];
        double mib_c = mibCurve.lookup(snr, channels[i]);
        double snr_dB_bpsk = bpskMib.ilookup(mib_c, 0);
        sum_snr_nondB_bpsk += NON_DB(snr_dB_bpsk);
    }
    return bpskMib.lookup(IN_DB(sum_snr_nondB_bpsk), 0);
}

// Lookup MIB value from snr[dB] for BPSK
// using approximated function of SNR-MIB curve.
//
// \param snr_nondB  SNR
//
// \return  MIB value
//
double PhyLteRxModelCaclJ(double snr_nondB)
{
    return 1.0 - exp(-snr_nondB/PHY_LTE_APPROX_J_FUNC_COEFF);
}

// Lookup snr value from MIB for BPSK
// using approximated function of SNR-MIB curve.
//
// \param mib  MIB
//
// \return  SNR (non dB)
//
double LteRxModelCalcInvJ(double mib)
{
    return -PHY_LTE_APPROX_J_FUNC_COEFF*log(1.0-mib);
}

// Calculate G function
//
// \param v  Arguments of G function
// \param startIndex  Start point of valid argument in v
//
// \return  G(v)
//
double PhyLteRxModelCalcG(const std::vector<double>& v, int startIndex)
{
    if (v.size() - startIndex == 1) return v[v.size()-1];
    return v[0] + ( 1.0 - v[0] ) * PhyLteRxModelCalcG(v, startIndex+1);
}

// Calculation of MIB in system level simulation assumption
//
// \param mibCurve  SNR-MIB curve
// \param txNo  Transmission number ( 0:1st tx, 1: 1st retx, 2: 2nd retx ... )
// \param snrdB_list  List of list of SNRs
// \param s  const vector<int>& : Channel array
//
// \return  MIB value
//
double PhyLteCalcAvgMibSL(
    const PhyLteMibCurve& mibCurve,
    int txNo,
    const std::vector< std::vector<double> >& snrdB_list, const std::vector<int>& s)
{
    int numTx = txNo + 1;
    int numCh = MAX(1, mibCurve.getNumCh() / 2); // 64QAM, 6ch, Identically 3, 16QAM, 4ch, Identically, 2, QPSK, BPSK, Identically 1
    std::vector< std::vector<double> > avgMib(numTx);
    for (int tx = 0; tx < numTx; ++tx)
    {
        avgMib[tx].resize(numCh, 0.0);
        for (int ch = 0; ch < numCh; ++ch)
        {
            for (size_t k = 0; k < snrdB_list[tx].size(); ++k)
            {
                avgMib[tx][ch] += mibCurve.lookup(snrdB_list[tx][k], ch * 2); // 64QAM,(0,1),(2,3),(4,5) are identical. 16QAM,(0,1),(2,3) are identical
            }
            avgMib[tx][ch] /= snrdB_list[tx].size();
        }
    }

    int sumS = std::accumulate(s.begin(), s.begin() + numTx, 0);
    SequenceGenerator p(numCh, sumS);

    double ret = 0.0;
    UInt64 pcnt = 0;
    while (!p.eos())
    {
        std::vector<double> argG(sumS, 0.0);

        int txId = 0;
        int nextThr = s[txId];
        for (int si = 0; si < sumS; ++si)
        {
            while (si == nextThr){ ++txId; nextThr += s[txId]; }
            argG[si] = avgMib[txId][(size_t)p[si]];
        }

        double Gv = PhyLteRxModelCalcG(argG);

        ret += Gv;

        ++p;
        ++pcnt;
    }

    ret /= pcnt; //pow(numCh, sumS);

    return ret;
}

// Get index of MCS-#RB 2-dimensional matrix
//
// \param mcsIndex  MCS
// \param numRb  Number of RBs
//
// \return Index
//
inline int mcsRbMatIdx(int mcsIndex, int numRb)
{
    return mcsIndex * PHY_LTE_SPEC_MAX_NUM_RB + (numRb-1);
}

// Get index of MCS-#RB-#Tx 3-dimensional matrix
//
// \param mcsIndex  MCS
// \param numRb  Number of RBs
// \param txNo  Transmission number ( 0 ... )
//
// \return  Index
//
inline int mcsRbRvidxMatIdx(int mcsIndex, int numRb, int txNo)
{
    return mcsIndex * PHY_LTE_SPEC_MAX_NUM_RB * PHY_LTE_NUM_RV + (numRb-1) * PHY_LTE_NUM_RV + txNo;
}


// Calculation of MIB in system level simulation assumption
//
// \param isDl  true if DL, otherwise UL
// \param mcsIndex  MCS index ( 0 ... 28 )
// \param numRb  Number of RB
// \param mibCUrve  SNR-MIB curve
// \param txNo  Transmission number ( 0:1st tx, 1: 1st retx, 2: 2nd retx ... )
// \param snrdB_list  : List of list of SNRs
//
// \return  MIB value
//
double PhyLteCalcAvgMibSL(
    bool isDl, int mcsIndex, int numRb,
    const PhyLteMibCurve& mibCurve,
    int txNo,
    const std::vector< std::vector<double>  >& snrdB_list)
{
    // Get permutation factor profile
    const std::vector<RepetitionFactorElem>& rf =
            PhyLteRepetitionFactorDB::getInstance().getRepetitionFactorProfile(isDl, mcsIndex, numRb, txNo);

    double rstar = 0.0;

    for (size_t i = 0; i < rf.size(); ++i)
    {
        double avgMib = PhyLteCalcAvgMibSL(mibCurve, txNo, snrdB_list, rf[i]._s);
        rstar += (avgMib * rf[i]._repetitionFactor);
    }

    return rstar;
}

//////////////////////////////////////////////////////////////////////////
// Constructor of PhyLteRepetitionFactorDB
//
PhyLteRepetitionFactorDB::PhyLteRepetitionFactorDB()
{
    std::string ph;
    BOOL ret = Product::GetProductHome(ph);
    ERROR_AssertArgs(ret == TRUE, "Cannot retrieve product home");

    std::string path = ph + LTE_PROF_DIR + LTE_REPETITION_PROF_FILE_NAME;

    std::vector< std::vector<std::string> > lines;
    ret = LteLoadExternalFile(path.c_str(), lines, ", \t\n", 9);

    if (!ret)
    {
        // Ensure non-LTE scenarios run without LTE data files.
        return;
    }

    for (size_t i = 0; i < lines.size(); ++i)
    {
        std::vector<std::string>& fields = lines[i];

        RepetitionFactorElem elem;

        BOOL isDL    = (BOOL)atoi(fields[0].c_str());
        elem._mcsIndex = atoi(fields[1].c_str());
        elem._numRb    = atoi(fields[2].c_str());
        elem._rvidx    = atoi(fields[3].c_str());
        elem._s.resize(4);
        elem._s[0]     = atoi(fields[4].c_str());
        elem._s[1]     = atoi(fields[5].c_str());
        elem._s[2]     = atoi(fields[6].c_str());
        elem._s[3]     = atoi(fields[7].c_str());
        elem._repetitionFactor = atof(fields[8].c_str());

        std::vector< std::vector< RepetitionFactorElem > >& repetitionFactorList =
                isDL ? _repetitionFactorListDL : _repetitionFactorListUL;

        if (repetitionFactorList.size() == 0)
        {
            repetitionFactorList.resize(PHY_LTE_NUM_RV * PHY_LTE_SPEC_MAX_NUM_RB * PHY_LTE_REGULAR_MCS_INDEX_LEN);
        }
        repetitionFactorList[mcsRbRvidxMatIdx(elem._mcsIndex, elem._numRb, elem._rvidx)].push_back(elem);
    }

#ifdef LTE_LIB_LOG
    for (int isDL = 1; isDL >= 0; --isDL){
        std::vector< std::vector< RepetitionFactorElem > >& repetitionFactorList =
                isDL ? _repetitionFactorListDL : _repetitionFactorListUL;
        for (int i = 0; i < repetitionFactorList.size(); ++i){
            for (int j = 0; j < repetitionFactorList[i].size(); ++j){
                lte::LteLog::DebugFormat(NULL, -1, "PHY", "DUMP_REPETITION_PROF,%s,%d,%d,mcs,%d,numRb,%d,rvidx,%d,rf,%f,{,%d,%d,%d,%d,}",
                        (isDL ? "DL" : "UL"), i, j,
                        repetitionFactorList[i][j]._mcsIndex,
                        repetitionFactorList[i][j]._numRb,
                        repetitionFactorList[i][j]._rvidx,
                        repetitionFactorList[i][j]._repetitionFactor,
                        repetitionFactorList[i][j]._s[0],
                        repetitionFactorList[i][j]._s[1],
                        repetitionFactorList[i][j]._s[2],
                        repetitionFactorList[i][j]._s[3]);

            }
        }
    }
#endif
}

// Get the instance of PhyLteRepetitionFactorDB
//
// \return  Reference to the instance
//
PhyLteRepetitionFactorDB& PhyLteRepetitionFactorDB::getInstance()
{
    // This class is thread-safe as long as the very first call
    // of getInstance() is called from single thread.
    // Not MPI-safe.
    static PhyLteRepetitionFactorDB theObj;
    return theObj;
}

// Desctructor of PhyLteRepetitionFactorDB
//
PhyLteRepetitionFactorDB::~PhyLteRepetitionFactorDB()
{
}

//  Get repetition factor profile
//
// \param isDl  true if DL, otherwise false
// \param mcsIndex  MCS
// \param numRb  Number of RBs
// \param txNo   Transmission No.
//
// \return  Repetition factor element
//
const std::vector<RepetitionFactorElem>&
PhyLteRepetitionFactorDB::getRepetitionFactorProfile(
        bool isDl, int mcsIndex, int numRb, int txNo)
{
    if (_repetitionFactorListDL.size() == 0)
    {
        ERROR_ReportErrorArgs("Failed to load external file : %s", LTE_REPETITION_PROF_FILE_NAME);
    }

    assert(mcsIndex >=  0 && mcsIndex <= (PHY_LTE_REGULAR_MCS_INDEX_LEN-1));
    assert(numRb >=  1 && numRb <= PHY_LTE_SPEC_MAX_NUM_RB);
    int index = mcsRbRvidxMatIdx(mcsIndex, numRb, txNo);
    return isDl ? _repetitionFactorListDL[index] : _repetitionFactorListUL[index];
}

// Constructor of PhyLteEcrDB
//
PhyLteEcrDB::PhyLteEcrDB()
{
    std::string ph;
    BOOL ret = Product::GetProductHome(ph);
    assert( ret == TRUE );

    std::string path = ph + LTE_PROF_DIR + LTE_ECR_PROF_FILE_NAME;
    std::vector< std::vector<std::string> > lines;
    ret = LteLoadExternalFile(path.c_str(), lines, ", \t\n", 5);

    if (!ret)
    {
        // Ensure non-LTE scenarios run without LTE data files.
        return;
    }

    _ecrProfDL.resize(PHY_LTE_NUM_RV * PHY_LTE_SPEC_MAX_NUM_RB * PHY_LTE_REGULAR_MCS_INDEX_LEN);
    _ecrProfUL.resize(PHY_LTE_NUM_RV * PHY_LTE_SPEC_MAX_NUM_RB * PHY_LTE_REGULAR_MCS_INDEX_LEN);

    for (size_t i = 0; i < lines.size(); ++i)
    {
        std::vector<std::string>& fields = lines[i];

        BOOL isDL    = (BOOL)atoi(fields[0].c_str());
        int mcsIndex = atoi(fields[1].c_str());
        int numRb    = atoi(fields[2].c_str());
        int rvidx    = atoi(fields[3].c_str());
        double ecr   = atof(fields[4].c_str());

        std::vector< double >& ecrProf = isDL ? _ecrProfDL : _ecrProfUL;

        ecrProf[mcsRbRvidxMatIdx(mcsIndex, numRb, rvidx)] = ecr;
    }

#ifdef LTE_LIB_LOG
    for (int i = 0; i < _ecrProfDL.size(); ++i)
        lte::LteLog::DebugFormat(NULL, -1, "PHY", "DUMP_ECR_PROF,DL,%d,%f", i, _ecrProfDL[i]);
    for (int i = 0; i < _ecrProfUL.size(); ++i)
        lte::LteLog::DebugFormat(NULL, -1, "PHY", "DUMP_ECR_PROF,UL,%d,%f", i, _ecrProfUL[i]);
#endif
}

// Get the instance of PhyLteEcrDB
//
// \return Reference to the instance
//
PhyLteEcrDB& PhyLteEcrDB::getInstance()
{
    // This class is thread-safe as long as the very first call
    // of getInstance() is called from single thread.
    // Not MPI-safe.
    static PhyLteEcrDB theObj;
    return theObj;
}

// Desctructor of PhyLteEcrDB
//
PhyLteEcrDB::~PhyLteEcrDB()
{
}

// Initialize ECR database
//
void PhyLteEcrDB::init()
{
}

// Get ECR(Effective Code Rate)
//
// \param isDL  true if DL, otherwise false
// \param mcsIndex  MCS index
// \param numRb  Number of RBS
// \param txNo  Transmission No
//
// \return  ECR value
//
double PhyLteEcrDB::getEcr(bool isDl, int mcsIndex, int numRb, int txNo)
{
    if (_ecrProfDL.size() == 0)
    {
        ERROR_ReportErrorArgs("Failed to load external file : %s", LTE_ECR_PROF_FILE_NAME);
    }

    assert(mcsIndex >=  0 && mcsIndex <= (PHY_LTE_REGULAR_MCS_INDEX_LEN-1));
    assert(numRb >=  1 && numRb <= PHY_LTE_SPEC_MAX_NUM_RB);
    int index = mcsRbRvidxMatIdx(mcsIndex, numRb, txNo);
    return isDl ? _ecrProfDL[index] : _ecrProfUL[index];
}

// Constructor of PhyLteCbsDB
//
PhyLteCbsDB::PhyLteCbsDB()
{
    std::string ph;
    BOOL ret = Product::GetProductHome(ph);
    ERROR_AssertArgs(ret == TRUE, "Cannot retrieve product home");

    std::string path = ph + LTE_PROF_DIR + LTE_CBS_PROF_FILE_NAME;
    std::vector< std::vector<std::string> > lines;
    ret = LteLoadExternalFile(path.c_str(), lines, ", \t\n", 4);

    if (!ret)
    {
        // Ensure non-LTE scenarios run without LTE data files.
        return;
    }

    _cbsProfDL.resize(PHY_LTE_SPEC_MAX_NUM_RB * PHY_LTE_REGULAR_MCS_INDEX_LEN);
    _cbsProfUL.resize(PHY_LTE_SPEC_MAX_NUM_RB * PHY_LTE_REGULAR_MCS_INDEX_LEN);

    for (size_t i = 0; i < lines.size(); ++i)
    {
        std::vector<std::string>& fields = lines[i];

        BOOL isDL    = (BOOL)atoi(fields[0].c_str());
        int mcsIndex = atoi(fields[1].c_str());
        int numRb    = atoi(fields[2].c_str());
        int numCb    = atoi(fields[3].c_str());

        std::vector<int>& cbsProf = isDL ? _cbsProfDL : _cbsProfUL;

        cbsProf[mcsRbMatIdx(mcsIndex, numRb)] = numCb;
    }

#ifdef LTE_LIB_LOG
    for (int i = 0; i < _cbsProfDL.size(); ++i)
        lte::LteLog::DebugFormat(NULL, -1, "PHY", "DUMP_CBS_PROF,DL,%d,%d", i, _cbsProfDL[i]);
    for (int i = 0; i < _cbsProfUL.size(); ++i)
        lte::LteLog::DebugFormat(NULL, -1, "PHY", "DUMP_CBS_PROF,UL,%d,%d", i, _cbsProfUL[i]);
#endif
}

// Get the instance of PhyLteCbsDB
//
// \return  PhyLteCbsDB& : Reference to the instance
//
PhyLteCbsDB& PhyLteCbsDB::getInstance()
{
    // This class is thread-safe as long as the very first call
    // of getInstance() is called from single thread.
    // Not MPI-safe.
    static PhyLteCbsDB theObj;
    return theObj;
}

// Destructor of PhyLteCbsDB
//
PhyLteCbsDB::~PhyLteCbsDB()
{
}

// Initialize ECR database
//
void PhyLteCbsDB::init()
{
}

// Get number of code blocks
//
// \param isDL  true if DL, otherwise false
// \param mcsIndex  MCS index
// \param numRb  Number of RBS
//
// \return  Number of code blocks
//
int PhyLteCbsDB::getNumCodeBlocks(bool isDl, int mcsIndex, int numRb)
{
    if (_cbsProfDL.size() == 0)
    {
        ERROR_ReportErrorArgs("Failed to load external file : %s", LTE_CBS_PROF_FILE_NAME);
    }

    assert(mcsIndex >=  0 && mcsIndex <= (PHY_LTE_REGULAR_MCS_INDEX_LEN-1));
    assert(numRb >=  1 && numRb <= PHY_LTE_SPEC_MAX_NUM_RB);

    std::vector<int>& cbsProf = isDl ? _cbsProfDL : _cbsProfUL;
    return cbsProf[mcsRbMatIdx(mcsIndex, numRb)];
}

// Destructor of PhyLteMibCurve
//
PhyLteMibCurve::~PhyLteMibCurve()
{
    for (size_t i = 0; i < _mibArray.size(); ++i)
    {
        delete [] _mibArray[i].chMib;
        _mibArray[i].chMib = NULL;
    }

    for (size_t i = 0; i < _invMibArray.size(); ++i)
    {
        delete [] _invMibArray[i].chMib;
        _invMibArray[i].chMib = NULL;
    }
}

// Load SNR-MIB entry from RAW data
//
// \param entries  RAW SNR-MIB curve data
// \param numElem  Number of elements in entries
//
void PhyLteMibCurve::loadMib(const char* fn, int modType)
{
    std::vector< std::vector<std::string> > lines;
    BOOL ret = LteLoadExternalFile(fn, lines, " \t\n", -1);

    if (!ret)
    {
        // Ensure non-LTE scenarios run without LTE data files.
        return;
    }

    for (size_t i = 0; i < lines.size(); ++i)
    {
        PhyLteMibEntry me;
        me.snr_dB = atof(lines[i][0].c_str());
        me.avgMib = atof(lines[i][1].c_str());
        me.numCh = modType;
        me.chMib = new double[me.numCh];
        for (int c = 0; c < me.numCh; ++c)
            me.chMib[c] = MIN(1.0, atof(lines[i][2+c].c_str()));
        _mibArray.push_back(me);
    }


    double intv = _mibArray[1].snr_dB - _mibArray[0].snr_dB;
    double invIntv = 0.01;
    double r = intv/invIntv;
    _invMibArray.reserve(_mibArray.size() * (int)r);
    // Make data for inverse lookup
    for (double snr_dB = _mibArray[0].snr_dB;
        snr_dB <= _mibArray[_mibArray.size()-1].snr_dB; snr_dB += invIntv)
    {
        PhyLteMibEntry me;
        me.snr_dB = snr_dB;
        me.avgMib = lookup(snr_dB, -1);
        me.numCh  = _mibArray[0].numCh;
        me.chMib = new double[me.numCh];
        for (int c = 0; c < me.numCh; ++c)
            me.chMib[c] = lookup(snr_dB, c);
        _invMibArray.push_back(me);
    }

#ifdef LTE_LIB_LOG
    Dump();
#endif
}

// Lookup MIB value from SNR
//
// \param snr_dB  SNR[dB]
// \param chNo  Channel number
//
// \return  MIB value corresponding to snr_dB
//
double PhyLteMibCurve::lookup(double snr_dB, int chNo) const
{
    if (_mibArray.size() == 0)
    {
        ERROR_ReportError("Failed to load MIB file");
    }

    double intv = _mibArray[1].snr_dB - _mibArray[0].snr_dB;
    double ridx = (snr_dB - _mibArray[0].snr_dB) / intv;
    int idx = (int)ridx;

    double left, right;
    if (idx == -1){ left = 0.0; right = (chNo >= 0 ? _mibArray[0].chMib[chNo]: _mibArray[0].avgMib) ; }
    else if (idx == _mibArray.size()-1){ left = (chNo >= 0 ? _mibArray[idx].chMib[chNo]: _mibArray[idx].avgMib); right = 1.0; }
    else if (idx < 0){ left = right = 0.0; }
    else if (idx >= (int)_mibArray.size()) { left = right = 1.0; }
    else{ left = (chNo >= 0 ? _mibArray[idx].chMib[chNo]: _mibArray[idx].avgMib); right = (chNo >= 0 ? _mibArray[idx+1].chMib[chNo]: _mibArray[idx+1].avgMib); }
    return lininterp(left, right, ridx - idx);
}

// /Lookup SNR value from MIB
//
// \param mib  MIB value
// \param chNo  Channel number
//
// \return  SNR[dB] value corresponding to mib.
//
double PhyLteMibCurve::ilookup(double mib, int chNo) const
{
    if (_mibArray.size() == 0)
    {
        ERROR_ReportError("Failed to load MIB file");
    }

    const std::vector<PhyLteMibEntry>& ref = _invMibArray; // Lookup Interpolated Curve
    if (chNo == -1)
    {
        std::vector<PhyLteMibEntry>::const_iterator it =
                std::lower_bound(ref.begin(), ref.end(), mib, PhyLtePredAvgMib());
        if (it == ref.end())
            return ref[ref.size()-1].snr_dB;
        return it->snr_dB;
    }else
    {
        std::vector<PhyLteMibEntry>::const_iterator it =
                std::lower_bound(ref.begin(), ref.end(), mib, PhyLtePredChMib(chNo));
        if (it == ref.end())
            return ref[ref.size()-1].snr_dB;
        return it->snr_dB;
    }
}


#ifdef LTE_LIB_LOG
// Dump SNR-MIB curve
//
void PhyLteMibCurve::Dump()
{
    for (int i = 0; i < _mibArray.size(); ++i)
    {
        const PhyLteMibEntry& e = _mibArray[i];
        std::stringstream ss;
        for (int c = 0; c < e.numCh; ++c)
        {
            ss << e.chMib[c] << ",";
        }
        lte::LteLog::DebugFormat(NULL,-1,"PHY","DUMP_MIB,snr,%f,avgMib,%f,numCh,%d,chMib,%s",
                e.snr_dB,e.avgMib,e.numCh,ss.str().c_str());

    }

    for (int i = 0; i < _invMibArray.size(); ++i)
    {
        const PhyLteMibEntry& e = _invMibArray[i];
        std::stringstream ss;
        for (int c = 0; c < e.numCh; ++c)
        {
            ss << e.chMib[c] << ",";
        }
        lte::LteLog::DebugFormat(NULL,-1,"PHY","DUMP_INV_MIB,snr,%f,avgMib,%f,numCh,%d,chMib,%s",
                e.snr_dB,e.avgMib,e.numCh,ss.str().c_str());

    }
}
#endif

// Get the instance of PhyLteMibCurveDB
//
// \return Reference to the instance
//
PhyLteMibCurveDB& PhyLteMibCurveDB::getInstance()
{
    // This class is thread-safe as long as the very first call
    // of getInstance() is called from single thread.
    // Not MPI-safe.
    static PhyLteMibCurveDB theObj;
    return theObj;
}

// Constructor of PhyLteMibCurveDB. Load MIB curves for
// QPSK, 16QAM, 64QAM
//
PhyLteMibCurveDB::PhyLteMibCurveDB()
{
    std::string ph;
    BOOL ret = Product::GetProductHome(ph);
    ERROR_AssertArgs(ret == TRUE, "Cannot retrieve product home");

    std::string path = ph + LTE_MIB_DIR;

    _qpsk  = new PhyLteMibCurve((path + "QPSK.mib").c_str(),  2);
    _qam16 = new PhyLteMibCurve((path + "16QAM.mib").c_str(), 4);
    _qam64 = new PhyLteMibCurve((path + "64QAM.mib").c_str(), 6);
}

// Get SNR-MIB curve for specified modulation order
//
// \param modType : int : Modulation order ( 2 : QPSK, 4 : 16QAM, 6 : 64QAM )
// RETURN     :: PhyLteMibCurve* : Pointer to SNR-MIB curve
// **/
PhyLteMibCurve* PhyLteMibCurveDB::getCurve(int modType)
{
    if (modType == 2)
        return _qpsk;
    else if (modType == 4)
        return _qam16;
    else if (modType == 6)
        return _qam64;
    else
        return NULL;
}

// Destructor of PhyLteMibCurveDB
//
PhyLteMibCurveDB::~PhyLteMibCurveDB()
{
    delete _qpsk;
    _qpsk = NULL;
    delete _qam16;
    _qam16 = NULL;
    delete _qam64;
    _qam64 = NULL;
}

// Get number of code blocks in transport
// block defined for specified MCS and #RB
//
// \param isDL  true if DL, otherwise false
// \param mcsIndex  MCS index
// \param numRb  Number of RBs
//
// \return   Number of code blocks
//
int PhyLteGetNumCodeBlocks(bool isDL, int mcsIndex, int numRb)
{
    assert(mcsIndex >=  0 && mcsIndex <= (PHY_LTE_REGULAR_MCS_INDEX_LEN-1));
    assert(numRb >=  1 && numRb <= PHY_LTE_SPEC_MAX_NUM_RB);
    return PhyLteCbsDB::getInstance().getNumCodeBlocks(isDL, mcsIndex, numRb);
}
