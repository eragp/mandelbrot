#include "PredictionBalancer.h"
#include "Region.h"
#include "Fractal.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <string>

const std::string PredictionBalancer::NAME = "prediction";

PredictionBalancer::~PredictionBalancer() {
    delete f;
}

// Worst case scenario: Prediction changes suddenly from small to big values
Region *PredictionBalancer::balanceLoad(Region region, int nodeCount) {
    int predictionLengthX = region.width / region.guaranteedDivisor;
    int predictionLengthY = region.height / region.guaranteedDivisor;

    // Declare all variables used by both if branches
    Region lowRes = region;

    // Prediction per part with width and height == guaranteedDivisor
    std::vector<std::vector<int>> n(predictionLengthX, std::vector<int>(predictionLengthY));
    std::vector<int> nColSums(predictionLengthX);
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
                int iterationCount;
                precision_t projReal = lowRes.minReal + x * deltaReal;
                precision_t projImag = lowRes.maxImaginary - y * deltaImaginary;
                f->calculateFractal(&projReal,
                                    &projImag,
                                    lowRes.maxIteration,
                                    1,
                                    &iterationCount);
                // wtf?!
                n[x / predictionAccuracy][y / predictionAccuracy] += iterationCount;
                // Sum over expected iteration per column
                nColSums[x / predictionAccuracy] += iterationCount;
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

        // calculate also a prediction for overlaying parts of size guaranteedDivisor * guaranteedDivisor
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
                int iterationCount;
                precision_t projReal = lowRes.minReal + x * deltaReal;
                precision_t projImag = lowRes.maxImaginary - y * deltaImaginary;
                f->calculateFractal(&projReal,
                                    &projImag,
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
                            nSum += iterationCount;
                        }
                    }
                }
            }
        }

        // Set deltas to represent delta per prediction piece
        deltaReal /= predictionAccuracy;
        deltaImaginary /= predictionAccuracy;

        // Make predictionAccuracy negative again, so that next call has same conditions
        predictionAccuracy = -predictionAccuracy;
    } else {
        std::cerr << "predictionAccuracy cannot be 0." << std::endl;
        return nullptr;
    }

    // Debug
    std::cout << "nSum: " << nSum << std::endl;
    std::cout << "nColSums[0]: " << nColSums[0] << std::endl;
    //---
    auto *allRegions = new Region[nodeCount];

    // Same as in naive
    auto cols = (int) sqrt(nodeCount); // cols = Quantity of separate parts in x direction --> Number of Columns
    while (nodeCount % cols != 0) {    // Find the next integral divisor of nodeCount smaller than its sqrt
        cols--;
    }

    int rows = nodeCount / cols;    // nodeCount = cols * rows; --> Number of Rows
    int desiredN = nSum / cols;


    Region tmp{};
    // These stay the same over all iterations
    tmp.minImaginary = region.minImaginary;
    tmp.maxImaginary = region.maxImaginary;
    tmp.height = region.height;
    tmp.maxIteration = region.maxIteration;
    tmp.vOffset = region.vOffset;
    tmp.validation = region.validation;
    tmp.guaranteedDivisor = region.guaranteedDivisor;

    // This will change, so will width and maxReal
    tmp.minReal = region.minReal;
    tmp.hOffset = region.hOffset;

    int currentN = 0;
    int currentCol = 0;
    int cumulativeN = 0; // Needed to update desiredN
    int usedPredictionCols = 0; // Needed to determine resolution

    for (int i = 0; i < predictionLengthX; i++) {
        if (currentCol == cols - 1) {
            // Take the rest
            tmp.maxReal = region.maxReal;
            tmp.width = region.width - region.guaranteedDivisor * i;

            std::vector<int> colPrediction(predictionLengthY);
            for (int j = 0; j < predictionLengthY; j++) {
                colPrediction[j] = 0;
                for (int k = i; k < predictionLengthX; k++) {
                    colPrediction[j] += n[k][j];
                }
            }
            Region *colRegions = PredictionBalancer::splitCol(tmp, rows, nSum - cumulativeN,
                                                              colPrediction,
                                                              deltaImaginary); //length: rows
            for (int j = 0; j < rows; j++) {
                allRegions[rows * currentCol + j] = colRegions[j];
            }

            delete[] colRegions;
            currentCol++;

            // End the loop
            break;
        }

        // Add nColSums entry to currentN
        currentN += nColSums[i];
        // Increment usedPredictionCols
        usedPredictionCols++;

        // Reached 1/cols of nSum or there is only one piece of prediction left for each remaining col
        if (currentN > desiredN || predictionLengthX - i <= cols - currentCol) {
            tmp.maxReal = region.minReal + (i + 1) * deltaReal;
            tmp.width = region.guaranteedDivisor * usedPredictionCols;

            std::vector<int> colPrediction(predictionLengthY);
            for (int j = 0; j < predictionLengthY; j++) {
                colPrediction[j] = 0;
                for (int k = i + 1 - usedPredictionCols; k < i + 1; k++) {
                    colPrediction[j] += n[k][j];
                }
            }

            Region *colRegions = PredictionBalancer::splitCol(tmp, rows, currentN, colPrediction,
                                                              deltaImaginary); //length: rows

            for (int j = 0; j < rows; j++) {
                allRegions[rows * currentCol + j] = colRegions[j];
            }

            delete[] colRegions;

            // Prepare tmp for next col
            tmp.minReal = tmp.maxReal;
            tmp.hOffset += tmp.width;

            // Debug
            std::cout << "currentN for col " << currentCol << ": " << currentN << std::endl;
            //---
            // Update cumulativeN
            cumulativeN += currentN;
            // Increment currentCol
            currentCol++;
            // Update desiredN --> this should reduce the difference of n between the cols
            desiredN = (nSum - cumulativeN) / (cols - currentCol);
            // Reset currentN
            currentN = 0;
            // Reset usedPredictionCols
            usedPredictionCols = 0;
        }
    }
    
    // Fill rest of allRegions with empty Regions, needed when `region` is covered with less than `cols` columns 
    // This happens if guaranteedDivisor or nodeCount becomes to big (especially when nodeCount is a prime number)
    // Set tmp to empty region
    tmp.minImaginary = 0.0;
    tmp.maxImaginary = 0.0;

    tmp.minReal = 0.0;
    tmp.maxReal = 0.0;

    tmp.height = 0;
    tmp.width = 0;

    tmp.vOffset = 0;
    tmp.hOffset = 0;

    while (currentCol < cols) {
        for (int i = 0; i < rows; i++) {
            allRegions[rows * currentCol + i] = tmp;
        }
        currentCol++;
    }

    return allRegions;
}

/**
 * Splits the current col into #parts pieces.
 * @param col
 * @param parts
 * @param nSum
 * @param n
 * @param deltaImaginary
 * @return
 */
Region *PredictionBalancer::splitCol(Region col, int parts, int nSum, std::vector<int> n, double deltaImaginary) {
    auto *regions = new Region[parts];

    int desiredN = nSum / parts;

    Region tmp{};
    // These stay the same over all iterations
    tmp.minReal = col.minReal;
    tmp.maxReal = col.maxReal;
    tmp.width = col.width;
    tmp.maxIteration = col.maxIteration;
    tmp.hOffset = col.hOffset;
    tmp.validation = col.validation;
    tmp.guaranteedDivisor = col.guaranteedDivisor;

    // This will change, so will height and minImaginary
    tmp.maxImaginary = col.maxImaginary;
    tmp.vOffset = col.vOffset;

    int currentN = 0;
    int currentPart = 0;
    int cumulativeN = 0;
    int usedPredictionRows = 0;

    for (unsigned int i = 0; i < n.size(); i++) {
        if (currentPart == parts - 1) {
            // Take the rest
            tmp.minImaginary = col.minImaginary;
            tmp.height = col.height - col.guaranteedDivisor * i;

            regions[currentPart] = tmp;

            currentPart++;
            // End the loop
            break;
        }

        // Add n entry to currentN
        currentN += n[i];
        // Increment usedPredictionRows
        usedPredictionRows++;

        // Reached 1/parts of nSum or there is only one piece of prediction left for each remaining part
        if (currentN > desiredN || n.size() - i <= (unsigned int) (parts - currentPart)) {
            tmp.minImaginary = col.maxImaginary - (i + 1) * deltaImaginary;
            tmp.height = col.guaranteedDivisor * usedPredictionRows;

            regions[currentPart] = tmp;

            // Prepare tmp for next part
            tmp.maxImaginary = tmp.minImaginary;
            tmp.vOffset += tmp.height;

            // Debug
            std::cout << "currentN for part " << currentPart << ": " << currentN << std::endl;
            //---
            // Update cumulativeN
            cumulativeN += currentN;
            // Increment currentPart
            currentPart++;
            // Update desiredN
            desiredN = (nSum - cumulativeN) / (parts - currentPart);
            // Reset currentN
            currentN = 0;
            // Reset usedPredictionCols
            usedPredictionRows = 0;
        }
    }
    
    // Fill rest of `regions` with empty Regions, needed when `col` is covered with less than `parts` parts
    // This happens if guaranteedDivisor or nodeCount becomes to big (especially when nodeCount is a prime number)
    // Set tmp to empty region
    tmp.minImaginary = 0.0;
    tmp.maxImaginary = 0.0;

    tmp.minReal = 0.0;
    tmp.maxReal = 0.0;

    tmp.height = 0;
    tmp.width = 0;

    tmp.vOffset = 0;
    tmp.hOffset = 0;

    while (currentPart < parts) {
        regions[currentPart] = tmp;
        currentPart++;
    }
    
    return regions;
}

PredictionBalancer *PredictionBalancer::create(Fractal *f, int predictionAccuracy) {
    PredictionBalancer *b = new PredictionBalancer();
    b->f = f;
    b->predictionAccuracy = predictionAccuracy;
    return b;
}