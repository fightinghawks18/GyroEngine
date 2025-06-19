//
// Created by lepag on 6/8/2025.
//

#include <iostream>
#include <memory>

#include <SDL3/SDL.h>

#include "utils.h"
#include "window.h"
#include "context/rendering_device.h"
#include "factories/geometry_factory.h"
#include "rendering/renderer.h"
#include "rendering/rendergraph/render_graph.h"
#include "rendering/rendergraph/passes/clear_pass.h"
#include "rendering/rendergraph/passes/scene_pass.h"
#include "resources/shader.h"
#include "utilities/shader.h"

using namespace GyroEngine;

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }
    // Compile shaders first
    Utils::Shader::CompileShaderToFile(utils::GetExecutableDir() + "/content/shaders/post_effects/sky.frag",
                                     shaderc_fragment_shader);
    Utils::Shader::CompileShaderToFile(utils::GetExecutableDir() + "/content/shaders/post_effects/fullscreen_quad.vert",
                                 shaderc_vertex_shader);
    Utils::Shader::CompileShaderToFile(utils::GetExecutableDir() + "/content/shaders/geometry/object.vert",
                                     shaderc_vertex_shader);
    Utils::Shader::CompileShaderToFile(utils::GetExecutableDir() + "/content/shaders/geometry/object.frag",
                                     shaderc_fragment_shader);

    auto window = std::make_unique<Platform::Window>();
    if (!window->Init())
    {
        std::cerr << "Failed to create window." << std::endl;
        return -1;
    }

    auto device = std::make_unique<Device::RenderingDevice>();
    if (!device->Init())
    {
        std::cerr << "Failed to initialize rendering device." << std::endl;
        return -1;
    }
    auto renderer = std::make_shared<Rendering::Renderer>(*device);
    if (!renderer->Init(window.get()))
    {
        std::cerr << "Failed to initialize renderer." << std::endl;
        return -1;
    }


    const auto clearPass = std::make_shared<Rendering::Passes::ClearPass>();
    const auto scenePass = std::make_shared<Rendering::Passes::ScenePass>();
    const auto renderGraph = std::make_shared<Rendering::RenderGraph>();
    renderGraph->SetDebug(true);

    clearPass->SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});

    auto pipeline = std::make_shared<Resources::Pipeline>(*device);
    auto& pipelineConfig = pipeline->GetPipelineConfig();

    auto vertexShader = std::make_shared<Resources::Shader>(*device);
    vertexShader->SetShaderPath(utils::GetExecutableDir() + "/content/shaders/geometry/object.vert.spv");
    if (!vertexShader->Init())
    {
        std::cerr << "Failed to initialize vertex shader." << std::endl;
        return -1;
    }

    auto fragmentShader = std::make_shared<Resources::Shader>(*device);
    fragmentShader->SetShaderPath(utils::GetExecutableDir() + "/content/shaders/geometry/object.frag.spv");
    if (!fragmentShader->Init())
    {
        std::cerr << "Failed to initialize fragment shader." << std::endl;
        return -1;
    }

    auto descriptorManager = std::make_shared<Resources::DescriptorManager>(*device);
    const auto objectLayout = descriptorManager->CreateDescriptorLayout();
    objectLayout->AddBinding({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr});
    if (!objectLayout->Init())
    {
        std::cerr << "Failed to initialize object descriptor layout." << std::endl;
        return -1;
    }
    const auto objectPool = descriptorManager->CreateDescriptorPool();
    objectPool->AddPoolSize({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10});
    objectPool->SetMaxSets(10);
    if (!objectPool->Init())
    {
        std::cerr << "Failed to initialize object descriptor pool." << std::endl;
        return -1;
    }

    Utils::Pipeline::PipelineShaderStage vertexShaderStage = {};
    vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStage.module = vertexShader->GetShaderModule();
    vertexShaderStage.entryPoint = "main";

    Utils::Pipeline::PipelineShaderStage fragmentShaderStage = {};
    fragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStage.module = fragmentShader->GetShaderModule();
    fragmentShaderStage.entryPoint = "main";

    std::vector<Utils::Pipeline::PipelineShaderStage> shaderStages = {};
    shaderStages.push_back(vertexShaderStage);
    shaderStages.push_back(fragmentShaderStage);

    pipelineConfig.shaderStages = shaderStages;
    pipelineConfig.colorFormat = renderer->GetSwapchainColorFormat();

    pipelineConfig.vertexInputState.addBinding(0, sizeof(Types::Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    pipelineConfig.vertexInputState.addAttribute(0, 0, offsetof(Types::Vertex, position), VK_FORMAT_R32G32B32_SFLOAT); // position
    pipelineConfig.vertexInputState.addAttribute(0, 1, offsetof(Types::Vertex, normal),   VK_FORMAT_R32G32B32_SFLOAT); // normal
    pipelineConfig.vertexInputState.addAttribute(0, 2, offsetof(Types::Vertex, texCoords),VK_FORMAT_R32G32_SFLOAT);    // texCoords
    pipelineConfig.vertexInputState.addAttribute(0, 3, offsetof(Types::Vertex, tangent),  VK_FORMAT_R32G32B32_SFLOAT); // tangent
    pipelineConfig.vertexInputState.addAttribute(0, 4, offsetof(Types::Vertex, color),    VK_FORMAT_R32G32B32A32_SFLOAT); // color

    pipeline->SetDescriptorManager(descriptorManager);
    if (!pipeline->Init())
    {
        std::cerr << "Failed to initialize pipeline." << std::endl;
        return -1;
    }

    const auto cube = Factories::GeometryFactory::CreateCube();
    auto cubeGeometry = std::make_shared<Resources::Geometry>(*device);
    cubeGeometry->UseObjectMap(cube)
                .UsePipeline(pipeline);
    if (!cubeGeometry->Generate())
    {
        std::cerr << "Failed to generate cube geometry." << std::endl;
        return -1;
    }

    while (!window->HasRequestedQuit())
    {
        window->Update();
        if (window->IsWindowAlive())
        {
            if (renderer->RecordFrame())
            {
                scenePass->AddGeometry(cubeGeometry);

                cubeGeometry->GetRotation() += glm::vec3(0.0f, 0.01f, 0.0f);

                renderGraph->AddPass(clearPass);
                renderGraph->AddPass(scenePass);

                Rendering::Viewport viewport{};
                renderer->BindViewport(viewport);
                renderGraph->Execute(*renderer);
                renderer->SubmitFrame();
            } else
            {
                renderer->NextFrameIndex();
            }
        } else
        {
            break;
        }
    }

    device->WaitForIdle();

    // Cleanup resources
    cubeGeometry.reset();
    pipeline.reset();
    fragmentShader.reset();
    vertexShader.reset();
    descriptorManager.reset();

    // Cleanup renderer, device and window
    renderer.reset();
    window.reset();
    device.reset();
    SDL_Quit();
    return 0;
}
