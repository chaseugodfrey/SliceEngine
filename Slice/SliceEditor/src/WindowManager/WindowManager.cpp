#include <pch.h>
#include "WindowManager.h"
#include "ICreateWindow.h"
#include "../../src/Input/InputSystem.h"

namespace SliceEditor
{

	void WindowManager::Init()
	{
		SLICE_LOG("Initializing WindowManager.");

		AddWindow("ContentBrowser");
		AddWindow("Profiler");
		AddWindow("SceneView");
		AddWindow("Hierarchy");
		AddWindow("Inspector");
		//AddWindow<ContentBrowserWindow>();
		//AddWindow<SceneViewWindow>();
		//AddWindow<GameView>(editorState);
		//AddWindow<Hierarchy>(editorState);
		//AddWindow<Inspector>(editorState);
		//AddWindow<Console>();
		//AddWindow<Animator>();
		//AddWindow<Profiler>();
		//AddWindow<Animation>();
	}

	void WindowManager::RegisterInterface(const std::string& name, ICreateWindow* interfaceInstance)
	{
		windowFactoryMap[name] = interfaceInstance;
	}

	void WindowManager::AddWindow(const std::string& name)
	{
		auto it = windowFactoryMap.find(name);
		if (it != windowFactoryMap.end())
		{
			auto window = it->second->CreateWindow();
			list.push_back(std::move(window));
		}
		else
		{
			SLICE_LOG_ERROR(std::string("No registered window with name: ") + name.c_str());
		}
	}

	void WindowManager::Render()
	{
		DrawMainMenu();
		DrawPlayState();
		DrawDockspace();
		DrawPlayState();
		
		for (auto& window : list)
		{
			window->Draw();
		}
	}

	void WindowManager::DrawMainMenu()
	{
		auto style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 10.0f));
		ImGui::SetNextWindowSize({ 0, 30 });
		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene"))
			{

			}

			if (ImGui::MenuItem("Save Scene"))
			{

			}

			ImGui::Separator();

			if (ImGui::MenuItem("Preferences"))
			{

			}

			if (ImGui::MenuItem("Exit"))
			{

			}

			ImGui::Separator();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Animation"))
			{

			}

			if (ImGui::MenuItem("Animator"))
			{

			}

			if (ImGui::MenuItem("Console"))
			{

			}

			if (ImGui::MenuItem("Content Browser"))
			{

			}

			if (ImGui::MenuItem("Game"))
			{

			}

			if (ImGui::MenuItem("Hierachy"))
			{

			}

			if (ImGui::MenuItem("Inspector"))
			{

			}

			if (ImGui::MenuItem("Scene"))
			{

			}

			if (ImGui::MenuItem("Profiler"))
			{

			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
		ImGui::PopStyleVar();

	}

	void WindowManager::DrawDockspace()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::Begin("Dockspace", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoMove);
		ImGuiID dockspace_id = ImGui::GetID("Dockspace");
		ImGui::DockSpace(dockspace_id, { 0,0 }, ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::End();

	}

	void WindowManager::DrawPlayState()
	{
		auto* window = SliceEngine::Core::GetInstance()->GetWindow();

		int xPos{}, yPos{}, width{}, height{};
		glfwGetWindowPos(window, &xPos, &yPos);
		glfwGetWindowSize(window, &width, &height);

		ImGui::BeginViewportSideBar("PlayBar", ImGui::GetMainViewport(), ImGuiDir_Up, 50.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking);

        ImGuiIO& io = ImGui::GetIO();
		auto inputs = SliceEngine::Core::GetInstance()->GetInputSystem();
		inputs->SetImGuiCapture(io.WantCaptureKeyboard, io.WantCaptureMouse);

        static bool isPlaying = false;

		if (ImGui::Button("Play", ImVec2{ 60, 35 }))
        {
         isPlaying = !isPlaying;

			if (isPlaying) // if its play, enable game input
			{
				inputs->SetMode(SliceEngine::InputMode::Game); // set input mode to game
				inputs->BindCallbacksToWindow(SliceEngine::Core::GetInstance()->GetWindow()); // bind callbacks to window so game can receive input
			}
			else // else, keep input in editor mode and unbind callbacks, leaving it to imgui
			{
				inputs->UnbindCallbacks();
				inputs->SetMode(SliceEngine::InputMode::Editor);
			}   
        }
        ImGui::SameLine();
		if (ImGui::Button("Pause", ImVec2{ 60, 35 }));

		ImGui::End();
	}

}
