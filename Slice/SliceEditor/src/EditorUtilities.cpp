#include <pch.h>
#include "EditorUtilities.h"
#include <History/HistoryManager.h>
#include <Selection/SelectionManager.h>
#include <ContentBrowser/ContentBrowserManager.h>
#include <../src/Systems/SceneSystem.h>
#include <../src/Systems/PrefabSystem.h>


namespace SliceEditor
{
	namespace EditorUtilities
	{

		SliceEngine::GameObject GameObject_CreateEmpty(entt::entity parent, HistoryManager* history)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateBox(entt::entity parent, HistoryManager* history)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Box();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateSphere(entt::entity parent, HistoryManager* history)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Sphere();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateCapsule(entt::entity parent, HistoryManager* history)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Capsule();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateCam(entt::entity parent, HistoryManager* history)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Cam();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateModel(SliceEngine::GUID guid, entt::entity parent, HistoryManager* history)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Model(guid);

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateCanvas(entt::entity parent, HistoryManager* history)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Canvas();

			//no parent for now because only overlay

			/*if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);*/

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}
		SliceEngine::GameObject GameObject_CreateImage(entt::entity parent, HistoryManager* history)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Image();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreateButton(entt::entity parent, HistoryManager* history)
		{
			auto& factory = SliceEngine::FactoryInstance;
			auto go = factory.CreateGO_Button();

			if (parent != entt::null)
				factory.SetParent(go.GetEntity(), parent);

			if (history)
			{
				history->AddCommand(std::make_unique<CreateEntityCommand>(go.GetEntity()));
			}

			return go;
		}

		SliceEngine::GameObject GameObject_CreatePrefab(SliceEngine::GUID guid, entt::entity parent, HistoryManager* history)
		{
			return SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PrefabSystem>().CreatePrefab(guid);
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
				factory.SetNewSceneGraphLocation(target, destination, destSceneGraph.neighbours[SliceEngine::SceneGraph::LEFT]);

				if (history)
				{

				}
			}
		}

		void GameObject_RemoveComponent(entt::entity entity, const std::string& componentName, HistoryManager* history = nullptr)
		{

		}
		
		void Scene_Load(const std::filesystem::path& path, SelectionManager& selectionManager)
		{
			SliceEngine::Core::GetInstance()->GetSceneSystem()->LoadSceneIntoQueue(path);
			selectionManager.ClearSelection(true);
		}

		void Scene_Stop(SelectionManager& selectionManager)
		{
			selectionManager.ClearSelection(true);
		}

		void Scene_Save()
		{
			SliceEngine::Core::GetInstance()->GetSceneSystem()->SaveCurrentScene();
		}

		void ContentBrowser_Refresh(ContentBrowserManager& contentBrowserManager)
		{
			contentBrowserManager.RebuildDirectory(*contentBrowserManager.rootNode);
		}

		void MenuList_CreateFiles(Registry& reg,std::filesystem::path descPath)
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Folder"))
				{

				}

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

				}

				ImGui::EndMenu();
			}
		}

		void MenuList_CreateGameObjects(HistoryManager* history, entt::entity parent)
		{
			if (ImGui::MenuItem("Empty"))
			{
				EditorUtilities::GameObject_CreateEmpty(parent, history);
			}

			if (ImGui::MenuItem("Camera"))
			{
				EditorUtilities::GameObject_CreateCam(parent, history);
			}

			if (ImGui::BeginMenu("3D Object"))
			{
				if (ImGui::MenuItem("Box"))
				{
					EditorUtilities::GameObject_CreateBox(parent, history);
				}

				if (ImGui::MenuItem("Sphere"))
				{
					EditorUtilities::GameObject_CreateSphere(parent, history);
				}

				if (ImGui::MenuItem("Capsule"))
				{
					EditorUtilities::GameObject_CreateCapsule(parent, history);
				}

				if (ImGui::MenuItem("Quad"))
				{
					EditorUtilities::GameObject_CreateBox(parent, history);
				}

				if (ImGui::MenuItem("Plane"))
				{
					EditorUtilities::GameObject_CreateBox(parent, history);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("UI"))
			{
				if (ImGui::MenuItem("Canvas"))
				{
					EditorUtilities::GameObject_CreateCanvas(entt::null, history);
				}

				if (ImGui::MenuItem("Text"))
				{
					//next tri
				}

				if (ImGui::MenuItem("Image"))
				{
					EditorUtilities::GameObject_CreateImage(entt::null, history);
				}

				if (ImGui::MenuItem("Button"))
				{
					EditorUtilities::GameObject_CreateButton(entt::null, history);
				}

				if (ImGui::MenuItem("Slider"))
				{
					//EditorUtilities::GameObject_CreateSlider(entt::null, history); - its jsut button with extra stuff
				}

				ImGui::EndMenu();
			}
		}

		void CreateFile_MaterialFile(Registry& reg, std::filesystem::path descPath)
		{
			reg.GetAssetManager().CreateDefaultAsset(descPath, AssetType::Material);
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
			default:
				break;
			}
		}

		#pragma endregion
	}

#pragma endregion
}