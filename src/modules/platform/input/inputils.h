//
// Created by tyler on 6/29/25.
//

#pragma once

#include "input.h"
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>

namespace GyroEngine::Platform
{
    static SDL_Scancode GetSDLScancode(const KeyCode key)
    {
        switch (key)
        {
        case KeyCode::A: return SDL_SCANCODE_A;
        case KeyCode::B: return SDL_SCANCODE_B;
        case KeyCode::C: return SDL_SCANCODE_C;
        case KeyCode::D: return SDL_SCANCODE_D;
        case KeyCode::E: return SDL_SCANCODE_E;
        case KeyCode::F: return SDL_SCANCODE_F;
        case KeyCode::G: return SDL_SCANCODE_G;
        case KeyCode::H: return SDL_SCANCODE_H;
        case KeyCode::I: return SDL_SCANCODE_I;
        case KeyCode::J: return SDL_SCANCODE_J;
        case KeyCode::K: return SDL_SCANCODE_K;
        case KeyCode::L: return SDL_SCANCODE_L;
        case KeyCode::M: return SDL_SCANCODE_M;
        case KeyCode::N: return SDL_SCANCODE_N;
        case KeyCode::O: return SDL_SCANCODE_O;
        case KeyCode::P: return SDL_SCANCODE_P;
        case KeyCode::Q: return SDL_SCANCODE_Q;
        case KeyCode::R: return SDL_SCANCODE_R;
        case KeyCode::S: return SDL_SCANCODE_S;
        case KeyCode::T: return SDL_SCANCODE_T;
        case KeyCode::U: return SDL_SCANCODE_U;
        case KeyCode::V: return SDL_SCANCODE_V;
        case KeyCode::W: return SDL_SCANCODE_W;
        case KeyCode::X: return SDL_SCANCODE_X;
        case KeyCode::Y: return SDL_SCANCODE_Y;
        case KeyCode::Z: return SDL_SCANCODE_Z;
        case KeyCode::One: return SDL_SCANCODE_1;
        case KeyCode::Two: return SDL_SCANCODE_2;
        case KeyCode::Three: return SDL_SCANCODE_3;
        case KeyCode::Four: return SDL_SCANCODE_4;
        case KeyCode::Five: return SDL_SCANCODE_5;
        case KeyCode::Six: return SDL_SCANCODE_6;
        case KeyCode::Seven: return SDL_SCANCODE_7;
        case KeyCode::Eight: return SDL_SCANCODE_8;
        case KeyCode::Nine: return SDL_SCANCODE_9;
        case KeyCode::Zero: return SDL_SCANCODE_0;
        case KeyCode::F1: return SDL_SCANCODE_F1;
        case KeyCode::F2: return SDL_SCANCODE_F2;
        case KeyCode::F3: return SDL_SCANCODE_F3;
        case KeyCode::F4: return SDL_SCANCODE_F4;
        case KeyCode::F5: return SDL_SCANCODE_F5;
        case KeyCode::F6: return SDL_SCANCODE_F6;
        case KeyCode::F7: return SDL_SCANCODE_F7;
        case KeyCode::F8: return SDL_SCANCODE_F8;
        case KeyCode::F9: return SDL_SCANCODE_F9;
        case KeyCode::F10: return SDL_SCANCODE_F10;
        case KeyCode::F11: return SDL_SCANCODE_F11;
        case KeyCode::F12: return SDL_SCANCODE_F12;
        case KeyCode::Escape: return SDL_SCANCODE_ESCAPE;
        case KeyCode::Enter: return SDL_SCANCODE_RETURN;
        case KeyCode::Space: return SDL_SCANCODE_SPACE;
        case KeyCode::Backspace: return SDL_SCANCODE_BACKSPACE;
        case KeyCode::Tab: return SDL_SCANCODE_TAB;
        case KeyCode::LeftShift: return SDL_SCANCODE_LSHIFT;
        case KeyCode::RightShift: return SDL_SCANCODE_RSHIFT;
        case KeyCode::LeftControl: return SDL_SCANCODE_LCTRL;
        case KeyCode::RightControl: return SDL_SCANCODE_RCTRL;
        case KeyCode::LeftAlt: return SDL_SCANCODE_LALT;
        case KeyCode::RightAlt: return SDL_SCANCODE_RALT;
        case KeyCode::LeftMeta: return SDL_SCANCODE_LGUI;
        case KeyCode::RightMeta: return SDL_SCANCODE_RGUI;
        case KeyCode::PrintScreen: return SDL_SCANCODE_PRINTSCREEN;
        case KeyCode::ScrollLock: return SDL_SCANCODE_SCROLLLOCK;
        case KeyCode::Pause: return SDL_SCANCODE_PAUSE;
        case KeyCode::Insert: return SDL_SCANCODE_INSERT;
        case KeyCode::Home: return SDL_SCANCODE_HOME;
        case KeyCode::PageUp: return SDL_SCANCODE_PAGEUP;
        case KeyCode::Delete: return SDL_SCANCODE_DELETE;
        case KeyCode::End: return SDL_SCANCODE_END;
        case KeyCode::PageDown: return SDL_SCANCODE_PAGEDOWN;
        case KeyCode::CapsLock: return SDL_SCANCODE_CAPSLOCK;
        case KeyCode::NumLock: return SDL_SCANCODE_NUMLOCKCLEAR;
        case KeyCode::LeftBracket: return SDL_SCANCODE_LEFTBRACKET;
        case KeyCode::RightBracket: return SDL_SCANCODE_RIGHTBRACKET;
        case KeyCode::Semicolon: return SDL_SCANCODE_SEMICOLON;
        case KeyCode::Apostrophe: return SDL_SCANCODE_APOSTROPHE;
        case KeyCode::GraveAccent: return SDL_SCANCODE_GRAVE;
        case KeyCode::Menu: return SDL_SCANCODE_APPLICATION;
        case KeyCode::LeftSuper:
        case KeyCode::RightSuper: return SDL_SCANCODE_MENU;
        case KeyCode::Comma: return SDL_SCANCODE_COMMA;
        case KeyCode::Period: return SDL_SCANCODE_PERIOD;
        case KeyCode::Slash: return SDL_SCANCODE_SLASH;
        case KeyCode::Backslash: return SDL_SCANCODE_BACKSLASH;
        case KeyCode::Equals: return SDL_SCANCODE_EQUALS;
        case KeyCode::Minus: return SDL_SCANCODE_MINUS;
        case KeyCode::Up: return SDL_SCANCODE_UP;
        case KeyCode::Down: return SDL_SCANCODE_DOWN;
        case KeyCode::Left: return SDL_SCANCODE_LEFT;
        case KeyCode::Right: return SDL_SCANCODE_RIGHT;
        case KeyCode::Numpad0: return SDL_SCANCODE_KP_0;
        case KeyCode::Numpad1: return SDL_SCANCODE_KP_1;
        case KeyCode::Numpad2: return SDL_SCANCODE_KP_2;
        case KeyCode::Numpad3: return SDL_SCANCODE_KP_3;
        case KeyCode::Numpad4: return SDL_SCANCODE_KP_4;
        case KeyCode::Numpad5: return SDL_SCANCODE_KP_5;
        case KeyCode::Numpad6: return SDL_SCANCODE_KP_6;
        case KeyCode::Numpad7: return SDL_SCANCODE_KP_7;
        case KeyCode::Numpad8: return SDL_SCANCODE_KP_8;
        case KeyCode::Numpad9: return SDL_SCANCODE_KP_9;
        case KeyCode::NumpadDecimal: return SDL_SCANCODE_KP_PERIOD;
        case KeyCode::NumpadDivide: return SDL_SCANCODE_KP_DIVIDE;
        case KeyCode::NumpadMultiply: return SDL_SCANCODE_KP_MULTIPLY;
        case KeyCode::NumpadSubtract: return SDL_SCANCODE_KP_MINUS;
        case KeyCode::NumpadAdd: return SDL_SCANCODE_KP_PLUS;
        case KeyCode::NumpadEnter: return SDL_SCANCODE_KP_ENTER;
        case KeyCode::NumpadEqual: return SDL_SCANCODE_KP_EQUALS;
        case KeyCode::VolumeUp: return SDL_SCANCODE_VOLUMEUP;
        case KeyCode::VolumeDown: return SDL_SCANCODE_VOLUMEDOWN;
        case KeyCode::Mute: return SDL_SCANCODE_MUTE;
        case KeyCode::MediaNext: return SDL_SCANCODE_MEDIA_NEXT_TRACK;
        case KeyCode::MediaPrevious: return SDL_SCANCODE_MEDIA_PREVIOUS_TRACK;
        case KeyCode::MediaStop: return SDL_SCANCODE_MEDIA_STOP;
        case KeyCode::MediaPlayPause: return SDL_SCANCODE_MEDIA_PLAY_PAUSE;
        default: return SDL_SCANCODE_UNKNOWN; // Handle unknown key
        }
    }

    static KeyCode GetKeyCodeFromGamepadAxis(const SDL_GamepadAxisEvent& axis)
    {
        switch (axis.axis)
        {
        case SDL_GAMEPAD_AXIS_LEFT_TRIGGER: return KeyCode::GamepadLeftTrigger;
        case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: return KeyCode::GamepadRightTrigger;
        default: return KeyCode::Unknown; // Handle unknown axis
        }
    }

    static KeyCode GetKeyCodeFromGamepadButton(const SDL_GamepadButtonEvent& button)
    {
        switch (button.button)
        {
        case SDL_GAMEPAD_BUTTON_SOUTH: return KeyCode::GamepadA;
        case SDL_GAMEPAD_BUTTON_EAST: return KeyCode::GamepadB;
        case SDL_GAMEPAD_BUTTON_WEST: return KeyCode::GamepadX;
        case SDL_GAMEPAD_BUTTON_NORTH: return KeyCode::GamepadY;
        case SDL_GAMEPAD_BUTTON_LEFT_PADDLE1: return KeyCode::GamepadLeftBumper;
        case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1: return KeyCode::GamepadRightBumper;
        case SDL_GAMEPAD_BUTTON_BACK: return KeyCode::GamepadBack;
        case SDL_GAMEPAD_BUTTON_START: return KeyCode::GamepadStart;
        case SDL_GAMEPAD_BUTTON_LEFT_STICK: return KeyCode::GamepadLeftStick;
        case SDL_GAMEPAD_BUTTON_RIGHT_STICK: return KeyCode::GamepadRightStick;
        case SDL_GAMEPAD_BUTTON_DPAD_UP: return KeyCode::GamepadDPadUp;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return KeyCode::GamepadDPadDown;
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return KeyCode::GamepadDPadLeft;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return KeyCode::GamepadDPadRight;
        default: return KeyCode::Unknown; // Handle unknown button
        }
    }

    static KeyCode GetMouseButtonFromSDL(const SDL_MouseButtonEvent& mouseEvent)
    {
        switch (mouseEvent.button)
        {
        case SDL_BUTTON_LEFT: return KeyCode::MouseButtonLeft;
        case SDL_BUTTON_RIGHT: return KeyCode::MouseButtonRight;
        case SDL_BUTTON_MIDDLE: return KeyCode::MouseButtonMiddle;
        case SDL_BUTTON_X1: return KeyCode::MouseButtonX1;
        case SDL_BUTTON_X2: return KeyCode::MouseButtonX2;
        default: return KeyCode::Unknown; // Handle unknown mouse button
        }
    }

    static KeyCode GetKeyCodeFromScancode(const SDL_Scancode scancode)
    {
        switch (scancode)
        {
        case SDL_SCANCODE_A: return KeyCode::A;
        case SDL_SCANCODE_B: return KeyCode::B;
        case SDL_SCANCODE_C: return KeyCode::C;
        case SDL_SCANCODE_D: return KeyCode::D;
        case SDL_SCANCODE_E: return KeyCode::E;
        case SDL_SCANCODE_F: return KeyCode::F;
        case SDL_SCANCODE_G: return KeyCode::G;
        case SDL_SCANCODE_H: return KeyCode::H;
        case SDL_SCANCODE_I: return KeyCode::I;
        case SDL_SCANCODE_J: return KeyCode::J;
        case SDL_SCANCODE_K: return KeyCode::K;
        case SDL_SCANCODE_L: return KeyCode::L;
        case SDL_SCANCODE_M: return KeyCode::M;
        case SDL_SCANCODE_N: return KeyCode::N;
        case SDL_SCANCODE_O: return KeyCode::O;
        case SDL_SCANCODE_P: return KeyCode::P;
        case SDL_SCANCODE_Q: return KeyCode::Q;
        case SDL_SCANCODE_R: return KeyCode::R;
        case SDL_SCANCODE_S: return KeyCode::S;
        case SDL_SCANCODE_T: return KeyCode::T;
        case SDL_SCANCODE_U: return KeyCode::U;
        case SDL_SCANCODE_V: return KeyCode::V;
        case SDL_SCANCODE_W: return KeyCode::W;
        case SDL_SCANCODE_X: return KeyCode::X;
        case SDL_SCANCODE_Y: return KeyCode::Y;
        case SDL_SCANCODE_Z: return KeyCode::Z;
        case SDL_SCANCODE_1: return KeyCode::One;
        case SDL_SCANCODE_2: return KeyCode::Two;
        case SDL_SCANCODE_3: return KeyCode::Three;
        case SDL_SCANCODE_4: return KeyCode::Four;
        case SDL_SCANCODE_5: return KeyCode::Five;
        case SDL_SCANCODE_6: return KeyCode::Six;
        case SDL_SCANCODE_7: return KeyCode::Seven;
        case SDL_SCANCODE_8: return KeyCode::Eight;
        case SDL_SCANCODE_9: return KeyCode::Nine;
        case SDL_SCANCODE_0: return KeyCode::Zero;
        case SDL_SCANCODE_F1: return KeyCode::F1;
        case SDL_SCANCODE_F2: return KeyCode::F2;
        case SDL_SCANCODE_F3: return KeyCode::F3;
        case SDL_SCANCODE_F4: return KeyCode::F4;
        case SDL_SCANCODE_F5: return KeyCode::F5;
        case SDL_SCANCODE_F6: return KeyCode::F6;
        case SDL_SCANCODE_F7: return KeyCode::F7;
        case SDL_SCANCODE_F8: return KeyCode::F8;
        case SDL_SCANCODE_F9: return KeyCode::F9;
        case SDL_SCANCODE_F10: return KeyCode::F10;
        case SDL_SCANCODE_F11: return KeyCode::F11;
        case SDL_SCANCODE_F12: return KeyCode::F12;
        case SDL_SCANCODE_ESCAPE: return KeyCode::Escape;
        case SDL_SCANCODE_RETURN: return KeyCode::Enter;
        case SDL_SCANCODE_SPACE: return KeyCode::Space;
        case SDL_SCANCODE_BACKSPACE: return KeyCode::Backspace;
        case SDL_SCANCODE_TAB: return KeyCode::Tab;
        case SDL_SCANCODE_LSHIFT: return KeyCode::LeftShift;
        case SDL_SCANCODE_RSHIFT: return KeyCode::RightShift;
        case SDL_SCANCODE_LCTRL: return KeyCode::LeftControl;
        case SDL_SCANCODE_RCTRL: return KeyCode::RightControl;
        case SDL_SCANCODE_LALT: return KeyCode::LeftAlt;
        case SDL_SCANCODE_RALT: return KeyCode::RightAlt;
        case SDL_SCANCODE_LGUI: return KeyCode::LeftMeta;
        case SDL_SCANCODE_RGUI: return KeyCode::RightMeta;
        case SDL_SCANCODE_PRINTSCREEN: return KeyCode::PrintScreen;
        case SDL_SCANCODE_SCROLLLOCK: return KeyCode::ScrollLock;
        case SDL_SCANCODE_PAUSE: return KeyCode::Pause;
        case SDL_SCANCODE_INSERT: return KeyCode::Insert;
        case SDL_SCANCODE_HOME: return KeyCode::Home;
        case SDL_SCANCODE_PAGEUP: return KeyCode::PageUp;
        case SDL_SCANCODE_DELETE: return KeyCode::Delete;
        case SDL_SCANCODE_END: return KeyCode::End;
        case SDL_SCANCODE_PAGEDOWN: return KeyCode::PageDown;
        case SDL_SCANCODE_CAPSLOCK: return KeyCode::CapsLock;
        case SDL_SCANCODE_NUMLOCKCLEAR: return KeyCode::NumLock;
        case SDL_SCANCODE_LEFTBRACKET: return KeyCode::LeftBracket;
        case SDL_SCANCODE_RIGHTBRACKET: return KeyCode::RightBracket;
        case SDL_SCANCODE_SEMICOLON: return KeyCode::Semicolon;
        case SDL_SCANCODE_APOSTROPHE: return KeyCode::Apostrophe;
        case SDL_SCANCODE_GRAVE: return KeyCode::GraveAccent;
        case SDL_SCANCODE_APPLICATION: return KeyCode::Menu;
        case SDL_SCANCODE_MENU: return KeyCode::LeftSuper;
        case SDL_SCANCODE_COMMA: return KeyCode::Comma;
        case SDL_SCANCODE_PERIOD: return KeyCode::Period;
        case SDL_SCANCODE_SLASH: return KeyCode::Slash;
        case SDL_SCANCODE_BACKSLASH: return KeyCode::Backslash;
        case SDL_SCANCODE_EQUALS: return KeyCode::Equals;
        case SDL_SCANCODE_MINUS: return KeyCode::Minus;
        case SDL_SCANCODE_UP: return KeyCode::Up;
        case SDL_SCANCODE_DOWN: return KeyCode::Down;
        case SDL_SCANCODE_LEFT: return KeyCode::Left;
        case SDL_SCANCODE_RIGHT: return KeyCode::Right;
        case SDL_SCANCODE_KP_0: return KeyCode::Numpad0;
        case SDL_SCANCODE_KP_1: return KeyCode::Numpad1;
        case SDL_SCANCODE_KP_2: return KeyCode::Numpad2;
        case SDL_SCANCODE_KP_3: return KeyCode::Numpad3;
        case SDL_SCANCODE_KP_4: return KeyCode::Numpad4;
        case SDL_SCANCODE_KP_5: return KeyCode::Numpad5;
        case SDL_SCANCODE_KP_6: return KeyCode::Numpad6;
        case SDL_SCANCODE_KP_7: return KeyCode::Numpad7;
        case SDL_SCANCODE_KP_8: return KeyCode::Numpad8;
        case SDL_SCANCODE_KP_9: return KeyCode::Numpad9;
        case SDL_SCANCODE_KP_PERIOD: return KeyCode::NumpadDecimal;
        case SDL_SCANCODE_KP_DIVIDE: return KeyCode::NumpadDivide;
        case SDL_SCANCODE_KP_MULTIPLY: return KeyCode::NumpadMultiply;
        case SDL_SCANCODE_KP_MINUS: return KeyCode::NumpadSubtract;
        case SDL_SCANCODE_KP_PLUS: return KeyCode::NumpadAdd;
        case SDL_SCANCODE_KP_ENTER: return KeyCode::NumpadEnter;
        case SDL_SCANCODE_KP_EQUALS: return KeyCode::NumpadEqual;
        case SDL_SCANCODE_VOLUMEUP: return KeyCode::VolumeUp;
        case SDL_SCANCODE_VOLUMEDOWN: return KeyCode::VolumeDown;
        case SDL_SCANCODE_MUTE: return KeyCode::Mute;
        case SDL_SCANCODE_MEDIA_NEXT_TRACK: return KeyCode::MediaNext;
        case SDL_SCANCODE_MEDIA_PREVIOUS_TRACK: return KeyCode::MediaPrevious;
        case SDL_SCANCODE_MEDIA_STOP: return KeyCode::MediaStop;
        case SDL_SCANCODE_MEDIA_PLAY_PAUSE: return KeyCode::MediaPlayPause;
        default: return KeyCode::Unknown;
        }
    }
}
