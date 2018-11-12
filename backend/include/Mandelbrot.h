#pragma once
#include "Fractal.h"
class Mandelbrot : public Fractal {
   public:
    void calculateFractal(long double* cReal, long double* cImaginary, int maxIteration, int vectorLength, int* dest);
    int calculateFractalNonParallel(long double cReal, long double cImaginary, int maxIteration);
};
