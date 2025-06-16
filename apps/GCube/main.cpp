//
// Created by lepag on 6/8/2025.
//

#include <iostream>
#include <memory>

#include <SDL3/SDL.h>

#include "utils.h"
#include "window.h"
#include "context/rendering_device.h"
#include "rendering/renderer.h"
#include "rendering/render_steps/clear_step.h"
#include "rendering/render_steps/post_step.h"
#include "rendering/render_steps/scene_step.h"
#include "resources/push_constant.h"
#include "resources/shader.h"
#include "utilities/shader.h"


int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }
        // Compile shaders first
        shaderutils::compileShaderToFile(utils::getExecutableDir() + "/content/shaders/simple_object.vert", shaderc_vertex_shader);
        shaderutils::compileShaderToFile(utils::getExecutableDir() + "/content/shaders/simple_object.frag", shaderc_fragment_shader);
        shaderutils::compileShaderToFile(utils::getExecutableDir() + "/content/shaders/blur_h.frag", shaderc_fragment_shader);
        shaderutils::compileShaderToFile(utils::getExecutableDir() + "/content/shaders/blur_v.frag", shaderc_fragment_shader);
        shaderutils::compileShaderToFile(utils::getExecutableDir() + "/content/shaders/fullscreen_quad.vert", shaderc_vertex_shader);

        auto window = std::make_unique<Window>();
        if (!window->create())
        {
            std::cerr << "Failed to create window." << std::endl;
            return -1;
        }

        auto device = std::make_unique<RenderingDevice>();
        if (!device->init())
        {
            std::cerr << "Failed to initialize rendering device." << std::endl;
            return -1;
        }
        auto renderer = std::make_shared<Renderer>(*device);
        if (!renderer->init(window.get()))
        {
            std::cerr << "Failed to initialize renderer." << std::endl;
            return -1;
        }

        std::vector<types::Vertex> vertices = {
            { // Bottom right
                {
                    0.5f, -0.5f, 0.0f
                },
                {
                    1.0f, 0.0f, 0.0f
                }
            },
            { // Top right
                {
                    0.5f, 0.5f, 0.0f
                },
                {
                    0.0f, 1.0f, 0.0f
                },
            },
            { // Top left
                {
                    -0.5f, 0.5f, 0.0f
                },
                {
                    0.0f, 0.0f, 1.0f
                }
            },
            { // Bottom left
                {
                    -0.5f, -0.5f, 0.0f
                },
                {
                    1.0f, 1.0f, 1.0f
                }
            }

        };

        std::vector<uint32_t> indices = {
            0, 1, 2,
            2, 3, 0
        };

    {
        auto clearPass = std::make_shared<ClearStep>();
        auto scenePass = std::make_shared<SceneStep>();
        auto postPass = std::make_shared<PostStep>();

        Viewport viewport{0};
        viewport.width = 1.f;
        viewport.height = 1.f;


        auto pipeline = std::make_shared<Pipeline>(*device);
        pipeline->setColorFormat(renderer->getSwapchainColorFormat());

        auto vertexBuffer = std::make_shared<Buffer>(*device);
        auto indexBuffer = std::make_shared<Buffer>(*device);

        vertexBuffer->setBufferType(Buffer::BufferType::Vertex)
                    .setSize(vertices.size() * sizeof(types::Vertex))
                    .setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
                    .setMemoryUsage(VMA_MEMORY_USAGE_CPU_TO_GPU)
                    .setSharingMode(VK_SHARING_MODE_EXCLUSIVE)
                    .init();
        indexBuffer->setBufferType(Buffer::BufferType::Index)
                   .setSize(indices.size() * sizeof(uint32_t))
                   .setUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
                   .setMemoryUsage(VMA_MEMORY_USAGE_CPU_TO_GPU)
                   .setSharingMode(VK_SHARING_MODE_EXCLUSIVE)
                   .init();

        vertexBuffer->map(vertices.data());
        indexBuffer->map(indices.data());

        auto vertexShader = std::make_shared<Shader>(*device);
        auto fragmentShader = std::make_shared<Shader>(*device);

        vertexShader->setShaderPath(utils::getExecutableDir() + "/content/shaders/simple_object.vert.spv")
                    .init();
        fragmentShader->setShaderPath(utils::getExecutableDir() + "/content/shaders/simple_object.frag.spv")
                      .init();

        auto& pipelineConfig = pipeline->getPipelineConfig();

        pipelineutils::PipelineShaderStage vertexShaderStage = {};
        vertexShaderStage.entryPoint = "main";
        vertexShaderStage.module = vertexShader->getShaderModule();
        vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;

        pipelineutils::PipelineShaderStage fragmentShaderStage = {};
        fragmentShaderStage.entryPoint = "main";
        fragmentShaderStage.module = fragmentShader->getShaderModule();
        fragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

        pipelineutils::PipelineColorBlendAttachment colorAttachment = {};
        colorAttachment.blendEnable = false;
        pipelineConfig.colorBlendState.colorBlendStates.push_back(colorAttachment);

        pipelineConfig.vertexInputState.addBinding(
            0,
            sizeof(types::Vertex),
            VK_VERTEX_INPUT_RATE_VERTEX);

        pipelineConfig.vertexInputState.addAttribute(
            0,
            0,
            offsetof(types::Vertex, position),
            VK_FORMAT_R32G32B32_SFLOAT);
        pipelineConfig.vertexInputState.addAttribute(
            0,
            1,
            offsetof(types::Vertex, color),
            VK_FORMAT_R32G32B32_SFLOAT);

        pipelineConfig.rasterizerState.cullMode = VK_CULL_MODE_NONE;
        pipelineConfig.depthStencilState.depthTest = VK_FALSE;

        pipelineConfig.shaderStages.push_back(vertexShaderStage);
        pipelineConfig.shaderStages.push_back(fragmentShaderStage);

        pipeline->init();

        auto quadShader = std::make_shared<Shader>(*device);
        quadShader->setShaderPath(utils::getExecutableDir() + "/content/shaders/fullscreen_quad.vert.spv")
                   .init();

        auto blurhShader = std::make_shared<Shader>(*device);
        auto blurvShader = std::make_shared<Shader>(*device);
        blurhShader->setShaderPath(utils::getExecutableDir() + "/content/shaders/blur_h.frag.spv");
        blurvShader->setShaderPath(utils::getExecutableDir() + "/content/shaders/blur_v.frag.spv");

        if (!blurhShader->init() || !blurvShader->init())
        {
            std::cerr << "Failed to initialize blur shaders." << std::endl;
            return -1;
        }

        pipelineutils::PipelineShaderStage quadShaderStage = {};
        quadShaderStage.entryPoint = "main";
        quadShaderStage.module = quadShader->getShaderModule();
        quadShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;

        pipelineutils::PipelineShaderStage blurhShaderStage = {};
        blurhShaderStage.entryPoint = "main";
        blurhShaderStage.module = blurhShader->getShaderModule();
        blurhShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

        pipelineutils::PipelineShaderStage blurvShaderStage = {};
        blurvShaderStage.entryPoint = "main";
        blurvShaderStage.module = blurvShader->getShaderModule();
        blurvShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

        // Create post process pipeline
        // ^ This pass will be used to apply a horizontal and vertical blur to the scene

        auto blurhPConstant = new PushConstant();
        blurhPConstant->setStageFlags(VK_SHADER_STAGE_FRAGMENT_BIT)
                     .setOffset(0)
                     .setSize(sizeof(float));

        auto blurvPConstant = new PushConstant();
        blurvPConstant->setStageFlags(VK_SHADER_STAGE_FRAGMENT_BIT)
                     .setOffset(0)
                     .setSize(sizeof(float));

        auto blurhPipeline = std::make_shared<Pipeline>(*device);
        blurhPipeline->setColorFormat(renderer->getSwapchainColorFormat());


        blurhPipeline->getPipelineConfig().pushConstants.push_back(blurhPConstant);

        blurhPipeline->getPipelineConfig().inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

        blurhPipeline->getPipelineConfig().colorBlendState.colorBlendStates.push_back(colorAttachment);
        blurhPipeline->getPipelineConfig().rasterizerState.cullMode = VK_CULL_MODE_NONE;
        blurhPipeline->getPipelineConfig().depthStencilState.depthTest = VK_FALSE;

        blurhPipeline->getPipelineConfig().shaderStages.push_back(quadShaderStage);
        blurhPipeline->getPipelineConfig().shaderStages.push_back(blurhShaderStage);

        auto blurvPipeline = std::make_shared<Pipeline>(*device);
        blurvPipeline->setColorFormat(renderer->getSwapchainColorFormat());

        blurvPipeline->getPipelineConfig().pushConstants.push_back(blurvPConstant);

        blurvPipeline->getPipelineConfig().inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

        blurvPipeline->getPipelineConfig().colorBlendState.colorBlendStates.push_back(colorAttachment);
        blurvPipeline->getPipelineConfig().rasterizerState.cullMode = VK_CULL_MODE_NONE;
        blurvPipeline->getPipelineConfig().depthStencilState.depthTest = VK_FALSE;

        blurvPipeline->getPipelineConfig().shaderStages.push_back(quadShaderStage);
        blurvPipeline->getPipelineConfig().shaderStages.push_back(blurvShaderStage);

        auto blurhDescriptorManager = std::make_shared<DescriptorManager>(*device);
        auto blurvDescriptorManager = std::make_shared<DescriptorManager>(*device);

        auto blurhDescriptorLayout = blurhDescriptorManager->createDescriptorLayout();
        auto blurvDescriptorLayout = blurvDescriptorManager->createDescriptorLayout();

        blurhDescriptorLayout->addBinding({0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT});
        blurvDescriptorLayout->addBinding({0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT});

        auto blurhPool = blurhDescriptorManager->createDescriptorPool();
        auto blurvPool = blurvDescriptorManager->createDescriptorPool();

        blurhPool->addPoolSize({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 200});
        blurvPool->addPoolSize({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 200});
        blurhPool->setMaxSets(1);
        blurvPool->setMaxSets(1);

        blurhDescriptorLayout->init();
        blurvDescriptorLayout->init();
        blurhPool->init();
        blurvPool->init();

        blurvPipeline->setDescriptorManager(blurvDescriptorManager);
        blurhPipeline->setDescriptorManager(blurhDescriptorManager);

        blurhPipeline->init();
        blurvPipeline->init();

        auto blurhDescriptorSet = blurhDescriptorManager->createDescriptorSet(blurhPool, blurhDescriptorLayout);
        auto blurvDescriptorSet = blurvDescriptorManager->createDescriptorSet(blurvPool, blurvDescriptorLayout);

        if (!blurhDescriptorSet->init() || !blurvDescriptorSet->init())
        {
            std::cerr << "Failed to initialize descriptor sets for post processing." << std::endl;
            return -1;
        }

        // Add the post process pipelines to the post pass

        postPass->addPipeline({ blurhPipeline.get(), blurhDescriptorSet.get(), 0 });
        postPass->addPipeline({ blurvPipeline.get(), blurvDescriptorSet.get(), 0 });

        // Add object data to the scene pass

        scenePass->setIndexBuffer(indexBuffer.get());
        scenePass->setVertexBuffer(vertexBuffer.get());
        scenePass->setIndices(indices);
        scenePass->setVertices(vertices);
        scenePass->setPipeline(pipeline.get());

        // Let device manage these resources
        device->manageResource(pipeline);
        device->manageResource(vertexBuffer);
        device->manageResource(indexBuffer);
        device->manageResource(vertexShader);
        device->manageResource(fragmentShader);

        while (!window->isRequestedQuit())
        {
            window->update();
            if (window->isValid())
            {
                auto screenWidth = static_cast<float>(window->getWidth());
                auto screenHeight = static_cast<float>(window->getHeight());

                float blurHValue = 1.0f / screenWidth;
                float blurVValue = 1.0f / screenHeight;

                blurhPConstant->set(&blurHValue, sizeof(float), 0);
                blurvPConstant->set(&blurVValue, sizeof(float), 0);

                VkCommandBuffer commandBuffer = renderer->beginFrame();
                if (commandBuffer != VK_NULL_HANDLE)
                {


                    renderer->setViewport(viewport);
                    renderer->getPipeline().addStep(clearPass);
                    renderer->getPipeline().addStep(scenePass);
                    renderer->getPipeline().addStep(postPass);
                    renderer->renderFrame();
                    renderer->endFrame();
                }
                else
                {
                    renderer->advanceFrame();
                }
            } else
            {
                break;
            }
        }

        blurhDescriptorManager.reset();
        blurvDescriptorManager.reset();
        delete blurhPConstant;
        delete blurvPConstant;
    }
    renderer.reset();
    window.reset();
    device.reset();
    SDL_Quit();
    return 0;
}
