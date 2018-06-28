#include "IntegerBalancer.h"
#include <cmath>
#include "RegionOld.h"

/**
 * IntergerBalancer takes a region and separates it into #nodeCount
 * columns tile sets. (eg. region is split into #nodeCount columns)
 */
RegionOld *IntegerBalancer::balanceLoad(RegionOld region, const int nodeCount) {
    auto regions = new RegionOld[nodeCount];
    int regionWidth = region.getWidth();
    int columnWidth = regionWidth / nodeCount;
    int x = region.tlX;
    for (int i = 0; i < nodeCount; i++) {
        RegionOld r{};
        regions[i] = r;
        regions[i].tlX = x;
        regions[i].brX = (i == nodeCount - 1) ? region.brX : (x + columnWidth);
        // just copying the unchanged values for each RegionOld struct
        regions[i].tlY = region.tlY;
        regions[i].brY = region.brY;
        regions[i].maxIteration = region.maxIteration;
        regions[i].resX = region.resX;
        regions[i].resY = region.resY;
        regions[i].zoom = region.zoom;
        x += columnWidth + 1;
    }
    return regions;
}