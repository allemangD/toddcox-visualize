cmake_minimum_required(VERSION 3.21)
project(toddcox-visualize)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

include(External/eigen.cmake)
include(External/fmt.cmake)
include(External/glad.cmake)
include(External/glfw.cmake)
include(External/gtest.cmake)
include(External/imgui.cmake)
include(External/json.cmake)
include(External/peglib.cmake)

include_directories(include)

enable_testing()
add_subdirectory(tc)

add_custom_target(resources DEPENDS resources_output)
add_custom_command(
    OUTPUT resources_output
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
    COMMENT "Copying Resources")

add_executable(vis
    src/main.cpp
    src/gl/debug.hpp
    src/gl/shader.hpp
    src/gl/buffer.hpp
    src/gl/vertexarray.hpp
    src/gl/types.hpp)
target_link_libraries(vis glfw glad imgui eigen nlohmann_json)
add_dependencies(vis resources)

#add_executable(serial src/serialtest.cpp)
#target_link_libraries(serial eigen nlohmann_json)

#add_executable(combotest src/combotest.cpp)
#target_link_libraries(combotest eigen tc)

#add_executable(geometrytest src/geometrytest.cpp)
#target_link_libraries(geometrytest eigen tc nlohmann_json)
