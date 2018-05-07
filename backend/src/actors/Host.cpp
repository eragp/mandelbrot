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
#include <queue>
#include <map>
#include <string>

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#define TRACE(msg) wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

//ACHTUNG: (2048 / steps = nat. Zahl) muss gelten! Ein Bereich aus Speps*Steps Pixel wird von einem Prozessor / Kern berechnet.
const int steps = 265;
int Host::maxIteration = 200;
double Host::minReal = -1.5;
double Host::maxReal = 0.7;
double Host::minImaginary = -1.0;
double Host::maxImaginary = 1.0;
std::map<std::vector<int>, std::queue<web::http::http_request>> Host::request_dictionary;
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
		if(itsize != data.end()){
			size = stoi(data[U("size")]);
		}

		// Initiate computation on a slave
		// TODO caching could check whether this square has already been computed or is computing 
		// i.e. via request_dictionary[identifier].size() > 0
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
		cout << "Storing request at "  << identifier[0] << identifier[1] << identifier [2] << identifier[3] << endl;
		request_dictionary[identifier].push(request);

		// Invoke the longest unused available slave
		MPI_Request req; // Later => store to check if has been received
		cout << "Invoking core " << avail_cores.front() << endl;
		MPI_Isend((const void*)&info, sizeof(Info), MPI_BYTE, avail_cores.front(), 1, MPI_COMM_WORLD, &req);
		avail_cores.pop();
	} else {
		TRACE(L"Not enough arguments\n");
	}
	
	
}

void Host::init(int world_rank, int world_size) {
	int cores = world_size;
	maxIteration = 200;
	minReal = -1.5;
	maxReal = 0.7;
	minImaginary = -1.0;
	maxImaginary = 1.0;

	// Initialize the cores (except for yourself (id==0))
	for (int i = 1; i < cores; i++) {
		Info info;
		info.start_x = -1;
		Returned returned;
		MPI_Request req;
		MPI_Isend((const void*)&info, sizeof(info), MPI_BYTE, i, 1, MPI_COMM_WORLD, &req);
		//TODO: Send data from "returned" to Frontend. Write a new method for that.
		//TODO: only push if core received data
		avail_cores.push(i);
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
			// TODO asynchronous (maybe?)
			Returned returned;
			MPI_Recv((void*)&returned, sizeof(Returned), MPI_BYTE, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			cout << "Data recv from " << returned.world_rank << " ; " << returned.n[0][0] << endl;
			

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
			// Get the request that was stored before
			// If more than one request demanded exactly this square, answer them all
			cout << "Getting request from "  << identifier[0] << identifier[1] << identifier [2] << identifier[3] << endl;
			while(request_dictionary[identifier].size() > 0){
				http_request request = request_dictionary[identifier].front();
				request.reply(response);
				request_dictionary[identifier].pop();
			}

			avail_cores.push(returned.world_rank);
			//TODO: Send data from "returned" to Frontend. Write a new method for that.	
			cout << "done" << endl;
		}
	}
	catch (exception const &e)
	{
		wcout << e.what() << endl;
	}
	
	MPI_Finalize();
}