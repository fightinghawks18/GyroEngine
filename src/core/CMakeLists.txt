add_library(GyroEngine STATIC
        engine.cpp
)

target_link_libraries(GyroEngine
        PRIVATE
        RendererModule
        UtilitiesModule
        PlatformModule
)

target_include_directories(GyroEngine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(GyroEngine
        PROPERTIES
        OUTPUT_NAME "gyroengine"
)

# Copy content to the build directory
copy_directory_to(${CMAKE_CURRENT_SOURCE_DIR}/content ${GyroBuildDir}/content)