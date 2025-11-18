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
#include <WindowManager/WindowManager.h>

namespace SliceEditor
{
	//Time class for physics simulation or any other system that uses fixeddt
	void EnableMemoryLeakChecking(int breakAlloc = -1)
	{
		int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
		_CrtSetDbgFlag(tmpDbgFlag);

		if (breakAlloc != -1) _CrtSetBreakAlloc(breakAlloc);
	}

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
		EnableMemoryLeakChecking(-1);

		// Scan the resource folder for any hanging resource files or smth
		// before engine's resource manager scans it to prevent broken meta files/resource files

		assetManager.ScanResourceFolder();
		assetManager.Init();

		engine.Init();

		auto inputSys = SliceEngine::Core::GetInstance()->GetInputSystem();
		inputSys->UnbindCallbacks(); // unbind input callbacks, let editor handle input

		// todo: calling this here first to put this when loading scene + 
		// reminder to change scene root to a list in case we want to have multiple scenes
		//SliceEngine::Core::GetInstance()->mFactory.InitRootEntity();

		//assetManager.CreateDefaultAsset(assetManager.mAssetDirectory, SliceEditor::AssetType::Controller);

		InitImGUI(SliceEngine::Core::GetInstance()->GetWindow());
		SLICE_LOG("Initializing Editor Systems.");

		InitManagers();
		InitWindowManager();

		engine.SceneInit();
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
			assetManager.UpdateFolder();
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
		auto input = SliceEngine::Core::GetInstance()->GetInputSystem();
		input->SetImGuiCapture(io.WantCaptureKeyboard, io.WantCaptureMouse); // set imgui capture state in inputsystem to capture input

		registry.GetManager<WindowManager>("Windows")->Render();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	void Editor::Exit()
	{
		navMesh.Clear();
		assetManager.CleanUpSceneTemp();
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
		//manager->RebuildDirectory(*manager->rootNode);
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