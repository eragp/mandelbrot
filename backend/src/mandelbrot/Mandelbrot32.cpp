#include "Mandelbrot32.h"

#ifdef __ARM_NEON
#include <arm_neon.h>

int calculateFractalNonParallel32(precision_t cReal, precision_t cImaginary, unsigned short int maxIteration){
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

#endif

// Non-simd-izable version
void Mandelbrot32::calculateFractal(precision_t* cReal, precision_t* cImaginary, unsigned short int maxIteration, int vectorLength, unsigned short int* dest) {
    for(int j = 0; j < vectorLength; j++){
        #ifdef __ARM_NEON
        dest[j] = calculateFractalNonParallel32(cReal[j], cImaginary[j], maxIteration);
        #else
        dest[j] = 0;
        #endif
    }
}
