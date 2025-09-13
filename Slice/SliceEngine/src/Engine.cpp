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

		mRender->InitAndLink(window);

		//framerateManager = std::make_unique<FramerateManager>();
		//framerateManager->Init();
	}

	void Engine::Update()
	{

		//physics.Bind(mRegistry);

		////auto& entity = Core::GetInstance()->mFactory.CreateGO();
		//Entity entity = Core::GetInstance()->mRegistry.create();
		//Core::GetInstance()->mRegistry.emplace<Transform>(entity, glm::vec3(0.f), glm::vec3(50.f, 0.f, 0.f));
		//Core::GetInstance()->mRegistry.emplace<RigidBody>(entity, false);
		//Core::GetInstance()->mRegistry.emplace<Renderer>(entity);
		Entity const& entity = Core::GetInstance()->mFactory.CreateGO();
		bool ret = Core::GetInstance()->GetRegistry().valid(entity);
		//Core::GetInstance()->mRegistry.emplace<Transform>(entity.GetEntity(), glm::vec3(0.f), glm::vec3(50.f, 0.f, 0.f));
		Core::GetInstance()->GetRegistry().emplace_or_replace<Renderer>(entity);
		ret = Core::GetInstance()->GetRegistry().valid(entity);
		Core::GetInstance()->GetRegistry().emplace_or_replace<Camera>(entity);
		ret = Core::GetInstance()->GetRegistry().valid(entity);

		//entity.AddComponent<Renderer>();
		//entity.AddComponent<Transform>(glm::vec3(0.f), glm::vec3(50.f, 0.f, 0.f));
		//entity.AddComponent<RigidBody>(false);
		//entity.GetComponent<Transform>().position = glm::vec3(0.f);
		//entity.GetComponent<Transform>().rotation = glm::vec3(50.f, 0.f, 0.f);


		//Core::GetInstance()->mFactory.TestLoop();

		//auto& test = Core::GetInstance()->mRegistry.get<Transform>(entity.GetEntity());
		//mPhysicsTest->Update(2.0f);
		//physics(2.0f);

		//entity.RemoveComponent<RigidBody>();
		//Core::GetInstance()->mRegistry.remove<RigidBody>(entity);

		while (isRunning)
		{
			glfwMakeContextCurrent(window);
			glfwPollEvents();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Main Body
			//framerateManager->StartFrame();

			//framerateManager->StartSystem("Input");
			//inputs->Update();
			//framerateManager->EndSystem("Input");

			//framerateManager->EndFrame();
			////

			mRender->Render(window, mResource.get());


			glfwSwapBuffers(window);

			if (glfwWindowShouldClose(window))
				isRunning = false;
		}
	}

	void Engine::Exit()
	{
		Core::GetInstance()->UnbindSystems();
		audio->Exit();

		Window::CloseWindow(window);
		SLICE_LOG("Shutting Down Slice Engine.");
	}
}