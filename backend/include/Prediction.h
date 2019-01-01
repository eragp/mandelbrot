#pragma once

#include <vector>

struct Prediction {
    std::vector<std::vector<int>> n; // Access with n[x][y], length [predictionLengthX][predictionLengthY]
    std::vector<int> nColSums; // length [predictionLengthX]
    std::vector<int> nRowSums; // length [predictionLengthY]
    int nSum;

    int predictionLengthX;
    int predictionLengthY;

    double deltaReal;
    double deltaImaginary;
};