#include <pch.h>
#include "Application.h"

#include <Engine.h>
#include <Input/InputSystem.h>

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

		engine.Init();

		auto window = SliceEngine::Core::GetInstance()->GetWindow();
		auto inputSys = SliceEngine::Core::GetInstance()->GetInputSystem();
		inputSys->BindCallbacksToWindow(window);

		engine.SceneInit();

		inputSys->SetMode(SliceEngine::InputMode::Editor);

		EventManager::GetInstance()->Publish<OnPlayEvent>();

		EventManager::GetInstance()->Subscribe<OnGameStopEvent, &Application::QuitGameEvent>(this);
	}

	void Application::Run()
	{
		while (!glfwWindowShouldClose(SliceEngine::Core::GetInstance()->GetWindow()))
		{
			engine.Update();
			engine.Draw();
			engine.EndFrame();
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