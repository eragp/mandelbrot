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

// Utils
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <algorithm>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#define TRACE(msg) std::wcout << msg
#define TRACE_ACTION(a, k, v) std::wcout << a << L" (" << k << L", " << v << L")\n"

const int default_res = 128;
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

void answer_request(http_request request, Tile tile, TileData data) {
    auto response = http_response();
    response.set_status_code(status_codes::OK);
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Access-Control-Allow-Methods"), U("GET"));

    // Create json value from returned
    json::value answer;
    answer[U("rank")] = json::value(data.world_rank);
    json::value tile_json = json::value();
    for (unsigned int y = 0; y < tile.resY; y++) {
        for (unsigned int x = 0; x < tile.resX; x++) {
            unsigned int index = y * tile.resX + x;
            tile_json[index] = data.n[index];
        }
    }
    answer[U("tile")] = tile_json;
    response.set_body(answer);
    request.reply(response);
}

void answer_request_error(http_request request, std::string message) {
    auto response = http_response();
    response.set_status_code(500);
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.headers().add(U("Access-Control-Allow-Methods"), U("GET"));
    request.reply(response);
}

/**
 * Answers all stored requests for the given region.
 */
void Host::answer_requests(Region rendered_region) {
    // is the region still the requested one?
    bool inside_current_region;
    {
        std::lock_guard<std::mutex> lock(big_tile_lock);
        inside_current_region = current_big_tile.contains(rendered_region.tlX, rendered_region.tlY,
                                                          rendered_region.brX, rendered_region.brY,
                                                          rendered_region.zoom);

    }
    if (!inside_current_region) {
        std::cerr << "Host: region no longer needed" << std::endl;
        return;
    }
    std::vector<Tile> tiles = rendered_region.getTiles();
    for (auto const &tile : tiles) {
        std::lock_guard<std::mutex> lock1(available_tiles_lock[tile]);
        TileData data = available_tiles[tile];
        std::lock_guard<std::mutex> lock2(request_dictionary_lock[tile]);
        // iterate over all requests in the queue and check if they can be answered
        // Because we remove elements from this vector in time it is important
        // that we iterate backwards
        auto it_tile = request_dictionary.find(tile);
        if (it_tile == request_dictionary.end()) {
            std::cerr << "Request not found for " << " x:" << tile.x
                      << " y:" << tile.y
                      << " z:" << tile.zoom
                      << " size:" << tile.resX << std::endl;
            continue;
        }
        std::vector<web::http::http_request> requests = request_dictionary[tile];
        for (auto request : requests) {
            std::cout << "Answering Request for"
                      << " x:" << tile.x
                      << " y:" << tile.y
                      << " z:" << tile.zoom
                      << " size:" << tile.resX << std::endl;
            // reply with data
            answer_request(request, tile, data);
        }
        request_dictionary.erase(tile);
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
    auto it_x = data.find(U("x")),
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

        // Check if this tile is still going to be answered
        /*bool inside_current_region;
        {
            std::lock_guard<std::mutex> lock(big_tile_lock);
            inside_current_region = current_big_tile.contains(requested_tile.x, requested_tile.y,
                                       requested_tile.x, requested_tile.y,
                                       requested_tile.zoom);

        }
        if (!inside_current_region) {
            std::cerr << "Host: Tile not in current region ("
                      << requested_tile.x << ", " << requested_tile.y << ", " << requested_tile.zoom << ")"
                      << std::endl;
            answer_request_error(request, "Tile not in current reqion");
            return;
        }*/

        // Store or answer requests
        bool answerable;
        {
            std::lock_guard<std::mutex> lock(available_tiles_lock[requested_tile]);
            auto it_available = available_tiles.find(requested_tile);
            answerable = !(it_available == available_tiles.end());
            if (answerable) {
                answer_request(request, requested_tile, available_tiles[requested_tile]);
            }
        }
        if (!answerable) {
            std::cout << "tile not found in available tiles" << std::endl;
            std::cout << "Storing Request at"
                      << " x:" << x
                      << " y:" << y
                      << " z:" << z
                      << " size:" << size << std::endl;
            std::lock_guard<std::mutex> lock(request_dictionary_lock[requested_tile]);
            request_dictionary[requested_tile].push_back(request);
        }
    } else {
        TRACE(U("Not enough arguments\n"));
    }
}

/**
 * Accept the request for the computation of a complete region,
 * answer with the result of the loadbalancer
 * TODO lock this method if necessary
 */
void Host::handle_get_region(http_request request) {
    std::cout << "Handle GET region" << std::endl;
    // Expect coordinates from query
    auto data = uri::split_query(request.request_uri().query());
    auto it_zoom = data.find(U("zoom")),
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

        {
            std::lock_guard<std::mutex> lock(big_tile_lock);
            current_big_tile = region;
        }

        // TODO increase by one as soon as host is invoked as worker too
        int nodeCount = Host::world_size - 1;
        // TODO make this based on balancer variable defined above (sounds like strategy pattern...)
        Balancer *b = new IntegerBalancer();
        Region *blocks = b->balanceLoad(region, nodeCount);  //Tiles is array with nodeCount members
        // DEBUG
        std::cout << "Balancer Output:" << std::endl;
        for (int i = 0; i < nodeCount; i++) {
            std::cout << "Tile " << i << ": "
                      << "(" << blocks[i].tlX << ", " << blocks[i].brX
                      << ")" << std::endl;
        }
        // send regions to cores deterministically
        MPI_Request region_requests[nodeCount];
        MPI_Status region_status[nodeCount];
        for (int i = 0; i < nodeCount; i++) {
            Region region = blocks[i];
            int rank_worker = i + 1; // TODO for now, see nodeCount
            std::cout << "Invoking core " << rank_worker << std::endl;
            // Tag for computation requests is 1
            // Send new region
            MPI_Isend((const void *) &region, sizeof(Region), MPI_BYTE, rank_worker, 1, MPI_COMM_WORLD,
                      &region_requests[i]);
            transmitted_regions[rank_worker] = region;
        }
        // All workers received their region
        MPI_Waitall(nodeCount, region_requests, region_status);

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


        // Throw away all requests inside requested_tiles, that are not inside this region
        for (auto requested_tile = request_dictionary.begin();
             requested_tile != request_dictionary.end(); requested_tile++) {
            Tile key = requested_tile->first;
            bool inside_current_region;
            {
                std::lock_guard<std::mutex> lock(big_tile_lock);
                inside_current_region = current_big_tile.contains(key.x, key.y,
                                                                  key.x, key.y,
                                                                  key.zoom);
            }
            if (!inside_current_region) {
                std::cerr << "Host: Tile not in current region ("
                          << key.x << ", " << key.y << ", " << key.zoom << ")"
                          << std::endl;
                auto requests = requested_tile->second;
                for (auto request : requests) {
                    answer_request_error(request, "Tile not in current reqion");
                }
                request_dictionary.erase(key);
            }
        }
    }
}

void Host::init(int world_rank, int world_size) {
    Host::world_size = world_size;
    int cores = world_size;
    maxIteration = 3000;
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

    // Test if all cores are available
    // We assume from programs side that all cores *are* available, this is merely debug
    for (int i = 1; i < cores; i++) {
        int testSend = i;
        MPI_Send((const void *) &testSend, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        int testReceive;
        MPI_Recv(&testReceive, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (testSend == testReceive) {
            std::cout << "Core " << i << " ready!" << std::endl;
        }
    }

    // MPI - receiving answers and answering requests
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
        unsigned int region_size = rendered_region.getBytes();
        std::vector<int> worker_data(region_size);
        std::cout << "Host: waiting for receive from " << worker_rank << std::endl;
        MPI_Recv((void *) &worker_data[0], region_size, MPI_INT, worker_rank, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Host: received from " << worker_rank << std::endl;

        // Check if this data is up to date requested data
        bool inside_current_region;
        {
            std::lock_guard<std::mutex> lock(big_tile_lock);
            inside_current_region = current_big_tile.contains(rendered_region.tlX, rendered_region.tlY,
                                                              rendered_region.brX, rendered_region.brY,
                                                              rendered_region.zoom);

        }
        if (!inside_current_region) {
            std::cout << "Host: no longer interested in the rendered region" << std::endl;
            continue;
        }
        std::cout << rendered_region.resX << ", " << rendered_region.resY << std::endl;
        // copy the received data in std::vector to individual tiles
        std::vector<TileData> tile_data;
        for (unsigned int i = 0;
             i < static_cast<unsigned int>(rendered_region.getWidth() * rendered_region.getHeight()); i++) {
            TileData data(worker_rank, rendered_region.resX * rendered_region.resY);
            // loop over every pixel and write received data to tiles
            for (unsigned int y = 0; y < (unsigned int) rendered_region.resY; y++) {
                for (unsigned int x = 0; x < (unsigned int) rendered_region.resX; x++) {
                    unsigned int block_offset = i * rendered_region.resX * rendered_region.resY;
                    unsigned int index = y * rendered_region.resX + x;
                    data.n[index] = worker_data.at(block_offset + index);
                }
            }

            tile_data.push_back(data);
        }
        // copy the tiles from worker to available_tiles
        auto tiles = rendered_region.getTiles();
        for (std::vector<int>::size_type i = 0; i != tiles.size(); i++) {
            Tile t = tiles.at(i);
            std::lock_guard<std::mutex> lock(available_tiles_lock[t]);
            available_tiles[t] = tile_data.at(i);
        }
        std::cout << "added " << tiles.size() << " tiles to available_tiles" << std::endl;
        Host::answer_requests(rendered_region);
    }
}