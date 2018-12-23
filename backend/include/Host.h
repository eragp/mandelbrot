#ifndef HOST_H
#define HOST_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <map>
#include <queue>
#include <vector>
#include <mutex>

#include "Region.h"
#include "RegionData.h"

class Host {
public:
    static void init(int worldRank, int worldSize);

private:
    // Static, as currently method handles are passed without bind

    static int maxIteration;
    static int world_size;
    
    // Defines if a Node can or should be used
    static bool* usableNodes;

    // Store for the current big region
    static Region current_big_region;
    static std::mutex current_big_region_lock;
    // And for the split up regions

    // Transfer region requests from Websocket-Thread to MPI-Thread
    static bool mpi_send_regions;
    static std::map<int, Region> transmit_regions;
    static std::mutex transmit_regions_lock;

    // Transfer RegionData from MPI-Thread to Websocket-Result-Thread
    static std::vector<RegionData> mpi_to_websocket_result;
    static std::mutex mpi_to_websocket_result_lock;

    // Websocket server
    static websocketpp::server<websocketpp::config::asio> websocket_server;
    static void start_server();
    static websocketpp::connection_hdl client;
    static void register_client(websocketpp::connection_hdl conn);
    static void deregister_client(websocketpp::connection_hdl conn);
    static void send();
    static void handle_region_request(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg);
};
#endif

