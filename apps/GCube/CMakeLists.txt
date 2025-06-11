add_executable(GCubeApp
    main.cpp
)

set_target_properties(GCubeApp
        PROPERTIES
        OUTPUT_NAME "gcube"
)

target_link_libraries(GCubeApp PUBLIC
    RendererModule
    UtilitiesModule
    PlatformModule
)

add_custom_command(
        OUTPUT ${GyroBuildDir}/content
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/content ${GyroBuildDir}/content
        COMMENT "Copying content folder to build directory"
)

add_custom_target(copy_content ALL DEPENDS ${GyroBuildDir}/content)

target_include_directories(GCubeApp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)