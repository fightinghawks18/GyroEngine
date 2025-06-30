//
// Created by tyler on 6/24/25.
//

#include "input.h"
#include "inputils.h"
#include "debug/logger.h"

namespace GyroEngine::Platform
{
    Input::Input()
    {
        // Add all keyboard keys to the key map
        for (auto i = static_cast<uint32_t>(KeyCode::A); i <= static_cast<uint32_t>(KeyCode::MediaPlayPause); ++i) {
            auto key = static_cast<KeyCode>(i);
            m_keyMap[key] = KeyData{false, 0}; // or your desired default value
        }

        // Add all mouse events to the mouse map
        for (auto i = static_cast<uint32_t>(MouseEvent::Motion); i <= static_cast<uint32_t>(MouseEvent::WheelDown); ++i) {
            auto event = static_cast<MouseEvent>(i);
            m_mouseMap[event] = KeyData{false, 0}; // or your desired default value
        }

        // Add all gamepad buttons to the gamepad map
        for (auto i = static_cast<uint32_t>(KeyCode::GamepadA); i <= static_cast<uint32_t>(KeyCode::GamepadDPadRight); ++i) {
            auto button = static_cast<KeyCode>(i);
            m_gamepadMap[button] = KeyData{false, 0}; // or your desired default value
        }

        // Add all gamepad axes to the gamepad axis map
        m_gamepadAxisMap[KeyCode::GamepadLeftTrigger] = KeyData{0.0f, 0};
        m_gamepadAxisMap[KeyCode::GamepadRightTrigger] = KeyData{0.0f, 0};

        // Add all XR buttons to the XR key map
        for (auto i = static_cast<uint32_t>(XRKeyCode::VrButtonA); i <= static_cast<uint32_t>(XRKeyCode::VrButtonJoystick); ++i) {
            auto button = static_cast<XRKeyCode>(i);
            m_xrKeyMap[button] = KeyData{false, 0}; // or your desired default value
        }
    }

    bool Input::IsKeyDown(const KeyCode key)
    {
        return GetKeyState(key) == InputState::Pressed;
    }

    bool Input::IsGamepadButtonDown(const KeyCode button)
    {
        return GetKeyState(button) == InputState::Pressed;
    }

    bool Input::IsXRButtonDown(const XRKeyCode button)
    {
        return GetXRKeyState(button) == InputState::Pressed;
    }

    void Input::Update(const SDL_Event& event)
    {
        switch (event.type)
        {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            {
                const KeyCode key = GetKeyCodeFromScancode(event.key.scancode);
                if (key != KeyCode::Unknown)
                {
                    auto& [value, timestamp] = m_keyMap[key];
                    value = event.type == SDL_EVENT_KEY_DOWN;
                    timestamp = event.key.timestamp;
                }
                break;
            }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                const KeyCode button = GetMouseButtonFromSDL(event.button);
                if (button != KeyCode::Unknown)
                {
                    auto& [value, timestamp] = m_keyMap[button];
                    value = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
                    timestamp = event.button.timestamp;
                }
            }
        case SDL_EVENT_MOUSE_MOTION:
            {
                m_mouseInfo.x = event.motion.x;
                m_mouseInfo.y = event.motion.y;
                break;
            }
        case SDL_EVENT_MOUSE_WHEEL:
            {
                m_mouseInfo.wheel += event.wheel.y;
                break;
            }
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            {
                const KeyCode button = GetKeyCodeFromGamepadButton(event.gbutton);
                if (button != KeyCode::Unknown)
                {
                    auto& [value, timestamp] = m_gamepadMap[button];
                    value = event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN;
                    timestamp = event.gbutton.timestamp;
                }
                break;
            }
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            {
                const KeyCode axis = GetKeyCodeFromGamepadAxis(event.gaxis);
                if (axis != KeyCode::Unknown)
                {
                    auto& [value, timestamp] = m_gamepadMap[axis];
                    value = static_cast<float>(event.gaxis.value) / 32767.0f; // Divide by max int16_t value to normalize
                    timestamp = event.gaxis.timestamp;
                }
                break;
            }
        default: break;
        }
    }

    void Input::SetMousePosition(const glm::vec2 position)
    {
        SDL_WarpMouseInWindow(nullptr, position.x, position.y);
        Get().m_mouseInfo.x = position.x;
        Get().m_mouseInfo.y = position.y;
    }

    bool Input::HasFocus()
    {
        return true;
    }

    glm::vec2 Input::GetMousePosition()
    {
        auto mouseInfo = Get().m_mouseInfo;
        return {mouseInfo.x, mouseInfo.y};
    }
}
