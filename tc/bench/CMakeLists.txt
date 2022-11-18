add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark PUBLIC tc fmt::fmt)

add_executable(named named.cpp)
target_link_libraries(named PUBLIC tc fmt::fmt)
