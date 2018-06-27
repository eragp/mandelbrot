#include <fstream>
#include <iostream>
#include "Client.h"
#include "Host.h"

// MPI Libraries
#include <mpi.h>

#include <string>

#define TRACE(msg) wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

// Attention: default_res should divide 2048! Square of default_res*default_res will be returned
const int default_res = 256;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	std::cout << "Process " << world_rank << " of " << world_size << std::endl;

    if (world_size == 1) {
       std:: cerr << "need at least 2 processes to run. Currently have " << world_size << std::endl;
        MPI_Finalize();
        return -1;  // return with error
    }
    if (world_rank == 0) {
        Host::init(world_rank, world_size);
    } else {
        Client::init(world_rank, world_size);
    }
    return 0;
}