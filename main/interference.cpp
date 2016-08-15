#include "main.h"
#include "interference.h"
#include "propagation.h"
#include "antenna.h"
#include "message.h"
#include "node.h"


period::period(const Message* msg) 
{
  PropRxInfo* prx = (PropRxInfo*)MESSAGE_ReturnInfo(msg);
  clocktype start = prx->rxStartTime;
  clocktype duration = prx->duration;
  interval_t p(start, start + duration);
  m_interval = p;  
  
}

period::period(const PropRxInfo* prx) 
{
  clocktype start = prx->rxStartTime;
  clocktype duration = prx->duration;
  interval_t p(start, start + duration);
  m_interval = p;  
}

class myclock 
{
public:
  myclock(clocktype c) : m_c(c) { }
  clocktype get() const {return m_c;}
private:
  const clocktype m_c;
};

static clocktype epoch;

static std::ostream& operator <<(std::ostream& os, myclock c) 
{
  if (epoch == 0) epoch = c.get();
  os << (double)(c.get() - epoch)/SECOND;
  return os;
}

std::ostream& operator <<(std::ostream& os, const period& p) 
{
  if (epoch == 0) epoch = p.lower();
  os << "(" << myclock(p.lower()) << "-" << myclock(p.upper()) << ")";
  return os;
}

PropOverlappingSignal::PropOverlappingSignal(Node* node, Message* msg, clocktype start, clocktype duration) :
    m_band(MESSAGE_GetSpectralBand(msg)),
    m_times(start, start + duration) 
{
    init(node, msg);
}

PropOverlappingSignal::PropOverlappingSignal(Node* node, Message* msg, spectralBand* b) :
  m_band(b != 0 ? b :  MESSAGE_GetSpectralBand(msg)),
    m_times(msg)
{
    init(node, msg);
}

void PropOverlappingSignal::init(Node* node, Message* msg) 
{
  PropRxInfo* rxInfo = (PropRxInfo*)MESSAGE_ReturnInfo(msg);
  int nPhy = node->numberPhys;
  m_rxInfo = rxInfo;
  m_powerAtPhy.resize(nPhy);

  for (int i = 0; i < nPhy; i++) 
  {
    double antennaGain =  ANTENNA_GainForThisSignal(node, i, rxInfo);
    m_powerAtPhy[i] = NON_DB(rxInfo->rxPower_dBm +  antennaGain);

    //m_powerAtPhy[i] = NON_DB(rxInfo->rxPower_dBm);
  }

}
   
PropOverlappingSignal::~PropOverlappingSignal() 
{
  m_band = 0;
}

std::ostream& PropOverlappingSignal::fmt(std::ostream& os) const 
{
  os << m_band << " " << m_times << " ";

  for (size_t i = 0; i < m_powerAtPhy.size(); i++) 
  {
    os << " " << m_powerAtPhy[i];
  }

  return os;
}

void PropInterference::insert(Node* node, Message* msg) 
{
  spectralBand* sb = MESSAGE_GetSpectralBand(msg);

  if (sb == 0) return;
  if (m_node == 0) m_node = node;

  assert(node == m_node);

  PropOverlappingSignal* sg = new PropOverlappingSignal(node, msg, sb);

  if (m_signals == NULL) m_signals = new std::set<PropOverlappingSignal*>;
  

#if DEBUG
  cout << "node:" << node->nodeId << " " << "insert:" << *sg << endl;
#endif

  m_signals->insert(sg);
}

PropInterference::~PropInterference() 
{
  delete m_signals;
  m_signals = 0;
}

double PropInterference::s(const PropRxInfo* prx, int phyIndex)
{
  spectralBand* msgBand = MESSAGE_GetSpectralBand(prx->txMsg);
  double s_calc = s(prx, msgBand, phyIndex);
  return s_calc;
}

double PropInterference::s(const Message* msg, const spectralBand* band, int phyIndex)
{
  PropRxInfo* rxInfo = (PropRxInfo*)MESSAGE_ReturnInfo(msg);
  assert(rxInfo != NULL);

  return s(msg, band, rxInfo->rxStartTime, rxInfo->duration, phyIndex);
}

double PropInterference::s(const Message* msg, int phyIndex)
{
  spectralBand* msgBand = MESSAGE_GetSpectralBand(msg);
  double s_calc = s(msg, msgBand, phyIndex);
  return s_calc;
}

double PropInterference::snr(PropRxInfo* rxInfo, const spectralBand* band, clocktype start_time,
                            clocktype duration, int phyIndex)
{

  double signal_dBm = s(rxInfo, band, start_time, duration, phyIndex);
  double ni_dBm = ni(rxInfo, band, start_time, duration, phyIndex);

#if DEBUG
  std::cout << "snr() signal(dBm): " << signal_dBm << " ni(dBm): " << ni_dBm << std::endl;
#endif

  return signal_dBm - ni_dBm;
}

double PropInterference::snr(PropRxInfo* rxInfo, const spectralBand* band, int phyIndex)
{
  clocktype duration = rxInfo->duration;
  clocktype start_time = rxInfo->rxStartTime;

  return snr(rxInfo, band, start_time, duration, phyIndex);
}

double PropInterference::snr(PropRxInfo* rxInfo, int phyIndex)
{
  spectralBand* msgBand = MESSAGE_GetSpectralBand(rxInfo->txMsg);
  double snr_calc = snr(rxInfo, msgBand, phyIndex);
  return snr_calc;
}

double PropInterference::ni(double total_mW, double signal_mW)
{
  const double k_logZero(-200.0);
  double ni_mW = total_mW - signal_mW;

#if DEBUG
  printf("total(mW): %0.15lf signal(mW): %0.15lf ni(mW): %0.15lf\n",
         total_mW, signal_mW, ni_mW);
#endif

  if (ni_mW > 0)
  {
    return IN_DB(ni_mW);
  }

  return k_logZero;
}

double PropInterference::ni(const PropRxInfo* prx, const spectralBand* band,
                            clocktype start, clocktype duration, int phyIndex)
{
  double total_mW = totalBandPower_mW(band, start, duration, phyIndex);
  double signal_mW = signalPower_mW(prx, band, start, duration, phyIndex);

  return ni(total_mW, signal_mW);
}

double PropInterference::ni(const PropRxInfo* prx, const spectralBand* band,
                            clocktype start, int phyIndex)
{
  double total_mW = totalBandPower_mW(band, start, phyIndex);
  double signal_mW = signalPower_mW(prx, band, start, phyIndex);

  return ni(total_mW, signal_mW);
}

double PropInterference::ni(const PropRxInfo* prx, const spectralBand* band, int phyIndex)
{
  double total_mW = totalBandPower_mW(prx, band, phyIndex);
  double signal_mW = signalPower_mW(prx, band, phyIndex);

  return ni(total_mW, signal_mW);
}

double PropInterference::ni(const PropRxInfo* prx, int phyIndex)
{
  double total_mW = totalBandPower_mW(prx, phyIndex);
  double antennaGain = ANTENNA_GainForThisSignal(m_node, phyIndex, const_cast<PropRxInfo*>(prx));
  double signal_mW = NON_DB(prx->rxPower_dBm + antennaGain);

  return ni(total_mW, signal_mW);
}

double PropInterference::ni(Message* msg, const spectralBand* band, int phyIndex)
{
  double total_mW = totalBandPower_mW(msg, band, phyIndex);
  double signal_mW = signalPower_mW(msg, band, phyIndex);

  return ni(total_mW, signal_mW);
}

double PropInterference::ni(Message* msg, int phyIndex)
{
   double total_mW = totalBandPower_mW(msg, phyIndex);
   double signal_mW = signalPower_mW(msg, phyIndex);

   return ni(total_mW, signal_mW);
}

double PropInterference::n(int phyIndex, double bandwidth)
{
  return IN_DB(noise_mW(phyIndex, bandwidth));
}

double PropInterference::n(int phyIndex, const spectralBand* band)
{
  return IN_DB(noise_mW(phyIndex, band));
}

double PropInterference::n(int phyIndex, int channelIndex)
{
  return IN_DB(noise_mW(phyIndex, channelIndex));
}

double PropInterference::s(const Message* msg, const spectralBand* band,
                           clocktype start, clocktype duration, int phyIndex)
{
  double signal_mW = signalPower_mW(msg, band, start, duration, phyIndex);
  double signal_dBm = IN_DB(signal_mW);

#if DEBUG
  std::cout << " signal(mW): " << signal_mW << " signal(dBm): " << signal_dBm << std::endl;
#endif

  return signal_dBm;
}

double PropInterference::s(const PropRxInfo* prx, const spectralBand* band,
                           clocktype start, clocktype duration, int phyIndex)
{
  return IN_DB(signalPower_mW(prx, band, start, duration, phyIndex));
}

double PropInterference::s(const PropRxInfo* prx, const spectralBand* band, int phyIndex)
{
  return s(prx, band, prx->rxStartTime, prx->duration, phyIndex);
}







double PropInterference::snr(Message* msg, const spectralBand* band, clocktype start_time, 
                            clocktype duration, int phyIndex)
{
  double signal_dBm = s(msg, band, start_time, duration, phyIndex);
  double ni_dBm = ni(msg, phyIndex);

#if DEBUG
  std::cout << "snr() signal(dBm): " << signal_dBm << " ni(dBm): " << ni_dBm << std::endl;
#endif

  return signal_dBm - ni_dBm;
}

double PropInterference::snr(Message* msg, const spectralBand* band, int phyIndex)
{
  PropRxInfo* rxInfo = (PropRxInfo*)MESSAGE_ReturnInfo(msg);
  assert(rxInfo != NULL);

  clocktype duration = rxInfo->duration;
  clocktype start_time = rxInfo->rxStartTime;

  return snr(msg, band, start_time, duration, phyIndex);
}

double PropInterference::snr(Message* msg, int phyIndex)
{
  spectralBand* msgBand = MESSAGE_GetSpectralBand(msg);
  double snr_calc = snr(msg, msgBand, phyIndex);
  return snr_calc;
}

double PropInterference::rssi(const PropRxInfo* prx, const spectralBand* band, int phyIndex)
{
  return IN_DB(totalBandPower_mW(prx, band, phyIndex));
}

double PropInterference::rssi(const spectralBand* band, clocktype start, int phyIndex)
{
  return IN_DB(totalBandPower_mW(band, start, phyIndex));
}

double PropInterference::rssi(const PropRxInfo* prx, int phyIndex)
{
  return IN_DB(totalBandPower_mW(prx, phyIndex));
}

double PropInterference::rssi(const spectralBand* band, clocktype start, 
                              clocktype duration, int phyIndex)
{
  return IN_DB(totalBandPower_mW(band, start, duration, phyIndex));
}

double PropInterference::rssi(Message* msg, const spectralBand* band, int phyIndex)
{
  PropRxInfo* rxInfo = (PropRxInfo*)MESSAGE_ReturnInfo(msg);
  assert(rxInfo != NULL);

  clocktype duration = rxInfo->duration;
  clocktype start_time = rxInfo->rxStartTime;

  return rssi(band, start_time, duration, phyIndex);
}

double PropInterference::rssi(Message* msg, int phyIndex)
{
  spectralBand* band = MESSAGE_GetSpectralBand(msg);
  double rssi_calc = rssi(msg, band, phyIndex);
  return rssi_calc;
}

double PropInterference::totalBandPower_mW(const spectralBand* band, clocktype start, int phyIndex)
{
  if (m_signals == NULL) return 0.0;

  
  double powerTotal = 0;
  std::set<PropOverlappingSignal*>::iterator it = m_signals->begin();

  while (it != m_signals->end()) 
  {
    PropOverlappingSignal* val = *it++;

    if (val->m_times.in(start)) 
    {
#if DEBUG
      std::cout << "band1: " << band << " band2: " << val->m_band << std::endl;
#endif

        double bandFraction 
          = band->convolve(val->m_band) / val->m_band->getBandwidth();

        double power_mW =  val->m_powerAtPhy[phyIndex];

#if DEBUG
        std::cout << "bandFraction: " << bandFraction << " power(mW): " << power_mW << std::endl;
#endif

        powerTotal += bandFraction * power_mW;
    }
  }

  double noisePower_mW = noise_mW(phyIndex, band);
  powerTotal += noisePower_mW;

  return powerTotal;
}

double PropInterference::totalBandPower_mW(const spectralBand* band, clocktype start, 
                                           clocktype duration, int phyIndex) 
{
  if (m_signals == NULL) return 0.0;
  if (band == NULL) return 0.0;
  if (duration == 0) return 0.0;

  double energyTotal = 0.0;
  double timeTotal(0.0);

  period interval(start, duration + start);

  std::set<PropOverlappingSignal*>::iterator it = m_signals->begin();
  while (it != m_signals->end()) 
  {
    PropOverlappingSignal& val = *(*it); 
    it++;

    double bandOverlap = band->convolve(val.m_band);
    double bandFraction = bandOverlap / val.m_band->getBandwidth();

    double timeOverlap = (double)interval.overlap(val.m_times);
    // double timeFraction = timeOverlap / (double)duration;

    double power_mW =  val.m_powerAtPhy[phyIndex];

#if DEBUG
    std::cout << "bandFraction: " << bandFraction << " timeOverlap: " << timeOverlap
              << " power(mW): " << power_mW << std::endl;
#endif
    assert(timeOverlap <= duration);

    // This actually gives energy, not power
    energyTotal += timeOverlap * bandFraction * val.m_powerAtPhy[phyIndex];
  }

  double powerTotal = energyTotal / (double)duration;

#if DEBUG
  std::cout << " powerTotal(mW): " << powerTotal << std::endl;
#endif

  double noisePower_mW = noise_mW(phyIndex, band);

#if DEBUG
  std::cout << "noiseTotal(mW): " << noisePower_mW << std::endl;
#endif

  powerTotal += noisePower_mW;

  return powerTotal;
}

double PropInterference::totalBandPower_mW(Message* msg, const spectralBand* band, int phyIndex)
{
  PropRxInfo* rxInfo = (PropRxInfo*)MESSAGE_ReturnInfo(msg);
  assert(rxInfo != NULL);

  clocktype start = rxInfo->rxStartTime;
  clocktype duration = rxInfo->duration;

  return totalBandPower_mW(band, start, duration, phyIndex);
}

double PropInterference::totalBandPower_mW(Message* msg, int phyIndex)
{
  spectralBand* band = MESSAGE_GetSpectralBand(msg);
  double totalBandPower_mW_calc = totalBandPower_mW(msg, band, phyIndex);
  return totalBandPower_mW_calc;
}

double PropInterference::totalBandPower_mW(const PropRxInfo* prx, const spectralBand* band, int phyIndex)
{
  return totalBandPower_mW(band, prx->rxStartTime, phyIndex);
}

double PropInterference::totalBandPower_mW(const PropRxInfo* prx, int phyIndex)
{
  return totalBandPower_mW(m_node->getRadioBand(phyIndex), 
                           prx->rxStartTime, phyIndex);
}

double PropInterference::noise_mW(int phyIndex, double bandwidth) 
{
  assert(m_node != NULL);
  assert(phyIndex >= 0 && phyIndex < m_node->numberPhys);

  double noise_mW_Hz =  m_node->phyData[phyIndex]->noise_mW_hz;
  double f_Hz = (double)bandwidth;

#if DEBUG
      std::cout << "noise/Hz(mW): " << noise_mW_Hz << " f(Hz): " << f_Hz << std::endl;
#endif
  return noise_mW_Hz * f_Hz;
}

double PropInterference::noise_mW(int phyIndex, const spectralBand* band) 
{
  return noise_mW(phyIndex, band->getBandwidth());
}

double PropInterference::noise_mW(int phyIndex, int channelIndex) 
{
  if (m_node == NULL) return 0.0;

  assert(phyIndex >= 0 && phyIndex < m_node->numberPhys);

  return noise_mW(phyIndex, m_node->getRadioBand(phyIndex));
}

double PropInterference::signalPower_mW(const Message* msg, const spectralBand* band, 
                                        clocktype start, clocktype duration, int phyIndex) 
{
    if (msg == NULL) return 0.0;

    spectralBand* msgBand = MESSAGE_GetSpectralBand(msg);

    double bandFraction = (band == NULL || msgBand == NULL) 
      ? 1.0 
      : band->convolve(msgBand)/msgBand->getBandwidth();


/*
    Assume power is constant here:

    period p1(msg);
    period p2(start, start + duration);

    double timeOverlap = (double)p1.overlap(p2)/(double)duration;
*/

    PropRxInfo* rxInfo = (PropRxInfo*)MESSAGE_ReturnInfo(msg);
    double antennaGain = ANTENNA_GainForThisSignal(m_node, phyIndex, const_cast<PropRxInfo*>(rxInfo));
    double signalPower_mW = NON_DB(rxInfo->rxPower_dBm + antennaGain);

#if DEBUG
    std::cout << "MBcc: overlap: " << bandFraction << ", totalSignalPower: "
              << signalPower_mW << " in db:" << rxInfo->rxPower_dBm << std::endl;
#endif

    return signalPower_mW * bandFraction;
}

double PropInterference::signalPower_mW(const Message* msg, const spectralBand* band, int phyIndex)
{
  PropRxInfo* rxInfo = (PropRxInfo*)MESSAGE_ReturnInfo(msg);
  assert(rxInfo != NULL);

  clocktype start_time = rxInfo->rxStartTime;
  clocktype duration = rxInfo->duration;

  return signalPower_mW(msg, band, start_time, duration, phyIndex);
}

double PropInterference::signalPower_mW(const Message* msg, int phyIndex)
{
  spectralBand* msgBand = MESSAGE_GetSpectralBand(msg);
  double s_calc = signalPower_mW(msg, msgBand, phyIndex);
  return s_calc;
}
  
double PropInterference::signalPower_mW(const PropRxInfo* prx, const spectralBand* band, 
                                        clocktype start, clocktype duration, int phyIndex) 
{
    spectralBand* msgBand = MESSAGE_GetSpectralBand(prx->txMsg);

    double bandOverlap = (band == NULL) 
      ? 1.0 
      : band->convolve(msgBand)/msgBand->getBandwidth();


/*
    Assume power is constant here:
    period p1(prx);

    if (duration == 0) duration = p1.upper() - p1.lower();
    if (start == 0) start = p1.lower();

    period p2(start, start + duration);
    double timeOverlap = (double)p1.overlap(p2)/(double)duration;
*/

    double antennaGain = ANTENNA_GainForThisSignal(m_node, phyIndex, const_cast<PropRxInfo*>(prx));
    double totalSignalPower = NON_DB(prx->rxPower_dBm + antennaGain);

    // This would be energy, not power
    // double signal_power_mW = power * bandOverlap * timeOverlap;

    double subband_signal_power = totalSignalPower * bandOverlap;

#if DEBUG
    std::cout << "PBcc: overlap: " << bandOverlap << ", totalSignalPower: "
              << totalSignalPower << std::endl;
#endif

    return subband_signal_power;
}

double PropInterference::signalPower_mW(const PropRxInfo* rxInfo, const spectralBand* band,
                 clocktype now, int phyIndex)
{
  spectralBand* msgBand = MESSAGE_GetSpectralBand(rxInfo->txMsg);

  double bandFraction = (band == NULL || msgBand == NULL)
    ? 1.0 
    : band->convolve(msgBand)/msgBand->getBandwidth();

  clocktype startTime = rxInfo->rxStartTime;
  clocktype endTime = startTime + rxInfo->duration;

  if (now >= startTime && now < endTime)
  {
    double antennaGain = ANTENNA_GainForThisSignal(m_node, phyIndex, const_cast<PropRxInfo*>(rxInfo));
    double signalPower_mW = NON_DB(rxInfo->rxPower_dBm + antennaGain) * bandFraction;
    return signalPower_mW;
  }

  return 0.0;
}

double PropInterference::signalPower_mW(const PropRxInfo* prx, const spectralBand* band, int phyIndex)
{
  clocktype start_time = prx->rxStartTime;
  clocktype duration = prx->duration;

  return signalPower_mW(prx, band, start_time, duration, phyIndex);
}

double PropInterference::signalPower_mW(const PropRxInfo* prx, int phyIndex)
{
  spectralBand* msgBand = MESSAGE_GetSpectralBand(prx->txMsg);
  double s_calc = signalPower_mW(prx, msgBand, phyIndex);
  return s_calc;
}

void PropInterference::prune(clocktype now) 
{
  if (m_signals == NULL) return;

#if DEBUG
  cout << "prune at:" << myclock(now) << endl;
#endif
  // need to find the not yet ended signal that started the longest time ago.
  // lower is begining of signal
  // upper is end time

  clocktype oldest = now;
  std::set<PropOverlappingSignal*>::iterator it = m_signals->begin();
  while (it != m_signals->end()) 
  {
    PropOverlappingSignal* val= *it++;

    if (val->m_times.upper() > now && val->m_times.lower() < oldest)
    {
      oldest = val->m_times.lower();
    }
  }

  it = m_signals->begin();
  while (it != m_signals->end()) 
  {
    PropOverlappingSignal* val = *it++;

    if (val->m_times.upper() > oldest) 
    {
      continue;  // this signal overlaps at least one current signal
    }

#if DEBUG
    cout << "prune:" << *val << endl;
#endif

    m_signals->erase(val);
    delete(val);
  }
}

std::ostream& PropInterference::fmt(std::ostream& os) const 
{
  if (m_signals == NULL) 
  {
    os << "not initialized";
    return os;
  }

  SignalSetIterator it = m_signals->begin();
  if (it == m_signals->end()) os << "empty";

  while (it != m_signals->end()) 
  {
    PropOverlappingSignal* val = *it++;
    os << *val << endl;
  }

  return os;
}
