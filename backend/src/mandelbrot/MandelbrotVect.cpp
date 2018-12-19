#include "MandelbrotVect.h"
#include "MandelbrotOpt.h"
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <arm_neon.h>

// Note that this is set for Raspi3 and Odroid (ARM Cortex A53)
// According to https://developer.arm.com/technologies/neon and https://en.wikipedia.org/wiki/ARM_Cortex-A53
// it supports the ARMv8-A and thus 64bit x2 flops.
/**
 * Return whether at least one point still has an abs value below 2 => continue computation
 * @param dest Memory where it is stored whether this point is still computed (0 for no, 1 for yes)
 */
bool continueComp(precision_t* zReal, precision_t* zImaginary, int vectorLength, bool* factor){
    bool lessThanTwo = false;
    for(int k = 0; k < vectorLength; k++){
        factor[k] = (factor[k] && zReal[k] * zReal[k] + zImaginary[k] * zImaginary[k] < 4.0) ? 1 : 0;
        // or => if any value is true, true is returned
        lessThanTwo = lessThanTwo || factor[k];
    }
    return lessThanTwo;
}

// Probably more open to compiler optimization
// vectorlength >= 1 !!
void MandelbrotVect::calculateFractal(precision_t* cRealArray, precision_t* cImaginaryArray, int maxIteration, int vectorLength, int* dest) {
    int i = 0;
    if(vectorLength <= 0){
        throw std::invalid_argument("vectorLength may not be less than 1.");
    }
    // General form of vector commands
    // v<cmd>q_f<pr>
    // v -> vector command
    // q -> double amount of used registers
    // f -> float
    if(sizeof(precision_t) == 64) {
        // 64 bit vectorization
        //float64x2_t 
    }
    else if (sizeof(precision_t) == 32) {
        // 32 bit vectorization
        // Load values from array to simd vector
        float32x4_t cReal = vld4q_f32(cRealArray);
        float32x4_t cImaginary = vld4q_f32(cImaginaryArray);
        /* for (int i = 0; i < 4; i++){
            cReal = vsetq_lane_f32(cRealArray[i], cReal, i);
            cImaginary = vsetq_lane_f32(cImaginaryArray[i], cImaginary, i);
        } */
        // The z values
        float32x4_t zReal = vdupq_n_f32(0);
        float32x4_t zImaginary = vdupq_n_f32(0);
        // determines if computation will be continued
        uint32x4_t factor = vdupq_n_u32(1);
        float32x4_t two = vdupq_n_f32(2);
        float32x4_t four = vdupq_n_f32(4);
        int i = 0;
        while(i < maxIteration){
            // add a b -> a+b
            // mls a b c -> a - b*c
            // mul a b -> a*b
            float32x4_t nextZReal = vaddq_f32(vmlsq_f32(vmulq_f32(zReal, zReal), zImaginary, zImaginary), cReal);
            // mla a b c -> a + b*c
            float32x4_t nextZImaginary = vmlaq_f32(cImaginary, two, vmulq_f32(zReal, zImaginary));
            zReal = nextZReal;
            zImaginary = nextZImaginary;
            // Square of the absolute value -> determine when to stop
            float32x4_t absSquare = vmlaq_f32(vmulq_f32(zReal, zReal), cReal, cReal);
            // If square of the absolute is less than 4, abs<2 holds -> 1
            uint32x4_t absLesserThanTwo = vclt_f32(absSquare, four);
            // if any value is 1 in the vector (abs<2) then dont break
            // addlv => sum all elements of the vector
            if(vaddlvq_u32(absLesserThanTwo) == 0){
                break;
            }
        }

    }
    else{
        // No vectorization for you today
        return (new MandelbrotOpt())->calculateFractal(cRealArray, cImaginaryArray, maxIteration, vectorLength, dest);
    }
    
}
