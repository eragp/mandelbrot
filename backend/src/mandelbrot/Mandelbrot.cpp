#include "Mandelbrot.h"

// Non-simd-izable version
void Mandelbrot::calculateFractal(long double* cReal, long double* cImaginary, int maxIteration, int vectorLength, int* dest) {
    for(int j = 0; j < vectorLength; j++){
        dest[j] = calculateFractalNonParallel(cReal[j], cImaginary[j], maxIteration);
    }
}

int Mandelbrot::calculateFractalNonParallel(long double cReal, long double cImaginary, int maxIteration){
    int i = 0;
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
