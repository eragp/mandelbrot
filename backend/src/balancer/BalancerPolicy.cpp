#include "BalancerPolicy.h"
#include "Balancer.h"
#include "NaiveBalancer.h"
#include "ColumnBalancer.h"
#include "PredictionBalancer.h"
#include "Fractal.h"

#include <string>
#include <iostream>

// TODO: Maybe make this method non-static and store fractal and predictionAccuracy in object
Balancer* BalancerPolicy::chooseBalancer(std::string balancerName, Fractal* fractal) {
    int predictionAccuracy = 16;

    // C++ doesn't like string in switch :(
    if (balancerName == NaiveBalancer::NAME) {
        std::cout << "Chose naive balancer" << std::endl;
        return new NaiveBalancer();
    } else if (balancerName == ColumnBalancer::NAME) {
        std::cout << "Chose column balancer" << std::endl;
        return new ColumnBalancer();
    } else if (balancerName == PredictionBalancer::NAME) {
        std::cout << "Chose prediction balancer" << std::endl;
        return PredictionBalancer::create(fractal, predictionAccuracy);
    } else {
        std::cerr << "Unknown balancer name, returning NaiveBalancer" << std::endl;
        return new NaiveBalancer();
    }
}