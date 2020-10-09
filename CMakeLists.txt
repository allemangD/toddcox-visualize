cmake_minimum_required(VERSION 3.10)
project(toddcox-visualize)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(vendor/toddcox)

add_subdirectory(vendor/yaml-cpp)
add_subdirectory(vendor/cgl)
#add_subdirectory(vendor/glfw)

set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)
add_subdirectory(vendor/nanogui)

add_subdirectory(vis)

add_subdirectory(examples)
