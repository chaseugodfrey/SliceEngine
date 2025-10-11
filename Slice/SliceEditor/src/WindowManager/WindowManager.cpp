/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        WindowManager.cpp

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the WindowManager which is responsible for creating and managing all editor windows.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "WindowManager.h"
#include "ICreateWindow.h"
#include "../Core/Registry.h"
#include "Scripting/ScriptEditor.h"
#include "../Hierachy/HierarchyManager.h"
#include <Input/InputSystem.h>
#include <Scripting/ScriptSystem.h>
#include <Core/ComponentEventHandler.h>
#include <Configuration/ProjectSettings.h>
#include <Systems/SceneSystem.h>
#include <Networking/NetworkSystem.h>

namespace SliceEditor
{

	void WindowManager::Init()
	{
		SLICE_LOG("Initializing WindowManager.");
		SLICE_LOG("Registering Systems to WindowManager.");

		auto& managers = registry.GetManagers();

		for (const auto& [key, value] : managers)
		{
			if (auto other = dynamic_cast<ICreateWindow*>(value.get()))
			{
				RegisterInterface(key, other);
			}
		}

		// Create windows
		// todo: maybe read from imgui ini file and load accordingly
		AddWindow("ContentBrowser");
		AddWindow("Profiler");
		AddWindow("SceneView");
		AddWindow("Hierarchy");
		AddWindow("Inspector");
		AddWindow("GameView");
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
			auto window = it->second->CreateEditorWindow();
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
		DrawProjectSettings();
		
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

		//auto core = SliceEngine::Core::GetInstance();

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene"))
			{

			}

			if (ImGui::MenuItem("Save Scene"))
			{
				SliceEngine::Core::GetInstance()->GetSceneSystem()->SaveCurrentScene();
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Project Settings"))
			{
				projectSettingsPopupOpen = true;
			}

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

		auto& factory = SliceEngine::Core::GetInstance()->mFactory;

		if (ImGui::BeginMenu("GameObject"))
		{
			if (ImGui::BeginMenu("3D Object"))
			{
				if (ImGui::MenuItem("Box"))
				{
					auto go = factory.CreateGO_Box();
					registry.GetManager<HierarchyManager>("Hierarchy")->AddEntityDirectly(go.GetEntity());

				}

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Camera"))
			{
				auto go = factory.CreateGO_Cam();
				registry.GetManager<HierarchyManager>("Hierarchy")->AddEntityDirectly(go.GetEntity());
			}

			ImGui::EndMenu();
		}

#pragma region Custom Title Bar (Disabled for now)
		//// todo : custom title bar!!!
		//ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 30);
		//if (ImGui::ButtonEx("X##close", ImVec2{ 30,30 }, ImGuiButtonFlags_PressedOnRelease))
		//{
		//	// todo : create engine window functionality for this
		//	glfwSetWindowShouldClose(SliceEngine::Core::GetInstance()->GetWindow(), true);
		//}
#pragma endregion

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

		if(!isPlaying)
		{
			if (ImGui::Button("Play", ImVec2{ 60, 35 }))
			{
				isPlaying = !isPlaying;

				if (isPlaying) // if its play, enable game input
				{
					inputs->SetMode(SliceEngine::InputMode::Game); // set input mode to game
					inputs->SetEnabled(true);
					SliceEngine::gScriptSystem->OnStart();
					//inputs->BindCallbacksToWindow(SliceEngine::Core::GetInstance()->GetWindow()); // bind callbacks to window so game can receive input
				}
				else // else, keep input in editor mode and unbind callbacks, leaving it to imgui
				{
					//inputs->UnbindCallbacks();
					inputs->SetMode(SliceEngine::InputMode::Editor);
					inputs->SetEnabled(false);
				}
			}
		}
		else
		{
			if(ImGui::Button("Stop", ImVec2{ 60, 35 }))
			{
				isPlaying = !isPlaying;
				if (isPlaying) // if its play, enable game input
				{
					inputs->SetMode(SliceEngine::InputMode::Game); // set input mode to game
					inputs->SetEnabled(true);
					SliceEngine::gScriptSystem->OnStart();
					//inputs->BindCallbacksToWindow(SliceEngine::Core::GetInstance()->GetWindow()); // bind callbacks to window so game can receive input
				}
				else // else, keep input in editor mode and unbind callbacks, leaving it to imgui
				{
					//inputs->UnbindCallbacks();
					inputs->SetMode(SliceEngine::InputMode::Editor);
					inputs->SetEnabled(false);
				}
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Pause", ImVec2{ 60, 35 }))
		{

		}

		ImGui::SameLine();
		if (ImGui::Button("Bind", ImVec2{ 60, 35 }))
		{
			ImGui::OpenPopup("host_req");
		}
		ImGui::SameLine();
		if (ImGui::Button("Reload Scripts", ImVec2{ 60,35 }))
		{
			if (SliceEngine::gScriptSystem)
			{
				SliceEngine::gScriptSystem->ReloadAssembly();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Connect", ImVec2{ 60, 35 }))
		{
			ImGui::OpenPopup("connect_req");

		}

		if (ImGui::BeginPopup("host_req"))
		{

			ImGui::Text("Input Port: ");
			ImGui::SameLine();
			static std::string bindport;
			if (ImGui::InputText("##port_in", &bindport))
			{
			}

			if (ImGui::Button("Bind"))
			{
				/*std::string portNumber{};

				std::ifstream pfile("Assets/port.txt");
				if (!pfile)
				{
					std::cerr << "cannot open client file" << std::endl;
				}
				std::getline(pfile, portNumber);

				pfile.close();*/
				SliceEngine::OnNetworkBindPort(bindport);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Close"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("connect_req"))
		{
			SliceEngine::NetworkSystem* netw = SliceEngine::Core::GetInstance()->GetNetwork();

			std::string display = "Current IP: ";
			if (netw->data.IP.empty())
			{
				display += "Bind First";
			}
			else
			{
				display += netw->data.IP;
			}
			ImGui::Text(display.c_str());

			display = "Current Port: ";
			if (netw->data.port.empty())
			{
				display += "Bind First";
			}
			else
			{
				display += netw->data.port;
			}
			ImGui::Text(display.c_str());


			ImGui::Text("Input IP: ");
			ImGui::SameLine();
			static std::string ip;
			if (ImGui::InputText("##ip_in", &ip))
			{
				// changed
			}

			ImGui::Text("Input Port: ");
			ImGui::SameLine();
			static std::string port;
			if (ImGui::InputText("##new_port_in", &port))
			{
				// changed
			}

			if (ImGui::Button("Connect"))
			{
				/*std::pair<std::string, std::string> clientNumber{};
				std::ifstream cfile("Assets/client.txt");
				if (!cfile)
				{
					std::cerr << "cannot open client file" << std::endl;
				}
				std::getline(cfile, clientNumber.first);
				std::getline(cfile, clientNumber.second);

				cfile.close();*/
				SliceEngine::OnNetworkClientConnect(ip, port);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Close"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		

		ImGui::End();
	}

	void WindowManager::DrawProjectSettings()
	{
		if (!projectSettingsPopupOpen)
			return;

		bool isOpen;
		if (ImGui::Begin("project_settings_window", &isOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize ))
		{
			auto gSettings = SliceEngine::Core::GetInstance()->GetProjectSettingsService();
			auto& s = gSettings->Edit(); // we�ll set dirty only if something changes

			bool changed = false;
			if (ImGui::InputText("Product Name", &s.productName)) { changed = true; }
			int w = s.width, h = s.height;
			if (ImGui::InputInt("Width", &w)) { s.width = std::max(16, w); changed = true; }
			if (ImGui::InputInt("Height", &h)) { s.height = std::max(16, h); changed = true; }

			// Scenes list (very basic)
			for (size_t i = 0;i < s.scenes.size();++i) {
				ImGui::PushID((int)i);
				ImGui::InputText("Scene Path", &s.scenes[i]); // path string edit
				if (ImGui::SmallButton("Up") && i > 0) { std::swap(s.scenes[i], s.scenes[i - 1]); changed = true; }
				ImGui::SameLine();
				if (ImGui::SmallButton("Down") && i + 1 < s.scenes.size()) { std::swap(s.scenes[i], s.scenes[i + 1]); changed = true; }
				ImGui::SameLine();
				if (ImGui::SmallButton("X")) { s.scenes.erase(s.scenes.begin() + i); changed = true; ImGui::PopID(); break; }
				ImGui::PopID();
			}
			if (ImGui::Button("+ Add Scene")) { s.scenes.emplace_back("Assets/Scenes/New.scene"); changed = true; }

			// Startup scene combo
			if (!s.scenes.empty()) {
				int current = 0;
				for (int i = 0;i < (int)s.scenes.size();++i) if (s.scenes[i] == s.startupScene) current = i;
				if (ImGui::BeginCombo("Startup Scene", s.scenes[current].c_str())) {
					for (int i = 0;i < (int)s.scenes.size();++i) {
						bool sel = (i == current);
						if (ImGui::Selectable(s.scenes[i].c_str(), sel)) { s.startupScene = s.scenes[i]; changed = true; }
					}
					ImGui::EndCombo();
				}
			}

			// Save/Reload row
			if (ImGui::Button("Save")) gSettings->Save();
			ImGui::SameLine();
			if (ImGui::Button("Reload")) { gSettings->Load(); }

			// Set dirty timing + optional autosave
			if (changed) {
				// touching Edit() already marked dirty; reset the debounce timer by re-setting the change time
				// simplest: mark as dirty again; DebouncedAutosave accumulates time each frame
			}

			gSettings->DebouncedAutosave(1.0f/60.0f, /*delay*/0.75);

			// External change detection (prompt)
			if (gSettings->DetectExternalChange()) {
				ImGui::TextDisabled("ProjectSettings.json changed on disk.");
				ImGui::SameLine();
				if (ImGui::Button("Reload from Disk")) gSettings->Load();
			}

			ImGui::End();
		}

		projectSettingsPopupOpen = isOpen;

	}

}
