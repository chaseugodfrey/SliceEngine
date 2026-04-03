#include <pch.h>
#include "EditorUtilities.h"
#include <History/HistoryManager.h>
#include <Selection/SelectionManager.h>
#include <Session/SessionManager.h>
#include <ContentBrowser/ContentBrowserManager.h>
#include <Systems/SceneSystem.h>
#include <Systems/PrefabSystem.h>


namespace SliceEditor
{
	namespace EditorUtilities
	{
		void Scene_CreateDefault(std::string filename)
		{
			std::filesystem::path newScenePath = "Assets/Default/" + filename + ".scene";

			auto core = SliceEngine::Core::GetInstance();
			auto sceneSystem = core->GetSceneSystem();

			sceneSystem->UnloadCurrentScene();
			
			sceneSystem->SaveScene(newScenePath);
		}

		SliceEngine::GameObject GameObject_CreateEmpty(entt::entity parent, HistoryManager* history, bool isPrefabInspected)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (isPrefabInspected)
			{
				SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().AddToPrefab(go.GetEntity(), parent);
			}

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateBox(entt::entity parent, HistoryManager* history, bool isPrefabInspected)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Box();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (isPrefabInspected)
			{
				SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().AddToPrefab(go.GetEntity(), parent);
			}

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateSphere(entt::entity parent, HistoryManager* history, bool isPrefabInspected)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Sphere();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (isPrefabInspected)
			{
				SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().AddToPrefab(go.GetEntity(), parent);
			}

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateCapsule(entt::entity parent, HistoryManager* history, bool isPrefabInspected)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Capsule();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (isPrefabInspected)
			{
				SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().AddToPrefab(go.GetEntity(), parent);
			}

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateCylinder(entt::entity parent, HistoryManager* history, bool isPrefabInspected)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Cylinder();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (isPrefabInspected)
			{
				SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().AddToPrefab(go.GetEntity(), parent);
			}

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateCam(entt::entity parent, HistoryManager* history, bool isPrefabInspected)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Cam();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (isPrefabInspected)
			{
				SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().AddToPrefab(go.GetEntity(), parent);
			}

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateLight(entt::entity parent, HistoryManager* history, bool isPrefabInspected)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Light();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (isPrefabInspected)
			{
				SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().AddToPrefab(go.GetEntity(), parent);
			}

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateModel(SliceEngine::GUID guid, SliceEngine::GUID skeleGUID, SliceEngine::GUID animGUID, entt::entity parent, HistoryManager* history, bool isPrefabInspected)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Model(skeleGUID, animGUID, guid);

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}
			if (isPrefabInspected)
			{
				SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().AddToPrefab(go.GetEntity(), parent);
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateCanvas(entt::entity parent, HistoryManager* history, bool isPrefabInspected)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Canvas();

			//no parent for now because only overlay
			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			if (isPrefabInspected)
			{
				SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().AddToPrefab(go.GetEntity(), parent);
			}

			return go;
		}
		SliceEngine::GameObject GameObject_CreateImage(entt::entity parent, HistoryManager* history, bool isPrefabInspected)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Image();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			if (isPrefabInspected)
			{
				SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().AddToPrefab(go.GetEntity(), parent);
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateText(entt::entity parent, HistoryManager* history, bool isPrefabInspected)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Text();

			//no parent for now because only overlay
			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			if (isPrefabInspected)
			{
				SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().AddToPrefab(go.GetEntity(), parent);
			}
			return go;
		}

		SliceEngine::GameObject GameObject_CreateButton(entt::entity parent, HistoryManager* history, bool isPrefabInspected)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Button();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			if (isPrefabInspected)
			{
				SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().AddToPrefab(go.GetEntity(), parent);
			}
			return go;
		}

		SliceEngine::GameObject GameObject_CreateSlider(entt::entity parent, HistoryManager* history, bool isPrefabInspected)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Slider();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			if (isPrefabInspected)
			{
				SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().AddToPrefab(go.GetEntity(), parent);
			}
			return go;
		}

		SliceEngine::GameObject GameObject_CreatePrefab(SliceEngine::GUID guid, entt::entity parent, HistoryManager* history)
		{
			return SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().CreatePrefab(guid);
		}

		void GameObject_Unprefab(entt::entity entity)
		{
			SliceEngine::GameObject GO = SliceEngine::FactoryInstance.GetGOByEntity(entity);
			SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().UpdatePrefabComponent(entity, GO.GetComponent<SliceEngine::Prefab>().prefabGUID, true);
		}

		void GameObject_Clone(entt::entity entity)
		{
			SliceEngine::GameObject go = SliceEngine::FactoryInstance.GetGOByEntity(entity);
			SliceEngine::FactoryInstance.CloneGO(go);

		}

		void GameObject_Destroy(entt::entity target, HistoryManager* history)
		{
			EventManager::GetInstance()->Publish<ClearSelectionEvent>();
			SliceEngine::FactoryInstance.Destroy(target);

			// leaving undo & redo of deletion to future implementation 
			// since undoing deletion requires storing/saving data and retrieving it which is more complex
			// for now, deleting is permanent

			//if (!suppressHistory)
			//	EventManager::GetInstance()->Publish<AddCommandEvent>(std::make_unique<CreateGameObjectEvent>(target));
		}

		void GameObject_Parent(entt::entity child, entt::entity parent, HistoryManager* history)
		{
			auto& scene_graph = SliceEngine::FactoryInstance.GetGOByEntity(child).GetComponent<SliceEngine::SceneGraph>();
			auto old_parent = scene_graph.neighbours[SliceEngine::SceneGraph::UP];

			if (SliceEngine::FactoryInstance.SetParent(child, parent))
			{
				if (history)
				{
					history->AddCommand(std::make_unique<ParentEntityCommand>(child, old_parent, parent));
				}
			}
		}

		void GameObject_Unparent(entt::entity child, HistoryManager* history)
		{
			auto& scene_graph = SliceEngine::FactoryInstance.GetGOByEntity(child).GetComponent<SliceEngine::SceneGraph>();
			auto old_parent = scene_graph.neighbours[SliceEngine::SceneGraph::UP];

			if (SliceEngine::FactoryInstance.Unparent(child))
			{
				if (history)
				{
					auto new_parent = scene_graph.neighbours[SliceEngine::SceneGraph::UP];

					history->AddCommand(std::make_unique<ParentEntityCommand>(child, old_parent, new_parent));
				}

			}
		}

		void GameObject_SetSibling(entt::entity target, entt::entity destination, HistoryManager* history)
		{
			auto& factory = SliceEngine::FactoryInstance;

			if (destination == target)
				return;

			else
			{
				auto& destSceneGraph = factory.GetGOByEntity(destination).GetComponent<SliceEngine::SceneGraph>();
				factory.SetNewSceneGraphLocation(target, destination, destSceneGraph.neighbours[SliceEngine::SceneGraph::LEFT], destSceneGraph.neighbours[SliceEngine::SceneGraph::UP]);

				if (history)
				{

				}
			}
		}

		void GameObject_RemoveComponent(entt::entity entity, const std::string& componentName, HistoryManager* history = nullptr)
		{

		}

		void Scene_Load(const std::filesystem::path& path, SelectionManager& selectionManager, SessionManager& sessionManager)
		{
			SliceEngine::Core::GetInstance()->GetSceneSystem()->LoadSceneIntoQueue(path);
			if(sessionManager.IsPrefabInspected())
			{
				PrefabInspectedEvent event;
				event.prefabBeingInspected = false;
				EventManager::GetInstance()->Publish<PrefabInspectedEvent>(event);
			}
			selectionManager.ClearSelection(true);
		}

		void Scene_Stop(SelectionManager& selectionManager)
		{
			selectionManager.ClearSelection(true);
		}

		void Scene_Save()
		{
			if (SliceEngine::Core::GetInstance()->GetSceneSystem()->mCurrentState == SliceEngine::PAUSE_SCENE || SliceEngine::Core::GetInstance()->GetSceneSystem()->mCurrentState == SliceEngine::DEFAULT)
			{
				std::filesystem::path currentScenePath = SliceEngine::Core::GetInstance()->GetSceneSystem()->GetCurrentScenePath();

				//Check if the current scene set is already a temp scene
				if (currentScenePath.extension() == ".temp")
				{
					std::filesystem::path originalScenePath = currentScenePath;
					originalScenePath.replace_extension(".scene");

					//Set to scene path and remove temp file
					if (std::filesystem::exists(originalScenePath))
					{
						SliceEngine::Core::GetInstance()->GetSceneSystem()->LoadSceneIntoQueue(originalScenePath);
						std::filesystem::remove(currentScenePath);
					}
				}

				SliceEngine::Core::GetInstance()->GetSceneSystem()->SaveCurrentScene();
			}
		}

		void Scene_CleanTempFiles(Registry& registry)
		{
			registry.GetAssetManager().CleanUpSceneTemp();
		}

		void ContentBrowser_Refresh(ContentBrowserManager& contentBrowserManager)
		{
			contentBrowserManager.RebuildDirectory();
		}

		void Hierarchy_ToggleEntityID(Registry& registry)
		{
			auto mSession = registry.GetManager<SessionManager>("Session");

			mSession->ToggleHierarchyEntityIDs();
		}

		void MenuList_CreateFiles(Registry& reg, std::filesystem::path descPath)
		{
			if (ImGui::BeginMenu("Create"))
			{
				/*if (ImGui::MenuItem("Folder"))
				{

				}*/

				if (ImGui::MenuItem("Material"))
				{
					CreateFile_MaterialFile(reg, descPath);
				}

				if (ImGui::MenuItem("Animation Clip"))
				{

				}

				if (ImGui::MenuItem("Animator Controller"))
				{

				}

				if (ImGui::MenuItem("Shader"))
				{
					CreateFile_ShaderFile(reg, descPath);
				}

				ImGui::EndMenu();
			}
		}

		void MenuList_CreateGameObjects(HistoryManager* history, entt::entity parent, bool isPrefabInspected)
		{
			if (ImGui::MenuItem("Empty"))
			{
				EditorUtilities::GameObject_CreateEmpty(parent, history, isPrefabInspected);
			}

			if (ImGui::MenuItem("Camera"))
			{
				EditorUtilities::GameObject_CreateCam(parent, history);
			}

			if (ImGui::MenuItem("Light"))
			{
				EditorUtilities::GameObject_CreateLight(parent, history, isPrefabInspected);
			}

			if (ImGui::BeginMenu("3D Object"))
			{
				if (ImGui::MenuItem("Box"))
				{
					EditorUtilities::GameObject_CreateBox(parent, history, isPrefabInspected);
				}

				if (ImGui::MenuItem("Sphere"))
				{
					EditorUtilities::GameObject_CreateSphere(parent, history, isPrefabInspected);
				}

				if (ImGui::MenuItem("Capsule"))
				{
					EditorUtilities::GameObject_CreateCapsule(parent, history, isPrefabInspected);
				}

				if (ImGui::MenuItem("Cylinder"))
				{
					EditorUtilities::GameObject_CreateCylinder(parent, history, isPrefabInspected);
				}

				if (ImGui::MenuItem("Quad"))
				{
					EditorUtilities::GameObject_CreateBox(parent, history, isPrefabInspected);
				}

				if (ImGui::MenuItem("Plane"))
				{
					EditorUtilities::GameObject_CreateBox(parent, history, isPrefabInspected);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("UI"))
			{
				if (ImGui::MenuItem("Canvas"))
				{
					EditorUtilities::GameObject_CreateCanvas(parent, history, isPrefabInspected);
				}

				//Hidden Till it Works
				//if (ImGui::MenuItem("Text"))
				//{
				//	//next tri
				//}

				if (ImGui::MenuItem("Image"))
				{
					EditorUtilities::GameObject_CreateImage(parent, history, isPrefabInspected);
				}

				if (ImGui::MenuItem("Button"))
				{
					EditorUtilities::GameObject_CreateButton(parent, history, isPrefabInspected);
				}

				if (ImGui::MenuItem("Slider"))
				{
					EditorUtilities::GameObject_CreateSlider(parent, history, isPrefabInspected);
				}

				if (ImGui::MenuItem("Text"))
				{
					EditorUtilities::GameObject_CreateText(parent, history, isPrefabInspected);
				}

				ImGui::EndMenu();
			}
		}

		void CreateFile_MaterialFile(Registry& reg, std::filesystem::path descPath)
		{
			reg.GetAssetManager().CreateDefaultAsset(descPath, AssetType::Material);
		}

		void CreateFile_ShaderFile(Registry& reg, std::filesystem::path descPath)
		{
			reg.GetAssetManager().CreateDefaultAsset(descPath, AssetType::CustomShader);
		}

#pragma region Assets

#pragma endregion

#pragma region Preferences

#pragma region Themes
		void SetTheme_Dark()
		{
			ImGuiStyle& style = ImGui::GetStyle();

			style.Alpha = 1.0f;
			style.DisabledAlpha = 0.6f;
			style.WindowPadding = ImVec2(8.0f, 8.0f);
			style.WindowRounding = 0.0f;
			style.WindowBorderSize = 1.0f;
			style.WindowMinSize = ImVec2(32.0f, 32.0f);
			style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
			style.WindowMenuButtonPosition = ImGuiDir_Left;
			style.ChildRounding = 0.0f;
			style.ChildBorderSize = 1.0f;
			style.PopupRounding = 0.0f;
			style.PopupBorderSize = 1.0f;
			style.FramePadding = ImVec2(4.0f, 3.0f);
			style.FrameRounding = 0.0f;
			style.FrameBorderSize = 0.0f;
			style.ItemSpacing = ImVec2(8.0f, 4.0f);
			style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
			style.CellPadding = ImVec2(4.0f, 2.0f);
			style.IndentSpacing = 21.0f;
			style.ColumnsMinSpacing = 6.0f;
			style.ScrollbarSize = 14.0f;
			style.ScrollbarRounding = 9.0f;
			style.GrabMinSize = 10.0f;
			style.GrabRounding = 0.0f;
			style.TabRounding = 4.0f;
			style.TabBorderSize = 0.0f;
			style.ColorButtonPosition = ImGuiDir_Right;
			style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
			style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 1.0f);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05882353f, 0.05882353f, 0.05882353f, 0.94f);
			style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(0.078431375f, 0.078431375f, 0.078431375f, 0.94f);
			style.Colors[ImGuiCol_Border] = ImVec4(0.42745098f, 0.42745098f, 0.49803922f, 0.5f);
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15686275f, 0.28627452f, 0.47843137f, 0.54f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.4f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.67f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.039215688f, 0.039215688f, 0.039215688f, 1.0f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15686275f, 0.28627452f, 0.47843137f, 1.0f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.51f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.13725491f, 0.13725491f, 0.13725491f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.019607844f, 0.019607844f, 0.019607844f, 0.53f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30980393f, 0.30980393f, 0.30980393f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40784314f, 0.40784314f, 0.40784314f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50980395f, 0.50980395f, 0.50980395f, 1.0f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.23921569f, 0.5176471f, 0.8784314f, 1.0f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.4f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.05882353f, 0.5294118f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.31f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_Separator] = ImVec4(0.42745098f, 0.42745098f, 0.49803922f, 0.5f);
			style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.09803922f, 0.4f, 0.7490196f, 0.78f);
			style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803922f, 0.4f, 0.7490196f, 1.0f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.2f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.67f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.95f);
			style.Colors[ImGuiCol_Tab] = ImVec4(0.1764706f, 0.34901962f, 0.5764706f, 0.862f);
			style.Colors[ImGuiCol_TabHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
			style.Colors[ImGuiCol_TabActive] = ImVec4(0.19607843f, 0.40784314f, 0.6784314f, 1.0f);
			style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667f, 0.101960786f, 0.14509805f, 0.9724f);
			style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13333334f, 0.25882354f, 0.42352942f, 1.0f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(0.60784316f, 0.60784316f, 0.60784316f, 1.0f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.42745098f, 0.34901962f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392f, 0.69803923f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882353f, 0.1882353f, 0.2f, 1.0f);
			style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
			style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.22745098f, 0.22745098f, 0.24705882f, 1.0f);
			style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.35f);
			style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
			style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
			style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
			style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
		}

		void SetTheme_Microsoft()
		{
			// Microsoft style by usernameiwantedwasalreadytaken from ImThemes
			ImGuiStyle& style = ImGui::GetStyle();

			style.Alpha = 1.0f;
			style.DisabledAlpha = 0.6f;
			style.WindowPadding = ImVec2(4.0f, 6.0f);
			style.WindowRounding = 0.0f;
			style.WindowBorderSize = 0.0f;
			style.WindowMinSize = ImVec2(32.0f, 32.0f);
			style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
			style.WindowMenuButtonPosition = ImGuiDir_Left;
			style.ChildRounding = 0.0f;
			style.ChildBorderSize = 1.0f;
			style.PopupRounding = 0.0f;
			style.PopupBorderSize = 1.0f;
			style.FramePadding = ImVec2(8.0f, 6.0f);
			style.FrameRounding = 0.0f;
			style.FrameBorderSize = 1.0f;
			style.ItemSpacing = ImVec2(8.0f, 6.0f);
			style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
			style.CellPadding = ImVec2(4.0f, 2.0f);
			style.IndentSpacing = 20.0f;
			style.ColumnsMinSpacing = 6.0f;
			style.ScrollbarSize = 20.0f;
			style.ScrollbarRounding = 0.0f;
			style.GrabMinSize = 5.0f;
			style.GrabRounding = 0.0f;
			style.TabRounding = 4.0f;
			style.TabBorderSize = 0.0f;
			style.ColorButtonPosition = ImGuiDir_Right;
			style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
			style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

			style.Colors[ImGuiCol_Text] = ImVec4(0.09803922f, 0.09803922f, 0.09803922f, 1.0f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 1.0f);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.9490196f, 0.9490196f, 0.9490196f, 1.0f);
			style.Colors[ImGuiCol_ChildBg] = ImVec4(0.9490196f, 0.9490196f, 0.9490196f, 1.0f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_Border] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.0f, 0.46666667f, 0.8392157f, 0.2f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.46666667f, 0.8392157f, 1.0f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.039215688f, 0.039215688f, 0.039215688f, 1.0f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15686275f, 0.28627452f, 0.47843137f, 1.0f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.51f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.85882354f, 0.85882354f, 0.85882354f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.85882354f, 0.85882354f, 0.85882354f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.6862745f, 0.6862745f, 0.6862745f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.2f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(0.09803922f, 0.09803922f, 0.09803922f, 1.0f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.6862745f, 0.6862745f, 0.6862745f, 1.0f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.85882354f, 0.85882354f, 0.85882354f, 1.0f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.46666667f, 0.8392157f, 0.2f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.46666667f, 0.8392157f, 1.0f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.85882354f, 0.85882354f, 0.85882354f, 1.0f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.46666667f, 0.8392157f, 0.2f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.46666667f, 0.8392157f, 1.0f);
			style.Colors[ImGuiCol_Separator] = ImVec4(0.42745098f, 0.42745098f, 0.49803922f, 0.5f);
			style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.09803922f, 0.4f, 0.7490196f, 0.78f);
			style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803922f, 0.4f, 0.7490196f, 1.0f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.2f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.67f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.95f);
			style.Colors[ImGuiCol_Tab] = ImVec4(0.1764706f, 0.34901962f, 0.5764706f, 0.862f);
			style.Colors[ImGuiCol_TabHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
			style.Colors[ImGuiCol_TabActive] = ImVec4(0.19607843f, 0.40784314f, 0.6784314f, 1.0f);
			style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667f, 0.101960786f, 0.14509805f, 0.9724f);
			style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13333334f, 0.25882354f, 0.42352942f, 1.0f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(0.60784316f, 0.60784316f, 0.60784316f, 1.0f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.42745098f, 0.34901962f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392f, 0.69803923f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882353f, 0.1882353f, 0.2f, 1.0f);
			style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
			style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.22745098f, 0.22745098f, 0.24705882f, 1.0f);
			style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.35f);
			style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
			style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
			style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
			style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
		}

		void SetTheme_Light()
		{
			// Light style by dougbinks from ImThemes
			ImGuiStyle& style = ImGui::GetStyle();

			style.Alpha = 1.0f;
			style.DisabledAlpha = 0.6f;
			style.WindowPadding = ImVec2(8.0f, 8.0f);
			style.WindowRounding = 0.0f;
			style.WindowBorderSize = 1.0f;
			style.WindowMinSize = ImVec2(32.0f, 32.0f);
			style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
			style.WindowMenuButtonPosition = ImGuiDir_Left;
			style.ChildRounding = 0.0f;
			style.ChildBorderSize = 1.0f;
			style.PopupRounding = 0.0f;
			style.PopupBorderSize = 1.0f;
			style.FramePadding = ImVec2(4.0f, 3.0f);
			style.FrameRounding = 0.0f;
			style.FrameBorderSize = 0.0f;
			style.ItemSpacing = ImVec2(8.0f, 4.0f);
			style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
			style.CellPadding = ImVec2(4.0f, 2.0f);
			style.IndentSpacing = 21.0f;
			style.ColumnsMinSpacing = 6.0f;
			style.ScrollbarSize = 14.0f;
			style.ScrollbarRounding = 9.0f;
			style.GrabMinSize = 10.0f;
			style.GrabRounding = 0.0f;
			style.TabRounding = 4.0f;
			style.TabBorderSize = 0.0f;
			style.ColorButtonPosition = ImGuiDir_Right;
			style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
			style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

			style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.9372549f, 0.9372549f, 0.9372549f, 1.0f);
			style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.98f);
			style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.3f);
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.4f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.67f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.95686275f, 0.95686275f, 0.95686275f, 1.0f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.81960785f, 0.81960785f, 0.81960785f, 1.0f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 1.0f, 1.0f, 0.51f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.85882354f, 0.85882354f, 0.85882354f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.9764706f, 0.9764706f, 0.9764706f, 0.53f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.6862745f, 0.6862745f, 0.6862745f, 0.8f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4862745f, 0.4862745f, 0.4862745f, 0.8f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4862745f, 0.4862745f, 0.4862745f, 1.0f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.78f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45882353f, 0.5372549f, 0.8f, 0.6f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.4f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.05882353f, 0.5294118f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.31f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_Separator] = ImVec4(0.3882353f, 0.3882353f, 0.3882353f, 0.62f);
			style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.13725491f, 0.4392157f, 0.8f, 0.78f);
			style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.13725491f, 0.4392157f, 0.8f, 1.0f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.34901962f, 0.34901962f, 0.34901962f, 0.17f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.67f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.95f);
			style.Colors[ImGuiCol_Tab] = ImVec4(0.7607843f, 0.79607844f, 0.8352941f, 0.931f);
			style.Colors[ImGuiCol_TabHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
			style.Colors[ImGuiCol_TabActive] = ImVec4(0.5921569f, 0.7254902f, 0.88235295f, 1.0f);
			style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.91764706f, 0.9254902f, 0.93333334f, 0.9862f);
			style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.7411765f, 0.81960785f, 0.9137255f, 1.0f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(0.3882353f, 0.3882353f, 0.3882353f, 1.0f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.42745098f, 0.34901962f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392f, 0.69803923f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.44705883f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.7764706f, 0.8666667f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.5686275f, 0.5686275f, 0.6392157f, 1.0f);
			style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.6784314f, 0.6784314f, 0.7372549f, 1.0f);
			style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.29803923f, 0.29803923f, 0.29803923f, 0.09f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.35f);
			style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.95f);
			style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
			style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.69803923f, 0.69803923f, 0.69803923f, 0.7f);
			style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.2f);
			style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.35f);
		}

		void SetTheme_Excellency()
		{
			// Excellency style by gonzaloivan121 from ImThemes
			ImGuiStyle& style = ImGui::GetStyle();

			style.Alpha = 1.0f;
			style.DisabledAlpha = 0.6f;
			style.WindowPadding = ImVec2(10.0f, 10.0f);
			style.WindowRounding = 0.0f;
			style.WindowBorderSize = 1.0f;
			style.WindowMinSize = ImVec2(32.0f, 32.0f);
			style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
			style.WindowMenuButtonPosition = ImGuiDir_None;
			style.ChildRounding = 6.0f;
			style.ChildBorderSize = 1.0f;
			style.PopupRounding = 6.0f;
			style.PopupBorderSize = 1.0f;
			style.FramePadding = ImVec2(8.0f, 6.0f);
			style.FrameRounding = 6.0f;
			style.FrameBorderSize = 1.0f;
			style.ItemSpacing = ImVec2(6.0f, 6.0f);
			style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
			style.CellPadding = ImVec2(4.0f, 2.0f);
			style.IndentSpacing = 11.0f;
			style.ColumnsMinSpacing = 6.0f;
			style.ScrollbarSize = 14.0f;
			style.ScrollbarRounding = 6.0f;
			style.GrabMinSize = 10.0f;
			style.GrabRounding = 6.0f;
			style.TabRounding = 6.0f;
			style.TabBorderSize = 1.0f;
			style.ColorButtonPosition = ImGuiDir_Right;
			style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
			style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5019608f, 0.5019608f, 0.5019608f, 1.0f);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08235294f, 0.08235294f, 0.08235294f, 1.0f);
			style.Colors[ImGuiCol_ChildBg] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 1.0f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(0.19607843f, 0.19607843f, 0.19607843f, 1.0f);
			style.Colors[ImGuiCol_Border] = ImVec4(0.101960786f, 0.101960786f, 0.101960786f, 1.0f);
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(0.05882353f, 0.05882353f, 0.05882353f, 1.0f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.09019608f, 0.09019608f, 0.09019608f, 1.0f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.05882353f, 0.05882353f, 0.05882353f, 1.0f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08235294f, 0.08235294f, 0.08235294f, 1.0f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08235294f, 0.08235294f, 0.08235294f, 1.0f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15294118f, 0.15294118f, 0.15294118f, 1.0f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.019607844f, 0.019607844f, 0.019607844f, 0.53f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30980393f, 0.30980393f, 0.30980393f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4117647f, 0.4117647f, 0.4117647f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50980395f, 0.50980395f, 0.50980395f, 1.0f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(0.7529412f, 0.7529412f, 0.7529412f, 1.0f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.50980395f, 0.50980395f, 0.50980395f, 0.7f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.65882355f, 0.65882355f, 0.65882355f, 1.0f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.21960784f, 0.21960784f, 0.21960784f, 0.784f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.27450982f, 0.27450982f, 0.27450982f, 1.0f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.21960784f, 0.21960784f, 0.21960784f, 0.588f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.18431373f, 0.18431373f, 0.18431373f, 1.0f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.18431373f, 0.18431373f, 0.18431373f, 1.0f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.18431373f, 0.18431373f, 0.18431373f, 1.0f);
			style.Colors[ImGuiCol_Separator] = ImVec4(0.101960786f, 0.101960786f, 0.101960786f, 1.0f);
			style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.15294118f, 0.7254902f, 0.9490196f, 0.588f);
			style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.15294118f, 0.7254902f, 0.9490196f, 1.0f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.9098039f, 0.9098039f, 0.9098039f, 0.25f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.8117647f, 0.8117647f, 0.8117647f, 0.67f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.45882353f, 0.45882353f, 0.45882353f, 0.95f);
			style.Colors[ImGuiCol_Tab] = ImVec4(0.08235294f, 0.08235294f, 0.08235294f, 1.0f);
			style.Colors[ImGuiCol_TabHovered] = ImVec4(1.0f, 0.88235295f, 0.5294118f, 0.118f);
			style.Colors[ImGuiCol_TabActive] = ImVec4(1.0f, 0.88235295f, 0.5294118f, 0.235f);
			style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08235294f, 0.08235294f, 0.08235294f, 1.0f);
			style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.0f, 0.88235295f, 0.5294118f, 0.118f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6117647f, 0.6117647f, 0.6117647f, 1.0f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.43137255f, 0.34901962f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.9019608f, 0.7019608f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18431373f, 0.18431373f, 0.18431373f, 1.0f);
			style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
			style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.101960786f, 0.101960786f, 0.101960786f, 1.0f);
			style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.15294118f, 0.7254902f, 0.9490196f, 0.35f);
			style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
			style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.15294118f, 0.7254902f, 0.9490196f, 0.8f);
			style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
			style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
			style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
		}

		void SetTheme_Rest()
		{
			// Rest style by AaronBeardless from ImThemes
			ImGuiStyle& style = ImGui::GetStyle();

			style.Alpha = 1.0f;
			style.DisabledAlpha = 0.5f;
			style.WindowPadding = ImVec2(13.0f, 10.0f);
			style.WindowRounding = 0.0f;
			style.WindowBorderSize = 1.0f;
			style.WindowMinSize = ImVec2(32.0f, 32.0f);
			style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
			style.WindowMenuButtonPosition = ImGuiDir_Right;
			style.ChildRounding = 3.0f;
			style.ChildBorderSize = 1.0f;
			style.PopupRounding = 5.0f;
			style.PopupBorderSize = 1.0f;
			style.FramePadding = ImVec2(20.0f, 8.1f);
			style.FrameRounding = 2.0f;
			style.FrameBorderSize = 0.0f;
			style.ItemSpacing = ImVec2(3.0f, 3.0f);
			style.ItemInnerSpacing = ImVec2(3.0f, 8.0f);
			style.CellPadding = ImVec2(6.0f, 14.1f);
			style.IndentSpacing = 0.0f;
			style.ColumnsMinSpacing = 10.0f;
			style.ScrollbarSize = 10.0f;
			style.ScrollbarRounding = 2.0f;
			style.GrabMinSize = 12.1f;
			style.GrabRounding = 1.0f;
			style.TabRounding = 2.0f;
			style.TabBorderSize = 0.0f;
			style.ColorButtonPosition = ImGuiDir_Right;
			style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
			style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

			style.Colors[ImGuiCol_Text] = ImVec4(0.98039216f, 0.98039216f, 0.98039216f, 1.0f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 1.0f);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09411765f, 0.09411765f, 0.09411765f, 1.0f);
			style.Colors[ImGuiCol_ChildBg] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 1.0f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(0.09411765f, 0.09411765f, 0.09411765f, 1.0f);
			style.Colors[ImGuiCol_Border] = ImVec4(1.0f, 1.0f, 1.0f, 0.09803922f);
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.09803922f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.15686275f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.047058824f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.11764706f, 0.11764706f, 0.11764706f, 1.0f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 1.0f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.11764706f, 0.11764706f, 0.11764706f, 1.0f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.10980392f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.0f, 1.0f, 1.0f, 0.39215687f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.47058824f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.09803922f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 1.0f, 1.0f, 0.39215687f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.3137255f);
			style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 1.0f, 0.09803922f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.15686275f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.047058824f);
			style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 1.0f, 1.0f, 0.09803922f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.15686275f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.047058824f);
			style.Colors[ImGuiCol_Separator] = ImVec4(1.0f, 1.0f, 1.0f, 0.15686275f);
			style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.23529412f);
			style.Colors[ImGuiCol_SeparatorActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.23529412f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.15686275f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.23529412f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.23529412f);
			style.Colors[ImGuiCol_Tab] = ImVec4(1.0f, 1.0f, 1.0f, 0.09803922f);
			style.Colors[ImGuiCol_TabHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.15686275f);
			style.Colors[ImGuiCol_TabActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.3137255f);
			style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0f, 0.0f, 0.0f, 0.15686275f);
			style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.23529412f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(1.0f, 1.0f, 1.0f, 0.3529412f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 1.0f, 1.0f, 0.3529412f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 1.0f);
			style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(1.0f, 1.0f, 1.0f, 0.3137255f);
			style.Colors[ImGuiCol_TableBorderLight] = ImVec4(1.0f, 1.0f, 1.0f, 0.19607843f);
			style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.019607844f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.16862746f, 0.23137255f, 0.5372549f, 1.0f);
			style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
			style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
			style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5647059f);
		}

		void SetTheme_ClassicSteam()
		{
			// Classic Steam style by metasprite from ImThemes
			ImGuiStyle& style = ImGui::GetStyle();
			style.Alpha = 1.0f;
			style.DisabledAlpha = 0.6f;
			style.WindowPadding = ImVec2(8.0f, 8.0f);
			style.WindowRounding = 0.0f;
			style.WindowBorderSize = 1.0f;
			style.WindowMinSize = ImVec2(32.0f, 32.0f);
			style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
			style.WindowMenuButtonPosition = ImGuiDir_Left;
			style.ChildRounding = 0.0f;
			style.ChildBorderSize = 1.0f;
			style.PopupRounding = 0.0f;
			style.PopupBorderSize = 1.0f;
			style.FramePadding = ImVec2(4.0f, 3.0f);
			style.FrameRounding = 0.0f;
			style.FrameBorderSize = 1.0f;
			style.ItemSpacing = ImVec2(8.0f, 4.0f);
			style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
			style.CellPadding = ImVec2(4.0f, 2.0f);
			style.IndentSpacing = 21.0f;
			style.ColumnsMinSpacing = 6.0f;
			style.ScrollbarSize = 14.0f;
			style.ScrollbarRounding = 0.0f;
			style.GrabMinSize = 10.0f;
			style.GrabRounding = 0.0f;
			style.TabRounding = 0.0f;
			style.TabBorderSize = 0.0f;
			style.ColorButtonPosition = ImGuiDir_Right;
			style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
			style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 1.0f);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.28627452f, 0.3372549f, 0.25882354f, 1.0f);
			style.Colors[ImGuiCol_ChildBg] = ImVec4(0.28627452f, 0.3372549f, 0.25882354f, 1.0f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(0.23921569f, 0.26666668f, 0.2f, 1.0f);
			style.Colors[ImGuiCol_Border] = ImVec4(0.5372549f, 0.5686275f, 0.50980395f, 0.5f);
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.13725491f, 0.15686275f, 0.10980392f, 0.52f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(0.23921569f, 0.26666668f, 0.2f, 1.0f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26666668f, 0.29803923f, 0.22745098f, 1.0f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.29803923f, 0.3372549f, 0.25882354f, 1.0f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.23921569f, 0.26666668f, 0.2f, 1.0f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.28627452f, 0.3372549f, 0.25882354f, 1.0f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.51f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.23921569f, 0.26666668f, 0.2f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.34901962f, 0.41960785f, 0.30980393f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2784314f, 0.31764707f, 0.23921569f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.24705882f, 0.29803923f, 0.21960784f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.22745098f, 0.26666668f, 0.20784314f, 1.0f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(0.5882353f, 0.5372549f, 0.1764706f, 1.0f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.34901962f, 0.41960785f, 0.30980393f, 1.0f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5372549f, 0.5686275f, 0.50980395f, 0.5f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.28627452f, 0.3372549f, 0.25882354f, 0.4f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.34901962f, 0.41960785f, 0.30980393f, 1.0f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.5372549f, 0.5686275f, 0.50980395f, 0.5f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.34901962f, 0.41960785f, 0.30980393f, 1.0f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.34901962f, 0.41960785f, 0.30980393f, 0.6f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.5372549f, 0.5686275f, 0.50980395f, 0.5f);
			style.Colors[ImGuiCol_Separator] = ImVec4(0.13725491f, 0.15686275f, 0.10980392f, 1.0f);
			style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.5372549f, 0.5686275f, 0.50980395f, 1.0f);
			style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.5882353f, 0.5372549f, 0.1764706f, 1.0f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1882353f, 0.22745098f, 0.1764706f, 0.0f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.5372549f, 0.5686275f, 0.50980395f, 1.0f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.5882353f, 0.5372549f, 0.1764706f, 1.0f);
			style.Colors[ImGuiCol_Tab] = ImVec4(0.34901962f, 0.41960785f, 0.30980393f, 1.0f);
			style.Colors[ImGuiCol_TabHovered] = ImVec4(0.5372549f, 0.5686275f, 0.50980395f, 0.78f);
			style.Colors[ImGuiCol_TabActive] = ImVec4(0.5882353f, 0.5372549f, 0.1764706f, 1.0f);
			style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.23921569f, 0.26666668f, 0.2f, 1.0f);
			style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.34901962f, 0.41960785f, 0.30980393f, 1.0f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(0.60784316f, 0.60784316f, 0.60784316f, 1.0f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.5882353f, 0.5372549f, 0.1764706f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.7764706f, 0.2784314f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882353f, 0.1882353f, 0.2f, 1.0f);
			style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
			style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.22745098f, 0.22745098f, 0.24705882f, 1.0f);
			style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.5882353f, 0.5372549f, 0.1764706f, 1.0f);
			style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.7294118f, 0.6666667f, 0.23921569f, 1.0f);
			style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.5882353f, 0.5372549f, 0.1764706f, 1.0f);
			style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
			style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
			style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
		}

		void SetTheme_GreenLeaf()
		{
			// Green Leaf style by Fizub from ImThemes
			ImGuiStyle& style = ImGui::GetStyle();

			style.Alpha = 1.0f;
			style.DisabledAlpha = 0.6f;
			style.WindowPadding = ImVec2(8.0f, 8.0f);
			style.WindowRounding = 0.0f;
			style.WindowBorderSize = 0.0f;
			style.WindowMinSize = ImVec2(32.0f, 32.0f);
			style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
			style.WindowMenuButtonPosition = ImGuiDir_Left;
			style.ChildRounding = 4.2f;
			style.ChildBorderSize = 1.0f;
			style.PopupRounding = 7.3f;
			style.PopupBorderSize = 1.0f;
			style.FramePadding = ImVec2(4.0f, 3.0f);
			style.FrameRounding = 0.0f;
			style.FrameBorderSize = 0.0f;
			style.ItemSpacing = ImVec2(8.0f, 4.0f);
			style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
			style.CellPadding = ImVec2(4.0f, 2.0f);
			style.IndentSpacing = 21.0f;
			style.ColumnsMinSpacing = 6.0f;
			style.ScrollbarSize = 10.2f;
			style.ScrollbarRounding = 9.0f;
			style.GrabMinSize = 10.0f;
			style.GrabRounding = 0.0f;
			style.TabRounding = 4.0f;
			style.TabBorderSize = 0.0f;
			style.ColorButtonPosition = ImGuiDir_Right;
			style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
			style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.44313726f, 0.44313726f, 0.44313726f, 1.0f);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.19607843f, 0.22745098f, 0.14117648f, 0.85f);
			style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(0.10980392f, 0.10980392f, 0.13725491f, 0.92f);
			style.Colors[ImGuiCol_Border] = ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 0.5f);
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(0.42745098f, 0.42745098f, 0.42745098f, 0.39f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.07450981f, 0.75686276f, 0.57254905f, 0.4f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2784314f, 1.0f, 0.7490196f, 0.69f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.3882353f, 0.53333336f, 0.34117648f, 0.83f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.3019608f, 0.57254905f, 0.29803923f, 0.87f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.4392157f, 0.8f, 0.4f, 0.2f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2901961f, 0.38039216f, 0.27450982f, 0.8f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2f, 0.24705882f, 0.29803923f, 0.6f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39215687f, 0.9137255f, 0.58431375f, 0.3f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4f, 0.8f, 0.5803922f, 0.4f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.3882353f, 0.8f, 0.4117647f, 0.6f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(0.5137255f, 0.5882353f, 0.3372549f, 1.0f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.23921569f, 0.8784314f, 0.49411765f, 1.0f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.19607843f, 0.49019608f, 0.25882354f, 1.0f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.5254902f, 0.8039216f, 0.64705884f, 0.62f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.39607844f, 0.63529414f, 0.47843137f, 0.79f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1764706f, 0.28627452f, 0.16862746f, 1.0f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.3882353f, 0.5019608f, 0.30980393f, 0.45f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.47058824f, 0.6392157f, 0.53333336f, 0.8f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.5686275f, 0.78431374f, 0.56078434f, 0.8f);
			style.Colors[ImGuiCol_Separator] = ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 0.6f);
			style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.20784314f, 0.32941177f, 0.27058825f, 1.0f);
			style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.29411766f, 0.4509804f, 0.3764706f, 1.0f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.41568628f, 0.99215686f, 0.58431375f, 0.6f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.35686275f, 0.5372549f, 0.43529412f, 0.9f);
			style.Colors[ImGuiCol_Tab] = ImVec4(0.23529412f, 0.34509805f, 0.26666668f, 0.786f);
			style.Colors[ImGuiCol_TabHovered] = ImVec4(0.10980392f, 0.23137255f, 0.12941177f, 0.8f);
			style.Colors[ImGuiCol_TabActive] = ImVec4(0.5764706f, 1.0f, 0.5803922f, 0.38197422f);
			style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.22745098f, 0.40392157f, 0.22352941f, 0.8212f);
			style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.39607844f, 0.77254903f, 0.5921569f, 0.8372f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.8980392f, 0.69803923f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392f, 0.69803923f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.28235295f, 0.3764706f, 0.26666668f, 1.0f);
			style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30980393f, 0.44705883f, 0.3254902f, 1.0f);
			style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.25882354f, 0.25882354f, 0.2784314f, 1.0f);
			style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.07f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2627451f, 0.63529414f, 0.3647059f, 0.35f);
			style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
			style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.36862746f, 0.5019608f, 0.24313726f, 0.8f);
			style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
			style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
			style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.35f);
		}

		void SetTheme_Darcula()
		{
			// Darcula style by ice1000 from ImThemes
			ImGuiStyle& style = ImGui::GetStyle();

			style.Alpha = 1.0f;
			style.DisabledAlpha = 0.6f;
			style.WindowPadding = ImVec2(8.0f, 8.0f);
			style.WindowRounding = 5.3f;
			style.WindowBorderSize = 1.0f;
			style.WindowMinSize = ImVec2(32.0f, 32.0f);
			style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
			style.WindowMenuButtonPosition = ImGuiDir_Left;
			style.ChildRounding = 0.0f;
			style.ChildBorderSize = 1.0f;
			style.PopupRounding = 0.0f;
			style.PopupBorderSize = 1.0f;
			style.FramePadding = ImVec2(4.0f, 3.0f);
			style.FrameRounding = 2.3f;
			style.FrameBorderSize = 1.0f;
			style.ItemSpacing = ImVec2(8.0f, 6.5f);
			style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
			style.CellPadding = ImVec2(4.0f, 2.0f);
			style.IndentSpacing = 21.0f;
			style.ColumnsMinSpacing = 6.0f;
			style.ScrollbarSize = 14.0f;
			style.ScrollbarRounding = 5.0f;
			style.GrabMinSize = 10.0f;
			style.GrabRounding = 2.3f;
			style.TabRounding = 4.0f;
			style.TabBorderSize = 0.0f;
			style.ColorButtonPosition = ImGuiDir_Right;
			style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
			style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

			style.Colors[ImGuiCol_Text] = ImVec4(0.73333335f, 0.73333335f, 0.73333335f, 1.0f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.34509805f, 0.34509805f, 0.34509805f, 1.0f);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.23529412f, 0.24705882f, 0.25490198f, 0.94f);
			style.Colors[ImGuiCol_ChildBg] = ImVec4(0.23529412f, 0.24705882f, 0.25490198f, 0.0f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(0.23529412f, 0.24705882f, 0.25490198f, 0.94f);
			style.Colors[ImGuiCol_Border] = ImVec4(0.33333334f, 0.33333334f, 0.33333334f, 0.5f);
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 0.0f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16862746f, 0.16862746f, 0.16862746f, 0.54f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.4509804f, 0.6745098f, 0.99607843f, 0.67f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.47058824f, 0.47058824f, 0.47058824f, 0.67f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.039215688f, 0.039215688f, 0.039215688f, 1.0f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.51f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15686275f, 0.28627452f, 0.47843137f, 1.0f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.27058825f, 0.28627452f, 0.2901961f, 0.8f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.27058825f, 0.28627452f, 0.2901961f, 0.6f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.21960784f, 0.30980393f, 0.41960785f, 0.51f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.21960784f, 0.30980393f, 0.41960785f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.13725491f, 0.19215687f, 0.2627451f, 0.91f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(0.8980392f, 0.8980392f, 0.8980392f, 0.83f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.69803923f, 0.69803923f, 0.69803923f, 0.62f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.29803923f, 0.29803923f, 0.29803923f, 0.84f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.33333334f, 0.3529412f, 0.36078432f, 0.49f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.21960784f, 0.30980393f, 0.41960785f, 1.0f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.13725491f, 0.19215687f, 0.2627451f, 1.0f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.33333334f, 0.3529412f, 0.36078432f, 0.53f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4509804f, 0.6745098f, 0.99607843f, 0.67f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.47058824f, 0.47058824f, 0.47058824f, 0.67f);
			style.Colors[ImGuiCol_Separator] = ImVec4(0.3137255f, 0.3137255f, 0.3137255f, 1.0f);
			style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3137255f, 0.3137255f, 0.3137255f, 1.0f);
			style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.3137255f, 0.3137255f, 0.3137255f, 1.0f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.85f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.6f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.9f);
			style.Colors[ImGuiCol_Tab] = ImVec4(0.1764706f, 0.34901962f, 0.5764706f, 0.862f);
			style.Colors[ImGuiCol_TabHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
			style.Colors[ImGuiCol_TabActive] = ImVec4(0.19607843f, 0.40784314f, 0.6784314f, 1.0f);
			style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667f, 0.101960786f, 0.14509805f, 0.9724f);
			style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13333334f, 0.25882354f, 0.42352942f, 1.0f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(0.60784316f, 0.60784316f, 0.60784316f, 1.0f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.42745098f, 0.34901962f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392f, 0.69803923f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882353f, 0.1882353f, 0.2f, 1.0f);
			style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
			style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.22745098f, 0.22745098f, 0.24705882f, 1.0f);
			style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.18431373f, 0.39607844f, 0.7921569f, 0.9f);
			style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
			style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
			style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
			style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
		}

		void SetTheme_DiscordDark()
		{
			// Discord (Dark) style by BttrDrgn from ImThemes
			ImGuiStyle& style = ImGui::GetStyle();

			style.Alpha = 1.0f;
			style.DisabledAlpha = 0.6f;
			style.WindowPadding = ImVec2(8.0f, 8.0f);
			style.WindowRounding = 0.0f;
			style.WindowBorderSize = 1.0f;
			style.WindowMinSize = ImVec2(32.0f, 32.0f);
			style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
			style.WindowMenuButtonPosition = ImGuiDir_Left;
			style.ChildRounding = 0.0f;
			style.ChildBorderSize = 1.0f;
			style.PopupRounding = 0.0f;
			style.PopupBorderSize = 1.0f;
			style.FramePadding = ImVec2(4.0f, 3.0f);
			style.FrameRounding = 0.0f;
			style.FrameBorderSize = 0.0f;
			style.ItemSpacing = ImVec2(8.0f, 4.0f);
			style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
			style.CellPadding = ImVec2(4.0f, 2.0f);
			style.IndentSpacing = 21.0f;
			style.ColumnsMinSpacing = 6.0f;
			style.ScrollbarSize = 14.0f;
			style.ScrollbarRounding = 0.0f;
			style.GrabMinSize = 10.0f;
			style.GrabRounding = 0.0f;
			style.TabRounding = 0.0f;
			style.TabBorderSize = 0.0f;
			style.ColorButtonPosition = ImGuiDir_Right;
			style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
			style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 1.0f);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.21176471f, 0.22352941f, 0.24705882f, 1.0f);
			style.Colors[ImGuiCol_ChildBg] = ImVec4(0.18431373f, 0.19215687f, 0.21176471f, 1.0f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(0.078431375f, 0.078431375f, 0.078431375f, 0.94f);
			style.Colors[ImGuiCol_Border] = ImVec4(0.42745098f, 0.42745098f, 0.49803922f, 0.5f);
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(0.30980393f, 0.32941177f, 0.36078432f, 1.0f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30980393f, 0.32941177f, 0.36078432f, 1.0f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.34509805f, 0.39607844f, 0.9490196f, 1.0f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.18431373f, 0.19215687f, 0.21176471f, 1.0f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1254902f, 0.13333334f, 0.14509805f, 1.0f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1254902f, 0.13333334f, 0.14509805f, 1.0f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1254902f, 0.13333334f, 0.14509805f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.019607844f, 0.019607844f, 0.019607844f, 0.53f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30980393f, 0.30980393f, 0.30980393f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40784314f, 0.40784314f, 0.40784314f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50980395f, 0.50980395f, 0.50980395f, 1.0f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(0.23137255f, 0.64705884f, 0.3647059f, 1.0f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.30980393f, 0.32941177f, 0.36078432f, 1.0f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.40784314f, 0.42745098f, 0.4509804f, 1.0f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1254902f, 0.13333334f, 0.14509805f, 1.0f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.30980393f, 0.32941177f, 0.36078432f, 1.0f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.40784314f, 0.42745098f, 0.4509804f, 1.0f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.40784314f, 0.42745098f, 0.4509804f, 1.0f);
			style.Colors[ImGuiCol_Separator] = ImVec4(0.42745098f, 0.42745098f, 0.49803922f, 0.5f);
			style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.09803922f, 0.4f, 0.7490196f, 0.78f);
			style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.09803922f, 0.4f, 0.7490196f, 1.0f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.2f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.67f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.95f);
			style.Colors[ImGuiCol_Tab] = ImVec4(0.18431373f, 0.19215687f, 0.21176471f, 1.0f);
			style.Colors[ImGuiCol_TabHovered] = ImVec4(0.23529412f, 0.24705882f, 0.27058825f, 1.0f);
			style.Colors[ImGuiCol_TabActive] = ImVec4(0.25882354f, 0.27450982f, 0.3019608f, 1.0f);
			style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667f, 0.101960786f, 0.14509805f, 0.9724f);
			style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13333334f, 0.25882354f, 0.42352942f, 1.0f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(0.60784316f, 0.60784316f, 0.60784316f, 1.0f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.34509805f, 0.39607844f, 0.9490196f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.34509805f, 0.39607844f, 0.9490196f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.36078432f, 0.4f, 0.42745098f, 1.0f);
			style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882353f, 0.1882353f, 0.2f, 1.0f);
			style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
			style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.22745098f, 0.22745098f, 0.24705882f, 1.0f);
			style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.050980393f, 0.41960785f, 0.85882354f, 1.0f);
			style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.34509805f, 0.39607844f, 0.9490196f, 1.0f);
			style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
			style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
			style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
			style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
		}

		void SetTheme_DeepDark()
		{
			// Deep Dark style by janekb04 from ImThemes
			ImGuiStyle& style = ImGui::GetStyle();

			style.Alpha = 1.0f;
			style.DisabledAlpha = 0.6f;
			style.WindowPadding = ImVec2(8.0f, 8.0f);
			style.WindowRounding = 7.0f;
			style.WindowBorderSize = 1.0f;
			style.WindowMinSize = ImVec2(32.0f, 32.0f);
			style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
			style.WindowMenuButtonPosition = ImGuiDir_Left;
			style.ChildRounding = 4.0f;
			style.ChildBorderSize = 1.0f;
			style.PopupRounding = 4.0f;
			style.PopupBorderSize = 1.0f;
			style.FramePadding = ImVec2(5.0f, 2.0f);
			style.FrameRounding = 3.0f;
			style.FrameBorderSize = 1.0f;
			style.ItemSpacing = ImVec2(6.0f, 6.0f);
			style.ItemInnerSpacing = ImVec2(6.0f, 6.0f);
			style.CellPadding = ImVec2(6.0f, 6.0f);
			style.IndentSpacing = 25.0f;
			style.ColumnsMinSpacing = 6.0f;
			style.ScrollbarSize = 15.0f;
			style.ScrollbarRounding = 9.0f;
			style.GrabMinSize = 10.0f;
			style.GrabRounding = 3.0f;
			style.TabRounding = 4.0f;
			style.TabBorderSize = 1.0f;
			style.ColorButtonPosition = ImGuiDir_Right;
			style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
			style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 1.0f);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09803922f, 0.09803922f, 0.09803922f, 1.0f);
			style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1882353f, 0.1882353f, 0.1882353f, 0.92f);
			style.Colors[ImGuiCol_Border] = ImVec4(0.1882353f, 0.1882353f, 0.1882353f, 0.29f);
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.24f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(0.047058824f, 0.047058824f, 0.047058824f, 0.54f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1882353f, 0.1882353f, 0.1882353f, 0.54f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2f, 0.21960784f, 0.22745098f, 1.0f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.05882353f, 0.05882353f, 0.05882353f, 1.0f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.13725491f, 0.13725491f, 0.13725491f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.047058824f, 0.047058824f, 0.047058824f, 0.54f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3372549f, 0.3372549f, 0.3372549f, 0.54f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4f, 0.4f, 0.4f, 0.54f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5568628f, 0.5568628f, 0.5568628f, 0.54f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(0.32941177f, 0.6666667f, 0.85882354f, 1.0f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3372549f, 0.3372549f, 0.3372549f, 0.54f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5568628f, 0.5568628f, 0.5568628f, 0.54f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.047058824f, 0.047058824f, 0.047058824f, 0.54f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1882353f, 0.1882353f, 0.1882353f, 0.54f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2f, 0.21960784f, 0.22745098f, 1.0f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.52f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.36f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2f, 0.21960784f, 0.22745098f, 0.33f);
			style.Colors[ImGuiCol_Separator] = ImVec4(0.2784314f, 0.2784314f, 0.2784314f, 0.29f);
			style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.4392157f, 0.4392157f, 0.4392157f, 0.29f);
			style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.4f, 0.4392157f, 0.46666667f, 1.0f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2784314f, 0.2784314f, 0.2784314f, 0.29f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.4392157f, 0.4392157f, 0.4392157f, 0.29f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.4f, 0.4392157f, 0.46666667f, 1.0f);
			style.Colors[ImGuiCol_Tab] = ImVec4(0.0f, 0.0f, 0.0f, 0.52f);
			style.Colors[ImGuiCol_TabHovered] = ImVec4(0.13725491f, 0.13725491f, 0.13725491f, 1.0f);
			style.Colors[ImGuiCol_TabActive] = ImVec4(0.2f, 0.2f, 0.2f, 0.36f);
			style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0f, 0.0f, 0.0f, 0.52f);
			style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13725491f, 0.13725491f, 0.13725491f, 1.0f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.52f);
			style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0f, 0.0f, 0.0f, 0.52f);
			style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2784314f, 0.2784314f, 0.2784314f, 0.29f);
			style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
			style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2f, 0.21960784f, 0.22745098f, 1.0f);
			style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.32941177f, 0.6666667f, 0.85882354f, 1.0f);
			style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 0.0f, 0.0f, 0.7f);
			style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.2f);
			style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.35f);
		}

		void SetTheme_Moonlight()
		{
			// Moonlight style by Madam-Herta from ImThemes
			ImGuiStyle& style = ImGui::GetStyle();

			style.Alpha = 1.0f;
			style.DisabledAlpha = 1.0f;
			style.WindowPadding = ImVec2(12.0f, 12.0f);
			style.WindowRounding = 11.5f;
			style.WindowBorderSize = 0.0f;
			style.WindowMinSize = ImVec2(20.0f, 20.0f);
			style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
			style.WindowMenuButtonPosition = ImGuiDir_Right;
			style.ChildRounding = 0.0f;
			style.ChildBorderSize = 1.0f;
			style.PopupRounding = 0.0f;
			style.PopupBorderSize = 1.0f;
			style.FramePadding = ImVec2(20.0f, 3.4f);
			style.FrameRounding = 11.9f;
			style.FrameBorderSize = 0.0f;
			style.ItemSpacing = ImVec2(4.3f, 5.5f);
			style.ItemInnerSpacing = ImVec2(7.1f, 1.8f);
			style.CellPadding = ImVec2(12.1f, 9.2f);
			style.IndentSpacing = 0.0f;
			style.ColumnsMinSpacing = 4.9f;
			style.ScrollbarSize = 11.6f;
			style.ScrollbarRounding = 15.9f;
			style.GrabMinSize = 3.7f;
			style.GrabRounding = 20.0f;
			style.TabRounding = 0.0f;
			style.TabBorderSize = 0.0f;
			style.ColorButtonPosition = ImGuiDir_Right;
			style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
			style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.27450982f, 0.31764707f, 0.4509804f, 1.0f);
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
			style.Colors[ImGuiCol_ChildBg] = ImVec4(0.09411765f, 0.101960786f, 0.11764706f, 1.0f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
			style.Colors[ImGuiCol_Border] = ImVec4(0.15686275f, 0.16862746f, 0.19215687f, 1.0f);
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(0.11372549f, 0.1254902f, 0.15294118f, 1.0f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15686275f, 0.16862746f, 0.19215687f, 1.0f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.15686275f, 0.16862746f, 0.19215687f, 1.0f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
			style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09803922f, 0.105882354f, 0.12156863f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.15686275f, 0.16862746f, 0.19215687f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(0.972549f, 1.0f, 0.49803922f, 1.0f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.972549f, 1.0f, 0.49803922f, 1.0f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.79607844f, 0.49803922f, 1.0f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.18039216f, 0.1882353f, 0.19607843f, 1.0f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.15294118f, 0.15294118f, 0.15294118f, 1.0f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.14117648f, 0.16470589f, 0.20784314f, 1.0f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.105882354f, 0.105882354f, 0.105882354f, 1.0f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
			style.Colors[ImGuiCol_Separator] = ImVec4(0.12941177f, 0.14901961f, 0.19215687f, 1.0f);
			style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.15686275f, 0.18431373f, 0.2509804f, 1.0f);
			style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.15686275f, 0.18431373f, 0.2509804f, 1.0f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.14509805f, 0.14509805f, 0.14509805f, 1.0f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.972549f, 1.0f, 0.49803922f, 1.0f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_Tab] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
			style.Colors[ImGuiCol_TabHovered] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
			style.Colors[ImGuiCol_TabActive] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
			style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
			style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1254902f, 0.27450982f, 0.57254905f, 1.0f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(0.52156866f, 0.6f, 0.7019608f, 1.0f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.039215688f, 0.98039216f, 0.98039216f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.88235295f, 0.79607844f, 0.56078434f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.95686275f, 0.95686275f, 0.95686275f, 1.0f);
			style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
			style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
			style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
			style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.09803922f, 0.105882354f, 0.12156863f, 1.0f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.9372549f, 0.9372549f, 0.9372549f, 1.0f);
			style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.49803922f, 0.5137255f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26666668f, 0.2901961f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.49803922f, 0.5137255f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.19607843f, 0.1764706f, 0.54509807f, 0.5019608f);
			style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.19607843f, 0.1764706f, 0.54509807f, 0.5019608f);
		}

#pragma endregion

		EditorThemeType GetThemeTypeFromString(std::string themeName)
		{

			for (int i = 0; i < (int)EditorThemes.size(); i++)
			{
				if (EditorThemes[i] == themeName)
					return EditorThemeType(i);
			}

			return EditorThemeType{};
		}

		void SetTheme(EditorThemeType type)
		{
			switch (type)
			{
			case EditorThemeType::DARK:
				SetTheme_Dark();
				break;
			case EditorThemeType::LIGHT:
				SetTheme_Light();
				break;
			case EditorThemeType::MICROSOFT:
				SetTheme_Microsoft();
				break;
			case EditorThemeType::CLASSICSTEAM:
				SetTheme_ClassicSteam();
				break;
			case EditorThemeType::GREENLEAF:
				SetTheme_GreenLeaf();
				break;
			case EditorThemeType::DARCULA:
				SetTheme_Darcula();
				break;
			case EditorThemeType::DISCORDDARK:
				SetTheme_DiscordDark();
				break;
			case EditorThemeType::DEEPDARK:
				SetTheme_DeepDark();
				break;
			case EditorThemeType::MOONLIGHT:
				SetTheme_Moonlight();
				break;
			case EditorThemeType::EXCELLENCY:
				SetTheme_Excellency();
				break;
			case EditorThemeType::REST:
				SetTheme_Rest();
				break;
			default:
				break;
			}
		}

#pragma endregion
	}

#pragma endregion
}