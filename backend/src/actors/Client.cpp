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
    // Initial test if this core is ready
	int test;
    MPI_Recv(&test, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Send((const void *) &test, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    // Start with actual work of this worker
	while (true) {
        std::cout << "Worker " << world_rank << " is ready to receive Data." << std::endl;
        
		// Abort running computations
		MPI_Request request;
		MPI_Status status;
		int abort;
		int flag;
		bool loopFlag = false;
		MPI_Irecv(&abort, 1, MPI_INT, 0, 4, MPI_COMM_WORLD, &request); // Tag 4: Abort
		
		// Recieve instructions for computation
		Region region;
        MPI_Recv(&region, sizeof(Region), MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		
		// Ignore abort if it is send befor recieving instructions for computation
		MPI_Test(&request, &flag, &status);
		if (flag != 0) { // Abort only if new abort is send
			std::cout << "Worker " << world_rank << " doesn't need to abort." << std::endl;
			MPI_Irecv(&abort, 1, MPI_INT, 0, 4, MPI_COMM_WORLD, &request); // Tag 4: Abort
		}
		
		// Debug Output
        std::cout << "Worker " << world_rank << " received data: TopLeft: (" << region.tlX << ", " << region.tlY << ", "
                  << region.zoom
                  << ") ->  BottomRight: (" << region.brX << ", " << region.brY << ", " << region.zoom
                  << ") MaxIteration: " << region.maxIteration << std::endl;

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
                    int index = (unsigned int) (i + 1) * (y * tile.resX + x);
                    data[index] = f->calculateFractal(real, imag, tile.maxIteration);
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
