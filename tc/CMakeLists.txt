add_library(tc
    include/tc/groups.hpp
    src/groups.cpp
    include/tc/core.hpp
    src/core.cpp
    src/solve.cpp)

target_include_directories(tc PUBLIC include)
