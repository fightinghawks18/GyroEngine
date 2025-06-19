find_package(glm CONFIG REQUIRED)

add_library(UtilitiesModule STATIC
        tasks/maid.cpp
        debug/Logger.cpp
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
)

target_include_directories(UtilitiesModule PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)