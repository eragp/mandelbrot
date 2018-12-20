#include "Worker.h"

#include "Fractal.h"
#include "Mandelbrot.h"
#include "Mandelbrot32.h"
#include "Mandelbrot64.h"
#include "MandelbrotSIMD32.h"
#include "MandelbrotSIMD64.h"
#include "Region.h"
#include "Tile.h"
#include "WorkerInfo.h"

#include <mpi.h>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstring>
#include <thread>
#include <chrono>

void Worker::init(int world_rank, int world_size) {
    // Initial test if this core is ready
    int test;
    MPI_Status status;
    MPI_Recv(&test, 1, MPI_INT, MPI_ANY_SOURCE, 10, MPI_COMM_WORLD, &status);
    MPI_Send((void *) &test, 1, MPI_INT, status.MPI_SOURCE, 11, MPI_COMM_WORLD);
    int host_rank = status.MPI_SOURCE;
    Fractal *f ;

    bool loopFlag = false;
    Region region, newRegion;
    MPI_Request request;
    int flag;
    // Recieve instructions for computation
    std::cout << "Worker " << world_rank << " is ready to receive Data." << std::endl;
    // Init and start persistent receive (asynchronously)
    MPI_Recv_init(&newRegion, sizeof(Region), MPI_BYTE, host_rank, 1, MPI_COMM_WORLD, &request);
    MPI_Start(&request);
    // Start with actual work of this worker
    while (true) {
        MPI_Test(&request, &flag, &status);
        if (flag != 0) {
            // Set current region to newRegion, copy value explicitly (solve more beautiful if you want to)
            std::memcpy(&region, &newRegion, sizeof(Region));
            // Debug Output
            std::cout << "Worker " << world_rank
                      << " received data: "
                      << "topLeft: (" << region.minReal << ", " << region.maxImaginary << ", " << region.validation
                      << ") ->  bottomRight: (" << region.maxReal << ", " << region.minImaginary << ", "
                      << region.validation
                      << ") maxIteration: " << region.maxIteration
                      << " width: " << region.width << " height: " << region.height << std::endl;

            // Reset loop flag
            loopFlag = false;

            // Recieve instructions for computation
            std::cout << "Worker " << world_rank << " is listening during computation." << std::endl;
            // Listen for a region asynchronously using persistent receive => store inside newRegion
            MPI_Start(&request);

            // Execute computations
            const unsigned int data_len = region.getPixelCount();
            int* data = new int[data_len];
            
            int i = 0;

            // Choose fractal
            switch(region.fractal){
                case mandelbrot32:
                    f = new Mandelbrot32();
                    break;
                case mandelbrot64:
                    f = new Mandelbrot64();
                    break;
                case mandelbrotSIMD32:
                    f = new MandelbrotSIMD32();
                    break;
                case mandelbrotSIMD64:
                    f = new MandelbrotSIMD64();
                    break;
                default:
                    f = new Mandelbrot();
            }

            // The real computation starts here --> start time measurement here
            auto startTime = std::chrono::high_resolution_clock::now();

            int vectorLength = 4; // Maximum SIMD size
            precision_t* projReal = new precision_t[vectorLength];
            precision_t* projImag = new precision_t[vectorLength];
            for (unsigned int y = 0; y < region.height && !loopFlag; y++) {
                for (unsigned int x = 0; x < region.width && !loopFlag; x+=vectorLength) {
                    // Abort
                    MPI_Test(&request, &flag, &status);
                    if (flag != 0) {
                        std::cout << "Worker " << world_rank << " abort." << std::endl;
                        loopFlag = true;
                    }
                    // Computations
                    // Project all points to be computed
                    int reverseY = region.height - y - 1;
                    for(int k = 0; k < vectorLength; k++){
                        projReal[k] = region.projectReal(x+k);
                        projImag[k] = region.projectImag(reverseY);
                    }
                    // Directly write them into our n array
                    f->calculateFractal(projReal,
                                        projImag,
                                        region.maxIteration,
                                        vectorLength,
                                        &data[i]);
                    i += vectorLength;
                }
            }
            delete[] projReal;
            delete[] projImag;

            // Computation ends here --> stop the clock
            auto endTime = std::chrono::high_resolution_clock::now();

            if (!loopFlag) {
                // We measure time in microseconds
                unsigned long elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

                // Create WorkerInfo
                WorkerInfo workerInfo;
				workerInfo.rank = world_rank;
				workerInfo.computationTime = elapsedTime;
				workerInfo.region = region;

                // Pack "workerInfo" and the computed "data" in one coherent storage area "ret"
                const unsigned int ret_len = sizeof(int) * data_len + sizeof(WorkerInfo);
                uint8_t* ret = new uint8_t[ret_len];
                std::memcpy(ret, &workerInfo, sizeof(WorkerInfo));
                std::memcpy(ret + sizeof(WorkerInfo), data, data_len * sizeof(int));
                
                // Send "ret" to the Host using one MPI_Send operation
                std::cout << "Worker " << world_rank << " is sending the data. Total length: " << ret_len << " Bytes. Elapsed time: " << elapsedTime << " microseconds." << std::endl;
                MPI_Send(ret, ret_len, MPI_BYTE, host_rank, 2, MPI_COMM_WORLD); // Rank: 2

                delete[] ret;
            }
            delete[] data;
            delete f;
        } else {
            // Reduce processor usage on idle
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    MPI_Cancel(&request);
    MPI_Request_free(&request);
}
