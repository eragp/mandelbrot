#pragma once
#ifndef PREDICTIONBALANCER_H	// Prevents multiple includes of this header
#define PREDICTIONBALANCER_H

#include "Balancer.h"
#include "Region.h"
#include "Fractal.h"

#include <vector>
#include <string>

class PredictionBalancer : public Balancer {
private:
	// The lower, the better; predictionAccuracy = 1 means that the Fractal is calculated in full resolution for prediction
	// predictionAccuracy has to divide guaranteedDivisor of passed region
	int predictionAccuracy;
	Fractal* f;
	Region* splitCol(Region col, int parts, int nSum, std::vector<int> n, double deltaImaginary);
public:
	static const std::string NAME;
	Region* balanceLoad(Region region, int nodeCount);
	static PredictionBalancer* create(Fractal* f, int predictionAccuracy);
};

#endif