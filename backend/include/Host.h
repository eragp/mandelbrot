#pragma once
#include <cpprest/http_listener.h>
#include <map>
#include <queue>
#include <vector>
#include "Tile.h"
#include <mutex>
class Host {
	public:
		static void init(int worldRank, int worldSize);
	private:
		// Static da als handle übergeben werden muss
		// => ganzer Rest auch static, lediglich Informationskapselung
		static void handle_get(web::http::http_request request);
		static void request_more();

		static int maxIteration;
		// Das Dictionary, in dem nach (x,y,z,fraktalart,size) der zugehörige Request aufbewahrt werden
		// Queue von requests, damit mehrere Requests auf denselben bereich nicht überlappen
		static std::map<std::vector<int>, std::queue<web::http::http_request>> request_dictionary;
		// Verwaltet die verfügbaren Kerne
		static std::queue<int> avail_cores;		
		static std::queue<Tile> requested_tiles;
		// Synchronisiert den Zugriff darauf
		static std::mutex avail_cores_lock;
		static std::mutex requested_tiles_lock;
		static std::map<std::vector<int>, std::mutex> request_dictionary_lock;
};


