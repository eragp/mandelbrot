#include "Mandelbrot32.h"
#include <arm_neon.h>

int calculateFractalNonParallel32(precision_t cReal, precision_t cImaginary, int maxIteration){
    int i = 0;
    float32_t zReal = 0.0;
    float32_t zImaginary = 0.0;
    while (i < maxIteration && zReal * zReal + zImaginary * zImaginary < 4.0) {
        float32_t nextZReal = (zReal * zReal - zImaginary * zImaginary) + (float32_t) cReal;
        float32_t nextZImaginary = 2 * (zReal * zImaginary) + (float32_t) cImaginary;
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