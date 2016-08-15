#include <math.h>
#include <assert.h>

#include <vector>

#ifndef __MIMO_MAPPER_H__
#define __MIMO_MAPPER_H__

//#define DEBUG_MIMO

namespace MIMO {

class StbcDescription
{
  int d_ant;
  double d_rate;

public:

  StbcDescription(int p_ant, double p_rate)
  : d_ant(p_ant), d_rate(p_rate) { ; }

  double rate() const { return d_rate; }
  int size() const { return d_ant; }
};

namespace Stbc {

class Alamouti : public StbcDescription
{
public:
  Alamouti() : StbcDescription(2, 1.0) { ; }
} ;

class Uncoded : public StbcDescription
{
public:
  Uncoded() : StbcDescription(1, 1.0) { ; }
} ;

}

class Mapper
{
  int d_ntx;
  int d_active_ntx;
  double d_ptx_mW;
  int d_tx_alloc;

public:

  Mapper(int p_ntx, int p_nrx, double p_ptx_mW) 
  : d_active_ntx(std::min(p_ntx, p_nrx)), d_ntx(p_ntx),
    d_ptx_mW(p_ptx_mW), d_tx_alloc(0) 
  { ; }

  Mapper& operator()(const StbcDescription& strm) 
  { 
    d_tx_alloc += strm.size(); 
    assert(d_tx_alloc <= d_active_ntx);

    return *this;
  }

  std::vector<double> operator()()
  {
    assert(d_tx_alloc == d_active_ntx);
    std::vector<double> power_vec(d_ntx, 0.0);

    double rho_mW = d_ptx_mW / (double)d_active_ntx;
    for (int k(0); k < d_active_ntx; k++)
    {
      power_vec[k] = rho_mW;
    }

    return power_vec;
  }

  int size() { return d_ntx; }
};

class Demapper 
{
  bool d_blast;
  std::vector<double> d_snr;

  static double ndb(double u) { return pow(10.0, u/10.0); }
  static double db(double u) { return 10.0 * log10(u); }

  static double plus(double x, double y)
  {
    return db(1.0/(1.0/ndb(x) + 1.0/ndb(y)));
  }

  double d_avg_snr;
  bool d_first_snr;
  double d_kr;
  int d_ss_count;
  int d_nrx;
  int d_ntx;

  typedef Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> matrix_t;

public:

  Demapper(int p_ntx, int p_nrx, double branch_snr_dB,
           const matrix_t& p_lambda, bool p_blast)
  : d_blast(p_blast), d_first_snr(true), d_avg_snr(0.0),
    d_kr((double)p_nrx/(double)p_ntx), d_ss_count(0),
    d_nrx(p_nrx), d_ntx(p_ntx)
  {
#ifdef DEBUG_MIMO
      std::cout << "ntx: " << d_ntx << " nrx: " << d_nrx << std::endl;
#endif
    for (int k(0); k < p_lambda.size(); k++) 
    {
      double lr = 10.0*log10(std::real(p_lambda(k)));
      double siso_snr = lr + branch_snr_dB;
#ifdef DEBUG_MIMO
      std::cout << "siso_snr[" << k << "]: " << siso_snr << " dB" << std::endl;
#endif
      d_snr.push_back(siso_snr);
    }
  }

  Demapper& operator()(int rx0, const StbcDescription& stbc)
  {
    assert(stbc.size() > 0);
    
    const int lb = rx0;
    const int ub = lb + stbc.size() - 1;
    assert(ub < d_snr.size());

    double snr(d_snr.at(lb));
    for (int k(lb+1); k < ub; k++)
    {
      snr = plus(snr, d_snr.at(k));
    }

    if (stbc.rate() != 1.0)
    {
      snr -= 10.0 * log10(stbc.rate());
    }

    if (d_nrx != d_ntx)
    {
      snr += 10.0 * log10(d_kr);
    }

    if (d_first_snr)
    {
      d_avg_snr = snr;
      d_first_snr = false;
    }
    else
    {
      if (!d_blast)
      {
        std::cout << "Warning: overwriting SNR value in demapper, consider using BLAST." << std::endl;
      }

      d_avg_snr = (d_blast) ? plus(d_avg_snr, snr) : snr;
    }

    d_ss_count++;

    return *this;
  }

  double operator()() { assert(d_first_snr == false); return d_avg_snr; }
} ;

}

#endif
