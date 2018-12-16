#!/bin/bash
# Build mandelbrot executables

echo "Activating MPI libs"
module load mpi

echo "Prepare build"
cd ~/eragp-mandelbrot/backend/build
cmake ..
echo "Start build"
make