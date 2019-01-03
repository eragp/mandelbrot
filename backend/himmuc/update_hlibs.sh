#!/bin/bash
# Install header only libraries on backend
export http_proxy=proxy.in.tum.de:8080
export https_proxy=proxy.in.tum.de:8080

# websocketpp
echo "Updating websocketpp"
cd ~/.eragp-mandelbrot/install/websocketpp
git pull

# rapidjson
echo "Updating rapidjson"
cd ~/.eragp-mandelbrot/install/rapidjson
git pull