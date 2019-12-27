cmake_minimum_required(VERSION 3.10)
project(tc)

set(CMAKE_CXX_STANDARD 17)

add_library(tc STATIC src/groups.cpp src/solver.cpp)
target_include_directories(tc PUBLIC include)

add_subdirectory(example)
