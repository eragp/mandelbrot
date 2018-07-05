#include <cmath>
#include "NaiveBalancer.h"
#include "Region.h"

Region* NaiveBalancer::balanceLoad(Region region, int nodeCount)
{

	Region* ret = new Region[nodeCount];

	int xQuantity = (int)floor(sqrt(nodeCount));		// xQuantity = Quantity of separate parts in x direction --> Number of Columns 
	while (nodeCount % xQuantity != 0) {				// Find the next integral divisor of nodeCount smaller than its squareroot
		xQuantity--;
	}

	int yQuantity = nodeCount / xQuantity;	// nodeCount = xQuantity * yQuantity; --> Number of Rows

	int partWidth = (region.width / (region.guaranteedDivisor * xQuantity)) * region.guaranteedDivisor;	// Split Resolution for x, multiple of guaranteedDivisor

	int partHeight = (region.height / (region.guaranteedDivisor * yQuantity)) * region.guaranteedDivisor;	// Split Resolution for y, multiple of guaranteedDivisor

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