#pragma once
class Fractal {
   public:
    virtual int calculateFractal(long double cReal, long double cImaginary, int maxIteration) = 0;
    virtual ~Fractal();

    static double deltaReal(double maxReal, double minReal, int xRes);
    static double deltaImaginary(double maxImaginary, double minImaginary, int yRes);
};
