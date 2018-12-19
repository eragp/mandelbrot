#pragma once
#include "Fractal.h"
class MandelbrotSIMD32 : public Fractal {
   public:
    void calculateFractal(precision_t* cReal, precision_t* cImaginary, int maxIteration, int vectorLength, int* dest);
};
