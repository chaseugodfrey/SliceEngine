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
#include "WindowTypes.h"

#include "Scripting/ScriptEditor.h"
#include <Input/InputSystem.h>
#include <Scripting/ScriptSystem.h>
#include <Core/ComponentEventHandler.h>
#include <Configuration/ProjectSettings.h>
#include <Systems/SceneSystem.h>
#include <Networking/NetworkSystem.h>
#include <Profiler/ProfilerManager.h>
#include <Input/ActionMapping.h>
#include <History/HistoryManager.h>

namespace SliceEditor
{
	static int DetectGlfwKeyPress(GLFWwindow* window)
	{
		// GLFW keys range
		for (int k = GLFW_KEY_SPACE; k <= GLFW_KEY_LAST; ++k)
		{
			if (glfwGetKey(window, k) == GLFW_PRESS) return k;
		}
		return -1;
	}

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

		AddWindow<ContentBrowserWindow>("ContentBrowser");
		AddWindow<ProfilerWindow>("Profiler");
		AddWindow<SceneViewWindow>();
		//AddWindow<PrefabViewWindow>();
		AddWindow<GameViewWindow>();
		AddWindow<HierarchyWindow>();
		AddWindow<InspectorWindow>();
		AddWindow<AnimatorWindow>();
		AddWindow<AnimationWindow>();
		AddWindow<ConsoleWindow>();

		EventManager::GetInstance()->Subscribe<OnGameStopEvent, &WindowManager::QuitGameEvent>(this);
	}

	void WindowManager::Update()
	{
		for (auto& window : list)
		{
			if (window->markForRemoval)
				list.erase(std::remove(list.begin(), list.end(), window));
		}
	}

	void WindowManager::RegisterInterface(const std::string& name, ICreateWindow* interfaceInstance)
	{
		windowFactoryMap[name] = interfaceInstance;
	}

	void WindowManager::Render()
	{
		DrawMainMenu();
		DrawPlayState();
		DrawDockspace();
		DrawProjectSettings();
		DrawSaveSceneAsPopup();
		DrawNewScenePopup();
		
		for (auto& window : list)
		{
			window->Draw();
		}

		DrawSavePopupModal();
	}

	void WindowManager::CloseSaveScenePopup()
	{
		saveScenePopupClose = true;
	}

	void WindowManager::OpenSaveScenePopup()
	{
		saveScenePopupOpen = true;
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
				newScenePopupOpen = true;
			}

			if (ImGui::MenuItem("Save Scene"))
			{
				EventManager::GetInstance()->Publish<OnSceneSaveEvent>();
			}

			if (ImGui::MenuItem("Save Scene As"))
			{
				saveSceneAsPopup = true;

			}

			ImGui::Separator();

			if (ImGui::MenuItem("Project Settings"))
			{
				AddWindow<ProjectSettingsWindow>();
			}

			if (ImGui::MenuItem("Preferences"))
			{
				AddWindow<PreferenceWindow>();
			}

			if (ImGui::MenuItem("Exit"))
			{

			}

			ImGui::Separator();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Undo History"))
			{
				AddWindow<HistoryWindow>();
			}

			if (ImGui::MenuItem("Content Browser"))
			{
				AddWindow<ContentBrowserWindow>("ContentBrowser");
			}

			if (ImGui::MenuItem("Console"))
			{
				AddWindow<ConsoleWindow>();
			}

			if (ImGui::MenuItem("Game"))
			{
				AddWindow<GameViewWindow>();
			}

			if (ImGui::MenuItem("Hierarchy"))
			{
				AddWindow<HierarchyWindow>();
			}

			if (ImGui::MenuItem("Inspector"))
			{
				AddWindow<InspectorWindow>();
			}

			if (ImGui::MenuItem("Scene"))
			{
				AddWindow<SceneViewWindow>();
			}

			if (ImGui::MenuItem("Prefab View"))
			{
				//AddWindow<PrefabViewWindow>();
			}

			if (ImGui::MenuItem("Profiler"))
			{
				AddWindow<ProfilerWindow>("Profiler");
			}

			if (ImGui::MenuItem("Animation"))
			{
				AddWindow<AnimationWindow>();
			}

			if (ImGui::MenuItem("Animator"))
			{
				AddWindow<AnimatorWindow>();
			}

			if (ImGui::MenuItem("Shader Graph"))
			{
				AddWindow<CustomShaderWindow>();
			}

			ImGui::EndMenu();
		}

		//auto& factory = SliceEngine::Core::GetInstance()->mFactory;

		if (ImGui::BeginMenu("Debug"))
		{
			if (ImGui::BeginMenu("Hierachy"))
			{

				if(ImGui::MenuItem("Show Entity IDs"))
				{
					EditorUtilities::Hierarchy_ToggleEntityID(registry);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("GameObject"))
		{
			EditorUtilities::MenuList_CreateGameObjects(registry.GetManager<HistoryManager>("History"), entt::null);
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
		auto* cwindow = SliceEngine::Core::GetInstance()->GetWindow();
		auto scene = SliceEngine::Core::GetInstance()->GetSceneSystem();

		int xPos{}, yPos{}, width{}, height{};
		glfwGetWindowPos(cwindow, &xPos, &yPos);
		glfwGetWindowSize(cwindow, &width, &height);

		ImGui::BeginViewportSideBar("PlayBar", ImGui::GetMainViewport(), ImGuiDir_Up, 50.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking);

        ImGuiIO& io = ImGui::GetIO();
		auto inputs = SliceEngine::Core::GetInstance()->GetInputSystem();
		inputs->SetImGuiCapture(io.WantCaptureKeyboard, io.WantCaptureMouse);

		if(!isPlaying)
		{
			if (ImGui::Button("Play", ImVec2{ 60, 35 }))
			{
				isPlaying = !isPlaying;

				if (isPlaying) // if its play, enable game input
				{
					EventManager::GetInstance()->Publish<OnPlayEvent>();
					ClearSelectionEvent clearedEvent;
					clearedEvent.suppressHistory = true;
					EventManager::GetInstance()->Publish<ClearSelectionEvent>(clearedEvent);
				}
							
			}
		}
		else
		{
			if(ImGui::Button("Stop", ImVec2{ 60, 35 }))
			{
				isPlaying = !isPlaying;
				if (!isPlaying) // if its play, enable game input
				{
					isPaused = false;
					scene->Stop();
					registry.GetManager<ProfilerManager>("Profiler")->mClearStatistics = true;
					ClearSelectionEvent clearedEvent;
					clearedEvent.suppressHistory = true;
					EventManager::GetInstance()->Publish<ClearSelectionEvent>(clearedEvent);
					EventManager::GetInstance()->Publish<OnStopEvent>();
				}
			}
		}

		ImGui::SameLine();
		if (!isPaused)
		{
			if (ImGui::Button("Pause", ImVec2{ 60, 35 }))
			{
				isPaused = !isPaused;
				registry.GetManager<ProfilerManager>("Profiler")->mClearStatistics = true;
				if (isPaused)
				{
					if (SliceEngine::Core::GetInstance()->GetSceneSystem()->mCurrentState == SliceEngine::PLAY_SCENE)
					{
						scene->Pause();

					}
				}
			}
		}
		else
		{
			if (ImGui::Button("Unpause", ImVec2{ 60, 35 }))
			{
				isPaused = !isPaused;

				registry.GetManager<ProfilerManager>("Profiler")->mClearStatistics = true;
				if (!isPaused)
				{
					if (SliceEngine::Core::GetInstance()->GetSceneSystem()->mCurrentState == SliceEngine::PAUSE_SCENE)
					{
						scene->Play();

					}
					
				}
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Action Mapping", ImVec2{ 60, 35 }))
		{ 
			if (!isPaused)
			{
				ImGui::OpenPopup("action_map_popup");
			}
		}

		//ImGui::SameLine();
		//if (ImGui::Button("Bind", ImVec2{ 60, 35 }))
		//{
		//	ImGui::OpenPopup("host_req");
		//}
		//ImGui::SameLine();
		//if (ImGui::Button("Connect", ImVec2{ 60, 35 }))
		//{
		//	ImGui::OpenPopup("connect_req");
		//}

		ImGui::BeginDisabled(isPlaying);

		ImGui::SameLine();
		if (ImGui::Button("Reload Scripts",ImVec2{0,35}))
		{
			if (SliceEngine::gScriptSystem)
			{
				SliceEngine::gScriptSystem->ReloadAssembly();
			}
		}

		ImGui::EndDisabled();

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
		
		if (ImGui::BeginPopup("action_map_popup"))
		{
			//using namespace SliceEngine; // this allows us to access slicenegine classes without prefixing
			//auto inputSys = SliceEngine::Core::GetInstance()->GetInputSystem();
			auto& AM = SliceEngine::GetActionMappingSystem();
			auto* core = SliceEngine::Core::GetInstance();
			auto* window = core->GetWindow();

			// saving to json path
			static std::string kJsonPath = "../SliceEditor/Assets/InputMappings.json";

			// save/load buttons
			if (ImGui::Button("Save All")) 
			{
				AM.SaveToJson(kJsonPath);
			}
			ImGui::SameLine();
			if (ImGui::Button("Load All")) 
			{
				AM.LoadFromJson(kJsonPath);
			}

			// create new base[?] map/action
			static char newMap[64] = "Gameplay";
			static char newAction[64] = "Jump";
			static int newType = 0; // 0 = button, 1 = 2D value
			static float dirX = 0.0f, dirY = 0.0f; // for 2D value but i don't think im gg be using this just yet

			ImGui::TextUnformatted("Create/Add");
			ImGui::Separator();
			ImGui::InputText("Action Map Name", newMap, IM_ARRAYSIZE(newMap));
			ImGui::InputText("Action Name", newAction, IM_ARRAYSIZE(newAction));
			ImGui::RadioButton("Button", &newType, 0); ImGui::SameLine();
			ImGui::RadioButton("1D Value", &newType, 2); ImGui::SameLine();
			ImGui::RadioButton("2D Value", &newType, 1);

			// for 2D value, input direction x and y
			if (newType == 1)
			{
				ImGui::InputFloat("Direction X", &dirX);
				ImGui::InputFloat("Direction Y", &dirY);
			}
			// for 1D value, input direction x only
			else if (newType == 2)
			{
				ImGui::InputFloat("Direction X", &dirX);
			}

			// creating a new action map
			if (ImGui::Button("Create Map"))
			{
				AM.CreateMap(newMap);
				AM.enableMap(newMap, true); // enable map upon creation
			}

			ImGui::SameLine();
			if (ImGui::Button("Add Action"))
			{
				AM.CreateMap(newMap); // ensure map exists but idk if this is necessary cus am i creating 2 maps?
				if (newType == 0) // button
				{
					AM.AddButton(newMap, newAction);
					AM.SaveToJson(kJsonPath);
				}
				else if (newType == 1) // 2D value
				{
					AM.AddValue2D(newMap, newAction);
					AM.SaveToJson(kJsonPath);
				}
				else if (newType == 2) // 1D value
				{
					AM.AddValue1D(newMap, newAction);
					AM.SaveToJson(kJsonPath);
				}
			}

			ImGui::Dummy({ 0,8 }); // spacing
			ImGui::Separator();
			ImGui::TextUnformatted("Current Action Maps");

			// list all action maps and their actions + keybinds
			for (auto& thisMap : AM.GetActionMaps())
			{
				auto& mapName = thisMap.first;
				auto& map = thisMap.second;

				// create header with enable toggle
				bool enabled = map.enabled;
				if (ImGui::CollapsingHeader(mapName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					// enable or disable
					if (ImGui::Checkbox(("Enabled##" + mapName).c_str(), &enabled))
					{
						AM.enableMap(mapName, enabled); // enable or disable map based on checkbox
					}
					// deelte whote map
					ImGui::SameLine();
					if (ImGui::SmallButton(("Delete Map##" + mapName).c_str())) 
					{
						AM.ClearMap(mapName);
						AM.SaveToJson(kJsonPath);
						// map is gone; skip drawing its currently invalid table this frame
						ImGui::Dummy({ 0,4 });
						continue;
					}
					// list all actions in this map by creating a table of of the actions
					if (ImGui::BeginTable(("table" + mapName).c_str(), 5, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
					{
						ImGui::TableSetupColumn("Action");
						ImGui::TableSetupColumn("Type");
						ImGui::TableSetupColumn("Bindings");
						ImGui::TableSetupColumn("Bind...");
						ImGui::TableSetupColumn("Edit");
						ImGui::TableHeadersRow();

						// idk how this fixes things tbh but it does
						// persistent per-row capture state [which rows are currently capturing]
						// use unordered_map to map action names to bools
						static std::unordered_map<std::string, bool> sCapturing;

						// loop through all actions in this map and list them
						for (size_t i{}; i < map.definitions.size(); ++i)
						{
							const auto& definition = map.definitions[i];
							ImGui::TableNextRow();

							// action name
							ImGui::TableSetColumnIndex(0);
							ImGui::TextUnformatted(definition.name.c_str());

							// action type
							ImGui::TableSetColumnIndex(1);
							if (definition.type == SliceEngine::ActionType::Button)
							{
								ImGui::TextUnformatted("Button");
							}
							else if (definition.type == SliceEngine::ActionType::Value2D)
							{
								ImGui::TextUnformatted("2D Value");
							}
							// for 1d values
							else if (definition.type == SliceEngine::ActionType::Value1D)
							{
								ImGui::TextUnformatted("1D Value");
							}

							// current bindings
							ImGui::TableSetColumnIndex(2);
							{
								if (definition.bindings.empty())
								{
									ImGui::TextDisabled("No Bindings");
								}
								else
								{
									// show key names using the inputsystem's keycode to name function
									//for (size_t j{}; j < definition.bindings.size(); ++j)
									//{
									for (size_t bi = 0; bi < definition.bindings.size(); ++bi)
									{
										const auto& b = definition.bindings[bi];
										const char* label = SliceEngine::InputSystem::KeyNameFallback(b.keyCode); // convert keycode to name
										if (definition.type == SliceEngine::ActionType::Button)
										{
											ImGui::Text("%s", label);
										}
										else
										{
											ImGui::Text("%s  (%.0f, %.0f)", label, b.x, b.y);
										}
									}
									//}
								}
							}

							// bind new key/UI
							ImGui::TableSetColumnIndex(3);

							// create unique key for this aciton's capture state
							const std::string capKey = mapName + " : " + definition.name;
							bool& captureInput = sCapturing[capKey]; // check if we're capturing input for this action

							ImGui::PushID((int)i); // push id for button
							if (!captureInput)
							{
								if (ImGui::Button("Bind"))
								{
									captureInput = true;
								}
							}
							else
							{
								ImGui::TextDisabled("Press a key...");
								// capture next key press
								int key = DetectGlfwKeyPress(window);
								if (key != -1)
								{
									if (definition.type == SliceEngine::ActionType::Button)
									{
										AM.BindButton(mapName, definition.name, key);
										// here is where we call the savetofile function to file
										AM.SaveToJson(kJsonPath);
									}
									else if (definition.type == SliceEngine::ActionType::Value2D)
									{
										AM.Bind2D(mapName, definition.name, key, dirX, dirY);
										AM.SaveToJson(kJsonPath);
									}
									else if (definition.type == SliceEngine::ActionType::Value1D)
									{
										AM.Bind1D(mapName, definition.name, key, dirX);
										AM.SaveToJson(kJsonPath);
									}
									captureInput = false; // stop capturing after key press
								}
								// cancel button
								ImGui::SameLine();
								if (ImGui::Button("Cancel"))
								{
									captureInput = false;
								}
							}
							ImGui::PopID();

							// had no idea how to do this portion so thanks gpt
							// edit bindings: clear all or delete action
							ImGui::TableSetColumnIndex(4);
							ImGui::PushID((int)i + 100000);  // separate ID space for edit buttons

							bool clearBinds = ImGui::SmallButton("Clear Binds");
							ImGui::SameLine();
							bool deleteAction = ImGui::SmallButton("Delete Action");

							ImGui::PopID();

							if (clearBinds) 
							{
								AM.ClearBindings(mapName, definition.name);
								AM.SaveToJson(kJsonPath);
							}

							if (deleteAction) 
							{
								AM.ClearAction(mapName, definition.name);
								AM.SaveToJson(kJsonPath);
								break;
							}

						}
						ImGui::EndTable();
					}
				}
			}
			// ---------------- 1 ----------------
			// write text header [Action Maps]
			//std::string display = "Current Action Maps:";
			//ImGui::Text(display.c_str());
			// call action map lists and pull their names
			// open action map list, call maps
			// for each action map, create a collapsing header with its name
			// ---------------- 2 ----------------
			// user clicks on collapsing header
			// open action map
			// call that action map and pull all actions
			// list all actions and then have a square/rectangle beside it to input keybind
			// ---------------- 3 ----------------
			// user keys in new keybind
			// action: "jump" -> keybind: "spacebar"
			// call parsing function to sort through glfw keycodes and match "spacebar" to its keycode
			// call BindButton or Bind2D with action name and keycode
			// bind the new keycode to that action
			// ---------------- 4 ----------------
			// create button at bottom of header to add either new action map or new action to existing map
			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void WindowManager::DrawPreferenceSettings()
	{
	}

	void WindowManager::DrawProjectSettings()
	{
		//if (!projectSettingsPopupOpen)
		//	return;

		//bool isOpen;
		//if (ImGui::Begin("project_settings_window", &isOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize ))
		//{
		//	auto gSettings = SliceEngine::Core::GetInstance()->GetProjectSettingsService();
		//	auto& s = gSettings->Edit(); // we�ll set dirty only if something changes

		//	bool changed = false;
		//	if (ImGui::InputText("Product Name", &s.productName)) { changed = true; }
		//	int w = s.width, h = s.height;
		//	if (ImGui::InputInt("Width", &w)) { s.width = std::max(16, w); changed = true; }
		//	if (ImGui::InputInt("Height", &h)) { s.height = std::max(16, h); changed = true; }

		//	// Scenes list (very basic)
		//	for (size_t i = 0;i < s.scenes.size();++i) {
		//		ImGui::PushID((int)i);
		//		ImGui::InputText("Scene Path", &s.scenes[i]); // path string edit
		//		if (ImGui::SmallButton("Up") && i > 0) { std::swap(s.scenes[i], s.scenes[i - 1]); changed = true; }
		//		ImGui::SameLine();
		//		if (ImGui::SmallButton("Down") && i + 1 < s.scenes.size()) { std::swap(s.scenes[i], s.scenes[i + 1]); changed = true; }
		//		ImGui::SameLine();
		//		if (ImGui::SmallButton("X")) { s.scenes.erase(s.scenes.begin() + i); changed = true; ImGui::PopID(); break; }
		//		ImGui::PopID();
		//	}
		//	if (ImGui::Button("+ Add Scene")) { s.scenes.emplace_back("Assets/Scenes/New.scene"); changed = true; }

		//	// Startup scene combo
		//	if (!s.scenes.empty()) {
		//		int current = 0;
		//		for (int i = 0;i < (int)s.scenes.size();++i) if (s.scenes[i] == s.startupScene) current = i;
		//		if (ImGui::BeginCombo("Startup Scene", s.scenes[current].c_str())) {
		//			for (int i = 0;i < (int)s.scenes.size();++i) {
		//				bool sel = (i == current);
		//				if (ImGui::Selectable(s.scenes[i].c_str(), sel)) { s.startupScene = s.scenes[i]; changed = true; }
		//			}
		//			ImGui::EndCombo();
		//		}
		//	}

		//	// Save/Reload row
		//	if (ImGui::Button("Save")) gSettings->Save();
		//	ImGui::SameLine();
		//	if (ImGui::Button("Reload")) { gSettings->Load(); }

		//	// Set dirty timing + optional autosave
		//	if (changed) {
		//		// touching Edit() already marked dirty; reset the debounce timer by re-setting the change time
		//		// simplest: mark as dirty again; DebouncedAutosave accumulates time each frame
		//	}

		//	gSettings->DebouncedAutosave(1.0f/60.0f, /*delay*/0.75);

		//	// External change detection (prompt)
		//	if (gSettings->DetectExternalChange()) {
		//		ImGui::TextDisabled("ProjectSettings.json changed on disk.");
		//		ImGui::SameLine();
		//		if (ImGui::Button("Reload from Disk")) gSettings->Load();
		//	}

		//	ImGui::End();
		//}

		//projectSettingsPopupOpen = isOpen;

	}

	void WindowManager::DrawSaveSceneAsPopup()
	{
		if (!saveSceneAsPopup)
		{
			return;
		}

		static std::string sceneName = "";

		bool isOpen = true;
		if (ImGui::Begin("Save Scene as..", &isOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (ImGui::InputText("New Scene Name", &sceneName))
			{

			}

			if (ImGui::SmallButton("Save Scene"))
			{
				if (!sceneName.empty())
				{
					auto sceneSystem = SliceEngine::Core::GetInstance()->GetSceneSystem();
					
					std::filesystem::path newScenePath = "Assets/Default/" + sceneName + ".scene";
					std::filesystem::path currentPath = sceneSystem->GetCurrentScenePath();

					sceneSystem->OnSceneSave(newScenePath);

					SliceEngine::gScriptSystem->OnEnd();

					sceneSystem->LoadSceneIntoQueue(newScenePath);

					saveSceneAsPopup = false;					
				}
			}

			ImGui::End();
		}

		if (!isOpen)
		{
			saveSceneAsPopup = false;
		}
	}

	void WindowManager::DrawNewScenePopup()
	{
		if (!newScenePopupOpen)
		{
			return;
		}

		static std::string sceneName = "NewScene";

		bool isOpen = true;
		if (ImGui::Begin("new scene window", &isOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (ImGui::InputText("New Scene Name", &sceneName))
			{

			}

			if (ImGui::SmallButton("Create Scene"))
			{
				if (!sceneName.empty())
				{
					EditorUtilities::Scene_CreateDefault(sceneName);
					//SliceEngine::Core::GetInstance()->GetSceneSystem()->SaveCurrentScene();
					SliceEngine::Core::GetInstance()->GetSceneSystem()->LoadSceneIntoQueue("Assets/Default/" + sceneName + ".scene");
					newScenePopupOpen = false;
					sceneName = "NewScene";
				}
			}

			ImGui::End();
		}

		if (!isOpen)
		{
			newScenePopupOpen = false;
		}
	}

	void WindowManager::DrawSavePopupModal()
	{
		if (saveScenePopupOpen)
		{
			ImGui::OpenPopup("SaveScenePopup");
			saveScenePopupOpen = false;
		}


		if (ImGui::BeginPopupModal("SaveScenePopup"))
		{
			ImGui::Text("Saving...");

			if (saveScenePopupClose)
			{
				ImGui::CloseCurrentPopup();
				saveScenePopupClose = false;
			}

			ImGui::EndPopup();
		}
	}

	void WindowManager::QuitGameEvent(OnGameStopEvent e)
	{
		isPlaying = false;
		isPaused = false;
		SliceEngine::Core::GetInstance()->GetSceneSystem()->Stop();
	}

	void WindowManager::MenuToggleBit(const char* label, unsigned char& mask, unsigned char bit)
	{
		bool checked = (mask & bit) != 0;

		if (ImGui::MenuItem(label, nullptr, checked))
		{
			if (checked)
				mask &= ~bit;
			else
				mask |= bit;
		}
	}

	//void WindowManager::SetTheme_Microsoft()
	//{
	//}

}
