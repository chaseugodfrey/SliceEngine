/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        Editor.cpp

 author:	  Chase Rodrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the Editor class, which is the main class of the editor application.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "Editor.h"
#include "Scripting/ScriptEditor.h"
#include <Input/InputSystem.h>
#include <Systems/SceneSystem.h>
#include <Graphics/TransformHelper.h>

namespace SliceEditor
{
	void Editor::MasterKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{

		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
		auto inputSys = SliceEngine::Core::GetInstance()->GetInputSystem();
		if (inputSys->GetMode() == SliceEngine::InputMode::Game)
		{
			if (action == GLFW_PRESS)
			{
				// update that particular key to pressed state
				inputSys->UpdateKeyMap(key, SliceEngine::KeyStates::PRESS);
			}
			else if (action == GLFW_RELEASE)
			{
				inputSys->UpdateKeyMap(key, SliceEngine::KeyStates::RELEASE);
			}
		}
	}
	void Editor::MasterMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

		// 2. Check if ImGui wants to capture the mouse
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse)
		{
			return; // Stop processing, ImGui has it
		}

		auto input = SliceEngine::Core::GetInstance()->GetInputSystem();
		if (action == GLFW_PRESS)
		{
			input->UpdateMouseMap(button, SliceEngine::KeyStates::PRESS);
		}
		else if (action == GLFW_RELEASE)
		{
			input->UpdateMouseMap(button, SliceEngine::KeyStates::RELEASE);
		}
	}

	void Editor::Init()
	{
		SLICE_LOG("Initializing Editor.");

		// Scan the resource folder for any hanging resource files or smth
		// before engine's resource manager scans it to prevent broken meta files/resource files

		assetManager.ScanResourceFolder();
		assetManager.Init();

		engine.Init();

		/*
		SliceEngine::GameObject FloorTest = SliceEngine::Core::GetInstance()->mFactory.CreateGO("FloorQuad");
        FloorTest.AddComponent<SliceEngine::Renderer>();
        FloorTest.GetComponent<SliceEngine::Renderer>().model = static_cast<SliceEngine::GUID>(SliceEngine::DefaultResourceIDs::QUAD_DEFAULT);
        FloorTest.GetComponent<SliceEngine::Transform>().rotation = SliceEngine::Vec3ToQuat(glm::vec3(-90.f, 0.f, 0.f));
        FloorTest.GetComponent<SliceEngine::Transform>().scale = glm::vec3(10.f, 10.f, 10.f); // Scale it up!

        auto &transform = FloorTest.GetComponent<SliceEngine::Transform>();

        // Build transformation matrix
        glm::mat4 transformMatrix = glm::translate(glm::mat4(1.0f), transform.position)
            * glm::mat4_cast(transform.rotation)
            * glm::scale(glm::mat4(1.0f), transform.scale);

        auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
        auto &model = *rm->get<SliceEngine::SliceEngineTypes::Model>(FloorTest.GetComponent<SliceEngine::Renderer>().model).get();
		*/
		SliceEngine::GameObject FloorTest1 = SliceEngine::Core::GetInstance()->mFactory.CreateGO("FloorQuad");
		FloorTest1.AddComponent<SliceEngine::Renderer>();
		FloorTest1.GetComponent<SliceEngine::Renderer>().model = static_cast<SliceEngine::GUID>(SliceEngine::DefaultResourceIDs::SPHERE_LOW_POLY_DEFAULT);
		FloorTest1.GetComponent<SliceEngine::Transform>().rotation = SliceEngine::Vec3ToQuat(glm::vec3(-90.f, 0.f, 0.f));
		FloorTest1.GetComponent<SliceEngine::Transform>().scale = glm::vec3(10.f, 10.f, 10.f);

		auto &transform1 = FloorTest1.GetComponent<SliceEngine::Transform>();
		transform1.position = glm::vec3(-10.f, 5.f, -10.f);

		// Build transformation matrix
		glm::mat4 transformMatrix1 = glm::translate(glm::mat4(1.0f), transform1.position)
			* glm::mat4_cast(transform1.rotation)
			* glm::scale(glm::mat4(1.0f), transform1.scale);

		auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
		auto &model1 = *rm->get<SliceEngine::SliceEngineTypes::Model>(FloorTest1.GetComponent<SliceEngine::Renderer>().model).get();

		SliceEngine::GameObject FloorTest2 = SliceEngine::Core::GetInstance()->mFactory.CreateGO("FloorQuad");
		FloorTest2.AddComponent<SliceEngine::Renderer>();
		FloorTest2.GetComponent<SliceEngine::Renderer>().model = static_cast<SliceEngine::GUID>(SliceEngine::DefaultResourceIDs::QUAD_DEFAULT);
		FloorTest2.GetComponent<SliceEngine::Transform>().rotation = SliceEngine::Vec3ToQuat(glm::vec3(-90.f, 0.f, 0.f));
		FloorTest2.GetComponent<SliceEngine::Transform>().scale = glm::vec3(10.f, 10.f, 10.f);

		auto &transform2 = FloorTest2.GetComponent<SliceEngine::Transform>();
		transform2.position = glm::vec3(1.0f, 0.0f, 0.0f);

		// Build transformation matrix
		glm::mat4 transformMatrix2 = glm::translate(glm::mat4(1.0f), transform2.position)
			* glm::mat4_cast(transform2.rotation)
			* glm::scale(glm::mat4(1.0f), transform2.scale);

		auto &model2 = *rm->get<SliceEngine::SliceEngineTypes::Model>(FloorTest2.GetComponent<SliceEngine::Renderer>().model).get();

		std::vector<SliceEngine::SliceEngineTypes::Model> models = { model1, model2 };
		std::vector<glm::mat4> transforms = { transformMatrix1, transformMatrix2 };

		if (navMesh.BuildFromModel(models, transforms))
		{
			SLICE_LOG_DEBUG("NAVMESH BUILT SUCCESSFULLY");
		}
		else
		{
			SLICE_LOG_ERROR("NAVMESH NOT BUILT");
		}

		auto inputSys = SliceEngine::Core::GetInstance()->GetInputSystem();
		inputSys->UnbindCallbacks(); // unbind input callbacks, let editor handle input

		// todo: calling this here first to put this when loading scene + 
		// reminder to change scene root to a list in case we want to have multiple scenes
		//SliceEngine::Core::GetInstance()->mFactory.InitRootEntity();

		InitImGUI(SliceEngine::Core::GetInstance()->GetWindow());
		SLICE_LOG("Initializing Editor Systems.");

		InitManagers();
		InitWindowManager();

		//SliceEditor::InitFileWatcher();

		inputSys->SetMode(SliceEngine::InputMode::Editor);
		inputs.isActive = true;
		
	}

	void Editor::Run()
	{
		while (!glfwWindowShouldClose(SliceEngine::Core::GetInstance()->GetWindow()))
		{
			registry.Update();
			inputs.Update();
			engine.Update();
			Render();
			engine.EndFrame();
		}
	}

	void Editor::Render()
	{
		glfwMakeContextCurrent(SliceEngine::Core::GetInstance()->GetWindow());
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		// in order to toggle game input on/off from editor UI w/o restarting, tell inputsystem if imgui is capturing input this frame
		// editor tells inputsystem each frame whether imgui is using keyboard/mouse
		ImGuiIO& io = ImGui::GetIO();
		auto inputs = SliceEngine::Core::GetInstance()->GetInputSystem();
		inputs->SetImGuiCapture(io.WantCaptureKeyboard, io.WantCaptureMouse); // set imgui capture state in inputsystem to capture input

		registry.GetManager<WindowManager>("Windows")->Render();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	void Editor::Exit()
	{
		navMesh.Clear();
		engine.Exit();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	// Inits

	void Editor::InitImGUI(GLFWwindow* window)
	{
		SLICE_LOG("Checking ImGUI Version.");
		IMGUI_CHECKVERSION();

		SLICE_LOG("Creating ImGui Context.");
		SLICE_LOG_VALUES("ImGui Version: ", IMGUI_VERSION);
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		//io.IniFilename = nullptr; //To disable the config for ImGui

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
		ImGui_ImplOpenGL3_Init("#version 450");
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, MasterKeyCallback);
		glfwSetMouseButtonCallback(window, MasterMouseButtonCallback);
		glfwSetDropCallback(window, Editor::DropCallback);
	}

	void Editor::InitManagers()
	{
		SLICE_LOG("EDITOR: Initializing Managers.");
		registry.Init();
	}

	void Editor::InitEditorState()
	{
		SLICE_LOG("EDITOR: Initializing Session.");
		//editorState = std::make_unique<EditorState>();
		//editorState->Init();
	}

	void Editor::InitWindowManager()
	{

	}

	void Editor::DropCallback(GLFWwindow* window, int count, const char** paths)
	{
		Editor* editor = static_cast<Editor*>(glfwGetWindowUserPointer(window));

		for (int i = 0; i < count; i++)
		{
			std::filesystem::path path = paths[i];
			//Handles folders just in case
			if (std::filesystem::is_directory(path))
			{
				for (auto& entry : std::filesystem::recursive_directory_iterator(path))
				{
					editor->HandleDrop(entry.path());
				}
			}
			else
			{
				editor->HandleDrop(path);
			}
		}
		auto manager = editor->registry.GetManager<ContentBrowserManager>("ContentBrowser");
		manager->RebuildDirectory(*manager->rootNode);
	}

	void Editor::HandleDrop(const std::filesystem::path path)
	{
		auto manager = registry.GetManager<ContentBrowserManager>("ContentBrowser");
		auto target = manager->selectedFolder->path / path.filename();

		std::filesystem::copy(path, target, std::filesystem::copy_options::overwrite_existing);
		SLICE_LOG("Dropped this file: " + path.filename().string());
		//DirectoryNode node = *manager->selectedFolder;
		manager->RebuildDirectory(*manager->rootNode);
		//manager->SetSelectedFolder(node);

		//Create the Package for the ContentBrowser to read
		std::string fileExt = target.extension().string();

		if (registry.GetAssetManager().mSupportedAssetTypes.find(fileExt) == registry.GetAssetManager().mSupportedAssetTypes.end())
		{
			SLICE_LOG_VALUES("Dropped Unsupported Asset Type");
			return;
		}

		DroppedFile file;

		file.assetType = registry.GetAssetManager().mSupportedAssetTypes[fileExt].first;
		file.filePath = target;
		switch (file.assetType)
		{
		case AssetType::Texture:
			file.metaData = std::make_unique<TextureData>();
			break;
		case AssetType::Model:
			file.metaData = std::make_unique<ModelData>();
			break;
		case AssetType::Audio:
			file.metaData = std::make_unique<AudioData>();
			break;
		case AssetType::Scene:
			file.metaData = std::make_unique<SceneData>();
			break;
		case AssetType::Shader:
			file.metaData = std::make_unique<ShaderData>();
			break;
		case AssetType::Prefab:
			file.metaData = std::make_unique<PrefabData>();
			break;
		}
		//Default Init the MetaData base class
		file.metaData->InitMetaData(target, file.assetType, registry.GetAssetManager().mAssetExtensions[file.assetType]);

		manager->mPendingDrops.push(std::move(file));
	}



}