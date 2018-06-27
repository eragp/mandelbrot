#ifndef TILEDATA_H    // Prevents multiple includes of this header
#define TILEDATA_H

struct TileData {
    // result of tile computation
    int *n;
    // World-Rank of the used render node
    int world_rank;
    int size;

    TileData(int _world_rank, int _size) {
        size = _size;
        n = new int[_size];
        world_rank = _world_rank;
    }

    TileData() {
        world_rank = -1;
    }
};

#endif // !TILEDATA_H