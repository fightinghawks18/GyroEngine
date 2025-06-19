//
// Created by lepag on 6/19/2025.
//

#include "engine.h"

#include "factories/mesh_factory.h"

namespace GyroEngine
{
    Engine::~Engine()
    {
        DestroyRenderingDevice();
        SDL_Quit();
    }

    bool Engine::Init()
    {
        if (!CreateRenderingDevice()) return false;
        StartFactories();
        return true;
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

    void Engine::StartFactories()
    {
        if (!m_device)
        {
            Logger::LogError("Cannot start factories without a valid rendering device");
            return;
        }
    }
}
