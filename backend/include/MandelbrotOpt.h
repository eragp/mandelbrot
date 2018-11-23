#pragma once
#include "Fractal.h"
class MandelbrotOpt : public Fractal {
   public:
    void calculateFractal(long double* cReal, long double* cImaginary, int maxIteration, int vectorLength, int* dest);
    int calculateFractalNonParallel(long double cReal, long double cImaginary, int maxIteration);
};
