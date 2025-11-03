#ifndef EDITOR_UTILS_H
#define EDITOR_UTILS_H

namespace SliceEditor
{
	class HistoryManager;
	class SelectionManager;

	namespace EditorUtilities
	{
		SliceEngine::GameObject GameObject_CreateEmpty(entt::entity = entt::null, HistoryManager* history = nullptr);
		SliceEngine::GameObject GameObject_CreateBox(entt::entity = entt::null, HistoryManager* history = nullptr);
		SliceEngine::GameObject GameObject_CreateCam(entt::entity = entt::null, HistoryManager* history = nullptr);
		SliceEngine::GameObject GameObject_CreateModel(entt::entity parent, SliceEngine::GUID guid, HistoryManager* history = nullptr);
		void GameObject_Destroy(entt::entity target, HistoryManager* history = nullptr);
		void GameObject_Parent(entt::entity child, entt::entity parent = entt::null, HistoryManager* history = nullptr);
		void GameObject_Unparent(entt::entity child, HistoryManager* history = nullptr);
		void GameObject_SetSibling(entt::entity target, entt::entity destination, HistoryManager* history = nullptr);
		void Scene_Load(const std::filesystem::path& path, SelectionManager& selectionManager);
		void Scene_Stop(SelectionManager& selectionManager);

		// Preferences
		EditorThemeType GetThemeTypeFromString(std::string themeName);
		void SetTheme(EditorThemeType type);

		
	}
}

#endif