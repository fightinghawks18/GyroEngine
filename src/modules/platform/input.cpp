//
// Created by tyler on 6/24/25.
//

#include "input.h"

namespace GyroEngine::Platform
{
    void Input::Update(SDL_Event& event)
    {
    }

    bool Input::IsKeyDown(KeyMap key)
    {
    }

    bool Input::IsMouseButtonDown(MouseMap button)
    {
    }

    void Input::GetMousePosition(int32_t& x, int32_t& y)
    {
    }

    void Input::SetMousePosition(int32_t x, int32_t y)
    {
    }

    bool Input::UpdateKeyboard(SDL_Event& event)
    {
        return true;
    }

    bool Input::UpdateMouse(const SDL_Event& event)
    {

    }

    bool Input::UpdateGamepad(SDL_Event& event)
    {
    }
}
