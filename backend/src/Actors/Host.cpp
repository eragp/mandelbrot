#include <fstream>
#include <iostream>
#include "Fractal.h"
#include "Mandelbrot.h"
#include "Host.h"
#include "Returned.h"
#include "Info.h"

// MPI Libraries
#include <mpi.h>

// Teile diese Codes zur Erzeugung eines Servers sind Abwandlungen oder Kopien von http://mariusbancila.ro/blog/2017/11/19/revisited-full-fledged-client-server-example-with-c-rest-sdk-2-10/
// Cpp REST libraries
#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <set>
#include <string>

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#define TRACE(msg) wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

//ACHTUNG: (2048 / steps = nat. Zahl) muss gelten! Ein Bereich aus Speps*Steps Pixel wird von einem Prozessor / Kern berechnet.
const int steps = 64;
int Host::maxIteration = 200;
double Host::minReal = -1.5;
double Host::maxReal = 0.7;
double Host::minImaginary = -1.0;
double Host::maxImaginary = 1.0;
std::map<std::vector<int>, web::http::http_request> Host::request_dictionary;
// Verwaltet die verfügbaren Kerne
std::queue<int> Host::avail_cores;	

void Host::handle_get(http_request request)
{
	TRACE(L"\nhandle GET\n");

	// Expect a data map string->string with x, y and z coordinate
	auto data = uri::split_query(request.request_uri().query());
	map<utility::string_t, utility::string_t>::iterator itx = data.find(U("x"));
	map<utility::string_t, utility::string_t>::iterator ity = data.find(U("y"));
	map<utility::string_t, utility::string_t>::iterator itsize = data.find(U("size"));
	map<utility::string_t, utility::string_t>::iterator itz = data.find(U("z"));

	// Returns either value at x/y or the whole array
	// TODO correctly assign x/y to areas of the mandelbrot set
	// TODO access the buffer of computed tiles to choose the correct one
	if (itx != data.end() && ity != data.end() && itz != data.end()) {
		int x = stoi(data[U("x")]);
		int y = stoi(data[U("y")]);
		int z = stoi(data[U("z")]);
		int size = 256;
		if(itsize == data.end()){
			size = stoi(data[U("size")]);
		}

		// Initiate computation on a slave
		Info info;
		info.start_x = x;
		info.start_y = y;
		info.end_x = x + size;
		info.end_y = y + size;
		info.z = z;
		info.size = size;
		info.maxIteration = maxIteration;
		info.minReal = minReal;
		info.maxReal = maxReal;
		info.minImaginary = minImaginary;
		info.maxImaginary = maxImaginary;

		// Create an identifier to store the received request
		vector<int> identifier = {x,y,z,size};
		request_dictionary.insert(pair<vector<int>, http_request>(identifier, request));

		// Invoke any available slave
		MPI_Request req; // Later => store to check if has been received
		MPI_Isend((const void*)&info, sizeof(Info), MPI_BYTE, MPI_ANY_TAG, 0, MPI_COMM_WORLD, &req); //avail_cores.pop(), 0, MPI_COMM_WORLD);
	}
		
	
	
}

void Host::init(int world_rank, int world_size) {
	int cores = world_size;
	maxIteration = 200;
	minReal = -1.5;
	maxReal = 0.7;
	minImaginary = -1.0;
	maxImaginary = 1.0;

	// Initialize the cores
	for (int i = 0; i < cores; i++) {
		Info info;
		info.start_x = -1;
		Returned returned;
		MPI_Request req;
		MPI_Isend((const void*)&info, sizeof(info), MPI_BYTE, i, 0, MPI_COMM_WORLD, &req);
		//TODO: Send data from "returned" to Frontend. Write a new method for that.
	}

	// Kommunikationsteil

	// REST 
	/*
	* IMPORTANT: use the U() Makro for any strings passed to cpprest. This is required for Linux compilation
	*/
	auto resturl = web::uri_builder();
	resturl.set_host(U("0.0.0.0"));
	resturl.set_scheme(U("http"));
	resturl.set_port(U("80"));
	resturl.set_path(U("mandelbrot"));

	http_listener listener(resturl.to_uri());
	listener.support(methods::GET, handle_get);


	try
	{
		listener
			.open()
			.then([&listener]() { TRACE(U("\nstarting to listen\n")); })
			.wait();

		while (true){
			// Listen for incoming complete computations
			// TODO asynchronous
			Returned returned;
			MPI_Recv((void*)&returned, sizeof(Returned), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			auto response = http_response();
			response.set_status_code(status_codes::OK);
			// CORs enabling
			response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
			response.headers().add(U("Access-Control-Allow-Methods"), U("GET"));
			// Create json value from returned
			json::value tile;
			for(int x = 0; x < returned.size; x++){
				for(int y = 0; y < returned.size; y++){
					tile[x+y*returned.size] = returned.n[x][y];
				}
			}
			response.set_body(tile);

			vector<int> identifier = {
				returned.start_x,
				returned.start_y,
				returned.z,
				returned.size
			};
			http_request request = request_dictionary.at(identifier);
			request.reply(response);

			avail_cores.push(returned.world_rank);

			
			cout << "Data recv from " << returned.world_rank << " ; " << returned.n[0][0] << "\n";
			//TODO: Send data from "returned" to Frontend. Write a new method for that.
		}
	}
	catch (exception const &e)
	{
		wcout << e.what() << endl;
	}
	
	MPI_Finalize();
}