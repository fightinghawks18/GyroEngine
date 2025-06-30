//
// Created by lepag on 6/19/2025.
//

#include "engine.h"

#include "factories/mesh_factory.h"
#include "input/input.h"

namespace GyroEngine
{
    Engine::~Engine()
    {
        DestroyRenderingDevice();
        DestroyWindow();
        SDL_Quit();
    }

    bool Engine::Init()
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        {
            Logger::LogError("Failed to initialize SDL: " + std::string(SDL_GetError()));
            return false;
        }
        if (!CreateWindow())
        {
            Logger::LogError("Failed to create window.");
            return false;
        }
        if (!CreateRenderingDevice()) return false;
        StartFactories();
        StartServices();

        return true;
    }

    void Engine::Run()
    {
        while (m_window->IsWindowAlive())
        {
            if (m_closing)
            {
                break;
            }
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (m_window->HasRequestedQuit())
                {
                    m_closing = true;
                }
                m_window->Update(event);
                // Pass the event to the input system
                Platform::Input::Get().Update(event);
            }

            m_updateFunction();
        }
        Destroy();
    }

    void Engine::Destroy()
    {
        if (m_destroyFunction)
        {
            m_destroyFunction();
        }
        DestroyWindow();
        DestroyRenderingDevice();
    }

    bool Engine::CreateRenderingDevice()
    {
        m_device = std::make_unique<Device::RenderingDevice>();
        m_device->AllowDiscrete(true);
        if (!m_device->Init()) return false;
        return true;
    }

    void Engine::DestroyRenderingDevice()
    {
        if (m_device)
        {
            m_device.reset();
        }
    }

    bool Engine::CreateWindow()
    {
        m_window = std::make_shared<Platform::Window>();
        if (!m_window->Init())
        {
            std::cerr << "Failed to create window." << std::endl;
            return false;
        }

        return true;
    }

    void Engine::DestroyWindow()
    {
        if (m_window)
        {
            m_window.reset();
        }
    }

    void Engine::StartFactories()
    {
        if (!m_device)
        {
            Logger::LogError("Cannot start factories without a valid rendering device");
            return;
        }
    }

    void Engine::StartServices()
    {
        if (!m_device)
        {
            Logger::LogError("Cannot start services without a valid rendering device");
            return;
        }
    }
}
