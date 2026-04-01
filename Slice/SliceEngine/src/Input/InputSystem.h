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
#include <GL/glew.h>
#include <glfw3.h>
#include "InputTypes.h"
#include <array>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <functional>
#include <math.h>


// create struct for GLFWwindow to avoid including GLFW in header
struct GLFWwindow;

namespace SliceEngine
{
    struct InputEvent
    {
        bool isKey; // true if key event, false if mouse event
        int code; // keycode or buttoncode
        KeyStates state; // new state
    };


    class InputSystem
    {
    private:
        GLFWwindow* windowRef = nullptr;
        std::unordered_map<int, KeyStates> keyMap; // keycode, keystate
        std::unordered_map<int, KeyStates> mouseMap; // buttoncode, buttonstate
        //std::queue<std::tuple<bool, int, KeyStates>> changedQueue; // isKey, code, state
        std::queue<InputEvent> changedQueue; // container for one frame events

        glm::vec2 currMousePos{ 0.0, 0.0 }; // reset mouse positions
        glm::vec2 prevMousePos{ 0.0, 0.0 };
        glm::vec2 internalMousePos{ 0.0, 0.0 }; // for disabled cursor state use
        glm::vec2 mouseDelta{ 0.0, 0.0 };
        glm::vec2 currMouseNDC{ 0.0, 0.0 };


        float scrollDelta = 0.0f; // reset scroll delta

        // runtime control
        bool enabled = true; // global enable/disable
        InputMode mode = InputMode::Game; // default to game mode

        // imgui capture flags (editor tells us if imgui is consuming input this frame)
        bool imguiWantsKeyboard = false;
        bool imguiWantsMouse = false;

        // internal gate
        // if enabled and in game mode, allow input if imgui is not consuming it
        inline bool allowGameKeyboard() const { return enabled && mode == InputMode::Game; }
        inline bool allowGameMouse() const { return enabled && mode == InputMode::Game; }

        CursorState currentCursorState{ CursorState::DEFAULT };
        CursorState prevCursorState{ CursorState::DEFAULT };

        void SetCursorState();

        // installation state for callbacks
        bool callbacksBound = false; // to prevent double-binding

    public:
        glm::ivec2 windowDim{ 1920, 1080 };

        // func to convert keycode to string
        static const char* KeyNameFallback(int key);

        // lifecycle
        void Init(GLFWwindow* window);
        void Update();
        void UpdatePrevInput();
        void UpdateCursorData(); // editor doesn't call this

        // queue 
        size_t EventsThisFrame() const
        {
            return changedQueue.size();
        }

        // bind/unbind callbacks explicitly (instead of always on)
        void BindCallbacksToWindow(GLFWwindow* window);
        void UnbindCallbacks();

        // enable/disable & mode
        void SetEnabled(bool on); // set enabled/disabled
        bool IsEnabled() const { return enabled; } // check if enabled
        void SetMode(InputMode m); // set input mode
        InputMode GetMode() const { return mode; } // get current input mode

        // imgui capture hints (call each frame from editor layer)
        void SetImGuiCapture(bool wantKeyboard, bool wantMouse); // set imgui capture flags

        // key queries
        bool IsKeyPressed(int key);
        bool IsKeyReleased(int key);
        bool IsKeyDown(int key);
        bool IsKeyHold(int key);
        bool IsKeyUp(int key);

        // mouse queries
        bool IsMousePressed(MouseButtons button);
        bool IsMouseReleased(MouseButtons button);
        bool IsMouseDown(MouseButtons button);
        bool IsMouseHold(MouseButtons button);
        bool IsMouseUp(MouseButtons button);

        // mouse position
        glm::vec2 GetMousePosition() const;
        glm::vec2 GetMouseDelta() const;
        glm::vec2 GetMouseNDC() const;
        double GetMouseX() const;
        double GetMouseY() const;

        // cursor states
        void SetCursorState(CursorState state);
        CursorState GetCurrCursorState() const;
        void ResetCursorState();

#ifdef SLICE_INPUT_USE_GLM
        // this is used to interface with glm, why are we using this function? its so that we dont have to include glm in this header
        glm::dvec2 GetMousePositionGlm() const { return glm::dvec2(currMousePos.x, currMousePos.y); }
#endif

        // scroll
        float GetScrollDelta() const { return scrollDelta; }

        // updates from callbacks
        void UpdateKeyMap(int key, KeyStates state);
        void UpdateMouseMap(int button, KeyStates state);
        void SetMousePosition(double x, double y);
        void SetMouseDelta(double x, double y);
        void SetScrollOffset(double offset);
        void SetWindowDim(int width, int height);
        void SetMouseNDC(double x, double y);
    };
}
#endif