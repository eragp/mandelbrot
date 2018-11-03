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

	// Deal with numbers that don't fit
	
	int recCounter = log2(nodeCount); // Floor or ceil??? --> decide later

	int partsMade = balancingHelper(region, prediction, allRegions, 0, recCounter);
	
	if (partsMade != nodeCount) {
		std::cerr << "Too much/few parts were made." << std::endl;
	}

	return allRegions;
}

int RecursivePredictionBalancer::balancingHelper(Region region, Prediction* prediction, Region* result, int resultIndex, int recCounter) {
	// Store region in result
	if (recCounter == 0) {
		result[resultIndex++] = region;
		delete prediction;
		return resultIndex;
	}

	// Alloc memory for predictions and regions
	Prediction* halve0 = new Prediction();
	Prediction* halve1 = new Prediction();
	Region* halves; // Will have length 2

	// Check whether to divide vertically or horizontally
	if (recCounter % 2 == 0) {
		halves = halveRegionVertically(region, *prediction, halve0, halve1);
	}
	else {
		halves = halveRegionHorizontally(region, *prediction, halve0, halve1);
	}

	// Free prediction since it's no longer needed
	delete prediction;

	recCounter--;
	resultIndex = balancingHelper(halves[0], halve0, result, resultIndex, recCounter);
	return balancingHelper(halves[1], halve1, result, resultIndex, recCounter);
	
}

// Halves the region according to prediction, puts new predictions to left and right
Region *RecursivePredictionBalancer::halveRegionVertically(Region region, Prediction prediction, Prediction* left, Prediction* right) {
	Region* halves = new Region[2];
	halves[0] = halves[1] = region;

	// The deltas stay the same since the ratio is preserved
	left->deltaImaginary = prediction.deltaImaginary;
	left->deltaReal = prediction.deltaReal;
	left->predictionLengthY = prediction.predictionLengthY;
	left->nColSums.resize(prediction.predictionLengthX);

	int desiredN = prediction.nSum / 2;
	int currentN = 0;

	for (int i = 0; i < prediction.predictionLengthX; i++) {
		currentN += prediction.nColSums[i];
		left->nColSums[i] = prediction.nColSums[i];
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
	right->predictionLengthX = prediction.predictionLengthX - left->predictionLengthX;
	right->predictionLengthY = prediction.predictionLengthY;
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
			right->nRowSums[y] += predictionValue;
			if (y == 0) {
				right->nColSums[x] = prediction.nColSums[left->predictionLengthX + x];
			}
		}
	}

	return halves;
}

// Halves the region according to prediction, puts new prediction to top and bot
Region *RecursivePredictionBalancer::halveRegionHorizontally(Region region, Prediction prediction, Prediction* top, Prediction* bot) {
	Region* halves = new Region[2];
	halves[0] = halves[1] = region;

	// The deltas stay the same since the ratio is preserved
	top->deltaImaginary = prediction.deltaImaginary;
	top->deltaReal = prediction.deltaReal;
	top->predictionLengthX = prediction.predictionLengthX;
	top->nRowSums.resize(prediction.predictionLengthY);

	int desiredN = prediction.nSum / 2;
	int currentN = 0;

	for (int i = 0; i < prediction.predictionLengthY; i++) {
		currentN += prediction.nRowSums[i];
		top->nRowSums.push_back(prediction.nRowSums[i]);
		// Reached 1/2 of nSum or there is only one piece of prediction left for the other half
		if (currentN > desiredN || prediction.predictionLengthY - i <= 1) {
			halves[0].minImaginary = region.maxImaginary - (i + 1) * prediction.deltaImaginary;
			halves[0].height = region.guaranteedDivisor * (i + 1);

			top->predictionLengthY = i + 1;
			top->nRowSums.resize(i + 1);
			break;
		}
	}

	top->nSum = currentN;
	top->n.resize(top->predictionLengthX);
	top->nColSums.resize(top->predictionLengthX);

	for (int x = 0; x < top->predictionLengthX; x++) {
		top->nColSums[x] = 0;
		top->n[x].resize(top->predictionLengthY);
		for (int y = 0; y < top->predictionLengthY; y++) {
			int predictionValue = prediction.n[x][y];
			top->n[x][y] = predictionValue;
			top->nColSums[x] += predictionValue;
		}
	}

	halves[1].maxImaginary = halves[0].minImaginary;
	halves[1].height = region.height - halves[0].height;
	halves[1].vOffset = region.vOffset + halves[0].height;

	// The deltas stay the same since the ratio is preserved
	bot->deltaImaginary = prediction.deltaImaginary;
	bot->deltaReal = prediction.deltaReal;
	bot->predictionLengthX = prediction.predictionLengthX;
	bot->predictionLengthY = prediction.predictionLengthY - top->predictionLengthY;
	bot->nSum = prediction.nSum - top->nSum;

	bot->n.resize(bot->predictionLengthX);
	bot->nRowSums.resize(bot->predictionLengthY);
	bot->nColSums.resize(bot->predictionLengthX);

	for (int x = 0; x < bot->predictionLengthX; x++) {
		bot->nColSums[x] = 0;
		bot->n[x].resize(bot->predictionLengthY);
		for (int y = 0; y < bot->predictionLengthY; y++) {
			int predictionValue = prediction.n[x][top->predictionLengthY + y];
			bot->n[x][y] = predictionValue;
			bot->nColSums[x] += predictionValue;
			if (x == 0) {
				bot->nRowSums[y] = prediction.nRowSums[top->predictionLengthY + y];
			}
		}
	}

	return halves;
}

RecursivePredictionBalancer *RecursivePredictionBalancer::create(Fractal *f, int predictionAccuracy) {
	RecursivePredictionBalancer *b = new RecursivePredictionBalancer();
	b->f = f;
	b->predictionAccuracy = predictionAccuracy;
	return b;
}