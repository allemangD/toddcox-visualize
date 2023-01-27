cmake_minimum_required(VERSION 3.10)
project(toddcox-faster)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
include(vendor/fmt.cmake)
include(vendor/peglib.cmake)
include(vendor/glad.cmake)
include(vendor/glfw.cmake)
include(vendor/glm.cmake)
include(vendor/gtest.cmake)
include(vendor/yaml-cpp.cmake)

include(vendor/embed.cmake)

add_subdirectory(tc)
add_subdirectory(vis)

add_subdirectory(examples)
