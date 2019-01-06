#include "MandelbrotSIMD32.h"
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <iostream>

#include <arm_neon.h>

// Note that this is set for Raspi3 and Odroid (ARM Cortex A53)
// According to https://developer.arm.com/technologies/neon and https://en.wikipedia.org/wiki/ARM_Cortex-A53
// it supports the ARMv8-A and thus 64bit x2 flops.

// Probably more open to compiler optimization
// vectorlength >= 1 !!
void MandelbrotSIMD32::calculateFractal(precision_t* cRealArray, precision_t* cImaginaryArray, unsigned short int maxIteration, int vectorLength, unsigned short int* dest) {
    if(vectorLength <= 0){
        throw std::invalid_argument("vectorLength may not be less than 1.");
    }
    for(int j = 0; j < (vectorLength/2); j++){
    // General form of vector commands
    // v<cmd>q_f<pr>
    // v -> vector command
    // q -> double amount of used registers
    // f -> float
    // 32 bit vectorization
    // Load casted values from array to simd vector
    float32x2_t cReal = vdup_n_f32(0);// = vld1q_f32(cRealArray); if casting weren't necessary this would work
    cReal = vset_lane_f32((float32_t) cRealArray[0], cReal, 0);
    cReal = vset_lane_f32((float32_t) cRealArray[1], cReal, 1);
    float32x2_t cImaginary = vdup_n_f32(0);
    cImaginary = vset_lane_f32((float32_t) cImaginaryArray[0], cImaginary, 0);
    cImaginary = vset_lane_f32((float32_t) cImaginaryArray[1], cImaginary, 1);
    // The z values
    float32x2_t zReal = vdup_n_f32(0);
    float32x2_t zImaginary = vdup_n_f32(0);
    // Helper variables
    float32x2_t two = vdup_n_f32(2);
    float32x2_t four = vdup_n_f32(4);
    uint32x2_t one = vdup_n_u32(1);
    // result iterations
    uint32x2_t n = vdup_n_u32(0);
    // vector with 1 if absolute value of component is less than two
    uint32x2_t absLesserThanTwo = vdup_n_u32(1);
    int i = 0;
    // addv => sum all elements of the vector
    while(i < maxIteration && vaddv_u32(absLesserThanTwo) > 0){
        // mls a b c -> a - b*c
        float32x2_t nextZReal = vadd_f32(vmls_f32(vmul_f32(zReal, zReal), zImaginary, zImaginary), cReal);
        // mla a b c -> a + b*c
        float32x2_t nextZImaginary = vmla_f32(cImaginary, two, vmul_f32(zReal, zImaginary));
        zReal = nextZReal;
        zImaginary = nextZImaginary;
        // Square of the absolute value -> determine when to stop
        float32x2_t absSquare = vmla_f32(vmul_f32(zReal, zReal), zImaginary, zImaginary);
        // If square of the absolute is less than 4, abs<2 holds -> 1 else 0
        absLesserThanTwo = vand_u32(vclt_f32(absSquare, four), one);
        // if any value is 1 in the vector (abs<2) then dont break
        n = vadd_u32(n, absLesserThanTwo);
        i++;
    }
    // write n to dest
    dest[0] = vget_lane_u32(n, 0);
    dest[1] = vget_lane_u32(n, 1);

    cRealArray += 2;
    cImaginaryArray += 2;
    dest += 2;
    }
}