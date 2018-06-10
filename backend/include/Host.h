#pragma once
#include <cpprest/http_listener.h>
#include <map>
#include <queue>
#include <vector>
#include "TileInfo.h"
#include <mutex>
class Host {
	public:
		static void init(int worldRank, int worldSize);
	private:
		// Static da als handle übergeben werden muss
		// => ganzer Rest auch static, lediglich Informationskapselung
		static void handle_get_tile(web::http::http_request request);
		static void handle_get_region(web::http::http_request request);
		static void request_more();
		static void printTileInfo(TileInfo tileInfo);
		static void answer_requests(TileInfo tileInfo);

		static int maxIteration;
		static int world_size;
		// Dictionary for storing requests that overlap with the given region 
		static std::map<TileInfo, std::vector<web::http::http_request>> request_dictionary;
		
		// Store for the current big tile
		static TileInfo current_big_tile;
		// And for the split up regions
		static TileInfo *regions;
		
		// Verwaltet die verfügbaren Kerne
		static std::queue<int> avail_cores;		
		
		// Store requests
		static std::queue<TileInfo> requested_tiles;
		// And answers
		static std::map<TileInfo, bool> available_tiles;
		static std::map<TileInfo, int> Host::available_tiles_rank;
		
		//Store send MPI Requests
		static std::map<int, TileInfo> transmitted_tiles;
		
		// Buffer for completed computations
		static int *data_buffer;

		// Synchronise access
		static std::mutex avail_cores_lock;
		static std::mutex requested_tiles_lock;
		static std::mutex transmitted_tiles_lock;
		static std::map<TileInfo, std::mutex> available_tiles_lock;
		static std::mutex data_buffer_lock;
		static std::map<TileInfo, std::mutex> request_dictionary_lock;
};


