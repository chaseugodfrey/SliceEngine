#include <pch.h>
#include "Editor.h"

namespace SliceEditor
{
	void Editor::Init()
	{
		SLICE_LOG("Initializing Editor.");
		engine.Init();
		InitImGUI(engine.window);
		//InitEditorState();
		InitWindowManager();
	}

	void Editor::Run()
	{
		while (!glfwWindowShouldClose(engine.window))
		{
			engine.Update();
			Render();
			engine.EndFrame();
		}
	}

	void Editor::Render()
	{
		glfwMakeContextCurrent(engine.window);
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		windowManager.Render();
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
		ImGui_ImplOpenGL3_Init("#version 450");
		glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
	}

	void Editor::InitEditorState()
	{
		SLICE_LOG("EDITOR: Initializing Session.");
		//editorState = std::make_unique<EditorState>();
		//editorState->Init();
	}

	void Editor::InitWindowManager()
	{
		//SLICE_LOG("EDITOR: Creating Window Manager.");
		windowManager.Init();
	}



}