#include "Host.h"

#include <fstream>
#include <iostream>
#include "Balancer.h"
#include "Fractal.h"
#include "IntegerBalancer.h"
#include "Region.h"
#include "Tile.h"
#include "TileData.h"

// MPI Libraries
#include <mpi.h>

// Cpp REST libraries
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <string>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#define TRACE(msg) std::wcout << msg
#define TRACE_ACTION(a, k, v) std::wcout << a << L" (" << k << L", " << v << L")\n"

const int default_res = 256;
// Init values with some arbitrary value
int Host::maxIteration = 200;
int Host::world_size = 0;

std::map<Tile, std::vector<web::http::http_request>> Host::request_dictionary;
std::map<Tile, std::mutex> Host::request_dictionary_lock;

// Store for the current big tile
Region Host::current_big_tile;
std::mutex Host::big_tile_lock;

// And the subdivided regions
// Region *Host::regions;

// Manage available cores
std::queue<int> Host::avail_cores;
std::mutex Host::avail_cores_lock;

// Manage requests for regions
std::queue<Region> Host::requested_regions;
std::mutex Host::requested_regions_lock;

// Manage available = already computed regions
std::map<Tile, TileData> Host::available_tiles;
std::map<Tile, std::mutex> Host::available_tiles_lock;

// Store send MPI Requests
std::map<int, Region> Host::transmitted_regions;
std::mutex Host::transmitted_regions_lock;

// Buffer for completed computations
// int *Host::data_buffer;

// synchronized access to critical data structures

/**
 * TODO this will be removed because requests (except for region) will not init cores anymore!
 * Invoke a new core for a new tile
 * Checks necessary conditions on its own, always safe to call
 */
void Host::request_more() {
    // Will never lead to deadlock because this is the only block aquiring these two locks
    // ATTENTION: if another block with 2 locks is introduced, this will not hold anymore
    // Take care then of locking in the same order as here
    std::lock_guard<std::mutex> lock1(avail_cores_lock);
    std::lock_guard<std::mutex> lock2(requested_regions_lock);
    std::lock_guard<std::mutex> lock3(transmitted_regions_lock);
    if (avail_cores.size() > 0 && requested_regions.size() > 0) {
        Region region = requested_regions.front();
        int rank_worker = avail_cores.front();
        MPI_Request req;  // Later => store to check if has been received
        std::cout << "Invoking core " << rank_worker << std::endl;
        // Tag for computation requests is 1
        MPI_Isend((const void *) &region, sizeof(Region), MPI_BYTE, rank_worker, 1, MPI_COMM_WORLD, &req);
        avail_cores.pop();
        requested_regions.pop();
        transmitted_regions[rank_worker] = region;
    }
}

/**
 * Answers all stored requests for the given region.
 */
void Host::answer_requests(Region rendered_region) {
    // is the region still the requested one?
    if (!current_big_tile.contains(rendered_region.tlX, rendered_region.tlY,
                                   rendered_region.brX, rendered_region.brY,
                                   rendered_region.zoom)) {
        std::cerr << "region no longer needed" << std::endl;
        return;
    }
    std::vector<Tile> tiles = rendered_region.getTiles();
    for (auto const &tile : tiles) {
        std::lock_guard<std::mutex> lock(available_tiles_lock[tile]);
        TileData data = available_tiles[tile];
        std::lock_guard<std::mutex> lock2(request_dictionary_lock[tile]);
        // iterate over all requests in the queue and check if they can be answered
        // Because we remove elements from this vector in time it is important
        // that we iterate backwards
        std::map<Tile, std::vector<web::http::http_request>>::const_iterator it_tile =
                request_dictionary.find(tile);
        if (it_tile == request_dictionary.end()) {
            continue;
        }
        std::vector<web::http::http_request> requests = request_dictionary[tile];
        for (int j = requests.size() - 1; j >= 0; j--) {
            auto request = requests[j];
            // fill reply with data
            auto response = http_response();
            response.set_status_code(status_codes::OK);
            response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
            response.headers().add(U("Access-Control-Allow-Methods"), U("GET"));

            // Create json value from returned
            json::value answer;
            answer[U("rank")] = json::value(data.world_rank);
            json::value tile_json;
            for (int x = 0; x < tile.resX; x++) {
                for (int y = 0; y < tile.resX; y++) {
                    int index = y * tile.resX + x;
                    tile_json[index] = data.n[index];
                }
            }
            answer[U("tile")] = tile_json;
            response.set_body(answer);
            request.reply(response);
            // remove request from dict
            requests.erase(requests.begin() + j);
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
                size = default_res;
        if (it_size != data.end()) {
            size = stoi(data["size"]);
        }

        Tile requested_tile{};
        requested_tile.x = x;
        requested_tile.y = y;
        requested_tile.zoom = z;
        requested_tile.resX = size;
        requested_tile.resY = size;
        requested_tile.maxIteration = Host::maxIteration;

        if (!current_big_tile.contains(requested_tile.x, requested_tile.y,
                                       requested_tile.x, requested_tile.y,
                                       requested_tile.zoom)) {
            std::cerr << "Tile not in current region ("
                      << requested_tile.x << ", " << requested_tile.y << ", " << requested_tile.zoom << ")"
                      << std::endl;
            return;
        }
        std::cout << "Storing Request at"
                  << " x:" << x
                  << " y:" << y
                  << " z:" << z
                  << " size:" << size << std::endl;
        {
            std::lock_guard<std::mutex> lock(request_dictionary_lock[requested_tile]);
            request_dictionary[requested_tile].push_back(request);
        }
        // TODO if regions have been computed, answer request
        // => same method as in mpi_recv loop
//        Host::answer_requests(requested_tile);

        // TODO send answer
    } else {
        TRACE(U("Not enough arguments\n"));
    }
}

/**
 * Accept the request for the computation of a complete region,
 * answer with the result of the loadbalancer
 * 
 */
void Host::handle_get_region(http_request request) {
    TRACE(U("\nhandle GET region\n"));
    // Expect coordinates from query
    auto data = uri::split_query(request.request_uri().query());
    std::map<utility::string_t, utility::string_t>::iterator it_zoom = data.find(U("zoom")),
            it_tlX = data.find(U("topLeftX")),
            it_tlY = data.find(U("topLeftY")),
            it_brX = data.find(U("bottomRightX")),
            it_brY = data.find(U("bottomRightY")),
            it_balancer = data.find(U("balancer"));
    // Returns either value at x/y or the whole array
    if (it_zoom != data.end() && it_tlX != data.end() && it_tlY != data.end() && it_brX != data.end() &&
        it_brY != data.end()) {
        int zoom = stoi(data["zoom"]),
                tlX = stoi(data["topLeftX"]),
                tlY = stoi(data["topLeftY"]),
                brX = stoi(data["bottomRightX"]),
                brY = stoi(data["bottomRightY"]),
                size = default_res;
        // if (it_size != data.end()) {
        //     size = stoi(data["size"]);
        // }
        // Additional option to define balancer in region
        utility::string_t balancer = "naive";
        if (it_balancer != data.end()) {
            balancer = data["balancer"];
        }
        Region region{};
        region.tlX = tlX;
        region.tlY = tlY;
        region.brX = brX;
        region.brY = brY;
        region.zoom = zoom;
        region.resX = size;
        region.resY = size;
        region.maxIteration = maxIteration;

        // Update data_buffer size
        /*
        {
            std::lock_guard<std::mutex> lock(data_buffer_lock);
            delete[] data_buffer;
            data_buffer = new int[region.xRes * region.yRes];
        }
        */
        {
            std::lock_guard<std::mutex> lock(big_tile_lock);
            current_big_tile = region;
        }

        int nodeCount = Host::world_size;
        // TODO make this based on balancer variable defined above
        Balancer *b = new IntegerBalancer();
        Region *blocks = b->balanceLoad(region, nodeCount);  //Tiles is array with nodeCount members
        delete b;
        // DEBUG
        for (int i = 0; i < nodeCount; i++) {
            std::cout << "Tile " << i << ": "
                      << "(" << blocks[i].tlX << ", " << blocks[i].brX
                      << ")" << std::endl;
        }
        {
            std::lock_guard<std::mutex> lock(requested_regions_lock);
            for (int i = 0; i < nodeCount; i++) {
                requested_regions.push(blocks[i]);
            }
        }
        // Store that these blocks have not been computed yet
//        for (int i = 0; i < world_size; i++) {
//            std::lock_guard<std::mutex> lock(available_tiles_lock[blocks[i]]);
//            available_tiles[blocks[i]] = false;
//            available_tiles_rank[blocks[i]] = -1;
//        }
        // Invoke request more at least once per region (without causing deadlock)
        for (int i = 0; i < world_size; i++) {
            request_more();
        }
        // Send region division to frontend
        auto response = http_response();
        response.set_status_code(status_codes::OK);
        // CORs enabling
        response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
        response.headers().add(U("Access-Control-Allow-Methods"), U("GET"));

        auto reply = json::value();
        for (int i = 0; i < Host::world_size; i++) {
            auto t = blocks[i];
            reply[i] = json::value();
            reply[i]["nodeID"] = i;
            reply[i]["topLeftX"] = t.tlX;
            reply[i]["topLeftY"] = t.tlY;
            reply[i]["bottomRightX"] = t.brX;
            reply[i]["bottomRightY"] = t.brY;
            reply[i]["zoom"] = t.zoom;
        }
        response.set_body(reply);
        request.reply(response);
    }
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

    // Tile GET requests are ansewered on http://localhost:80/mandelbrot
    auto resturl = web::uri_builder();
    resturl.set_host(U("0.0.0.0"));
    resturl.set_scheme(U("http"));
    resturl.set_port(U("80"));
    resturl.set_path(U("mandelbrot"));

    http_listener listener_tile(resturl.to_uri());
    listener_tile.support(methods::GET, handle_get_tile);

    // Reqion GET requests are answered on http://localhost:80/region
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
                .then([&listener_tile]() { TRACE(U("\nlistening for HTTP Tile Requests\n")); })
                .wait();
        listener_region
                .open()
                .then([&listener_region]() { TRACE(U("\nlistening for HTTP Region Requests\n")); })
                .wait();

    } catch (std::exception const &e) {
        std::wcout << e.what() << std::endl;
    }

    // Test and put cores (except yourself; id = 0) in Queue
    for (int i = 1; i < cores; i++) {
        int testSend = i;
        MPI_Send((const void *) &testSend, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        int testReceive;
        MPI_Recv(&testReceive, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (testSend == testReceive) {
            {
                std::lock_guard<std::mutex> lock(avail_cores_lock);
                avail_cores.push(i);
            }
            std::cout << "Core " << i << " ready!" << std::endl;
        }
    }

    // Only for testing - start
    Region region{};
    region.tlX = -3;
    region.tlY = -4;
    region.brX = 2;
    region.brY = 3;
    region.zoom = 1;
    region.resX = 256;
    region.resY = 256;
    region.maxIteration = maxIteration;
    /*
    {
        std::lock_guard<std::mutex> lock(data_buffer_lock);
        data_buffer = new int[region.resX * region.resY];
    }
     */
    MPI_Request req;
    MPI_Isend((const void *) &region, sizeof(Region), MPI_BYTE, 1, 1, MPI_COMM_WORLD, &req);
    {
        std::lock_guard<std::mutex> lock(transmitted_regions_lock);
        transmitted_regions[1] = region;
    }
    // Only for testing - end

    // MPI
    while (true) {
        // Listen for incoming complete computations from workers (MPI)
        // This accepts messages by the workers and answers requests that were stored before
        // TODO: asynchronous (maybe?)
        int worker_rank;
        MPI_Recv((void *) &worker_rank, 1, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);  // 3 is tag for world_rank of worker
        Region rendered_region{};
        {
            std::lock_guard<std::mutex> lock(transmitted_regions_lock);
            rendered_region = transmitted_regions[worker_rank];
            transmitted_regions.erase(worker_rank);
        }
        //int worker_data[rendered_region.xRes][rendered_region.yRes];
        unsigned int region_size = rendered_region.getBytes();
        std::vector<int> worker_data(region_size);
        //MPI_Recv((void *)&worker_data, sizeof(worker_data), MPI_BYTE, worker_rank, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // 2 is tag for computed data
        MPI_Recv((void *) &worker_data[0], region_size, MPI_INT, worker_rank, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //std::cout << "Last received Data at (" << (rendered_region.xRes - 1) << ", " << (rendered_region.yRes - 1) << ") is " << worker_data[rendered_region.xRes - 1][rendered_region.yRes - 1] << std::endl;

        // TODO check if we (=User) is still interested in this region
        // or if new big tile has been requested in the meantime
        if (!current_big_tile.contains(region.tlX, region.tlY, region.brX, region.brY, region.zoom)) {
            continue;
        }
        // Put Worker back in Queue
        {
            std::lock_guard<std::mutex> lock(avail_cores_lock);
            avail_cores.push(worker_rank);
        }
        std::vector<TileData> tile_data;
        for (int i = 0; i < rendered_region.getWidth() * rendered_region.getHeight(); i++) {
            TileData data(world_rank, rendered_region.resX * rendered_region.resY);
            for (unsigned int y = 0; y < (unsigned int) rendered_region.resY; y++) {
                for (unsigned int x = 0; x < (unsigned int) rendered_region.resX; x++) {
                    unsigned int index = (y * rendered_region.resY + x);
                    data.n[index] = worker_data.at((i + 1) * index);
                }
            }
        }
        // Store received worker_data in buffer
        {
            // std::lock_guard<std::mutex> lock(data_buffer_lock);
            // for (int x = 0; x < rendered_region.xRes; x++) {
            //     for (int y = 0; y < rendered_region.yRes; y++) {
            //         //data_buffer[(start_x + x) + ((start_y + y) * region.xRes)] = worker_data[x][y];
            //         data_buffer[(start_x + x) + ((start_y + y) * region.xRes)] = worker_data[x + y * rendered_region.xRes];
            //         std::cout << data_buffer[(start_x + x) + ((start_y + y) * region.xRes)] << " ";
            //     }
            //     std::cout << std::endl;
            // }
        }
        // TODO write to tiles instead of region
        std::vector<Tile> tiles = region.getTiles();
        for (std::vector<int>::size_type i = 0; i != tiles.size(); i++) {
            Tile t = tiles.at(i);
            std::lock_guard<std::mutex> lock(available_tiles_lock[t]);
            available_tiles[t] = tile_data.at(i);
        }

        // TODO check if there are http requests waiting for this data
        // => access the dictionary where we stored the requests accepted by handle_get_tile
        // this can of course be outsourced to a new method
        Host::answer_requests(rendered_region);
    }
    // why is this here?
    // MPI_Finalize();
}