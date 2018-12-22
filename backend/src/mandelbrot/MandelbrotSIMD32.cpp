#include "MandelbrotSIMD32.h"
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <iostream>

#ifdef __ARM_NEON

#include <arm_neon.h>

// Note that this is set for Raspi3 and Odroid (ARM Cortex A53)
// According to https://developer.arm.com/technologies/neon and https://en.wikipedia.org/wiki/ARM_Cortex-A53
// it supports the ARMv8-A and thus 64bit x2 flops.

// Probably more open to compiler optimization
// vectorlength >= 1 !!
void MandelbrotSIMD32::calculateFractal(precision_t* cRealArray, precision_t* cImaginaryArray, int maxIteration, int vectorLength, int* dest) {
    if(vectorLength <= 0){
        throw std::invalid_argument("vectorLength may not be less than 1.");
    }
    for(int j = 0; j < (vectorLength/4); j++){
    // General form of vector commands
    // v<cmd>q_f<pr>
    // v -> vector command
    // q -> double amount of used registers (quad=>4)
    // f -> float
    // 32 bit vectorization
    // Load values from array to simd vector
    float32x4_t cReal = vdupq_n_f32(0);// = vld4q_f32(cRealArray);
    cReal = vsetq_lane_f32((float32_t) cRealArray[0], cReal, 0);
    cReal = vsetq_lane_f32((float32_t) cRealArray[1], cReal, 1);
    cReal = vsetq_lane_f32((float32_t) cRealArray[2], cReal, 2);
    cReal = vsetq_lane_f32((float32_t) cRealArray[3], cReal, 3);
    float32x4_t cImaginary = vdupq_n_f32(0);// = vld4q_f32(cImaginaryArray);
    cImaginary = vsetq_lane_f32((float32_t) cImaginaryArray[0], cImaginary, 0);
    cImaginary = vsetq_lane_f32((float32_t) cImaginaryArray[1], cImaginary, 1);
    cImaginary = vsetq_lane_f32((float32_t) cImaginaryArray[2], cImaginary, 2);
    cImaginary = vsetq_lane_f32((float32_t) cImaginaryArray[3], cImaginary, 3);
    // The z values
    float32x4_t zReal = vdupq_n_f32(0);
    float32x4_t zImaginary = vdupq_n_f32(0);
    // determines if computation will be continued
    float32x4_t two = vdupq_n_f32(2);
    float32x4_t four = vdupq_n_f32(4);
    // result iterations
    uint32x4_t one = vdupq_n_u32(1);
    uint32x4_t n = vdupq_n_u32(0);
    uint32x4_t absLesserThanTwo = vdupq_n_u32(1);
    int i = 0;
    while(i < maxIteration && vaddvq_u32(absLesserThanTwo) != 0){
        // add a b -> a+b
        // mls a b c -> a - b*c
        // mul a b -> a*b
        float32x4_t nextZReal = vaddq_f32(vmlsq_f32(vmulq_f32(zReal, zReal), zImaginary, zImaginary), cReal);
        // mla a b c -> a + b*c
        float32x4_t nextZImaginary = vmlaq_f32(cImaginary, two, vmulq_f32(zReal, zImaginary));
        zReal = nextZReal;
        zImaginary = nextZImaginary;
        // Square of the absolute value -> determine when to stop
        float32x4_t absSquare = vmlaq_f32(vmulq_f32(zReal, zReal), zImaginary, zImaginary);
        // If square of the absolute is less than 4, abs<2 holds -> 1
        absLesserThanTwo = vandq_u32(vcltq_f32(absSquare, four), one);
        // if any value is 1 in the vector (abs<2) then dont break
        // addv => sum all elements of the vector
        n = vaddq_u32(n, absLesserThanTwo);
        i++;
    }
    // write n to dest
    dest[0] = vgetq_lane_u32(n, 0);
    dest[1] = vgetq_lane_u32(n, 1);
    dest[2] = vgetq_lane_u32(n, 2);
    dest[3] = vgetq_lane_u32(n, 3);

    cRealArray += 4;
    cImaginaryArray += 4;
    dest += 4;
    }
}

#else 

void MandelbrotSIMD32::calculateFractal(precision_t* cReal, precision_t* cImaginary, int maxIteration, int vectorLength, int* dest) {
    for(int j = 0; j < vectorLength; j++){
        dest[j] = 0;
    }
}
#endif
