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
Region *halveRegionHorizontally(Region region, Prediction prediction) {
    Region* halves = new Region[2];
    halves[0] = halves[1] = region;

    int desiredN = prediction.nSum / 2;
    int currentN = 0;

    for (int i = 0; i < prediction.predictionLengthX; i++) {
        currentN += prediction.nColSums[i];
        // Reached 1/2 of nSum or there is only one piece of prediction left for the other half
        if (currentN > desiredN || prediction.predictionLengthX - i <= 1) {
            halves[0].maxReal = region.minReal + (i + 1) * prediction.deltaReal;
            halves[0].width = region.guaranteedDivisor * (i + 1);
            break;
        }
    }

    halves[1].minReal = halves[0].maxReal;
    halves[1].width = region.width - halves[0].width;
    halves[1].hOffset = region.hOffset + halves[0].width;

    return halves;
}

// Halves the region according to prediction
Region *halveRegionVertically(Region region, Prediction prediction) {
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