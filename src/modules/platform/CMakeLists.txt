find_package(SDL3 CONFIG REQUIRED)

add_library(PlatformModule SHARED
    window.cpp
)

target_link_libraries(PlatformModule PUBLIC
    SDL3::SDL3
)