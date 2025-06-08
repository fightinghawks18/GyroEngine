find_package(SDL3 CONFIG REQUIRED)

add_library(PlatformModule SHARED
    window.cpp
)

target_link_libraries(PlatformModule PUBLIC
    SDL3::SDL3
)

target_include_directories(PlatformModule PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)