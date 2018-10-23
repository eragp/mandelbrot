#pragma once

#include "Balancer.h"

#include "Balancer.h"
#include "Region.h"
#include "Fractal.h"

#include <vector>
#include <string>

class RecursivePredictionBalancer : public Balancer {
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

public:
    static const std::string NAME;

    Region *balanceLoad(Region region, int nodeCount) override;

    ~RecursivePredictionBalancer() override;

    static RecursivePredictionBalancer *create(Fractal *f, int predictionAccuracy);
};