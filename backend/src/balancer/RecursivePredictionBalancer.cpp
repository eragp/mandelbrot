#include "RecursivePredictionBalancer.h"
#include "Region.h"
#include "Fractal.h"
#include "Predicter.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <string>

const std::string RecursivePredictionBalancer::NAME = "predictionRecursive";

RecursivePredictionBalancer::~RecursivePredictionBalancer() {
    delete f;
}

// Worst case scenario: Prediction changes suddenly from small to big values
Region *RecursivePredictionBalancer::balanceLoad(Region region, int nodeCount) {
    auto *allRegions = new Region[nodeCount];
    Prediction* prediction = Predicter::getPrediction(region, f, predictionAccuracy);

    // Determine number of recursive steps --> log2
    // Deal with numbers that don't fit
    // Update the prediction after halving

    return allRegions;
}

// Halves the region according to prediction
Region *halveRegionVertically(Region region, Prediction prediction, Prediction* left, Prediction* right) {
    Region* halves = new Region[2];
    halves[0] = halves[1] = region;

	// The deltas stay the same since the ratio is preserved
	left->deltaImaginary = prediction.deltaImaginary;
	left->deltaReal = prediction.deltaReal;
	left->predictionLengthY = prediction.predictionLengthY;

    int desiredN = prediction.nSum / 2;
    int currentN = 0;

    for (int i = 0; i < prediction.predictionLengthX; i++) {
        currentN += prediction.nColSums[i];
		left->nColSums.push_back(prediction.nColSums[i]);
        // Reached 1/2 of nSum or there is only one piece of prediction left for the other half
        if (currentN > desiredN || prediction.predictionLengthX - i <= 1) {
            halves[0].maxReal = region.minReal + (i + 1) * prediction.deltaReal;
            halves[0].width = region.guaranteedDivisor * (i + 1);

			left->predictionLengthX = i + 1;
			left->nColSums.resize(i + 1);
            break;
        }
    }

	left->nSum = currentN;
	left->n.resize(left->predictionLengthX);
	left->nRowSums.resize(left->predictionLengthY);

	for (int y = 0; y < left->predictionLengthY; y++) {
		left->nRowSums[y] = 0;
		for (int x = 0; x < left->predictionLengthX; x++) {
			if (y == 0) {
				left->n[x].resize(left->predictionLengthY);
			}
			int predictionValue = prediction.n[x][y];
			left->n[x][y] = predictionValue;
			left->nRowSums[y] += predictionValue;
		}
	}


    halves[1].minReal = halves[0].maxReal;
    halves[1].width = region.width - halves[0].width;
    halves[1].hOffset = region.hOffset + halves[0].width;

	// The deltas stay the same since the ratio is preserved
	right->deltaImaginary = prediction.deltaImaginary;
	right->deltaReal = prediction.deltaReal;
	right->predictionLengthY = prediction.predictionLengthY;
	right->predictionLengthX = prediction.predictionLengthX - left->predictionLengthX;
	right->nSum = prediction.nSum - left->nSum;

	right->n.resize(right->predictionLengthX);
	right->nRowSums.resize(right->predictionLengthY);
	right->nColSums.resize(right->predictionLengthX);

	for (int y = 0; y < right->predictionLengthY; y++) {
		right->nRowSums[y] = 0;
		for (int x = 0; x < right->predictionLengthX; x++) {
			if (y == 0) {
				right->n[x].resize(right->predictionLengthY);
			}
			int predictionValue = prediction.n[left->predictionLengthX + x][y];
			right->n[x][y] = predictionValue;
			right->nColSums[x] = prediction.nColSums[left->predictionLengthX + x];
			right->nRowSums[y] += predictionValue;
		}
	}

    return halves;
}


// Halves the region according to prediction
Region *halveRegionHorizontally(Region region, Prediction prediction) {
    Region* halves = new Region[2];
    halves[0] = halves[1] = region;

    int desiredN = prediction.nSum / 2;
    int currentN = 0;

    for (int i = 0; i < prediction.predictionLengthY; i++) {
        currentN += prediction.nRowSums[i];
        // Reached 1/2 of nSum or there is only one piece of prediction left for the other half
        if (currentN > desiredN || prediction.predictionLengthY - i <= 1) {
            halves[0].minImaginary = region.maxImaginary - (i + 1) * prediction.deltaImaginary;
            halves[0].height = region.guaranteedDivisor * (i + 1);
            break;
        }
    }

    halves[1].maxImaginary= halves[0].minImaginary;
    halves[1].height = region.height - halves[0].height;
    halves[1].vOffset = region.vOffset + halves[0].height;

    return halves;
}

RecursivePredictionBalancer *RecursivePredictionBalancer::create(Fractal *f, int predictionAccuracy) {
    RecursivePredictionBalancer *b = new RecursivePredictionBalancer();
    b->f = f;
    b->predictionAccuracy = predictionAccuracy;
    return b;
}