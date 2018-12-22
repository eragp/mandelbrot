#pragma once
#include "Fractal.h"
class Mandelbrot : public Fractal {
   public:
    int calculateFractal(long double cReal, long double cImaginary, unsigned short int maxIteration);
};
