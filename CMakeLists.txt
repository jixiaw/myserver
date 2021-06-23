cmake_minimum_required(VERSION 2.6)

project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++11")

find_package(Threads)

add_subdirectory(server/base)
add_subdirectory(server/net)
add_subdirectory(examples)
