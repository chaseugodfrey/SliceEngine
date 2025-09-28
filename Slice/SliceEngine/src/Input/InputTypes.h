/* file documentation -----------------------------------------------------------------------------
\file		InputTypes.hpp
\author     Micah Lim, (micahshengyao.lim)

email:      micahshengyao.lim@digipen.edu

\brief      This file defines various enumerations used for handling keyboard and mouse input in the
            game engine. These enumerations include key codes for keyboard keys, mouse buttons, and
            states (pressed, released, hold) for both.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-------------------------------------------------------------------------------------------------*/

#ifndef INPUT_TYPES_H
#define INPUT_TYPES_H

//#include <Windows.h>
// note for micah: dont include windows.h
// it fks up glfw window 

#include <glfw3.h>

namespace SliceEngine
{
    // i am not sure i need this anymore tbh
    /*
    enum Keys : int
    {
        // Special and Unknown Keys
        KEY_UNKNOWN = GLFW_KEY_UNKNOWN,

        // Printable Keys
        KEY_SPACEBAR = GLFW_KEY_SPACE,
        KEY_APOSTROPHE = GLFW_KEY_APOSTROPHE,
        KEY_COMMA = GLFW_KEY_COMMA,
        KEY_MINUS = GLFW_KEY_MINUS,
        KEY_PERIOD = GLFW_KEY_PERIOD,
        KEY_SLASH = GLFW_KEY_SLASH,
        KEY_SEMICOLON = GLFW_KEY_SEMICOLON,
        KEY_EQUAL = GLFW_KEY_EQUAL,
        KEY_LEFT_BRACKET = GLFW_KEY_LEFT_BRACKET,
        KEY_BACKSLASH = GLFW_KEY_BACKSLASH,
        KEY_RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET,
        KEY_GRAVE_ACCENT = GLFW_KEY_GRAVE_ACCENT,
        KEY_WORLD_1 = GLFW_KEY_WORLD_1,
        KEY_WORLD_2 = GLFW_KEY_WORLD_2,

        KEY_0 = GLFW_KEY_0,
        KEY_1 = GLFW_KEY_1,
        KEY_2 = GLFW_KEY_2,
        KEY_3 = GLFW_KEY_3,
        KEY_4 = GLFW_KEY_4,
        KEY_5 = GLFW_KEY_5,
        KEY_6 = GLFW_KEY_6,
        KEY_7 = GLFW_KEY_7,
        KEY_8 = GLFW_KEY_8,
        KEY_9 = GLFW_KEY_9,
        KEY_A = GLFW_KEY_A,
        KEY_B = GLFW_KEY_B,
        KEY_C = GLFW_KEY_C,
        KEY_D = GLFW_KEY_D,
        KEY_E = GLFW_KEY_E,
        KEY_F = GLFW_KEY_F,
        KEY_G = GLFW_KEY_G,
        KEY_H = GLFW_KEY_H,
        KEY_I = GLFW_KEY_I,
        KEY_J = GLFW_KEY_J,
        KEY_K = GLFW_KEY_K,
        KEY_L = GLFW_KEY_L,
        KEY_M = GLFW_KEY_M,
        KEY_N = GLFW_KEY_N,
        KEY_O = GLFW_KEY_O,
        KEY_P = GLFW_KEY_P,
        KEY_Q = GLFW_KEY_Q,
        KEY_R = GLFW_KEY_R,
        KEY_S = GLFW_KEY_S,
        KEY_T = GLFW_KEY_T,
        KEY_U = GLFW_KEY_U,
        KEY_V = GLFW_KEY_V,
        KEY_W = GLFW_KEY_W,
        KEY_X = GLFW_KEY_X,
        KEY_Y = GLFW_KEY_Y,
        KEY_Z = GLFW_KEY_Z,

        // Function Keys
        KEY_ESCAPE = GLFW_KEY_ESCAPE,
        KEY_ENTER = GLFW_KEY_ENTER,
        KEY_TAB = GLFW_KEY_TAB,
        KEY_BACKSPACE = GLFW_KEY_BACKSPACE,
        KEY_INSERT = GLFW_KEY_INSERT,
        KEY_DELETE = GLFW_KEY_DELETE,
        KEY_RIGHT = GLFW_KEY_RIGHT,
        KEY_LEFT = GLFW_KEY_LEFT,
        KEY_DOWN = GLFW_KEY_DOWN,
        KEY_UP = GLFW_KEY_UP,
        KEY_PAGE_UP = GLFW_KEY_PAGE_UP,
        KEY_PAGE_DOWN = GLFW_KEY_PAGE_DOWN,
        KEY_HOME = GLFW_KEY_HOME,
        KEY_END = GLFW_KEY_END,
        KEY_CAPS_LOCK = GLFW_KEY_CAPS_LOCK,
        KEY_SCROLL_LOCK = GLFW_KEY_SCROLL_LOCK,
        KEY_NUM_LOCK = GLFW_KEY_NUM_LOCK,
        KEY_PRINT_SCREEN = GLFW_KEY_PRINT_SCREEN,
        KEY_PAUSE = GLFW_KEY_PAUSE,

        // Function Keys
        KEY_F01 = GLFW_KEY_F1,
        KEY_F02 = GLFW_KEY_F2,
        KEY_F03 = GLFW_KEY_F3,
        KEY_F04 = GLFW_KEY_F4,
        KEY_F05 = GLFW_KEY_F5,
        KEY_F06 = GLFW_KEY_F6,
        KEY_F07 = GLFW_KEY_F7,
        KEY_F08 = GLFW_KEY_F8,
        KEY_F09 = GLFW_KEY_F9,
        KEY_F10 = GLFW_KEY_F10,
        KEY_F11 = GLFW_KEY_F11,
        KEY_F12 = GLFW_KEY_F12,

        // Modifier Keys
        KEY_LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
        KEY_LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL,
        KEY_LEFT_ALT = GLFW_KEY_LEFT_ALT,
        KEY_LEFT_SUPER = GLFW_KEY_LEFT_SUPER,
        KEY_RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,
        KEY_RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL,
        KEY_RIGHT_ALT = GLFW_KEY_RIGHT_ALT,
        KEY_RIGHT_SUPER = GLFW_KEY_RIGHT_SUPER,
        KEY_MENU = GLFW_KEY_MENU,

        MAX_KEYBOARD_KEYS
    };
    */

    // enum to switch between game and editor
    enum class InputMode : uint8_t
    {
        Game = 0,
        Editor
    };

    // these modifiers are used to check if ctrl, shift, alt are pressed
    struct Modifiers
    {
        bool ctrl{ false };
        bool shift{ false };
        bool alt{ false };

        static Modifiers FromGLFW(int mods)
        {
            Modifiers m;
            m.shift = (mods & GLFW_MOD_SHIFT) != 0;
            m.ctrl = (mods & GLFW_MOD_CONTROL) != 0;
            m.alt = (mods & GLFW_MOD_ALT) != 0;
            return m;
        }

        bool matches(int mods) const
        {
            const bool wantShift = shift, wantCtrl = ctrl, wantAlt = alt;
            const bool hasShift = (mods & GLFW_MOD_SHIFT) != 0;
            const bool hasCtrl = (mods & GLFW_MOD_CONTROL) != 0;
            const bool hasAlt = (mods & GLFW_MOD_ALT) != 0;
            return wantShift == hasShift && wantCtrl == hasCtrl && wantAlt == hasAlt;
        }
    };

    enum MouseButtons
    {
        LEFT = GLFW_MOUSE_BUTTON_LEFT,
        RIGHT = GLFW_MOUSE_BUTTON_RIGHT,
        MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE,
        MAX_MOUSE_BUTTONS
    };

    enum KeyStates
    {
        NONE = -1,
        RELEASE,
        RELEASED,
        PRESS,
        PRESSED,
        HOLD,
        DRAGGING
    };
}

#endif