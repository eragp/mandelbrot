#include "Mandelbrot32.h"

int calculateFractalNonParallel32(precision_t cReal, precision_t cImaginary, int maxIteration){
    int i = 0;
    float zReal = 0.0;
    float zImaginary = 0.0;
    while (i < maxIteration && zReal * zReal + zImaginary * zImaginary < 4.0) {
        float nextZReal = (zReal * zReal - zImaginary * zImaginary) + (float) cReal;
        float nextZImaginary = 2 * (zReal * zImaginary) + (float) cImaginary;
        zReal = nextZReal;
        zImaginary = nextZImaginary;
        i++;
    }
    return i;
}

// Non-simd-izable version
void Mandelbrot32::calculateFractal(precision_t* cReal, precision_t* cImaginary, int maxIteration, int vectorLength, int* dest) {
    for(int j = 0; j < vectorLength; j++){
        dest[j] = calculateFractalNonParallel32(cReal[j], cImaginary[j], maxIteration);
    }
}