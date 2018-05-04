#pragma once
#include <cpprest/http_listener.h>
#include <map>
#include <queue>
#include <vector>
class Host {
	public:
		static void init(int worldRank, int worldSize);
		// Static da als handle übergeben werden muss
		// => ganzer Rest auch static, lediglich Informationskapselung
		static void handle_get(web::http::http_request request);

		static int maxIteration;
		static double minReal;
		static double maxReal;
		static double minImaginary;
		static double maxImaginary;
		// Das Dictionary, in dem nach x,y,z,fraktalart,size der zugehörige Request aufbewahrt werden
		static std::map<std::vector<int>, web::http::http_request> request_dictionary;
		// Verwaltet die verfügbaren Kerne
		static std::queue<int> avail_cores;		
};


