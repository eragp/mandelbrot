#include "Client.h"

#include "Fractal.h"
#include "Mandelbrot.h"
#include "RegionOld.h"
#include "Tile.h"

#include <mpi.h>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstring>

void Client::init(int world_rank, int world_size) {
    Fractal *f = new Mandelbrot();
    // Initial test if this core is ready
    int test;
    MPI_Recv(&test, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Send((const void *) &test, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    bool loopFlag = false;
    RegionOld region, newRegion;
    MPI_Request request;
    int flag;
    MPI_Status status;
    // Recieve instructions for computation
    std::cout << "Worker " << world_rank << " is ready to receive Data." << std::endl;
    MPI_Irecv(&newRegion, sizeof(RegionOld), MPI_BYTE, 0, 1, MPI_COMM_WORLD, &request); // Listen for a region asynchronously
    // Start with actual work of this worker
    while (true) {
        MPI_Test(&request, &flag, &status);
        if (flag != 0) {
            // Set current region to newRegion, copy value explicitly (solve more beautiful if you want to)
            std::memcpy(&region, &newRegion, sizeof(RegionOld));
            // Debug Output
            std::cout << "Worker " << world_rank << " received data: TopLeft: (" << region.tlX << ", " << region.tlY
                      << ", "
                      << region.zoom
                      << ") ->  BottomRight: (" << region.brX << ", " << region.brY << ", " << region.zoom
                      << ") MaxIteration: " << region.maxIteration << std::endl;

            // Reset loop flag
            loopFlag = false;

            // Recieve instructions for computation
            std::cout << "Worker " << world_rank << " is listening during computation." << std::endl;
            MPI_Irecv(&newRegion, sizeof(RegionOld), MPI_BYTE, 0, 1, MPI_COMM_WORLD,
                      &request); // Listen for a region asynchronously => store inside newRegion

            // Execute computations
            std::vector<Tile> tiles = region.getTiles();
            int data_len = region.getBytes();
            std::vector<int> data((unsigned long) data_len);
            for (std::vector<Tile>::size_type i = 0; i != tiles.size() && !loopFlag; i++) {
                Tile tile = tiles[i];
                for (unsigned int y = 0; y < (unsigned int) tile.resY && !loopFlag; y++) {
                    for (unsigned int x = 0; x < (unsigned int) tile.resX && !loopFlag; x++) {

                        // Abort
                        MPI_Test(&request, &flag, &status);
                        if (flag != 0) {
                            std::cout << "Worker " << world_rank << " abort." << std::endl;
                            loopFlag = true;
                        }

                        // Computations
                        long double real = Fractal::xToReal(tile.x, tile.zoom, x, tile.resX);
                        long double imag = Fractal::yToImaginary(tile.y, tile.zoom, y, tile.resY);

                        unsigned int block_offset = static_cast<unsigned int>(i * region.resX * region.resY);
                        unsigned int index = y * region.resX + x;
                        data[block_offset + index] = f->calculateFractal(real, imag, tile.maxIteration);
                    }
                }
            }
            if (!loopFlag) {
                std::cout << "Worker " << world_rank << " is sending the data" << std::endl;
                MPI_Send((const void *) &world_rank, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
                MPI_Send((const void *) &data[0], data_len, MPI_INT, 0, 2, MPI_COMM_WORLD);
            }
        }
    }
}
