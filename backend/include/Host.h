#pragma once
#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <map>
#include <queue>
#include <vector>
class Host
{
public:
	Host(int worldRank, int worldSize);
	void handle_get(web::http::http_request request);
private:
    int cores;
	int height;
	int width;
	int maxIteration;
	double minReal;
	double maxReal;
	double minImaginary;
	double maxImaginary;
    // Das Dictionary, in dem nach x,y,z,fraktalart,size der zugehörige Request aufbewahrt werden
	std::map<std::vector<int>, web::http::http_request> dictionary;
	// Verwaltet die verfügbaren Kerne
	std::queue<int> avail_cores;
};

