#include "Host.h"

#include <fstream>
#include <iostream>
#include "Balancer.h"
#include "Fractal.h"
#include "ColumnBalancer.h"
#include "RegionOld.h"
#include "Region.h"
#include "Tile.h"
#include "TileData.h"

// MPI Libraries
#include <mpi.h>

// Cpp REST libraries
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

// Store for the current big region
Region Host::current_big_region;
std::mutex Host::current_big_region_lock;

// Store send MPI Requests
std::map<int, Region> Host::transmitted_regions;
std::mutex Host::transmitted_regions_lock;

// Websocket server
websocketpp::server<websocketpp::config::asio> Host::websocket_server;
websocketpp::connection_hdl Host::client;

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

	Region region;
    try{
        utility::string_t balancer = request["balancer"].as_string();
        /*region.tlX = request["tlX"].as_integer();
        region.tlY = request["tlY"].as_integer();
        region.brX = request["brX"].as_integer();
        region.brY = request["brY"].as_integer();
        region.zoom = request["zoom"].as_integer();*/
        
        // TODO talk to Max about protocol for websocket communication
        region.minReal = -1.5; // request["minReal"].as_double();
        region.maxImaginary = 1.0; // request["maxImag"].as_double();
        
        region.maxReal = 0.7; // request["maxReal"].as_double();
        region.minImaginary = -1.0; // request["minImag"].as_double();

        region.width = default_res; // request["width"].as_integer();
        region.height = default_res; // request["height"].as_integer();

        region.hOffset = 0;
        region.vOffset = 0;

        region.maxIteration = maxIteration; // request["iterations"].as_integer();
        region.validation = 0; // request["validation"].as_integer();
        region.guaranteedDivisor = 8; // request["divisor"].as_integer();
    } catch(std::out_of_range e){
        std::cerr << "Inclompletely specified region requested: " << request_string;
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(current_big_region_lock);
//        if (region == current_big_region) {
//            std::cerr << "region has not changed" << std::endl;
//            return;
//        }
        current_big_region = region;
    }

    // TODO increase by one as soon as host is invoked as worker too
    int nodeCount = Host::world_size - 1;
    // TODO make this based on balancer variable defined above (sounds like strategy pattern...) --> That was the idea :)
    Balancer *b = new ColumnBalancer();
    Region *blocks = b->balanceLoad(region, nodeCount);  // Blocks is array with nodeCount members
    // DEBUG
    std::cout << "Balancer Output:" << std::endl;
    for (int i = 0; i < nodeCount; i++) {
        std::cout << "Region " << i << ": "
                    << " TopLeft: (" << blocks[i].minReal << ", " << blocks[i].maxImaginary << ") -> BottomRight: ("
                    << blocks[i].maxReal << ", " << blocks[i].minImaginary << ") Resolution: ("
                    << blocks[i].width << ", " << blocks[i].height << std::endl;
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
        // TODO @Tobi you might want to look at this
        MPI_Isend((const void *) &region, sizeof(Region), MPI_BYTE, rank_worker, 1, MPI_COMM_WORLD,
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
        Region t = blocks[i];
        regions[i] = json::value();
        regions[i][U("nodeID")] = json::value(i);
        // regions[i][U("topLeftX")] = json::value(t.tlX);
        // regions[i][U("topLeftY")] = json::value(t.tlY);
        // regions[i][U("bottomRightX")] = json::value(t.brX);
        // regions[i][U("bottomRightY")] = json::value(t.brY);
        // regions[i][U("zoom")] = json::value(t.zoom);
        
        // TODO talk to Max about protocol for websocket communication
        regions[i][U("minReal")] = json::value((double) t.minReal);
        regions[i][U("maxImag")] = json::value((double) t.maxImaginary);

        regions[i][U("maxReal")] = json::value((double) t.maxReal);
        regions[i][U("minImag")] = json::value((double) t.minImaginary);

        regions[i][U("width")] = json::value(t.width);
        regions[i][U("height")] = json::value(t.height);

        regions[i][U("hOffset")] = json::value(t.hOffset);
        regions[i][U("vOffset")] = json::value(t.vOffset);

        regions[i][U("iterations")] = json::value(t.maxIteration);
        regions[i][U("validation")] = json::value(t.validation);
        regions[i][U("divisor")] = json::value(t.guaranteedDivisor);
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

    // TODO @Tobi you might want to look at this    
    // MPI - receiving answers and answering requests
    while (true) {
        /*
        // Listen for incoming complete computations from workers (MPI)
        // This accepts messages by the workers and answers requests that were stored before
        // TODO: asynchronous (maybe?)
        int worker_rank;
        MPI_Recv((void *) &worker_rank, 1, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);  // 3 is tag for world_rank of worker
        Region rendered_region;
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
            std::lock_guard<std::mutex> lock(current_big_region_lock);
            inside_current_region = current_big_region.contains(rendered_region.tlX, rendered_region.tlY,
                                                              rendered_region.brX, rendered_region.brY,
                                                              rendered_region.zoom);

        }
        if (!inside_current_region) {
            std::cout << "Host: no longer interested in the rendered region" << std::endl;
            continue;
        }* /
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
        for (std::vector<int>::size_type i = 0; i != tiles.size(); i++) {
            Tile t = tiles.at(i);
            Host::send(tile_data.at(i), t);
        }
        */
       
       // Only to make the processor stay calm
       // IMPORTANT: remove when above is finished
       usleep(10000);
    }
    
}