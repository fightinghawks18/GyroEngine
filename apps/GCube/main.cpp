//
// Created by lepag on 6/8/2025.
//

#include <iostream>
#include <memory>

#include <SDL3/SDL.h>

#include "Utils.h"
#include "window.h"
#include "../../src/core/engine.h"
#include "context/rendering_device.h"
#include "factories/mesh_factory.h"
#include "rendering/renderer.h"
#include "rendering/rendergraph/render_graph.h"
#include "rendering/rendergraph/passes/clear_pass.h"
#include "rendering/rendergraph/passes/scene_pass.h"
#include "resources/buffer_types.h"
#include "resources/light_data.h"
#include "resources/shader.h"
#include "resources/texture.h"
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
    Utils::Shader::CompileShaderToFile(Utils::GetExecutableDir() + "/content/shaders/post_effects/sky.frag",
                                     shaderc_fragment_shader);
    Utils::Shader::CompileShaderToFile(Utils::GetExecutableDir() + "/content/shaders/post_effects/fullscreen_quad.vert",
                                 shaderc_vertex_shader);
     Utils::Shader::CompileShaderToFile(Utils::GetExecutableDir() + "/content/shaders/geometry/object.vert",
                                     shaderc_vertex_shader);
    Utils::Shader::CompileShaderToFile(Utils::GetExecutableDir() + "/content/shaders/geometry/object.frag",
                                     shaderc_fragment_shader);

    auto& engine = Engine::Get();
    if (!engine.Init())
    {
        std::cerr << "Failed to initialize engine." << std::endl;
        return -1;
    }

    auto window = std::make_unique<Platform::Window>();
    if (!window->Init())
    {
        std::cerr << "Failed to create window." << std::endl;
        return -1;
    }

    auto renderer = std::make_shared<Rendering::Renderer>(engine.GetDevice());
    if (!renderer->Init(window.get()))
    {
        std::cerr << "Failed to initialize renderer." << std::endl;
        return -1;
    }


    const auto clearPass = std::make_shared<Rendering::Passes::ClearPass>();
    const auto scenePass = std::make_shared<Rendering::Passes::ScenePass>();
    const auto renderGraph = std::make_shared<Rendering::RenderGraph>();

    auto view = glm::mat4(1.0f);
    auto proj = glm::mat4(1.0f);

    // Enable render graph debug mode if in debug build
#ifdef DEBUG
    renderGraph->SetDebug(true);
#endif

    clearPass->SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});

    auto pipeline = std::make_shared<Resources::Pipeline>(engine.GetDevice());
    auto& pipelineConfig = pipeline->GetPipelineConfig();

    auto vertexShader = std::make_shared<Resources::Shader>(engine.GetDevice());
    vertexShader->SetShaderPath(Utils::GetExecutableDir() + "/content/shaders/geometry/object.vert.spv")
                .SetShaderStage(Utils::Shader::ShaderStage::Vertex);
    if (!vertexShader->Init())
    {
        std::cerr << "Failed to initialize vertex shader." << std::endl;
        return -1;
    }

    auto fragmentShader = std::make_shared<Resources::Shader>(engine.GetDevice());
    fragmentShader->SetShaderPath(Utils::GetExecutableDir() + "/content/shaders/geometry/object.frag.spv")
                    .SetShaderStage(Utils::Shader::ShaderStage::Fragment);
    if (!fragmentShader->Init())
    {
        std::cerr << "Failed to initialize fragment shader." << std::endl;
        return -1;
    }

    auto pipelineBindings = std::make_shared<Resources::PipelineBindings>(engine.GetDevice());
    pipelineBindings->AddShader(vertexShader);
    pipelineBindings->AddShader(fragmentShader);
    if (!pipelineBindings->Init())
    {
        std::cerr << "Failed to initialize pipeline bindings." << std::endl;
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

    Utils::Pipeline::PipelineColorBlendAttachment colorBlendState = {};
    colorBlendState.blendEnable = VK_TRUE;
    colorBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendState.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    pipelineConfig.shaderStages = shaderStages;
    pipelineConfig.colorFormat = renderer->GetSwapchainColorFormat();
    pipelineConfig.colorBlendState.colorBlendStates.push_back(colorBlendState);

    pipelineConfig.rasterizerState.cullMode = VK_CULL_MODE_BACK_BIT;
    pipelineConfig.rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    pipelineConfig.vertexInputState.addBinding(0, sizeof(Types::Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    pipelineConfig.vertexInputState.addAttribute(0, 0, offsetof(Types::Vertex, position), VK_FORMAT_R32G32B32_SFLOAT); // position
    pipelineConfig.vertexInputState.addAttribute(0, 1, offsetof(Types::Vertex, normal),   VK_FORMAT_R32G32B32_SFLOAT); // normal
    pipelineConfig.vertexInputState.addAttribute(0, 2, offsetof(Types::Vertex, texCoords),VK_FORMAT_R32G32_SFLOAT);    // texCoords
    pipelineConfig.vertexInputState.addAttribute(0, 3, offsetof(Types::Vertex, tangent),  VK_FORMAT_R32G32B32_SFLOAT); // tangent
    pipelineConfig.vertexInputState.addAttribute(0, 4, offsetof(Types::Vertex, color),    VK_FORMAT_R32G32B32A32_SFLOAT); // color

    pipeline->SetPipelineBindings(pipelineBindings);
    if (!pipeline->Init())
    {
        std::cerr << "Failed to initialize pipeline." << std::endl;
        return -1;
    }

    auto texture = std::make_shared<Resources::Texture>(engine.GetDevice());
    if (!texture->Init())
    {
        std::cerr << "Failed to initialize texture." << std::endl;
        return -1;
    }

    texture->SetTexturePath(Utils::GetExecutableDir() + "/content/textures/woolymammoth.jpg");
    if (!texture->Generate())
    {
        std::cerr << "Failed to generate texture." << std::endl;
        return -1;
    }

    auto cube = Factories::MeshFactory::CreateFromFile(Utils::GetExecutableDir() + "/content/primitives/cube.glb");
    cube->UsePipeline(pipeline);
    if (!cube->Generate())
    {
        std::cerr << "Failed to initialize cube mesh." << std::endl;
        return -1;
    }

    // Create light source
    Resources::LightSource light = {};
    light.position = {0.0f, 0.0f, 0.0f};
    light.direction = {0.0f, -1.0f, 0.0f};
    light.color = {1.0f, 1.0f, 1.0f};
    light.angle = 50.0f;
    light.type = 0;
    light.intensity = 1.0f;
    light.range = 100.0f;

    Resources::LightBuffer lights = {};
    lights.light[0] = light;
    lights.lightCount = 1;


    // Create light buffer
    auto lightBuffer = std::make_shared<Resources::Buffer>(engine.GetDevice());
    lightBuffer->SetMemoryUsage(VMA_MEMORY_USAGE_AUTO)
                .SetSharingMode(VK_SHARING_MODE_EXCLUSIVE)
                .SetSize(sizeof(Resources::LightBuffer))
                .SetUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    if (!lightBuffer->Init())
    {
        std::cerr << "Failed to initialize light buffer." << std::endl;
        return -1;
    }
    lightBuffer->Map(&lights);

    // Create camera buffer
    auto cameraBuffer = std::make_shared<Resources::Buffer>(engine.GetDevice());
    cameraBuffer->SetMemoryUsage(VMA_MEMORY_USAGE_AUTO)
                .SetSharingMode(VK_SHARING_MODE_EXCLUSIVE)
                .SetSize(sizeof(Resources::CameraBuffer) * 1)
                .SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    if (!cameraBuffer->Init())
    {
        Logger::LogError("Failed to initialize camera buffer.");
        return -1;
    }

    Resources::CameraBuffer camera = {};
    camera.position = glm::vec3(4.0f, 3.0f, 3.0f);
    camera.direction = glm::vec3(0.0f, 0.0f, -1.0f);
    camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.fov = glm::radians(45.0f);
    camera.aspect = 1.0f;
    camera.nearPlane = 0.1f;
    camera.farPlane = 10.0f;

    cameraBuffer->Map(&camera);




    while (!window->HasRequestedQuit())
    {
        window->Update();
        if (window->IsWindowAlive())
        {
            if (renderer->RecordFrame())
            {
                scenePass->AddGeometry(cube);

                const auto frame = renderer->GetFrameContext();

                view = glm::lookAtRH(glm::vec3(4.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                proj = glm::perspectiveRH(glm::radians(45.0f),
                                          static_cast<float>(frame.swapchainExtent.width) /
                                          static_cast<float>(frame.swapchainExtent.height),
                                          0.1f, 10.0f);
                proj[1][1] *= -1.0f;

                cube->SetTransforms(view, proj);
                cube->GetRotation() += glm::vec3(0.0f, 0.01f, 0.0f);

                if (pipelineBindings->DoesBindingExist("usTexture"))
                {
                    pipelineBindings->UpdateDescriptorImage("usTexture", texture->GetSampler(), texture->GetImage(), frame.frameIndex);
                }
                if (pipelineBindings->DoesBindingExist("cam"))
                {
                    pipelineBindings->UpdateDescriptorBuffer("cam", cameraBuffer, frame.frameIndex);
                }
                if (pipelineBindings->DoesBindingExist("lightBuffer"))
                {
                    pipelineBindings->UpdateDescriptorBuffer("lightBuffer", lightBuffer, frame.frameIndex);
                }

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

    engine.GetDevice().WaitForIdle();

    // Cleanup resources
    cube.reset();
    pipeline.reset();
    fragmentShader.reset();
    vertexShader.reset();
    pipelineBindings.reset();

    // Cleanup renderer, device and window
    renderer.reset();
    window.reset();
    return 0;
}
