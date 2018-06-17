#ifndef REGION_H  // Prevents multiple includes of this header
#define REGION_H

#include <cmath>
#include <iosfwd>
#include <ostream>
#include <string>
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
    // Resolution of the individual tiles in this region
    int resX;  // width
    int resY;  // height

    // Iterations, the calculateFractal class should do.
    int maxIteration;

    int getWidth() {
        return abs(tlX - brX) + 1;
    }

    int getHeight() {
        return abs(tlY - brY) + 1;
    }

    unsigned int getBytes() {
        return (unsigned int) getWidth() * resX * getHeight() * resY;
    }

    /**
     * Returns true if given region is completely inside this region
     */
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

    /**
     * Returns a vector of tiles that completely comprises this region
     */
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

//    std::ostream &operator<<(std::ostream os, const Region &r) {
//        os << "Region: ";
//        return os;
//    }
};


#endif  // !REGION_H
