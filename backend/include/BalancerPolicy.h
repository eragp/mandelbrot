#pragma once
#include "Balancer.h"
#include "Fractal.h"

#include <string>

// For the sake of ordering methods
class BalancerPolicy {
    public:
        static Balancer* chooseBalancer(std::string balancerName, Fractal* fractal);
};