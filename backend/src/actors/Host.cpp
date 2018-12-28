#include "Host.h"

#include "Balancer.h"
#include "BalancerPolicy.h"

#include "Fractal.h"
#include "Mandelbrot.h"
#include "Mandelbrot32.h"
#include "Mandelbrot64.h"
#include "MandelbrotSIMD32.h"
#include "MandelbrotSIMD64.h"

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

#include <boost/algorithm/string.hpp>

using namespace rapidjson;

const int default_res = 256;
// Init values with some arbitrary value
int Host::maxIteration = 200;
int Host::world_size = 0;

// Defines if a Node can or should be used
bool* Host::usable_nodes;

// Store for the current big region
Region Host::current_big_region;
std::mutex Host::current_big_region_lock;

// Transfer region requests from Websocket-Request-Thread to MPI-Thread
bool Host::mpi_send_regions = false;
std::map<int, Region> Host::websocket_request_to_mpi;
std::mutex Host::websocket_request_to_mpi_lock;

// Transfer RegionData from MPI-Thread to Websocket-Result-Thread
std::vector<RegionData> Host::mpi_to_websocket_result;
std::mutex Host::mpi_to_websocket_result_lock;

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
    if(request.HasParseError()){
        auto error = request.GetParseError();
        std::cerr << "Error " << error << " on parsing request\n on "
                << request_string << std::endl;
        return;
    }

    if(std::strcmp(request["type"].GetString(), "regionRequest") != 0){
        return;
    }

    Region region{};
    const char* balancer;
    enum fractal_type fractal_type;
    Fractal* fractal_bal = nullptr;
    try {
        balancer = request["balancer"].GetString();
        const char * fractal_str = request["fractal"].GetString();
        // Case insensitive compares (just convenience for frontend devs)
        if(boost::iequals(fractal_str, "mandelbrot32")){
            fractal_type = mandelbrot32;
            fractal_bal = new Mandelbrot32();
        }
        else if(boost::iequals(fractal_str, "mandelbrot64")){
            fractal_type = mandelbrot64;
            fractal_bal = new Mandelbrot64();
        }
        else if(boost::iequals(fractal_str, "mandelbrotsimd32")){
            fractal_type = mandelbrotSIMD32;
            fractal_bal = new Mandelbrot32();
        }
        else if(boost::iequals(fractal_str, "mandelbrotsimd64")){
            fractal_type = mandelbrotSIMD64;
            fractal_bal = new Mandelbrot64();
        }
        else if(boost::iequals(fractal_str, "mandelbrot")){
            fractal_type = mandelbrot;
            fractal_bal = new Mandelbrot();
        } else {
            std::cerr << "Inclompletely specified region requested: " << request_string;
            return;
        }
        std::cout << "Host: chose fractal " << fractal_str << std::endl;

        region.minReal = request["region"]["minReal"].GetDouble();
        region.maxImaginary = request["region"]["maxImag"].GetDouble();

        region.maxReal = request["region"]["maxReal"].GetDouble();
        region.minImaginary = request["region"]["minImag"].GetDouble();

        region.width = request["region"]["width"].GetUint();
        region.height = request["region"]["height"].GetUint();

        region.hOffset = 0;
        region.vOffset = 0;

        region.maxIteration = (unsigned short int) request["region"]["maxIteration"].GetUint();
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
        current_big_region = region;
    }

    int nodeCount = world_size - 1;
    Balancer *b = BalancerPolicy::chooseBalancer(balancer, fractal_bal);
    Region *blocks = b->balanceLoad(region, nodeCount);  // Blocks is array with nodeCount members
    // DEBUG
    std::cout << "Balancer Output:" << std::endl;
    for (int i = 0; i < nodeCount; i++) {
        std::cout << "Region " << i << ": "
                  << " TopLeft: (" << blocks[i].minReal << ", " << blocks[i].maxImaginary << ") -> BottomRight: ("
                  << blocks[i].maxReal << ", " << blocks[i].minImaginary << ") Resolution: ("
                  << blocks[i].width << ", " << blocks[i].height << ")" << std::endl;
    }

    // Delete empty subregions
    std::vector<Region> newBlocks;
    for (int i = 0 ; i < nodeCount ; i++) {
        if ((blocks[i].minReal == blocks[i].maxReal && blocks[i].maxImaginary == blocks[i].minImaginary) || blocks[i].width == 0 || blocks[i].height == 0) {
            std::cout << "Empty Region " << i << " deleted." << std::endl;
        } else {
            newBlocks.push_back(blocks[i]);
        }
    }
    blocks = &newBlocks[0];
    nodeCount = newBlocks.size();
    std::cout << "There are " << nodeCount << " Regions to compute" << std::endl;

    // Debug
    for (int i = 0; i < nodeCount; i++) {
        std::cout << "Region " << i << ": "
                  << " TopLeft: (" << blocks[i].minReal << ", " << blocks[i].maxImaginary << ") -> BottomRight: ("
                  << blocks[i].maxReal << ", " << blocks[i].minImaginary << ") Resolution: ("
                  << blocks[i].width << ", " << blocks[i].height << ")" << std::endl;
    }

    // Send regions to MPI-Thread
    {
        std::lock_guard<std::mutex> lock(websocket_request_to_mpi_lock);
        websocket_request_to_mpi.clear();
        for (int i = 0 ; i < nodeCount ; i++) {
            blocks[i].fractal = fractal_type;
            websocket_request_to_mpi[i] = blocks[i];
        }
        mpi_send_regions = true;
    }
    std::cout << "Sending Region division" << std::endl;

    // Determine which Worker gets which Region
    int region_to_worker[nodeCount];
    int counter = 0;
    for (int rank = 0 ; rank < world_size && counter < nodeCount ; rank++) {
        if (usable_nodes[rank] == true) {
            std::cout << "Region " << counter << " will be computed by Worker " << rank << std::endl;
            region_to_worker[counter] = rank;
            counter ++;
        }
    }

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
        entry.AddMember("rank", region_to_worker[i], reply.GetAllocator());
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

void Host::send() {
    std::cout << "Host: Websocket-Result-Thread is running." << std::endl;

    while (true) { 
        RegionData data;
        
        // Check if there is something to send. If not, sleep to reduce processor usage.
        {
            std::lock_guard<std::mutex> lock(mpi_to_websocket_result_lock);
            if (mpi_to_websocket_result.size() > 0) {
                data = mpi_to_websocket_result.front();
                mpi_to_websocket_result.erase(mpi_to_websocket_result.begin());
                std::cout << "Host Websocket-Result-Thread: There are still " << mpi_to_websocket_result.size() << " elements left in the vector." << std::endl;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
        }

        // Extract WorkerInfo and Region
        WorkerInfo workerInfo = data.workerInfo;
        Region region = data.workerInfo.region;

        std::cout << "Host Websocket-Result-Thread: Start to send RegionData from Worker " << workerInfo.rank << "." << std::endl;

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

        delete[] data.data;

        std::cout << "Host Websocket-Result-Thread: Sending RegionData from Worker " << workerInfo.rank << " done." << std::endl;

    }
}

void Host::init(int world_rank, int world_size) {
    MPI_Comm_set_errhandler(MPI_COMM_WORLD,MPI_ERRORS_RETURN); /* return info about errors */
    Host::world_size = world_size;
    std::cout << "Host init " << world_size << std::endl;

    // Websockets
    // Start a thread that hosts the server
    std::thread websocket_server(start_server);

    // Start Websocket-Result-Thread (sends RegionData filled with computed mandelbrot data to frontend)
    std::thread websocket_result(send);

    // Init usable_nodes and set Host as not usable
    usable_nodes = new bool[world_size];
    usable_nodes[world_rank] = false;

    // Test if all cores are available
    // We assume from programs side that all cores *are* available, this allows Workers to get the Host rank
    for (int rank = 0; rank < world_size; rank++) {
        if(rank != world_rank){
            int testSend = rank;
            MPI_Send((const void *) &testSend, 1, MPI_INT, rank, 10, MPI_COMM_WORLD);
            int testReceive;
            MPI_Recv(&testReceive, 1, MPI_INT, rank, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (testSend == testReceive) {
                std::cout << "Host: Core " << rank << " ready!" << std::endl;
                usable_nodes[rank] = true;
            } else {
                std::cout << "Host: Core " << rank << " NOT ready! This core won't be used by default." << std::endl;
                usable_nodes[rank] = false;
            }
        }
    }

    // TODO: ENTFERNEN
    // usable_nodes[2] = false;

    // Approximately the time that MPI communication with one Worker has taken in microseconds
    std::chrono::high_resolution_clock::time_point *mpiCommunicationStart = new std::chrono::high_resolution_clock::time_point[world_size];
    unsigned long *mpiCommunicationTime = new unsigned long[world_size];

    // Init persistent asynchronous send. Each process gets his own Request, Status and Buffer
    MPI_Request region_requests[world_size];
    MPI_Status region_status[world_size];
    Region persistent_send_buffer[world_size];
    for (int rank = 0 ; rank < world_size ; rank++) {
        MPI_Send_init(&persistent_send_buffer[rank], sizeof(Region), MPI_BYTE, rank, 1, MPI_COMM_WORLD, &region_requests[rank]);
    }
  
    // MPI communication between Host and Workers (send region requests and receive computed data) and start websocket send
    while (true) {
        // Send regions to cores deterministically using persistent asynchronous send
        {
            std::lock_guard<std::mutex> lock(websocket_request_to_mpi_lock);
            if (mpi_send_regions == true) {
                unsigned int transmit_counter = 0;
                for (int rank = 0 ; rank < world_size && transmit_counter < websocket_request_to_mpi.size() ; rank++) {
                    if (usable_nodes[rank] == true) {
                        std::cout << "Host: Start invoking core " << rank << std::endl;
                        // Copy requested Region from joint datastructure "websocket_request_to_mpi" to MPI Send buffer "persistent_send_buffer"
                        std::memcpy(&persistent_send_buffer[rank], &websocket_request_to_mpi[transmit_counter++], sizeof(Region));
                        // Start the clock for MPI communication
                        mpiCommunicationStart[rank] = std::chrono::high_resolution_clock::now();
                        // Start send to one Worker using persistent asynchronous send
                        MPI_Start(&region_requests[rank]);
                    }
                }
                if (transmit_counter != websocket_request_to_mpi.size()) {
                    std::cerr << "Not enough Workers to compute all subregions." << std::endl;
                }
                std::cout << "Host: Start invoking all cores done." << std::endl;
                // Wait to complete all send operations
                MPI_Waitall(world_size, region_requests, region_status);
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
            int ierr = MPI_Recv(recv, recv_len, MPI_BYTE, status.MPI_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // Tag: 2
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

            // Compute time approximately used for MPI communication
            mpiCommunicationTime[workerInfo.rank-1] = std::chrono::duration_cast<std::chrono::microseconds>(mpiCommunicationEnd - mpiCommunicationStart[workerInfo.rank]).count() - workerInfo.computationTime;
            std::cout << "Host: MPI communication with Worker " << workerInfo.rank << " took approximately " << mpiCommunicationTime[workerInfo.rank] << " microseconds." << std::endl;
        
            // Extract "worker_data" from the received message
            unsigned int region_size = workerInfo.region.getPixelCount();
            unsigned short int* worker_data = new unsigned short int[region_size];
            std::memcpy(worker_data, recv + sizeof(WorkerInfo), region_size * sizeof(unsigned short int));
            std::cout << "Host: Receive from Worker " << workerInfo.rank << " complete." << std::endl;
        
            // Fill "RegionData"
            RegionData region_data{};
            region_data.data = worker_data;
            region_data.data_length = region_size;
            region_data.workerInfo = workerInfo;

            // Send RegionData to Websocket-Result-Thread
            {
                std::lock_guard<std::mutex> lock(mpi_to_websocket_result_lock);
                mpi_to_websocket_result.push_back(region_data);
            }

            delete[] recv;
            // Note: worker_data will be deleted by Websocket-Result-Thread after the send operation is complete.
        }
        
    }

}