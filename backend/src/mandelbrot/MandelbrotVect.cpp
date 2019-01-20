#include "MandelbrotVect.h"
#include <algorithm>
#include <stdexcept>
#include <vector>

// Probably more open to compiler optimization
// vectorlength >= 1 !!
void MandelbrotVect::calculateFractal(precision_t* cReal, precision_t* cImaginary, unsigned short int maxIteration, unsigned int vectorLength, unsigned short int* dest) {
    if(vectorLength == 0){
        throw std::invalid_argument("vectorLength may not be less than 1.");
    }
    std::fill_n(dest, vectorLength, 0);
    precision_t* zReal = new precision_t[vectorLength];
    precision_t* zImaginary = new precision_t[vectorLength];
    precision_t* nextZReal = new precision_t[vectorLength];
    precision_t* nextZImaginary = new precision_t[vectorLength];
    // Factor that is added on iteration count
    // is 0 or 1 and determines whether that point is still being computed
    bool* factor = new bool[vectorLength];
    std::fill_n(factor, vectorLength, 1);
    // Integer storing number of Z components with absolute value
    // below two => continue computation
    unsigned int lessThanTwo = vectorLength; // as we begin with ZReal/ZImag as 0
    int i = 0;
    while (i < maxIteration && lessThanTwo > 0){
        lessThanTwo = 0;
        for(unsigned int k = 0; k < vectorLength; k++){
            // Compute next step in iteration
            nextZReal[k] = (zReal[k] * zReal[k] - zImaginary[k] * zImaginary[k]) + cReal[k];
            nextZImaginary[k] = 2 * (zReal[k] * zImaginary[k]) + cImaginary[k];
            zReal[k] = nextZReal[k];
            zImaginary[k] = nextZImaginary[k];
            // Determine whether to stop
            factor[k] = (zReal[k] * zReal[k] + zImaginary[k] * zImaginary[k] < 4.0) ? 1 : 0;
            // sum => if any number is still less than two, we need to continue
            lessThanTwo += factor[k];
            // increase number of iterations if this number wasnt aborted yet
            dest[k] += factor[k];
        }
    }
    delete[] zReal;
    delete[] zImaginary;
    delete[] nextZReal;
    delete[] nextZImaginary;
    delete[] factor;
}
