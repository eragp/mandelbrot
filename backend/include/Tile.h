#pragma once
typedef struct {
    int x, y, zoom, size;
    // Size <= 0 specifies an empty tile
    int start_x;  //included
    int start_y;  //included
    int end_x;    //excluded
    int end_y;    //excluded
    int maxIteration;
    double minReal;
    double maxReal;
    double minImaginary;
    double maxImaginary;
} Tile;
