cmake_minimum_required(VERSION 3.10)
project(toddcox-visualize)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(vendor/toddcox)

add_subdirectory(vendor/yaml-cpp)

set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)
set(NANOGUI_USE_GLAD      ON  CACHE BOOL " " FORCE)
add_subdirectory(vendor/nanogui)
set_property(TARGET glfw glfw_objects nanogui PROPERTY FOLDER "dependencies")

add_subdirectory(vis)

add_subdirectory(examples)
