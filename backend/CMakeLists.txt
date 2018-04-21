cmake_minimum_required(VERSION 3.7.0)

project(Mandelbrot)
# compiler flags
# load the header files
include_directories(include)
# load all the *.cpp files
file(GLOB_RECURSE SOURCES "src/*.cpp")
set(TARGET_NAME mandelbrot)
add_executable(mandelbrot ${SOURCES})
# load libraries
find_package(cpprestsdk REQUIRED)
target_link_libraries(mandelbrot PUBLIC cpprestsdk::cpprest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lboost_system -lcrypto -lssl -lccprest")

