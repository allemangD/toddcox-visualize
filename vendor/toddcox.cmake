include(FetchContent)

FetchContent_Declare(
    toddcox
    GIT_REPOSITORY https://github.com/jcraymond/toddcox-faster.git
    GIT_TAG 265de59917bdf94709b40ad8aef5dd9ce5574242
    GIT_PROGRESS TRUE
)

set(TC_BUILD_EXAMPLE OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(toddcox)
