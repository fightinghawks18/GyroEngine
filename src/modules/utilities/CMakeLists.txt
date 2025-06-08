add_library(UtilitiesModule SHARED
        tasks/maid.cpp

        debug/printer.cpp
)

target_include_directories(UtilitiesModule PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)