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
    shaderutils::compileShaderToFile(utils::getExecutableDir() + "/content/shaders/simple_object.vert",
                                     shaderc_vertex_shader);
    shaderutils::compileShaderToFile(utils::getExecutableDir() + "/content/shaders/simple_object.frag",
                                     shaderc_fragment_shader);
    shaderutils::compileShaderToFile(utils::getExecutableDir() + "/content/shaders/blur_h.frag",
                                     shaderc_fragment_shader);
    shaderutils::compileShaderToFile(utils::getExecutableDir() + "/content/shaders/blur_v.frag",
                                     shaderc_fragment_shader);
    shaderutils::compileShaderToFile(utils::getExecutableDir() + "/content/shaders/fullscreen_quad.vert",
                                     shaderc_vertex_shader);
    shaderutils::compileShaderToFile(utils::getExecutableDir() + "/content/shaders/invert_color.frag",
                                     shaderc_vertex_shader);

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


    while (!window->isRequestedQuit())
    {
        window->update();
        if (window->isValid())
        {
            if (renderer->beginFrame())
            {
                Viewport viewport{};
                renderer->bindViewport(viewport);
                renderer->beginRendering();
                renderer->endRendering();
                renderer->endFrame();
            } else
            {
                renderer->advanceFrame();
            }
        } else
        {
            break;
        }
    }
    renderer.reset();
    window.reset();
    device.reset();
    SDL_Quit();
    return 0;
}
