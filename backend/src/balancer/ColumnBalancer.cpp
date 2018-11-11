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
    int nodes = nodeCount;
    Region* regions = new Region[nodeCount];

    int columnWidth = (region.width / (region.guaranteedDivisor * nodeCount)) * region.guaranteedDivisor;
    // If there are more nodes than guaranteedDivisor*guaranteedDivisor cells, the balancer needs to make empty regions
    if (columnWidth <= 0) {
        columnWidth = region.guaranteedDivisor;
        int spareNodes = nodeCount - (region.width / region.guaranteedDivisor);
        
        // Put empty regions for spare nodes
        Region empty;
        empty.minImaginary = 0.0;
        empty.maxImaginary = 0.0;

        empty.minReal = 0.0;
        empty.maxReal = 0.0;

        empty.height = 0;
        empty.width = 0;

        empty.vOffset = 0;
        empty.hOffset = 0;

        empty.maxIteration = region.maxIteration;
        empty.validation = region.validation;
        empty.guaranteedDivisor = region.guaranteedDivisor;
        for (int i = 0; i < spareNodes; i++) {
            regions[nodeCount - 1 - i] = empty;
        }

        nodes -= spareNodes;
    }
    double xDelta = Fractal::deltaReal(region.maxReal, region.minReal, region.width) * columnWidth;

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

    for (int i = 0; i < nodes; i++) {
       if (i == nodes - 1) {
           // Take the rest
           tmp.maxReal = region.maxReal;
		   tmp.width = region.width - columnWidth * i;
           regions[i] = tmp;
       } else {
           // We don't want inaccuracy from repeatedly adding doubles
           tmp.maxReal = region.minReal + (i + 1) * xDelta;
           tmp.width = columnWidth;

           regions[i] = tmp;
           // Update minReal and hOffset
           tmp.minReal = tmp.maxReal;
           tmp.hOffset += tmp.width;
       }
    }
    
    return regions;
}