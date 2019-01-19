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
int Host::usable_nodes_count;

// Store for the current big region
Region Host::current_big_region;
std::mutex Host::current_big_region_lock;

// Transfer region requests from Websocket-Request-Thread to MPI-Thread
bool Host::mpi_send_regions = false;
std::vector<Region> Host::websocket_request_to_mpi;
std::mutex Host::websocket_request_to_mpi_lock;

// Transfer RegionData from MPI-Thread to Websocket-Result-Thread
std::vector<RegionData> Host::mpi_to_websocket_result;
std::mutex Host::mpi_to_websocket_result_lock;

// Websocket server
server_endpoint_type Host::websocket_server;
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

    int regionCount = 0;
    Region region{};

    const char* balancer;
    int predictionAccuracy = 4; // Remove standard value, when implemented in frontend

    const char* fractal_str;
    Fractal* fractal_bal = nullptr;
    try {
        balancer = request["balancer"].GetString();
        predictionAccuracy = request["predictionAccuracy"].GetInt();
        std::cout << "Chose prediction Accuracy: " << predictionAccuracy << std::endl;

        enum fractal_type fractal_type;
        fractal_str = request["fractal"].GetString();
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
        }
        else if(boost::iequals(fractal_str, "mandelbrotopenmp32")){
            fractal_type = mandelbrotOpenMP32;
            fractal_bal = new Mandelbrot32();
        }
        else if(boost::iequals(fractal_str, "mandelbrotopenmp64")){
            fractal_type = mandelbrotOpenMP64;
            fractal_bal = new Mandelbrot64();
        }
        else if(boost::iequals(fractal_str, "mandelbrotopenmpsimd32")){
            fractal_type = mandelbrotOpenMPSIMD32;
            fractal_bal = new Mandelbrot32();
        }
        else if(boost::iequals(fractal_str, "mandelbrotopenmpsimd64")){
            fractal_type = mandelbrotOpenMPSIMD64;
            fractal_bal = new Mandelbrot64();
        }
        else if(boost::iequals(fractal_str, "mandelbrotopenmp")){
            fractal_type = mandelbrotOpenMP;
            fractal_bal = new Mandelbrot();
        } else {
            std::cerr << "Inclompletely specified region requested: " << request_string;
            return;
        }
        // Reproducible and equivalent balancing for all implementation choices
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
        region.fractal = fractal_type;

        regionCount = request["nodes"].GetInt();

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

    // Set regionCount correctly if value is erroneous
    if (regionCount > usable_nodes_count || regionCount <= 0) {
        regionCount = usable_nodes_count;
        std::cout << "RegionCount was erroneous. Set to " << regionCount << " to prevent errors." << std::endl;
    } else {
        std::cout << "RegionCount is " << regionCount << std::endl;
    }

    Balancer *b = BalancerPolicy::chooseBalancer(balancer, predictionAccuracy, fractal_bal);
    // Measure time needed for balancing - Start
    std::chrono::high_resolution_clock::time_point balancerTimeStart = std::chrono::high_resolution_clock::now();
    // Call balanceLoad
    Region *blocks = b->balanceLoad(region, regionCount);  // Blocks is array with regionCount members
    // Measure time needed for balancing - End
    std::chrono::high_resolution_clock::time_point balancerTimeEnd = std::chrono::high_resolution_clock::now();
    unsigned long balancerTime = std::chrono::duration_cast<std::chrono::microseconds>(balancerTimeEnd - balancerTimeStart).count();
    std::cout << "Balancing took " << balancerTime << " microseconds." << std::endl;

    // DEBUG
    std::cout << "Balancer Output:" << std::endl;
    for (int i = 0; i < regionCount; i++) {
        std::cout << "Region " << i << ": "
                  << " TopLeft: (" << blocks[i].minReal << ", " << blocks[i].maxImaginary << ") -> BottomRight: ("
                  << blocks[i].maxReal << ", " << blocks[i].minImaginary << ") Resolution: ("
                  << blocks[i].width << ", " << blocks[i].height << ")" << std::endl;
    }

    // do not Delete empty subregions - they are idleing too!
    std::cout << "There are " << regionCount << " Regions to compute" << std::endl;

    // Determine which Worker gets which Region
    int region_to_worker[regionCount];
    int counter = 0;
    for (int rank = 0 ; rank < world_size && counter < regionCount ; rank++) {
        if (usable_nodes[rank] == true) {
            std::cout << "Region " << counter << " will be computed by Worker " << rank << std::endl;
            region_to_worker[counter] = rank;
            counter ++;
        }
    }

    Document reply;
    reply.SetObject();

    reply.AddMember("type", "region", reply.GetAllocator());
    reply.AddMember("regionCount", regionCount, reply.GetAllocator());
    reply.AddMember("balancerTime", Value().SetInt64(balancerTime), reply.GetAllocator());

    Value workers;
    workers.SetArray();
    for (int i = 0; i < regionCount; i++) {
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
        region.AddMember("fractal", Value().SetString(fractal_str, strlen(fractal_str)), reply.GetAllocator());
        region.AddMember("regionCount", regionCount, reply.GetAllocator());

        Value entry;
        entry.SetObject();
        entry.AddMember("rank", region_to_worker[i], reply.GetAllocator());
        entry.AddMember("computationTime", -1, reply.GetAllocator());
        entry.AddMember("mpiTime", -1, reply.GetAllocator());
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

    // Send regions to MPI-Thread *after* we have sent the division
    {
        std::lock_guard<std::mutex> lock(websocket_request_to_mpi_lock);
        websocket_request_to_mpi.clear();
        for (int i = 0 ; i < regionCount; i++) {
            // Store fractal in region
            blocks[i].regionCount = regionCount;
            websocket_request_to_mpi.push_back(blocks[i]);
        }
        mpi_send_regions = true;
    }
    std::cout << "Sending Region division" << std::endl;
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
        workerInfoJSON.AddMember("mpiTime", Value().SetInt64(data.mpiCommunicationTime), answer.GetAllocator());

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
        // Add fractal
        const char* fractal_str;
        switch(region.fractal){
            case mandelbrot32:
                fractal_str = "mandelbrot32";
                break;
            case mandelbrot64:
                fractal_str = "mandelbrot64";
                break;
            case mandelbrotSIMD32:
                fractal_str = "mandelbrotSIMD32";
                break;
            case mandelbrotSIMD64:
                fractal_str = "mandelbrotSIMD64";
                break;
            default:
                fractal_str = "mandelbrot";
        }
        regionJSON.AddMember("fractal", Value().SetString(fractal_str, strlen(fractal_str)), answer.GetAllocator());
        regionJSON.AddMember("regionCount", region.regionCount, answer.GetAllocator());

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
    usable_nodes_count = world_size - 1;

    // Test if all cores are available
    // This also allows the Workers to get the rank of the Host
    MPI_Request test_requests[usable_nodes_count];
    MPI_Status test_status[usable_nodes_count];
    // Send test messages to every Worker
    for (int rank = 0 ; rank < world_size ; rank++) {
        if (rank != world_rank) {
            // Skip Host
            int acc = 0;
            if (rank > world_rank) {
                acc = 1;
            }
            int test_send = rank;
            int ierr = MPI_Issend((const void *) &test_send, 1, MPI_INT, rank, 10, MPI_COMM_WORLD, &test_requests[rank - acc]);
            // Error handling
            if (ierr != MPI_SUCCESS){
                std::cerr << "Error on MPI_Issend to worker " << rank << " on test send: " << std::endl;
                char err_buffer[MPI_MAX_ERROR_STRING];
                int resultlen;
                MPI_Error_string(ierr, err_buffer, &resultlen);
                fprintf(stderr, err_buffer);
            }
            // Error handling - end
        }
    }
    // Start timer
    std::chrono::high_resolution_clock::time_point test_time_start = std::chrono::high_resolution_clock::now();
    unsigned long test_time = 0;
    // Check if every Worker has started its receive operation
    int *array_of_indices = new int[usable_nodes_count];
    do {
        int outcount;
        int ierr = MPI_Testsome(usable_nodes_count, test_requests, &outcount, array_of_indices, test_status);
        // Error handling
        if (ierr != MPI_SUCCESS){
            for (int i = 0 ; i < usable_nodes_count ; i++) {
                ierr = test_status[i].MPI_ERROR;
                if (ierr != MPI_SUCCESS) {
                    std::cerr << "Error on MPI_Testsome with index " << i << " on test send: " << std::endl;
                    char err_buffer[MPI_MAX_ERROR_STRING];
                    int resultlen;
                    MPI_Error_string(ierr, err_buffer, &resultlen);
                    fprintf(stderr, err_buffer);
                }
            }
        }
        // Error handling - end
        // Every send operation was successful
        if (outcount == MPI_UNDEFINED) {
            std::cout << "Tests successful. Break test loop." << std::endl;
            break;
        }
        // One or more send operations were successful. Set their MPI_Request to MPI_REQUEST_NULL
        if (outcount != 0) {
            for (int i = 0 ; i < outcount ; i++) {
                test_requests[array_of_indices[i]] = MPI_REQUEST_NULL;
            }
        }
        // Check how much time has passed
        std::chrono::high_resolution_clock::time_point test_time_end = std::chrono::high_resolution_clock::now();
        test_time = std::chrono::duration_cast<std::chrono::microseconds>(test_time_end - test_time_start).count();
    } while (test_time < 1000000);
    delete[] array_of_indices;
    // Update usable_nodes and usable_nodes_count according to the test results
    for (int rank = 0 ; rank < world_size ; rank++) {
        if (rank != world_rank) {
            // Skip Host.
            int acc = 0;
            if (rank > world_rank) {
                acc = 1;
            }
            if (test_requests[rank - acc] != MPI_REQUEST_NULL) {
                usable_nodes[rank] = false;
                usable_nodes_count--;
                // Cancel uncompleted send operations
                MPI_Cancel(&test_requests[rank - acc]);
                MPI_Status cancel_status;
                MPI_Wait(&test_requests[rank - acc], &cancel_status);
                int cancel_flag;
                MPI_Test_cancelled(&cancel_status, &cancel_flag);
                if (cancel_flag == true) {
                    std::cout << "Host: Worker " << rank << " has NOT started his receive operation. This Worker is NOT usable. Cancel was successful." << std::endl;
                } else {
                     std::cout << "Host: Worker " << rank << " has started his receive operation to late. This Worker is NOT usable. Cancel was NOT successful." << std::endl;
                }
            } else {
                usable_nodes[rank] = true;
                std::cout << "Host: Worker " << rank << " is usable." << std::endl;
            }
        }
    }
    std::cout << "Host: There are " << usable_nodes_count << " usable Worker." << std::endl;
    // Test if all cores are available - end

    // Approximately the time that MPI communication with one Worker has taken in microseconds
    std::chrono::high_resolution_clock::time_point *mpiCommunicationStart = new std::chrono::high_resolution_clock::time_point[world_size];

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
        MPI_Status status;
        int probe_flag;
        // Check if it is possible to receive a finished computation
        MPI_Iprobe(MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &probe_flag, &status); // Tag: 2
        if (probe_flag == true) {
            int recv_len;
            // Determine the length of the incoming message
            MPI_Get_count(&status, MPI_BYTE, &recv_len);
            std::cout << "Host is receiving Data from Worker " << status.MPI_SOURCE << " Total length: " << recv_len << " Bytes." << std::endl;
            // Allocate the receive buffer
            uint8_t* recv = new uint8_t[recv_len];
            // Execute the actual receive operation
            int ierr = MPI_Recv(recv, recv_len, MPI_BYTE, status.MPI_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // Tag: 2
            // Error handling
            if(ierr != MPI_SUCCESS){
                std::cerr << "Error on receiving data from worker: " << std::endl;
                char err_buffer[MPI_MAX_ERROR_STRING];
                int resultlen;
                MPI_Error_string(ierr, err_buffer, &resultlen);
                fprintf(stderr, err_buffer);
                continue;
            }
            // Error handling - end

            // Stop the clock for MPI communication
            std::chrono::high_resolution_clock::time_point mpiCommunicationEnd = std::chrono::high_resolution_clock::now();

            // Extract "workerInfo" from the received message
            WorkerInfo workerInfo;
            std::memcpy(&workerInfo, recv, sizeof(WorkerInfo));

            // Compute time approximately used for MPI communication
            unsigned long mpiCommunicationTime = std::chrono::duration_cast<std::chrono::microseconds>(mpiCommunicationEnd - mpiCommunicationStart[workerInfo.rank]).count() - workerInfo.computationTime;
            std::cout << "Host: MPI communication with Worker " << workerInfo.rank << " took approximately " << mpiCommunicationTime << " microseconds." << std::endl;
        
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
            region_data.mpiCommunicationTime = mpiCommunicationTime;

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