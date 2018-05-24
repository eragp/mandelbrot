#include <fstream>
#include <iostream>
#include "Fractal.h"
#include "Host.h"
#include "Info.h"
#include "Mandelbrot.h"
#include "Returned.h"

// MPI Libraries
#include <mpi.h>

// Teile diese Codes zur Erzeugung eines Servers sind Abwandlungen oder Kopien von http://mariusbancila.ro/blog/2017/11/19/revisited-full-fledged-client-server-example-with-c-rest-sdk-2-10/
// Cpp REST libraries
#include <map>
#include <set>
#include <string>

using namespace std;

#define TRACE(msg) wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

//ACHTUNG: (2048 / steps = nat. Zahl) muss gelten! Ein Bereich aus Speps*Steps Pixel wird von einem Prozessor / Kern berechnet.
const int steps = 256;

double xToReal(int x, double maxReal, double minReal, int width) {
    return x * ((maxReal - minReal) / width) + minReal;
}

double yToImaginary(int y, double maxImaginary, double minImaginary, int height) {
    return y * ((maxImaginary - minImaginary) / height) + minImaginary;
}

void client(int world_rank, int world_size) {
    Fractal* f = new Mandelbrot();
    while (true) {
        Info info;
        MPI_Recv(&info, sizeof(Info), MPI_BYTE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << "Received data for "
             << "(" << info.start_x << "," << info.start_y << ")"
             << " -> "
             << "(" << info.end_x << "," << info.end_y << ")"
             << endl;
        if (info.start_x != -1) {
            Returned returned;
            for (int x = info.start_x; x < info.end_x; x++) {
                for (int y = info.start_y; y < info.end_y; y++) {
                    returned.n[x - info.start_x][y - info.start_y] = f->calculateFractal(
                        xToReal(x, info.maxReal, info.minReal, info.size),
                        yToImaginary(y, info.maxImaginary, info.minImaginary, info.size),
                        info.maxIteration);
                }
            }
            returned.start_x = info.start_x;
            returned.start_y = info.start_y;
            returned.end_x = info.end_x;
            returned.end_y = info.end_y;
            returned.z = info.z;
            returned.size = info.size;
            returned.world_rank = world_rank;
            MPI_Send((const void*)&returned, sizeof(Returned), MPI_BYTE, 0, 2, MPI_COMM_WORLD);
        } else {
            // Intentionally left blank
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    cout << "Process " << world_rank << " of " << world_size << endl;

    if (world_size == 1) {
        cerr << "need at least 2 processes to run. Currently have "<< world_size << endl;
        MPI_Finalize();
        return -1;  // return with error
    }
    if (world_rank == 0) {
        Host::init(world_rank, world_size);
    } else {
        client(world_rank, world_size);
    }
    return 0;
}