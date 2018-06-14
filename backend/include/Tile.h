#ifndef TILE_H
#define TILE_H

#include <cmath>

typedef struct {
    int x, y, zoom;
    // Size <= 0 specifies an empty tile
    int resX, resY;
    int maxIteration;
} Tile;
#endif