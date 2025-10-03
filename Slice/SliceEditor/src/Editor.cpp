#include <pch.h>
#include "Editor.h"
#include "../../src/Input/InputSystem.h"

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
		engine.Init();
		auto inputSys = SliceEngine::Core::GetInstance()->GetInputSystem();
		inputSys->UnbindCallbacks(); // unbind input callbacks, let editor handle input

		// todo: calling this here first to put this when loading scene + 
		// reminder to change scene root to a list in case we want to have multiple scenes
		//SliceEngine::Core::GetInstance()->mFactory.InitRootEntity();

		InitImGUI(SliceEngine::Core::GetInstance()->GetWindow());
		SLICE_LOG("Initializing Editor Systems.");

		InitManagers();
		assetManager.Init();
		InitWindowManager();

		inputSys->SetMode(SliceEngine::InputMode::Editor);
	}

	void Editor::Run()
	{
		while (!glfwWindowShouldClose(SliceEngine::Core::GetInstance()->GetWindow()))
		{
			engine.Update();
			Render();
			engine.EndFrame();
		}
	}

	void Editor::CheckInputs()
	{
		if (ImGui::GetIO().KeyCtrl)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_S))
			{
				//SliceEngine::Core::GetInstance()->GetSceneSystem()->SaveScene();
			}
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
			editor->HandleDrop(path);
		}
	}

	void Editor::HandleDrop(const std::filesystem::path path)
	{
		auto manager = registry.GetManager<ContentBrowserManager>("ContentBrowser");
		auto target = manager->selectedFolder->path / path.filename();

		std::filesystem::copy(path, target, std::filesystem::copy_options::overwrite_existing);
		SLICE_LOG("Dropped this file: " + path.filename().string());
		manager->RebuildDirectory(*manager->rootNode);
	}



}