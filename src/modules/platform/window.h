//
// Created by lepag on 6/7/2025.
//

#pragma once

#include <stdexcept>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>


namespace GyroEngine
{
    class Window {
    public:
        Window();
        ~Window();

        bool Init();
        void Destroy();

        void Update(const SDL_Event& event);

        [[nodiscard]] bool IsWindowAlive() const {
            return m_window != nullptr;
        }

        [[nodiscard]] bool HasRequestedQuit() const {
            return m_requestedQuit;
        }

        [[nodiscard]] bool IsFullscreen() const {
            return SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN;
        }

        [[nodiscard]] SDL_Window* GetWindowHandle() const {
            return m_window;
        }

        [[nodiscard]] uint32_t GetWindowWidth() const {
            int width, height;
            SDL_GetWindowSize(m_window, &width, &height);
            return static_cast<uint32_t>(width);
        }

        [[nodiscard]] uint32_t GetWindowHeight() const {
            int width, height;
            SDL_GetWindowSize(m_window, &width, &height);
            return static_cast<uint32_t>(height);
        }
    private:
        SDL_Window* m_window{};
        bool m_requestedQuit = false;
    };
}
