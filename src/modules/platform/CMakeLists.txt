find_package(SDL3 CONFIG REQUIRED)

add_library(PlatformModule STATIC
        window.cpp
)

set_target_properties(PlatformModule
        PROPERTIES
        OUTPUT_NAME "platform"
)

target_link_libraries(PlatformModule PUBLIC
    SDL3::SDL3
)

target_include_directories(PlatformModule PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)