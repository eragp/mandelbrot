#pragma once

// Define the precision for computations globally
typedef long double precision_t;

enum fractal_type {
    mandelbrot,
    mandelbrot32,
    mandelbrot64,
    mandelbrotSIMD32,
    mandelbrotSIMD64,
};

class Fractal {
   public:
    /**
     * Calculates the fractal iteration value for a given complex (real, imaginary) pair
     * @param cReal real coordinate
     * @param cImaginary imaginary coordinate
     * @param maxIteration maximum amount of iterations to perform
     * @param vectorLength number of coordinates to be calculated at once
     * @param dest memory position where result(number of iterations for coordinates) should be stored
     */
    virtual void calculateFractal(precision_t* cReal, precision_t* cImaginary, unsigned short int maxIteration, int vectorLength, unsigned short int* dest) = 0;
    virtual ~Fractal();

    /**
     * Calculates the step size of a single pixel in the x direction (real) on the complex plane.
     * @param maxReal max real bound
     * @param minReal min real bound
     * @param width pixel resolution from minReal -> maxReal
     * @return step size on real axis
     */
    static precision_t deltaReal(precision_t maxReal, precision_t minReal, int width);

    /**
     * Calculates the step size of a single pixel in the y direction (imaginary) on the complex plane.
     * @param maxImaginary max imaginary bound
     * @param minImaginary min imaginary bound
     * @param height pixel resolution from minImaginary -> maxImaginary
     * @return step size on imaginary axis
     */
    static precision_t deltaImaginary(precision_t maxImaginary, precision_t minImaginary, int height);
};
