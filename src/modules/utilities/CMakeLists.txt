find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_library(UtilitiesModule STATIC
        tasks/maid.cpp
        debug/logger.cpp
        types.h
        utils.h
        singleton.h
)

set_target_properties(UtilitiesModule
        PROPERTIES
        OUTPUT_NAME "utilities"
)

target_link_libraries(UtilitiesModule
        PUBLIC
        glm::glm
        fmt::fmt
)

target_include_directories(UtilitiesModule PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)