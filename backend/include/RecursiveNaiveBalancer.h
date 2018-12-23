#pragma once
#ifndef RECURSIVENAIVEBALANCER_H	// Prevents multiple includes of this header
#define RECURSIVENAIVEBALANCER_H

#include "Balancer.h"
#include "Region.h"

#include <string>

struct NaiveBalancingContext {
	Region* result;

	int resultIndex;
	int recCounter;
	int onLowestLevel;

	double deltaReal;
	double deltaImaginary;
};

class RecursiveNaiveBalancer : public Balancer {
private:
	int balancingHelper(Region region, NaiveBalancingContext context);

public:
	static const std::string NAME;
	
	Region* balanceLoad(Region region, int nodeCount) override;
};

#endif