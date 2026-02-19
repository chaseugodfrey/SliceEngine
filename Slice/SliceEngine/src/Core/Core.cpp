/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Core.cpp
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Singleton for accessing systems, and data

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include "pch.h"
#include "Core.h"
//#include "Graphics/ResourceManager.h"
#include "Resource/ResourceManager.h"
#include "Networking/NetworkSystem.h"
#include "Graphics/RenderManager.h"
#include "Systems/FramerateManager.h"
#include "../Audio/AudioManager.h"
#include "Systems/SceneSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Input/InputSystem.h"
#include "Input/ActionMapping.h"
#include "Systems/LayerManager.h"
#include "Navigation/NavigationSystem.h"
#include "Configuration/ProjectSettingsManager.h"

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
		mWindowManager.CreateGLFWWindow(); // This has to be first

		mResource = std::make_unique<ResourceManager>();
		mRender = std::make_unique<RenderManager>();
		mAudioManager = std::make_unique<AudioManager>();
		mFramerateManager = std::make_unique<FramerateManager>();
		mLayerManager = std::make_unique<LayerManager>();
		mNetwork = std::make_unique<NetworkSystem>();
		//mFactory.RegisterSerializableComponent<Transform>();
		mInputPtr = std::make_unique<InputSystem>();
		mInputPtr->Init(mWindowManager.GetWindow());
		mInputPtr->BindCallbacksToWindow(mWindowManager.GetWindow());
		SliceEngine::GetActionMappingSystem().SetInputSystem(mInputPtr.get()); // set global action mapping system's input system pointer
		mScenePtr = std::make_unique<SceneSystem>();
		mProjectSettingsManager = std::make_unique<ProjectSettingsManager>();

		mFactory.RegisterComponent<Transform>();
		mFactory.RegisterComponent<SceneGraph>();
		mFactory.RegisterComponent<Renderer>();
		mFactory.RegisterComponent<Camera>();
		mFactory.RegisterComponent<SliceEntity>();
		mFactory.RegisterComponent<Script>();
		mFactory.RegisterComponent<RigidBody>();
		mFactory.RegisterComponent<ColliderShape>();
		mFactory.RegisterComponent<AudioSource>();
		mFactory.RegisterComponent<AudioListener>();
		mFactory.RegisterComponent<Light>();
		mFactory.RegisterComponent<TempTransform>();
		mFactory.RegisterComponent<ParticleSystem>();
		mFactory.RegisterComponent<Prefab>();
		mFactory.RegisterComponent<Animator>();
		mFactory.RegisterComponent<Bone>();
		mFactory.RegisterComponent<InactiveEntity>();
		mFactory.RegisterComponent<RectTransform>();
		mFactory.RegisterComponent<Canvas>();
		mFactory.RegisterComponent<SpriteRenderer>();
		mFactory.RegisterComponent<FontRenderer>();
		mFactory.RegisterComponent<Button>();
		mFactory.RegisterComponent<Slider>();
		mFactory.RegisterComponent<NavAgent>();
		mFactory.RegisterComponent<NavMeshLink>();
		mFactory.RegisterComponent<NavObstacle>();

		mFactory.RegisterComponent<Prefab>();
		mResource->InitResourceManager();

	}

	void Core::ExitCore()
	{
		mFactory.FactoryShutdown();
		mResource->Shutdown();
		mWindowManager.CloseWindow();
		mProjectSettingsManager->Exit();
		UnbindSystems();
		glfwTerminate();

		mAudioManager->Exit();
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

	SceneSystem* Core::GetSceneSystem()
	{
		return mScenePtr.get();
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

	LayerManager* Core::GetLayerManager()
	{
		return mLayerManager.get();
	}

	AudioManager* Core::GetAudioManager()
	{
		return mAudioManager.get();
	}

	GLFWwindow* Core::GetWindow()
	{

		return mWindowManager.GetWindow();
	}

	GLFWWindowManager* Core::GetWindowManager()
	{
		return &mWindowManager;
	}

	NetworkSystem* Core::GetNetwork()
	{
		return mNetwork.get();
	}



	//NavigationSystem *Core::GetNavAgent()
	//{
	//	return mNavAgent.get();
	//}

	ProjectSettingsManager* Core::GetProjectSettingsManager()
	{
		return mProjectSettingsManager.get();
	}

}