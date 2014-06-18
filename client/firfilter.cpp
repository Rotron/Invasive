#include "FIRFilter.h"

FIRFilter::FIRFilter(Type type, double sampling, double width, double edge1, double edge2)
{
  int tap_count = std::round(3.1 / (width / sampling));
  if (tap_count % 2 == 0) tap_count++;
  weights_.resize(tap_count);
  delay_.set_capacity(tap_count);

  double f1 = (edge1 / sampling);
  double f2 = (edge2 / sampling);
  for (size_t i = 0; i < weights_.size(); ++i) {
    int m = static_cast<int>(i - weights_.size() / 2);
    switch (type) {
    case FIR_LOWPASS:
      weights_[i] = (2.0 * f1 * sinc(2.0 * M_PI * f1 * m)) * kaiser_bessel(i);
      break;
    case FIR_HIGHPASS:
      weights_[i] = (sinc(M_PI * m) - 2.0 * f1 * sinc(2.0 * M_PI * f1 * m)) * kaiser_bessel(i);
      break;
    case FIR_BANDPASS:
      weights_[i] = 2.0 * (f2 * sinc(2.0 * M_PI * f2 * m) - f1 * sinc(2.0 * M_PI * f1 * m)) * kaiser_bessel(i);
      break;
    case FIR_BANDSTOP:
      weights_[i] = (sinc(M_PI * m) - 2.0 * f2 * sinc(2.0 * M_PI * f2 * m) +
                    2.0 * f1 * sinc(2.0 * M_PI * f1 * m)) * kaiser_bessel(i);
      break;
    }
  }
}

double FIRFilter::process(double in)
{
  double out = 0;
  delay_.push_back(in);
  for (size_t i = 0; i < delay_.size(); ++i) {
    out += delay_[i] * weights_[i];
  }
  return out;
}

void FIRFilter::setTap(const std::vector<double>& weights)
{
  weights_ = weights;
  delay_.set_capacity(weights_.size());
}

double FIRFilter::sinc(double x)
{
  if (x == 0.0) {
    return 1.0;
  }
  else {
    return std::sin(x) / x;
  }
}

size_t FIRFilter::size()
{
  return weights_.size();
}

double FIRFilter::kaiser_bessel(int x)
{
  double alpha = M_PI * 100;
  int np = static_cast<int>(weights_.size() - 1) / 2;
  return bessel(alpha * std::sqrt(1 - std::pow(static_cast<double>(x - np) / np, 2))) / bessel(alpha);
}

double FIRFilter::bessel(double x)
{
  double d = 0.0;
  double ds = 1.0;
  double s = 1.0;
  do {
    d += 2.0;
    ds *= x * x / (d * d);
    s += ds;
  }
  while (ds > s * 1e-6);
  return s;
}
