#pragma once
#include "Fractal.h"
class Mandelbrot : public Fractal {
   public:
    int calculateFractal(double cReal, double cImaginary, int maxIteration);
};
