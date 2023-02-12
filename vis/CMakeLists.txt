add_subdirectory(shaders)

add_custom_target(presets ALL DEPENDS preset_output)
add_custom_command(
        OUTPUT preset_output
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/presets ${CMAKE_CURRENT_BINARY_DIR}/presets
        COMMENT "copied preses"
)

add_executable(vis
    src/main.cpp
    src/components.hpp
    )
target_include_directories(vis PRIVATE include)
target_link_libraries(vis PRIVATE
    tc shaders
    glad eigen glfw yaml-cpp fmt EnTT imgui
    )
add_dependencies(vis presets)
