//
// Created by lepag on 6/7/2025.
//

#pragma once

#include <stdexcept>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>


class Window {
public:
    Window();
    ~Window();

    bool create();
    void destroy();

    void update();

    [[nodiscard]] bool isValid() const {
        return m_window != nullptr;
    }

    [[nodiscard]] bool isRequestedQuit() const {
        return m_requestedQuit;
    }

    [[nodiscard]] bool isFullscreen() const {
        return SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN;
    }

    [[nodiscard]] SDL_Window* getWindow() const {
        return m_window;
    }

    [[nodiscard]] uint32_t getWidth() const {
        int width, height;
        SDL_GetWindowSize(m_window, &width, &height);
        return static_cast<uint32_t>(width);
    }

    [[nodiscard]] uint32_t getHeight() const {
        int width, height;
        SDL_GetWindowSize(m_window, &width, &height);
        return static_cast<uint32_t>(height);
    }
private:
    SDL_Window* m_window = nullptr;
    bool m_requestedQuit = false;


};
