cmake_minimum_required(VERSION 3.10)
project(toddcox-faster)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

include(vendor/toddcox.cmake)
include(vendor/yaml-cpp.cmake)
include(vendor/glad.cmake)
include(vendor/glfw.cmake)
include(vendor/glm.cmake)

add_subdirectory(vis)

add_subdirectory(examples)
