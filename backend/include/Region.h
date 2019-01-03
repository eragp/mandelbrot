#pragma once

#include "Fractal.h"

/**
* The new and up-to-date Region struct for view and subdivided regions
*/
struct Region {

    /**
    * The coordinates for the top left corner of the region
    * in the complex plane.
    * Included in the region.
    */
    precision_t minReal, maxImaginary;

    /**
    * The coordinates for the bottom right corner of the region
    * in the complex plane.
    * Excluded from the region.
    */
    precision_t maxReal, minImaginary;

    /**
    * Pixel amount in respective dimension of this region.
    * Equivalent to resolution in x- and y- dimension.
    */
    unsigned int width, height;

    /**
    * Maximum n value for iteration in this region.
    */
    unsigned short int maxIteration;

    /**
    * Frontend specific information, identification/validation value, do not touch
    * With leaflet frontend equivalent to zoomfactor.
    * Used to decide whether the data is still needed.
    */
    int validation;

    /**
    * Value for which it is guaranteed that width and height are divisible by.
    * Same goes for hOffset and vOffset
    * Recursivley applies to all subregions
    */
    unsigned int guaranteedDivisor;

    /**
    * Horizontal and vertical offset respective to superregion.
    * Equivalent to xOffset/yOffset.
    * Should not become negative usually.
    */
    int hOffset, vOffset;

    /**
     * Value to determine, which fractal will be computed
     */
    enum fractal_type fractal;

    /**
     * Do not change.
     * Number of nodes used to compute the corresponding big region.
     */
    unsigned short int nodeCount;

    unsigned int getPixelCount() {
        return width * height;
    }


    long double projectReal(int pixelX) {
        return minReal + (static_cast<long double>(pixelX)
                * (maxReal - minReal)) / width;
    }

    long double projectImag(int pixelY) {
        return minImaginary + (static_cast<long double>(pixelY)
                * (maxImaginary - minImaginary)) / height;
    }
};