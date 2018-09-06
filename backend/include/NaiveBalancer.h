#pragma once
#ifndef NAIVEBALANCER_H	// Prevents multiple includes of this header
#define NAIVEBALANCER_H

#include "Balancer.h"
#include "Region.h"

#include <string>

class NaiveBalancer : public Balancer {
	public:
		static const std::string NAME;
		Region* balanceLoad(Region region, int nodeCount) override;
};

#endif