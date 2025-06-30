//
// Created by tyler on 6/24/25.
//

#pragma once

#include <vector>
#include <cstdint>

#include <unordered_map>
#include <variant>
#define GLM_ENABLE_EXPERIMENTAL
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_gamepad.h>

#include "singleton.h"

namespace GyroEngine::Platform
{
    enum class InputType : uint32_t
    {
        Desktop = 0,
        Mobile,
        Controller,
        XR,
    };

    enum class InputState : uint32_t
    {
        Pressed = 0,
        Released,
    };

    enum class KeyCode : uint32_t
    {
        Unknown = 0,

        // Printable keys
        A, B, C, D, E, F, G, H, I, J, K, L,
        M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        // Numeric keys
        One, Two, Three, Four, Five,
        Six, Seven, Eight, Nine, Zero,

        // Function keys
        F1, F2, F3, F4, F5, F6, F7, F8,
        F9, F10, F11, F12,

        // Special keys
        Escape, Enter, Space, Backspace, Tab,
        LeftShift, RightShift, LeftControl, RightControl,
        LeftAlt, RightAlt, LeftMeta, RightMeta,
        PrintScreen, ScrollLock, Pause,
        Insert, Home, PageUp, Delete, End,
        PageDown, CapsLock, NumLock,
        LeftBracket, RightBracket, Semicolon, Apostrophe,
        GraveAccent, Menu, LeftSuper, RightSuper,
        Comma, Period, Slash, Backslash,
        Equals, Minus,

        // Arrow keys
        Up, Down, Left, Right,

        // Numpad keys
        Numpad0, Numpad1, Numpad2, Numpad3,
        Numpad4, Numpad5, Numpad6, Numpad7,
        Numpad8, Numpad9, NumpadDecimal, NumpadDivide,
        NumpadMultiply, NumpadSubtract, NumpadAdd, NumpadEnter,
        NumpadEqual,

        // Media keys
        VolumeUp, VolumeDown, Mute,
        MediaNext, MediaPrevious, MediaStop, MediaPlayPause,

        // Gamepad keys
        GamepadA, GamepadB, GamepadX, GamepadY,
        GamepadLeftBumper, GamepadRightBumper,
        GamepadLeftTrigger, GamepadRightTrigger,
        GamepadBack, GamepadStart,
        GamepadLeftStick, GamepadRightStick,
        GamepadDPadUp, GamepadDPadDown,
        GamepadDPadLeft, GamepadDPadRight,

        // Mouse buttons
        MouseButtonLeft, MouseButtonRight, MouseButtonMiddle,
        MouseButtonX1, MouseButtonX2,
    };

    enum class MouseEvent : uint32_t
    {
        Unknown = 0,

        // Mouse motion events
        Motion,

        // Mouse wheel events
        WheelUp,
        WheelDown,
    };

    enum class XRKeyCode : uint32_t
    {
        Unknown = 0,

        // VR controller buttons
        VrButtonA, VrButtonB, VrButtonX, VrButtonY,
        VrButtonGrip, VrButtonTrigger, VrButtonMenu,
        VrButtonSystem, VrButtonTouchpad, VrButtonJoystick,
    };

    using KeyValue = std::variant<int, bool, float, glm::vec2>;

    struct KeyData
    {
        KeyValue value;
        uint32_t timestamp;
    };

    struct MouseInfo
    {
        float x;
        float y;
        float wheel;
    };

    class Input : public Utils::ISingleton<Input>
    {
        friend class ISingleton;

    public:
        Input();

        [[nodiscard]] static bool IsKeyRegistered(const KeyCode key)
        {
            return Get().m_keyMap.find(key) != Get().m_keyMap.end();
        }

        [[nodiscard]] static bool IsMouseEventRegistered(const MouseEvent button)
        {
            return Get().m_mouseMap.find(button) != Get().m_mouseMap.end();
        }

        [[nodiscard]] static bool IsGamepadButtonRegistered(const KeyCode button)
        {
            return Get().m_gamepadMap.find(button) != Get().m_gamepadMap.end();
        }

        [[nodiscard]] static bool IsXRButtonRegistered(const XRKeyCode button)
        {
            return Get().m_xrKeyMap.find(button) != Get().m_xrKeyMap.end();
        }

        [[nodiscard]] static InputState GetKeyState(const KeyCode key)
        {
            auto it = Get().m_keyMap.find(key);
            if (it != Get().m_keyMap.end())
            {
                return GetInputStateFromValue(it->second.value);
            }
            return InputState::Released; // Default state if not found
        }

        [[nodiscard]] static InputState GetXRKeyState(const XRKeyCode key)
        {
            auto it = Get().m_xrKeyMap.find(key);
            if (it != Get().m_xrKeyMap.end())
            {
                return GetInputStateFromValue(it->second.value);
            }
            return InputState::Released; // Default state if not found
        }

        template <typename T>
        [[nodiscard]] static T GetKeyValue(const KeyCode key)
        {
            // Check if the key is registered in the key map
            if (const auto it = Get().m_keyMap.find(key); it != Get().m_keyMap.end())
            {
                if (std::holds_alternative<T>(it->second.value))
                    return std::get<T>(it->second.value);
                throw std::runtime_error("Key found, but value type mismatch");
            }

            // Search gamepad map if not found in key map
            if (const auto it = Get().m_gamepadMap.find(key); it != Get().m_gamepadMap.end())
            {
                if (std::holds_alternative<T>(it->second.value))
                    return std::get<T>(it->second.value);
                throw std::runtime_error("Gamepad key found, but value type mismatch");
            }
            throw std::runtime_error("Key not found in input map");
        }

        [[nodiscard]] static bool IsKeyDown(KeyCode key);
        [[nodiscard]] static bool IsGamepadButtonDown(KeyCode button);
        [[nodiscard]] static bool IsXRButtonDown(XRKeyCode button);

        void Update(const SDL_Event& event);

        static void SetMousePosition(glm::vec2 position);

        [[nodiscard]] static bool HasFocus();

        [[nodiscard]] static glm::vec2 GetMousePosition();

    private:
        std::unordered_map<KeyCode, KeyData> m_keyMap;
        std::unordered_map<MouseEvent, KeyData> m_mouseMap;
        std::unordered_map<KeyCode, KeyData> m_gamepadMap;
        std::unordered_map<KeyCode, KeyData> m_gamepadAxisMap;
        std::unordered_map<XRKeyCode, KeyData> m_xrKeyMap;
        MouseInfo m_mouseInfo = {};

        static InputState GetInputStateFromValue(const KeyValue& value)
        {
            if (std::holds_alternative<int>(value))
            {
                return std::get<int>(value) > 0 ? InputState::Pressed : InputState::Released;
            }
            if (std::holds_alternative<float>(value))
            {
                return std::get<float>(value) > 0.0f ? InputState::Pressed : InputState::Released;
            }
            if (std::holds_alternative<bool>(value))
            {
                return std::get<bool>(value) ? InputState::Pressed : InputState::Released;
            }
            if (std::holds_alternative<glm::vec2>(value))
            {
                const auto vec2 = std::get<glm::vec2>(value);
                return (vec2.x > 0 || vec2.y > 0) ? InputState::Pressed : InputState::Released;
            }
            return InputState::Released; // Default state if not applicable
        }
    };
}
