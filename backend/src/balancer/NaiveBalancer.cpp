#include "NaiveBalancer.h"
#include "Region.h"

#include <cmath>
#include <string>
#include <iostream>

const std::string NaiveBalancer::NAME = "naive";
const Region emptyRegion = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void balanceLoadRecursively(Region region, int nodeCount, Region* dest){
    std::cout << "Slicing region "
              << " TopLeft: (" << region.minReal << ", " << region.maxImaginary << ") -> BottomRight: ("
              << region.maxReal << ", " << region.minImaginary << ") Resolution: ("
              << region.width << ", " << region.height << ") Position: (" <<region.hOffset << ", " << region.vOffset << ")"<< std::endl;

    unsigned int gD = region.guaranteedDivisor;
    unsigned int halvWidth = region.width/2;
    unsigned int halvHeight = region.height/2;
    // Note that guaranteed divisor should divide the height and width from the beginning on
    if(nodeCount == 1 || (halvWidth < gD && halvHeight < gD)){
        // We cannot divide this region any further
        // so we write the current region and empty regions for the rest
        dest[0] = region;
        for(int i = 1; i < nodeCount; i++){
            dest[i] = emptyRegion;
        }
        return;
    }
    Region firstRegion = region;
    Region secondRegion = region;
    // Determine by width/height how to divide 
    // A division is only appropiate if the resulting halve is still greater than gD
    if(halvWidth >= gD && region.width >= region.height){
        firstRegion.width = halvWidth;
        secondRegion.width = halvWidth;
        long double midReal = (region.minReal + region.maxReal) / 2;
        firstRegion.maxReal = midReal;
        secondRegion.minReal = midReal;
        secondRegion.hOffset = region.hOffset + halvWidth;
    }
    else if(halvHeight >= gD){
        firstRegion.height = halvHeight;
        secondRegion.height = halvHeight;
        long double midImag = (region.minImaginary + region.maxImaginary) / 2;
        firstRegion.minImaginary = midImag;
        secondRegion.maxImaginary = midImag;
        secondRegion.vOffset = region.vOffset + halvHeight;
    }
    else{
        std::cout << "Height of " << halvHeight << ", still slicing? gd: " << region.guaranteedDivisor << std::endl;
    }
    int halvNodes = nodeCount/2;
    balanceLoadRecursively(firstRegion, halvNodes, dest);
    balanceLoadRecursively(secondRegion, nodeCount-halvNodes, dest+halvNodes);
}

// Recursively divides region in half and balances load onto them
Region* NaiveBalancer::balanceLoad(Region region, int nodeCount)
{
    Region* result = new Region[nodeCount];
    if(region.guaranteedDivisor == 0){
        region.guaranteedDivisor = 1;
    }
    if(nodeCount > 0){
        balanceLoadRecursively(region, nodeCount, result);
    }
    return result;
}