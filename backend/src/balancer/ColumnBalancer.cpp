#include "ColumnBalancer.h"
#include "Region.h"
#include "Fractal.h"

#include <cmath>
#include <string>

const std::string ColumnBalancer::NAME = "column";

/**
 * IntergerBalancer takes a region and separates it into #nodeCount
 * columns tile sets. (eg. region is split into #nodeCount columns)
 */
Region* ColumnBalancer::balanceLoad(Region region, const int nodeCount) {
    Region* regions = new Region[nodeCount];

    int columnWidth = (region.width / (region.guaranteedDivisor * nodeCount)) * region.guaranteedDivisor;
    int xDelta = Fractal::deltaReal(region.maxReal, region.minReal, region.width) * columnWidth;

    Region tmp;
	// These stay the same over all iterations
	tmp.minImaginary = region.minImaginary;
	tmp.maxImaginary = region.maxImaginary;
	tmp.height = region.height;
	tmp.maxIteration = region.maxIteration;
	tmp.vOffset = region.vOffset;
	tmp.validation = region.validation;
	tmp.guaranteedDivisor = region.guaranteedDivisor;

	// This will change, so will width and maxReal
	tmp.minReal = region.minReal;
	tmp.hOffset = region.hOffset;

    for (int i = 0; i < nodeCount; i++) {
       if (i == nodeCount - 1) {
           // Take the rest
           tmp.maxReal = region.maxReal;
		   tmp.width = region.width - columnWidth * i;
           regions[i] = tmp;
       } else {
           tmp.maxReal = tmp.minReal + xDelta;
           tmp.width = columnWidth;

           regions[i] = tmp;
           // Update minReal and hOffset
           tmp.minReal = tmp.maxReal;
           tmp.hOffset += tmp.width;
       }
    }
    
    return regions;
}