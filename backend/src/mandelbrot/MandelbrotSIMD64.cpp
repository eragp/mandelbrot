#include "MandelbrotSIMD64.h"
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <iostream>

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

// Note that this is set for Raspi3 and Odroid (ARM Cortex A53)
// According to https://developer.arm.com/technologies/neon and https://en.wikipedia.org/wiki/ARM_Cortex-A53
// it supports the ARMv8-A and thus 64bit x2 flops.

// Probably more open to compiler optimization
// vectorlength >= 1 !!
void MandelbrotSIMD64::calculateFractal(precision_t* cRealArray, precision_t* cImaginaryArray, unsigned short int maxIteration, int vectorLength, unsigned short int* dest) {
    #ifdef __ARM_NEON
    if(vectorLength <= 0){
        throw std::invalid_argument("vectorLength may not be less than 1.");
    }
    for(int j = 0; j < (vectorLength/2); j++){
    // General form of vector commands
    // v<cmd>q_f<pr>
    // v -> vector command
    // q -> double amount of used registers (quad=>4)
    // f -> float
    // 64 bit vectorization
    // Load values from array to simd vector
    float64x2_t cReal = vdupq_n_f64(0);// = vld4q_f64(cRealArray);
    cReal = vsetq_lane_f64((float64_t) cRealArray[0], cReal, 0);
    cReal = vsetq_lane_f64((float64_t) cRealArray[1], cReal, 1);
    float64x2_t cImaginary = vdupq_n_f64(0);// = vld4q_f64(cImaginaryArray);
    cImaginary = vsetq_lane_f64((float64_t) cImaginaryArray[0], cImaginary, 0);
    cImaginary = vsetq_lane_f64((float64_t) cImaginaryArray[1], cImaginary, 1);
    // The z values
    float64x2_t zReal = vdupq_n_f64(0);
    float64x2_t zImaginary = vdupq_n_f64(0);
    // determines if computation will be continued
    float64x2_t two = vdupq_n_f64(2);
    float64x2_t four = vdupq_n_f64(4);
    // result iterations
    uint64x2_t one = vdupq_n_u64(1);
    uint64x2_t n = vdupq_n_u64(0);
    uint64x2_t absLesserThanTwo = vdupq_n_u64(1);
    int i = 0;
    while(i < maxIteration && vaddvq_u64(absLesserThanTwo) != 0){
        // add a b -> a+b
        // mls a b c -> a - b*c
        // mul a b -> a*b
        float64x2_t nextZReal = vaddq_f64(vmlsq_f64(vmulq_f64(zReal, zReal), zImaginary, zImaginary), cReal);
        // mla a b c -> a + b*c
        float64x2_t nextZImaginary = vmlaq_f64(cImaginary, two, vmulq_f64(zReal, zImaginary));
        zReal = nextZReal;
        zImaginary = nextZImaginary;
        // Square of the absolute value -> determine when to stop
        float64x2_t absSquare = vmlaq_f64(vmulq_f64(zReal, zReal), zImaginary, zImaginary);
        // If square of the absolute is less than 4, abs<2 holds -> 1
        absLesserThanTwo = vandq_u64(vcltq_f64(absSquare, four), one);
        // if any value is 1 in the vector (abs<2) then dont break
        // addv => sum all elements of the vector
        n = vaddq_u64(n, absLesserThanTwo);
        i++;
    }
    // write n to dest
    dest[0] = vgetq_lane_u64(n, 0);
    dest[1] = vgetq_lane_u64(n, 1);

    cRealArray += 2;
    cImaginaryArray += 2;
    dest += 2;
    }
    #else
    for(int j = 0; j < vectorLength; j++){
        dest[j] = 0;
    }
    #endif
}
