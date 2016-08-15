#include <iostream>
#include <sstream>
#include <cstring>
#include <map>

#ifdef USE_BOOST_ARCHIVE
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif

#include "main.h"
#include "node.h"
#include "spectrum.h"
#include "message.h"
#include "partition.h"



// Implimentation of the several  methods in spectral band
// Since the functions each have a lot in common they are grouped
// by function rather than by class.
spectralBand_Square::spectralBand_Square() :
  spectralBand("Unspecified"),
  m_center(0.0),
  m_width(0.0),
  m_chBandwidth(CHBWDTH_UNKNOWN)
 { ; }

spectralBand_Square::spectralBand_Square(double center, enum ChBandwidth bw, const std::string& name) :
  spectralBand(name),
  m_center(center),
  m_width(bandwidth(bw)),
  m_chBandwidth(bw)
  { ; }

spectralBand_Square::spectralBand_Square(const spectralBand_Square* s) :
  spectralBand(std::string(s->typeName())), 
  m_center(s->m_center), 
  m_width(s->m_width),
  m_chBandwidth(s->m_chBandwidth)
  { ; }

spectralBand_Square::spectralBand_Square(const spectralBand_Square& s) :
  spectralBand(std::string(s.typeName())), 
  m_center(s.m_center), 
  m_width(s.m_width),
  m_chBandwidth(s.m_chBandwidth)
  { ; }

spectralBand_Pair::spectralBand_Pair(spectralBand* e1, spectralBand* e2) 
: m_e1(e1), m_e2(e2) 
{
        m_name = "pair of "; 
        m_name += m_e1->typeName();
}

spectralBand::~spectralBand() {
}

spectralBand_Pair::~spectralBand_Pair() {
   m_e1 = 0;
   m_e2 = 0;
}

spectralBand_Square::~spectralBand_Square() { ; }

double spectralBand::spectralMap(double frequency) const { return 0.0; }

double spectralBand_Pair::spectralMap(double f) const 
{
    if (m_e1 == 0 && m_e2 == 0) return spectralBand::spectralMap(f);

    if (m_e1 == 0) return m_e2->spectralMap(f);
    if (m_e2 == 0) return m_e1->spectralMap(f);

    return m_e1->spectralMap(f) + m_e2->spectralMap(f);
}

double spectralBand_Square::spectralMap(double f) const
{
    if (f < m_center - m_width) return 0.0;

    if (f > m_center + m_width) return 0.0;

    return 1.0; 
}

double spectralBand::convolve(const spectralBand* b) const
{
  return 0.0;
}

double spectralBand_Square::convolve(const spectralBand* b) const 
{
    const spectralBand_Square* s = dynamic_cast<const spectralBand_Square*>(b);

    if (s != 0) return convolve(s);
    return b->convolve(this);
}

double spectralBand_Square::convolve(const spectralBand_Square* other) const 
{
    assert(other != 0);
    if (!boost::numeric::overlap(getBand(), other->getBand())) return 0.0;
    band overlap = boost::numeric::intersect(getBand(), other->getBand());
    return boost::numeric::width(overlap);
}

double spectralBand_Pair::convolve(const spectralBand* b) const 
{
    const spectralBand_Square* s = dynamic_cast<const spectralBand_Square*>(b);
    if (s != 0) return s->convolve(m_e1) + s->convolve(m_e2);
    const spectralBand_Pair* p = dynamic_cast<const spectralBand_Pair*>(b);
    if (p != 0) return p->convolve(m_e1) + p->convolve(m_e2);
    return 0.0;
}

bool spectralBand::overlaps(const spectralBand& b)
{
  double fc1 = getFrequency();
  double fb1 = getBandwidth();

  if (fb1 == 0.0)
  {
    return false;
  }

  double f01 = fc1 - fb1 / 2.0;
  double f11 = fc1 + fb1 / 2.0;

  double fc2 = b.getFrequency();
  double fb2 = b.getBandwidth();

  if (fb2 == 0.0)
  {
    return false;
  }

  double f02 = fc2 - fb2 / 2.0;
  double f12 = fc2 + fb2 / 2.0;

  boost::numeric::interval<double> f1(f01, f11);
  boost::numeric::interval<double> f2(f02, f12);

  return boost::numeric::overlap(f1, f2); 
}

spectralBand* spectralBand_Square::makeBand(double frequency, enum ChBandwidth bw, const char* tag, PartitionData* partitionData, int radioOverlayID)
{
    return partitionData->theSpectrum.make(frequency, bw, tag, radioOverlayID);
}

void MESSAGE_SetSpectralBand(Node*, Message* msg, const spectralBand* band) 
{
  msg->m_band = const_cast<spectralBand*>(band);
}

void MESSAGE_SetSpectralBand(Node* node, Message* msg, int phyIndex, int channelIndex) 
{
    const spectralBand* b = node->getRadioBand(phyIndex);
    MESSAGE_SetSpectralBand(node, msg, b);
}

spectralBand* MESSAGE_GetSpectralBand(const Message* msg) 
{
  return const_cast<spectralBand*>(msg->m_band);
}

void MESSAGE_CopySpectralBand(Node* node, Message* dest, Message* src) 
{
  dest->m_band = src->m_band;
}

std::ostream& spectralBand::fmt(const char* tag, std::ostream& os) const {
    os << tag;
    this->fmt(os);
    return os;
}

std::ostream& spectralBand::fmt(std::ostream& os) const { return os;}

std::ostream& spectralBand_Pair::fmt(std::ostream& os) const {
    m_e1->fmt(os);
    m_e2->fmt(os);
    return os;
}
 
std::ostream& spectralBand_Square::fmt(std::ostream& os) const {
    os << "(" << m_center << ',' << m_width << ")";
    return os;
}

std::ostream& spectrum::fmt(std::ostream& os) const {
    for (size_t i = 0; i < m_elements.size(); ++i) {
      os << " RadioID:" << i << " ";
       std::vector<spectralBand*> elements = m_elements[i];
       int nlCount = 0;
       for (size_t j = 0; j < elements.size(); ++j) {
         if (elements[j] == 0)
         {
           if (nlCount == 0) 
           {
             std::cout << "(null) ";
           }
           nlCount++;
         }
         else
         {
           if (nlCount > 1)
           {
             std::cout << "X " << nlCount << " ";
             nlCount = 0;
           }
           elements[j]->fmt(" ", os);
         }
       }
       if (nlCount > 1)
       {
         std::cout << "X " << nlCount << " ";
       }
    }
    return os;
}

#ifdef USE_BOOST_ARCHIVE
template<class Archive> void spectralBand::serialize(Archive & ar, const unsigned int version) {
}
 
template<class Archive> void spectralBand_Pair::serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::base_object<spectralBand>(*this);
    ar & m_e1;
    ar & m_e2;
    ar & m_name;
}

template<class Archive> void spectralBand_Square::serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::base_object<spectralBand>(*this);
    ar & m_center;
    ar & m_width;
}

template<class Archive> void spectralBand_80211b::serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::base_object<spectralBand_DSSS>(*this);
}

template<class Archive> void spectralBand_80211a::serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::base_object<spectralBand_OFDM>(*this);
}

template<class Archive> void spectralBand_80211ac20::serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::base_object<spectralBand_OFDM>(*this);
}
template<class Archive> void spectralBand_80211ac40::serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::base_object<spectralBand_OFDM>(*this);
}
template<class Archive> void spectralBand_80211ac80::serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::base_object<spectralBand_OFDM>(*this);
}
template<class Archive> void spectralBand_80211ac160::serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::base_object<spectralBand_OFDM>(*this);
}

template<class Archive> void spectralBand_80211ac160disc::serialize(Archive & ar, const unsigned int version) {
    ar & boost::serialization::base_object<spectralBand_Pair>(*this);
}


BOOST_CLASS_EXPORT_GUID(spectralBand, "spectralBand");
BOOST_CLASS_EXPORT_GUID(spectralBand_Pair, "spectralBand_Pair");
BOOST_CLASS_EXPORT_GUID(spectralBand_Square, "spectralBand_Square");
BOOST_CLASS_EXPORT_GUID(spectralBand_80211b, "spectralBand_80211b");
BOOST_CLASS_EXPORT_GUID(spectralBand_80211a, "spectralBand_80211a");
BOOST_CLASS_EXPORT_GUID(spectralBand_80211ac20, "spectralBand_80211ac20");
BOOST_CLASS_EXPORT_GUID(spectralBand_80211ac40, "spectralBand_80211ac40");
BOOST_CLASS_EXPORT_GUID(spectralBand_80211ac80, "spectralBand_80211ac80");
BOOST_CLASS_EXPORT_GUID(spectralBand_80211ac160, "spectralBand_80211ac160");
BOOST_CLASS_EXPORT_GUID(spectralBand_80211ac160disc, "spectralBand_80211ac160disc");
#endif


