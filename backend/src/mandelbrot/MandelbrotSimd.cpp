#include <stdlib.h>
#include "MandelbrotSimd.h"
#include <arm_neon.h>

// Raspi3 processor
// https://developer.arm.com/products/processors/cortex-a/cortex-a53
// 
// #if __ARM_NEON
//  #include <arm_neon.h>
//  /* Advanced SIMD intrinsics are now available to use.  */
// #endif

// Maximum hardware flp precision: 64 bit
// https://developer.arm.com/products/software-development-tools/compilers/arm-compiler-5/docs/101028/latest/5-feature-test-macros#ssec-hwfp
#if __ARM_FP & 0x8
    #define MAXPRECISION 64
#elif __ARM_FP & 0x4
    #define MAXPRECISION 32
#elif __ARM_FP & 0x2
    #define MAXPRECISION 16
#else
    #define MAXPRECISION 0
#endif

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

int* MandelbrotSimd::calculateFractal(long double* cReal, long double* cImaginary, int maxIteration, int vector_size = 1) {

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
