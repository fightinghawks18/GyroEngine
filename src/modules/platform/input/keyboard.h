//
// Created by tyler on 7/3/25.
//

#pragma once

#include <unordered_map>

#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>

#include "inpututils.h"
#include "singleton.h"

namespace GyroEngine::Platform
{
    enum class Key : uint32_t
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
    };

    struct KeyData
    {
        uint32_t timestamp;
        bool pressed = false;
    };

    class Keyboard : public Utils::ISingleton<Keyboard>
    {
        friend class ISingleton;
    public:
        void Init();
        void Update(const SDL_Event& event);
        static bool IsKeyDown(const Key key) { return Get().m_keyMap[key].pressed; }
    private:
        void UpdateKey(const SDL_KeyboardEvent& event);

        std::unordered_map<Key, KeyData> m_keyMap;

        static Key ScancodeToKey(const SDL_Scancode code)
        {
            switch (code)
            {
                case SDL_SCANCODE_A: return Key::A;
                case SDL_SCANCODE_B: return Key::B;
                case SDL_SCANCODE_C: return Key::C;
                case SDL_SCANCODE_D: return Key::D;
                case SDL_SCANCODE_E: return Key::E;
                case SDL_SCANCODE_F: return Key::F;
                case SDL_SCANCODE_G: return Key::G;
                case SDL_SCANCODE_H: return Key::H;
                case SDL_SCANCODE_I: return Key::I;
                case SDL_SCANCODE_J: return Key::J;
                case SDL_SCANCODE_K: return Key::K;
                case SDL_SCANCODE_L: return Key::L;
                case SDL_SCANCODE_M: return Key::M;
                case SDL_SCANCODE_N: return Key::N;
                case SDL_SCANCODE_O: return Key::O;
                case SDL_SCANCODE_P: return Key::P;
                case SDL_SCANCODE_Q: return Key::Q;
                case SDL_SCANCODE_R: return Key::R;
                case SDL_SCANCODE_S: return Key::S;
                case SDL_SCANCODE_T: return Key::T;
                case SDL_SCANCODE_U: return Key::U;
                case SDL_SCANCODE_V: return Key::V;
                case SDL_SCANCODE_W: return Key::W;
                case SDL_SCANCODE_X: return Key::X;
                case SDL_SCANCODE_Y: return Key::Y;
                case SDL_SCANCODE_Z: return Key::Z;

                case SDL_SCANCODE_1: return Key::One;
                case SDL_SCANCODE_2: return Key::Two;
                case SDL_SCANCODE_3: return Key::Three;
                case SDL_SCANCODE_4: return Key::Four;
                case SDL_SCANCODE_5: return Key::Five;
                case SDL_SCANCODE_6: return Key::Six;
                case SDL_SCANCODE_7: return Key::Seven;
                case SDL_SCANCODE_8: return Key::Eight;
                case SDL_SCANCODE_9: return Key::Nine;
                case SDL_SCANCODE_0: return Key::Zero;

                case SDL_SCANCODE_F1: return Key::F1;
                case SDL_SCANCODE_F2: return Key::F2;
                case SDL_SCANCODE_F3: return Key::F3;
                case SDL_SCANCODE_F4: return Key::F4;
                case SDL_SCANCODE_F5: return Key::F5;
                case SDL_SCANCODE_F6: return Key::F6;
                case SDL_SCANCODE_F7: return Key::F7;
                case SDL_SCANCODE_F8: return Key::F8;
                case SDL_SCANCODE_F9: return Key::F9;
                case SDL_SCANCODE_F10: return Key::F10;
                case SDL_SCANCODE_F11: return Key::F11;
                case SDL_SCANCODE_F12: return Key::F12;

                case SDL_SCANCODE_ESCAPE: return Key::Escape;
                case SDL_SCANCODE_RETURN: return Key::Enter;
                case SDL_SCANCODE_SPACE: return Key::Space;
                case SDL_SCANCODE_BACKSPACE: return Key::Backspace;
                case SDL_SCANCODE_TAB: return Key::Tab;
                case SDL_SCANCODE_LSHIFT: return Key::LeftShift;
                case SDL_SCANCODE_RSHIFT: return Key::RightShift;
                case SDL_SCANCODE_LCTRL: return Key::LeftControl;
                case SDL_SCANCODE_RCTRL: return Key::RightControl;
                case SDL_SCANCODE_LALT: return Key::LeftAlt;
                case SDL_SCANCODE_RALT: return Key::RightAlt;
                case SDL_SCANCODE_LGUI: return Key::LeftMeta;
                case SDL_SCANCODE_RGUI: return Key::RightMeta;
                case SDL_SCANCODE_PRINTSCREEN: return Key::PrintScreen;
                case SDL_SCANCODE_SCROLLLOCK: return Key::ScrollLock;
                case SDL_SCANCODE_PAUSE: return Key::Pause;
                case SDL_SCANCODE_INSERT: return Key::Insert;
                case SDL_SCANCODE_HOME: return Key::Home;
                case SDL_SCANCODE_PAGEUP: return Key::PageUp;
                case SDL_SCANCODE_DELETE: return Key::Delete;
                case SDL_SCANCODE_END: return Key::End;
                case SDL_SCANCODE_PAGEDOWN: return Key::PageDown;
                case SDL_SCANCODE_CAPSLOCK: return Key::CapsLock;
                case SDL_SCANCODE_NUMLOCKCLEAR: return Key::NumLock;
                case SDL_SCANCODE_LEFTBRACKET: return Key::LeftBracket;
                case SDL_SCANCODE_RIGHTBRACKET: return Key::RightBracket;
                case SDL_SCANCODE_SEMICOLON: return Key::Semicolon;
                case SDL_SCANCODE_APOSTROPHE: return Key::Apostrophe;
                case SDL_SCANCODE_GRAVE: return Key::GraveAccent;
                case SDL_SCANCODE_MENU: return Key::Menu;
                case SDL_SCANCODE_COMMA: return Key::Comma;
                case SDL_SCANCODE_PERIOD: return Key::Period;
                case SDL_SCANCODE_SLASH: return Key::Slash;
                case SDL_SCANCODE_BACKSLASH: return Key::Backslash;
                case SDL_SCANCODE_EQUALS: return Key::Equals;
                case SDL_SCANCODE_MINUS: return Key::Minus;

                case SDL_SCANCODE_UP: return Key::Up;
                case SDL_SCANCODE_DOWN: return Key::Down;
                case SDL_SCANCODE_LEFT: return Key::Left;
                case SDL_SCANCODE_RIGHT: return Key::Right;

                case SDL_SCANCODE_KP_0: return Key::Numpad0;
                case SDL_SCANCODE_KP_1: return Key::Numpad1;
                case SDL_SCANCODE_KP_2: return Key::Numpad2;
                case SDL_SCANCODE_KP_3: return Key::Numpad3;
                case SDL_SCANCODE_KP_4: return Key::Numpad4;
                case SDL_SCANCODE_KP_5: return Key::Numpad5;
                case SDL_SCANCODE_KP_6: return Key::Numpad6;
                case SDL_SCANCODE_KP_7: return Key::Numpad7;
                case SDL_SCANCODE_KP_8: return Key::Numpad8;
                case SDL_SCANCODE_KP_9: return Key::Numpad9;
                case SDL_SCANCODE_KP_PERIOD: return Key::NumpadDecimal;
                case SDL_SCANCODE_KP_DIVIDE: return Key::NumpadDivide;
                case SDL_SCANCODE_KP_MULTIPLY: return Key::NumpadMultiply;
                case SDL_SCANCODE_KP_MINUS: return Key::NumpadSubtract;
                case SDL_SCANCODE_KP_PLUS: return Key::NumpadAdd;
                case SDL_SCANCODE_KP_ENTER: return Key::NumpadEnter;
                case SDL_SCANCODE_KP_EQUALS: return Key::NumpadEqual;

                case SDL_SCANCODE_VOLUMEUP: return Key::VolumeUp;
                case SDL_SCANCODE_VOLUMEDOWN: return Key::VolumeDown;
                case SDL_SCANCODE_MUTE: return Key::Mute;
                case SDL_SCANCODE_MEDIA_NEXT_TRACK: return Key::MediaNext;
                case SDL_SCANCODE_MEDIA_PREVIOUS_TRACK: return Key::MediaPrevious;
                case SDL_SCANCODE_MEDIA_STOP: return Key::MediaStop;
                case SDL_SCANCODE_MEDIA_PLAY_PAUSE: return Key::MediaPlayPause;

                case SDL_SCANCODE_UNKNOWN:
                default: return Key::Unknown;
            }
        }
    };
}
