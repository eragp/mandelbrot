#!/bin/bash

echo "Starting the Build Process"

docker build -t mandelbrot:0.1 .
docker run  \
	-d -p 4000:80 mandelbrot:0.1 \
    ./t mandelbrot.out

# echo "Finished the Build Process"
