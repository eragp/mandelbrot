#ifndef HOST_H
#define HOST_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/extensions/permessage_deflate/enabled.hpp>

#include <map>
#include <queue>
#include <vector>
#include <mutex>

#include "Region.h"
#include "RegionData.h" 

struct deflate_server_config : public websocketpp::config::asio {
	// ... additional custom config if you need it for other things

	/// permessage_compress extension
    struct permessage_deflate_config {};

    typedef websocketpp::extensions::permessage_deflate::enabled
        <permessage_deflate_config> permessage_deflate_type;
};

typedef websocketpp::server<deflate_server_config> server_endpoint_type;

class Host {
public:
    static void init(int worldRank, int worldSize);

private:
    // Static, as currently method handles are passed without bind

    static int maxIteration;
    static int world_size;
    
    // Defines if a Node can or should be used
    static bool* usable_nodes;
    static int usable_nodes_count;

    // Store for the current big region
    static Region current_big_region;
    static std::mutex current_big_region_lock;
    // And for the split up regions

    // Transfer region requests from Websocket-Thread to MPI-Thread
    static bool mpi_send_regions;
    static std::vector<Region> websocket_request_to_mpi;
    static std::mutex websocket_request_to_mpi_lock;

    // Transfer RegionData from MPI-Thread to Websocket-Result-Thread
    static std::vector<RegionData> mpi_to_websocket_result;
    static std::mutex mpi_to_websocket_result_lock;

    // Websocket server
    static server_endpoint_type websocket_server;
    static void start_server();
    static websocketpp::connection_hdl client;
    static void register_client(websocketpp::connection_hdl conn);
    static void deregister_client(websocketpp::connection_hdl conn);
    static void send();
    static void handle_region_request(websocketpp::connection_hdl hdl, server_endpoint_type::message_ptr msg);
};
#endif

