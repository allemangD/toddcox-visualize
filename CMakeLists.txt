cmake_minimum_required(VERSION 3.21)
project(toddcox-visualize)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

include(External/eigen.cmake)
include(External/fmt.cmake)
include(External/glad.cmake)
include(External/glfw.cmake)
include(External/gtest.cmake)
include(External/imgui.cmake)
include(External/json.cmake)
include(External/peglib.cmake)

include(embed.cmake)

enable_testing()
add_subdirectory(tc)
add_subdirectory(vis)
