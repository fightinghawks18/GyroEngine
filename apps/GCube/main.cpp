//
// Created by lepag on 6/8/2025.
//

#include <iostream>
#include <memory>

#include <SDL3/SDL.h>

#include "utils.h"
#include "window.h"
#include "../../src/core/engine.h"
#include "context/rendering_device.h"
#include "factories/mesh_factory.h"
#include "input/input.h"
#include "rendering/renderer.h"
#include "rendering/rendergraph/render_graph.h"
#include "rendering/rendergraph/passes/clear_pass.h"
#include "rendering/rendergraph/passes/scene_pass.h"
#include "resources/buffer_types.h"
#include "resources/light_data.h"
#include "resources/shader.h"
#include "resources/texture.h"
#include "utilities/shader.h"

#include "debug/logger.h"

using namespace GyroEngine;
using namespace Platform;

int main()
{
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

    auto renderer = std::make_shared<Rendering::Renderer>(engine.GetDevice());
    if (!renderer->Init(engine.GetWindow().get()))
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
    colorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    pipelineConfig.shaderStages = shaderStages;
    pipelineConfig.colorFormat = renderer->GetSwapchainColorFormat();
    pipelineConfig.colorBlendState.colorBlendStates.push_back(colorBlendState);

    pipelineConfig.rasterizerState.cullMode = VK_CULL_MODE_BACK_BIT;
    pipelineConfig.rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    pipelineConfig.vertexInputState.addBinding(0, sizeof(Types::Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
                  .addAttribute("ivVertexPosition", offsetof(Types::Vertex, position))
                  .addAttribute("ivVertexNormal", offsetof(Types::Vertex, normal))
                  .addAttribute("ivVertexUV", offsetof(Types::Vertex, texCoords))
                  .addAttribute("ivVertexTangent", offsetof(Types::Vertex, tangent))
                  .addAttribute("ivVertexColor", offsetof(Types::Vertex, color));

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

    texture->SetTexturePath(Utils::GetExecutableDir() + "/content/textures/woolymammoth.jpeg");
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
    light.position = {4.0f, 1.0f, 0.0f};
    light.direction = {0.0f, -1.0f, 0.0f};
    light.color = {1.0f, 1.0f, 1.0f};
    light.angle = 500.0f;
    light.type = 0;
    light.intensity = 1.0f;
    light.range = 100.0f;

    Resources::LightSource light2 = {};
    light.position = {-4.0f, 1.0f, 0.0f};
    light.direction = {0.0f, -1.0f, 0.0f};
    light.color = {1.0f, 0.0f, 0.0f};
    light.angle = 500.0f;
    light.type = 0;
    light.intensity = 1.0f;
    light.range = 100.0f;

    Resources::LightBuffer lights = {};
    lights.lights[0] = light;
    lights.lights[1] = light2;
    lights.lightCount = 2;


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

    glm::vec3 cameraPosition = {0.0f, 0.0f, -6.0f};

    Resources::CameraBuffer camera = {};
    camera.position = cameraPosition;
    camera.direction = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.fov = glm::radians(45.0f);
    camera.aspect = 1.0f;
    camera.nearPlane = 0.1f;
    camera.farPlane = 10.0f;

    glm::vec2 mousePos = Input::GetMousePosition();
    glm::vec2 mousePos2 = mousePos;

    float yaw = 0.0f;
    float pitch = 0.0f;

    auto wind = engine.GetWindow();

        engine.SetDestroyFunction([&]()
    {
        pipeline.reset();
        fragmentShader.reset();
        vertexShader.reset();
        pipelineBindings.reset();
        renderer.reset();
        cube.reset();
    });

    engine.SetUpdateFunction([&]()
    {
        if (Input::GetKeyValue<bool>(KeyCode::Escape))
        {
            engine.Close();
        }
        if (renderer->RecordFrame())
        {
            scenePass->AddGeometry(cube);

            const auto frame = renderer->GetFrameContext();

            mousePos = Input::GetMousePosition();
            glm::vec2 mouseDelta = (mousePos - mousePos2) * 0.005f; // Invert Y axis for camera movement
            mousePos2 = mousePos;

            yaw -= mouseDelta.x;
            pitch -= mouseDelta.y;

            glm::vec3 cameraDirection;
            cameraDirection.x = cos(pitch) * sin(yaw);
            cameraDirection.y = sin(pitch);
            cameraDirection.z = cos(pitch) * cos(yaw);
            cameraDirection = glm::normalize(cameraDirection);

            glm::vec3 forward = glm::normalize(cameraDirection);
            glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));

            float moveSpeed = 0.005f;
            glm::vec3 movement = glm::vec3(0.0f);

            if (Input::GetKeyValue<bool>(KeyCode::W)) movement += forward * moveSpeed;
            if (Input::GetKeyValue<bool>(KeyCode::S)) movement -= forward * moveSpeed;
            if (Input::GetKeyValue<bool>(KeyCode::D)) movement += right * moveSpeed;
            if (Input::GetKeyValue<bool>(KeyCode::A)) movement -= right * moveSpeed;

            cameraPosition += movement;

            camera.position = cameraPosition;
            camera.direction = camera.position + cameraDirection;
            cameraBuffer->Map(&camera);

            Logger::Log("POS: {}, {}, {}", camera.position.x, camera.position.y, camera.position.z);

            view = glm::lookAtRH(camera.position, camera.direction, camera.up);
            proj = glm::perspectiveRH(glm::radians(90.0f),
                                      static_cast<float>(frame.swapchainExtent.width) /
                                      static_cast<float>(frame.swapchainExtent.height),
                                      0.1f, 1000.0f);
            proj[1][1] *= -1.0f;

            cube->SetTransforms(view, proj);
            cube->GetRotation() += glm::vec3(0.0f, 0.001f, 0.0f);

            if (pipelineBindings->DoesBindingExist("usTexture"))
            {
                pipelineBindings->UpdateDescriptorImage("usTexture", texture->GetSampler(), texture->GetImage(),
                                                        frame.frameIndex);
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
        }
        else
        {
            renderer->NextFrameIndex();
        }
    });

    engine.Run();

    return 0;
}
