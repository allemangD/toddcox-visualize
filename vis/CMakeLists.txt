add_executable(vis
        src/main.cpp)
target_include_directories(vis PRIVATE include)
target_link_libraries(vis PRIVATE tc glad glm glfw)
