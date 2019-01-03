#include <iostream>
#include <cmath>
#include <cstring>

#include "Region.h"
#include "Fractal.h"
#include "Mandelbrot.h"

#include "RecursivePredictionBalancer.h"
#include "PredictionBalancer.h"
#include "NaiveBalancer.h"
#include "RecursiveNaiveBalancer.h"
#include "ColumnBalancer.h"
#include "Balancer.h"

struct TestCase {
	std::string testName;
	int nodeCount;
	Balancer* balancer;
	Region* region;
};

bool printSubregions = false;

void printRegion(Region region) {
	std::cout << "(minReal -> " << region.minReal << "); (maxImaginary -> " << region.maxImaginary << "); "
		<< "(maxReal -> " << region.maxReal << "); (minImaginary -> " << region.minImaginary << ")" << std::endl
		<< "(width -> " << region.width << "); (height -> " << region.height << "); "
		<< "(hOffset -> " << region.hOffset << "); (vOffset -> " << region.vOffset << ")" << std::endl
		<< "(guaranteedDivisor -> " << region.guaranteedDivisor << "); (validation -> " << region.validation << "); "
		<< "(maxIteration -> " << region.maxIteration << "); (fractal -> " << region.fractal << ")" << std::endl;
}

bool isEmptyRegion(Region region) {
	return region.width == 0 &&
		region.height == 0 &&
		region.minImaginary == 0.0 &&
		region.maxImaginary == 0.0 &&
		region.minReal == 0.0 &&
		region.maxReal == 0.0 &&
		region.hOffset == 0 &&
		region.vOffset == 0;

}

bool testInvariants(Region region, Region subregion) {
	bool divisorEqual = region.guaranteedDivisor == subregion.guaranteedDivisor;
	if (!divisorEqual) {
		std::cout << "guaranteedDivisor: expected -> " << region.guaranteedDivisor << " observed -> " << subregion.guaranteedDivisor << std::endl;
	}

	bool fractalEqual = region.fractal == subregion.fractal;
	if (!fractalEqual) {
		std::cout << "fractal: expected -> " << region.fractal << " observed -> " << subregion.fractal << std::endl;
	}

	bool validationEqual = region.validation == subregion.validation;
	if (!validationEqual) {
		std::cout << "validation: expected -> " << region.validation << " observed -> " << subregion.validation << std::endl;
	}

	bool maxIterEqual = region.maxIteration == subregion.maxIteration;
	if (!maxIterEqual) {
		std::cout << "maxIteration: expected -> " << region.maxIteration << " observed -> " << subregion.maxIteration << std::endl;
	}

	return divisorEqual && fractalEqual && validationEqual && maxIterEqual;
}

bool testDivisor(Region subregion) {
	bool dividesWidth = subregion.width % subregion.guaranteedDivisor == 0;
	if (!dividesWidth) {
		std::cout << "guaranteedDivisor doesn't divide width: expected -> 0 observed -> " << subregion.width % subregion.guaranteedDivisor << std::endl;
	}

	bool dividesHeight = subregion.height % subregion.guaranteedDivisor == 0;
	if (!dividesHeight) {
		std::cout << "guaranteedDivisor doesn't divide height: expected -> 0 observed -> " << subregion.height % subregion.guaranteedDivisor << std::endl;
	}

	return dividesWidth && dividesHeight;
}

bool testRatio(Region region, Region subregion) {
	// We need to define a range for the values, since doubles are never exact
	double epsilon = 0.000000001; // Checks if the values vary in the 8 visible digits

	bool xInRange = true;
	bool yInRange = true;

	if (subregion.width != 0) {
		double expectedXRatio = (region.maxReal - region.minReal) / region.width;
		double observedXRatio = (subregion.maxReal - subregion.minReal) / subregion.width;
		xInRange = (expectedXRatio - epsilon <= observedXRatio) && (observedXRatio <= expectedXRatio + epsilon);

		if (!xInRange) {
			std::cout << "xRatio: expected -> " << expectedXRatio << " observed -> " << observedXRatio << std::endl;
		}

	}

	if (subregion.height != 0) {
		double expectedYRatio = (region.maxImaginary - region.minImaginary) / region.height;
		double observedYRatio = (subregion.maxImaginary - subregion.minImaginary) / subregion.height;
		yInRange = (expectedYRatio - epsilon <= observedYRatio) && (observedYRatio <= expectedYRatio + epsilon);

		if (!yInRange) {
			std::cout << "yRatio: expected -> " << expectedYRatio << " observed -> " << observedYRatio << std::endl;
		}
	}

	return xInRange && yInRange;
}

bool testOffset(Region region, Region subregion) {
	double deltaReal = Fractal::deltaReal(region.maxReal, region.minReal, region.width);
	double deltaImaginary = Fractal::deltaImaginary(region.maxImaginary, region.minImaginary, region.height);

	// doubles are never exact --> round
	int expectedHOffset = (int)round((subregion.minReal - region.minReal) / deltaReal);
	if (subregion.hOffset != expectedHOffset) {
		std::cout << "hOffset: expected -> " << expectedHOffset << " observed -> " << subregion.hOffset << std::endl;
	}

	int expectedVOffset = (int)round((region.maxImaginary - subregion.maxImaginary) / deltaImaginary);
	if (subregion.vOffset != expectedVOffset) {
		std::cout << "vOffset: expected -> " << expectedVOffset << " observed -> " << subregion.vOffset << std::endl;
	}

	return subregion.hOffset == expectedHOffset && subregion.vOffset == expectedVOffset;
}

bool testCoverage(TestCase test, Region* subregions) {
	bool disjunct = true;
	bool entirelyCovered = true;

	int widthTile = test.region->width / test.region->guaranteedDivisor;
	int heightTile = test.region->height / test.region->guaranteedDivisor;

	std::vector<std::vector<bool>> covered(heightTile, std::vector<bool>(widthTile));

	for (int i = 0; i < test.nodeCount; i++) {
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

bool testBalancerOutput(TestCase test, Region* subregions) {
	bool failed = false;
	for (int i = 0; i < test.nodeCount; i++) {
		if (isEmptyRegion(subregions[i])) {
			// Empty regions are always correct
			continue;
		}
		
		if (!testInvariants(*test.region, subregions[i])) {
			std::cerr << "subregions[" << i << "] failed invariants test." << std::endl;
			failed = true;
		}

		if (!testDivisor(subregions[i])) {
			std::cerr << "subregions[" << i << "] failed divisor test." << std::endl;
			failed = true;
		}

		if (!testRatio(*test.region, subregions[i])) {
			std::cerr << "subregions[" << i << "] failed ratio test." << std::endl;
			failed = true;
		}

		if (!testOffset(*test.region, subregions[i])) {
			std::cerr << "subregions[" << i << "] failed offset test." << std::endl;
			failed = true;
		}
	}

	if (!testCoverage(test, subregions)) {
		std::cerr << "subregions failed coverage test" << std::endl;
		failed = true;
	}

	if (!failed) {
		std::cout << "Output test succesful!" << std::endl;
	}

	return !failed;
}

bool testBalancer(TestCase test) {
	std::cout << test.testName << ":" << std::endl;

	std::cout << "Region:" << std::endl;
	printRegion(*test.region);
	std::cout << std::endl;

	Region* balance = test.balancer->balanceLoad(*test.region, test.nodeCount);
	if (printSubregions) {
		std::cout << "Subregions:" << std::endl;
		for (int i = 0; i < test.nodeCount; i++) {
			std::cout << "subregions[" << i << "]: ";
			printRegion(balance[i]);
			std::cout << std::endl;
		}
	}

	std::cout << "Test result:" << std::endl;
	bool passed = testBalancerOutput(test, balance);
	std::cout << std::endl;

	delete[] balance;

	return passed;
}

int main(int argc, char** argv) {
	if (argc == 2) {
		if (strcmp(argv[1], "-v") == 0) {
			printSubregions = true;
		}
	}

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

	Balancer* column = new ColumnBalancer();
	Balancer* naive = new NaiveBalancer();
	Balancer* naiveRec = new RecursiveNaiveBalancer();
	Balancer* prediction = PredictionBalancer::create(new Mandelbrot(), 4);
	Balancer* predictionNeg = PredictionBalancer::create(new Mandelbrot(), -4);
	Balancer* predictionRec = RecursivePredictionBalancer::create(new Mandelbrot(), 4);

	int nodeCount = 6;

	const int testCount = 6;

	TestCase testCases[testCount];
	testCases[0] = { "Column", nodeCount, column, &test };
	testCases[1] = { "Naive", nodeCount, naive, &test };
	testCases[2] = { "NaiveRec", nodeCount, naiveRec, &test };
	testCases[3] = { "Prediction", nodeCount, prediction, &test };
	testCases[4] = { "PredictionNeg", nodeCount, predictionNeg, &test };
	testCases[5] = { "PredictionRec", nodeCount, predictionRec, &test };

	int failed = 0;
	for (int i = 0; i < testCount; i++) {
		if (!testBalancer(testCases[i])) {
			failed++;
		}
	}

	std::cout << "Tests concluded!" << std::endl;
	if (failed == 0) {
		std::cout << "All test cases passed!" << std::endl;
	} else {
		std::cout << failed << " test cases failed!" << std::endl;
	}

	delete column;
	delete naive;
	delete naiveRec;
	delete prediction;
	delete predictionNeg;
	delete predictionRec;

	// std::cin.get();
	return 0;
}