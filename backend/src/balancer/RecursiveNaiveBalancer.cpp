#include "RecursiveNaiveBalancer.h"
#include "Region.h"
#include "Fractal.h"

#include <iostream>
#include <cmath>
#include <string>

const std::string RecursiveNaiveBalancer::NAME = "naiveRecursive";

Region *RecursiveNaiveBalancer::balanceLoad(Region region, int nodeCount) {
	auto *allRegions = new Region[nodeCount];

	int recCounter = (int)floor(log2(nodeCount));
	// Deal with numbers that are not powers of 2
	int onLowestLevel = (int)(nodeCount - pow(2, recCounter)) * 2;
	recCounter++;
	
	BalancingContext context = { allRegions, 0, recCounter, onLowestLevel, 0.0, 0.0 };
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
	if (context.recCounter == 0 || (context.recCounter == 1 && context.resultIndex >= context.onLowestLevel)) {
		context.result[context.resultIndex++] = region;
		return context.resultIndex;
	}

	// Alloc memory for regions
	Region* halves; // Will have length 2

	// Check whether to divide vertically or horizontally
	if (region.width <= region.guaranteedDivisor) {
		halves = halveRegionHorizontally(region, context);
	}
	else if (region.height <= region.guaranteedDivisor) {
		halves = halveRegionVertically(region, context);
	}
	else if (context.recCounter % 2 == 0) {
		halves = halveRegionVertically(region, context);
	}
	else {
		halves = halveRegionHorizontally(region, context);
	}

	// Explicitly set halves[1] to zero, if needed (not needed for halves[0] since in this case also region is 0)
	if (halves[1].width == 0 || halves[1].height == 0) {
		halves[1].minImaginary = 0.0;
		halves[1].maxImaginary = 0.0;

		halves[1].minReal = 0.0;
		halves[1].maxReal = 0.0;

		halves[1].height = 0;

		halves[1].vOffset = 0;
		halves[1].hOffset = 0;
	}

	context.recCounter--;
	context.resultIndex = balancingHelper(halves[0], context);
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