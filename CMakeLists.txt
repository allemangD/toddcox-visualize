cmake_minimum_required(VERSION 3.10)
project(toddcox-faster)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(vendor/glad)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)

add_subdirectory(tc)
add_subdirectory(vis)
add_subdirectory(example)

message("${GLM_TEST_ENABLE}")