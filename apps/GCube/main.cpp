//
// Created by lepag on 6/8/2025.
//

#include <iostream>

#include <SDL3/SDL.h>

#include "window.h"
#include "context/rendering_device.h"
#include "rendering/renderer.h"
#include "rendering/render_steps/clear_step.h"
#include "rendering/render_steps/scene_step.h"
#include "resources/shader.h"
#include "utilities/shader.h"

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }
    {
        // Compile shaders first
        shaderutils::compileShaderToFile("content/shaders/simple_object.vert", shaderc_vertex_shader);
        shaderutils::compileShaderToFile("content/shaders/simple_object.frag", shaderc_fragment_shader);

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
        auto renderer = std::make_unique<Renderer>(*device);
        if (!renderer->init(window.get()))
        {
            std::cerr << "Failed to initialize renderer." << std::endl;
            return -1;
        }

        std::vector<types::Vertex> vertices = {
            {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Bottom vertex (red)
            {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // Right vertex (green)
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}  // Left vertex (blue)
        };

        std::vector<uint32_t> indices = {
            0, 1, 2 // Triangle
        };

        auto clearPass = std::make_shared<ClearStep>();
        auto scenePass = std::make_shared<SceneStep>();

        Viewport viewport{0};
        viewport.width = 0.5f;
        viewport.height = 0.5f;


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

        auto vertexShader = std::make_shared<Shader>(*device);
        auto fragmentShader = std::make_shared<Shader>(*device);

        vertexShader->setShaderPath("content/shaders/simple_object.vert.spv")
                    .init();
        fragmentShader->setShaderPath("content/shaders/simple_object.frag.spv")
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

        pipelineConfig.vertexInputState.addAttribute(
            0,
            0,
            sizeof(types::Vertex),
            offsetof(types::Vertex, position),
            VK_VERTEX_INPUT_RATE_VERTEX, VK_FORMAT_R32G32B32_SFLOAT);
        pipelineConfig.vertexInputState.addAttribute(
            0,
            1,
            sizeof(types::Vertex),
            offsetof(types::Vertex, color),
            VK_VERTEX_INPUT_RATE_VERTEX, VK_FORMAT_R32G32B32_SFLOAT);

        pipelineConfig.shaderStages.push_back(vertexShaderStage);
        pipelineConfig.shaderStages.push_back(fragmentShaderStage);

        pipeline->init();

        scenePass->setIndexBuffer(indexBuffer.get());
        scenePass->setVertexBuffer(vertexBuffer.get());
        scenePass->setIndices(indices);
        scenePass->setVertices(vertices);
        scenePass->setPipeline(pipeline.get());

        while (!window->isRequestedQuit())
        {
            window->update();
            if (window->isValid())
            {
                VkCommandBuffer commandBuffer = renderer->beginFrame();
                if (commandBuffer != VK_NULL_HANDLE)
                {
                    renderer->setViewport(viewport);
                    renderer->getPipeline().addStep(clearPass);
                    renderer->getPipeline().addStep(scenePass);
                    renderer->renderFrame();
                    renderer->endFrame();
                } else
                {
                    renderer->advanceFrame();
                }
            }
        }
    }
    SDL_Quit();
    return 0;
}
