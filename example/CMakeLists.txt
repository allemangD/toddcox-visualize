add_executable(bench bench.cpp)
target_link_libraries(bench PRIVATE tc)

add_executable(path path.cpp)
target_link_libraries(path PRIVATE tc)
