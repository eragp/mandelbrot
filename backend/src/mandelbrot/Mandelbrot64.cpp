#include "Mandelbrot64.h"
#include <arm_neon.h>

int calculateFractalNonParallel64(precision_t cReal, precision_t cImaginary, int maxIteration){
    int i = 0;
    float64_t zReal = 0.0;
    float64_t zImaginary = 0.0;
    while (i < maxIteration && zReal * zReal + zImaginary * zImaginary < 4.0) {
        float64_t nextZReal = (zReal * zReal - zImaginary * zImaginary) + (float64_t) cReal;
        float64_t nextZImaginary = 2 * (zReal * zImaginary) + (float64_t) cImaginary;
        zReal = nextZReal;
        zImaginary = nextZImaginary;
        i++;
    }
    return i;
}

// Non-simd-izable version
void Mandelbrot64::calculateFractal(precision_t* cReal, precision_t* cImaginary, int maxIteration, int vectorLength, int* dest) {
    for(int j = 0; j < vectorLength; j++){
        dest[j] = calculateFractalNonParallel64(cReal[j], cImaginary[j], maxIteration);
    }
}