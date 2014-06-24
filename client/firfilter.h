#pragma once
#include <vector>
#include <cmath>
#include <functional>
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
  FIRFilter(Type type, const std::function<double(int,int)>& func, double sampling, double width, double edge1, double edge2 = 0.0);
  double process(double in);
  void setTap(const std::vector<double>& weights);
  size_t size();

private:
  static double sinc(double x);

private:
  std::vector<double> weights_;
  RingBuffer<double> delay_;

};
