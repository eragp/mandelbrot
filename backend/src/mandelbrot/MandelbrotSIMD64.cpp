#include "MandelbrotSIMD64.h"
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <iostream>

#include <arm_neon.h>

// Note that this is set for Raspi3 and Odroid (ARM Cortex A53)
// According to https://developer.arm.com/technologies/neon and https://en.wikipedia.org/wiki/ARM_Cortex-A53
// it supports the ARMv8-A and thus 64bit x1 flops.

// Probably more open to compiler optimization
// vectorlength >= 1 !!
void MandelbrotSIMD64::calculateFractal(precision_t* cRealArray, precision_t* cImaginaryArray, unsigned short int maxIteration, int vectorLength, unsigned short int* dest) {
    if(vectorLength <= 0){
        throw std::invalid_argument("vectorLength may not be less than 1.");
    }
    for(int j = 0; j < (vectorLength/1); j++){
    // General form of vector commands
    // v<cmd>_f<pr>
    // v -> vector command
    // q -> double amount of used registers (quad=>4)
    // f -> float
    // 64 bit vectorization
    // Load values from array to simd vector
    float64x1_t cReal = vdup_n_f64(0);// = vld4_f64(cRealArray);
    cReal = vset_lane_f64((float64_t) cRealArray[0], cReal, 0);
    float64x1_t cImaginary = vdup_n_f64(0);// = vld4_f64(cImaginaryArray);
    cImaginary = vset_lane_f64((float64_t) cImaginaryArray[0], cImaginary, 0);
    // The z values
    float64x1_t zReal = vdup_n_f64(0);
    float64x1_t zImaginary = vdup_n_f64(0);
    // determines if computation will be continued
    float64x1_t two = vdup_n_f64(2);
    float64x1_t four = vdup_n_f64(4);
    // result iterations
    uint64x1_t one = vdup_n_u64(1);
    uint64x1_t n = vdup_n_u64(0);
    uint64x1_t absLesserThanTwo = vdup_n_u64(1);
    int i = 0;
    while(i < maxIteration && vget_lane_u64(absLesserThanTwo, 0) != 0){
        // add a b -> a+b
        // mls a b c -> a - b*c
        // mul a b -> a*b
        float64x1_t nextZReal = vadd_f64(vmls_f64(vmul_f64(zReal, zReal), zImaginary, zImaginary), cReal);
        // mla a b c -> a + b*c
        float64x1_t nextZImaginary = vmla_f64(cImaginary, two, vmul_f64(zReal, zImaginary));
        zReal = nextZReal;
        zImaginary = nextZImaginary;
        // Square of the absolute value -> determine when to stop
        float64x1_t absSquare = vmla_f64(vmul_f64(zReal, zReal), zImaginary, zImaginary);
        // If square of the absolute is less than 4, abs<2 holds -> 1
        absLesserThanTwo = vand_u64(vclt_f64(absSquare, four), one);
        // if any value is 1 in the vector (abs<2) then dont break
        // addv => sum all elements of the vector
        n = vadd_u64(n, absLesserThanTwo);
        i++;
    }
    // write n to dest
    dest[0] = vget_lane_u64(n, 0);

    cRealArray += 1;
    cImaginaryArray += 1;
    dest += 1;
    }
}
