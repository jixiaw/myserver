cmake_minimum_required(VERSION 2.6)

project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

find_package(Threads)

add_subdirectory(netjxw/base)
add_subdirectory(netjxw/net)
