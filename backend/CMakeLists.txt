cmake_minimum_required(VERSION 3.7.0)

project(Mandelbrot)
# load the header files
include_directories(include)
# load all the *.cpp files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# set(CMAKE_CXX_STANDARD 11)

add_executable(mandelbrot.out ${SOURCES})
