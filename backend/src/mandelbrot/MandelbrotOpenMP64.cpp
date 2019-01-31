#include "MandelbrotOpenMP64.h"
#define float64_t double

int calculateFractalOpenMP64(precision_t cReal, precision_t cImaginary, int maxIteration){
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
void MandelbrotOpenMP64::calculateFractal(precision_t* cReal, precision_t* cImaginary, unsigned short int maxIteration, unsigned int vectorLength, unsigned short int* dest) {
    #pragma omp parallel for default(none) num_threads(4) shared(cReal, cImaginary, maxIteration, vectorLength, dest) schedule(nonmonotonic:dynamic)
    for(unsigned int j = 0; j < vectorLength; j++){
        dest[j] = calculateFractalOpenMP64(cReal[j], cImaginary[j], maxIteration);
    }
}