#include "NaiveBalancer.h"
#include "Region.h"

#include <cmath>
#include <string>
#include <iostream>

const std::string NaiveBalancer::NAME = "naive";

Region* NaiveBalancer::balanceLoad(Region region, int nodeCount)
{

	Region* ret = new Region[nodeCount];

	int xQuantity = (int)floor(sqrt(nodeCount));		// xQuantity = Quantity of separate parts in x direction --> Number of Columns 
	while (nodeCount % xQuantity != 0) {				// Find the next integral divisor of nodeCount smaller than its squareroot
		xQuantity--;
	}

	int yQuantity = nodeCount / xQuantity;	// nodeCount = xQuantity * yQuantity; --> Number of Rows

	// Define an empty region
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

	int partWidth = (region.width / (region.guaranteedDivisor * xQuantity)) * region.guaranteedDivisor;	// Split Resolution for x, multiple of guaranteedDivisor
	
	if (partWidth <= 0) {
		partWidth = region.guaranteedDivisor;
		int spareCols = xQuantity - (region.width / region.guaranteedDivisor);
		// Set affected regions to empty
		for (int i = 0; i < spareCols; i++) {
			for (int j = 0; j < yQuantity; j++) {
				ret[(xQuantity - 1 - i) * yQuantity + j] = empty;
			}
		}
		xQuantity -= spareCols;
	}

	int nodesWithExtendedWidth = (region.width - (partWidth * xQuantity)) / region.guaranteedDivisor;

	int partHeight = (region.height / (region.guaranteedDivisor * yQuantity)) * region.guaranteedDivisor;	// Split Resolution for y, multiple of guaranteedDivisor

	if (partHeight <= 0) {
		partHeight = region.guaranteedDivisor;
		int spareRows = yQuantity - (region.height / region.guaranteedDivisor);
		// Set affected regions to empty
		for (int i = 0; i < spareRows; i++) {
			for (int j = 0; j < xQuantity; j++) {
				ret[j * yQuantity + (yQuantity - 1 - i)] = empty;
			}
		}
		yQuantity -= spareRows;
	}

	int nodesWithExtendedHeight = (region.height - (partHeight * yQuantity)) / region.guaranteedDivisor;

	double xDelta = ((region.maxReal - region.minReal) / region.width);		// xDelta = x dimension of every region (except maybe the last) this method returns
	double extendedXDelta = xDelta * (partWidth + region.guaranteedDivisor);
	xDelta *= partWidth;

	double yDelta = ((region.maxImaginary - region.minImaginary) / region.height);	//  yDelta = y dimension of every region (except maybe the last) this method returns
	double extendedYDelta = yDelta * (partHeight + region.guaranteedDivisor);
	yDelta *= partHeight;

	int retPos = 0;	// Current position in the Array ret
	for (int x = 0; x < xQuantity; x++) {
		for (int y = 0; y < yQuantity; y++) {
			ret[retPos].maxIteration = region.maxIteration;
			ret[retPos].validation = region.validation;
			ret[retPos].guaranteedDivisor = region.guaranteedDivisor;

			if (x < nodesWithExtendedWidth) {
				// Take one additional piece (width += region.guaranteedDivisor), all parts before were extended
				ret[retPos].minReal = region.minReal + x * extendedXDelta;
				ret[retPos].maxReal = region.minReal + (x + 1) + extendedXDelta;
				ret[retPos].width = partWidth + region.guaranteedDivisor;
				ret[retPos].hOffset = x * (partWidth + region.guaranteedDivisor);
			} else {
				// Normal width, take account of previous extended parts
				ret[retPos].minReal = region.minReal + nodesWithExtendedWidth * extendedXDelta + (x - nodesWithExtendedWidth) * xDelta;
				ret[retPos].maxReal = region.minReal + nodesWithExtendedWidth * extendedXDelta + (x - nodesWithExtendedWidth + 1) * xDelta;
				ret[retPos].width = partWidth;
				ret[retPos].hOffset = x * partWidth + nodesWithExtendedWidth * region.guaranteedDivisor;
			}

			if (y < nodesWithExtendedHeight) {
				// Take one additional piece (height += region.guaranteedDivisor), all parts before were extended
				ret[retPos].maxImaginary = region.maxImaginary - y * extendedYDelta;
				ret[retPos].minImaginary = region.maxImaginary - (y + 1) * extendedYDelta;
				ret[retPos].height = partHeight + region.guaranteedDivisor;
				ret[retPos].vOffset = y * (partHeight + region.guaranteedDivisor);
			} else {
				// Normal height, take account of previous extended parts
				ret[retPos].maxImaginary = region.maxImaginary - nodesWithExtendedHeight * extendedYDelta - (y - nodesWithExtendedHeight) * yDelta;
				ret[retPos].minImaginary = region.maxImaginary - nodesWithExtendedHeight * extendedYDelta - (y - nodesWithExtendedHeight + 1) * yDelta;
				ret[retPos].height = partHeight;
				ret[retPos].vOffset = y * partHeight + nodesWithExtendedHeight * region.guaranteedDivisor;
			}
			
			// Special cases for parts at the end are no longer needed
			// The above already makes sure that the whole width and height is covered
			
			retPos++;
		}
	}

	return ret;
}