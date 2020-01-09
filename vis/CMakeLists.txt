add_executable(vis src/main.cpp)
target_include_directories(vis PRIVATE include)
target_link_libraries(vis PRIVATE tc glad glm glfw)

add_custom_target(shaders ALL DEPENDS shader_output)
add_custom_command(
        OUTPUT shader_output
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
        COMMENT "copied shaders"
)

add_dependencies(vis shaders)

add_library(vis-util INTERFACE)
target_include_directories(vis-util INTERFACE include)
