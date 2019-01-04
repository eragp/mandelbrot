#pragma once
#ifndef BALANCINGCONTEXT_H	// Prevents multiple includes of this header
#define BALANCINGCONTEXT_H

#include "Region.h"

struct BalancingContext {
	Region* result;

	int resultIndex;
	int partsLeft;
	int recCounter;

	double deltaReal;
	double deltaImaginary;
};

#endif