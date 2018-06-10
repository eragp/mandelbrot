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
// Init values with some arbitrary value
int Host::maxIteration = 200;
int Host::world_size = 0;


std::map<TileInfo, std::vector<web::http::http_request>> Host::request_dictionary;

// Store for the current big tile
TileInfo Host::current_big_tile;
// And the subdivided regions
TileInfo* Host::regions;

// Manage available cores 
std::queue<int> Host::avail_cores;

// Manage requests for tiles
std::queue<TileInfo> Host::requested_tiles;

// Manage available = already computed regions
std::map<TileInfo, bool> Host::available_tiles;
std::map<TileInfo, int> Host::available_tiles_rank;

// Store send MPI Requests
std::map<int, TileInfo> Host::transmitted_tiles;

// Buffer for completed computations
int* Host::data_buffer;

// Synchronise access to critical data structures
std::mutex Host::avail_cores_lock;
std::mutex Host::requested_tiles_lock;
std::mutex Host::transmitted_tiles_lock;
std::map<TileInfo, std::mutex> Host::available_tiles_lock;
std::mutex Host::data_buffer_lock;
std::map<TileInfo, std::mutex> Host::request_dictionary_lock;

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
 * Method for determining if two Tiles (described by TileInfo) are equal
 */
bool tiles_equal(TileInfo one, TileInfo two){
	return	(one.maxImaginary == two.maxImaginary) &&
			(one.minImaginary == two.minImaginary) &&
			(one.maxReal == two.maxReal) &&
			(one.minReal == two.minReal) &&
			(one.maxIteration == two.maxIteration) &&
			(one.xRes == two.xRes) &&
			(one.yRes == two.yRes);
}

/**
 * Returns true if the tiles overlap
 */
bool tiles_overlap(TileInfo one, TileInfo two){
	// TODO
}

/**
 * TODO this will be removed because requests (except for region) will not init cores anymore!
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
 * Determines all big regions that overlap with the given Tile
 * and check if all necessary data has been computed to answer the requests
 * in the corresponding queue
 */
void Host::answer_requests(TileInfo tileInfo){
	// determine overlapping regions
	for(int i = 0; i < Host::world_size; i++){
		TileInfo region = regions[i];
		if(tiles_overlap(tileInfo, region)){
			{
				std::lock_guard<std::mutex>  lock(request_dictionary_lock[region]);
				// iterate over all requests in the queue and check if they can be answered
				// Because we remove elements from this vector in time it is important
				// that we iterate backwards
				for(int j = request_dictionary[region].size(); j >= 0 ; j--){
					auto request = request_dictionary[region][j];
					// if all data is available in the data buffer, answer the request
					bool data_avail = true;
					for(int k = 0; k < Host::world_size; k++){
						TileInfo region = regions[i];
						if(tiles_overlap(tileInfo, region)){
							std::lock_guard<std::mutex>  lock(available_tiles_lock[region]);
							if(!available_tiles[region]){
								data_avail = false;
								break;
							}
						}
					}
					if(!data_avail) continue;

					// fill reply with data
					auto response = http_response();
					response.set_status_code(status_codes::OK);
					// CORs enabling
					response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
					response.headers().add(U("Access-Control-Allow-Methods"), U("GET"));
					// Create json value from returned
					json::value answer;
					answer[U("rank")] = json::value(available_tiles_rank[region]);
					json::value tile;
					int start_x = Fractal::realToX(tileInfo.minReal, current_big_tile.maxReal, current_big_tile.minReal, current_big_tile.xRes);
					int start_y = Fractal::imaginaryToY(tileInfo.minImaginary, current_big_tile.maxImaginary, current_big_tile.minImaginary, current_big_tile.yRes);
					{
						std::lock_guard<std::mutex> lock(data_buffer_lock);
						for (int x = 0 ; x < tileInfo.xRes ; x++) {
							for (int y = 0 ; y < tileInfo.yRes ; y++) {
								tile[x + y * tileInfo.xRes] = data_buffer[(start_x + x) + ((start_y + y) * current_big_tile.xRes)];
							}
						}
					}
					answer[U("tile")] = tile;
					response.set_body(answer);
					request.reply(response);
					// TODO Remove request
					request_dictionary[region].erase[j];
				}
			}
		}
	}
}

/**
 * Handle a request for a new tile from client side
 * This method is responsible to create the identifier for the response. It's not about new computation requests.
 */
void Host::handle_get_tile(http_request request) {
    TRACE(U("\nhandle GET tile\n"));
    // Expect coordinates from query
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
		
		// Create an identifier to store the received request
		// It is only important which node this request is associated to
		// @Niels Maybe its better to use x, y and size which represent the actual location in the data_buffer Array. I could change that, if that would be better for you. -Tobi
		 // TODO find corresponding regions devised by the host and push this request there
		// TODO need to compare x/y coordinates here -HOW?
		TileInfo requested_tile;
		requested_tile.maxIteration = Host::maxIteration;
		// TODO assign value correctly => transform x,y,z,size to minImaginary/maxImaginary/xRes etc
		for(int i = 0; i < Host::world_size; i++){
			TileInfo region = regions[i];
			if(tiles_overlap(requested_tile, region)){
				// TODO store incoming request at correct region queue (meaning if there is an overlap)
				// => in all regions that are hit, so that when worker is done per request check about "completeness" can be done and
				// incomplete tiles will simply be thrown away
				std::cout << "Storing request at"
					<< " x:" << x
					<< " y:" << y
					<< " z:" << z
					<< " size:" << size << std::endl;
				{
					std::lock_guard<std::mutex>  lock(request_dictionary_lock[region]);
					request_dictionary[region].push_back(request);
				}
			}
		}
		// TODO if regions have been computed, answer request
		// => same method as in mpi_recv loop
		Host::answer_requests(requested_tile);
		 
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
	TRACE(U("\nhandle GET tile\n"));
    // Expect coordinates from query
    auto data = uri::split_query(request.request_uri().query());
	std::map<utility::string_t, utility::string_t>::iterator it_x = data.find(U("x")),
                                                        it_y = data.find(U("y")),
                                                        it_z = data.find(U("z")),
                                                        it_size = data.find(U("size")),
														it_balancer = data.find(U("balancer"));
    // Returns either value at x/y or the whole array
    if (it_x != data.end() && it_y != data.end() && it_z != data.end()) {
        int x = stoi(data["x"]),
            y = stoi(data["y"]),
            z = stoi(data["z"]),
            size = steps;
        if (it_size != data.end()) {
            size = stoi(data["size"]);
        }
		// Additional option to define balancer in region
		utility::string_t balancer = "naive";
		if(it_balancer != data.end()){
			balancer = data["balancer"];
		}
		// TODO set this instance of TileInfo with all the recieved information
		// sizeX possible if we got both sizeX and sixeY from frontend
		current_big_tile.minReal = unprojectX(x, z, 0, size);
		current_big_tile.maxReal = unprojectX(x, z, size, size);
		current_big_tile.minImaginary = unprojectY(y, z, 0, size);
		current_big_tile.maxImaginary = unprojectY(y, z, size, size);
		current_big_tile.xRes = size;
		current_big_tile.yRes = size;
		current_big_tile.maxIteration = maxIteration;
		
		// Update data_buffer size
		{
			std::lock_guard<std::mutex> lock(data_buffer_lock);
			delete[] data_buffer;
			data_buffer = new int[current_big_tile.xRes*current_big_tile.yRes];
		}

		// TODO clear available tiles dict


		int nodeCount = Host::world_size;
		// TODO make this based on balancer variable defined above
		Balancer* b = new NaiveBalancer();
		TileInfo* tiles = b->balanceLoad(current_big_tile, nodeCount); //Tiles is array with nodeCount members
		delete b;
		std::cout << "Tile 0: " << tiles[0].minReal << std::endl;
		std::cout << "Tile 3: " << tiles[3].minReal << std::endl;
		
		// Send the information to Workers via Host::request_more method
		{
			std::lock_guard<std::mutex>  lock(requested_tiles_lock);
			for (int i=0 ; i<nodeCount ; i++) {
				requested_tiles.push(tiles[i]);
			}
		}
		// Store that these regions have not been computed yet
		for(int i = 0; i < world_size; i++){
			std::lock_guard<std::mutex>  lock(available_tiles_lock[tiles[i]]);
			available_tiles[tiles[i]] = false;
			available_tiles_rank[tiles[i]] = -1;
		}

		// Invoke request more at least once per tile (without causing deadlock)
		for(int i = 0; i < world_size; i++){
			// request more only invokes when a core is available
			// => will be invoked as soon as available
			request_more();
		}
		// Store split up regions
		
		delete[] Host::regions; // TODO does this work? want to delete memory of last tile split
		Host::regions = tiles;
		
		
		// Array tiles (struct TileData) of length nodeCount is your result. Converted to Frontend-Format
		// @Tobi is this the correct format?
		//Mock until now
		auto response = http_response();
		response.set_status_code(status_codes::OK);
		// CORs enabling
		response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
		response.headers().add(U("Access-Control-Allow-Methods"), U("GET"));

		auto reply = json::value();
		for(int i = 0; i < Host::world_size; i++){
			reply[i] = json::value();
			reply[i]["x"] = Fractal::realToX(tiles[i].minReal, current_big_tile.maxReal, current_big_tile.minReal, current_big_tile.xRes);
			reply[i]["y"] = Fractal::imaginaryToY(tiles[i].minImaginary, current_big_tile.maxImaginary, current_big_tile.minImaginary, current_big_tile.yRes);
			reply[i]["width"] = tiles[i].xRes;
			reply[i]["height"] = tiles[i].yRes;
		}
		
		response.set_body(reply);
		
		request.reply(response);
	}
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
	Host::world_size = world_size;
    int cores = world_size;
    maxIteration = 200;
	std::cout << "Host init " << world_size << std::endl;

	// REST
    /*
	* IMPORTANT: use the U() Makro for any strings passed to cpprest. This is required for Linux compilation
	*/

	// Init tile request handling
    auto resturl = web::uri_builder();
    resturl.set_host(U("0.0.0.0"));
    resturl.set_scheme(U("http"));
    resturl.set_port(U("80"));
    resturl.set_path(U("mandelbrot"));

    http_listener listener_tile(resturl.to_uri());
    listener_tile.support(methods::GET, handle_get_tile);

	// Init region request handling
    resturl = web::uri_builder();
    resturl.set_host(U("0.0.0.0"));
    resturl.set_scheme(U("http"));
    resturl.set_port(U("80"));
    resturl.set_path(U("region"));

    http_listener listener_region(resturl.to_uri());
    listener_region.support(methods::GET, handle_get_region);

	
    try {
		// These are listeners. They start a new thread that calls the methods given
		// a few lines before when a new get request arrives
		// handle_get and handle_get_region are never called directly!
		// They can store requests in a dictionary which we can access later in the MPI_recv loop
        listener_tile
            .open()
            .then([&listener_tile]() { TRACE(U("\nlistening for HTTP  Tile Requests\n")); })
            .wait();
		listener_region
            .open()
            .then([&listener_region]() { TRACE(U("\nlistening for HTTP Region Requests\n")); })
            .wait();
			
        }
    catch (std::exception const &e) {
		std::wcout << e.what() << std::endl;
    }

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
		// This accepts messages by the workers and answers requests that were stored before
        // TODO: asynchronous (maybe?)
		int worker_rank;
		MPI_Recv((void *)&worker_rank, 1, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // 3 is tag for world_rank of worker
		TileInfo worker_info;
		{
			std::lock_guard<std::mutex>  lock(transmitted_tiles_lock);
			worker_info = transmitted_tiles[worker_rank];
			transmitted_tiles.erase(worker_rank);
		}
		//int worker_data[worker_info.xRes][worker_info.yRes];
		std::vector<int> worker_data(worker_info.xRes * worker_info.yRes);
		//MPI_Recv((void *)&worker_data, sizeof(worker_data), MPI_BYTE, worker_rank, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // 2 is tag for computed data
		MPI_Recv((void *)&worker_data[0], worker_info.xRes * worker_info.yRes, MPI_INT, worker_rank, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//std::cout << "Last received Data at (" << (worker_info.xRes - 1) << ", " << (worker_info.yRes - 1) << ") is " << worker_data[worker_info.xRes - 1][worker_info.yRes - 1] << std::endl;
		
		// TODO check if we (=User) is still interested in this region
		// or if new big tile has been requested in the meantime
		bool interested = false;
		for(int i = 0; i < world_size; i++){
			if(tiles_equal(worker_info, regions[i])){
				interested = true;
				break;
			}
		}
		if(!interested) continue;

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
					//data_buffer[(start_x + x) + ((start_y + y) * current_big_tile.xRes)] = worker_data[x][y];
					data_buffer[(start_x + x) + ((start_y + y) * current_big_tile.xRes)] = worker_data[x + y * worker_info.xRes];
					std::cout << data_buffer[(start_x + x) + ((start_y + y) * current_big_tile.xRes)] << " ";
				}
				std::cout << std::endl;
			}
		}
		{
			std::lock_guard<std::mutex> lock(available_tiles_lock[worker_info]);
			available_tiles[worker_info] = true;
			available_tiles_rank[worker_info] = worker_rank;
		}

		// TODO check if there are http requests waiting for this data
		// => access the dictionary where we stored the requests accepted by handle_get_tile
		// this can of course be outsourced to a new method
		Host::answer_requests(worker_info);
	}			
    MPI_Finalize();
}