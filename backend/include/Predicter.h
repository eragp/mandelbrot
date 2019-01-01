#pragma once

#include "Region.h"
#include "Fractal.h"
#include "Prediction.h"

class Predicter {
    public:
        static Prediction* getPrediction(Region region, Fractal* fractal, int predictionAccuracy);
};