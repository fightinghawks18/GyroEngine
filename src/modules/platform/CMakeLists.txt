find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_library(PlatformModule STATIC
        window.cpp
        input/keyboard.cpp
        input/keyboard.h
        input/inpututils.h
        input/mouse.cpp
        input/mouse.h
)

set_target_properties(PlatformModule
        PROPERTIES
        OUTPUT_NAME "platform"
)

target_link_libraries(PlatformModule PUBLIC
        UtilitiesModule
        SDL3::SDL3
        glm::glm
)

target_include_directories(PlatformModule PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)