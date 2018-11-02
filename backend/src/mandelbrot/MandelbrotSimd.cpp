#include <stdlib.h>
#include "MandelbrotSimd.h"
#include "Mandelbrot.h"

// TODO remove
    #include <arm_neon.h>
// Raspi3 processor
// https://developer.arm.com/products/processors/cortex-a/cortex-a53
// 
#if __ARM_NEON
    #include <arm_neon.h>
    #define SIMD_SUPPORT 1
  /* Advanced SIMD intrinsics are now available to use.  */
#else
    #define SIMD_SUPPORT 0
#endif

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

bool any_left(int maxIteration, uint32x4_t i, float32x4_t zReal, float32x4_t zImaginary, int vector_size){
    bool any_left = false;
    uint32x4_t maxIt = {maxIteration};
    // Compare with maximum Iteration
    maxIt = vcltq_u32(i, maxIt);
    // If any scalar is below (has become 1 in previous computation), we continue
    if(vaddvq_u32(maxIt) > 0){
        return true;
    }
    // Compare absolute value to minimum for convergence
    float32x4_t realSquare = vmul_f32(zReal, zReal);
    float32x4_t imagSquare = vmul_f32(zImaginary, zImaginary);
    float32x4_t absolute = vadd_f32(realSquare, imagSquare);
    uint32x4_t abort_vec = vclt_f32(absolute, {4});
    if(vaddvq_u32(abort_vec) > 0){
        return true;
    }
    return false;
}

/**
 * Currently only accepts vectors of size 2
 **/
int* MandelbrotSimd::calculateFractal(long double* cReal, long double* cImaginary, int maxIteration, int vector_size = 1) {

    if(!SIMD_SUPPORT || vector_size != 2){
        // SIMD is not supported on this device, call normal Mandelbrot
        return new Mandelbrot()->calculateFractal(cReal, cImaginary, maxIteration, vector_size);
    }

    uint32x4_t i = {0};
    float32x4_t cReal_vec = vld4_f32(cReal);
    float32x4_t cImaginary_vec = vld4_f32(cImaginary);
    float32x4_t zReal = {0};
    float32x4_t zImaginary = {0};
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
