#include "../SliceEngine/src/pch.h"
#include <GL/glew.h>
#include <gtest/gtest.h>
#include "Input/InputSystem.h"

using namespace SliceEngine;

class InputSystemTest : public ::testing::Test 
{
protected:
    InputSystem input;

    void SetUp() override 
    {
        // No window needed for state-only tests
        input.Init(nullptr);
    }
};

TEST_F(InputSystemTest, KeyPressStateTransition) 
{
    int testKey = 65; // 'A'

    // Initial state
    EXPECT_FALSE(input.IsKeyPressed(testKey));

    // Simulate press
    input.UpdateKeyMap(testKey, KeyStates::PRESSED);
    
    // Before UpdatePrevInput, it should still be false in the map (it's in the queue)
    EXPECT_FALSE(input.IsKeyPressed(testKey));

    // Process events
    input.UpdatePrevInput();

    // Now it should be PRESSED
    EXPECT_TRUE(input.IsKeyPressed(testKey));
    EXPECT_TRUE(input.IsKeyDown(testKey));
    EXPECT_FALSE(input.IsKeyHold(testKey));
}

TEST_F(InputSystemTest, KeyHoldStateTransition) 
{
    int testKey = 65; // 'A'

    // 1. Press
    input.UpdateKeyMap(testKey, KeyStates::PRESSED);
    input.UpdatePrevInput();
    EXPECT_TRUE(input.IsKeyPressed(testKey));

    // 2. Next frame - should transition to HOLD automatically if no other event
    input.UpdatePrevInput(); 
    EXPECT_FALSE(input.IsKeyPressed(testKey));
    EXPECT_TRUE(input.IsKeyHold(testKey));
    EXPECT_TRUE(input.IsKeyDown(testKey));
}

TEST_F(InputSystemTest, KeyReleaseStateTransition) 
{
    int testKey = 65; // 'A'

    // 1. Press and Hold
    input.UpdateKeyMap(testKey, KeyStates::PRESSED);
    input.UpdatePrevInput();
    input.UpdatePrevInput();
    EXPECT_TRUE(input.IsKeyHold(testKey));

    // 2. Release
    input.UpdateKeyMap(testKey, KeyStates::RELEASED);
    input.UpdatePrevInput();

    EXPECT_FALSE(input.IsKeyHold(testKey));
    EXPECT_TRUE(input.IsKeyReleased(testKey));
    EXPECT_FALSE(input.IsKeyDown(testKey));
}

TEST_F(InputSystemTest, MouseButtonStates) 
{
    MouseButtons testButton = MouseButtons::LEFT;

    // Simulate click
    input.UpdateMouseMap((int)testButton, KeyStates::PRESSED);
    input.UpdatePrevInput();

    EXPECT_TRUE(input.IsMousePressed(testButton));
    EXPECT_TRUE(input.IsMouseDown(testButton));

    // Release
    input.UpdateMouseMap((int)testButton, KeyStates::RELEASED);
    input.UpdatePrevInput();

    EXPECT_FALSE(input.IsMousePressed(testButton));
    EXPECT_TRUE(input.IsMouseReleased(testButton));
}

TEST_F(InputSystemTest, MousePositionAndDelta) 
{
    input.SetMousePosition(100.0, 100.0);
    input.UpdatePrevInput(); // prev = 100, curr = 100

    input.SetMousePosition(150.0, 120.0);
    input.UpdatePrevInput(); // delta = 100 - 150 = -50, -20? 
    // Wait, let's check delta calculation in InputSystem.cpp:
    // mouseDelta = prevMousePos - currMousePos;

    EXPECT_EQ(input.GetMousePosition().x, 150.0);
    EXPECT_EQ(input.GetMousePosition().y, 120.0);
    EXPECT_EQ(input.GetMouseDelta().x, -50.0);
    EXPECT_EQ(input.GetMouseDelta().y, -20.0);
}

TEST_F(InputSystemTest, ScrollDelta) 
{
    input.SetScrollOffset(5.0);
    EXPECT_EQ(input.GetScrollDelta(), 5.0f);

    input.Update(); // Update() in InputSystem.cpp resets scrollDelta
    EXPECT_EQ(input.GetScrollDelta(), 0.0f);
}
