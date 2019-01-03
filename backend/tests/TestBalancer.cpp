#include <iostream>

#include "Region.h"
#include "Fractal.h"
#include "Mandelbrot.h"

#include "RecursivePredictionBalancer.h"
#include "PredictionBalancer.h"
#include "NaiveBalancer.h"
#include "RecursiveNaiveBalancer.h"
#include "ColumnBalancer.h"
#include "Balancer.h"

int nodes = 6;

void printRegion(Region region) {
	std::cout << "Debug for Region: (MinReal -> " << region.minReal << ") ; (MaxImaginary -> "
		<< region.maxImaginary << ") ; (MaxReal -> " << region.maxReal << ") ; (MinImaginary -> "
		<< region.minImaginary << ") ; (Width -> " << region.width << ") ; (Height -> " << region.height <<
		") ;" << std::endl << "(hOffset -> " << region.hOffset << ") ; (vOffset -> " << region.vOffset <<
		") ; (MaxIteration -> " << region.maxIteration << ") ; (Validation -> " << region.validation <<
		") ; (GuaranteedDivisor -> " << region.guaranteedDivisor << ") ; (Fractal -> " << region.fractal << ")" << std::endl;
}

bool testInvariants(Region fullRegion, Region part) {
	bool divisorEqual = fullRegion.guaranteedDivisor == part.guaranteedDivisor;
	if (!divisorEqual) {
		std::cout << "guaranteedDivisor: expected -> " << fullRegion.guaranteedDivisor << " observed -> " << part.guaranteedDivisor << std::endl;
	}

	bool fractalEqual = fullRegion.fractal == part.fractal;
	if (!fractalEqual) {
		std::cout << "fractal: expected -> " << fullRegion.fractal << " observed -> " << part.fractal << std::endl;
	}

	bool validationEqual = fullRegion.validation == part.validation;
	if (!validationEqual) {
		std::cout << "validation: expected -> " << fullRegion.validation << " observed -> " << part.validation << std::endl;
	}

	bool maxIterEqual = fullRegion.maxIteration == part.maxIteration;
	if (!maxIterEqual) {
		std::cout << "maxIteration: expected -> " << fullRegion.maxIteration << " observed -> " << part.maxIteration << std::endl;
	}

	return divisorEqual && fractalEqual && validationEqual && maxIterEqual;
}

bool testDivisor(Region fullRegion, Region part) {
	bool dividesWidth = part.width % part.guaranteedDivisor == 0;
	if (!dividesWidth) {
		std::cout << "guaranteedDivisor doesn't divide width: expected -> 0 observed -> " << part.width % part.guaranteedDivisor << std::endl;
	}

	bool dividesHeight = part.height % part.guaranteedDivisor == 0;
	if (!dividesHeight) {
		std::cout << "guaranteedDivisor doesn't divide height: expected -> 0 observed -> " << part.height % part.guaranteedDivisor << std::endl;
	}

	return dividesWidth && dividesHeight;
}

bool testRatio(Region fullRegion, Region part) {
	// We need to define a range for the values, since doubles are never exact
	double epsilon = 0.000000001; // Checks if the values vary in the 8 visible digits

	bool xInRange = true;
	bool yInRange = true;

	if (part.width != 0) {
		double expectedXRatio = (fullRegion.maxReal - fullRegion.minReal) / fullRegion.width;
		double observedXRatio = (part.maxReal - part.minReal) / part.width;
		xInRange = (expectedXRatio - epsilon <= observedXRatio) && (observedXRatio <= expectedXRatio + epsilon);

		if (!xInRange) {
			std::cout << "xRatio: expected -> " << expectedXRatio << " observed -> " << observedXRatio << std::endl;
		}

	}

	if (part.height != 0) {
		double expectedYRatio = (fullRegion.maxImaginary - fullRegion.minImaginary) / fullRegion.height;
		double observedYRatio = (part.maxImaginary - part.minImaginary) / part.height;
		yInRange = (expectedYRatio - epsilon <= observedYRatio) && (observedYRatio <= expectedYRatio + epsilon);

		if (!yInRange) {
			std::cout << "yRatio: expected -> " << expectedYRatio << " observed -> " << observedYRatio << std::endl;
		}
	}

	return xInRange && yInRange;
}

bool testOffset(Region fullRegion, Region part) {
	double deltaReal = Fractal::deltaReal(fullRegion.maxReal, fullRegion.minReal, fullRegion.width);
	double deltaImaginary = Fractal::deltaImaginary(fullRegion.maxImaginary, fullRegion.minImaginary, fullRegion.height);

	// doubles are never exact --> round
	int expectedHOffset = (int)round((part.minReal - fullRegion.minReal) / deltaReal);
	if (part.hOffset != expectedHOffset) {
		std::cout << "hOffset: expected -> " << expectedHOffset << " observed -> " << part.hOffset << std::endl;
	}

	int expectedVOffset = (int)round((fullRegion.maxImaginary - part.maxImaginary) / deltaImaginary);
	if (part.vOffset != expectedVOffset) {
		std::cout << "vOffset: expected -> " << expectedVOffset << " observed -> " << part.vOffset << std::endl;
	}

	return part.hOffset == expectedHOffset && part.vOffset == expectedVOffset;
}

bool testCoverage(Region region, Region* subregions) {
	bool disjunct = true;
	bool entirelyCovered = true;

	int widthTile = region.width / region.guaranteedDivisor;
	int heightTile = region.height / region.guaranteedDivisor;

	std::vector<std::vector<bool>> covered(heightTile, std::vector<bool>(widthTile));

	for (int i = 0; i < nodes; i++) {
		int partWidthTile = subregions[i].width / subregions[i].guaranteedDivisor;
		int partHOffsetTile = subregions[i].hOffset / subregions[i].guaranteedDivisor;

		int partHeightTile = subregions[i].height / subregions[i].guaranteedDivisor;
		int partVOffsetTile = subregions[i].vOffset / subregions[i].guaranteedDivisor;

		for (int y = partVOffsetTile; y < partHeightTile + partVOffsetTile; y++) {
			for (int x = partHOffsetTile; x < partWidthTile + partHOffsetTile; x++) {
				if (covered[y][x]) {
					disjunct = false;
				}

				covered[y][x] = true;
			}
		}
	}

	if (!disjunct) {
		std::cout << "subregions are not disjunct" << std::endl;
	}

	for (int y = 0; y < heightTile; y++) {
		for (int x = 0; x < widthTile; x++) {
			if (!covered[y][x]) {
				std::cout << "(" << x << "|" << y << ") not covered" << std::endl;
				entirelyCovered = false;
			}
		}
	}

	return disjunct && entirelyCovered;
}

void testBalancerOutput(Region fullRegion, Region* output) {
	bool failed = false;
	for (int i = 0; i < nodes; i++) {
		if (!testInvariants(fullRegion, output[i])) {
			std::cerr << "output[" << i << "] failed invariants test." << std::endl;
			failed = true;
		}

		if (!testDivisor(fullRegion, output[i])) {
			std::cerr << "output[" << i << "] failed divisor test." << std::endl;
			failed = true;
		}

		if (!testRatio(fullRegion, output[i])) {
			std::cerr << "output[" << i << "] failed ratio test." << std::endl;
			failed = true;
		}

		if (!testOffset(fullRegion, output[i])) {
			std::cerr << "output[" << i << "] failed offset test." << std::endl;
			failed = true;
		}
	}

	if (!testCoverage(fullRegion, output)) {
		std::cerr << "output failed coverage test" << std::endl;
		failed = true;
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
	test.fractal = mandelbrot;

	std::cout << "Region: " << std::endl;
	printRegion(test);
	std::cout << std::endl;

	Balancer* column = new ColumnBalancer();
	Balancer* naive = new NaiveBalancer();
	Balancer* naiveRec = new RecursiveNaiveBalancer();
	Balancer* prediction = PredictionBalancer::create(new Mandelbrot(), 4);
	Balancer* predictionNeg = PredictionBalancer::create(new Mandelbrot(), -4);
	Balancer* predictionRec = RecursivePredictionBalancer::create(new Mandelbrot(), 4);

	std::cout << "Column: " << std::endl;
	testBalancer(column, test, nodes);

	std::cout << "Naive: " << std::endl;
	testBalancer(naive, test, nodes);

	std::cout << "NaiveRec: " << std::endl;
	testBalancer(naiveRec, test, nodes);

	std::cout << "Prediction: " << std::endl;
	testBalancer(prediction, test, nodes);

	std::cout << "PredictionNegative: " << std::endl;
	testBalancer(predictionNeg, test, nodes);

	std::cout << "PredictionRec: " << std::endl;
	testBalancer(predictionRec, test, nodes);

	std::cout << "Tests concluded!" << std::endl;
	delete column;
	delete naive;
	delete naiveRec;
	delete prediction;
	delete predictionNeg;
	delete predictionRec;

	// std::cin.get();
	return 0;
}