#pragma once
typedef struct {
    int x, y, zoom, size;
    // Size <= 0 specifies an empty tile
    int maxIteration;
} Tile;
