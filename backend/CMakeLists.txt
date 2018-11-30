cmake_minimum_required(VERSION 3.7.0)

project(Mandelbrot)
# compiler flags
# load the header files
include_directories(include ~/try_install/local/include ~/try_install/install/websocketpp ~/try_install/install/rapidjson/include)
# load all the *.cpp files
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*main\.cpp")
#set(TARGET_NAME mandelbrot)
add_executable(host "src/host.main.cpp" ${SOURCES})
add_executable(worker "src/worker.main.cpp" ${SOURCES})
# load libraries
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

# link boost statically
ADD_LIBRARY(boost_system STATIC IMPORTED)
SET_TARGET_PROPERTIES(boost_system PROPERTIES
    IMPORTED_LOCATION $ENV{HOME}/try_install/local/lib/libboost_system.a)
# links boost and MPI
target_link_libraries(host PUBLIC ${MPI_LIBRARIES} boost_system)
target_link_libraries(worker PUBLIC ${MPI_LIBRARIES} boost_system)
set(CMAKE_CXX_STANDARD 11)
# lib configs
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -lpthread")

# custom flags for dependencies
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WEBSOCKETPP_CPP11_STL_")

if (MPI_COMPILE_FLAGS)
    set_target_properties(host worker PROPERTIES
            COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif ()

if (MPI_LINK_FLAGS)
    set_target_properties(host worker PROPERTIES
            LINK_FLAGS "${MPI_LINK_FLAGS}")
endif ()
