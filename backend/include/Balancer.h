#pragma once
#ifndef BALANCER_H	// Prevents multiple includes of this header
#define BALANCER_H
#include "Region.h"

class Balancer {
	public:
		virtual Region* balanceLoad(Region region, int nodeCount) = 0;
};

#endif