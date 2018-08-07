#include "Fractal.h"

Fractal::~Fractal() {}

double Fractal::deltaReal(double maxReal, double minReal, int xRes) {
    return (maxReal - minReal) / xRes;
}

double Fractal::deltaImaginary(double maxImaginary, double minImaginary, int yRes) {
    return (maxImaginary - minImaginary) / yRes;
}

