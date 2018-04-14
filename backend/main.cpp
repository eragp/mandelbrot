#include <iostream>
#include <fstream>

using namespace std;

int calculateMandelbrot(double cReal, double cImaginary, int maxIteration) {
	int i = 0;
	double zReal = 0.0;
	double zImaginary = 0.0;
	while (i < maxIteration && zReal * zReal + zImaginary * zImaginary < 4.0) {
		double nextZReal = (zReal * zReal - zImaginary * zImaginary) + cReal;
		double nextZImaginary = 2 * (zReal * zImaginary) + cImaginary;
		zReal = nextZReal;
		zImaginary = nextZImaginary;
		i++;
	}

	return i;
}

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

	ofstream fout("Mandelbrot.ppm");
	fout << "P3" << endl;
	fout << width << " " << height << endl;
	fout << "256" << endl;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int n = calculateMandelbrot(xToReal(x, maxReal, minReal, width), yToImaginary(y, maxImaginary, minImaginary, height), maxIteration);
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
	cout << "Fertig!" << endl;
	cin.get();
	return 0;
}
