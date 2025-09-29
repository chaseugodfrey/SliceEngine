#include "pch.h"
#include "Core.h"
#include "Graphics/ResourceManager.h"
#include "Graphics/RenderManager.h"
#include "Input/InputSystem.h"
namespace SliceEngine
{
	Core::Core()
	{

	}

	Core::~Core()
	{

	}

	Registry& Core::GetRegistry()
	{
		return mFactory.mRegistry;
	}

	void Core::InitCore()
	{
		mWindowManager.CreateWindow(); // This has to be first

		mResource = std::make_unique<ResourceManager>();
		mRender = std::make_unique<RenderManager>();
		mInputPtr = std::make_unique<InputSystem>();
		mInputPtr->Init(mWindowManager.GetWindow());
		mInputPtr->BindCallbacksToWindow(mWindowManager.GetWindow());
		mFactory.RegisterComponent<Transform>();
		mFactory.RegisterComponent<SceneGraph>();
	}

	void Core::ExitCore()
	{
		mWindowManager.CloseWindow();
		UnbindSystems();
	}

	void Core::UnbindSystems()
	{
		for (auto& system : mSystems)
		{
			system.second->Unbind();
			//system->Unbind();
		}

	}

	InputSystem* Core::GetInputSystem()
	{
		return mInputPtr.get();
	}

	ResourceManager* Core::GetResourceManager()
	{
		return mResource.get();
	}

	RenderManager* Core::GetRenderManager()
	{
		return mRender.get();
	}

	GLFWwindow* Core::GetWindow()
	{

		return mWindowManager.GetWindow();
	}
}