cmake_minimum_required(VERSION 3.10)
project(toddcox-faster)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(vendor/toddcox)

add_subdirectory(vendor/yaml-cpp)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)

add_subdirectory(vis)

add_subdirectory(examples)
