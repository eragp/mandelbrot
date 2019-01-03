#!/bin/bash
# Install header only libraries on backend
export http_proxy=proxy.in.tum.de:8080
export https_proxy=proxy.in.tum.de:8080

# websocketpp
echo "Installing websocketpp"
cd ~/.eragp-mandelbrot/install
git clone --branch 0.7.0 https://github.com/zaphoyd/websocketpp.git websocketpp --depth 1

# rapidjson
echo "Installing rapidjson"
cd ~/.eragp-mandelbrot/install
git clone https://github.com/Tencent/rapidjson/ --depth 1

# MPI
echo "MPI already installed"
# module load mpi