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
	// The higher, the better; a part of size guaranteedDivisor * guaranteedDivisor is represented by predictionAccuracy * predictionAccuracy pixels in the prediction
	// Negative accuracy means that a pixel in the prediction represents predictionAccuracy * predictionAccuracy parts of size guaranteedDivisor * guaranteedDivisor
	// Please note: If negative predictionAccuracy has to divide (region.width / region.guaranteedDivisor) and (region.height / region.guaranteedDivisor)
	int predictionAccuracy;
	Fractal* f;
	Region* splitCol(Region col, int parts, int nSum, std::vector<int> n, double deltaImaginary);
public:
	static const std::string NAME;

	Region* balanceLoad(Region region, int nodeCount);
	~PredictionBalancer();
	static PredictionBalancer* create(Fractal* f, int predictionAccuracy);
};

#endif