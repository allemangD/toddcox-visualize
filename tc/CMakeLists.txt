add_library(tc STATIC
        src/groups.cpp
        src/solver.cpp
        src/cosets.cpp)

target_include_directories(tc PUBLIC include)
