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

		static int maxIteration;
		static int world_size;
		// Das Dictionary, in dem nach (x,y,z,fraktalart,size) der zugehörige Request aufbewahrt werden
		// Queue von requests, damit mehrere Requests auf denselben bereich nicht überlappen
		static std::map<std::vector<int>, std::queue<web::http::http_request>> request_dictionary;
		
		// Store for the current big tile
		static TileInfo current_big_tile;
		
		// Verwaltet die verfügbaren Kerne
		static std::queue<int> avail_cores;		
		
		// Store requests
		static std::queue<TileInfo> requested_tiles;
		
		//Store send MPI Requests
		static std::map<int, TileInfo> transmitted_tiles;
		
		// Buffer for completed computations
		static int *data_buffer;

		// Synchronisiert den Zugriff darauf
		static std::mutex avail_cores_lock;
		static std::mutex requested_tiles_lock;
		static std::mutex transmitted_tiles_lock;
		static std::mutex data_buffer_lock;
		static std::map<std::vector<int>, std::mutex> request_dictionary_lock;
};


