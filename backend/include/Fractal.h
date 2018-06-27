#pragma once
class Fractal {
   public:
    virtual int calculateFractal(long double cReal, long double cImaginary, int maxIteration) = 0;

    //TODO is this still necessary? -> Yes, except xToReal und yToReal in the future. (@Florian uses them in a Balancer; He will change that)
    // static double deltaReal(double maxReal, double minReal, int xRes);
    // static double deltaImaginary(double maxImaginary, double minImaginary, int yRes);
    static long double xToReal(long x, long zoom, long localX, long size);
    static long double yToImaginary(long y, long zoom, long localY, long size);
    // static int realToX(double real, double maxReal, double minReal, int width);
    // static int imaginaryToY(double imaginary, double maxImaginary, double minImaginary, int height);
};
