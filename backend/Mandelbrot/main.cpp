#include <iostream>
#include <fstream>
#include "Fractal.h"
#include "Mandelbrot.h"

using namespace std;

double xToReal(int x, double maxReal, double minReal, int width) {
	return x * ((maxReal - minReal) / width) + minReal;
}

double yToImaginary(int y, double maxImaginary, double minImaginary, int height) {
	return y * ((maxImaginary - minImaginary) / height) + minImaginary;
}

int main() {
	int height = 2048;
	int width = 2048;
	int maxIteration = 200;
	double minReal = -1.5;
	double maxReal = 0.7;
	double minImaginary = -1.0;
	double maxImaginary = 1.0;
	Fractal* f = new Mandelbrot();

	ofstream fout("Mandelbrot.ppm");
	fout << "P3" << endl;
	fout << width << " " << height << endl;
	fout << "256" << endl;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int n = f->calculateFractal(xToReal(x, maxReal, minReal, width), yToImaginary(y, maxImaginary, minImaginary, height), maxIteration);
			int r = 0;
			int g = 0;
			int b = 0;
			if (n != maxIteration) {
				r = n * 10 % 256;
				g = n * 20 % 256;
				b = n * 40 % 256;
			}
			fout << r << " " << g << " " << b << " ";
		}
		fout << endl;
	}

	fout.close();
	free(f);
	cout << "\a" << "Fertig!" << endl;
	cin.get();
	return 0;
}
