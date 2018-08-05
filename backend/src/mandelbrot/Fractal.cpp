#include "Fractal.h"

const long double boundX = 4;
const long double boundY = 4;

Fractal::~Fractal() {}

double Fractal::deltaReal(double maxReal, double minReal, int xRes) {
    return (maxReal - minReal) / xRes;
}

double Fractal::deltaImaginary(double maxImaginary, double minImaginary, int yRes) {
    return (maxImaginary - minImaginary) / yRes;
}


/**
 * unprojects from Tile coordinate space to the complex plane.
 *
 */
long double unproject(long x, long zoom, double bound, long local, long size) {
    if (size == 0) {
        size = 1;
    }
    long long tile_count = static_cast<unsigned long long>(1) << static_cast<unsigned long long>(zoom);
    return (long double) (x * bound * size + local * bound) /
           (long double) (tile_count * size);
}


long double Fractal::xToReal(long x, long zoom, long localX, long size) {
    return unproject(x, zoom, boundX, localX, size);
}

long double Fractal::yToImaginary(long y, long zoom, long localY, long size) {
    return unproject(y, zoom, boundY, localY, size);
}

