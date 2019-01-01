#!/bin/bash -i
# Build mandelbrot executables

echo "Prepare build"
cd ~/eragp-mandelbrot/backend/build
echo "Activating MPI libs"
module load mpi
cmake ..
echo "Start build"
make