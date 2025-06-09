find_package(Vulkan REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)


add_library(RendererModule STATIC
        context/rendering_device.cpp

        implementation/volk_implementation.cpp

        rendering/renderer.cpp
        rendering/renderer.h

        utilities/renderer.h
        utilities/device.h
        rendering/viewport.h
        resources/image.cpp
        resources/image.h
        implementation/vma_implementation.cpp
        implementation/vma_implementation.h
        utilities/image.h
        resources/buffer.cpp
        resources/buffer.h
)

target_link_libraries(RendererModule
        PUBLIC
        Vulkan::Vulkan
        SDL3::SDL3
        vk-bootstrap::vk-bootstrap
        volk::volk
        PRIVATE
        UtilitiesModule
)

target_include_directories(RendererModule PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(RendererModule PUBLIC
    VK_NO_PROTOTYPES
)