#include "BalancerPolicy.h"
#include "Balancer.h"
#include "NaiveBalancer.h"
#include "RecursiveNaiveBalancer.h"
#include "ColumnBalancer.h"
#include "PredictionBalancer.h"
#include "RecursivePredictionBalancer.h"
#include "Fractal.h"

#include <string>
#include <iostream>

Balancer* BalancerPolicy::chooseBalancer(std::string balancerName, int predictionAccuracy, Fractal* fractal) {
    // C++ doesn't like string in switch :(
    if (balancerName == NaiveBalancer::NAME) {
        std::cout << "Chose naive balancer" << std::endl;
        // Fractal is not needed
        delete fractal;
        return new NaiveBalancer();
    } else if (balancerName == RecursiveNaiveBalancer::NAME) {
        std::cout << "Chose recursive naive balancer" << std::endl;
        // Fractal is not needed
        delete fractal;
        return new RecursiveNaiveBalancer();
    } else if (balancerName == ColumnBalancer::NAME) {
        std::cout << "Chose column balancer" << std::endl;
        // Fractal is not needed
        delete fractal;
        return new ColumnBalancer();
    } else if (balancerName == PredictionBalancer::NAME) {
        std::cout << "Chose prediction balancer" << std::endl;
        return PredictionBalancer::create(fractal, predictionAccuracy);
    } else if (balancerName == RecursivePredictionBalancer::NAME) {
        std::cout << "Chose recursive prediction balancer" << std::endl;
        return RecursivePredictionBalancer::create(fractal, predictionAccuracy);
    } else {
        std::cerr << "Unknown balancer name, returning NaiveBalancer" << std::endl;
        // Fractal is not needed
        delete fractal;
        return new NaiveBalancer();
    }
}