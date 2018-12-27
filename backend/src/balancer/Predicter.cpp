#include "Predicter.h"

#include "Region.h"
#include "Fractal.h"
#include "Prediction.h"

#include <iostream>

Prediction* Predicter::getPrediction(Region region, Fractal* fractal, int predictionAccuracy) {
    int predictionLengthX = region.width / region.guaranteedDivisor;
    int predictionLengthY = region.height / region.guaranteedDivisor;

    // Declare all variables used by both if branches
    Region lowRes = region;

    // Prediction per part with width and height == guaranteedDivisor
    std::vector<std::vector<int>> n(predictionLengthX, std::vector<int>(predictionLengthY));
    std::vector<int> nColSums(predictionLengthX);
    std::vector<int> nRowSums(predictionLengthY);
    int nSum = 0;

    precision_t deltaReal = 0.0;
    precision_t deltaImaginary = 0.0;

    // predicitionAccuracy > 0: (predicitionAccuracy^2) pixel samples in each (divisor^2)-Block
    if (predictionAccuracy > 0) {
        lowRes.width = (region.width / region.guaranteedDivisor) * predictionAccuracy;
        lowRes.height = (region.height / region.guaranteedDivisor) * predictionAccuracy;

        deltaReal = Fractal::deltaReal(lowRes.maxReal, lowRes.minReal, lowRes.width);
        deltaImaginary = Fractal::deltaImaginary(lowRes.maxImaginary, lowRes.minImaginary, lowRes.height);

        for (unsigned int x = 0; x < lowRes.width; x++) {
            for (unsigned int y = 0; y < lowRes.height; y++) {
                precision_t cReal = lowRes.minReal + x * deltaReal;
                precision_t cImaginary = lowRes.maxImaginary - y * deltaImaginary;
                unsigned short int iterationCount;
                fractal->calculateFractal(&cReal,
                                          &cImaginary,
                                          lowRes.maxIteration,
                                          1,
                                          &iterationCount);
                // Assign iteration count to a (divisor^2)-Block
                n[x / predictionAccuracy][y / predictionAccuracy] += iterationCount;
                // Sum over expected iteration per column
                nColSums[x / predictionAccuracy] += iterationCount;
                // Sum over expected iteration per row
                nRowSums[y / predictionAccuracy] += iterationCount;
                // Sum over all expected iterations
                nSum += iterationCount;
            }
        }

        // Set deltas to represent delta per prediction piece
        deltaReal *= predictionAccuracy;
        deltaImaginary *= predictionAccuracy;

    // predicitionAccuracy < 0: (predicitionAccuracy^2) (divisor^2)-Blocks in each pixel sample
    } else if (predictionAccuracy < 0) {
        // Make predictionAccuracy positive, sign just determines the operation internally predictionAccuracy is always positive
        predictionAccuracy = -predictionAccuracy;

        lowRes.width = predictionLengthX / predictionAccuracy;
        lowRes.height = predictionLengthY / predictionAccuracy;

        // calculate also a prediction for overlaying (divisor^2)-Blocks
        if (predictionLengthX % predictionAccuracy != 0) {
            lowRes.maxReal += (predictionAccuracy - predictionLengthX % predictionAccuracy)
                              * region.guaranteedDivisor
                              * Fractal::deltaReal(region.maxReal, region.minReal, region.width);
            lowRes.width += 1;
        }
        if (predictionLengthY % predictionAccuracy != 0) {
            lowRes.minImaginary -= (predictionAccuracy - predictionLengthY % predictionAccuracy)
                                   * region.guaranteedDivisor
                                   * Fractal::deltaImaginary(region.maxImaginary, region.minImaginary, region.height);
            lowRes.height += 1;
        }

        deltaReal = Fractal::deltaReal(lowRes.maxReal, lowRes.minReal, lowRes.width);
        deltaImaginary = Fractal::deltaImaginary(lowRes.maxImaginary, lowRes.minImaginary, lowRes.height);

        for (unsigned int x = 0; x < lowRes.width; x++) {
            for (unsigned int y = 0; y < lowRes.height; y++) {
                precision_t cReal = lowRes.minReal + x * deltaReal;
                precision_t cImaginary = lowRes.maxImaginary - y * deltaImaginary;
                unsigned short int iterationCount;
                fractal->calculateFractal(&cReal,
                                          &cImaginary,
                                          lowRes.maxIteration,
                                          1,
                                          &iterationCount);

                // Put iterationCount in every entry which belongs to this prediction part
                int xStartIndex = x * predictionAccuracy;
                int yStartIndex = y * predictionAccuracy;
                for (int i = 0; i < predictionAccuracy; i++) {
                    for (int j = 0; j < predictionAccuracy; j++) {
                        if (xStartIndex + i < predictionLengthX && yStartIndex + j < predictionLengthY) {
                            n[xStartIndex + i][yStartIndex + j] += iterationCount;
                            nColSums[xStartIndex + i] += iterationCount;
                            nRowSums[yStartIndex + j] += iterationCount;
                            nSum += iterationCount;
                        }
                    }
                }
            }
        }

        // Set deltas to represent delta per prediction piece
        deltaReal /= predictionAccuracy;
        deltaImaginary /= predictionAccuracy;
    } else {
        std::cerr << "predictionAccuracy cannot be 0." << std::endl;
        return nullptr;
    }

    Prediction* prediction = new Prediction();
    
    prediction->predictionLengthX = predictionLengthX;
    prediction->predictionLengthY = predictionLengthY;

    prediction->deltaReal = deltaReal;
    prediction->deltaImaginary = deltaImaginary;

    prediction->n = n;
    prediction->nColSums = nColSums;
    prediction->nRowSums = nRowSums;
    prediction->nSum = nSum;

    return prediction;
}