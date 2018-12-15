#include "MandelbrotOpt.h"
#include <algorithm>
#include <stdexcept>
#include <vector>

/**
 * Return whether at least one point still has an abs value below 2 => continue computation
 * @param dest Memory where it is stored whether this point is still computed (0 for no, 1 for yes)
 */
bool continueComp(std::vector<precision_t>* zReal, std::vector<precision_t>* zImaginary, int vectorLength, std::vector<int>* factor){
    bool lessThanTwo = false;
    for(int k = 0; k < vectorLength; k++){
        (*factor)[k] = ((*factor)[k] && (*zReal)[k] * (*zReal)[k] + (*zImaginary)[k] * (*zImaginary)[k] < 4.0) ? 1 : 0;
        // or => if any value is true, true is returned
        lessThanTwo = lessThanTwo || ((*factor)[k] > 0);
    }
    return lessThanTwo;
}

// Probably more open to compiler optimization
// vectorlength >= 1 !!
void MandelbrotOpt::calculateFractal(precision_t* cReal, precision_t* cImaginary, int maxIteration, int vectorLength, int* dest) {
    int i = 0;
    if(vectorLength <= 0){
        throw std::invalid_argument("vectorLength may not be less than 1.");
    }
    std::fill(dest, dest+vectorLength, 0);
    std::vector<precision_t> zReal(vectorLength);
    std::vector<precision_t> zImaginary(vectorLength);
    precision_t nextZReal[vectorLength];
    precision_t nextZImaginary[vectorLength];
    // Factor that is multiplied on iteration count and computation 
    // is 0 or 1 and determines whether that point is still being computed
    std::vector<int> factor(vectorLength);
    for(int i = 0; i <vectorLength; i++){
        factor[i] = 1;
    }
    // Bool storing information about whether any abs value that is being computed
    // is still below two => continue computation
    bool lessThanTwo = continueComp(&zReal, &zImaginary, vectorLength, &factor);
    while (i < maxIteration && lessThanTwo){
        for(int k = 0; k < vectorLength; k++){
            nextZReal[k] = (zReal[k] * zReal[k] - zImaginary[k] * zImaginary[k]) + cReal[k];
            nextZImaginary[k] = 2 * (zReal[k] * zImaginary[k]) + cImaginary[k];
            zReal[k] = factor[k]*nextZReal[k];
            zImaginary[k] = factor[k]*nextZImaginary[k];
            dest[k] += factor[k];
        }
        lessThanTwo = continueComp(&zReal, &zImaginary, vectorLength, &factor);
    }
}
