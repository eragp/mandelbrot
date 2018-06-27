#pragma once
#ifndef PREDICTIONBALANCER_H	// Prevents multiple includes of this header
#define PREDICTIONBALANCER_H
#include "Balancer.h"
#include "RegionOld.h"
#include "Fractal.h"

class PredictionBalancer : public Balancer {
private:
	Fractal * f;
public:
	RegionOld* balanceLoad(RegionOld tile, int nodeCount);
	static PredictionBalancer* create(Fractal* f);
};

#endif