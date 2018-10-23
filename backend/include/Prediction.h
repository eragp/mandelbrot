#pragma once

#include <vector>

struct Prediction {
    std::vector<std::vector<int>> n;
    std::vector<int> nColSums;
    std::vector<int> nRowSums;
    int nSum;

    int predictionLengthX;
    int predictionLengthY;

    double deltaReal;
    double deltaImaginary;
};