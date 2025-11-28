#ifndef EDITOR_UTILS_H
#define EDITOR_UTILS_H

namespace SliceEditor
{
	class HistoryManager;
	class SelectionManager;
	class ContentBrowserManager;
	class Registry;

	namespace EditorUtilities
	{
		//3D Objects
		SliceEngine::GameObject GameObject_CreateEmpty(entt::entity parent = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateBox(entt::entity parent = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateSphere(entt::entity parent = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateCapsule(entt::entity parent = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateCam(entt::entity parent = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);

		//2D Objects
		SliceEngine::GameObject GameObject_CreateCanvas(entt::entity = entt::null, HistoryManager* history = nullptr);
		SliceEngine::GameObject GameObject_CreateImage(entt::entity = entt::null, HistoryManager* history = nullptr);
		SliceEngine::GameObject GameObject_CreateButton(entt::entity = entt::null, HistoryManager* history = nullptr);


		SliceEngine::GameObject GameObject_CreateModel(SliceEngine::GUID guid, entt::entity parent = entt::null, HistoryManager* history = nullptr);
		SliceEngine::GameObject GameObject_CreatePrefab(SliceEngine::GUID guid, entt::entity parent = entt::null, HistoryManager* history = nullptr);

		void GameObject_Clone(entt::entity entity);
		void GameObject_Destroy(entt::entity target, HistoryManager* history = nullptr);
		void GameObject_Parent(entt::entity child, entt::entity parent = entt::null, HistoryManager* history = nullptr);
		void GameObject_Unparent(entt::entity child, HistoryManager* history = nullptr);
		void GameObject_SetSibling(entt::entity target, entt::entity destination, HistoryManager* history = nullptr);
		void Scene_Load(const std::filesystem::path& path, SelectionManager& selectionManager);
		void Scene_Stop(SelectionManager& selectionManager);
		void Scene_Save();
		void ContentBrowser_Refresh(ContentBrowserManager& contentBrowserManager);

		// Global Popup
		void MenuList_CreateFiles(Registry& reg, std::filesystem::path descPath);
		void MenuList_CreateGameObjects(HistoryManager* history = nullptr, entt::entity parent = entt::null, bool isPrefabInspected = false);


		//File Creation
		void CreateFile_MaterialFile(Registry& reg, std::filesystem::path descPath);

		//Asset Drops
		

		// Preferences
		EditorThemeType GetThemeTypeFromString(std::string themeName);
		void SetTheme(EditorThemeType type);

		
	}
}

#endif