#include <fstream>
#include <iostream>
#include "Fractal.h"
#include "Mandelbrot.h"
//#include "server_ws.hpp"

// MPI Libraries
#include <mpi.h>

// Teile diese Codes zur Erzeugung eines Servers sind Abwandlungen oder Kopien von http://mariusbancila.ro/blog/2017/11/19/revisited-full-fledged-client-server-example-with-c-rest-sdk-2-10/
// Cpp REST libraries
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <map>
#include <set>
#include <string>

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

#define TRACE(msg) wcout << msg
#define TRACE_ACTION(a, k, v) wcout << a << L" (" << k << L", " << v << L")\n"
map<utility::string_t, utility::string_t> dictionary;

int world_rank;
json::value imagebuf = json::value::array();

void handle_get(http_request request)
{
	TRACE(L"\nhandle GET\n");
	// TODO initiate computation

	// TODO wait until computation is finished

	auto response = http_response();
	response.set_status_code(status_codes::OK);
	response.headers().add(U("Access-Control-Allow-Origin"), U("localhost:3000"));
	response.headers().add(U("Access-Control-Allow-Methods"), U("GET"));

	// Expect a data map string->string with x, y and z coordinate
	auto data = uri::split_query(request.request_uri().query());
	map<utility::string_t, utility::string_t>::iterator itx = data.find(U("x"));
	map<utility::string_t, utility::string_t>::iterator ity = data.find(U("y"));

	// Returns either value at x/y or the whole array
	// TODO correctly assign x/y to areas of the mandelbrot set
	// TODO access the buffer of computed tiles to choose the correct one
	if (itx != data.end() && ity != data.end()) {
		int x = stoi(data[U("x")]);
		int y = stoi(data[U("y")]);

		// Hard-coded height, need some communication here, wish to avoid global vars
		// => lambda?
		json::value tile = json::value::array();
		for(int x1 = 0; x1 < 256; x1++){
			for(int y1 = 0; y1 < 256; y1++){
				if(x+x1+(y+y1)*2048 < 0  || x+x1+(y+y1)*2048 > 2048*2048){
					continue;
				}
				tile[x1+y1*256] = imagebuf[x+x1+(y+y1)*2048];
			}
		}
		response.set_body(tile);
	}
	else {
		response.set_body(imagebuf);
	}

	request.reply(response);
}


double xToReal(int x, double maxReal, double minReal, int width)
{
	return x * ((maxReal - minReal) / width) + minReal;
}

double yToImaginary(int y, double maxImaginary, double minImaginary,
                    int height) {
  return y * ((maxImaginary - minImaginary) / height) + minImaginary;
}

int run()
{
	// Hauptprozessor führt diesen Prozess durch
	if(world_rank == 0){
		// Berechnungsteil
			int height = 2048;
			int width = 2048;
			int maxIteration = 200;
			double minReal = -1.5;
			double maxReal = 0.7;
			double minImaginary = -1.0;
			double maxImaginary = 1.0;
			Fractal *f = new Mandelbrot();

			ofstream fout("Mandelbrot.ppm");
			fout << "P3" << endl;
			fout << width << " " << height << endl;
			fout << "256" << endl;

			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					int n = f->calculateFractal(xToReal(x, maxReal, minReal, width), yToImaginary(y, maxImaginary, minImaginary, height), maxIteration);
					int r = 0;
					int g = 0;
					int b = 0;
					if (n != maxIteration)
					{
						r = n * 10 % 256;
						g = n * 20 % 256;
						b = n * 40 % 256;
					}
					
					imagebuf[x+y*height] = n;

					fout << r << " " << g << " " << b << " ";
				}
				fout << endl;
			}

		fout.close();
		//free(f);
		cout << "\a"
			<< "Fertig!" << endl;
		//cin.get();

		// Kommunikationsteil


		// WebSocket
			// Tutorial for i.e. image transimission
			// https://github.com/Microsoft/cpprestsdk/wiki/Web-Socket
			// Cpp REST seems to only support WS clients!
		// Maybe this will work some day
		auto wsurl = web::uri_builder();
		wsurl.set_host(U("0.0.0.0"));
		wsurl.set_scheme(U("ws"));
		wsurl.set_port(U("1234"));
		
		

		// REST 
		/*
		* IMPORTANT: use the U() Makro for any strings passed to cpprest. This is required for Linux compilation
		*/
		auto resturl = web::uri_builder();
		resturl.set_host(U("0.0.0.0"));
		resturl.set_scheme(U("http"));
		resturl.set_port(U("80"));
		resturl.set_path(U("mandelbrot"));

		http_listener listener(resturl.to_uri());
		listener.support(methods::GET, handle_get);


		try
		{
			listener
				.open()
				.then([&listener]() { TRACE(U("\nstarting to listen\n")); })
				.wait();

			while (true){
			}
		}
		catch (exception const &e)
		{
			wcout << e.what() << endl;
		}
	}
	return 0;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    char processorname[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processorname, &name_len);

    cout << processorname << ", Size: " << world_size << ", Rank: " << world_rank << endl;
    run();

    MPI_Finalize();
    return 0;
}