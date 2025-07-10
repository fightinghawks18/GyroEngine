//
// Created by lepag on 6/19/2025.
//

#include "engine.h"

#include "factories/mesh_factory.h"

#include "input/keyboard.h"
#include "input/mouse.h"

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
        if (!BuildWindow())
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
        // Keep engine alive as long as the window is alive
        // ^ This will the first way the engine can exit
        while (m_window->IsWindowAlive())
        {
            // If the window needs to quit, or we want the engine to close, exit the loop prematurely
            if (m_window->HasRequestedQuit() || m_closing)
            {
                break;
            }

            // Reset input
            Input::Mouse::Get().ResetDelta();
            Input::Keyboard::Get().ResetFrame();

            // Start updating SDL objects
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                // Handle window events
                m_window->Update(event);

                // Pass the event to the input system
                Input::Keyboard::Get().Update(event);
                Input::Mouse::Get().Update(event);
            }

            // Run the update function if set
            m_updateFunction();
        }

        // Destroy the engine resources after the loop ends
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

    bool Engine::BuildWindow()
    {
        m_window = std::make_shared<Window>();
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

    void Engine::StartServices() const
    {
        if (!m_device)
        {
            Logger::LogError("Cannot start services without a valid rendering device");
            return;
        }

        Input::Keyboard::Get().Init();
        Input::Mouse::Get().Init();
    }
}
