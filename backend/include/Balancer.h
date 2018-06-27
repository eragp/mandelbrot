#pragma once
#ifndef BALANCER_H	// Prevents multiple includes of this header
#define BALANCER_H
#include "RegionOld.h"

class Balancer {
	public:
		virtual RegionOld* balanceLoad(RegionOld region, int nodeCount) = 0;
};

#endif