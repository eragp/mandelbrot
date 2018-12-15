#include "Host.h"

#include "Balancer.h"
#include "BalancerPolicy.h"

#include "Fractal.h"
#include "MandelbrotOpt.h"

#include "Region.h"
#include "Tile.h"
#include "TileData.h"
#include "WorkerInfo.h"

// MPI Libraries
#include <mpi.h>

// Json
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

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
#include <chrono>
#include <thread>

using namespace rapidjson;

const int default_res = 256;
// Init values with some arbitrary value
int Host::maxIteration = 200;
int Host::world_size = 0;
std::vector<int> Host::activeNodes;

// Store for the current big region
Region Host::current_big_region;
std::mutex Host::current_big_region_lock;

// Transfer region requests from Websocket-Thread to MPI-Thread
bool Host::mpi_send_regions = false;
std::map<int, Region> Host::transmit_regions;
std::mutex Host::transmit_regions_lock;

// Store send MPI Requests
std::map<int, Region> Host::transmitted_regions;
std::mutex Host::transmitted_regions_lock;

// Websocket server
websocketpp::server<websocketpp::config::asio> Host::websocket_server;
websocketpp::connection_hdl Host::client;

void Host::start_server() {

    std::cout << "Start Server" << std::endl;

    //print_server.set_message_handler(&listener_region);

    websocket_server.init_asio();
    // std::cout << "Run perpetual\n";
    websocket_server.start_perpetual();
    // std::cout << "Set listener\n";
    // Use IPv4 explicitely
    websocket_server.listen(websocketpp::lib::asio::ip::tcp::v4(), 9002);
    // std::cout << "Start accepting\n";
    websocket_server.start_accept();

    // std::cout << "Set open handler\n";
    websocket_server.set_open_handler(&register_client);
    // std::cout << "Set close handler\n";
    websocket_server.set_close_handler(&deregister_client);
    // std::cout << "Set message handler\n";
    websocket_server.set_message_handler(&handle_region_request);

    // How about not logging everything?
    websocket_server.clear_access_channels(websocketpp::log::alevel::all);

    std::cout << "Listening for connections on to websocket server on 9002 \n";
    websocket_server.run();
}

void Host::handle_region_request(const websocketpp::connection_hdl hdl,
                                 websocketpp::server<websocketpp::config::asio>::message_ptr msg) {
    std::cout << "Handle Region Request" << std::endl;
    
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
    const char* balancer;
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
    Balancer *b = BalancerPolicy::chooseBalancer(balancer, new MandelbrotOpt());
    Region *blocks = b->balanceLoad(region, nodeCount);  // Blocks is array with nodeCount members
    // DEBUG
    std::cout << "Balancer Output:" << std::endl;
    for (int i = 0; i < nodeCount; i++) {
        std::cout << "Region " << i << ": "
                  << " TopLeft: (" << blocks[i].minReal << ", " << blocks[i].maxImaginary << ") -> BottomRight: ("
                  << blocks[i].maxReal << ", " << blocks[i].minImaginary << ") Resolution: ("
                  << blocks[i].width << ", " << blocks[i].height << ")" << std::endl;
    }

    // Send regions to MPI-Thread
    {
        std::lock_guard<std::mutex> lock(transmit_regions_lock);
        for (int i = 0 ; i < nodeCount ; i++) {
            Region small_region = blocks[i];
            int rank_worker = activeNodes.at(i);
            transmit_regions[rank_worker] = small_region;
        }
        mpi_send_regions = true;
    }
    std::cout << "Transmit angefordert" << std::endl;

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
    MPI_Comm_set_errhandler(MPI_COMM_WORLD,MPI_ERRORS_RETURN); /* return info about errors */
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
                std::cout << "Host: Core " << i << " ready!" << std::endl;
            }
            activeNodes.push_back(i);
        }
    }

    // Approximately the time that MPI communication with one Worker has taken in microseconds
    std::chrono::high_resolution_clock::time_point *mpiCommunicationStart = new std::chrono::high_resolution_clock::time_point[activeNodes.size()];
    unsigned long *mpiCommunicationTime = new unsigned long[activeNodes.size()];

    // Init persistent asynchronous send
    MPI_Request region_requests[activeNodes.size()];
    MPI_Status region_status[activeNodes.size()];
    for (int i = 0 ; i < (int) activeNodes.size() ; i++) {
        int rank_worker = activeNodes.at(i);
        MPI_Send_init(&transmit_regions[rank_worker], sizeof(Region), MPI_BYTE, rank_worker, 1, MPI_COMM_WORLD, &region_requests[i]);
    }
  
    // MPI communication between Host and Workers (send region requests and receive computed data) and start websocket send
    while (true) {
        // Send regions to cores deterministically using persistent asynchronous send
        {
            std::lock_guard<std::mutex> lock(transmit_regions_lock);
            if (mpi_send_regions == true) {
                int nodeCount = activeNodes.size();
                for (int i = 0 ; i<nodeCount ; i++) {
                    int rank_worker = activeNodes.at(i);
                    std::cout << "Host: Start invoking core " << rank_worker << std::endl;
                    // Start the clock for MPI communication
                    mpiCommunicationStart[i] = std::chrono::high_resolution_clock::now();
                    // Start send to one Worker using persistent asynchronous send
                    MPI_Start(&region_requests[i]);
                    {
                        std::lock_guard<std::mutex> lock(transmitted_regions_lock);
                        transmitted_regions[rank_worker] = transmit_regions[rank_worker];
                    }
                }
                std::cout << "Host: Start invoking all cores done." << std::endl;
                // Wait to complete all send operations
                MPI_Waitall(nodeCount, region_requests, region_status);
                std::cout << "Host: Waitall returned. All send operations are complete." << std::endl;
                mpi_send_regions = false;
            }
        }
        
        // Listen for incoming complete computations from workers (MPI)
        // Receive one message of dynamic length containing "workerInfo" and the computed "worker_data"
        // TODO: asynchronous (maybe?)
        MPI_Status status;
        int probe_flag;
        MPI_Iprobe(MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &probe_flag, &status);
        if (probe_flag == true) { // Rank: 2
            probe_flag = 0;
            int recv_len;
            MPI_Get_count(&status, MPI_BYTE, &recv_len);
            std::cout << "Host is receiving Data from Worker " << status.MPI_SOURCE << " Total length: " << recv_len << " Bytes." << std::endl;
            uint8_t* recv = new uint8_t[recv_len];
            int ierr = MPI_Recv(recv, recv_len, MPI_BYTE, status.MPI_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // Rank: 2
            if(ierr != MPI_SUCCESS){
                std::cerr << "Error on receiving data from worker: " << std::endl;
                char err_buffer[MPI_MAX_ERROR_STRING];
                int resultlen;
                MPI_Error_string(ierr, err_buffer, &resultlen);
                fprintf(stderr, err_buffer);
                continue;
            }

            // Stop the clock for MPI communication
            std::chrono::high_resolution_clock::time_point mpiCommunicationEnd = std::chrono::high_resolution_clock::now();

            // Extract "workerInfo" from the received message
            WorkerInfo workerInfo;
            std::memcpy(&workerInfo, recv, sizeof(WorkerInfo));
            Region rendered_region{};
            {
                std::lock_guard<std::mutex> lock(transmitted_regions_lock);
                rendered_region = transmitted_regions[workerInfo.rank];
            }
            unsigned int region_size = rendered_region.getPixelCount();

            // Compute time approximately used for MPI communication
            mpiCommunicationTime[workerInfo.rank - 1] = std::chrono::duration_cast<std::chrono::microseconds>(mpiCommunicationEnd - mpiCommunicationStart[workerInfo.rank - 1]).count() - workerInfo.computationTime;
            std::cout << "Host: MPI communication with Worker " << workerInfo.rank << " took approximately " << mpiCommunicationTime[workerInfo.rank - 1] << " microseconds." << std::endl;
        
            // Extract "worker_data" from the received message
            int* worker_data = new int[region_size];
            std::memcpy(worker_data, recv + sizeof(WorkerInfo), region_size * sizeof(int));
            std::cout << "Host: Receive from Worker " << workerInfo.rank << " complete." << std::endl;
        
            // Fill "region_data"
            RegionData region_data{};
            region_data.data = worker_data;
            region_data.data_length = region_size;
            region_data.workerInfo = workerInfo;

            Host::send(region_data);

            delete[] recv;
            delete[] worker_data;
        }
        
    }

}