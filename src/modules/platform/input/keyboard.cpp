//
// Created by tyler on 7/3/25.
//

#include "keyboard.h"

namespace GyroEngine::Input
{
    void Keyboard::Init()
    {
        // Populate keyboard map
        // Starting from the first enum to the last enum
        for (auto k = static_cast<uint32_t>(Key::Unknown); k < static_cast<uint32_t>(Key::MediaPlayPause); k++)
        {
            m_keyMap[static_cast<Key>(k)] = {};
        }
    }

    void Keyboard::Update(const SDL_Event& event)
    {
        // Listen for key events and update
        switch (event.type)
        {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            {
                UpdateKey(event.key);
            }
        default: break;
        }
    }

    void Keyboard::ResetFrame()
    {
        // Reset justPressed state for all keys
        // ^ Just pressed lasts for one frame
        for (auto& [key, keyData] : m_keyMap)
        {
            keyData.justPressed = false;
        }
    }

    void Keyboard::UpdateKey(const SDL_KeyboardEvent& event)
    {
        const auto keyCode = ScancodeToKey(event.scancode);
        if (keyCode == Key::Unknown)
        {
            return;
        }

        auto& [timestamp, pressed, justPressed] = m_keyMap[keyCode];
        bool wasPressed = pressed;
        pressed = event.type == SDL_EVENT_KEY_DOWN;
        timestamp = event.timestamp;

        // If we just started pressing the key this frame, set justPressed to true
        if (pressed && !wasPressed)
        {
            justPressed = true;
        }
    }
}
