add_library(tc
    include/tc/core.hpp
    include/tc/groups.hpp

    src/cosets.cpp
    src/group.cpp
    src/groups.cpp
    src/lang.cpp
    src/solve.cpp
    )
target_link_libraries(tc peglib fmt)
target_include_directories(tc PUBLIC include)

add_library(tc::tc ALIAS tc)

add_subdirectory(test)
add_subdirectory(bench)
