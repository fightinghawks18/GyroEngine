add_executable(GCubeApp
    main.cpp
)

target_link_libraries(GCubeApp PUBLIC
    RendererModule
    UtilitiesModule
    PlatformModule
)

target_include_directories(GCubeApp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)