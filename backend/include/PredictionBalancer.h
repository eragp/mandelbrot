#pragma once
#ifndef PREDICTIONBALANCER_H	// Prevents multiple includes of this header
#define PREDICTIONBALANCER_H
#include "Balancer.h"
#include "TileInfo.h"
#include "Fractal.h"

class PredictionBalancer : public Balancer {
private:
	Fractal * f;
public:
	TileInfo* balanceLoad(TileInfo tile, int nodeCount);
	static PredictionBalancer* create(Fractal* f);
};

#endif