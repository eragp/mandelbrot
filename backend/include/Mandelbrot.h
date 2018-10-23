#pragma once
#include "Fractal.h"
class Mandelbrot : public Fractal {
   public:
    int* calculateFractal(long double* cReal, long double* cImaginary, int maxIteration, int vector_size);
};
