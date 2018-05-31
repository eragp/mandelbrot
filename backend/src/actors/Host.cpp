#include "Host.h"

#include <fstream>
#include <iostream>
#include "Returned.h"
#include "Tile.h"
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

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#define TRACE(msg) wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"

//ACHTUNG: (2048 / steps = nat. Zahl) muss gelten! Ein Bereich aus Speps*Steps Pixel wird von einem Prozessor / Kern berechnet.
const int steps = 256;
int Host::maxIteration = 200;
std::map<std::vector<int>, std::queue<web::http::http_request>> Host::request_dictionary;
// Verwaltet die verfügbaren Kerne
std::queue<int> Host::avail_cores;
// Verwalte requests
std::queue<Tile> Host::requested_tiles;
// Synchronisiere zugriffe auf alle kritischen datenstrukturen
std::mutex Host::avail_cores_lock;
std::mutex Host::requested_tiles_lock;
std::map<std::vector<int>, std::mutex> Host::request_dictionary_lock;

/**
 * Invoke a new core for a new tile
 * Checks necessary conditions on its own, always safe to call
 */
void Host::request_more(){
	// Will never lead to deadlock because this is the only block aquiring these two locks
	// ATTENTION: if another block with 2 locks is introduced, this will not hold anymore
	// Take care then of locking in the same order as here
	std::lock_guard<std::mutex>  lock1(avail_cores_lock);
	std::lock_guard<std::mutex>  lock2(requested_tiles_lock);
	if(avail_cores.size() > 0 && requested_tiles.size() > 0){
		Tile tile = requested_tiles.front();
		MPI_Request req;  // Later => store to check if has been received
		cout << "Invoking core " << avail_cores.front() << endl;
		// Tag for computation requests is 1
		MPI_Isend((const void *)&tile, sizeof(Tile), MPI_BYTE, avail_cores.front(), 1, MPI_COMM_WORLD, &req);
		avail_cores.pop();
		requested_tiles.pop();
		//TODO: Send data from "returned" to Frontend. Write a new method for that.
	}
}

/**
 * Handle a request for a new tile from client side
 */
void Host::handle_get(http_request request) {
    TRACE(U("\nhandle GET\n"));
    // Expect coordinates form query
    auto data = uri::split_query(request.request_uri().query());
    map<utility::string_t, utility::string_t>::iterator it_x = data.find(U("x")),
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
        Tile tile;
        tile.x = x;
        tile.y = y;
        tile.zoom = z;
        tile.size = size;
        tile.maxIteration = maxIteration;

        // Create an identifier to store the received request
        vector<int> identifier = {x, y, z, size};
        cout << "Storing request at"
             << " x:" << identifier[0]
             << " y:" << identifier[1]
             << " z:" << identifier[2]
             << " size:" << identifier[3] << endl;
		{
			std::lock_guard<std::mutex>  lock(request_dictionary_lock[identifier]);
			request_dictionary[identifier].push(request);
		}
		{
			std::lock_guard<std::mutex>  lock(requested_tiles_lock);
			requested_tiles.push(tile);
		}
		request_more();
    } else {
        TRACE(U("Not enough arguments\n"));
    }
}

void Host::init(int world_rank, int world_size) {
    int cores = world_size;
    maxIteration = 200;

    // Initialize the cores (except for yourself (id==0))
    for (int i = 1; i < cores; i++) {
        Tile tile;
        tile.size = 0;
        tile.x = 0;
        tile.y = 0;
        tile.zoom = 0;
        //Returned returned;
        MPI_Request req;
        MPI_Isend((const void *)&tile, sizeof(tile), MPI_BYTE, i, 1, MPI_COMM_WORLD, &req);
        //TODO: Send data from "returned" to Frontend. Write a new method for that.
        //TODO: only push if core received data
		{
			std::lock_guard<std::mutex>  lock(avail_cores_lock);
       		avail_cores.push(i);
		}
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

    try {
        listener
            .open()
            .then([&listener]() { TRACE(U("\nlistening for HTTP Requests\n")); })
            .wait();

        while (true) {
            // Listen for incoming complete computations
            // TODO: asynchronous (maybe?)
            Returned returned;
			// Tag for completed computation is 2
            MPI_Recv((void *)&returned, sizeof(Returned), MPI_BYTE, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            cout << "Host received from " << returned.world_rank << " ; " << returned.n[0][0] << endl;

            auto response = http_response();
            response.set_status_code(status_codes::OK);
            // CORs enabling
            response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
            response.headers().add(U("Access-Control-Allow-Methods"), U("GET"));
            // Create json value from returned
            json::value answer;
            answer[U("rank")] = json::value(returned.world_rank);
            json::value tile;
            for (int x = 0; x < returned.size; x++) {
                for (int y = 0; y < returned.size; y++) {
                    tile[x + y * returned.size] = returned.n[x][y];
                }
            }
            answer[U("tile")] = tile;
            response.set_body(answer);

            vector<int> identifier = {returned.x, returned.y, returned.zoom, returned.size};
            // Get the request that was stored before
            // If more than one request demanded exactly this square, answer them all
            cout << "Host answering Request"
                 << " x:" << identifier[0]
                 << " y:" << identifier[1]
                 << " z:" << identifier[2]
                 << " size:" << identifier[3] << endl;
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
				avail_cores.push(returned.world_rank);
			}
			cout << "Answered Request" << endl;
			// Invoke the longest unused available slave for another tile if available
			request_more();
        }
    } catch (exception const &e) {
        wcout << e.what() << endl;
    }
    MPI_Finalize();
}