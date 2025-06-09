//
// Created by lepag on 6/8/2025.
//

#include <iostream>

#include <SDL3/SDL.h>

#include "window.h"
#include "context/rendering_device.h"
#include "rendering/renderer.h"

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }
    {
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

        Viewport viewport{0};
        viewport.width = 0.5f;
        viewport.height = 0.5f;

        while (!window->isRequestedQuit())
        {
            window->update();
            if (window->isValid())
            {
                VkCommandBuffer commandBuffer = renderer->beginFrame();
                if (commandBuffer != VK_NULL_HANDLE)
                {
                    renderer->setViewport(viewport);
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