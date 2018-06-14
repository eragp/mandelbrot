#ifndef REGION_H  // Prevents multiple includes of this header
#define REGION_H

#include <cmath>
#include <vector>
#include "Tile.h"

struct Region {
    // tile Position
    // top left indices
    int tlX, tlY;
    // bottom right indices
    int brX, brY;
    // current zoom factor
    int zoom;
    // Resolution
    int resX;  // width
    int resY;  // height

    // Iterations, the calculateFractal class should do.
    int maxIteration;

    int getWidth() {
        return abs(tlX - brX);
    }

    int getHeight() {
        return abs(tlY - brY);
    }

    unsigned int getBytes() {
        return (unsigned int) getWidth() * resX * getHeight() * resY;
    }

    bool contains(int _tlX, int _tlY, int _brX, int _brY, int _zoom) {
        if (zoom != _zoom) {
            return false;
        }
        bool tl = false, br = false;
        for (int i = tlX; i <= brX; i++) {
            for (int j = tlY; j >= brY; j--) {
                if (_tlX == i && _tlY == j) {
                    tl = true;
                }
                if (_brX == i && _brY == j) {
                    br = true;
                }
            }
        }
        return tl && br;
    }

    std::vector<Tile> getTiles() {
        std::vector<Tile> tiles;
        for (int y = tlY; y >= brY; y--) {
            for (int x = tlX; x <= brX; x++) {
                Tile tile;
                tile.x = x;
                tile.y = y;
                tile.zoom = zoom;
                tile.maxIteration = maxIteration;
                tile.resX = resX;
                tile.resY = resY;
                tiles.push_back(tile);
            }
        }
        return tiles;
    }


    // definition of operator< is required to use this struct as a
    // std::map key
    bool const operator<(const Region &o) const {
        return tlX < o.tlX
               || (tlX == o.tlX && tlY < o.tlY)
               || (tlX == o.tlX && tlY == o.tlY && brX < o.brX)
               || (tlX == o.tlX && tlY == o.tlY && brX == o.brX && brY < o.brY);
    }

    bool const operator==(const Region &o) const {
        return tlX == o.tlX && tlY == o.tlY
               && brX == o.brX && brY == o.brY
               && zoom == o.zoom
               && resX == o.resX && resY == o.resY
               && maxIteration == o.maxIteration;
    }
};

#endif  // !REGION_H
