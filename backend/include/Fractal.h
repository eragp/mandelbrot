#pragma once
class Fractal {
   public:
    /**
     * Calculates the fractal iteration value for a given complex (real, imaginary) pair
     * @param cReal real coordinate
     * @param cImaginary imaginary coordinate
     * @param maxIteration maximum amount of iterations to perform
     * @param length of the above arrays
     * @return number of iterations for the given coordinates
     */
    virtual int* calculateFractal(long double* cReal, long double* cImaginary, int maxIteration, int vector_size) = 0;
    virtual ~Fractal();

    /**
     * Calculates the step size of a single pixel in the x direction (real) on the complex plane.
     * @param maxReal max real bound
     * @param minReal min real bound
     * @param xRes pixel resolution from minReal -> maxReal
     * @return step size on real axis
     */
    static double deltaReal(double maxReal, double minReal, int xRes);

    /**
     * Calculates the step size of a single pixel in the y direction (imaginary) on the complex plane.
     * @param maxImaginary max imaginary bound
     * @param minImaginary min imaginary bound
     * @param yRes pixel resolution from minImaginary -> maxImaginary
     * @return step size on imaginary axis
     */
    static double deltaImaginary(double maxImaginary, double minImaginary, int yRes);
};
