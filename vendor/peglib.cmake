include(FetchContent)

FetchContent_Declare(
    peglib
    GIT_REPOSITORY https://github.com/yhirose/cpp-peglib
    GIT_TAG v1.8.2
    GIT_PROGRESS TRUE
)

set(PEGLIB_BUILD_TESTS OFF CACHE INTERNAL "")

FetchContent_GetProperties(peglib)
if(NOT ${peglib}_POPULATED)
    FetchContent_Populate(peglib)
    find_package(Threads)
    add_library(peglib INTERFACE ${peglib_SOURCE_DIR}/peglib.h)
    target_include_directories(peglib INTERFACE ${peglib_SOURCE_DIR})
    target_link_libraries(peglib INTERFACE Threads::Threads)
    add_library(peglib::peglib ALIAS peglib)
endif()
