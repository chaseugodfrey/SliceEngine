/* File Documentation -----------------------------------------------------------------------------
file:           InputSystem.cpp

\author			Micah Lim (100%)

email:          micahshengyao.lim@digipen.edu

brief:          This file implements the InputSystem class, managing input handling for keyboard
				and mouse events. It provides methods for tracking key presses, mouse clicks. 
                Integrates GLFW for input callbacks and utilizes functionality to synchronize input 
                states for real-time applications.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
--------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "InputSystem.h"

namespace SliceEngine
{
    #pragma region GLFW callbacks
	// callbacks from GLFW
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        //UNUSED(window);
        //UNUSED(mods);

        // assign input variable to the singleton instance
        auto& input = InputSystem::Get();

        if (action == GLFW_PRESS)
        {
            // update that particular key to pressed state
            input.UpdateKeyMap(key, KeyStates::PRESS);

            // try GLFW's printable name first
            const char* printable = glfwGetKeyName(key, scancode);

            // glfwGetKeyName returns nullptr for non-printable keys
            if (printable && *printable)
            {
                std::cout << "Pressed: " << printable << std::endl;
            }
            else
            {
                std::cout << "Pressed: " << InputSystem::KeyNameFallback(key) << std::endl;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            input.UpdateKeyMap(key, KeyStates::RELEASE);
        }
    }


    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        auto& input = InputSystem::Get();
        if (action == GLFW_PRESS)
        {
            input.UpdateMouseMap(button, KeyStates::PRESS);
        }
        else if (action == GLFW_RELEASE)
        {
            input.UpdateMouseMap(button, KeyStates::RELEASE);
        }
    }

	// function is used to track mouse position
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        InputSystem::Get().SetMousePosition(xpos, ypos);
    }

	// track scroll offset
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        InputSystem::Get().SetScrollOffset(yoffset);
    }
    #pragma endregion

    #pragma region init, update, and prev input update
    // inputsystem methods
    void InputSystem::Init(GLFWwindow* window)
    {
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetCursorPosCallback(window, CursorPosCallback);
        glfwSetScrollCallback(window, ScrollCallback);
    }

    void InputSystem::Update()
    {
		// transition key states, loop through all keys in map and update states
        for (auto& [key, state] : keyMap)
        {
            if (state == KeyStates::PRESS) state = KeyStates::PRESSED;
            else if (state == KeyStates::RELEASE) state = KeyStates::RELEASED;
        }

        for (auto& [button, state] : mouseMap)
        {
            if (state == KeyStates::PRESS) state = KeyStates::PRESSED;
            else if (state == KeyStates::RELEASE) state = KeyStates::RELEASED;
        }

        scrollDelta = 0.0f; // reset each frame
    }

    void InputSystem::UpdatePrevInput()
    {
        prevMousePos = currMousePos;
    }
    #pragma endregion


    #pragma region key and mouse checks
    // queries/checks
    bool InputSystem::IsKeyPressed(int key) 
    { 
        return keyMap[key] == KeyStates::PRESS; 
    }
    bool InputSystem::IsKeyReleased(int key) 
    { 
        return keyMap[key] == KeyStates::RELEASE; 
    }
    bool InputSystem::IsKeyDown(int key) 
    { 
        return keyMap[key] == KeyStates::PRESSED || keyMap[key] == KeyStates::HOLD; 
    }

    bool InputSystem::IsMousePressed(MouseButtons button) 
    { 
        return mouseMap[(int)button] == KeyStates::PRESS; 
    }
    bool InputSystem::IsMouseReleased(MouseButtons button) 
    { 
        return mouseMap[(int)button] == KeyStates::RELEASE; 
    }
    bool InputSystem::IsMouseDown(MouseButtons button) 
    { 
        return mouseMap[(int)button] == KeyStates::PRESSED || mouseMap[(int)button] == KeyStates::HOLD; 
    }
    #pragma endregion


    #pragma region callback updates
    // updates from callbacks
    void InputSystem::UpdateKeyMap(int key, KeyStates state) 
    { 
        keyMap[key] = state; 
    }
    void InputSystem::UpdateMouseMap(int button, KeyStates state) 
    { 
        mouseMap[button] = state; 
    }
    void InputSystem::SetMousePosition(double x, double y) 
    {   
        currMousePos = { x, y }; 
    }
    void InputSystem::SetScrollOffset(double offset) 
    { 
        scrollDelta = (float)offset; 
    }
    #pragma endregion

	// func to convert keycode to string
	// for chars that are not printable, provide own fallback names
    const char* InputSystem::KeyNameFallback(int key)
    {
        switch (key)
        {
        // alphabetical and numeric keys
		case GLFW_KEY_A: return "A";
		case GLFW_KEY_B: return "B";
		case GLFW_KEY_C: return "C";
		case GLFW_KEY_D: return "D";
		case GLFW_KEY_E: return "E";
		case GLFW_KEY_F: return "F";
		case GLFW_KEY_G: return "G";
		case GLFW_KEY_H: return "H";
		case GLFW_KEY_I: return "I";
		case GLFW_KEY_J: return "J";
		case GLFW_KEY_K: return "K";
		case GLFW_KEY_L: return "L";
		case GLFW_KEY_M: return "M";
		case GLFW_KEY_N: return "N";
		case GLFW_KEY_O: return "O";
		case GLFW_KEY_P: return "P";
		case GLFW_KEY_Q: return "Q";
		case GLFW_KEY_R: return "R";
		case GLFW_KEY_S: return "S";
		case GLFW_KEY_T: return "T";
		case GLFW_KEY_U: return "U";
		case GLFW_KEY_V: return "V";
		case GLFW_KEY_W: return "W";
		case GLFW_KEY_X: return "X";
		case GLFW_KEY_Y: return "Y";
		case GLFW_KEY_Z: return "Z";
		case GLFW_KEY_0: return "0";
		case GLFW_KEY_1: return "1";
		case GLFW_KEY_2: return "2";
		case GLFW_KEY_3: return "3";
		case GLFW_KEY_4: return "4";
		case GLFW_KEY_5: return "5";
		case GLFW_KEY_6: return "6";
		case GLFW_KEY_7: return "7";
		case GLFW_KEY_8: return "8";
		case GLFW_KEY_9: return "9";

        // function keys
        case GLFW_KEY_F1:  return "F1";
        case GLFW_KEY_F2:  return "F2";
        case GLFW_KEY_F3:  return "F3";
        case GLFW_KEY_F4:  return "F4";
        case GLFW_KEY_F5:  return "F5";
        case GLFW_KEY_F6:  return "F6";
        case GLFW_KEY_F7:  return "F7";
        case GLFW_KEY_F8:  return "F8";
        case GLFW_KEY_F9:  return "F9";
        case GLFW_KEY_F10: return "F10";
        case GLFW_KEY_F11: return "F11";
        case GLFW_KEY_F12: return "F12";

        // modifiers
        case GLFW_KEY_LEFT_SHIFT:    return "Left Shift";
        case GLFW_KEY_RIGHT_SHIFT:   return "Right Shift";
        case GLFW_KEY_LEFT_CONTROL:  return "Left Ctrl";
        case GLFW_KEY_RIGHT_CONTROL: return "Right Ctrl";
        case GLFW_KEY_LEFT_ALT:      return "Left Alt";
        case GLFW_KEY_RIGHT_ALT:     return "Right Alt";
        case GLFW_KEY_LEFT_SUPER:    return "Left Super";
        case GLFW_KEY_RIGHT_SUPER:   return "Right Super";

        // navigation
        case GLFW_KEY_ESCAPE:     return "Escape";
        case GLFW_KEY_TAB:        return "Tab";
        case GLFW_KEY_ENTER:      return "Enter";
        case GLFW_KEY_BACKSPACE:  return "Backspace";
        case GLFW_KEY_INSERT:     return "Insert";
        case GLFW_KEY_DELETE:     return "Delete";
        case GLFW_KEY_HOME:       return "Home";
        case GLFW_KEY_END:        return "End";
        case GLFW_KEY_PAGE_UP:    return "Page Up";
        case GLFW_KEY_PAGE_DOWN:  return "Page Down";
        case GLFW_KEY_UP:         return "Arrow Up";
        case GLFW_KEY_DOWN:       return "Arrow Down";
        case GLFW_KEY_LEFT:       return "Arrow Left";
        case GLFW_KEY_RIGHT:      return "Arrow Right";

        // space & symbols (glfwGetKeyName covers these, but jic ig)
        case GLFW_KEY_SPACE:         return "Space";
        case GLFW_KEY_APOSTROPHE:    return "Apostrophe";
        case GLFW_KEY_COMMA:         return "Comma";
        case GLFW_KEY_MINUS:         return "Minus";
        case GLFW_KEY_PERIOD:        return "Period";
        case GLFW_KEY_SLASH:         return "Slash";
        case GLFW_KEY_SEMICOLON:     return "Semicolon";
        case GLFW_KEY_EQUAL:         return "Equal";
        case GLFW_KEY_LEFT_BRACKET:  return "Left Bracket";
        case GLFW_KEY_RIGHT_BRACKET: return "Right Bracket";
        case GLFW_KEY_BACKSLASH:     return "Backslash";

        default: return "Unknown Key";
        }
    }


}



