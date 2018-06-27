#include "NaiveBalancer.h"
#include <cmath>
#include "RegionOld.h"

//TileInfo* NaiveBalancer::balanceLoad(TileInfo tile, int nodeCount) {
//    TileInfo* ret = new TileInfo[nodeCount];
//
//    int xQuantity = (int)floor(sqrt(nodeCount));  // xQuantity = Quantity of seperate parts in x direction --> Number of Columns
//    while (nodeCount % xQuantity != 0) {
//        xQuantity--;
//    }
//
//    int yQuantity = nodeCount / xQuantity;  // nodeCount = xQuantity * yQuantity; --> Number of Rows
//
//    double xDelta = ((tile.maxReal - tile.minReal) / xQuantity);  // xDelta = x dimension of every tile (except maybe the last) this method returns
//
//    double yDelta = ((tile.maxImaginary - tile.minImaginary) / yQuantity);  //  yDelta = y dimension of every tile (except maybe the last) this method returns
//
//    int xResDelta = tile.xRes / xQuantity;  // Split Resolution for x
//
//    int yResDelta = tile.yRes / yQuantity;  // Split Resolution for y
//
//    int retPos = 0;  // Current position in the Array ret
//    for (int x = 0; x < xQuantity; x++) {
//        for (int y = 0; y < yQuantity; y++) {
//            ret[retPos].minReal = tile.minReal + x * xDelta;
//            ret[retPos].maxReal = tile.minReal + (x + 1) * xDelta;
//            ret[retPos].minImaginary = tile.minImaginary + y * yDelta;
//            ret[retPos].maxImaginary = tile.minImaginary + (y + 1) * yDelta;
//
//            // We want the same resolution as before, so the last part could be a bit larger
//            if (x + 1 == xQuantity) {
//                ret[retPos].xRes = tile.xRes - ((xQuantity - 1) * xResDelta);
//            } else {
//                ret[retPos].xRes = xResDelta;
//            }
//            if (y + 1 == yQuantity) {
//                ret[retPos].yRes = tile.yRes - ((yQuantity - 1) * yResDelta);
//            } else {
//                ret[retPos].yRes = yResDelta;
//            }
//            retPos++;
//        }
//    }
//
//    return ret;
//}