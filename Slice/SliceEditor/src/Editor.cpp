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
#include <Core/Registry.h>
#include <Input/InputSystem.h>
#include <ContentBrowser/ContentBrowserManager.h>
#include <Systems/SceneSystem.h>
#include <Graphics/TransformHelper.h>
#include <WindowManager/WindowManager.h>
#include <Configuration/PreferenceManager.h>
#include <Systems/FramerateManager.h>

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
		if (action == GLFW_PRESS)
		{
			// update that particular key to pressed state
			inputSys->UpdateKeyMap(key, SliceEngine::KeyStates::PRESSED);
		}
		else if (action == GLFW_RELEASE)
		{
			inputSys->UpdateKeyMap(key, SliceEngine::KeyStates::RELEASED);
		}
	}
	void Editor::MasterMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
		auto input = SliceEngine::Core::GetInstance()->GetInputSystem();

		if (action == GLFW_PRESS)
		{
			input->UpdateMouseMap(button, SliceEngine::KeyStates::PRESSED);
			//std::cout << "Mouse Button Pressed: " << std::endl;
		}
		else if (action == GLFW_RELEASE)
		{
			input->UpdateMouseMap(button, SliceEngine::KeyStates::RELEASED);
		}

		// 2. Check if ImGui wants to capture the mouse
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse)
		{
			return; // Stop processing, ImGui has it
		}

		
	}

	void Editor::Init()
	{
		SLICE_LOG("Initializing Editor.");
		//EnableMemoryLeakChecking(-1);

		// Scan the resource folder for any hanging resource files or smth
		// before engine's resource manager scans it to prevent broken meta files/resource files

		assetManager.Init();

		// Engine Core
		engine.Init();
		inputs = std::make_unique<EditorInputs>(registry, false);
		auto inputSys = SliceEngine::Core::GetInstance()->GetInputSystem();
		inputSys->UnbindCallbacks(); // unbind input callbacks, let editor handle input
		
		// Editor Core
		InitImGUI(SliceEngine::Core::GetInstance()->GetWindow());
		InitManagers();

		// Load Preferences & Set starting scene
		auto preferenceManager = registry.GetManager<PreferenceManager>("Preferences");
		preferenceManager->LoadPreferences();
		preferenceManager->SetPreferences();

		inputSys->SetMode(SliceEngine::InputMode::Editor);
		inputs->isActive = true;
		
	}

	void Editor::Run()
	{
		auto contentBrowser = registry.GetManager<ContentBrowserManager>("ContentBrowser");
		auto core = SliceEngine::Core::GetInstance();
		auto engineFRM = core->GetFramerateManager();

		while (!glfwWindowShouldClose(core->GetWindow()))
		{
			engineFRM->StartFrame();
			engineFRM->StartSystem("GLFW Swap Buffers");
			glfwMakeContextCurrent(core->GetWindow());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glfwPollEvents();
			core->GetInputSystem()->UpdatePrevInput();
			engineFRM->EndSystem("GLFW Swap Buffers");

			engineFRM->StartSystem("Editor Update");
			registry.Update();
			inputs->Update();
			if (contentBrowser)
			{
				AssetFileWatcher::UpdateFolder(*contentBrowser, assetManager);
			}
			engineFRM->EndSystem("Editor Update");

			//engineFRM->StartSystem("Engine");
			engine.Update(); //FRM already handled inside
			//engineFRM->EndSystem("Engine");
			engineFRM->StartSystem("Editor Render");
			Render();
			engineFRM->EndSystem("Editor Render");
			engine.EndFrame(); //FRM already handled inside

			engineFRM->EndFrame();
			engineFRM->CalculateSystemPercentages();

		}
	}

	void Editor::Render()
	{
		glfwMakeContextCurrent(SliceEngine::Core::GetInstance()->GetWindow());
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		registry.GetManager<WindowManager>("Windows")->Render();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	void Editor::Save()
	{
		registry.GetManager<PreferenceManager>("Preferences")->SavePreferences(false);
	}

	void Editor::Exit()
	{
		// Save all editor changes
		// to add save engine changes if needed
		Save();

		assetManager.CleanUpSceneTemp();
		engine.Exit();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImNodes::DestroyContext();
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
		ImNodes::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		io.Fonts->Clear(); // i dont want jetbrains, fuck that shit
		ImFont* font = io.Fonts->AddFontFromFileTTF("Assets/Fonts/Roboto-VariableFont.ttf", 14.);
		if (font) io.FontDefault = font;

		
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

		//glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos)	//yoinked this sht from inputsys.cpp
		//	{
		//		ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
		//		//ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
		//		SliceEngine::Core::GetInstance()->GetInputSystem()->SetMousePosition(xpos, ypos);
		//	});
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
		manager->RebuildDirectory();
	}

	void Editor::HandleDrop(const std::filesystem::path path)
	{
		auto manager = registry.GetManager<ContentBrowserManager>("ContentBrowser");
		const std::filesystem::path selectedfolderPath = manager->selectedFolder->fullPath;
		std::filesystem::path target = selectedfolderPath /path.filename();
		int counter = 1;

		while (std::filesystem::exists(target))
		{
			target = selectedfolderPath / (path.stem().string() + "_" + std::to_string(counter) + path.extension().string());
			++counter;
		}

		//Need to check and rename if the name already exists

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
		case AssetType::Font:
			file.metaData = std::make_unique<FontMetaData>();
			break;
		case AssetType::SequencePackage:
			file.metaData = std::make_unique<SequencePkgData>();
			break;
		default:
			SLICE_LOG_WARNING("File Type not supported in HandleDrop function yet: " + fileExt +". Letting Filewatcher handle.");
			return;
		}
		//Default Init the MetaData base class
		file.metaData->InitMetaData(target, file.assetType, registry.GetAssetManager().mAssetExtensions[file.assetType]);

		manager->mPendingDrops.push(std::move(file));
	}



}