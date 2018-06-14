#include "Client.h"

#include "Fractal.h"
#include "Mandelbrot.h"
#include "Region.h"
#include "Tile.h"

#include <mpi.h>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

void Client::init(int world_rank, int world_size) {
    Fractal *f = new Mandelbrot();
    int test;
    // TODO: what is this for? Initial communication? Please explain
    MPI_Recv(&test, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Send((const void *) &test, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    while (true) {
        std::cout << "Worker " << world_rank << " is ready to receive Data." << std::endl;
        Region region;
        MPI_Recv(&region, sizeof(Region), MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Worker " << world_rank << " received data: TopLeft: (" << region.tlX << ", " << region.tlY
                  << ") ->  BottomRight: (" << region.brX << ", " << region.brY
                  << ") MaxIteration: " << region.maxIteration << std::endl;

        //int data[tileInfo.xRes][tileInfo.yRes];
        std::vector<Tile> tiles = region.getTiles();
        int data_len = region.getBytes();
        std::vector<int> data2(data_len);
        for (std::vector<Tile>::size_type i = 0; i != tiles.size(); i++) {
            Tile tile = tiles[i];
            for (int y = 0; y < tile.resY; y++) {
                for (int x = 0; x < tile.resX; x++) {
                    //data[x][y] = f->calculateFractal((tileInfo.minReal + x * deltaReal), (tileInfo.minImaginary + y * deltaImaginary), tileInfo.maxIteration);
                    long double real = Fractal::xToReal(tile.x, tile.zoom, x, tile.resX);
                    long double imag = Fractal::yToImaginary(tile.y, tile.zoom, y, tile.resY);
                    int index = (i + 1) * (y * tile.resX + x);
                    data2[index] = f->calculateFractal(real, imag, tile.maxIteration);
                }
            }
        }

        //std::cout << "Last send Data at (" << (tileInfo.xRes - 1) << ", " << (tileInfo.yRes - 1) << ") is " << data[tileInfo.xRes - 1][tileInfo.yRes - 1] << std::endl;
        std::cout << "Worker " << world_rank << " is sending the data" << std::endl;
        MPI_Send((const void *) &world_rank, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
        //MPI_Send((const void*)&data, sizeof(data), MPI_BYTE, 0, 2, MPI_COMM_WORLD);
        MPI_Send((const void *) &data2[0], data_len, MPI_INT, 0, 2, MPI_COMM_WORLD);
    }
}
