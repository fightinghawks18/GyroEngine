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
        resources/image.cpp
        resources/image.h
        implementation/vma_implementation.cpp
        implementation/vma_implementation.h
        utilities/image.h
        resources/buffer.cpp
        resources/buffer.h
        resources/sampler.cpp
        resources/sampler.h
        resources/pipeline.cpp
        resources/pipeline.h
        utilities/pipeline.h
        resources/shader.cpp
        resources/shader.h
        utilities/shader.h
        resources/push_constant.h
        rendering/rendergraph/render_graph.cpp
        rendering/rendergraph/render_graph.h
        rendering/rendergraph/render_pass.h
        rendering/rendergraph/passes/clear_pass.h
        rendering/rendergraph/passes/scene_pass.h
        resources/mesh.cpp
        resources/mesh.h
        factories/mesh_factory.cpp
        factories/mesh_factory.h
        rendering/commands/command.h
        resources/pipeline_bindings.cpp
        resources/pipeline_bindings.h
        utilities/mesh.h
        resources/material.cpp
        resources/material.h
        implementation/stb_implementation.h
        implementation/stb_implementation.cpp
        resources/texture.cpp
        resources/texture.h
        resources/light_data.h
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