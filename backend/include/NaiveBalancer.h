#pragma once
#ifndef NAIVEBALANCER_H	// Prevents multiple includes of this header
#define NAIVEBALANCER_H
#include "Balancer.h"
#include "TileInfo.h"

class NaiveBalancer : public Balancer {
	public:
		TileInfo* balanceLoad(TileInfo tile, int nodeCount);
};

#endif