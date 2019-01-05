#include "RecursiveNaiveBalancer.h"
#include "Region.h"
#include "Fractal.h"

#include <iostream>
#include <string>

const std::string RecursiveNaiveBalancer::NAME = "naiveRecursive";

Region *RecursiveNaiveBalancer::balanceLoad(Region region, int nodeCount) {
	auto *allRegions = new Region[nodeCount];
	
	BalancingContext context = { allRegions, 0, nodeCount, 0, 0.0, 0.0 };
	context.deltaReal = Fractal::deltaReal(region.maxReal, region.minReal, region.width);
	context.deltaImaginary = Fractal::deltaImaginary(region.maxImaginary, region.minImaginary, region.height);

	int partsMade = balancingHelper(region, context);

	if (partsMade != nodeCount) {
		std::cerr << "Too much/few parts were made." << std::endl;
	}

	return allRegions;
}

int RecursiveNaiveBalancer::balancingHelper(Region region, BalancingContext context) {
	// Store region in result
	if (context.partsLeft == 1) {
		context.result[context.resultIndex++] = region;
		return context.resultIndex;
	}

	// Alloc memory for regions
	Region* halves; // Will have length 2

	// Check whether to divide vertically or horizontally
	if (region.width <= region.height || region.width <= region.guaranteedDivisor) {
		halves = halveRegionHorizontally(region, context);
	}
	else if (region.height <= region.width || region.height <= region.guaranteedDivisor) {
		halves = halveRegionVertically(region, context);
	}
	else {
        std::cerr << "Impossible" << std::endl;
	}

	// Explicitly set halves[i] to zero, if needed
	for (int i = 0; i < 2; i++) {
		if (halves[i].width == 0 || halves[i].height == 0) {
			halves[i].minImaginary = 0.0;
			halves[i].maxImaginary = 0.0;

			halves[i].minReal = 0.0;
			halves[i].maxReal = 0.0;

			halves[i].height = 0;
			halves[i].width = 0;

			halves[i].vOffset = 0;
			halves[i].hOffset = 0;
		}
	}

	context.recCounter++;
	int nodeCount = context.partsLeft;

	context.partsLeft = nodeCount / 2 + nodeCount % 2;	// Assign more workers to halves[0], since it tends to be bigger
	context.resultIndex = balancingHelper(halves[0], context);
	
	context.partsLeft = nodeCount / 2;
	context.resultIndex = balancingHelper(halves[1], context);
	// Allocated in halveRegionV/H --> halves is the only pointer left
	delete[] halves;
	return context.resultIndex;
}

Region *RecursiveNaiveBalancer::halveRegionVertically(Region region, BalancingContext context) {
	Region* halves = new Region[2];
	halves[0] = halves[1] = region;

	// Calculate new widths
	halves[1].width = ((region.width / region.guaranteedDivisor) / 2) * region.guaranteedDivisor;
	halves[0].width = region.width - halves[1].width;

	// Calculate new hOffset
	halves[1].hOffset = region.hOffset + halves[0].width;

	// Set complex bounds accordingly
	halves[0].maxReal = halves[1].minReal = region.minReal + (halves[0].width * context.deltaReal);

	return halves;
}

Region *RecursiveNaiveBalancer::halveRegionHorizontally(Region region, BalancingContext context) {
	Region* halves = new Region[2];
	halves[0] = halves[1] = region;
	
	// Calculate new heights
	halves[1].height = ((region.height / region.guaranteedDivisor) / 2) * region.guaranteedDivisor;
	halves[0].height = region.height - halves[1].height;

	// Calculate new vOffset
	halves[1].vOffset = region.vOffset + halves[0].height;

	// Set complex bounds accordingly
	halves[0].minImaginary = halves[1].maxImaginary = region.maxImaginary - (halves[0].height * context.deltaImaginary);

	return halves;
}