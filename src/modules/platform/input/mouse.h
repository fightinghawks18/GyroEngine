//
// Created by lepag on 7/10/2025.
//

#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <glm/vec2.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>

#include "singleton.h"

namespace GyroEngine::Input
{
    enum class MouseButton : uint32_t
    {
        Unknown = 0,
        Left,
        Right,
        Middle,
        X1,
        X2,
    };

    enum class MouseEvent
    {
        Motion,
        ScrollDown,
        ScrollUp,
        Enter,
        Leave,
    };

    struct ButtonData
    {
        uint32_t timestamp = 0;
        bool pressed = false;
    };

    enum class MouseMode
    {
        Free,
        Locked,
        Confined
    };

    class Mouse : public Utils::ISingleton<Mouse>
    {
        friend class ISingleton;
    public:
        void Init();
        void Update(const SDL_Event& event);
        void SetMode(MouseMode mode);
        void SetVisible(bool visible);
        void ResetDelta() { m_delta = glm::vec2(0.0f, 0.0f); }
        void Move(float x, float y);
        void Center();
        void SetSensitivity(float sensitivity);

        static bool IsButtonDown(const MouseButton button) { return Get().m_buttonMap[button].pressed; }
        static void HookMouseEvent(MouseEvent event, const std::function<void()>& callback);
        static void SetMouseSensitivity(const float sensitivity) { Get().SetSensitivity(sensitivity); }
        static void SetMouseMode(const MouseMode mode) { Get().SetMode(mode); }
        static void SetCursorVisible(const bool visible) { Get().SetVisible(visible); }
        static void MoveMouse(const float x, const float y)
        {
            Get().Move(x, y);
        }
        static void CenterMouse() { Get().Center(); }


        /// \brief Returns user defined sensitivity
        /// \note 1.0 is 100% sensitivity but internally is 0.01
        static float GetUserSensitivity() { return Get().m_mouseSensitivity * 100.0f; }
        /// \brief Returns the real value of sensitivity
        /// \note This value is the user defined sensitivity scaled down for mouse delta movement (1.0 -> 0.01)
        static float GetSensitivity()
        {
            return Get().m_mouseSensitivity;
        }
        static glm::vec2 GetPosition() { return Get().m_position; }
        static glm::vec2 GetDelta() { return Get().m_delta; }
        static bool IsRelative() { return Get().m_relative; }
        static MouseMode GetMode() { return Get().m_mouseMode; }
    private:
        std::unordered_map<MouseButton, ButtonData> m_buttonMap;
        std::unordered_map<MouseEvent, std::vector<std::weak_ptr<std::function<void()>>>> m_eventHooks;

        float m_mouseSensitivity = 1.0f;
        bool m_showCursor = true;
        MouseMode m_mouseMode = MouseMode::Free;

        glm::vec2 m_position{0.0f, 0.0f};
        glm::vec2 m_delta{0.0f, 0.0f};
        bool m_focused = false;
        bool m_relative = false;

        void UpdateButton(MouseButton mouseButton, const SDL_MouseButtonEvent& event);
        void UpdateEvent(MouseEvent mouseEvent, const SDL_Event& event);

        static MouseButton ScancodeToMouseButton(const SDL_MouseButtonEvent& scancode)
        {
            switch (scancode.button)
            {
                case SDL_BUTTON_LEFT: return MouseButton::Left;
                case SDL_BUTTON_RIGHT: return MouseButton::Right;
                case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
                case SDL_BUTTON_X1: return MouseButton::X1;
                case SDL_BUTTON_X2: return MouseButton::X2;
                default: return MouseButton::Unknown;
            }
        }
    };

}
