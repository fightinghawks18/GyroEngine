//
// Created by lepag on 6/7/2025.
//

#include "window.h"


Window::Window()
{}

Window::~Window()
{
    Destroy();
}

bool Window::Init()
{
    m_window = SDL_CreateWindow("Gyro", 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }
    return true;
}

void Window::Destroy()
{
    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    m_requestedQuit = false;
}

void Window::Update()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            m_requestedQuit = true;
        }
    }
}
