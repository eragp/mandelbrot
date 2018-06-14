#include "Fractal.h"

// double Fractal::deltaReal(double maxReal, double minReal, int xRes) {
// 	return (maxReal - minReal) / xRes;
// }

// double Fractal::deltaImaginary(double maxImaginary, double minImaginary, int yRes) {
// 	return (maxImaginary - minImaginary) / yRes;
// }
const double boundX = 4;
const double boundY = 4;

long double Fractal::xToReal(long x, long zoom, long localX, long size) {
    return unproject(x, zoom, boundX, localX, size);
}

long double Fractal::yToImaginary(long y, long zoom, long localY, long size) {
    return -unproject(y, zoom, boundY, localY, size);
}

/**
 * unproject maps form tile cooridate space to the comples plane.
 */
long double unproject(long x, long zoom, double bound, long local, long size) {
    if (size == 0) {
        size = 1;
    }
    long long tile_count = 1L << zoom;
    return (long double)(x * bound * size + local * bound) /
           (long double)(tile_count * size);
}

// int Fractal::realToX(double real, double maxReal, double minReal, int width) {
// 	return (int)((real - minReal) / ((maxReal - minReal) / width));
// }

// int Fractal::imaginaryToY(double imaginary, double maxImaginary, double minImaginary, int height) {
// 	return (int)((imaginary - minImaginary) / ((maxImaginary - minImaginary) / height));
// }
