#include <mpi.h>
#include <iostream>

using namespace std;

// Build with mpic++ main.cpp
// Run with mpirun -np 4 ./a.out

int world_rank;

void run() {
    const int PING_PONG_LIMIT = 10;
    int ping_pong_count = 0;
    int partner_rank = (world_rank + 1) % 2;
    while (ping_pong_count < PING_PONG_LIMIT) {
        if (world_rank == ping_pong_count % 2) {
            // Increment the ping pong count before you send it
            ping_pong_count++;
            MPI_Send(&ping_pong_count, 1, MPI_INT, partner_rank, 0,
                    MPI_COMM_WORLD);
            printf("%d sent and incremented ping_pong_count "
                "%d to rank %d\n", world_rank, ping_pong_count,
                partner_rank);
        } else {
            MPI_Recv(&ping_pong_count, 1, MPI_INT, partner_rank, 0,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("%d received ping_pong_count %d from rank %d\n",
                world_rank, ping_pong_count, partner_rank);
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    char processorname[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processorname, &name_len);

    cout << processorname << ", Size: " << world_size << ", Rank: " << world_rank << endl;
    run();

    MPI_Finalize();
    return 0;
}
