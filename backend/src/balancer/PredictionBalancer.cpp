#include "PredictionBalancer.h"
#include "Region.h"
#include "Fractal.h"
#include "Predicter.h"

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
	Prediction* prediction = Predicter::getPrediction(region, f, predictionAccuracy);

    auto *allRegions = new Region[nodeCount];

    // Same as in naive
    auto cols = (int) sqrt(nodeCount); // cols = Quantity of separate parts in x direction --> Number of Columns
    while (nodeCount % cols != 0) {    // Find the next integral divisor of nodeCount smaller than its sqrt
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

			Prediction* colPrediction = new Prediction();

			colPrediction->predictionLengthX = 1;
			colPrediction->predictionLengthY = prediction->predictionLengthY;

			colPrediction->nSum = prediction->nSum - cumulativeN;
			colPrediction->nRowSums.resize(colPrediction->predictionLengthY);
			// Not used, may be undefined
			// colPrediction->nColSums.resize(colPrediction->predictionLengthX);
			// colPrediction->n.resize(colPrediction->predictionLengthX);
			// for (std::vector<int> v : colPrediction->n) {
			//	v.resize(colPrediction->predictionLengthY);
			// }

            for (int j = 0; j < prediction->predictionLengthY; j++) {
                colPrediction->nRowSums[j] = 0;
                for (int k = i; k < prediction->predictionLengthX; k++) {
                    colPrediction->nRowSums[j] += prediction->n[k][j];
                }
            }

			colPrediction->deltaReal = prediction->deltaReal;
			colPrediction->deltaImaginary = prediction->deltaImaginary;

            Region *colRegions = PredictionBalancer::splitCol(tmp, rows, colPrediction); //length: rows
            for (int j = 0; j < rows; j++) {
                allRegions[rows * currentCol + j] = colRegions[j];
            }

			delete colPrediction;
			delete[] colRegions;
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

			Prediction* colPrediction = new Prediction();

			colPrediction->predictionLengthX = 1;
			colPrediction->predictionLengthY = prediction->predictionLengthY;

			colPrediction->nSum = currentN;
			colPrediction->nRowSums.resize(colPrediction->predictionLengthY);
			// Not used, may be undefined
			// colPrediction->nColSums.resize(colPrediction->predictionLengthX);
			// colPrediction->n.resize(colPrediction->predictionLengthX);
			// for (std::vector<int> v : colPrediction->n) {
			//	v.resize(colPrediction->predictionLengthY);
			// }

			for (int j = 0; j < prediction->predictionLengthY; j++) {
				colPrediction->nRowSums[j] = 0;
				for (int k = i + 1 - usedPredictionCols; k < i + 1; k++) {
					colPrediction->nRowSums[j] += prediction->n[k][j];
				}
			}

			colPrediction->deltaReal = prediction->deltaReal;
			colPrediction->deltaImaginary = prediction->deltaImaginary;

            Region *colRegions = PredictionBalancer::splitCol(tmp, rows, colPrediction); //length: rows

            for (int j = 0; j < rows; j++) {
                allRegions[rows * currentCol + j] = colRegions[j];
            }

			delete colPrediction;
			delete[] colRegions;

            // Prepare tmp for next col
            tmp.minReal = tmp.maxReal;
            tmp.hOffset += tmp.width;

            // Debug
            // std::cout << "currentN for col " << currentCol << ": " << currentN << std::endl;
            //---
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
    
	// Free prediction, not needed anymore
	delete prediction;

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
Region *PredictionBalancer::splitCol(Region col, int parts, Prediction* prediction) {
	/*if (prediction->predictionLengthX != 1) {
		std::cerr << "Invalid prediction: more/less than one column." << std::endl;
		return nullptr;
	}*/

	auto *regions = new Region[parts];

    int desiredN = prediction->nSum / parts;

    // Only height, vOffset, minImaginary and maxImaginary will change
    Region tmp = col;

    int currentN = 0;
    int currentPart = 0;
    int cumulativeN = 0;
    int usedPredictionRows = 0;

    for (int i = 0; i < prediction->predictionLengthY; i++) {
        if (currentPart == parts - 1) {
            // Take the rest
            tmp.minImaginary = col.minImaginary;
            tmp.height = col.height - col.guaranteedDivisor * i;

            regions[currentPart] = tmp;

            currentPart++;
            // End the loop
            break;
        }

        // Add nRowSum entry to currentN
        currentN += prediction->nRowSums[i];
        // Increment usedPredictionRows
        usedPredictionRows++;

        // Reached 1/parts of nSum or there is only one piece of prediction left for each remaining part
        if (currentN >= desiredN || prediction->predictionLengthY - (i + 1) <= parts - currentPart) {
            tmp.minImaginary = col.maxImaginary - (i + 1) * prediction->deltaImaginary;
            tmp.height = col.guaranteedDivisor * usedPredictionRows;

            regions[currentPart] = tmp;

            // Prepare tmp for next part
            tmp.maxImaginary = tmp.minImaginary;
            tmp.vOffset += tmp.height;

            // Debug
            // std::cout << "currentN for part " << currentPart << ": " << currentN << std::endl;
            //---
            // Update cumulativeN
            cumulativeN += currentN;
            // Increment currentPart
            currentPart++;
            // Update desiredN
            desiredN = (prediction->nSum - cumulativeN) / (parts - currentPart);
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
