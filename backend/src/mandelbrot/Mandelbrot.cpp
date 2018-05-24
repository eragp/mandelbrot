#include "Mandelbrot.h"

int Mandelbrot::calculateFractal(double cReal, double cImaginary, int maxIteration) {
    int i = 0;
    double zReal = 0.0;
    double zImaginary = 0.0;
    while (i < maxIteration && zReal * zReal + zImaginary * zImaginary < 4.0) {
        double nextZReal = (zReal * zReal - zImaginary * zImaginary) + cReal;
        double nextZImaginary = 2 * (zReal * zImaginary) + cImaginary;
        zReal = nextZReal;
        zImaginary = nextZImaginary;
        i++;
    }
    return i;
}
