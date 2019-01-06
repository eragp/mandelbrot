#include <cmath>

#include "Region.h"
#include "PredictionBalancer.h"
#include "Predicter.h"

Region *PredictionBalancer::balanceLoad(Region region, int nodeCount) {
    Prediction* prediction = Predicter::getPrediction(region, f, predictionAccuracy);

    auto *allRegions = new Region[nodeCount];

    // Same as in naive
    int cols = (int) sqrt(nodeCount);
    // Find the next integral divisor of nodeCount smaller than its sqrt
    while (nodeCount % cols != 0) {
        cols--;
    }

    int rows = nodeCount / cols;    // nodeCount = cols * rows; --> Number of Rows
    int desiredN = prediction->nSum / cols;

    // Only width, hOffset, minReal and maxReal will change
    Region tmp = region;

    int currentN = 0;
    int currentCol = 0;
    int cumulativeN = 0; // Needed to update desiredN
    int usedPredictionCols = 0; // Needed to determine resolution

    for (int i = 0; i < prediction->predictionLengthX; i++) {
        if (currentCol == cols - 1) {
            // Take the rest
            tmp.maxReal = region.maxReal;
            tmp.width = region.width - region.guaranteedDivisor * i;

			// Split this column (tmp) and write to result (left out)

            currentCol++;
            // End the loop
            break;
        }

        // Add nColSums entry to currentN
        currentN += prediction->nColSums[i];
        // Increment usedPredictionCols
        usedPredictionCols++;

        // Reached 1/cols of nSum or there is only one piece of prediction left for each remaining col
        if (currentN >= desiredN || prediction->predictionLengthX - (i + 1) <= cols - currentCol) {
            tmp.maxReal = region.minReal + (i + 1) * prediction->deltaReal;
            tmp.width = region.guaranteedDivisor * usedPredictionCols;

		    // Split this column (tmp) and write to result (left out)

            // Prepare tmp for next col
            tmp.minReal = tmp.maxReal;
            tmp.hOffset += tmp.width;

            // Update cumulativeN
            cumulativeN += currentN;
            // Increment currentCol
            currentCol++;
            // Update desiredN --> this should reduce the difference of n between the cols
            desiredN = (prediction->nSum - cumulativeN) / (cols - currentCol);
            // Reset currentN
            currentN = 0;
            // Reset usedPredictionCols
            usedPredictionCols = 0;
        }
    }

    // ...
}