add_custom_target(resources DEPENDS resources_output)
add_custom_command(
    OUTPUT resources_output
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
    COMMENT "Copying Resources")

add_executable(vis
    src/main.cpp
    )
target_link_libraries(vis
    tc::tc
    fmt::fmt
    glfw
    glad
    imgui
    eigen
    nlohmann_json
    )
target_include_directories(vis PUBLIC include)
add_dependencies(vis resources)
