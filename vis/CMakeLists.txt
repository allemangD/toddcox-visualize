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

add_definitions(${NANOGUI_EXTRA_DEFS})
include_directories(${NANOGUI_EXTRA_INCS})

add_executable(vis-gui src/main-gui.cpp)
target_include_directories(vis-gui PRIVATE include)
target_link_libraries(vis-gui PRIVATE tc nanogui yaml-cpp ${NANOGUI_EXTRA_LIBS})
add_dependencies(vis-gui shaders presets)
