#pragma once
#ifndef PREDICTIONBALANCER_H	// Prevents multiple includes of this header
#define PREDICTIONBALANCER_H
#include "Balancer.h"
#include "Region.h"
#include "Fractal.h"

class PredictionBalancer : public Balancer {
private:
	Fractal * f;
public:
	Region* balanceLoad(Region tile, int nodeCount);
	static PredictionBalancer* create(Fractal* f);
};

#endif