/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			GameStateManager.cpp
 author:		Micah Lim 
 email:			micahshengyao.lim@digipen.edu
 brief:			Handles Game states

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <iostream>
#include "pch.h"
#include "GameStateManager.h"

namespace SliceEngine
{
	void GameStateSystem::GSMInit()
	{

	}

	void GameStateSystem::GSMUpdate()
	{
		RunSystems();
	}

	void GameStateSystem::GSMExit()
	{

	}

	void RunSystems()
	{
		// run all systems that are needed in a game loop
		// PhysicsSystem->Update(dt);
	}


	// destructr handles cleanup of state's leftover stuff
	GameStateManager::~GameStateManager()
	{
		Cleanup();
	}

	void GameStateManager::ChangeGameState(std::unique_ptr<GameState> newState)
	{
		// remove old state 
		if (!gameStateStack.empty())
		{
			gameStateStack.top()->Exit(); // exit cur state
			gameStateStack.pop(); // remove just-exited state
		}

		gameStateStack.push(std::move(newState)); // push newly wanted state to top of stack
		gameStateStack.top()->Init(); // init new game state with all its stuff
	}

	// use if want to push next gs, but dont want to change it to current 
	void GameStateManager::PushGameState(std::unique_ptr<GameState> newState)
	{
		gameStateStack.push(std::move(newState));
		gameStateStack.top()->Init();
	}

	// use if want to pop current gs but dont want to change to next
	void GameStateManager::PopGameState()
	{
		if (!gameStateStack.empty())
		{
			gameStateStack.top()->Exit();
			gameStateStack.pop();
		}
	}

	// use if 
	void GameStateManager::Update()
	{
		if (!gameStateStack.empty())
		{
			gameStateStack.top()->Update(); // update current state
		}
	}

	// cleanup/remove all states
	void GameStateManager::Cleanup()
	{
		// cleanup all states
		while (!gameStateStack.empty())
		{
			gameStateStack.top()->Exit();
			gameStateStack.pop();
		}
	}

    void GameStateManager::RunGameLoopFunctions(GameStateManager& manager)
    {
		while (isGameRunning) // check if current state is running
		{
			manager.Update();
		}

		manager.Cleanup(); // cleanup all states when done
    }



}