#pragma once
#ifndef NAIVEBALANCER_H	// Prevents multiple includes of this header
#define NAIVEBALANCER_H
#include "Balancer.h"
#include "Region.h"

class NaiveBalancer : public Balancer {
	public:
		Region* balanceLoad(Region tile, int nodeCount);
};

#endif