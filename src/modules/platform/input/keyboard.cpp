//
// Created by tyler on 7/3/25.
//

#include "keyboard.h"

namespace GyroEngine::Platform
{
    void Keyboard::Init()
    {
        // Populate keyboard map
        for (auto k = static_cast<uint32_t>(Key::Unknown); k < static_cast<uint32_t>(Key::MediaPlayPause); k++)
        {
            m_keyMap[static_cast<Key>(k)] = {};
        }
    }

    void Keyboard::Update(const SDL_Event& event)
    {
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

    void Keyboard::UpdateKey(const SDL_KeyboardEvent& event)
    {
        const auto keyCode = ScancodeToKey(event.scancode);
        if (keyCode == Key::Unknown)
        {
            return;
        }

        auto& [timestamp, pressed] = m_keyMap[keyCode];
        pressed = event.type == SDL_EVENT_KEY_DOWN;
    }
}
