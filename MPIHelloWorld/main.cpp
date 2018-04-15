#include <mpi.h>
#include <iostream>

using namespace std;

// Build with mpic++ main.cpp
// Run with mpirun -np 4 ./a.out

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    char processorname[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processorname, &name_len);

    cout << "Hello!" << endl;
    cout << processorname << endl;
    cout << "Size: " << world_size << endl;

    MPI_Finalize();
    return 0;
}
