#pragma once
#ifndef PREDICTIONBALANCER_H    // Prevents multiple includes of this header
#define PREDICTIONBALANCER_H

#include "Balancer.h"
#include "Region.h"
#include "Fractal.h"

#include <vector>
#include <string>

class PredictionBalancer : public Balancer {
private:
    /**
     * predictionAccuracy > 0:
     *  predictionAccuracy^2 pixel samples are used in each guaranteedDivisor^2 Block.
     *
     * predictionAccuracy < 0:
     *  predictionAccuracy^2 Blocks of size guaranteedDivisor^2 are combined in one pixel sample.
     *
     * predictionAccuracy cannot be == 0
     */
    int predictionAccuracy;
    Fractal *f;

    Region *splitCol(Region col, int parts, int nSum, std::vector<int> n, double deltaImaginary);

public:
    static const std::string NAME;

    Region *balanceLoad(Region region, int nodeCount) override;

    ~PredictionBalancer() override;

    static PredictionBalancer *create(Fractal *f, int predictionAccuracy);
};

#endif