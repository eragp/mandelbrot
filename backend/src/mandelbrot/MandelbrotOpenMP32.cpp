#include "MandelbrotOpenMP32.h"
#define float32_t float

int calculateFractalOpenMP32(precision_t cReal, precision_t cImaginary, unsigned short int maxIteration){
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
void MandelbrotOpenMP32::calculateFractal(precision_t* cReal, precision_t* cImaginary, unsigned short int maxIteration, unsigned int vectorLength, unsigned short int* dest) {
    #pragma omp parallel for default(none) num_threads(4) shared(cReal, cImaginary, maxIteration, vectorLength, dest) schedule(nonmonotonic:dynamic, 10)
    for(unsigned int j = 0; j < vectorLength; j++){
        dest[j] = calculateFractalOpenMP32(cReal[j], cImaginary[j], maxIteration);
    }
}
