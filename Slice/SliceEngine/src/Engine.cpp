/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Engine.cpp
 author:		
 email:			
 brief:			Main Engine

Copyright (C) 2024 DigiPen Institute of Technology.
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
#include "AudioManager.h"
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
#include "Configuration/ProjectSettings.h"
#include "Networking/NetworkSystem.h"
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
		EnableMemoryLeakChecking(-1);

		SLICE_LOG("Initializing Slice Engine.");
		glfwInit();

		Core::GetInstance()->InitCore();
		//Core::GetInstance()->InitFactory();
		// Set up Engine Systems
		isRunning = true;
		//auto window = Core::GetInstance()->GetWindow();
		Core::GetInstance()->GetWindow();


		
		// mResource = std::make_unique<ResourceManager>();
		frm.Init();

		auto mAudioManager = Core::GetInstance()->GetAudioManager();
		//audio->LoadSound("Assets/Audio/BGM_MainMenu_Mix1.wav");
		mAudioManager->Init();
		mAudioManager->LoadSound("Assets/Audio/3DAudioTest.wav");
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
		//Core::GetInstance()->InitSystem<NetworkSystem>();
		
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
		//mResource->RegisterResourceAsset((GUID)1001, "Assets/Models/player_mdl.mdl");	//testing loading model
		//mResource->RegisterFileAsset("Assets/Shaders/basic.txt");
		//mResource->RegisterFileAsset("Assets/Shaders/deferredLighting.txt");
		//mResource->RegisterFileAsset("Assets/Shaders/instanced.txt");
		//mResource->RegisterFileAsset("Assets/Shaders/debugLine.txt");
		//mResource->RegisterFileAsset("Assets/Models/Cube.txt");
		//mResource->RegisterFileAsset("Assets/Models/FrustrumFake.txt");
		//mResource->RegisterFileAsset("Assets/Models/CubeWireframe.txt");
		//mResource->RegisterFileAsset("Assets/Models/Line.txt");
		//mResource->RegisterFileAsset("Assets/Models/Quad.txt");
		//mResource->RegisterFileAsset("Assets/Textures/5271507727521808385.txt");
		
		/*mResource->LoadShader("Assets/Shaders/basic.vert", "Assets/Shaders/basic.frag");
		mResource->LoadModel("Assets/Models/Cube.txt");*/
		// mResource->LoadShader("Assets/Shaders/basic.vert", "Assets/Shaders/basic.frag");
		// mResource->LoadShader("Assets/Shaders/instanced.vert", "Assets/Shaders/instanced.frag");
		// mResource->LoadShader("Assets/Shaders/debugLine.vert", "Assets/Shaders/debugLine.frag");
		// mResource->LoadModel("Assets/Models/Cube.txt");
		// mResource->LoadModel("Assets/Models/FrustrumFake.txt");
		// mResource->LoadModel("Assets/Models/CubeWireframe.txt");
		// mResource->LoadModel("Assets/Models/Line.txt");
		
		//mResource->LoadShader("Assets/Shaders/basic.vert", "Assets/Shaders/basic.frag");
		//mResource->LoadModel("Assets/Models/Cube.txt");

		//mRender = std::make_unique<RenderManager>();
		Core::GetInstance()->InitSystem<CameraSystem>();
		
		mRender->CreateInstancingParams();
		mRender->CreateDeferredTextures();
		mRender->CreateCamera();
		

		//entt::entity newCam = Core::GetInstance()->GetRegistry().create();
		//Core::GetInstance()->GetRegistry().emplace<Transform>(newCam);
		//Core::GetInstance()->GetRegistry().emplace<Renderer>(newCam);
		auto mNetwork = Core::GetInstance()->GetNetwork();
		mNetwork->Init();
		//NetworkingThread::printAddr();

		//test();

		
		//JSONSerializer::Test2();
		//JSONSerializer::Tests::RunTests(false);
		//JSONSerializer::Tests::RunTests(false);
		/*GameObject testing = Core::GetInstance()->mFactory.CreateGO("testing");

		testing.AddComponent<Renderer>();
		testing.AddComponent<AudioSource>();*/

		Core::GetInstance()->mFactory.TestLoop();
		LoadProjectSettings();
		//JSONSerializer::Tests::RunTests(false);
		//Core::GetInstance()->mFactory.TestLoop();

		GameObject light = Core::GetInstance()->mFactory.CreateGO("light");
		light.GetComponent<Transform>().position = glm::vec3(0.f, 5.f, 2.f);
		light.AddComponent<Light>();
	}

	void Engine::Update()
	{
		auto sceneSystem = Core::GetInstance()->GetSceneSystem();
		if (!sceneSystem->CheckQueueEmpty())
		{
			if (sceneSystem->isSceneUnloaded)
			{
				sceneSystem->LoadNextScene();
			}
		}

		frm.updateDeltaTime(); //update deltatime and currentnumber of steps for systems that uses fixeddt
		frm.StartFrame();

		//auto mResource = Core::GetInstance()->GetResourceManager();
		auto mRender = Core::GetInstance()->GetRenderManager();
		auto mAudioManager = Core::GetInstance()->GetAudioManager();
		auto inputs = Core::GetInstance()->GetInputSystem();

		frm.StartSystem("GLFW Poll Events");
		glfwMakeContextCurrent(Core::GetInstance()->GetWindow());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwPollEvents();

		frm.EndSystem("GLFW Poll Events");
		// Main Body


		frm.StartSystem("Input");
		//inputs->Update();
		inputs->UpdatePrevInput();
		frm.EndSystem("Input");

        frm.StartSystem("Audio");
		Core::GetInstance()->GetSystem<SoundSystem>().Update(static_cast<float>(frm.getDeltaTime()));
		mAudioManager->Update();
        frm.EndSystem("Audio");
        
		frm.StartSystem("Script");
		gScriptSystem->UpdateScripts();
		if (inputs->GetMode() == InputMode::Game)
		{
			gScriptSystem->OnUpdate((float)frm.getDeltaTime());
		}
		frm.EndSystem("Script");

		// TODO: Shouldn't be using input get mode to split play and editor mode

		for (size_t step = 0; step < frm.getCurrentNumberOfSteps(); ++step)
		{
			frm.StartSystem("Physics");
			if (inputs->GetMode() == InputMode::Game)
			{
				Core::GetInstance()->GetSystem<PhysicsSystem>().Update(static_cast<float>(frm.getFixedDeltaTime()));
			}
			frm.EndSystem("Physics");
		}

		frm.StartSystem("Transform");
		Core::GetInstance()->GetSystem<TransformSystem>().Update(static_cast<float>(frm.getFixedDeltaTime()));
		frm.EndSystem("Transform");

		frm.StartSystem("Graphics");
		mRender->Render();
		frm.EndSystem("Graphics");

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

		//Window::CloseWindow(window);
		SLICE_LOG("Shutting Down Slice Engine.");
	}

	void Engine::LoadProjectSettings()
	{
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
				Core::GetInstance()->GetSceneSystem()->LoadScene(sceneToLoad); // for now by filepath
			}
		}
	}

}