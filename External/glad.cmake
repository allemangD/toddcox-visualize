FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)
set(GLAD_PROFILE "core" CACHE INTERNAL "OpenGL profile")
set(GLAD_API "gl=4.6" CACHE INTERNAL "API type/version pairs, like \"gl=3.2,gles=\", no version means latest")
set(GLAD_GENERATOR "c" CACHE INTERNAL "Language to generate the binding for")
FetchContent_MakeAvailable(glad)
