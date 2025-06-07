find_package(Vulkan REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)


add_library(RendererModule SHARED
        context/rendering_device.cpp

        implementation/volk_implementation.cpp
)

target_link_libraries(RendererModule PUBLIC
    Vulkan::Vulkan
    SDL3::SDL3
    vk-bootstrap::vk-bootstrap
    volk::volk
)

target_include_directories(RendererModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(RendererModule PUBLIC
    VK_NO_PROTOTYPES
)