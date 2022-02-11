FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.10.5
)
SET(JSON_ImplicitConversions OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(json)
