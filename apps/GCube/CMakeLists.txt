include(${CMAKE_SOURCE_DIR}/cmake/copy_directory_to.cmake)

add_executable(GCubeApp
        main.cpp
)

set_target_properties(GCubeApp
        PROPERTIES
        OUTPUT_NAME "gcube"
)

target_link_libraries(GCubeApp PUBLIC
        GyroEngine
        RendererModule
        UtilitiesModule
        PlatformModule
)

target_include_directories(GCubeApp PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)