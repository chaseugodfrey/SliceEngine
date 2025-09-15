#include <pch.h>
#include "Engine.h"
#include "ECS/ECSTypes.h"
#include "ECS/PhysicSystem.h"
#include "Window.h"
#include "Core/Core.h"
#include "Input/InputSystem.h"
#include "AudioManager.h"
#include "Systems/TransformSystem.h"
#include "Graphics/ResourceManager.h"
#include "Graphics/RenderManager.h"
#include "ECS/BaseSystem.h"
#include "ECS/PhysicSystem.h"
#include "Systems/FramerateManager.h"


namespace SliceEngine
{
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
		window = Window::CreateWindow();
		Core::GetInstance()->InitFactory();
		// Set up Engine Systems
		isRunning = true;

		inputs = std::make_unique<InputSystem>();
		inputs->Init(window);
		audio = std::make_unique<AudioManager>();
		mResource = std::make_unique<ResourceManager>();

		Core::GetInstance()->InitSystem<SoundSystem>();
		Core::GetInstance()->InitSystem<WorldSpaceGraphicsSystem>();
		Core::GetInstance()->InitSystem<TransformSystem>();

		audio->Init();
		audio->LoadSound("BGMTest", "Assets/Audio/BGM_MainMenu_Mix1.wav", false, false);
		//audio->PlaySound("BGMTest", SliceEngine::SoundCategory::BGM, SliceEngine::AudioManager::InternalSound::SOUND_BGM, false, 0.5f);

		mResource->LoadShader("Assets/Shaders/basic.vert", "Assets/Shaders/basic.frag");
		mResource->LoadModel("Assets/Models/Cube.txt");
		mRender = std::make_unique<RenderManager>();
		Core::GetInstance()->InitSystem<CameraSystem>();

		mRender->CreateCamera(window);

		entt::entity newCam = Core::GetInstance()->GetRegistry().create();
		Core::GetInstance()->GetRegistry().emplace<Transform>(newCam);
		Core::GetInstance()->GetRegistry().emplace<Renderer>(newCam);

	}

	void Engine::Update()
	{
		glfwMakeContextCurrent(window);
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
		////

		mRender->Render(window, mResource.get());
	}

	void Engine::EndFrame()
	{
		if (glfwWindowShouldClose(window))
			isRunning = false;

		glfwSwapBuffers(window);
	}

	void Engine::Exit()
	{
		Core::GetInstance()->UnbindSystems();
		audio->Exit();

		Window::CloseWindow(window);
		SLICE_LOG("Shutting Down Slice Engine.");
	}
}