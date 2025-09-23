#include <pch.h>
#include "Editor.h"

namespace SliceEditor
{
	void Editor::Init()
	{
		SLICE_LOG("Initializing Editor.");
		engine.Init();
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

	void Editor::InitManagers()
	{
		SLICE_LOG("EDITOR: Initializing Managers.");
		contentBrowserManager.Init();

		hierarchyManager = std::make_unique<HierarchyManager>();
		// find a way to make tihs look prettier tbh
		sceneViewManager = std::make_unique<SceneViewManager>(*SliceEngine::RenderManagerInstance);
		//sceneViewManager = std::make_unique<SceneViewManager>(*SliceEngine::Core::GetInstance()->GetRenderManager());
		inspectorManager = std::make_unique<InspectorManager>();
		hierarchyManager->Init();
		sceneViewManager->Init();

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
		windowManager.RegisterInterface("ContentBrowser", &contentBrowserManager);
		windowManager.RegisterInterface("SceneView", sceneViewManager.get());
		windowManager.RegisterInterface("Hierarchy", hierarchyManager.get());
		windowManager.RegisterInterface("Inspector", inspectorManager.get());
		windowManager.Init();
	}
}