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

        // Pass a whole row at once to mandelbrot for computation
        // This is the maximum value for the vector length
        int vectorLength = lowRes.width;
        precision_t* projReal = new precision_t[vectorLength];
        precision_t* projImag = new precision_t[vectorLength];
        unsigned short int* data = new unsigned short int[vectorLength];

        for (unsigned int y = 0; y < lowRes.height; y++) {
            for (unsigned int x = 0; x < lowRes.width; x += vectorLength) {
                int reverseY = lowRes.height - y;
                for (int k = 0; k < vectorLength; k++) {
                    projReal[k] = lowRes.projectReal(x + k);
                    projImag[k] = lowRes.projectImag(reverseY);
                }

                fractal->calculateFractal(projReal,
                                          projImag,
                                          lowRes.maxIteration,
                                          vectorLength,
                                          data);
                for (int k = 0; k < vectorLength; k++) {
                    unsigned short int iterationCount = data[k];

                    // Assign iteration count to a (divisor^2)-Block
                    n[(x + k) / predictionAccuracy][y / predictionAccuracy] += iterationCount;
                    // Sum over expected iteration per column
                    nColSums[(x + k) / predictionAccuracy] += iterationCount;
                    // Sum over expected iteration per row
                    nRowSums[y / predictionAccuracy] += iterationCount;
                    // Sum over all expected iterations
                    nSum += iterationCount;
                }
            }
        }

        // Set deltas to represent delta per prediction piece
        deltaReal = Fractal::deltaReal(lowRes.maxReal, lowRes.minReal, lowRes.width) * predictionAccuracy;
        deltaImaginary = Fractal::deltaImaginary(lowRes.maxImaginary, lowRes.minImaginary, lowRes.height) * predictionAccuracy;

        delete[] projReal;
        delete[] projImag;
        delete[] data;
    }
    // predicitionAccuracy < 0: (predicitionAccuracy^2) (divisor^2)-Blocks in each pixel sample
    else if (predictionAccuracy < 0) {
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

        // Pass a whole row at once to mandelbrot for computation
        // This is the maximum value for the vector length
        int vectorLength = lowRes.width;
        precision_t* projReal = new precision_t[vectorLength];
        precision_t* projImag = new precision_t[vectorLength];
        unsigned short int* data = new unsigned short int[vectorLength];

        for (unsigned int y = 0; y < lowRes.height; y++) {
            for (unsigned int x = 0; x < lowRes.width; x += vectorLength) {
                int reverseY = lowRes.height - y;
                for (int k = 0; k < vectorLength; k++) {
                    projReal[k] = lowRes.projectReal(x + k);
                    projImag[k] = lowRes.projectImag(reverseY);
                }

                fractal->calculateFractal(projReal,
                                          projImag,
                                          lowRes.maxIteration,
                                          vectorLength,
                                          data);

                for (int k = 0; k < vectorLength; k++) {
                    unsigned short int iterationCount = data[k];

                    // Put iterationCount in every entry which belongs to this prediction part
                    int xStartIndex = (x + k) * predictionAccuracy;
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
        }

        // Set deltas to represent delta per prediction piece
        deltaReal = Fractal::deltaReal(lowRes.maxReal, lowRes.minReal, lowRes.width) / predictionAccuracy;
        deltaImaginary = Fractal::deltaImaginary(lowRes.maxImaginary, lowRes.minImaginary, lowRes.height) / predictionAccuracy;

        delete[] projReal;
        delete[] projImag;
        delete[] data;
    }
    else {
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