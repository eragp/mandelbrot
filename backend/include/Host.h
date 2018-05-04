#pragma once
#include <cpprest/json.h>
#include <cpprest/http_listener.h>
#include <map>
#include <queue>
typedef struct {
	int start_x;	//included
	int start_y;	//included
	int size;
	int fractal; // For later, identify fractal type
} ID;

class Host
{
public:
	Host(int worldRank, int worldSize);
private:
    void handle_get();
    int cores;
	int height;
	int width;
	int maxIteration;
	double minReal;
	double maxReal;
	double minImaginary;
	double maxImaginary;
    // Das Dictionary, in dem nach x,y,z,fraktalart,size der zugehörige Request aufbewahrt werden
	map<ID, web::http::http_request> dictionary;
	// Verwaltet die verfügbaren Kerne
	queue<int> avail_cores;
};

