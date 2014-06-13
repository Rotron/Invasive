#pragma once
#include <vector>
#include <cmath>
#include "ringbuffer.h"

class FIRFilter {
public:
  enum Type {
    FIR_LOWPASS,
    FIR_HIGHPASS,
    FIR_BANDPASS,
    FIR_BANDSTOP
  };

public:
  FIRFilter(Type type, double sampling, double width, double edge1, double edge2 = 0.0);
  double process(double in);
  void setTap(const std::vector<double>& weights);
  size_t size();

private:
  double sinc(double x);
  double kaiser_bessel(int x);
  double bessel(double x);

private:
  std::vector<double> weights_;
  RingBuffer<double> delay_;

};
