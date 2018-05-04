#include <fstream>
#include <iostream>
#include "Fractal.h"
#include "Mandelbrot.h"

// MPI Libraries
#include <mpi.h>

// Teile diese Codes zur Erzeugung eines Servers sind Abwandlungen oder Kopien von http://mariusbancila.ro/blog/2017/11/19/revisited-full-fledged-client-server-example-with-c-rest-sdk-2-10/
// Cpp REST libraries
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <map>
#include <set>
#include <string>

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#define TRACE(msg) wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

//ACHTUNG: (2048 / steps = nat. Zahl) muss gelten! Ein Bereich aus Speps*Steps Pixel wird von einem Prozessor / Kern berechnet.
const int steps = 256;

typedef struct {
	int start_x;	//included
	int start_y;	//included
	int end_x;		//excluded
	int end_y;		//excluded
	int height;
	int width;
	int maxIteration;
	double minReal;
	double maxReal;
	double minImaginary;
	double maxImaginary;
} Info;

typedef struct {
	int start_x;	//included
	int start_y;	//included
	int end_x;		//excluded
	int end_y;		//excluded
	int n[steps][steps];
	int world_rank;
} Returned;

Host host;

void client(int world_rank, int world_size) {
	Fractal* f = new Mandelbrot();
	while (true) {
		Info info;
		MPI_Recv(&info, sizeof(Info), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		cout << "Data recv " << info.start_x << " ; " << info.start_y << "\n";
		if (info.start_x != -1) {
			Returned returned;
			for (int x = info.start_x; x < info.end_x; x++) {
				for (int y = info.start_y; y < info.end_y; y++) {
					returned.n[x - info.start_x][y - info.start_y] = f->calculateFractal(xToReal(x, info.maxReal, info.minReal, info.width), yToImaginary(y, info.maxImaginary, info.minImaginary, info.height), info.maxIteration);
				}
			}
			returned.start_x = info.start_x;
			returned.start_y = info.start_y;
			returned.end_x = info.end_x;
			returned.end_y = info.end_y;
			returned.z = z;
			returned.world_rank = world_rank;
			MPI_Send((const void*)&returned, sizeof(Returned), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
		}
		else {
			return;
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

	if(world_size == 1){
		cout << "Not enough processes running!" << endl;
		MPI_Finalize();
	}
	else{
		if (world_rank == 0) {
			host = Host(world_rank, world_size);
		}
		else {
			client(world_rank, world_size);
		}
	}
	return 0;
}