include(FetchContent)

FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.11.1
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(entt)
