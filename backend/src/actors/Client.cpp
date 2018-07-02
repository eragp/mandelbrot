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
#include <cstring>

void Client::init(int world_rank, int world_size) {
    Fractal *f = new Mandelbrot();
    // Initial test if this core is ready
    int test;
    MPI_Recv(&test, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Send((void *) &test, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    bool loopFlag = false;
    Region region, newRegion;
    MPI_Request request;
    int flag;
    MPI_Status status;
    // Recieve instructions for computation
    std::cout << "Worker " << world_rank << " is ready to receive Data." << std::endl;
    MPI_Irecv(&newRegion, sizeof(Region), MPI_BYTE, 0, 1, MPI_COMM_WORLD, &request); // Listen for a region asynchronously
    // Start with actual work of this worker
    while (true) {
        MPI_Test(&request, &flag, &status);
        if (flag != 0) {
//            // Set current region to newRegion, copy value explicitly (solve more beautiful if you want to)
            std::memcpy(&region, &newRegion, sizeof(Region));
            // Debug Output
            std::cout << "Worker " << world_rank
                      << " received data: "
                      << "topLeft: (" << region.minReal << ", " << region.maxImaginary << ", " << region.validation
                      << ") ->  bottomRight: (" << region.maxReal << ", " << region.minImaginary << ", "
                      << region.validation
                      << ") maxIteration: " << region.maxIteration
                      << " width: " << region.width << "height: " << region.height << std::endl;

            // Reset loop flag
            loopFlag = false;

            // Recieve instructions for computation
            std::cout << "Worker " << world_rank << " is listening during computation." << std::endl;
            MPI_Irecv(&newRegion, sizeof(Region), MPI_BYTE, 0, 1, MPI_COMM_WORLD,
                      &request); // Listen for a region asynchronously => store inside newRegion

            // Execute computations
            const unsigned int data_len = region.getPixelCount();
            int data[data_len];
            
            int i = 0;

            for (unsigned int y = 0; y < region.width && !loopFlag; y++) {
                for (unsigned int x = 0; x < region.height && !loopFlag; x++) {
                    // Abort
                    MPI_Test(&request, &flag, &status);
                    if (flag != 0) {
                        std::cout << "Worker " << world_rank << " abort." << std::endl;
                        loopFlag = true;
                    }

                    // Computations
                    data[i++] = f->calculateFractal(region.projectReal(x),
                                                    region.projectImag(y),
                                                    region.maxIteration);
                }
            }
            if (!loopFlag) {
                std::cout << "Worker " << world_rank << " is sending the data: " << data_len << std::endl;
                MPI_Send(&world_rank, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
                MPI_Send(&data, data_len, MPI_INT, 0, 2, MPI_COMM_WORLD);
            }
        }
    }
}
