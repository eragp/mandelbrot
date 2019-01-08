#!/bin/bash
# Install header only libraries on backend
export http_proxy=proxy.in.tum.de:8080
export https_proxy=proxy.in.tum.de:8080

# websocketpp
echo "Installing websocketpp"
cd ~/.eragp-mandelbrot/install
mkdir websocketpp 2> /dev/null
if [ $? -eq 0 ]
then
    git clone --branch 0.7.0 https://github.com/zaphoyd/websocketpp.git websocketpp --depth 1
else
    echo "Already installed, updating"
    cd websocketpp
    git pull
fi

# rapidjson
echo "Installing rapidjson"
cd ~/.eragp-mandelbrot/install
mkdir rapidjson 2> /dev/null
if [ $? -eq 0 ]
then
    git clone https://github.com/Tencent/rapidjson/ --depth 1
else
    echo "Already installed, updating"
    cd rapidjson
    git pull
fi

# MPI
echo "MPI already installed"
# module load mpi

# ZLIB
echo "libz already installed"