add_library(tc
    include/tc/core.hpp
    include/tc/cosets.hpp
    include/tc/group.hpp
    include/tc/groups.hpp
    include/tc/lang.hpp
    include/tc/pair_map.hpp
    include/tc/util.hpp
    src/core.cpp
    src/groups.cpp
    src/lang.cpp
    )
target_link_libraries(tc PUBLIC peglib::peglib fmt::fmt)
target_include_directories(tc PUBLIC include)

add_library(tc::tc ALIAS tc)

add_subdirectory(test)
add_subdirectory(bench)

add_executable(langtest src/langtest.cpp)
target_link_libraries(langtest PUBLIC tc::tc)

add_executable(pegtest src/pegtest.cpp)
target_link_libraries(pegtest PUBLIC peglib::peglib fmt::fmt)
