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
Region* PredictionBalancer::balanceLoad(Region region, int nodeCount) {	
	int predictionLengthX = region.width / region.guaranteedDivisor;
	int predictionLengthY = region.height / region.guaranteedDivisor;

	// Declare all variables used by both if branches
	Region lowRes = region;

	// Prediction per part with width and height == guaranteedDivisor
	std::vector<std::vector<int>> n(predictionLengthX, std::vector<int>(predictionLengthY));
	std::vector<int> nColSums(predictionLengthX);
	int nSum = 0;

	double deltaReal = 0.0;
	double deltaImaginary = 0.0;

	if (predictionAccuracy > 0) {
		lowRes.width = (region.width / region.guaranteedDivisor) * predictionAccuracy;
		lowRes.height = (region.height / region.guaranteedDivisor) * predictionAccuracy;

		deltaReal = Fractal::deltaReal(lowRes.maxReal, lowRes.minReal, lowRes.width);
		deltaImaginary = Fractal::deltaImaginary(lowRes.maxImaginary, lowRes.minImaginary, lowRes.height);

		for (unsigned int x = 0; x < lowRes.width; x++) {
			for (unsigned int y = 0; y < lowRes.height; y++) {
				int iterationCount = f->calculateFractal(lowRes.minReal + x * deltaReal, lowRes.maxImaginary - y * deltaImaginary, lowRes.maxIteration);
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

	} else if (predictionAccuracy < 0) {
		// Make predictionAccuracy positive, sign just determines the operation internally predictionAccuracy is always positive
		predictionAccuracy = -predictionAccuracy;

		lowRes.width = predictionLengthX / predictionAccuracy;
		lowRes.height = predictionLengthY / predictionAccuracy;

		// calculate also a prediction for overlaying parts of size guaranteedDivisor * guaranteedDivisor
		if (predictionLengthX % predictionAccuracy != 0) {
			lowRes.width += 1;
		}
		if (predictionLengthY % predictionAccuracy != 0) {
			lowRes.height += 1;
		}

		deltaReal = Fractal::deltaReal(lowRes.maxReal, lowRes.minReal, lowRes.width);
		deltaImaginary = Fractal::deltaImaginary(lowRes.maxImaginary, lowRes.minImaginary, lowRes.height);

		for (unsigned int x = 0; x < lowRes.width; x++) {
			for (unsigned int y = 0; y < lowRes.height; y++) {
				int iterationCount = f->calculateFractal(lowRes.minReal + x * deltaReal, lowRes.maxImaginary - y * deltaImaginary, lowRes.maxIteration);
				
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
		std::cerr << "predictionAccuracy mustn't be 0." << std::endl;
		return nullptr;
	}

	// Debug
	std::cout << "nSum: " << nSum << std::endl;
	std::cout << "nColSums[0]: " << nColSums[0] << std::endl;
	//---
	Region* allRegions = new Region[nodeCount];

	// Same as in naive
	int cols = (int)floor(sqrt(nodeCount));		// cols = Quantity of separate parts in x direction --> Number of Columns 
	while (nodeCount % cols != 0) {				// Find the next integral divisor of nodeCount smaller than its squareroot
		cols--;
	}

	int rows = nodeCount / cols;	// nodeCount = cols * rows; --> Number of Rows

	int desiredN = nSum / cols;

	int currentN = 0; // Obviously needed
	int currentCol = 0; // Obviously needed
	int cumulativeN = 0; // Needed to update desiredN
	int usedPredictionCols = 0; // Needed to determine resolution

	Region tmp;
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

			Region* colRegions = PredictionBalancer::splitCol(tmp, rows, nSum - cumulativeN, colPrediction, deltaImaginary); //length: rows

			for (int j = 0; j < rows; j++) {
				allRegions[rows * currentCol + j] = colRegions[j];
			}

			delete[] colRegions;

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

			Region* colRegions = PredictionBalancer::splitCol(tmp, rows, currentN, colPrediction, deltaImaginary); //length: rows

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
	/* // Debug: return the col vector as array
	std::cout << "Cols: " << cols << std::endl;
	TileInfo* colArray = new TileInfo[cols];
	for (int i = 0; i < cols; i++) {
		colArray[i] = colTiles[i];
	}
	// return colArray;
	//---
	for (int i = 0; i < cols; i++) {
		TileInfo t = colArray[i];
	}*/

	return allRegions;
}

Region* PredictionBalancer::splitCol(Region col, int parts, int nSum, std::vector<int> n, double deltaImaginary) {
	Region* regions = new Region[parts];

	int desiredN = nSum / parts;

	int currentN = 0;
	int currentPart = 0;
	int cumulativeN = 0;
	int usedPredictionRows = 0;

	Region tmp;
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

	for (unsigned int i = 0; i < n.size(); i++) {
		if (currentPart == parts - 1) {
			// Take the rest
			tmp.minImaginary = col.minImaginary;
			tmp.height = col.height - col.guaranteedDivisor * i;

			regions[currentPart] = tmp;
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

	return regions;
}

PredictionBalancer* PredictionBalancer::create(Fractal* f, int predictionAccuracy) {
	PredictionBalancer* b = new PredictionBalancer();
	b->f = f;
	b->predictionAccuracy = predictionAccuracy;
	return b;
}