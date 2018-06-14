#ifndef TILE_H
#define TILE_H

#include <cmath>

struct Tile {
    int x, y, zoom;
    // Size <= 0 specifies an empty tile
    int resX, resY;
    int maxIteration;

    // definition of < operator is required to use this object as a
    // std::map key
    bool operator<(const Tile &o) const {
        return x < o.x
               || (x == o.x && y < o.y)
               || (x == o.x && y == o.y && zoom < o.zoom);
    }
};

#endif