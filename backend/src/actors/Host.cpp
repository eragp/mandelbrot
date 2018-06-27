#include "Host.h"

#include <fstream>
#include <iostream>
#include "Balancer.h"
#include "Fractal.h"
#include "IntegerBalancer.h"
#include "RegionOld.h"
#include "Tile.h"
#include "TileData.h"

// MPI Libraries
#include <mpi.h>

// Cpp REST libraries
#include <cpprest/http_listener.h>
#include <cpprest/json.h>

// Websockets
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

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

const int default_res = 256;
// Init values with some arbitrary value
int Host::maxIteration = 200;
int Host::world_size = 0;

std::map<Tile, std::vector<web::http::http_request>> Host::request_dictionary;
std::mutex Host::request_dictionary_lock;

// Store for the current big tile
RegionOld Host::current_big_tile;
std::mutex Host::current_big_tile_lock;

// And the subdivided regions
// RegionOld *Host::regions;

// Manage available cores
std::queue<int> Host::avail_cores;
std::mutex Host::avail_cores_lock;

// Manage requests for regions
std::queue<RegionOld> Host::requested_regions;
std::mutex Host::requested_regions_lock;

// Manage available = already computed regions
std::map<Tile, TileData> Host::available_tiles;
std::mutex Host::available_tiles_lock;

// Store send MPI Requests
std::map<int, RegionOld> Host::transmitted_regions;
std::mutex Host::transmitted_regions_lock;

// Websocket server
websocketpp::server<websocketpp::config::asio> Host::websocket_server;
websocketpp::connection_hdl Host::client;

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
    for (unsigned int y = 0; y < (unsigned int) tile.resY; y++) {
        for (unsigned int x = 0; x < (unsigned int) tile.resX; x++) {
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
void Host::answer_requests(RegionOld rendered_region) {
    // is the region still the requested one?
    bool inside_current_region;
    {
        std::lock_guard<std::mutex> lock(current_big_tile_lock);
        inside_current_region = current_big_tile.contains(rendered_region.tlX, rendered_region.tlY,
                                                          rendered_region.brX, rendered_region.brY,
                                                          rendered_region.zoom);

    }
    if (!inside_current_region) {
        std::cerr << "Host: region no longer needed" << std::endl;
        return;
    }
    std::vector<Tile> tiles = rendered_region.getTiles();
    std::lock_guard<std::mutex> lock1(request_dictionary_lock);
    std::lock_guard<std::mutex> lock2(available_tiles_lock);
    for (auto const &tile : tiles) {
        TileData data = available_tiles[tile];
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
    std::cout << "handle GET tile request" << std::endl;
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

        std::cout << "Tile: (" << requested_tile.x << ", " << requested_tile.y << ", " << requested_tile.zoom << ")"
                  << std::endl;

        // Check if this tile is still going to be answered
        /*bool inside_current_region;
        {
            std::lock_guard<std::mutex> lock(current_big_tile_lock);
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
        {
            // Lock important data structures
            // IMPORTANT: same order as everywhere else where we lock those two
            std::lock_guard<std::mutex> lock1(request_dictionary_lock);
            std::lock_guard<std::mutex> lock2(available_tiles_lock);
            auto it_available = available_tiles.find(requested_tile);
            bool answerable = !(it_available == available_tiles.end());
            if (answerable) {
//                std::cout << "answering " << requested_tile.x << ", " << requested_tile.y << ", "
//                          << requested_tile.zoom << std::endl;
                answer_request(request, requested_tile, available_tiles[requested_tile]);
            } else {
                std::cout << "tile not found in available tiles" << std::endl;
                std::cout << "Storing Request at"
                          << " x:" << x
                          << " y:" << y
                          << " z:" << z
                          << " size:" << size << std::endl;

                request_dictionary[requested_tile].push_back(request);
            }
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
    std::cout << "handle GET region request" << std::endl;
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
        RegionOld region{};
        region.tlX = tlX;
        region.tlY = tlY;
        region.brX = brX;
        region.brY = brY;
        region.zoom = zoom;
        region.resX = size;
        region.resY = size;
        region.maxIteration = maxIteration;
        {
            std::lock_guard<std::mutex> lock(current_big_tile_lock);
            if (region == current_big_tile) {
                std::cout << "region has not changed" << std::endl;
            }
            current_big_tile = region;
        }

        // TODO increase by one as soon as host is invoked as worker too
        int nodeCount = Host::world_size - 1;
        // TODO make this based on balancer variable defined above (sounds like strategy pattern...)
        Balancer *b = new IntegerBalancer();
        RegionOld *blocks = b->balanceLoad(region, nodeCount);  //Tiles is array with nodeCount members
        // DEBUG
        std::cout << "Balancer Output:" << std::endl;
        for (int i = 0; i < nodeCount; i++) {
            std::cout << "Region " << i << ": "
                      << " TopLeft: (" << blocks[i].tlX << ", " << blocks[i].tlY << ", " << blocks[i].zoom << ") -> ("
                      << blocks[i].brX << ", " << blocks[i].brY << ", " << blocks[i].zoom << ")" << std::endl;
        }
        // send regions to cores deterministically
        MPI_Request region_requests[nodeCount];
        MPI_Status region_status[nodeCount];
        for (int i = 0; i < nodeCount; i++) {
            RegionOld region = blocks[i];
            int rank_worker = i + 1; // TODO for now, see nodeCount
            std::cout << "Invoking core " << rank_worker << std::endl;
            // Tag for computation requests is 1
            // Send new region
            MPI_Isend((const void *) &region, sizeof(RegionOld), MPI_BYTE, rank_worker, 1, MPI_COMM_WORLD,
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
        for (int i = 0; i < nodeCount; i++) {
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
        {
            std::lock_guard<std::mutex> lock1(request_dictionary_lock);
            std::lock_guard<std::mutex> lock2(current_big_tile_lock);
            for (auto requested_tile = request_dictionary.begin();
                 requested_tile != request_dictionary.end(); requested_tile++) {
                Tile key = requested_tile->first;
                bool inside_current_region = current_big_tile.contains(key.x, key.y,
                                                                       key.x, key.y,
                                                                       key.zoom);
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
    } else {
        std::cerr << "Host: Invalid region request: " << request.to_string()
                  << std::endl;
    }
}

void Host::start_server(){

    //print_server.set_message_handler(&listener_region);

    websocket_server.init_asio();
    websocket_server.start_perpetual();
    websocket_server.listen(9002);
    websocket_server.start_accept();

    websocket_server.set_open_handler(&register_client);
    websocket_server.set_close_handler(&deregister_client);
    websocket_server.set_message_handler(&handle_region_request);

    // How about not logging everything?
    websocket_server.clear_access_channels(websocketpp::log::alevel::all);

    std::cout << "Listening for connections on to websocket server on 9002 \n";
    websocket_server.run();
}

void Host::handle_region_request(const websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg){
    client = hdl;

    std::string request_string = msg->get_payload();

    std::error_code error;
    json::value request = json::value::parse(request_string, error);
    if(error.value() > 0){
        std::cerr << "Error " << error.value() << "on parsing request: " << error.message() << "\n on " << request_string << std::endl;
        return;
    }

    RegionOld region{};
    try{
        utility::string_t balancer = request["balancer"].as_string();
        region.tlX = request["tlX"].as_integer();
        region.tlY = request["tlY"].as_integer();
        region.brX = request["brX"].as_integer();
        region.brY = request["brY"].as_integer();
        region.zoom = request["zoom"].as_integer();
        region.resX = default_res;
        region.resY = default_res;
        region.maxIteration = maxIteration;
    } catch(std::out_of_range e){
        std::cerr << "Inclompletely specified region requested: " << request_string;
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(current_big_tile_lock);
//        if (region == current_big_tile) {
//            std::cerr << "region has not changed" << std::endl;
//            return;
//        }
        current_big_tile = region;
    }

    // TODO increase by one as soon as host is invoked as worker too
    int nodeCount = Host::world_size - 1;
    // TODO make this based on balancer variable defined above (sounds like strategy pattern...)
    Balancer *b = new IntegerBalancer();
    RegionOld *blocks = b->balanceLoad(region, nodeCount);  //Tiles is array with nodeCount members
    // DEBUG
    std::cout << "Balancer Output:" << std::endl;
    for (int i = 0; i < nodeCount; i++) {
        std::cout << "Region " << i << ": "
                    << " TopLeft: (" << blocks[i].tlX << ", " << blocks[i].tlY << ", " << blocks[i].zoom << ") -> ("
                    << blocks[i].brX << ", " << blocks[i].brY << ", " << blocks[i].zoom << ")" << std::endl;
    }
    // send regions to cores deterministically
    MPI_Request region_requests[nodeCount];
    MPI_Status region_status[nodeCount];
    for (int i = 0; i < nodeCount; i++) {
        RegionOld region = blocks[i];
        int rank_worker = i + 1; // TODO for now, see nodeCount
        std::cout << "Invoking core " << rank_worker << std::endl;
        // Tag for computation requests is 1
        // Send new region
        MPI_Isend((const void *) &region, sizeof(RegionOld), MPI_BYTE, rank_worker, 1, MPI_COMM_WORLD,
                    &region_requests[i]);
        transmitted_regions[rank_worker] = region;
    }
    // All workers received their region
    MPI_Waitall(nodeCount, region_requests, region_status);

    json::value reply;
    std::cout << "2";
    reply[U("type")] = json::value::string(U("regions"));
    std::cout << "3";
    reply[U("nregions")] = json::value(nodeCount);
    json::value regions;
    for (int i = 0; i < nodeCount; i++) {
        RegionOld t = blocks[i];
        regions[i] = json::value();
        regions[i][U("nodeID")] = json::value(i);
        regions[i][U("topLeftX")] = json::value(t.tlX);
        regions[i][U("topLeftY")] = json::value(t.tlY);
        regions[i][U("bottomRightX")] = json::value(t.brX);
        regions[i][U("bottomRightY")] = json::value(t.brY);
        regions[i][U("zoom")] = json::value(t.zoom);
    }
    reply[U("regions")] = regions;
    try{
        std::cout << "4";
        websocket_server.send(hdl, reply.serialize().c_str(), websocketpp::frame::opcode::text);
    } catch(websocketpp::exception e){
        std::cerr << "Bad connection to client, Refresh connection" << std::endl;
    }
}

void Host::register_client(const websocketpp::connection_hdl hdl){
    // TODO frontend has to connect a second time during server
    // runtime for hdl not to be a BAD CONN
    // (this means to try the code one has to refresh fast or to set region fetching to manual)
    client = hdl;
}

void Host::deregister_client(websocketpp::connection_hdl hdl){
    // TODO maybe mock client or sth similar
}

void Host::send(TileData data, Tile tile){
    // TODO send data of this region to the client

    // Create json value from returned
    json::value answer;
    answer[U("rank")] = json::value(data.world_rank);
    json::value tile_json = json::value();
    for (unsigned int index = 0; index < (unsigned int) data.size; index++) {
        tile_json[index] = data.n[index];
    }
    answer[U("data")] = tile_json;
    answer[U("type")] = json::value("tile");

    // Include region
    json::value region_json = json::value();
    region_json[U("x")] = tile.x;
    region_json[U("y")] = tile.y;
    //region_json[U("tlX")] = rendered_region.tlX;
    //region_json[U("tlY")] = rendered_region.tlY;
    region_json[U("resX")] = tile.resX;
    region_json[U("resY")] = tile.resY;
    region_json[U("zoom")] = tile.zoom;
    region_json[U("maxIteration")] = tile.maxIteration;
    answer[U("tile")] = region_json;

    utility::string_t data_string = answer.serialize();

    try{
        websocket_server.send(client, data_string.c_str(), websocketpp::frame::opcode::text);
    } catch(websocketpp::exception e){
        std::cerr << e.what() << "\n" 
        << "Refresh websocket connection.\n";
    }
}

void Host::init(int world_rank, int world_size) {
    Host::world_size = world_size;
    int cores = world_size;
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
    resturl.set_path(U("region"));

    http_listener listener_region(resturl.to_uri());
    listener_region.support(methods::GET, handle_get_region);

    try {
        // These are listeners. They start a new thread that calls the methods given
        // a few lines before when a new get request arrives
        // handle_get and handle_get_region are never called directly!
        // They can store requests in a dictionary which we can access later in the MPI_recv loop
        /*listener_region
                .open()
                .then([&listener_region]() { TRACE(U("\nlistening for HTTP Region Requests\n")); })
                .wait();*/

    } catch (std::exception const &e) {
        std::wcout << e.what() << std::endl;
    }

    // Websockets
    // Start a thread that hosts the server
    std::thread websocket_server(start_server);

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
        RegionOld rendered_region{};
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
        /*bool inside_current_region;
        {
            std::lock_guard<std::mutex> lock(current_big_tile_lock);
            inside_current_region = current_big_tile.contains(rendered_region.tlX, rendered_region.tlY,
                                                              rendered_region.brX, rendered_region.brY,
                                                              rendered_region.zoom);

        }
        if (!inside_current_region) {
            std::cout << "Host: no longer interested in the rendered region" << std::endl;
            continue;
        }*/
        //std::cout << rendered_region.resX << ", " << rendered_region.resY << std::endl;

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
        {   
            std::lock_guard<std::mutex> lock(available_tiles_lock);
            for (std::vector<int>::size_type i = 0; i != tiles.size(); i++) {
                Tile t = tiles.at(i);
                Host::send(tile_data.at(i), t);
            }
        }

    }
}