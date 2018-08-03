#include <fstream>
#include <iostream>
#include "Worker.h"
#include "Host.h"

// MPI Libraries
#include <mpi.h>

#include <string>

#define TRACE(msg) wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

// Attention: default_res should divide 2048! Square of default_res*default_res will be returned
const int default_res = 256;

int init(int argc, char **argv, bool host) {
    const char* type = host ? "Host" : "Worker";

    MPI_Init(&argc, &argv);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    // Retreive the processor name to check if host and
    // worker share a node
    char* proc_name = new char[MPI_MAX_PROCESSOR_NAME];
    int proc_name_length;
    MPI_Get_processor_name(proc_name, &proc_name_length);
	std::cout << type << ": " << world_rank << " of " << world_size <<
        " on node " << proc_name << std::endl;

    if (world_size < 2) {
       std:: cerr << "Need at least 2 processes to run. Currently have " << world_size << std::endl;
        MPI_Finalize();
        return -1;  // return with error
    }
    if (host == true) {
        Host::init(world_rank, world_size);
    } else {
        Worker::init(world_rank, world_size);
    }

    MPI_Finalize();

    return 0;
}