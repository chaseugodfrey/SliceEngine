/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Engine.cpp
 author:		
 email:			
 brief:			Main Engine

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "Engine.h"
#include "ECS/ECSTypes.h"
#include "Physics/PhysicsSystem.h"
#include "GLFWWindowManager.h"
#include "Core/Core.h"
#include "Input/InputSystem.h"
#include "../src/Audio/AudioManager.h"
#include "Systems/TransformSystem.h"
#include <crtdbg.h>
//#include "Graphics/ResourceManager.h"
#include "Resource/ResourceManager.h"

#include "Graphics/CameraSystem.h"
#include "Graphics/RenderManager.h"
#include "Graphics/LightingSystem.h"
#include "ECS/BaseSystem.h"
#include "ECS/SliceRTTR.h"
#include "Systems/FramerateManager.h"
#include "Serializer/JSONSerializer.h"
#include "Serializer/CSVSerializer.h"
#include "Graphics/TransformHelper.h"
#include "Scripting/ScriptSystem.h"
#include "Systems/SceneSystem.h"
#include "Configuration/ProjectSettings.h"
#include "Networking/NetworkSystem.h"
#include "Systems/ParticleSystemManager.h"
#include "Systems/PrefabSystem.h"
#include "Input/ActionMapping.h"
#include "Animator/AnimatorSystem.h"
#include "Animator/BoneSystem.h"
#include "Systems/CoroutineManager.h"
//using namespace rttr;

//struct MyStruct { MyStruct() {}; void func(double) {}; int data; };

//RTTR_REGISTRATION
//{
//	registration::class_<MyStruct>("MyStruct")
//		 .constructor<>()
//		 .property("data", &MyStruct::data)
//		 .method("func", &MyStruct::func);
//}


namespace SliceEngine
{
	// forward declare global pointer to action mapping system
	//extern ActionMappingSystem* gActionMappingSystemInstance;
	//// actual single instance pointer which is static
	//static ActionMappingSystem actionMapSystemInstance(Core::GetInstance()->GetInputSystem());

	// removed this from engine.cpp because core.cpp now has the global action mapping system instance ptr


	//Time class for physics simulation or any other system that uses fixeddt
	void EnableMemoryLeakChecking(int breakAlloc = -1)
	{
		int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
		_CrtSetDbgFlag(tmpDbgFlag);

		if (breakAlloc != -1) _CrtSetBreakAlloc(breakAlloc);
	}

	Engine::Engine() : frm(SliceEngine::FramerateManager::getInstance())
	{
		isRunning = false;
	}
	Engine::~Engine()
	{
	}

	void Engine::Test()
	{
		std::cout << " Hi from Engine Test Function\n";
	}



	void Engine::Init()
	{
		//EnableMemoryLeakChecking(92083);

		SLICE_LOG("Initializing Slice Engine.");
		glfwInit();

		Core::GetInstance()->InitCore();


		// Set up Engine Systems
		isRunning = true;
		//auto window = Core::GetInstance()->GetWindow();
		Core::GetInstance()->GetWindow();
		
		// mResource = std::make_unique<ResourceManager>();
		frm.Init();

		auto mAudioManager = Core::GetInstance()->GetAudioManager();
		//audio->LoadSound("Assets/Audio/BGM_MainMenu_Mix1.wav");
		mAudioManager->Init();
		
		glm::vec3 posVec = { -2.0f,0.0f,0.0f };
		glm::vec3 velVec = { 0.0f,0.0f,1.0f };
		glm::vec3 forwardVec = { -1.0f,0.0f,0.0f };
		glm::vec3 upVec = { 0.0f,1.0f,0.0f };

		mAudioManager->SetListenerAttributes(posVec, velVec, forwardVec, upVec);
		
		FactoryInstance.InitRootEntity();
		Core::GetInstance()->InitSystem<SoundSystem>();
		Core::GetInstance()->InitSystem<WorldSpaceGraphicsSystem>();
		Core::GetInstance()->InitSystem<LightingSystem>();
		Core::GetInstance()->InitSystem<TransformSystem>();
		Core::GetInstance()->InitSystem<ParticleSystemManager>();
		Core::GetInstance()->InitSystem<PrefabSystem>();
		//Core::GetInstance()->InitSystem<NetworkSystem>();
		Core::GetInstance()->InitSystem<AnimatorSystem>();
		Core::GetInstance()->InitSystem<BoneSystem>();

		
		Core::GetInstance()->InitSystem<PhysicsSystem>();
		Core::GetInstance()->InitSystem<ScriptSystem>();
		Core::GetInstance()->GetSystem<PhysicsSystem>().Initialize(static_cast<float>(frm.getFixedDeltaTime()));
		Core::GetInstance()->GetSystem<PhysicsSystem>().SubscribeToEvents();
		Core::GetInstance()->GetSystem<SoundSystem>().BindToAudioSource();
		gScriptSystem->Init();
		//audio->PlaySound("BGM_MainMenu_Mix1", SliceEngine::SoundCategory::BGM, SliceEngine::AudioManager::InternalSound::SOUND_BGM, false, false, 0.5f);
		//audio->PlaySound("3DAudioTest", SliceEngine::SoundCategory::BGM, SliceEngine::AudioManager::InternalSound::SOUND_BGM, true, false, 0.5f);

		//auto mResource = Core::GetInstance()->GetResourceManager();
		Core::GetInstance()->GetResourceManager();
		auto mRender = Core::GetInstance()->GetRenderManager();
		Core::GetInstance()->InitSystem<CameraSystem>();
		
		mRender->CreateInstancingParams();
		mRender->CreateDeferredTextures();
		//mRender->CreateCamera();
		

		//entt::entity newCam = Core::GetInstance()->GetRegistry().create();
		//Core::GetInstance()->GetRegistry().emplace<Transform>(newCam);
		//Core::GetInstance()->GetRegistry().emplace<Renderer>(newCam);
		auto mNetwork = Core::GetInstance()->GetNetwork();
		mNetwork->Init();
		//NetworkingThread::printAddr();
	
	}

	void Engine::SceneInit()
	{
		LoadProjectSettings();

	}

	void Engine::Update()
	{
		auto core = Core::GetInstance();
		auto sTransform = core->GetSystem<TransformSystem>();
		auto sScene = Core::GetInstance()->GetSceneSystem();
		auto sRender = core->GetRenderManager();
		auto sAudio = core->GetAudioManager();
		auto sInputs = core->GetInputSystem();
		auto& sAnimator = core->GetSystem<AnimatorSystem>();
		auto& sBone = core->GetSystem<BoneSystem>();
		static bool isPlaying = false;

		if (!sScene->CheckQueueEmpty())
		{
			if (sScene->isSceneUnloaded)
			{
				sScene->LoadNextScene();
			}
		}

		while (sScene->mCurrentState != sScene->mNextState)
		{
			//Line to load resources
			if (sScene->mNextState == SceneState::PLAY_SCENE)
			{
				sInputs->SetMode(InputMode::Game);
				sInputs->SetEnabled(true);
				if (!isPlaying)
				{
					SliceEngine::gScriptSystem->OnStart();
					sAnimator.InitSystem();
					isPlaying = true;

				}

				if (sScene->mCurrentState == SceneState::DEFAULT)
				{
					
					sScene->WriteTempFile();

				}
				sScene->mCurrentState = SceneState::PLAY_SCENE;
			}

			if (sScene->mNextState == SceneState::PAUSE_SCENE)
			{
				sInputs->SetMode(InputMode::Editor);
				sInputs->SetEnabled(false);
				isPlaying = false;
				sScene->mCurrentState = SceneState::PAUSE_SCENE;
			}

			//When the stop button has been clicked and the scene state is set to STOP_SCENE, reload the current scene
			if (sScene->mNextState == SceneState::STOP_SCENE)
			{
				sInputs->SetMode(InputMode::Editor);
				sInputs->SetEnabled(false);
				sScene->ReloadScene();
				isPlaying = false;

				gScriptSystem->OnEnd();

				sScene->mCurrentState = SceneState::DEFAULT;
				sScene->mNextState = SceneState::DEFAULT;
			}
		}

		frm.updateDeltaTime(); //update deltatime and currentnumber of steps for systems that uses fixeddt
		frm.StartFrame();

		//auto mResource = Core::GetInstance()->GetResourceManager();


		frm.StartSystem("GLFW Poll Events");
		glfwMakeContextCurrent(core->GetWindow());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwPollEvents();

		frm.EndSystem("GLFW Poll Events");
		// Main Body

		frm.StartSystem("Input");
		sInputs->UpdatePrevInput();
		frm.EndSystem("Input");
		// process all enabled action maps in Game mode
		if (sScene->mCurrentState == SceneState::PLAY_SCENE) 
		{
			SliceEngine::GetActionMappingSystem().processAllInput();
		}


        frm.StartSystem("Audio");
		core->GetSystem<SoundSystem>().Update(static_cast<float>(frm.getDeltaTime()));
		sAudio->Update();
        frm.EndSystem("Audio");
        
		frm.StartSystem("Script");
		gScriptSystem->UpdateScripts();
		if (sScene->mCurrentState == SceneState::PLAY_SCENE)
		{
			gScriptSystem->OnUpdate((float)frm.getDeltaTime());
		}
		frm.EndSystem("Script");

		// TODO: Shouldn't be using input get mode to split play and editor mode


		frm.StartSystem("Transform");
		sTransform.Update(static_cast<float>(frm.getFixedDeltaTime()));
		sTransform.UpdateTransforms();
		frm.EndSystem("Transform");

		frm.StartSystem("Physics");
		/*if (sInputs->GetMode() == InputMode::Game)
		{
			for (size_t step = 0; step < frm.getCurrentNumberOfSteps(); ++step)
			{
				core->GetSystem<PhysicsSystem>().Update(static_cast<float>(frm.getFixedDeltaTime()));
			}
		}*/
		if (sScene->mCurrentState == SceneState::PLAY_SCENE)
		{
			for (size_t step = 0; step < frm.getCurrentNumberOfSteps(); ++step)
			{

				core->GetSystem<PhysicsSystem>().Update(static_cast<float>(frm.getFixedDeltaTime()));

				// Single world step
				core->GetSystem<PhysicsSystem>().StepWorld(static_cast<float>(frm.getFixedDeltaTime()));

				// Post-step: pull dynamic poses for rendering
				core->GetSystem<PhysicsSystem>().PostStepSync();
			}
			
		}
		frm.EndSystem("Physics");

		sAnimator.Update(static_cast<float>(frm.getFixedDeltaTime()));
		sBone.Update_Scenegraph();
		sAnimator.BoneUpdate();

		frm.StartSystem("Graphics");
		sRender->Render();
		frm.EndSystem("Graphics");

		frm.StartSystem("Particle System");
		core->GetSystem<ParticleSystemManager>().Update(static_cast<float>(frm.getDeltaTime()));
		frm.EndSystem("Particle System");

		frm.EndFrame();
		frm.CalculateSystemPercentages();
	}

	void Engine::EndFrame()
	{
		Core::FactoryInstance.UpdateDestroyed();
		Core::GetInstance()->GetSceneSystem()->isSceneUnloaded = true;

		auto window = Core::GetInstance()->GetWindow();
		if (glfwWindowShouldClose(window))
			isRunning = false;
		//auto inputs = Core::GetInstance()->GetInputSystem();
		glfwSwapBuffers(window);
	}

	void Engine::Exit()
	{
		auto mAudioManager = Core::GetInstance()->GetAudioManager();
		//Core::GetInstance()->UnbindSystems();
		Core::GetInstance()->ExitCore();
		mAudioManager->Exit();

		auto mNetwork = Core::GetInstance()->GetNetwork();
		mNetwork->Exit();

		//Window::CloseWindow(window);
		SLICE_LOG("Shutting Down Slice Engine.");
	}

	void Engine::LoadProjectSettings()
	{
		auto sScene = Core::GetInstance()->GetSceneSystem();
		auto sResourceManager = Core::GetInstance()->GetResourceManager();

		std::filesystem::path proj = "projectSettings.json";

		ProjectSettings s;
		if (!std::filesystem::exists(proj)) {
			// Safe defaults if file missing
			s.scenes = {};
			s.startupScene.clear();
		}

		else
		{
			std::ifstream in(proj);
			nlohmann::json j; in >> j;

			if (j.contains("product") && j["product"].contains("name"))
				s.productName = j["product"]["name"].get<std::string>();
			if (j.contains("render")) {
				s.width = j["render"].value("width", s.width);
				s.height = j["render"].value("height", s.height);
				s.vsync = j["render"].value("vsync", s.vsync);
			}
			if (j.contains("scenes")) s.scenes = j["scenes"].get<std::vector<std::string>>();
			s.startupScene = j.value("startupScene", s.startupScene);

			// Fallback: if startupScene empty, use first scene
			std::string sceneToLoad = !s.startupScene.empty()
				? s.startupScene
				: (s.scenes.empty() ? "" : s.scenes.front());

			if (sceneToLoad.empty()) {
				// Nothing to load�show blank/editor splash or exit gracefully
				// log: "No scenes configured."
			}

			else
			{
				std::filesystem::path sceneFilePath(sceneToLoad);
				auto path = sResourceManager->GetResourcePath(sceneFilePath.stem().string());

				if (path.has_value())
				{
					SLICE_LOG("Scene File Path" + path.value().string());
					sScene->SetDefaultScenePath(sceneFilePath);
					sScene->LoadScene(sceneFilePath);
					sScene->mCurrentState = sScene->mNextState = SceneState::DEFAULT;
				}
				
				//sScene->LoadScene(sceneToLoad); // for now by filepath
				//sScene->mCurrentState = sScene->mNextState = SceneState::DEFAULT;

				
			}
		}
	}

}