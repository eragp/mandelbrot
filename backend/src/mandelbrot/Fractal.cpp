#include "Fractal.h"

double Fractal::xToReal(int x, double maxReal, double minReal, int width) {
	return x * ((maxReal - minReal) / width) + minReal;
}

double Fractal::yToImaginary(int y, double maxImaginary, double minImaginary, int height) {
	return y * ((maxImaginary - minImaginary) / height) + minImaginary;
}

int Fractal::realToX(double real, double maxReal, double minReal, int width) {
	return (int)((real - minReal) / ((maxReal - minReal) / width));
}

int Fractal::imaginaryToY(double imaginary, double maxImaginary, double minImaginary, int height) {
	return (int)((imaginary - minImaginary) / ((maxImaginary - minImaginary) / height));
}
