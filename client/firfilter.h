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
  FIRFilter(Type type, double sampling, double width, double edge1,
            double edge2 = 0.0, const std::function<double(int,int)>& = kaiser_bessel);
  double process(double in);
  void setTap(const std::vector<double>& weights);
  size_t size();

  static double kaiser_bessel(int n, int x);
  static double kaiser_bessel2(int n, int x);
  static double flattop(int n, int x);

private:
  static double sinc(double x);
  static double bessel(double x);

private:
  std::vector<double> weights_;
  RingBuffer<double> delay_;

};
