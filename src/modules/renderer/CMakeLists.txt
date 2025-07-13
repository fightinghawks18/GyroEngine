find_package(Vulkan REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)
find_package(unofficial-spirv-reflect CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Stb REQUIRED)
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
        resources/texture/image.cpp
        resources/texture/image.h
        implementation/vma_implementation.cpp
        implementation/vma_implementation.h
        utilities/image.h
        resources/buffer/buffer.cpp
        resources/buffer/buffer.h
        resources/texture/sampler.cpp
        resources/texture/sampler.h
        resources/pipeline/pipeline.cpp
        resources/pipeline/pipeline.h
        utilities/pipeline.h
        resources/pipeline/shader.cpp
        resources/pipeline/shader.h
        utilities/shader.h
        resources/pipeline/push_constant.h
        resources/object/mesh.cpp
        resources/object/mesh.h
        factories/mesh_factory.cpp
        factories/mesh_factory.h
        utilities/mesh.h
        resources/object/material.cpp
        resources/object/material.h
        implementation/stb_implementation.h
        implementation/stb_implementation.cpp
        resources/texture/texture.cpp
        resources/texture/texture.h
        resources/buffer/light_data.h
        resources/pipeline/pipeline_bindings.cpp
        resources/pipeline/pipeline_bindings.h
        resources/buffer/buffer_types.h
        implementation/glm_implementation.h
        implementation/glm_implementation.cpp
        commands/command.h
        commands/mesh_command.h
        commands/material_command.h
        commands/clear_color_command.h
        commands/clear_depth_command.h
        commands/sky_command.h
        commands/particle_command.h
        resources/command_batch.cpp
        resources/command_batch.h
)

set_target_properties(RendererModule
        PROPERTIES
        OUTPUT_NAME "renderer"
)

target_link_libraries(RendererModule
        PUBLIC
        Vulkan::Vulkan
        unofficial::shaderc::shaderc
        unofficial::spirv-reflect
        SDL3::SDL3
        assimp::assimp
        vk-bootstrap::vk-bootstrap
        volk::volk
        PRIVATE
        UtilitiesModule
)

target_include_directories(RendererModule
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE
        ${Stb_INCLUDE_DIRS}
)

target_compile_definitions(RendererModule
        PUBLIC
        VK_NO_PROTOTYPES
)