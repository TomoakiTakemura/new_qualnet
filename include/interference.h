#ifndef INTERFERENCE_H
#define INTERFERENCE_H

#include <set>
#include <vector>
#include <boost/numeric/interval.hpp>

#include <math.h>

#include "propagation.h"
#include "spectrum.h"

/// \brief represent an interval of time
class period 
{
    typedef boost::numeric::interval<clocktype> interval_t;

public:

    period(clocktype t1, clocktype t2) : m_interval(t1, t2) { }
    period(const Message* msg);
    period(const PropRxInfo* prx);

    clocktype overlap(const period& other) 
    {
        if (!boost::numeric::overlap(m_interval, other.m_interval)) 
        {
          return 0;
        }

        interval_t dt = boost::numeric::intersect(m_interval, other.m_interval);

        return boost::numeric::width(dt);
    }

    bool in(clocktype t) const 
    {
        // boost::numeric::in(t, m_interval);
        return t >= lower() && t < upper(); 
    }

    clocktype upper() const {return m_interval.upper();}
    clocktype lower() const {return m_interval.lower();}

    clocktype duration() const {return m_interval.upper() - m_interval.lower();}

private:

    interval_t m_interval;
};

std::ostream& operator<<(std::ostream& os, const period& p);


/// \brief a signal
///
/// A single message stored in such a manner as to be useful
/// for noise contribution calcuations.
class PropOverlappingSignal 
{
public:

    PropOverlappingSignal(Node* node, Message* msg, 
                          clocktype start, clocktype duration);

    PropOverlappingSignal(Node* node, Message* msg, spectralBand* b = 0);

    ~PropOverlappingSignal();
    std::ostream& fmt(std::ostream& os) const;

private:

    friend class PropInterference;
    void init(Node* node, Message* msg);
    std::vector<double> m_powerAtPhy;
    PropRxInfo* m_rxInfo;
    spectralBand* m_band;
    period m_times;
};

inline std::ostream& operator<<(std::ostream& os, 
                                const PropOverlappingSignal& p) 
{
  return p.fmt(os);
}

/// \brief obtain the interference and noise in the location of a node
///
/// This will contain all of the signals needed to calcuate noise for any
/// band, along with procedures for the noise calcuation and manage the messages.
/// The class is intended to be a member of Node.  The phy can call any of the
/// interference() or noise() functions at any time as long as it has access to
/// the appropriate Node pointer.

class PropInterference 
{
protected: 
  double totalBandPower_mW(const spectralBand* band, clocktype start, int phyIndex);

    double totalBandPower_mW(const spectralBand* band, clocktype start,
                             clocktype duration, int phyIndex);

    double totalBandPower_mW(Message* msg, const spectralBand* band, int phyIndex);
    double totalBandPower_mW(Message* msg, int phyIndex);

    double totalBandPower_mW(const PropRxInfo* prx, const spectralBand* band, 
                             clocktype start_time, int phyIndex);

    double totalBandPower_mW(const PropRxInfo* prx, const spectralBand* band, int phyIndex);
    double totalBandPower_mW(const PropRxInfo* prx, int phyIndex);

    double noise_mW(int phyIndex, double bandwidth);
    double noise_mW(int phyIndex, const spectralBand* band);
    double noise_mW(int phyIndex, int channelIndex);
    
    double signalPower_mW(const Message* msg, const spectralBand* band,
                          clocktype start, clocktype duration, int phyIndex);

    double signalPower_mW(const Message* msg, const spectralBand* band, int phyIndex);
    double signalPower_mW(const Message* msg, int phyIndex);

    double signalPower_mW(const PropRxInfo* prx, const spectralBand* band,
                          clocktype start, clocktype duration, int phyIndex);

    double signalPower_mW(const PropRxInfo* prx, const spectralBand* band,
                          clocktype start, int phyIndex);

    double signalPower_mW(const PropRxInfo* prx, const spectralBand* band, int phyIndex);

    double signalPower_mW(const PropRxInfo* prx, int phyIndex);

public:

    // the constructor is part of the construction of Node.
     PropInterference(Node* n = 0) : m_node(n), m_signals(0){ }
    ~PropInterference();

    void prune(clocktype now);

    /// \brief add a message to the list of messages at this node
    ///
    /// \param Node* node the node that contains this PropInterference object
    /// \param Message* msg the message to add 
    void insert(Node* node, Message* msg);

    /// \brief return the thermal noise (in dBm) in a band for a given phy.
    double n(int phyIndex, double bandwidth);
    double n(int phyIndex, const spectralBand* band);
    double n(int phyIndex, int channelIndex);

    /// \brief return the signal strength (in dBm) in a message 
    double s(const Message* msg, const spectralBand* band,
             clocktype start, clocktype duration, int phyIndex);

    double s(const PropRxInfo* prx, const spectralBand* band,
             clocktype start, clocktype duration, int phyIndex);

    double s(const PropRxInfo* prx, const spectralBand* band, int phyIndex);
    double s(const PropRxInfo* prx, int phyIndex);
    double s(const Message* msg, const spectralBand* band, int phyIndex);
    double s(const Message* msg, int phyIndex);

    /// \brief noise (in mW) in a band for a given phy
    double ni(double total_mW, double signal_mW);
    double ni(const PropRxInfo* prx, const spectralBand* band, int phyIndex);
    double ni(const PropRxInfo* prx, int phyIndex);
    double ni(Message* msg, const spectralBand* band, int phyIndex);
    double ni(Message* msg, int phyIndex);
    double ni(const PropRxInfo* prx, const spectralBand* band, clocktype start, int phyIndex);

    double ni(const PropRxInfo* prx, const spectralBand* band, clocktype start, 
              clocktype duration, int phyIndex);

    /// \brief return signal strength indicator total power in band in dBm
    double rssi(const PropRxInfo* prx, const spectralBand* band, int phyIndex, 
                clocktype start, clocktype duration);

    double rssi(const PropRxInfo* prx, const spectralBand* band, int phyIndex);
    double rssi(const PropRxInfo* prx, int phyIndex);

    double rssi(const spectralBand* band, clocktype start, int phyIndex);

    double rssi(const spectralBand* band, clocktype start,
                clocktype duration, int phyIndex);

    double rssi(Message* msg, const spectralBand* band, int phyIndex);
    double rssi(Message* msg, int phyIndex);

    /// \brief signal to noise ratio. log10(signal/noise) 
    double snr(Message* msg, const spectralBand* band, clocktype start_time,
               clocktype duration, int phyIndex);

    double snr(Message* msg, const spectralBand* band, int phyIndex);
    double snr(Message* msg, int phyIndex);

    double snr(PropRxInfo* rxInfo, const spectralBand* band, clocktype start_time,
               clocktype duration, int phyIndex);

    double snr(PropRxInfo* rxInfo, const spectralBand* band, int phyIndex);
    double snr(PropRxInfo* rxInfo, int phyIndex);

    std::ostream& fmt(std::ostream& os) const;

private:

    Node* m_node;  // the node that this object is attached to.

    typedef std::set<PropOverlappingSignal*> SignalSet;
    typedef std::set<PropOverlappingSignal*>::iterator SignalSetIterator;

    // Want to use an object of this class inside node.  Unfortunately node is not
    // constructed. An area of memory is allocated as a void* and cast to Node*
    // This means can not use objects that need construction.  Get round it by
    // using pointers to the object and construting a new object on first reference

    SignalSet* m_signals;
};

inline std::ostream& operator<<(std::ostream& os, const PropInterference& p) 
{
  return p.fmt(os);
}

#endif // INTERFERENCE_H
