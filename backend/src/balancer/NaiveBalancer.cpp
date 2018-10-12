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

	double xDelta = ((region.maxReal - region.minReal) / region.width) * partWidth;		// xDelta = x dimension of every region (except maybe the last) this method returns

	double yDelta = ((region.maxImaginary - region.minImaginary) / region.height) * partHeight;	//  yDelta = y dimension of every region (except maybe the last) this method returns

	int retPos = 0;	// Current position in the Array ret
	for (int x = 0; x < xQuantity; x++) {
		for (int y = 0; y < yQuantity; y++) {
			ret[retPos].maxIteration = region.maxIteration;
			ret[retPos].validation = region.validation;
			ret[retPos].guaranteedDivisor = region.guaranteedDivisor;

			ret[retPos].minReal = region.minReal + x * xDelta;
			ret[retPos].maxImaginary = region.maxImaginary - y * yDelta;

			ret[retPos].maxReal = region.minReal + (x + 1) * xDelta;
			ret[retPos].minImaginary = region.maxImaginary - (y + 1) * yDelta;

			ret[retPos].width = partWidth;
			ret[retPos].height = partHeight;

			ret[retPos].hOffset = x * partWidth;
			ret[retPos].vOffset = y * partHeight;

			// We want the same resolution as before, so the last part could be a bit larger
			if (x + 1 == xQuantity) {
				ret[retPos].width = region.width - ((xQuantity - 1) * partWidth);
				ret[retPos].maxReal = region.maxReal;
			}

			if (y + 1 == yQuantity) {
				ret[retPos].height = region.height - ((yQuantity - 1) * partHeight);
				ret[retPos].minImaginary = region.minImaginary;
			}
			retPos++;
		}
	}

	return ret;
}