//
// Created by tyler on 6/24/25.
//

#pragma once

#include <vector>
#include <cstdint>
#include <SDL3/SDL.h>

#include "singleton.h"

namespace GyroEngine::Platform
{
    enum class InputDeviceType : uint16_t
    {
        KEYBOARD = 0,
        MOUSE = 1,
        GAMEPAD = 2
    };

    enum class InputState : uint16_t
    {
        RELEASED = 0,
        PRESSED = 1
    };

    enum class MouseMap : uint16_t
    {
        MOUSE_UNKNOWN = 0,
        MOUSE_LEFT = 1,
        MOUSE_MIDDLE = 2,
        MOUSE_RIGHT = 3
    };

    enum class GamepadMap : uint16_t
    {
        BUTTON_A = 0,
    };

    enum class KeyMap : uint16_t
    {
        KEY_UNKNOWN = 0,
        KEY_ESCAPE = 1,
        KEY_W = 2,
        KEY_A = 3,
        KEY_S = 4,
        KEY_D = 5,
        KEY_SPACE = 6,
    };

    struct MouseEvent
    {
        MouseMap button;
        InputState state;
        int32_t x;
        int32_t y;
        int32_t wheel;
    };

    struct KeyEvent
    {
        KeyMap key;
        InputState state;
    };

    struct GamepadEvent
    {
        GamepadMap button;
        InputState state;
        int gamepad;
    };

    struct InputEvent
    {
        InputDeviceType inputType;
        union
        {
            KeyEvent keyboard;
            MouseEvent mouse;
            GamepadEvent gamepad;
        };
    };

    class Input : Utils::ISingleton<Input>
    {
        static void Update(SDL_Event& event);
        static bool IsKeyDown(KeyMap key);
        static bool IsMouseButtonDown(MouseMap button);
        static void GetMousePosition(int32_t& x, int32_t& y);
        static void SetMousePosition(int32_t x, int32_t y);
    private:
        static float m_mouseX;
        static float m_mouseY;
        static std::vector<uint16_t> m_keyboardState;
        static std::vector<uint16_t> m_mouseState;
        static std::vector<uint16_t> m_gamepadState;

        static bool UpdateKeyboard(SDL_Event& event);
        static bool UpdateMouse(const SDL_Event& event);
        static bool UpdateGamepad(SDL_Event& event);
    };
}
