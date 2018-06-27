#pragma once
#ifndef NAIVEBALANCER_H	// Prevents multiple includes of this header
#define NAIVEBALANCER_H
#include "Balancer.h"
#include "RegionOld.h"

class IntegerBalancer : public Balancer {
	public:
		RegionOld* balanceLoad(RegionOld region, int nodeCount) override;
};

#endif