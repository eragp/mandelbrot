#pragma once
#ifndef NAIVEBALANCER_H	// Prevents multiple includes of this header
#define NAIVEBALANCER_H
#include "Balancer.h"
#include "RegionOld.h"

class NaiveBalancer : public Balancer {
	public:
		RegionOld* balanceLoad(RegionOld tile, int nodeCount);
};

#endif