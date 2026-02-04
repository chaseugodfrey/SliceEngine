#include <pch.h>
#include "Application.h"

#include <Engine.h>
#include <Input/InputSystem.h>
#include <filesystem>
#include <iostream>

namespace SliceBuild
{
	//Time class for physics simulation or any other system that uses fixeddt
	void EnableMemoryLeakChecking(int breakAlloc = -1)
	{
		int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
		_CrtSetDbgFlag(tmpDbgFlag);

		if (breakAlloc != -1) _CrtSetBreakAlloc(breakAlloc);
	}

	void Application::Init()
	{
		//SLICE_LOG("Initializing Application.");
		EnableMemoryLeakChecking(-1);

		std::cout << std::filesystem::current_path().string() << std::endl;

		engine.Init();

		SliceEngine::Core::GetInstance()->GetSceneSystem()->SetTempFileSaving(false);

		auto window = SliceEngine::Core::GetInstance()->GetWindow();
		auto windowManager = SliceEngine::Core::GetInstance()->GetWindowManager();
		auto inputSys = SliceEngine::Core::GetInstance()->GetInputSystem();
		inputSys->BindCallbacksToWindow(window);

		windowManager->FullScreenWindow();

		SliceEngine::Core::GetInstance()->GetSceneSystem()->LoadSceneByIndex(1);

		//engine.InitScene();

		//engine.Update();

		//inputSys->SetMode(SliceEngine::InputMode::Game);

		//EventManager::GetInstance()->Publish<OnPlayEvent>();

		//SliceEngine::Core::GetInstance()->GetSceneSystem()->mNextState = SliceEngine::SceneState::PLAY_SCENE;

		EventManager::GetInstance()->Subscribe<OnGameStopEvent, &Application::QuitGameEvent>(this);
	}

	void Application::Run()
	{
		auto inputSys = SliceEngine::Core::GetInstance()->GetInputSystem();
		

		while (!glfwWindowShouldClose(SliceEngine::Core::GetInstance()->GetWindow()))
		{
			engine.Update();
			engine.Draw();
			engine.EndFrame();

			engine.WindowSizeSwitch();

			if (SliceEngine::Core::GetInstance()->GetSceneSystem()->mCurrentState == SliceEngine::SceneState::DEFAULT)
			{
				SliceEngine::Core::GetInstance()->GetSceneSystem()->mNextState = SliceEngine::SceneState::PLAY_SCENE;
			}

			if (inputSys->IsKeyPressed(GLFW_KEY_SPACE))
			{
				if (SliceEngine::Core::GetInstance()->GetSceneSystem()->mCurrentState == SliceEngine::SceneState::PLAY_SCENE)
				{
					SliceEngine::Core::GetInstance()->GetSceneSystem()->mNextState = SliceEngine::SceneState::STOP_SCENE;
				}
				else
				{
					SliceEngine::Core::GetInstance()->GetSceneSystem()->mNextState = SliceEngine::SceneState::PLAY_SCENE;
				}

			}
		}

	}

	void Application::Exit()
	{
		engine.Exit();
	}

	void Application::QuitGameEvent(OnGameStopEvent e)
	{
		glfwSetWindowShouldClose(SliceEngine::Core::GetInstance()->GetWindow(), GLFW_TRUE);
	}
}