#include <stdlib.h>
#include "Mandelbrot.h"

bool any_left(int maxIteration, int* i, long double* zReal, long double* zImaginary, int vector_size){
    bool any_left = false;
    for(int j = 0; j < vector_size; j++){
        any_left |= (
            i[j] < maxIteration &&
            zReal[j] * zReal[j] + zImaginary[j] * zImaginary[j] < 4.0
        );
    }
    return any_left;
}

int* Mandelbrot::calculateFractal(long double* cReal, long double* cImaginary, int maxIteration, int vector_size = 1) {

    int* i = (int*) calloc(vector_size, sizeof(int));
    long double* zReal = (long double*) calloc(vector_size, sizeof(long double));
    long double* zImaginary = (long double*) calloc(vector_size, sizeof(long double));
    while (any_left(maxIteration, i, zReal, zImaginary, vector_size)) {
        for(int j = 0; j < vector_size; j++){
            long double nextZReal = (zReal[j] * zReal[j] - zImaginary[j] * zImaginary[j]) + cReal[j];
            long double nextZImaginary = 2 * (zReal[j] * zImaginary[j]) + cImaginary[j];
            zReal[j] = nextZReal;
            zImaginary[j] = nextZImaginary;
            i[j]++;
        }
    }
    return i;
}
