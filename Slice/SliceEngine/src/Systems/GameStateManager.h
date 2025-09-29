#ifndef GAME_STATE_MANAGER_H
#define GAME_STATE_MANAGER_H

#include "pch.h"
#include "ECS/BaseSystem.h"
#include <memory>
#include <stack>

namespace SliceEngine
{
	bool isGameRunning = false; // func to check if game is running
	struct GSMEntity {}; // empty struct to tag game state system

	class GameStateSystem : BaseSystem<GSMEntity, GameStateSystem>
	{
		bool isRunning = false; // bool to check if game state system is running
		void GSMInit();
		void GSMUpdate();
		void GSMExit();
	};

	void RunSystems();


	class GameState
	{
	public:
		bool isRunning = false; // bool to check if game state is running
		virtual ~GameState() = default; // constructor
		virtual void Init();
		virtual void Update();
		virtual void Exit();
	};

	class GameStateManager
	{
	private:
		std::stack<std::unique_ptr<GameState>> gameStateStack; // stack to hold game states

	public:
		GameStateManager() = default; // constructor
		~GameStateManager(); // destructor

		void ChangeGameState(std::unique_ptr<GameState> newState); // func takes in ptr of new game state to change to
		void PushGameState(std::unique_ptr<GameState> newState); // func that one can call to push the state they want to top of stack
		void PopGameState(); // func to remove the current game state at top of stack
		void Update(); // call update on current state
		void Cleanup(); // clean all assets we're currently using but don't need for next state, music, assets etc
		void RunGameLoopFunctions(GameStateManager& manager); // func to run all systems that are needed in a game loop
	};
}

#endif // GAME_STATE_MANAGER_H