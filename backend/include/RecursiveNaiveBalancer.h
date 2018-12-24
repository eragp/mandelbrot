#pragma once
#ifndef RECURSIVENAIVEBALANCER_H	// Prevents multiple includes of this header
#define RECURSIVENAIVEBALANCER_H

#include "Balancer.h"
#include "Region.h"
#include "BalancingContext.h"

#include <string>


class RecursiveNaiveBalancer : public Balancer {
private:
	int balancingHelper(Region region, BalancingContext context);

	Region *halveRegionVertically(Region region, BalancingContext context);

	Region *halveRegionHorizontally(Region region, BalancingContext context);

public:
	static const std::string NAME;
	
	Region* balanceLoad(Region region, int nodeCount) override;
};

#endif