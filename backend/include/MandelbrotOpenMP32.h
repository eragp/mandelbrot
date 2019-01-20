#pragma once
#include "Fractal.h"
class MandelbrotOpenMP32 : public Fractal {
   public:
    void calculateFractal(precision_t* cReal, precision_t* cImaginary, unsigned short int maxIteration, unsigned int vectorLength,  unsigned short int* dest);
};
