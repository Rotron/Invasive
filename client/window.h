#pragma once
#include <cmath>

template <int alpha>
class KaiserBessel {
public:
    double operator()(int n, int x)
    {
      double rad = M_PI * alpha;
      int np = static_cast<int>(n - 1) / 2;
      return bessel(rad * std::sqrt(1 - std::pow(static_cast<double>(x - np) / np, 2))) / bessel(rad);
    }

private:
    double bessel(double x)
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

};

class Flattop {
public:
    double operator()(int n, int x)
    {
        double np = x / (n - 1);
        return 1 - 1.93 * cos(2 * M_PI * np) + 1.29 * cos(4 * M_PI * np) - 0.388 * cos(6 * M_PI * np) + 0.032 * cos(8 * M_PI * np);
    }
};
