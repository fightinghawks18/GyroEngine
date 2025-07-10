//
// Created by lepag on 7/10/2025.
//

#include "mouse.h"

namespace GyroEngine::Input
{
    void Mouse::Init()
    {
        // Populate mouse button map
        // Starting from the first enum to the last enum
        for (auto k = static_cast<uint32_t>(MouseButton::Unknown); k < static_cast<uint32_t>(MouseButton::X2); k++)
        {
            m_buttonMap[static_cast<MouseButton>(k)] = {};
        }
    }

    void Mouse::Update(const SDL_Event &event)
    {
        // Listen for mouse events/buttons and update
        switch (event.type)
        {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                {
                    UpdateButton(ScancodeToMouseButton(event.button), event.button);
                }
                break;
            case SDL_EVENT_MOUSE_MOTION:
                {
                    UpdateEvent(MouseEvent::Motion, event);
                }
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                {
                    if (event.wheel.y > 0)
                    {
                        UpdateEvent(MouseEvent::ScrollUp, event);
                    }
                    else if (event.wheel.y < 0)
                    {
                        UpdateEvent(MouseEvent::ScrollDown, event);
                    }
                }
                break;
            case SDL_EVENT_WINDOW_MOUSE_ENTER:
                UpdateEvent(MouseEvent::Enter, event);
                m_focused = true;
                break;
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
                UpdateEvent(MouseEvent::Leave, event);
                m_focused = false;
                break;
            default:
                break;
        }

        // Get window from id
        SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);

        // Update cursor traits
        // ^ Only do this if the window isn't using relative mouse mode since SDL3 hides it automatically
        if (!SDL_GetWindowRelativeMouseMode(window))
        {
            if (m_showCursor && !SDL_CursorVisible())
            {
                SDL_ShowCursor();
            }
            else if (!m_showCursor && SDL_CursorVisible())
            {
                SDL_HideCursor();
            }
        }

        if (m_mouseMode == MouseMode::Locked)
        {
            SDL_SetWindowRelativeMouseMode(window, true); // Lock the mouse to the window
        } else if (m_mouseMode == MouseMode::Confined)
        {
            SDL_SetWindowRelativeMouseMode(window, false); // Confine the mouse to the window
            SDL_SetWindowMouseGrab(window, true); // Grab the window
        } else
        {
            SDL_SetWindowRelativeMouseMode(window, false); // Free the mouse
            SDL_SetWindowMouseGrab(window, false); // Release the window grab
        }

        bool wasRelative = m_relative;
        m_relative = SDL_GetWindowRelativeMouseMode(window);

        if (wasRelative && !m_relative)
        {
            // Warp the mouse to the center of the window when leaving relative mode
            Center();
        }
    }

    void Mouse::SetMode(const MouseMode mode)
    {
        if (m_mouseMode != mode)
        {
            m_mouseMode = mode;
        }
    }

    void Mouse::SetVisible(bool visible)
    {
        m_showCursor = visible;
    }

    void Mouse::Move(const float x, const float y)
    {
        // Move the mouse to the specified position
        SDL_Window* window = SDL_GetMouseFocus();
        if (window)
        {
            SDL_WarpMouseInWindow(window, static_cast<int>(x), static_cast<int>(y));
            m_position = glm::vec2(x, y); // Update the position
            m_delta = glm::vec2(0.0f, 0.0f); // Reset delta since we moved the mouse
        }
    }

    void Mouse::Center()
    {
        SDL_Window* window = SDL_GetMouseFocus();
        if (window)
        {
            int width, height;
            SDL_GetWindowSize(window, &width, &height);
            int centerX = width / 2;
            int centerY = height / 2;
            Move(centerX, centerY);
        }
    }

    void Mouse::SetSensitivity(const float sensitivity)
    {
        m_mouseSensitivity = sensitivity * 0.01f; // Scale down sensitivity for better control
    }

    void Mouse::HookMouseEvent(const MouseEvent event, const std::function<void()>& callback)
    {
        auto& mouse = Get();

        if (mouse.m_eventHooks.find(event) == mouse.m_eventHooks.end())
        {
            mouse.m_eventHooks[event] = std::vector<std::weak_ptr<std::function<void()>>>();
        }
    }

    void Mouse::UpdateButton(const MouseButton mouseButton, const SDL_MouseButtonEvent &event)
    {
        if (m_buttonMap.find(mouseButton) == m_buttonMap.end())
        {
            return; // No button map for this button
        }

        auto& [button, pressed] = m_buttonMap[mouseButton];
        pressed = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
        button = event.timestamp;
    }

    void Mouse::UpdateEvent(const MouseEvent mouseEvent, const SDL_Event& event)
    {
        // If we entered the window, set the mouse position to the position we entered
        // ^ If the mouse is being watched (i.e: camera rotation) the rotation would be large without this
        if (mouseEvent == MouseEvent::Enter)
        {
            float x, y;
            SDL_GetMouseState(&x, &y);
            m_position = glm::vec2(x, y);
        }

        // Only update motion if the mouse is focused onto a window
        if (mouseEvent == MouseEvent::Motion && m_focused)
        {
            m_position = glm::vec2(event.motion.x, event.motion.y);

            if (m_relative)
            {
                m_delta = glm::vec2(event.motion.xrel, event.motion.yrel);
            }
        }


        if (m_eventHooks.find(mouseEvent) == m_eventHooks.end())
        {
            return; // No hooks for this event
        }

        for (const auto& weakCallback : m_eventHooks[mouseEvent])
        {
            if (auto callback = weakCallback.lock())
            {
                (*callback)();
            }
        }
    }
}
