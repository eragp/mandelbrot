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
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

target_link_libraries(mandelbrot PUBLIC cpprestsdk::cpprest ${MPI_LIBRARIES})
set(CMAKE_CXX_STANDARD 11)
# lib configs
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -lboost_system -lcrypto -lssl")
# custom flags for dependencies
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WEBSOCKETPP_CPP11_STL_")

if (MPI_COMPILE_FLAGS)
    set_target_properties(mandelbrot PROPERTIES
            COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif ()

if (MPI_LINK_FLAGS)
    set_target_properties(mandelbrot PROPERTIES
            LINK_FLAGS "${MPI_LINK_FLAGS}")
endif ()
