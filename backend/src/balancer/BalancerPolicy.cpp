#include "BalancerPolicy.h"
#include "Balancer.h"
#include "NaiveBalancer.h"
#include "ColumnBalancer.h"
#include "PredictionBalancer.h"

#include <string>

Balancer* BalancerPolicy::chooseBalancer(std::string balancerName) {
    return new NaiveBalancer();
}