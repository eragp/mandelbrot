#!/bin/bash

echo "Starting the Build Process"

## start the cmake build
cd build
cmake ..
make
cd ..

docker build -t mandelbrot:0.1 .
docker run  \
	-d -p 4000:80 mandelbrot:0.1 \
    ./t mandelbrot.out

# echo "Finished the Build Process"
