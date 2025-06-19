//
// Created by lepag on 6/19/2025.
//

#pragma once

#include <SDL3/SDL.h>

#include "context/rendering_device.h"
#include "../utilities/singleton.h"

namespace GyroEngine
{
    class Engine : public Utils::ISingleton<Engine>
    {
        friend class ISingleton;
    public:
        ~Engine();

        bool Init();

        [[nodiscard]] Device::RenderingDevice& GetDevice()
        {
            return *m_device;
        }

        [[nodiscard]] std::shared_ptr<Device::RenderingDevice> GetDeviceSmart()
        {
            return m_device;
        }
    private:
        std::shared_ptr<Device::RenderingDevice> m_device;

        bool CreateRenderingDevice();
        void DestroyRenderingDevice();

        void StartFactories();
    };
}
