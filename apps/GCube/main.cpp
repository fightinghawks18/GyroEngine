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
    shaderutils::CompileShaderToFile(utils::GetExecutableDir() + "/content/shaders/simple_object.vert",
                                     shaderc_vertex_shader);
    shaderutils::CompileShaderToFile(utils::GetExecutableDir() + "/content/shaders/simple_object.frag",
                                     shaderc_fragment_shader);
    shaderutils::CompileShaderToFile(utils::GetExecutableDir() + "/content/shaders/blur_h.frag",
                                     shaderc_fragment_shader);
    shaderutils::CompileShaderToFile(utils::GetExecutableDir() + "/content/shaders/blur_v.frag",
                                     shaderc_fragment_shader);
    shaderutils::CompileShaderToFile(utils::GetExecutableDir() + "/content/shaders/fullscreen_quad.vert",
                                     shaderc_vertex_shader);
    shaderutils::CompileShaderToFile(utils::GetExecutableDir() + "/content/shaders/invert_color.frag",
                                     shaderc_vertex_shader);

    auto window = std::make_unique<Window>();
    if (!window->Init())
    {
        std::cerr << "Failed to create window." << std::endl;
        return -1;
    }

    auto device = std::make_unique<RenderingDevice>();
    if (!device->Init())
    {
        std::cerr << "Failed to initialize rendering device." << std::endl;
        return -1;
    }
    auto renderer = std::make_shared<Renderer>(*device);
    if (!renderer->Init(window.get()))
    {
        std::cerr << "Failed to initialize renderer." << std::endl;
        return -1;
    }


    while (!window->HasRequestedQuit())
    {
        window->Update();
        if (window->IsWindowAlive())
        {
            if (renderer->RecordFrame())
            {
                Viewport viewport{};
                renderer->BindViewport(viewport);
                renderer->StartRender();
                renderer->EndRender();
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
    renderer.reset();
    window.reset();
    device.reset();
    SDL_Quit();
    return 0;
}
