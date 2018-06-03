#include "Host.h"

#include <fstream>
#include <iostream>
#include "TileData.h" // TODO: Don't use TileData.h anymore. We want to delete it and use TileInfo and the data_buffer instead.
#include "TileInfo.h"
#include "Fractal.h"
#include "Balancer.h"
#include "NaiveBalancer.h"

// MPI Libraries
#include <mpi.h>

// Teile diese Codes zur Erzeugung eines Servers sind Abwandlungen oder Kopien von http://mariusbancila.ro/blog/2017/11/19/revisited-full-fledged-client-server-example-with-c-rest-sdk-2-10/
// Cpp REST libraries
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <mutex>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#define TRACE(msg) std::wcout << msg
#define TRACE_ACTION(a, k, v) std::wcout << a << L" (" << k << L", " << v << L")\n"


// TODO: Don't use TileData.h anymore. We want to delete it and use TileInfo and the data_buffer instead.





//ACHTUNG: (2048 / steps = nat. Zahl) muss gelten! Ein Bereich aus Speps*Steps Pixel wird von einem Prozessor / Kern berechnet.
const int steps = 256;
int Host::maxIteration = 200;


std::map<std::vector<int>, std::queue<web::http::http_request>> Host::request_dictionary;

// Store for the current big tile
TileInfo Host::current_big_tile;

// Verwaltet die verfügbaren Kerne
std::queue<int> Host::avail_cores;

// Verwalte requests
std::queue<TileInfo> Host::requested_tiles;

// Store send MPI Requests
std::map<int, TileInfo> Host::transmitted_tiles;

// Buffer for completed computations
int* Host::data_buffer;

// Synchronisiere zugriffe auf alle kritischen datenstrukturen
std::mutex Host::avail_cores_lock;
std::mutex Host::requested_tiles_lock;
std::mutex Host::transmitted_tiles_lock;
std::mutex Host::data_buffer_lock;
std::map<std::vector<int>, std::mutex> Host::request_dictionary_lock;

/**
 * All 3 methods are used to transform the Frontend-Tile to the Backend-Tile "TileInfo"
 */
double unproject(long tile, int zoom, long local, int size, int bound) {
    if (size == 0) size = 1;
    long tileCount = 1 << zoom;
    return (double)(tile * bound * size + local * bound) / (double)(tileCount * size);
}

double unprojectX(long tileX /*Start der großen Tile*/, int zoom, long localX /*Wo genau*/, int size/*Insgesamt große Tile. TileFläche = Size * Size*/) {
    const int boundReal = 2;
    return unproject(tileX, zoom, localX, size, boundReal);
}

double unprojectY(long tileY, int zoom, long localY, int size) {
    const int boundImag = 2;
    return unproject(tileY, zoom, localY, size, boundImag);
}

/**
 * Invoke a new core for a new tile
 * Checks necessary conditions on its own, always safe to call
 */
void Host::request_more() {
	// Will never lead to deadlock because this is the only block aquiring these two locks
	// ATTENTION: if another block with 2 locks is introduced, this will not hold anymore
	// Take care then of locking in the same order as here
	std::lock_guard<std::mutex>  lock1(avail_cores_lock);
	std::lock_guard<std::mutex>  lock2(requested_tiles_lock);
	std::lock_guard<std::mutex>  lock3(transmitted_tiles_lock);
	if(avail_cores.size() > 0 && requested_tiles.size() > 0){
		TileInfo tileInfo = requested_tiles.front();
		int rank_worker = avail_cores.front();
		MPI_Request req;  // Later => store to check if has been received
		std::cout << "Invoking core " << avail_cores.front() << std::endl;
		// Tag for computation requests is 1
		MPI_Isend((const void *)&tileInfo, sizeof(TileInfo), MPI_BYTE, rank_worker, 1, MPI_COMM_WORLD, &req);
		avail_cores.pop();
		requested_tiles.pop();
		transmitted_tiles[rank_worker] = tileInfo;
	}
}

/**
 * Handle a request for a new tile from client side
 * This method is responsible to create the identifier for the response. It's not about new computation requests.
 */
void Host::handle_get(http_request request) {
    TRACE(U("\nhandle GET\n"));
    // Expect coordinates form query
    auto data = uri::split_query(request.request_uri().query());
	std::map<utility::string_t, utility::string_t>::iterator it_x = data.find(U("x")),
                                                        it_y = data.find(U("y")),
                                                        it_z = data.find(U("z")),
                                                        it_size = data.find(U("size"));
    // Returns either value at x/y or the whole array
    if (it_x != data.end() && it_y != data.end() && it_z != data.end()) {
        int x = stoi(data["x"]),
            y = stoi(data["y"]),
            z = stoi(data["z"]),
            size = steps;
        if (it_size != data.end()) {
            size = stoi(data["size"]);
        }

        // Initiate computation on a slave
        // TODO: caching could check whether this square has already been computed or is computing
        // i.e. via request_dictionary[identifier].size() > 0
        
		// Delete. Not needed, because we don't init a computation.
		/* Tile tile;
        tile.x = x;
        tile.y = y;
        tile.zoom = z;
        tile.size = size;
        tile.maxIteration = maxIteration;*/
		
		// Create an identifier to store the received request
		// @Nils Maybe its better to use x, y and size which represent the actual location in the data_buffer Array. I could change that, if that would be better for you. -Tobi
		std::vector<int> identifier = {x, y, z, size};
		std::cout << "Storing request at"
             << " x:" << identifier[0]
             << " y:" << identifier[1]
             << " z:" << identifier[2]
             << " size:" << identifier[3] << std::endl;
		{
			std::lock_guard<std::mutex>  lock(request_dictionary_lock[identifier]);
			request_dictionary[identifier].push(request);
		}
    } else {
        TRACE(U("Not enough arguments\n"));
    }
}

/**
 * Accept the request for the computation of a complete region,
 * answer with the result of the loadbalancer
 * 
 * TODO what data should be transmitted by the client?
 * TODO 
 */
void Host::handle_get_region(http_request request){
    int x, y, z, size; //TODO
	// TODO set this instance of TileInfo with all the recieved information
	// sizeX possible if we got both sizeX and sixeY from frontend
	current_big_tile.minReal = unprojectX(x, z, 0, size);
	current_big_tile.maxReal = unprojectX(x, z, size, size);
	current_big_tile.minImaginary = unprojectY(y, z, 0, size);
	current_big_tile.maxImaginary = unprojectY(y, z, size, size);
	current_big_tile.xRes = size;
	current_big_tile.yRes = size;
	current_big_tile.maxIteration = 200;	//TODO Dynamic Iteration
	
	// Update data_buffer size
	{
		std::lock_guard<std::mutex> lock(data_buffer_lock);
		data_buffer = new int[current_big_tile.xRes*current_big_tile.yRes];
	}
	
	// TODO Test if this actually works
	int nodeCount = 4; // For the future: Get nodeCount from Frontend and put it instead of the 4
	Balancer* b = new NaiveBalancer();
	TileInfo* tiles = b->balanceLoad(current_big_tile, nodeCount); //Tiles is array with nodeCount members
	std::cout << "Tile 0: " << tiles[0].minReal << std::endl;
	std::cout << "Tile 3: " << tiles[3].minReal << std::endl;
	
	// Send the information to Workers via Host::request_more method
	{
		std::lock_guard<std::mutex>  lock(requested_tiles_lock);
		for (int i=0 ; i<nodeCount ; i++) {
			requested_tiles.push(tiles[i]);
			request_more();
		}
	}
	
	
	//@Nils Array tiles (struct TileData) of length nodeCount is your result. Maybe convert to Frontend-Format?
	//Mock until now
    auto response = http_response();
    response.set_status_code(status_codes::NotImplemented);
    // CORs enabling
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Access-Control-Allow-Methods"), U("GET"));

    auto reply = json::value(U("Please contact Niels what kind of information you want to transmit"));
    response.set_body(reply);
    
    request.reply(response);

}

/**
 * Response to handle_get HTTP request.
 */

void Host::handle_response() {
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

	// TODO @Nils Some problems here due to the new spezification with the little requests.
	
    /*try {
        listener
            .open()
            .then([&listener]() { TRACE(U("\nlistening for HTTP Requests\n")); })
            .wait();
			
			
			
			auto response = http_response();
            response.set_status_code(status_codes::OK);
            // CORs enabling
            response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
            response.headers().add(U("Access-Control-Allow-Methods"), U("GET"));
            // Create json value from tileData
            json::value tile;
            for (int x = 0; x < tileInfo.xRes; x++) {
                for (int y = 0; y < tileInfo.yRes; y++) {
                    tile[x + y * tileData.xRes] = tileData.n[x + y * tileData.xRes];
                }
            }
            response.set_body(tile);

			std::vector<int> identifier = {tileData.x, tileData.y, tileData.zoom, tileData.size};
			
            // Get the request that was stored before
            // If more than one request demanded exactly this square, answer them all
			std::cout << "Host answering Request"
                 << " x:" << identifier[0]
                 << " y:" << identifier[1]
                 << " z:" << identifier[2]
                 << " size:" << identifier[3] << std::endl;
			{
				std::lock_guard<std::mutex>  lock(request_dictionary_lock[identifier]);
				while (request_dictionary[identifier].size() > 0) {
					http_request request = request_dictionary[identifier].front();
					request.reply(response);
					request_dictionary[identifier].pop();
				}
			}
			{
				std::lock_guard<std::mutex>  lock(avail_cores_lock);
				avail_cores.push(tileData.world_rank);
			}
			std::cout << "Answered Request" << std::endl;
			// Invoke the longest unused available slave for another tile if available
			request_more();
        }
    } catch (std::exception const &e) {
		std::wcout << e.what() << std::endl;
    }*/
}

/**
 * Print function for struct TileInfo for debuging purposes.
 */
void Host::printTileInfo(TileInfo tileInfo) {
	std::cout << "Debug for TileInfo: (MinReal -> " << tileInfo.minReal << ") ; (MaxReal -> "
	<< tileInfo.maxReal << ") ; (MinImaginary -> " << tileInfo.minImaginary << ") ; (MaxImaginary -> "
	<< tileInfo.maxImaginary << ") ; (xRes -> " << tileInfo.xRes << ") ; ( yRes -> " << tileInfo.yRes <<
	") ; (MaxIteration -> " << tileInfo.maxIteration << ")" << std::endl;
}

void Host::init(int world_rank, int world_size) {
    int cores = world_size;
    maxIteration = 200;
	std::cout << "Host init " << world_size << std::endl;

    // Test and put cores (exept yourself ; id = 0) in Queue
    for (int i = 1; i < cores; i++) {
        int testSend = i;
		MPI_Send((const void*)&testSend, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		int testReceive;
		MPI_Recv(&testReceive, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		if (testSend == testReceive) {
			{
				std::lock_guard<std::mutex>  lock(avail_cores_lock);
				avail_cores.push(i);
			}
			std::cout << "Core " << i << " ready!" << std::endl;
		}
    }
	
	// Only for testing - start
	current_big_tile.minReal = -1;
	current_big_tile.maxReal = 1;
	current_big_tile.minImaginary = -1;
	current_big_tile.maxImaginary = 1;
	current_big_tile.xRes = 100;
	current_big_tile.yRes = 100;
	current_big_tile.maxIteration = 200;
	{
		std::lock_guard<std::mutex> lock(data_buffer_lock);
		data_buffer = new int[current_big_tile.xRes*current_big_tile.yRes];
	}
	
	TileInfo tileInfo;
	tileInfo.minReal = -1;
	tileInfo.maxReal = 1;
	tileInfo.minImaginary = -1;
	tileInfo.maxImaginary = 1;
	tileInfo.xRes = 100;
	tileInfo.yRes = 100;
	tileInfo.maxIteration = 200;
	MPI_Request req;
    MPI_Isend((const void *)&tileInfo, sizeof(TileInfo), MPI_BYTE, 1, 1, MPI_COMM_WORLD, &req);
	
	{
		std::lock_guard<std::mutex>  lock(transmitted_tiles_lock);
		transmitted_tiles[1] = tileInfo;
	}
	// Only for testing - end

    // MPI
    while (true) {
		// Listen for incoming complete computations from workers (MPI)
        // TODO: asynchronous (maybe?)
		int worker_rank;
		MPI_Recv((void *)&worker_rank, 1, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // 3 is tag for world_rank of worker
		TileInfo worker_info;
		{
			std::lock_guard<std::mutex>  lock(transmitted_tiles_lock);
			worker_info = transmitted_tiles[worker_rank];
			transmitted_tiles.erase(worker_rank);
		}
		int worker_data[worker_info.xRes][worker_info.yRes];
		MPI_Recv((void *)&worker_data, sizeof(worker_data), MPI_BYTE, worker_rank, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // 2 is tag for computed data
		std::cout << "Last received Data at (" << (worker_info.xRes - 1) << ", " << (worker_info.yRes - 1) << ") is " << worker_data[worker_info.xRes - 1][worker_info.yRes - 1] << std::endl;
		
		// Put Worker back in Queue
		{
			std::lock_guard<std::mutex>  lock(avail_cores_lock);
			avail_cores.push(worker_rank);
		}
		
		// Store received worker_data in buffer
		int start_x = Fractal::realToX(worker_info.minReal, current_big_tile.maxReal, current_big_tile.minReal, current_big_tile.xRes);
		int start_y = Fractal::imaginaryToY(worker_info.minImaginary, current_big_tile.maxImaginary, current_big_tile.minImaginary, current_big_tile.yRes);
		{
			std::lock_guard<std::mutex> lock(data_buffer_lock);
			for (int x = 0 ; x < worker_info.xRes ; x++) {
				for (int y = 0 ; y < worker_info.yRes ; y++) {
					data_buffer[(start_x + x) + ((start_y + y) * current_big_tile.xRes)] = worker_data[x][y];
					std::cout << data_buffer[(start_x + x) + ((start_y + y) * current_big_tile.xRes)] << " ";
				}
				std::cout << std::endl;
			}
		}
		
		// Notify HTTP response, that there is new data
		handle_response();
		
	}			
    MPI_Finalize();
}