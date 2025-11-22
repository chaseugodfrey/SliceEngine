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
#include <iostream>
#include "../Core/Core.h"

namespace SliceEngine
{
#pragma region GLFW callbacks
    // callbacks from GLFW
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        //UNUSED(window);
        //UNUSED(mods);

        // assign input variable to the singleton instance
        auto input = Core::GetInstance()->GetInputSystem();

        if (action == GLFW_PRESS)
        {
            // update that particular key to pressed state
            input->UpdateKeyMap(key, KeyStates::PRESS);
        }
        else if (action == GLFW_RELEASE)
        {
            input->UpdateKeyMap(key, KeyStates::RELEASE);
        }
    }

    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        //change way i get input sys, call singleton instance of class from core.h/cpp
        //auto& input = Core::GetInstance()->GetInputSystem();
        auto input = SliceEngine::Core::GetInstance()->GetInputSystem();

        if (action == GLFW_PRESS)
        {
            input->UpdateMouseMap(button, KeyStates::PRESS);
        }
        else if (action == GLFW_RELEASE)
        {
            input->UpdateMouseMap(button, KeyStates::RELEASE);
        }
    }

    // function is used to track mouse position
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        Core::GetInstance()->GetInputSystem()->SetMousePosition(xpos, ypos);
    }

    // track scroll offset
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        Core::GetInstance()->GetInputSystem()->SetScrollOffset(yoffset);
    }
#pragma endregion


#pragma region binding callbacks, init, update, and prev input update

    void InputSystem::Init(GLFWwindow* window)
    {
        windowRef = window;
        // default: do not autobind; editor should exist without game input
        // call BindCallbacksToWindow(window) when launching the game/player
        // callback binds were initially here but we separate them out to a different func so they're not auto-bound to the window
    }

    // update function to transition key states and reset scroll delta
    void InputSystem::Update()
    {



        // transition states only if enabled
        if (!enabled)
        {
            scrollDelta = 0.0f;
            return;
        }

        // transition key states, loop through all keys in map and update states
        for (auto& [key, state] : keyMap)
        {
            if (state == KeyStates::PRESS) 
                state = KeyStates::PRESSED;
            else if (state == KeyStates::RELEASE) 
                state = KeyStates::RELEASED;
        }

        for (auto& [button, state] : mouseMap)
        {
            if (state == KeyStates::PRESS) 
                state = KeyStates::PRESSED;
            else if (state == KeyStates::RELEASE) 
                state = KeyStates::RELEASED;
        }

        scrollDelta = 0.0f; // reset each frame
    }

    void InputSystem::UpdatePrevInput()
    {
        // create queue to hold frame edges [pressed/released only]
        std::queue<InputEvent> nextFrameEdges;

        while (!changedQueue.empty())
        {
            InputEvent event = changedQueue.front();
            changedQueue.pop(); // pop front in order to process it

            KeyStates newState = KeyStates::NONE;
            switch (event.state) // check which state the frame edge event is in
            {
            case KeyStates::PRESS:
                newState = KeyStates::PRESSED;
                break;
            case KeyStates::PRESSED:
                newState = KeyStates::HOLD;
                break;
            case KeyStates::RELEASE:
                newState = KeyStates::RELEASED;
                break;
            case KeyStates::RELEASED:
                newState = KeyStates::NONE;
                break;
            default:
                newState = KeyStates::NONE;
                break;
            }
            // update the key/mouse map with the new state
            if (event.isKey)
                keyMap[event.code] = newState;
            else
                mouseMap[event.code] = newState;

            // keep only one frame edges so we can see them next frame
            if (newState == KeyStates::PRESSED || newState == KeyStates::RELEASED)
                nextFrameEdges.push({ event.isKey, event.code, newState });
        }

        // swap the queues so changedQueue now has only the frame edges for next frame
        changedQueue.swap(nextFrameEdges);
        prevMousePos = currMousePos;
        scrollDelta = 0.0f;
    }

    // bind callbacks to window (if not already bound)
    void InputSystem::BindCallbacksToWindow(GLFWwindow* window)
    {
        if (callbacksBound) return;
        windowRef = window;
        glfwSetKeyCallback(windowRef, KeyCallback);
        glfwSetMouseButtonCallback(windowRef, MouseButtonCallback);
        glfwSetCursorPosCallback(windowRef, CursorPosCallback);
        glfwSetScrollCallback(windowRef, ScrollCallback);
        callbacksBound = true;
    }

    // unbind callbacks from window (if bound)
    void InputSystem::UnbindCallbacks()
    {
        if (!callbacksBound || !windowRef) return;
        // detach callbacks so editor can own them 
        glfwSetKeyCallback(windowRef, nullptr);
        glfwSetMouseButtonCallback(windowRef, nullptr);
        glfwSetCursorPosCallback(windowRef, nullptr);
        glfwSetScrollCallback(windowRef, nullptr);
        callbacksBound = false;
    }

    // func to enable/disable input system and clear states if disabling
    void InputSystem::SetEnabled(bool on)
    {
        enabled = on;
        if (!enabled)
        {
            keyMap.clear();
            mouseMap.clear();
            scrollDelta = 0.0f;
        }
    }

    // setter function to set input mode to whatever i want
    void InputSystem::SetMode(InputMode m) { mode = m; }

    // call this function to set whether imgui is consuming keyboard/mouse input this frame
    void InputSystem::SetImGuiCapture(bool wantKeyboard, bool wantMouse)
    {
        // imgui capture (call each frame from editor layer), 
        // this func is called by editor each frame to tell inputsystem whether imgui is using input
        imguiWantsKeyboard = wantKeyboard;
        imguiWantsMouse = wantMouse;
    }

#pragma endregion


#pragma region KEY AND MOUSE CHECKS

    bool InputSystem::IsKeyPressed(int key)
    {
        return keyMap[key] == PRESS || keyMap[key] == PRESSED;
    }

    bool InputSystem::IsKeyReleased(int key)
    {
        return keyMap[key] == RELEASE || keyMap[key] == RELEASED;
    }

    bool InputSystem::IsKeyDown(int key)
    {
		return keyMap[key] == HOLD || keyMap[key] == PRESSED || keyMap[key] == PRESS;
    }

    bool InputSystem::IsMousePressed(MouseButtons b)  
    {
        //auto it = mouseMap.find((int)b);
        //const auto s = (it == mouseMap.end() ? KeyStates::NONE : it->second);
        //return allowGameMouse() && (s == KeyStates::PRESS || s == KeyStates::PRESSED);

        int key = (int)b;
        return mouseMap[key] == PRESS || mouseMap[key] == PRESSED;
    }

    bool InputSystem::IsMouseReleased(MouseButtons b)  
    {
        //auto it = mouseMap.find((int)b);
        //const auto s = (it == mouseMap.end() ? KeyStates::NONE : it->second);
        //return allowGameMouse() && (s == KeyStates::RELEASE || s == KeyStates::RELEASED);
        int key = (int)b;
        return mouseMap[key] == RELEASE || mouseMap[key] == RELEASED;
    }

    bool InputSystem::IsMouseDown(MouseButtons b)  
    {
        //auto it = mouseMap.find((int)b);
        //const auto s = (it == mouseMap.end() ? KeyStates::NONE : it->second);
        //return allowGameMouse() && (s == KeyStates::PRESSED || s == KeyStates::HOLD);
        int key = (int)b;
        return mouseMap[key] == PRESS || mouseMap[key] == PRESSED;
    }

#pragma endregion


#pragma region callback updates
    void InputSystem::UpdateKeyMap(int key, KeyStates state)
    {
  //      if (!enabled) return;
  //      if (mode != InputMode::Game) return; // editor/imgui owns keyboard
		//if (imguiWantsKeyboard) return;

        keyMap[key] = state; // update to immediate key state in map
        changedQueue.push({ true, key, state }); // record down the changed event in the queue
    }

    void InputSystem::UpdateMouseMap(int button, KeyStates state)
    {
  //      if (!enabled) return;
  //      if (mode != InputMode::Game) return; // editor/imgui owns mouse
		//if (imguiWantsMouse) return;

        mouseMap[button] = state;
        changedQueue.push({ false, button, state });
    }

    void InputSystem::SetMousePosition(double x, double y)
    {
        currMousePos = { x, y };
    }

    void InputSystem::SetScrollOffset(double offset)
    {
        scrollDelta = (float)offset;
    }

    //// event queue functions
    //bool InputSystem::PollEvent(InputEvent& out)
    //{
    //    if (changedQueue.empty()) 
    //        return false; // check if queue is empty, return false if it is
    //    out = changedQueue.front(); // copy front
    //    changedQueue.pop(); // pop front
    //    std::cout << "EVENT POLLED: " << std::endl;
    //    return true; // return true if an event was polled
    //}

    //bool InputSystem::PeekEvent(InputEvent& out) const
    //{
    //    if (changedQueue.empty()) return false;
    //    out = changedQueue.front(); // copy front without popping
    //    return true;
    //}

    //void InputSystem::ClearEvents()
    //{
    //    while (!changedQueue.empty()) changedQueue.pop(); // clear the queue
    //}
#pragma endregion


    // func to convert keycode to string, for chars that are not printable, provide own fallback names
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