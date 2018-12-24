#include "Fractal.h"

Fractal::~Fractal() {}

precision_t Fractal::deltaReal(precision_t maxReal, precision_t minReal, int xRes) {
    return (maxReal - minReal) / xRes;
}

precision_t Fractal::deltaImaginary(precision_t maxImaginary, precision_t minImaginary, int yRes) {
    return (maxImaginary - minImaginary) / yRes;
}

