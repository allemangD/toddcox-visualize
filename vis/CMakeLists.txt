add_subdirectory(shaders)

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
    shaders
    )
target_include_directories(vis PUBLIC include)
