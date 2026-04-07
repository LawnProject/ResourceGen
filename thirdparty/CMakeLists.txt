set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)

include(FetchContent)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG main
)

FetchContent_Declare(
    wxWidgets 
    GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets
    GIT_TAG        v3.3.2
)

FetchContent_Declare(
    tinyxml2 
    GIT_REPOSITORY https://github.com/leethomason/tinyxml2
    GIT_TAG        11.0.0
)

FetchContent_Declare(
    fmt 
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        main
)

FetchContent_MakeAvailable(SDL3 wxWidgets tinyxml2 fmt)