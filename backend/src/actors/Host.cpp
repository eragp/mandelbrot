#include "Host.h"

#include "Balancer.h"
#include "BalancerPolicy.h"

#include "Fractal.h"
#include "Mandelbrot.h"

#include "Region.h"
#include "Tile.h"
#include "TileData.h"
#include "WorkerInfo.h"

// MPI Libraries
#include <mpi.h>

// Json
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

// Websockets
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

// Utils
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <algorithm>
#include <vector>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace rapidjson;

const int default_res = 256;
// Init values with some arbitrary value
int Host::maxIteration = 200;
int Host::world_size = 0;
std::vector<int> Host::activeNodes;

// Store for the current big region
Region Host::current_big_region;
std::mutex Host::current_big_region_lock;

// Store send MPI Requests
std::map<int, Region> Host::transmitted_regions;
std::mutex Host::transmitted_regions_lock;

// Websocket server
websocketpp::server<websocketpp::config::asio> Host::websocket_server;
websocketpp::connection_hdl Host::client;

void Host::start_server() {

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

void Host::handle_region_request(const websocketpp::connection_hdl hdl,
                                 websocketpp::server<websocketpp::config::asio>::message_ptr msg) {
    client = hdl;

    std::string request_string = msg->get_payload();

    Document request;
    request.Parse(request_string.c_str());
    /* TODO
        try{
        } catch (ParseErrorCode error){
            std::cerr << "Error " << error. << "on parsing request: " << error.message() << "\n on "
                    << request_string << std::endl;
            return;
        }
        */

    if(std::strcmp(request["type"].GetString(), "regionRequest") != 0){
        return;
    }

    Region region{};
    utility::string_t balancer;
    try {
        balancer = request["balancer"].GetString();

        region.minReal = request["region"]["minReal"].GetDouble();
        region.maxImaginary = request["region"]["maxImag"].GetDouble();

        region.maxReal = request["region"]["maxReal"].GetDouble();
        region.minImaginary = request["region"]["minImag"].GetDouble();

        region.width = request["region"]["width"].GetUint();
        region.height = request["region"]["height"].GetUint();

        region.hOffset = 0;
        region.vOffset = 0;

        region.maxIteration = request["region"]["maxIteration"].GetUint();
        region.validation = request["region"]["validation"].GetInt();
        region.guaranteedDivisor = request["region"]["guaranteedDivisor"].GetUint();
    } catch (std::out_of_range &e) {
        std::cerr << "Inclompletely specified region requested: " << request_string;
        return;
    }

    // DEBUG
    std::cout << "Recieved Region: "
              << " TopLeft: (" << region.minReal << ", " << region.maxImaginary << ") -> BottomRight: ("
              << region.maxReal << ", " << region.minImaginary << ") Resolution: ("
              << region.width << ", " << region.height << ")" << std::endl;

    {
        std::lock_guard<std::mutex> lock(current_big_region_lock);
//        if (region == current_big_region) {
//            std::cerr << "region has not changed" << std::endl;
//            return;
//        }
        current_big_region = region;
    }

    // TODO increase by one as soon as host is invoked as worker too
    int nodeCount = activeNodes.size();
    // TODO let frontend choose fractal similar to balancer
    Balancer *b = BalancerPolicy::chooseBalancer(balancer, new Mandelbrot());
    Region *blocks = b->balanceLoad(region, nodeCount);  // Blocks is array with nodeCount members
    // DEBUG
    std::cout << "Balancer Output:" << std::endl;
    for (int i = 0; i < nodeCount; i++) {
        std::cout << "Region " << i << ": "
                  << " TopLeft: (" << blocks[i].minReal << ", " << blocks[i].maxImaginary << ") -> BottomRight: ("
                  << blocks[i].maxReal << ", " << blocks[i].minImaginary << ") Resolution: ("
                  << blocks[i].width << ", " << blocks[i].height << ")" << std::endl;
    }

    // send regions to cores deterministically
    MPI_Request region_requests[nodeCount];
    MPI_Status region_status[nodeCount];
    for (int i = 0; i < nodeCount; i++) {
        Region small_region = blocks[i];
        int rank_worker = activeNodes.at(i);
        std::cout << "Invoking core " << rank_worker << std::endl;
        // Tag for computation requests is 1
        // Send new region
        // TODO @Tobi you might want to look at this
        {
            std::lock_guard<std::mutex> lock(transmitted_regions_lock);
            transmitted_regions[rank_worker] = small_region;
        }
        MPI_Isend(&small_region, sizeof(Region), MPI_BYTE, rank_worker, 1, MPI_COMM_WORLD,
                  &region_requests[i]);
    }
    // All workers received their region
    MPI_Waitall(nodeCount, region_requests, region_status);

    Document reply;
    reply.SetObject();

    reply.AddMember("type", "region", reply.GetAllocator());
    reply.AddMember("regionCount", nodeCount, reply.GetAllocator());

    Value workers;
    workers.SetArray();
    for (int i = 0; i < nodeCount; i++) {
        Region t = blocks[i];

        Value region;
        region.SetObject();
        region.AddMember("minReal", Value().SetDouble(t.minReal), reply.GetAllocator());
        region.AddMember("maxImag", Value().SetDouble(t.maxImaginary), reply.GetAllocator());

        region.AddMember("maxReal", Value().SetDouble(t.maxReal), reply.GetAllocator());
        region.AddMember("minImag", Value().SetDouble(t.minImaginary), reply.GetAllocator());

        region.AddMember("width", t.width, reply.GetAllocator());
        region.AddMember("height", t.height, reply.GetAllocator());

        region.AddMember("hOffset", t.hOffset, reply.GetAllocator());
        region.AddMember("vOffset", t.vOffset, reply.GetAllocator());

        region.AddMember("maxIteration", t.maxIteration, reply.GetAllocator());
        region.AddMember("validation", t.validation, reply.GetAllocator());
        region.AddMember("guaranteedDivisor", t.guaranteedDivisor, reply.GetAllocator());

        Value entry;
        entry.SetObject();
        entry.AddMember("rank", activeNodes.at(i), reply.GetAllocator());
        entry.AddMember("computationTime", 0, reply.GetAllocator());
        entry.AddMember("region", region, reply.GetAllocator());

        workers.PushBack(entry, reply.GetAllocator());
    }
    reply.AddMember("regions", workers, reply.GetAllocator());
    // Stringify reply
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    reply.Accept(writer);
    try {
        websocket_server.send(hdl, buffer.GetString(), websocketpp::frame::opcode::text);
    } catch (websocketpp::exception &e) {
        std::cerr << "Bad connection to client, Refresh connection" << std::endl;
    }
}

void Host::register_client(const websocketpp::connection_hdl hdl) {
    // TODO frontend has to connect a second time during server
    // runtime for hdl not to be a BAD CONN
    // (this means to try the code one has to refresh fast or to set region fetching to manual)
    client = hdl;
}

void Host::deregister_client(websocketpp::connection_hdl hdl) {
    // TODO maybe mock client or sth similar
}

void Host::send(RegionData data) {
    WorkerInfo workerInfo = data.workerInfo;
    Region region = data.workerInfo.region;

    // Create json value from returned
    Document answer(kObjectType);
    // answer[U("rank")] = json::value(data.world_rank);
    Value rawDataJSON(kArrayType);
    for (unsigned int index = 0; index < (region.width * region.height); index++) {
        rawDataJSON.PushBack(data.data[index], answer.GetAllocator());
    }

    Value workerInfoJSON(kObjectType);
    workerInfoJSON.AddMember("rank", workerInfo.rank, answer.GetAllocator());
    workerInfoJSON.AddMember("computationTime", Value().SetInt64(workerInfo.computationTime), answer.GetAllocator());

    // Maybe put this into extra method
    Value regionJSON;
    regionJSON.SetObject();
    regionJSON.AddMember("minReal", Value().SetDouble(region.minReal), answer.GetAllocator());
    regionJSON.AddMember("maxImag", Value().SetDouble(region.maxImaginary), answer.GetAllocator());

    regionJSON.AddMember("maxReal", Value().SetDouble(region.maxReal), answer.GetAllocator());
    regionJSON.AddMember("minImag", Value().SetDouble(region.minImaginary), answer.GetAllocator());

    regionJSON.AddMember("width", region.width, answer.GetAllocator());
    regionJSON.AddMember("height", region.height, answer.GetAllocator());

    regionJSON.AddMember("hOffset", region.hOffset, answer.GetAllocator());
    regionJSON.AddMember("vOffset", region.vOffset, answer.GetAllocator());

    regionJSON.AddMember("maxIteration", region.maxIteration, answer.GetAllocator());
    regionJSON.AddMember("validation", region.validation, answer.GetAllocator());
    regionJSON.AddMember("guaranteedDivisor", region.guaranteedDivisor, answer.GetAllocator());

    workerInfoJSON.AddMember("region", regionJSON, answer.GetAllocator());

    answer.AddMember("workerInfo", workerInfoJSON, answer.GetAllocator());
    answer.AddMember("data", rawDataJSON, answer.GetAllocator());
    answer.AddMember("type", "regionData", answer.GetAllocator());

    // Stringify reply
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    answer.Accept(writer);

    try {
        websocket_server.send(client, buffer.GetString(), websocketpp::frame::opcode::text);
    } catch (websocketpp::exception &e) {
        std::cerr << e.what() << "\n" << "Refresh websocket connection.\n";
    }
}

void Host::init(int world_rank, int world_size) {
    MPI_Errhandler_set(MPI_COMM_WORLD,MPI_ERRORS_RETURN); /* return info about errors */

    Host::world_size = world_size;
    int cores = world_size;
    int my_rank = world_rank;
    std::cout << "Host init " << world_size << std::endl;

    // Websockets
    // Start a thread that hosts the server
    std::thread websocket_server(start_server);

    // Test if all cores are available
    // We assume from programs side that all cores *are* available, this allows Workers to get the Host rank
    activeNodes = std::vector<int>();
    for (int i = 0; i < cores; i++) {
        if(i != my_rank){
            int testSend = i;
            MPI_Send((const void *) &testSend, 1, MPI_INT, i, 10, MPI_COMM_WORLD);
            int testReceive;
            MPI_Recv(&testReceive, 1, MPI_INT, i, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (testSend == testReceive) {
                std::cout << "Core " << i << " ready!" << std::endl;
            }
            activeNodes.push_back(i);
        }
    }

    // TODO @Tobi you might want to look at this    
    // MPI - receiving answers and answering requests
    while (true) {
        // Listen for incoming complete computations from workers (MPI)
        // This accepts messages by the workers and answers requests that were stored before
        // TODO: asynchronous (maybe?)
        WorkerInfo workerInfo;
        MPI_Recv((void *) &workerInfo, sizeof(WorkerInfo), MPI_BYTE, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);  // 3 is tag for WorkerInfo
        Region rendered_region{};
        {
            std::lock_guard<std::mutex> lock(transmitted_regions_lock);
            rendered_region = transmitted_regions[workerInfo.rank];
        }

        unsigned int region_size = rendered_region.getPixelCount();
        int* worker_data = new int[region_size];
        std::cout << "Host: waiting for receive from " << workerInfo.rank << ": " << region_size << std::endl;
        
        int ierr = MPI_Recv(worker_data, region_size, MPI_INT, workerInfo.rank, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // 2 is tag for raw data
        if(ierr != MPI_SUCCESS){
            std::cerr << "Error on receiving data from worker: " << std::endl;
            char err_buffer[MPI_MAX_ERROR_STRING];
            int resultlen;
            MPI_Error_string(ierr,err_buffer,&resultlen);
            fprintf(stderr,err_buffer);
            continue;
        }

        std::cout << "Host: received from " << workerInfo.rank << std::endl;

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
        }*/
        //std::cout << rendered_region.resX << ", " << rendered_region.resY << std::endl;

        // copy the received data from worker into regionData struct
        /*auto *worker_array = new int[region_size];
        int i = 0;
        for (unsigned int y = 0; y < rendered_region.height; y++) {
            for (unsigned int x = 0; x < rendered_region.width; x++) {
                worker_array[i] = worker_data[i];
                i++;
            }
        }*/
        RegionData region_data{};
        region_data.data = worker_data;
        region_data.data_length = region_size;
        region_data.workerInfo = workerInfo;

        Host::send(region_data);
        delete[] worker_data;
    }

}