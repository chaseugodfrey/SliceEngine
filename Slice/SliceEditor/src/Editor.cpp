#include <pch.h>
#include "Editor.h"
#include "../../src/Input/InputSystem.h"

namespace SliceEditor
{
	void Editor::Init()
	{
		SLICE_LOG("Initializing Editor.");
		engine.Init();

		// todo: calling this here first to put this when loading scene + 
		// reminder to change scene root to a list in case we want to have multiple scenes
		SliceEngine::Core::GetInstance()->mFactory.InitRootEntity();

		InitImGUI(SliceEngine::Core::GetInstance()->GetWindow());
		SLICE_LOG("Initializing Editor Systems.");

		InitManagers();
		InitWindowManager();

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

	void Editor::Render()
	{
		glfwMakeContextCurrent(SliceEngine::Core::GetInstance()->GetWindow());
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		// in order to toggle game input on/off from editor UI w/o restarting, tell inputsystem if imgui is capturing input this frame
		// editor tells inputsystem each frame whether imgui is using keyboard/mouse
		ImGuiIO& io = ImGui::GetIO();
		auto inputs = SliceEngine::Core::GetInstance()->GetInputSystem();
		inputs->SetImGuiCapture(io.WantCaptureKeyboard, io.WantCaptureMouse); // set imgui capture state in inputsystem to capture input


		windowManager.Render();
		
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
		glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
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
		SLICE_LOG("Registering Systems to WindowManager.");

		auto& managers = registry.GetManagers();

		for (const auto& [key, value] : managers)
		{
			if (auto other = dynamic_cast<ICreateWindow*>(value.get()))
			{
				SLICE_LOG(key);
				windowManager.RegisterInterface(key, other);
			}
		}

		//windowManager.RegisterInterface("Profiler", &profilerManager);

		windowManager.Init();
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