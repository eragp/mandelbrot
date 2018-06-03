#include "Client.h"

#include <iostream>
#include "Fractal.h"
#include "Mandelbrot.h"
#include "TileData.h"
#include "TileInfo.h"

#include <mpi.h>

#include <map>
#include <set>
#include <string>

void Client::init(int world_rank, int world_size) {
    Fractal* f = new Mandelbrot();
    int test;
	MPI_Recv(&test, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Send((const void*)&test, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	while (true) {
        std::cout << "Worker " << world_rank << " is ready to receive Data." << std::endl;
		TileInfo tileInfo;
        MPI_Recv(&tileInfo, sizeof(TileInfo), MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Worker " << world_rank << " received data: (MinReal -> " << tileInfo.minReal << ") ; (MaxReal -> " << tileInfo.maxReal << ") ; (MinImaginary -> " << tileInfo.minImaginary << ") ; (MaxImaginary -> " << tileInfo.maxImaginary << ") ; (MaxIteration -> " << tileInfo.maxIteration << ")" << std::endl;
		
		int data[tileInfo.xRes][tileInfo.yRes];
		
		double deltaReal = Fractal::deltaReal(tileInfo.maxReal, tileInfo.minReal, tileInfo.xRes);
		double deltaImaginary = Fractal::deltaImaginary(tileInfo.maxImaginary, tileInfo.minImaginary, tileInfo.yRes);
        for (int x = 0 ; x < tileInfo.xRes ; x++) {
			for (int y = 0 ; y < tileInfo.yRes ; y++) {
				data[x][y] = f->calculateFractal((tileInfo.minReal + x * deltaReal), (tileInfo.minImaginary + y * deltaImaginary), tileInfo.maxIteration);
			}
		}

        std::cout << "Last send Data at (" << (tileInfo.xRes - 1) << ", " << (tileInfo.yRes - 1) << ") is " << data[tileInfo.xRes - 1][tileInfo.yRes - 1] << std::endl;
		std::cout << world_rank << " is sending the data" << std::endl;
		MPI_Send((const void*)&world_rank, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
		MPI_Send((const void*)&data, sizeof(data), MPI_BYTE, 0, 2, MPI_COMM_WORLD);
    }
}
