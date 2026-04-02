#ifndef EDITOR_UTILS_H
#define EDITOR_UTILS_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace SliceEditor
{
	class HistoryManager;
	class SelectionManager;
	class SessionManager;
	class ContentBrowserManager;
	class Registry;

	namespace EditorUtilities
	{

		//3D Objects
		SliceEngine::GameObject GameObject_CreateEmpty(entt::entity parent = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateBox(entt::entity parent = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateSphere(entt::entity parent = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateCapsule(entt::entity parent = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateCylinder(entt::entity parent = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateCam(entt::entity parent = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateLight(entt::entity parent = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);

		//2D Objects
		SliceEngine::GameObject GameObject_CreateCanvas(entt::entity = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateImage(entt::entity = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateButton(entt::entity = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateSlider(entt::entity = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreateText(entt::entity = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);


		SliceEngine::GameObject GameObject_CreateModel(SliceEngine::GUID guid, SliceEngine::GUID skeleGUID = SliceEngine::GUID::null(), SliceEngine::GUID animGUID = SliceEngine::GUID::null(), entt::entity parent = entt::null, HistoryManager* history = nullptr, bool isPrefabInspected = false);
		SliceEngine::GameObject GameObject_CreatePrefab(SliceEngine::GUID guid, entt::entity parent = entt::null, HistoryManager* history = nullptr);

		void GameObject_Unprefab(entt::entity entity);
		void GameObject_Clone(entt::entity entity);
		void GameObject_Destroy(entt::entity target, HistoryManager* history = nullptr);
		void GameObject_Parent(entt::entity child, entt::entity parent = entt::null, HistoryManager* history = nullptr);
		void GameObject_Unparent(entt::entity child, HistoryManager* history = nullptr);
		void GameObject_SetSibling(entt::entity target, entt::entity destination, HistoryManager* history = nullptr);
		void Scene_CreateDefault(std::string);
		void Scene_Load(const std::filesystem::path& path, SelectionManager& selectionManager,SessionManager& sessionManager);
		void Scene_Stop(SelectionManager& selectionManager);
		void Scene_Save();
		void Scene_CleanTempFiles(Registry& registry);
		void ContentBrowser_Refresh(ContentBrowserManager& contentBrowserManager);
		void Hierarchy_ToggleEntityID(Registry& registry);

		// Global Popup
		void MenuList_CreateFiles(Registry& reg, std::filesystem::path descPath);
		void MenuList_CreateGameObjects(HistoryManager* history = nullptr, entt::entity parent = entt::null, bool isPrefabInspected = false);


		//File Creation
		void CreateFile_MaterialFile(Registry& reg, std::filesystem::path descPath);
		void CreateFile_ShaderFile(Registry& reg, std::filesystem::path descPath);

		//Asset Drops
		

		// Preferences
		EditorThemeType GetThemeTypeFromString(std::string themeName);
		void SetTheme(EditorThemeType type);

		template <typename T>
		std::string ValueToString(const T& v)
		{
			using Decayed = std::decay_t<T>;

			if constexpr (std::is_same_v<Decayed, std::string>)
			{
				return v; // already a string
			}
			else if constexpr (std::is_same_v<Decayed, const char*>)
			{
				return v ? std::string(v) : std::string{};
			}
			else if constexpr (std::is_arithmetic_v<Decayed>)
			{
				return std::to_string(v);
			}
			else
			{
				// You are now *assuming* these are GLM types here
				return glm::to_string(v);
			}
		}

		// GUID overload
		inline std::string ValueToString(const SliceEngine::GUID& v)
		{
			return v.toString();
		}

		// Handle<T> overload
		template <typename T>
		std::string ValueToString(const SliceEngine::Handle<T>& v)
		{
			return ValueToString(v.getGUID());
		}
	}
}

#endif