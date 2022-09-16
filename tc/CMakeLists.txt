add_library(tc
    include/tc/pair_map.hpp
    include/tc/core.hpp
    include/tc/cosets.hpp
    include/tc/group.hpp
    include/tc/groups.hpp
    include/tc/util.hpp
    src/core.cpp
    src/groups.cpp)

target_include_directories(tc PUBLIC include)

add_subdirectory(test)
add_subdirectory(bench)
