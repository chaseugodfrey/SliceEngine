#include <pch.h>
#include "Engine.h"
#include "ECS/ECSTypes.h"
#include "Physics/PhysicsSystem.h"
#include "GLFWWindowManager.h"
#include "Core/Core.h"
#include "Input/InputSystem.h"
#include "AudioManager.h"
#include "Systems/TransformSystem.h"
#include "Graphics/ResourceManager.h"
#include "Graphics/RenderManager.h"
#include "ECS/BaseSystem.h"
#include "ECS/SliceRTTR.h"
#include "Systems/FramerateManager.h"
#include "test.h"
#include "Serializer/JSONSerializer.h"
#include "Serializer/CSVSerializer.h"
#include "Scripting/ScriptSystem.h"

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
		SLICE_LOG("Initializing Slice Engine.");
		glfwInit();

		Core::GetInstance()->InitCore();
		//Core::GetInstance()->InitFactory();
		// Set up Engine Systems
		isRunning = true;

		auto window = Core::GetInstance()->GetWindow();


		audio = std::make_unique<AudioManager>();
		// mResource = std::make_unique<ResourceManager>();
		frm.Init();
		FactoryInstance.InitRootEntity();
		Core::GetInstance()->InitSystem<SoundSystem>();
		Core::GetInstance()->InitSystem<WorldSpaceGraphicsSystem>();
		Core::GetInstance()->InitSystem<TransformSystem>();
		Core::GetInstance()->InitSystem<PhysicsSystem>();
		Core::GetInstance()->InitSystem<ScriptSystem>();
		Core::GetInstance()->GetSystem<PhysicsSystem>().Initialize(frm.getFixedDeltaTime());
		Core::GetInstance()->GetSystem<PhysicsSystem>().SubscribeToCollisionEvents();
		gScriptSystem->Init();
		audio->Init();
		audio->LoadSound("BGMTest", "Assets/Audio/BGM_MainMenu_Mix1.wav", false, false);
		//audio->PlaySound("BGMTest", SliceEngine::SoundCategory::BGM, SliceEngine::AudioManager::InternalSound::SOUND_BGM, false, 0.5f);

		auto mResource = Core::GetInstance()->GetResourceManager();
		auto mRender = Core::GetInstance()->GetRenderManager();

		mResource->LoadShader("Assets/Shaders/basic.vert", "Assets/Shaders/basic.frag");
		mResource->LoadModel("Assets/Models/Cube.txt");

		//mRender = std::make_unique<RenderManager>();
		Core::GetInstance()->InitSystem<CameraSystem>();

		mRender->CreateCamera();

		//entt::entity newCam = Core::GetInstance()->GetRegistry().create();
		//Core::GetInstance()->GetRegistry().emplace<Transform>(newCam);
		//Core::GetInstance()->GetRegistry().emplace<Renderer>(newCam);

		//JSONSerializer::Tests::RunTests(false);
		//Core::GetInstance()->mFactory.TestLoop();

		GameObject floor = Core::GetInstance()->mFactory.CreateGO("floor");
		floor.AddComponent<Transform>();
		floor.GetComponent<Transform>().position = glm::vec3(0.f, -1.8f, 0.f);
		floor.GetComponent<Transform>().scale = glm::vec3(10.f, 1.f, 10.f);
		floor.AddComponent<ColliderShape>();
		floor.AddComponent<Renderer>();

		
	}

	void Engine::Update()
	{
		frm.updateDeltaTime(); //update deltatime and currentnumber of steps for systems that uses fixeddt
		frm.StartFrame();

		auto mResource = Core::GetInstance()->GetResourceManager();
		auto mRender = Core::GetInstance()->GetRenderManager();
		auto inputs = Core::GetInstance()->GetInputSystem();

		frm.StartSystem("GLFW Poll Events");
		glfwMakeContextCurrent(Core::GetInstance()->GetWindow());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwPollEvents();

		frm.EndSystem("GLFW Poll Events");
		// Main Body

		gScriptSystem->UpdateScripts();
		gScriptSystem->OnUpdate((float)frm.getDeltaTime());

		frm.StartSystem("Input");
		 if (inputs->IsKeyPressed(KEY_W))
		 {
		 	std::cout << " test " << std::endl;
		 }
		inputs->Update();
		frm.EndSystem("Input");
		frm.StartSystem("Physics");
		for (size_t step = 0; step < frm.getCurrentNumberOfSteps(); ++step)
		{

			Core::GetInstance()->GetSystem<PhysicsSystem>().Update(frm.getFixedDeltaTime());
			
		}
		frm.EndSystem("Physics");
		// framerateManager->CapFPS(60);

		////

		frm.StartSystem("Graphics");
		mRender->Render(mResource);
		frm.EndSystem("Graphics");



		frm.EndFrame();
		frm.CalculateSystemPercentages();
	}

	void Engine::EndFrame()
	{
		Core::FactoryInstance.UpdateDestroyed();

		auto window = Core::GetInstance()->GetWindow();
		if (glfwWindowShouldClose(window))
			isRunning = false;

		glfwSwapBuffers(window);
	}

	void Engine::Exit()
	{
		//Core::GetInstance()->UnbindSystems();
		Core::GetInstance()->ExitCore();
		audio->Exit();

		//Window::CloseWindow(window);
		SLICE_LOG("Shutting Down Slice Engine.");
	}

}