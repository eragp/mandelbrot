#include "Client.h"

#include <iostream>
#include "Fractal.h"
#include "Mandelbrot.h"
#include "Returned.h"
#include "Tile.h"

#include <mpi.h>

#include <map>
#include <set>
#include <string>

using namespace std;

double unproject(long tile, int zoom, long local, int size, int bound) {
    if (size == 0) size = 1;
    long tileCount = 1 << zoom;
    return (double)(tile * bound * size + local * bound) / (double)(tileCount * size);
}

double unprojectX(long tileX, int zoom, long localX, int size) {
    const int boundReal = 2;
    return unproject(tileX, zoom, localX, size, boundReal);
}

double unprojectY(long tileY, int zoom, long localY, int size) {
    const int boundImag = 2;
    return unproject(tileY, zoom, localY, size, boundImag);
}

void Client::init(int world_rank, int world_size) {
    Fractal* f = new Mandelbrot();
    while (true) {
        Tile tile;
        MPI_Recv(&tile, sizeof(Tile), MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << "Client received data for "
             << "(" << tile.x << "," << tile.y << "," << tile.zoom << ")"
             << endl;
        if (tile.start_x != -1) {
            Returned returned;
            for (int x = 0; x < tile.size; x++) {
                for (int y = 0; y < tile.size; y++) {
                    returned.n[x][y] = f->calculateFractal(
                        unprojectX(tile.x, tile.zoom, x, tile.size),
                        unprojectY(tile.y, tile.zoom, y, tile.size),
                        tile.maxIteration);
                }
            }
            returned.x = tile.x;
            returned.y = tile.y;
            returned.zoom = tile.zoom;
            returned.size = tile.size;
            returned.world_rank = world_rank;
            cout << world_rank << " is sending the data" << endl;
            MPI_Send((const void*)&returned, sizeof(Returned), MPI_BYTE, 0, 2, MPI_COMM_WORLD);
        }
    }
}
