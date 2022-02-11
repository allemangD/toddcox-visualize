cmake_minimum_required(VERSION 3.21)
project(toddcox-visualize)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

include(External/glfw.cmake)
include(External/imgui.cmake)
include(External/eigen.cmake)
include(External/glad.cmake)
include(External/json.cmake)

add_executable(vis main.cpp)
target_link_libraries(vis glfw glad imgui eigen nlohmann_json)

add_executable(serial serialtest.cpp)
target_link_libraries(serial eigen nlohmann_json)
