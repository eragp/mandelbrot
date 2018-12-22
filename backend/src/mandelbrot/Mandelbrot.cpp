#include "Mandelbrot.h"

int Mandelbrot::calculateFractal(long double cReal, long double cImaginary, unsigned short int maxIteration) {
    unsigned short int i = 0;
    long double zReal = 0.0;
    long double zImaginary = 0.0;
    while (i < maxIteration && zReal * zReal + zImaginary * zImaginary < 4.0) {
        long double nextZReal = (zReal * zReal - zImaginary * zImaginary) + cReal;
        long double nextZImaginary = 2 * (zReal * zImaginary) + cImaginary;
        zReal = nextZReal;
        zImaginary = nextZImaginary;
        i++;
    }
    return i;
}
