#include "Worker.h"

#include "Fractal.h"
#include "Mandelbrot.h"
#include "Region.h"
#include "Tile.h"
#include "WorkerInfo.h"

#include <mpi.h>

#ifdef _OPENMP
    #include <omp.h>
#endif

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstring>
#include <thread>
#include <chrono>

void Worker::init(int world_rank, int world_size) {
    Fractal *f = new Mandelbrot();
    // Initial test if this core is ready
    int test;
    MPI_Status status;
    MPI_Recv(&test, 1, MPI_INT, MPI_ANY_SOURCE, 10, MPI_COMM_WORLD, &status);
    MPI_Send((void *) &test, 1, MPI_INT, status.MPI_SOURCE, 11, MPI_COMM_WORLD);
    int host_rank = status.MPI_SOURCE;

    Region region, newRegion;
    MPI_Request request;
    int flag;
    // Recieve instructions for computation
    std::cout << "Worker " << world_rank << " is ready to receive Data." << std::endl;

    // Check if OpenMp is enabled
    #ifdef _OPENMP
        std::cout << "OpenMP is enabled." << std::endl;
        std::cout << "OpenMP cancellation: " << omp_get_cancellation() << std::endl;
    #else
        std::cout << "OpenMP is disabled." << std::endl;
    #endif

    MPI_Irecv(&newRegion, sizeof(Region), MPI_BYTE, host_rank, 1, MPI_COMM_WORLD, &request); // Listen for a region asynchronously
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

            // Recieve instructions for computation
            std::cout << "Worker " << world_rank << " is listening during computation." << std::endl;
            MPI_Irecv(&newRegion, sizeof(Region), MPI_BYTE, host_rank, 1, MPI_COMM_WORLD,
                      &request); // Listen for a region asynchronously => store inside newRegion

            // Execute computations
            const unsigned int data_len = region.getPixelCount();
            int* data = new int[data_len];

            // The real computation starts here --> start time measurement here
            auto startTime = std::chrono::high_resolution_clock::now();

            #ifdef _OPENMP

                // The number of OpenMP Threads used
                int num_threads = -1;

                // Start parallel computation using OpenMP Threads.
                // Abort is possible.
                // TODO Remove default(none). Only for development.
                #pragma omp parallel default(none) shared(region, data, f, num_threads, request, flag, status)
                {
                    #pragma omp for schedule(nonmonotonic:dynamic, 10) collapse(2)
                    for (unsigned int y = 0; y < region.height ; y++) {
                        for (unsigned int x = 0; x < region.width; x++) {
                    
                            // Set number of OpenMP Threads used (only once)
                            #ifdef _OPENMP
                                if (y == 0 && x == 0) {
                                    num_threads = omp_get_num_threads();
                                }
                            #endif

                            // Computations
                            data[y * region.width + x] = f->calculateFractal(region.projectReal(x),
                                                                            region.projectImag(region.height - y - 1),
                                                                            region.maxIteration);

                            // Abort
                            if (omp_get_thread_num() == 0) { // Only Master Thread checks if Worker has to abort.
                                MPI_Test(&request, &flag, &status);
                                if (flag != 0) {
                                    // Tell all other threads to abort and execute abort for itself.
                                    #pragma omp cancel for
                                }
                            } else { // Execute abort for all threads exept the Master Thread.
                                #pragma omp cancellation point for
                            }
                        }
                    }
                }

                std::cout << "Worker " << world_rank << " used " << num_threads << " OpenMP Threads." << std::endl;

                if (flag != 0) {
                    std::cout << "Worker " << world_rank << " abort." << std::endl;
                }

            #else

                // Flag to stop computation loops
                bool loopFlag = false;

                // Current position in data array
                int i = 0;

                for (unsigned int y = 0; y < region.height && !loopFlag; y++) {
                    int reverseY = region.height - y - 1;
                    for (unsigned int x = 0; x < region.width && !loopFlag; x++) {
                        // Computations
                        data[i++] = f->calculateFractal(region.projectReal(x),
                                                        region.projectImag(reverseY),
                                                        region.maxIteration);

                        // Abort
                        MPI_Test(&request, &flag, &status);
                        if (flag != 0) {
                            std::cout << "Worker " << world_rank << " abort." << std::endl;
                            loopFlag = true;
                        }
                    }
                }

            #endif

            // Computation ends here --> stop the clock
            auto endTime = std::chrono::high_resolution_clock::now();


            if (flag == 0) {
                // We measure time in microseconds
                unsigned long elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
                std::cout << "Worker " << world_rank << " elapsed time: " << elapsedTime << std::endl;

                // Create WorkerInfo
                WorkerInfo workerInfo;
				workerInfo.rank = world_rank;
				workerInfo.computationTime = elapsedTime;
				workerInfo.region = region;

                std::cout << "Worker " << world_rank << " is sending the data: " << data_len << std::endl;
                MPI_Send(&workerInfo, sizeof(WorkerInfo), MPI_BYTE, host_rank, 3, MPI_COMM_WORLD);
                MPI_Send(data, data_len, MPI_INT, host_rank, 2, MPI_COMM_WORLD);
            }
            delete[] data;
        } else {
            // Reduce processor usage on idle
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
