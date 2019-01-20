#include "Mandelbrot.h"

int calculateFractalNonParallel(precision_t cReal, precision_t cImaginary, unsigned short int maxIteration){
    int i = 0;
    precision_t zReal = 0.0;
    precision_t zImaginary = 0.0;
    while (i < maxIteration && zReal * zReal + zImaginary * zImaginary < 4.0) {
        precision_t nextZReal = (zReal * zReal - zImaginary * zImaginary) + cReal;
        precision_t nextZImaginary = 2 * (zReal * zImaginary) + cImaginary;
        zReal = nextZReal;
        zImaginary = nextZImaginary;
        i++;
    }
    return i;
}

// Non-simd-izable version
void Mandelbrot::calculateFractal(precision_t* cReal, precision_t* cImaginary, unsigned short int maxIteration, unsigned int vectorLength, unsigned short int* dest) {
    for(unsigned int j = 0; j < vectorLength; j++){
        dest[j] = calculateFractalNonParallel(cReal[j], cImaginary[j], maxIteration);
    }
}