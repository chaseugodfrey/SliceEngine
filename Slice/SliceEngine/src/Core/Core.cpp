#include "pch.h"
#include "Core.h"
#include "Graphics/ResourceManager.h"
#include "Graphics/RenderManager.h"
#include "Systems/FramerateManager.h"

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
		mFramerateManager = std::make_unique<FramerateManager>();
		mNetwork = std::make_unique<NetworkSystem>();
		//mFactory.RegisterSerializableComponent<Transform>();

		mFactory.RegisterComponent<Transform>();
		mFactory.RegisterComponent<SceneGraph>();
		mFactory.RegisterComponent<SliceEntity>();
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

	ResourceManager* Core::GetResourceManager()
	{
		return mResource.get();
	}

	RenderManager* Core::GetRenderManager()
	{
		return mRender.get();
	}

	FramerateManager* Core::GetFramerateManager()
	{
		return mFramerateManager.get();
	}

	GLFWwindow* Core::GetWindow()
	{
		
		return mWindowManager.GetWindow();
	}

	NetworkSystem* Core::GetNetwork()
	{
		return mNetwork.get();
	}
}