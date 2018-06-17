#ifndef HOST_H
#define HOST_H

#include <cpprest/http_listener.h>
#include <map>
#include <queue>
#include <vector>
#include <mutex>

#include "Region.h"
#include "Tile.h"
#include "TileData.h"

class Host {
public:
    static void init(int worldRank, int worldSize);

private:
    // Static da als handle übergeben werden muss
    // => ganzer Rest auch static, lediglich Informationskapselung
	
	static void handle_get_tile(web::http::http_request request);

    static void handle_get_region(web::http::http_request request);

    static void answer_requests(Region requested_tile);

    static int maxIteration;
    static int world_size;
    // Dictionary for storing requests for a given tile
    static std::map<Tile, std::vector<web::http::http_request>> request_dictionary;
    static std::map<Tile, std::mutex> request_dictionary_lock;

    // Store for the current big tile
    static Region current_big_tile;
    static std::mutex big_tile_lock;
    // And for the split up regions

    // Keeps track of all available cores
    static std::queue<int> avail_cores;
    static std::mutex avail_cores_lock;

    // Store requests
    static std::queue<Region> requested_regions;
    static std::mutex requested_regions_lock;

    // And answers
    static std::map<Tile, TileData> available_tiles;
    static std::map<Tile, std::mutex> available_tiles_lock;

    //Store send MPI Requests
    static std::map<int, Region> transmitted_regions;
    static std::mutex transmitted_regions_lock;

};
#endif

