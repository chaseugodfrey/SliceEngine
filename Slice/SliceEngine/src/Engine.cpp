#include <pch.h>
#include "Engine.h"
#include "ECS/ECSTypes.h"
#include "Physics/PhysicsSystem.h"
//#include "Window.h"
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
#include "SliceTime.h"
#include "test.h"
#include "Serializer/JSONSerializer.h"
#include "Serializer/CSVSerializer.h"


	using namespace rttr;

	struct MyStruct { MyStruct() {}; void func(double) {}; int data; };

	RTTR_REGISTRATION
	{
		registration::class_<MyStruct>("MyStruct")
			 .constructor<>()
			 .property("data", &MyStruct::data)
			 .method("func", &MyStruct::func);
	}

namespace SliceEngine
{
	//Time class for physics simulation or any other system that uses fixeddt
	GameTime& Engine::gameTime = GameTime::getInstance();

	Engine::Engine()
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

		inputs = std::make_unique<InputSystem>();
		inputs->Init(window);
		audio = std::make_unique<AudioManager>();
		//mResource = std::make_unique<ResourceManager>();

		Core::GetInstance()->InitSystem<SoundSystem>();
		Core::GetInstance()->InitSystem<WorldSpaceGraphicsSystem>();
		Core::GetInstance()->InitSystem<TransformSystem>();

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

		entt::entity newCam = Core::GetInstance()->GetRegistry().create();
		Core::GetInstance()->GetRegistry().emplace<Transform>(newCam);
		Core::GetInstance()->GetRegistry().emplace<Renderer>(newCam);

		//test();

		
		JSONSerializer::Test2();
	}

	void Engine::Update()
	{
		auto mResource = Core::GetInstance()->GetResourceManager();
		auto mRender = Core::GetInstance()->GetRenderManager();

		glfwMakeContextCurrent(Core::GetInstance()->GetWindow());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwPollEvents();

		// Main Body
		//framerateManager->StartFrame();

		//framerateManager->StartSystem("Input");
		if (inputs->IsKeyDown(GLFW_KEY_LEFT))
		{
			std::cout << " test " << std::endl;
		}
		inputs->Update();
		//framerateManager->EndSystem("Input");

		//framerateManager->EndFrame();
		//

		mRender->Render(mResource);
	}

	void Engine::EndFrame()
	{
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