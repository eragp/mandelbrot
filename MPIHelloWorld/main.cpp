#include <mpi.h>
#include <iostream>

using namespace std;

// Build with mpic++ main.cpp
// Run with mpirun -np 4 ./a.out

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char processorname[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processorname, &name_len);

    cout << "Hello!" << endl;
    cout << processorname << endl;
    cout << "Size: " << world_size << endl << "Rank: " << rank <<endl;

    MPI_Finalize();
    return 0;
}
