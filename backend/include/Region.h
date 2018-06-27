/**
 * The new and up-to-date Region struct for view and subdivided regions
 */
struct Region {

    /**
     * The coordinates for the top left corner of the region
     * in real values. 
     * Included in the region.
     */
    long double minReal, maxImaginary;

    /**
     * The coordinates for the bottom right corner of the region
     * in real values.
     * Excluded from the region.
     */
    long double maxReal, minImaginary;

    /**
     * Pixel amount in respective dimension of this region.
     * Equivalent to resolution in x- and y- dimension.
     */
    unsigned int width, height;

    /**
     * Maximum n value for iuteration in this region.
     */
    unsigned int maxIteration;

    /**
     * Forntend specific information, identification/ validation value value, do not touch
     * With leaflet frontend equivalent to zoomfactor.
     */
    int validation;

    /**
     * Value for which it is guaranteed that widht and height are divisible by.
     * Recursivley applies to all subregions
     */
    unsigned int guaranteedDivisor; 

    /**
     * Horizonatl and vertical offset respective to superregion.
     * Equivalent to xOffset/yOffset.
     * Should not become negative usually.
     */
    int hOffset, vOffset;


};