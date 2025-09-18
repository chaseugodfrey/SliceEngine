/* File Documentation -----------------------------------------------------------------------------
file:           InputSystem.h

\author			Micah Lim [100%]

email:          micahshengyao.lim@digipen.edu

brief:          This file declares the InputSystem class, responsible for handling keyboard and mouse
				input events in the application. It provides methods to query the state of keys and
				mouse buttons, as well as tracking mouse position and scroll offsets.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
--------------------------------------------------------------------------------------------------*/


#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H
#include "../ECS/BaseSystem.h"
#include "InputTypes.h"
#include <array>
#include <queue>
#include <tuple>
#include <unordered_map>
// create struct for GLFWwindow to avoid including GLFW in header
struct GLFWwindow;

namespace SliceEngine
{
    class InputSystem
    {
    private:
		//GLFWwindow* windowRef = nullptr; // reference to the GLFW window
		std::unordered_map<int, KeyStates> keyMap; // keycode, keystate
		std::unordered_map<int, KeyStates> mouseMap; // buttoncode, buttonstate
		std::queue<std::tuple<bool, int, KeyStates>> changedQueue; // isKey, code, state

        Vec2d currMousePos{ 0.0, 0.0 }; // reset mouse positions
        Vec2d prevMousePos{ 0.0, 0.0 }; // reset mouse positions
		float scrollDelta = 0.0f; // reset scroll delta

    public:
        static InputSystem& Get()
        {
            static InputSystem instance;
            return instance;
        }

		// func to convert keycode to string
        static const char* KeyNameFallback(int key);

        void Init(GLFWwindow* window);
        void Update();
        void UpdatePrevInput();

        // key queries
        bool IsKeyPressed(int key);
        bool IsKeyReleased(int key);
        bool IsKeyDown(int key);

        // mouse queries
        bool IsMousePressed(MouseButtons button);
        bool IsMouseReleased(MouseButtons button);
        bool IsMouseDown(MouseButtons button);

        // mouse position
        Vec2d GetMousePosition() const { return currMousePos; }
        double GetMouseX() const { return currMousePos.x; }
        double GetMouseY() const { return currMousePos.y; }

        // scroll
        float GetScrollDelta() const { return scrollDelta; }

        // updates from callbacks
        void UpdateKeyMap(int key, KeyStates state);
        void UpdateMouseMap(int button, KeyStates state);
        void SetMousePosition(double x, double y);
        void SetScrollOffset(double offset);
    };
}
#endif