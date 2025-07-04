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
        void Run();
        void Close() { m_closing = true; }
        void Destroy();
        void SetDestroyFunction(const std::function<void()>& destroyFunc) { m_destroyFunction = destroyFunc; }
        void SetUpdateFunction(const std::function<void()>& updateFunc) { m_updateFunction = updateFunc; }

        [[nodiscard]] Device::RenderingDevice& GetDevice()
        {
            return *m_device;
        }

        [[nodiscard]] std::shared_ptr<Device::RenderingDevice> GetDeviceSmart()
        {
            return m_device;
        }

        [[nodiscard]] std::shared_ptr<Platform::Window> GetWindow() const
        {
            return m_window;
        }

        static Device::RenderingDevice& SGetDevice()
        {
            return Get().GetDevice();
        }
        static std::shared_ptr<Device::RenderingDevice> SGetDeviceSmart()
        {
            return Get().GetDeviceSmart();
        }
        static std::shared_ptr<Platform::Window> SGetWindow()
        {
            return Get().GetWindow();
        }
    private:
        std::shared_ptr<Device::RenderingDevice> m_device;
        std::shared_ptr<Platform::Window> m_window;

        std::function<void()> m_destroyFunction;
        std::function<void()> m_updateFunction;

        bool m_closing = false;

        bool CreateRenderingDevice();
        void DestroyRenderingDevice();

        bool BuildWindow();
        void DestroyWindow();

        void StartFactories();
        void StartServices() const;
    };
}
