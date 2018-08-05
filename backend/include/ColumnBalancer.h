#pragma once
#ifndef COLUMNBALANCER_H	// Prevents multiple includes of this header
#define COLUMNBALANCER_H

#include "Balancer.h"
#include "Region.h"

#include <string>

class ColumnBalancer : public Balancer {
	public:
		static const std::string NAME;
		Region* balanceLoad(Region region, int nodeCount) override;
};

#endif