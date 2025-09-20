#include "pch.h"
#include "Core.h"
#include "Graphics/ResourceManager.h"
#include "Graphics/RenderManager.h"
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

		mFactory.RegisterSerializableComponent<Transform>();
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

	GLFWwindow* Core::GetWindow()
	{
		
		return mWindowManager.GetWindow();
	}
}