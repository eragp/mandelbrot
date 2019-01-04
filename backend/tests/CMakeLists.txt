cmake_minimum_required(VERSION 3.7.0)
set(CMAKE_CXX_STANDARD 11)

project(TestBalancer)

include_directories(../include)

file(GLOB_RECURSE SOURCES "../src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*\.main\.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*init\.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*/actors/.*\.cpp")

add_executable(testbalancer "TestBalancer.cpp" ${SOURCES})