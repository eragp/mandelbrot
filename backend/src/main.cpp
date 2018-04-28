#include <fstream>
#include <iostream>
#include "Fractal.h"
#include "Mandelbrot.h"

// Teile diese Codes zur Erzeugung eines Servers sind Abwandlungen oder Kopien
// von
// http://mariusbancila.ro/blog/2017/11/19/revisited-full-fledged-client-server-example-with-c-rest-sdk-2-10/
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

void handle_get(http_request request) {
  TRACE(L"\nhandle GET\n");

  request.reply(status_codes::OK, "Hello");
}

double xToReal(int x, double maxReal, double minReal, int width) {
  return x * ((maxReal - minReal) / width) + minReal;
}

double yToImaginary(int y, double maxImaginary, double minImaginary,
                    int height) {
  return y * ((maxImaginary - minImaginary) / height) + minImaginary;
}

int main() {
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

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int n = f->calculateFractal(
          xToReal(x, maxReal, minReal, width),
          yToImaginary(y, maxImaginary, minImaginary, height), maxIteration);
      int r = 0;
      int g = 0;
      int b = 0;
      if (n != maxIteration) {
        r = n * 10 % 256;
        g = n * 20 % 256;
        b = n * 40 % 256;
      }
      fout << r << " " << g << " " << b << " ";
    }
    fout << endl;
  }

  fout.close();
  free(f);
  cout << "\a"
       << "Fertig!" << endl;
//   cin.get();

  // Kommunikationsteil
  /*
   * IMPORTANT: use the U() Makro for any strings passed to cpprest. This is
   * required for Linux compilation
   */
  http_listener listener(U("http://localhost/restdemo"));

  listener.support(methods::GET, handle_get);

  try {
    listener.open()
        .then([&listener]() { TRACE(U("\nstarting to listen\n")); })
        .wait();

    while (true)
      ;
  } catch (exception const &e) {
    wcout << e.what() << endl;
  }
  return 0;
}
