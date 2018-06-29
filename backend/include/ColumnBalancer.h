#pragma once
#ifndef COLUMNBALANCER_H	// Prevents multiple includes of this header
#define COLUMNBALANCER_H
#include "Balancer.h"
#include "Region.h"

class ColumnBalancer : public Balancer {
	public:
		Region* balanceLoad(Region region, int nodeCount) override;
};

#endif