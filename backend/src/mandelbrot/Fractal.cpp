#include "Fractal.h"

Fractal::~Fractal() {}

precision_t Fractal::deltaReal(precision_t maxReal, precision_t minReal, int width) {
    return (maxReal - minReal) / width;
}

precision_t Fractal::deltaImaginary(precision_t maxImaginary, precision_t minImaginary, int height) {
    return (maxImaginary - minImaginary) / height;
}