cmake_minimum_required(VERSION 3.0)
project(POOL)
file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable((app ${SRC}))