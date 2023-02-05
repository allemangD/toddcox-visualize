cmake_minimum_required(VERSION 3.10)
project(toddcox-faster)

set(CMAKE_CXX_STANDARD 20)

file(GLOB externals ext/*.cmake)
foreach (external ${externals})
    include(${external})
endforeach ()

include(cmake/embed.cmake)

add_subdirectory(tc)
add_subdirectory(vis)
