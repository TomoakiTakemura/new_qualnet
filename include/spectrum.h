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

#ifndef SPECTRUM_H
#define SPECTRUM_H
#include <math.h>
#include <iostream>
#include <vector>

#ifdef USE_BOOST_ARCHIVE
#include <boost/serialization/export.hpp>
#endif
#include <boost/numeric/interval.hpp>

/// \brief a convienence macro to simplify boost serialization
#ifdef USE_BOOST_ARCHIVE
#define DECLARE_SERIALIZE_SUPPORT \
    friend class boost::serialization::access; \
    template<class Archive> void serialize(Archive & ar, const unsigned int version);
#else
#define DECLARE_SERIALIZE_SUPPORT 
#endif


/// \brief a class to represent part of the EM spectrum
///
/// This abstract base class represents the characteristics of a radio
/// transmitter or reciever in response to the EM spectrum. 

class spectralBand 
{
    friend class spectrum;

protected:
    spectralBand(const std::string& name = "Unspecified") : m_name(name) { }
    virtual ~spectralBand();
public:

    bool overlaps(const spectralBand& b);

    virtual double spectralMap(double frequency) const = 0;
    virtual double convolve(const spectralBand* b) const;
    virtual double getFrequency() const = 0;  ///< \pure  \brief center frequency of band
    virtual double getBandwidth() const = 0;  ///< \pure  \brief bandwidth
    int getQualnetChannel() const {return m_radioOverlayID;}

    std::ostream& fmt(const char* tag, std::ostream& os) const;
    virtual std::ostream& fmt(std::ostream& os) const;

    const char* typeName() const { return m_name.c_str(); }

    /// \brief get frequency from channel index
    ///
    /// The IEEE defines chanel index numbers that map to
    /// various bands in the EM spectrum.  These functions
    /// handle mapping from channel index to frquency
    ///
    /// \param int channelIndex the number of the channel
    /// \returns the center frequency of the band.
    static double chanFreq_24GHz(int channelIndex)
    {
      if (channelIndex <= 0) return 0;
      if (channelIndex > 14) return 0;
      if (channelIndex == 14) return 2484.0e6;
      return 2407.0e6 + 5.0e6 * (double)channelIndex;
    }

    /// \overload
    static double chanFreq_50GHz(int channelIndex) 
    {
      if (channelIndex <= 0) return 0;
      if (channelIndex > 170) return 0;
      
      return 5000.0e6 + 5.0e6 * (double)channelIndex;
      // I have seen some references to some channels just
      // below 5Hz with channel numbers 180-195 layed out as:
      //return MHz(4000  + chan*5);  // ignore them for now.
    }

    static double chanFreq_59GHz(int channelIndex) 
    {
        if (channelIndex <= 170) return 0;
        if (channelIndex > 184) return 0;
        return 5000.0e6 + 5.0e6 * (double)channelIndex;
    }

    /// ChBandwidth
    // The enum types for 802.11n/802.11ac channel bandwidth
    enum ChBandwidth 
    {
      CHBWDTH_10MHZ = 0,
      CHBWDTH_20MHZ,
      CHBWDTH_40MHZ,
      CHBWDTH_80MHZ,
      CHBWDTH_160MHZ,
      CHBWDTH_80PLUS80,
      CHBWDTH_22MHZ,     // used by 802.11b in 2.4Ghz band
      CHBWDTH_16MHZ,     // used by 802.11a in 5GHz band
      CHBWDTH_UNKNOWN,
      CHBWDTH_MAX        // to size arrays.
    };

    // get the bandwidth as a double in Hz from the ChBandwidth
    static double bandwidth(enum ChBandwidth b)
    {
      switch (b) 
      {
        case CHBWDTH_10MHZ:  return  10.0e6;
        case CHBWDTH_16MHZ:  return  16.6e6;
        case CHBWDTH_20MHZ:  return  20.0e6;
        case CHBWDTH_22MHZ:  return  22.0e6;
        case CHBWDTH_40MHZ:  return  40.0e6;
        case CHBWDTH_80MHZ:  return  80.0e6;
        case CHBWDTH_160MHZ: return 160.0e6;
        case CHBWDTH_80PLUS80: return 160.0e6;
        case CHBWDTH_UNKNOWN: return 0.0;
        case CHBWDTH_MAX: return 0.0;
      }
      ERROR_AssertArgs(false, "ChBandwidth value %d unknown", (int)b);
      return 0.0;
    }

    static bool testChBandwidth(double bw, enum ChBandwidth b)
    {
      if (bw == 0.0) return false;
      double bw2 = bandwidth(b);
      if (bw2 == 0.0) return false;
      double delta = bw - bw2;
      // if bw is within 5Hz of bandwidth(b) then we will
      // say that its the same band
      if (delta > 5.0) return false;
      if (delta < -5.0) return false;
      return true;
    }

    // given a double value for the bandwidth find the corrisponding value of enum ChBandwidth
    // have to take a little care as even if the value is off by a small fraction it can still
    // be a valid number.
    static enum ChBandwidth findChBandwidth(double bw)
    {
        if (testChBandwidth(bw, CHBWDTH_10MHZ))  return CHBWDTH_10MHZ;
        if (testChBandwidth(bw, CHBWDTH_20MHZ))  return CHBWDTH_20MHZ;
        if (testChBandwidth(bw, CHBWDTH_40MHZ))  return CHBWDTH_40MHZ;
        if (testChBandwidth(bw, CHBWDTH_80MHZ))  return CHBWDTH_80MHZ;
        if (testChBandwidth(bw, CHBWDTH_160MHZ)) return CHBWDTH_160MHZ;
        return CHBWDTH_UNKNOWN;
    }

 protected:
    std::string m_name;
    int m_spectrumIndex;
    int m_radioOverlayID;

public:
    DECLARE_SERIALIZE_SUPPORT;
};

class spectralBand_Pair : public spectralBand 
{
    friend class spectrum;

    spectralBand_Pair(spectralBand* e1 = 0, spectralBand* e2 = 0);
    virtual ~spectralBand_Pair();

public:
     double spectralMap(double f) const;
     double convolve(const spectralBand* b) const;
     double getFrequency() const {return (m_e1->getFrequency() + m_e2->getFrequency())/2.0;}
     double getBandwidth() const {return (m_e1->getBandwidth() + m_e2->getBandwidth());}
     std::ostream& fmt(std::ostream& os) const;

private:
     const spectralBand* m_e1;
     const spectralBand* m_e2;


     DECLARE_SERIALIZE_SUPPORT;
 };


class spectralBand_Square : public spectralBand 
{
    friend class spectrum;

protected:

    double m_center;
    double m_width;
    enum ChBandwidth m_chBandwidth;

    DECLARE_SERIALIZE_SUPPORT;

public:

 private:
    spectralBand_Square();
    spectralBand_Square(double center, enum ChBandwidth bw, const std::string& name);
    spectralBand_Square(const spectralBand_Square* s);
    spectralBand_Square(const spectralBand_Square& s);

    virtual ~spectralBand_Square();

 public:

    static spectralBand* make_24GHz(int chan, PartitionData* partitionData, int radioOverlayID = -1)
    { return makeBand(chanFreq_24GHz(chan), CHBWDTH_22MHZ, "2.4GHz Band", partitionData, radioOverlayID);}
    static spectralBand* make_50GHz(int chan, double bandwidth, PartitionData* partitionData, int radioOverlayID = -1)
    { return makeBand(chanFreq_50GHz(chan), findChBandwidth(bandwidth), "5.0GHz Band", partitionData, radioOverlayID);}
    static spectralBand* make_50GHz(int chan, enum ChBandwidth b, PartitionData* partitionData, int radioOverlayID = -1)
    { return makeBand(chanFreq_50GHz(chan), b, "5.0GHz Band", partitionData, radioOverlayID);}

    static spectralBand* make_80211b(int chan, PartitionData* partitionData, int radioOverlayID = -1) 
    { return makeBand(chanFreq_24GHz(chan), CHBWDTH_22MHZ, "802.11b", partitionData, radioOverlayID); }
    static spectralBand* make_80211a(int chan, PartitionData* partitionData, int radioOverlayID = -1) 
    { return makeBand(chanFreq_50GHz(chan), CHBWDTH_20MHZ, "802.11a", partitionData, radioOverlayID); }
    static spectralBand* make_80211ac20(int chan, PartitionData* partitionData, int radioOverlayID = -1) 
    { return makeBand(chanFreq_50GHz(chan), CHBWDTH_20MHZ, "802.11ac20", partitionData, radioOverlayID); }
    static spectralBand* make_80211ac40(int chan, PartitionData* partitionData, int radioOverlayID = -1) 
    { return makeBand(chanFreq_50GHz(chan), CHBWDTH_40MHZ, "802.11ac40", partitionData, radioOverlayID); }
    static spectralBand* make_80211ac80(int chan, PartitionData* partitionData, int radioOverlayID = -1) 
    { return makeBand(chanFreq_50GHz(chan), CHBWDTH_80MHZ, "802.11ac80", partitionData, radioOverlayID); }
    static spectralBand* make_80211ac160(int chan, PartitionData* partitionData, int radioOverlayID = -1) 
    { return makeBand(chanFreq_50GHz(chan), CHBWDTH_160MHZ, "802.11ac160", partitionData, radioOverlayID); }

    static spectralBand* makeBand(double frequency, enum ChBandwidth b, const char* name, PartitionData* partitionData, int radioOverlayID = -1);
    static spectralBand* makeBand(double frequency, double bw, const char* name, PartitionData* partitionData, int radioOverlayID = -1) 
    { return makeBand(frequency, findChBandwidth(bw), name, partitionData, radioOverlayID);}

    double spectralMap(double f) const;
    double convolve(const spectralBand_Square* b) const;
    double convolve(const spectralBand* b) const;
    typedef boost::numeric::interval<double> band;
    std::ostream& fmt(std::ostream& os) const;
    double getFrequency() const {return m_center;}
    double getBandwidth() const {return m_width;}
    band getBand() const {return band(m_center-m_width/2, m_center+m_width/2);}
    enum ChBandwidth getChBandwidth() {return m_chBandwidth;}
};

// This class represents the RF spectrum and its use in channels.
// The primary purpose is to facillitate interference calculations
// in the propagation layer, where sender and reciever are tuned
// to different but overlapping channels.  The overlap can occur
// for several reasons, in 802.11b the spectral masks of the
// channels overlap, a 802.11n 40Mhz channel can overlap two
// 802.11a 20Mhz channels.
// The spectrum is a singleton that contains a collection of
// spectral elements.  For each center frequency and bandwidth
// there only exists one spectral element.  This facillitates
// pre-calculating the overlap for bands and simplifies the
// calcuation needed to convert beween spectral bands and
// the use of qualnet channel as a 'community of interest'

class spectrum {
public:
  spectrum() { }

  ~spectrum() 
  {
    for (unsigned int i = 0; i < m_elements.size(); i++)
    {
      std::vector<spectralBand*> element = m_elements[i];
      for (unsigned int j = 0; j < element.size(); j++)
      {
       spectralBand* e = element[j];
       element[j] = 0;
       delete e;
      }
    }
  }

  spectralBand* make(double frequency, enum spectralBand::ChBandwidth bw, const char* name, int radioOverlayID = -1)
  {
    int idx = bandIdx(frequency, bw);
    assert(idx != 0);

    if (radioOverlayID != -1) 
    {
      if (m_elements.size() < (unsigned int) radioOverlayID + 1)
      {
       m_elements.resize(radioOverlayID+1);
      }
      std::vector<spectralBand*>& elements = m_elements[radioOverlayID];
      if (elements.size() == 0
          || (unsigned int)idx >= elements.size()-1)
      {
        elements.resize(idx + 10);
      }
      if (elements[idx] == 0)
      {
       elements[idx] = new spectralBand_Square(frequency, bw, name);
       elements[idx]->m_radioOverlayID = radioOverlayID;
       elements[idx]->m_spectrumIndex = idx;
      }
      return elements[idx];
    }
    for (unsigned int i = 0; i < m_elements.size(); i++)
    {
      std::vector<spectralBand*>& elements = m_elements[i];
      if ((unsigned int)idx < elements.size() && elements[idx] != 0)
      {
       return elements[idx];
      }
    }
    if (m_elements.size() < 1)
    {
      m_elements.resize(1);
    }
    std::vector<spectralBand*>& elements = m_elements[0];
    if ((unsigned int) idx-1 >= elements.size())
    {
      elements.resize(idx + 10);
    }
    if (elements[idx] == 0)
    {
      elements[idx] = new spectralBand_Square(frequency, bw, name);
      elements[idx]->m_radioOverlayID = 0;
      elements[idx]->m_spectrumIndex = idx;
    }
#ifdef DEBUG
    cout << idx << endl;
    cout << elements[idx] << endl;
    fmt(std::cout);
    std::cout << std::endl;
#endif
    return elements[idx];
  }

  /*
    int bandIdx(double f, double bw)
      convert the center frequency and bandwidth into an integer that can
      be used to index an array of spectralElement. This relies on the fact
      that in a communications environemnt there are a small number of
      frequencies that are used.
   */
  int bandIdx(double f, enum spectralBand::ChBandwidth bw) 
  {
    int ix;
    if (f < 3.0E9) 
    {
      ix = (int)((f - 2407e6)*200/1.0E9);  //this will have the range 0-15
    }
    else if (f < 6.0E9)
{
      ix = (int)((f - 5000e6)*200/1.0E9) + 20;  // will cover the range 20-204
    }
    ERROR_AssertArgs(ix >= 0, "f=%f", f);
    ERROR_AssertArgs(ix < 214, "f=%f", f);
    int ib = bw;
    ERROR_AssertArgs(ib >= 0, "bw=%d", bw);
    ERROR_AssertArgs(ib <= spectralBand::CHBWDTH_MAX, "bw=%d", bw);
    return ix*spectralBand::CHBWDTH_MAX + ib; 
  }

  std::ostream& fmt(std::ostream& os) const;
private:
  // a vector of vectors to represent the radioOverlayID and the spectral band
  // it will be a rather sparse matrix with often only one element for radioOverlayID
  // As there is only one of these the memory use is inconsiquential but the rapid
  // access from using an array is important.
  std::vector<std::vector<spectralBand*> > m_elements;
};
inline std::ostream& operator<<(std::ostream& os, const spectralBand* b) {return b->fmt(b->typeName(), os);}

struct Node;
class Message;

/// \brief Annoate the message with a sepctralBand that the signal is transmitted on.
///
/// The Message \p msg is assumed to contain a signal that is being prepared
/// for transmission.  This function serializes the spectralBand \p band
/// into \p msg.   \p node is a required parameter to access memory allocation
/// functions for the info fields in msg.
///
/// \param Node* node  the node containing the phy of the source of the transmission
/// \param Message* msg message to be annotated.
/// \param SpectralBand* band range of frequencies for the transmssion.
///
void MESSAGE_SetSpectralBand(Node* node, Message* msg, const spectralBand* band);

/// \brief Annotate the message with the spectralBand that the phy is tuned to
///
/// The Message \p msg is assumed to contain a signal that is being prepared
/// for transmission.  This function obtains the spectralBand from the phy
/// at \p phyIndex inside of \p node. This is serialzied into \p msg
/// into \p msg.   The phy must be 'tuned' to the apprpriate band using 
/// one of the Node::setRadio() functions.  \p channelIndex will be required
/// onyl if Node::setRadio() has not been called, as in that case PHY_GetFrequency()
/// and PHY_GetBandwidth() are used to obtain the characteristics of the transmssion.
///
/// \param Node* node  the node containing the phy of the source of the transmission
/// \param Message* msg message to be annotated.
/// \param int phyIndex the specific phy on the node
/// \param int channelIndex the channel that is being used for transmission.
///
void MESSAGE_SetSpectralBand(Node* node, Message* msg, int phyIndex, int channelIndex = 0);

/// brief obtain the spectral band from a message annotated by MESSAGE_SetSpectralBand()
///
/// \param Message* msg the message containing the spectral Band
///
/// This function will return null if the message does not contain a spectralBand
/// info element.
/// The return value is a pointer to a freshly allocated object and must be deleted.
// by the called once it is no longed needed.
/// \return the range of frequencies the message was transmiitted on
spectralBand* MESSAGE_GetSpectralBand(const Message* msg);

/// brief copy spectral band from one message to another
///
/// \param Node* node any node on same partition as destination for message
/// \param Message* dest the message to recieve the spectral band
/// \param Message* src the message containing the original spectral band
///  performs the equivelent of:
///   MESSAGE_SetSpectralBand(node, dest, MESSAGE_GetSpectralBand(src))
void MESSAGE_CopySpectralBand(Node* node, Message* dest, Message* src);
#endif  // SPECTRUM_H
