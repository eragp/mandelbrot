#ifndef HOST_H
#define HOST_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <cpprest/http_listener.h>

#include <map>
#include <queue>
#include <vector>
#include <mutex>

#include "RegionOld.h"
#include "Tile.h"
#include "TileData.h"

class Host {
public:
    static void init(int worldRank, int worldSize);

private:
    // Static, as currently method handles are passed without bind

    static int maxIteration;
    static int world_size;

    // Store for the current big tile
    static RegionOld current_big_tile;
    static std::mutex current_big_tile_lock;
    // And for the split up regions

    //Store send MPI Requests
    static std::map<int, RegionOld> transmitted_regions;
    static std::mutex transmitted_regions_lock;

    // Websocket server
    static websocketpp::server<websocketpp::config::asio> websocket_server;
    static void start_server();
    static websocketpp::connection_hdl client;
    static void register_client(websocketpp::connection_hdl conn);
    static void deregister_client(websocketpp::connection_hdl conn);
    static void send(TileData data, Tile tile);
    static void handle_region_request(websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::asio>::message_ptr msg);
};
#endif

