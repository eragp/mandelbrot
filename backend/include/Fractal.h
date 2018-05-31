#pragma once
class Fractal {
   public:
    virtual int calculateFractal(double cReal, double cImaginary, int maxIteration) = 0;

    //TODO is this still necessary?
    static double xToReal(int x, double maxReal, double minReal, int width);
	static double yToImaginary(int y, double maxImaginary, double minImaginary, int height);
	static int realToX(double real, double maxReal, double minReal, int width);
	static int imaginaryToY(double imaginary, double maxImaginary, double minImaginary, int height);

};
