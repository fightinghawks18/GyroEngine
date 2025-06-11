find_package(Vulkan REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)
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
        resources/device_resource.h
        resources/sampler.cpp
        resources/sampler.h
        resources/descriptor_manager.cpp
        resources/descriptor_manager.h
        resources/descriptor_set.cpp
        resources/descriptor_set.h
        resources/descriptor_pool.cpp
        resources/descriptor_pool.h
        resources/descriptor_layout.cpp
        resources/descriptor_layout.h
        rendering/render_pipeline.cpp
        rendering/render_pipeline.h
        rendering/render_step.h
        rendering/render_steps/clear_step.cpp
        rendering/render_steps/clear_step.h
        resources/pipeline.cpp
        resources/pipeline.h
        utilities/pipeline.h
        resources/shader.cpp
        resources/shader.h
        utilities/shader.h
        rendering/render_steps/scene_step.cpp
        rendering/render_steps/scene_step.h
)

set_target_properties(RendererModule
        PROPERTIES
        OUTPUT_NAME "renderer"
)

target_link_libraries(RendererModule
        PUBLIC
        Vulkan::Vulkan
        unofficial::shaderc::shaderc
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