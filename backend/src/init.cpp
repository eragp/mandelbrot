#include <fstream>
#include <iostream>

// MPI Libraries
#include <mpi.h>


#define TRACE(msg) wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

// Attention: default_res should divide 2048! Square of default_res*default_res will be returned
const int default_res = 256;

// General initialization of host and worker processes
int init(int argc, char **argv, const char* type, void (*initFunc) (int world_rank, int world_size)) {

    // Set thread level to MPI_THREAD_FUNNELED. We use MPI in exactly one thread per actor.
    int providedMPIThreadLevel;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &providedMPIThreadLevel);

    // std::cout << "Provided thread level: " << providedMPIThreadLevel << " ; Needed thread level: " << MPI_THREAD_FUNNELED << std::endl;

    if (providedMPIThreadLevel < MPI_THREAD_FUNNELED) {
        std::cerr << "MPI thread level support is insufficient." << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
        return -1;  // Return with error
    }

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
       std::cerr << "Need at least 2 processes to run. Currently have " << world_size << std::endl;
        MPI_Finalize();
        return -1;  // return with error
    }
    initFunc(world_rank, world_size);
    MPI_Finalize();

    return 0;
}