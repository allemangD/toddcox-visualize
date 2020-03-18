add_custom_target(shaders ALL DEPENDS shader_output)
add_custom_command(
        OUTPUT shader_output
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
        COMMENT "copied shaders"
)

add_custom_target(presets ALL DEPENDS preset_output)
add_custom_command(
        OUTPUT preset_output
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/presets ${CMAKE_CURRENT_BINARY_DIR}/presets
        COMMENT "copied preses"
)

find_package(yaml-cpp REQUIRED)

add_library(vis-util INTERFACE)
target_include_directories(vis-util INTERFACE include)

add_executable(vis src/main.cpp)
target_include_directories(vis PRIVATE include)
target_link_libraries(vis PRIVATE tc glad glm glfw yaml-cpp)
add_dependencies(vis shaders presets)

add_executable(ctest src/combotest.cpp)
target_link_libraries(ctest PRIVATE yaml-cpp)
target_include_directories(ctest PRIVATE include)
add_dependencies(ctest presets)
