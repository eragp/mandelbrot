#include <iostream>

#include "Region.h"
#include "Mandelbrot.h"

#include "PredictionBalancer.h"
#include "NaiveBalancer.h"
#include "ColumnBalancer.h"
#include "Balancer.h"

int nodes = 6;

void printRegion(Region region) {
	std::cout << "Debug for Region: (MinReal -> " << region.minReal << ") ; (MaxImaginary -> "
		<< region.maxImaginary << ") ; (MaxReal -> " << region.maxReal << ") ; (MinImaginary -> "
		<< region.minImaginary << ") ; (Width -> " << region.width << ") ; (Height -> " << region.height <<
		") ;" << std::endl << "(hOffset -> " << region.hOffset << ") ; (vOffset -> " << region.vOffset <<
		") ; (MaxIteration -> " << region.maxIteration << ") ; (Validation -> " << region.validation <<
		") ; (GuaranteedDivisor -> " << region.guaranteedDivisor << ")" << std::endl;
}

bool testRatio(Region fullRegion, Region part) {
	// We need to define a range for the values, since doubles are never exact
	double epsilon = 0.000000001; // Checks if the values vary in the 8 visible digits

	double expectedXRatio = (fullRegion.maxReal - fullRegion.minReal) / fullRegion.width;
	double expectedYRatio = (fullRegion.maxImaginary - fullRegion.minImaginary) / fullRegion.height;

	double observedXRatio = (part.maxReal - part.minReal) / part.width;
	double observedYRatio = (part.maxImaginary - part.minImaginary) / part.height;

	bool xInRange = (expectedXRatio - epsilon <= observedXRatio) && (observedXRatio <= expectedXRatio + epsilon);
	bool yInRange = (expectedYRatio - epsilon <= observedYRatio) && (observedYRatio <= expectedYRatio + epsilon);

	if (!xInRange) {
		std::cout << "xRatio: expected -> " << expectedXRatio << " observed -> " << observedXRatio << std::endl;
	}
	if (!yInRange) {
		std::cout << "yRatio: expected -> " << expectedYRatio << " observed -> " << observedYRatio << std::endl;
	}
	return xInRange && yInRange;
}

void testBalancerOutput(Region fullRegion, Region* output) {
	bool failed = false;
	for (int i = 0; i < nodes; i++) {
		if (!testRatio(fullRegion, output[i])) {
			std::cerr << "output[" << i << "] failed ratio test." << std::endl;
			failed = true;
		}
	}
	if (!failed) {
		std::cout << "Output test succesful!" << std::endl;
	}
}

void testBalancer(Balancer* b, Region r, int nodeCount) {
	Region* balance = b->balanceLoad(r, nodeCount);
	for (int i = 0; i < nodeCount; i++) {
		printRegion(balance[i]);
	}
	std::cout << std::endl;

	testBalancerOutput(r, balance);
	std::cout << std::endl;

	delete[] balance;
}

int main(int argc, char** argv) {
	Region test;
	test.minReal = -1.5;
	test.maxImaginary = 1.0;

	test.maxReal = 0.7;
	test.minImaginary = -1.0;

	test.width = 2048;
	test.height = 2048;

	test.vOffset = 0;
	test.hOffset = 0;

	test.maxIteration = 200;
	test.validation = 1;
	test.guaranteedDivisor = 64;

	std::cout << "Region: " << std::endl;
	printRegion(test);
	std::cout << std::endl;

	Balancer* column = new ColumnBalancer();
	Balancer* naive = new NaiveBalancer();
	Balancer* prediction = PredictionBalancer::create(new Mandelbrot(), 16);

	std::cout << "Column: " << std::endl;
	testBalancer(column, test, nodes);

	std::cout << "Naive: " << std::endl;
	testBalancer(naive, test, nodes);

	std::cout << "Prediction: " << std::endl;
	testBalancer(prediction, test, nodes);

	delete column;
	delete naive;
	delete prediction;

	std::cin.get();
	return 0;
}