add_executable(memo memotest.cpp)
target_link_libraries(memo PRIVATE tc vis-util)

add_executable(geom geomtest.cpp)
target_link_libraries(geom PRIVATE tc vis-util)

