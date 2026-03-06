#include <GL/glew.h>
#include <catch.hpp>
#include "Input/InputSystem.h"

using namespace SliceEngine;

TEST_CASE("InputSystem State Transitions", "[Input]") 
{
    InputSystem input;
    input.Init(nullptr);
    int testKey = 65; // GLFW_KEY_A, used as tester

    SECTION("Key Press and Hold") 
    {
        // 1. Initial State: Up
        REQUIRE(input.IsKeyUp(testKey) == true);

        // 2. Event: Key Pressed
        input.UpdateKeyMap(testKey, KeyStates::PRESSED);
        
        // 3. Process the event
        input.UpdatePrevInput();
        
        // 4. Verification: Should now be in PRESSED state
        REQUIRE(input.IsKeyPressed(testKey) == true);

        // 5. Next frame transition (automatic persist to HOLD)
        input.UpdatePrevInput();
        REQUIRE(input.IsKeyHold(testKey) == true);
    }

    SECTION("Key Release") 
    {
        // 1. Start in Pressed state
        input.UpdateKeyMap(testKey, KeyStates::PRESSED);
        input.UpdatePrevInput();
        REQUIRE(input.IsKeyPressed(testKey) == true);

        // 2. Event: Key Released
        input.UpdateKeyMap(testKey, KeyStates::RELEASED);
        
        // 3. Process the event
        input.UpdatePrevInput();
        
        // 4. Verification: Should now be in RELEASED state
        REQUIRE(input.IsKeyReleased(testKey) == true);

        // 5. Next frame transition (automatic reset to UP/NONE)
        input.UpdatePrevInput();
        REQUIRE(input.IsKeyUp(testKey) == true);
    }

    SECTION("Mouse Movement Delta") 
    {
        // 1. Set initial position
        input.SetMousePosition(100.0, 100.0);
        
        // 2. Update to set prevPos = currPos (delta will be 0)
        input.UpdateCursorData();

        // 3. Move mouse
        input.SetMousePosition(150.0, 120.0);
        
        // 4. Update to calculate delta: prev(100) - curr(150) = -50
        input.UpdateCursorData();

        REQUIRE(input.GetMousePosition().x == 150.0);
        REQUIRE(input.GetMouseDelta().x == -50.0);
    }
}
