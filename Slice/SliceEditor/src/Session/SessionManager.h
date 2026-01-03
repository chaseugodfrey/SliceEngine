#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include "../Core/IBaseManager.h"
#include <Core/Events.h>

namespace SliceEditor
{
	class Registry;
	class SelectedNode;

	class SessionManager : public IBaseManager
	{
		std::unique_ptr<Preferences> mPreferences;

		std::unordered_map<entt::entity, std::unique_ptr<EntityNode>> mEntityNodes;
		std::unordered_map<entt::entity, std::unique_ptr<EntityNode>> mPrefabNodes; //For Hierarchy
		Entity mPrefabRootEntity; //The Most-parented entity in the prefab

		bool mPrefabInspected;

		std::unique_ptr<AnimatorData> mAnimatorData;

		std::atomic<bool> isSavingScene{ false };
		std::atomic<bool> isSavingDone{ false };
		double duration = 0.0f;

	public:
		SessionManager(Registry& reg);
		~SessionManager();

		void Init() override;
		void Update() override;

		void OpenPreferences();
		void SetPreferences();
		void CreateDefaultPreferenceFile();
		void SavePreferences();
		Preferences& GetPreferences();

		void OnSceneSave(OnSceneSaveEvent);

		//Node Settings
		void SetNodeAsPrefab(EntityNode* entity, bool isPrefab);


		void CreateEntityNodes();
		void CreatePrefabNodes();
		void OnSceneChange(const OnSceneLoadedEvent& event);
		void OnSceneStop(const OnSceneStopEvent& event);
		void OnAssetFileChanged(const AssetFileChangedEvent& event);
		void PrefabInspected(const PrefabInspectedEvent& event);
		void BuildPrefabTree(Entity node);
		bool IsPrefabInspected();
		Entity GetPrefabInspected();
		std::unordered_map<entt::entity, std::unique_ptr<EntityNode>>& GetEntityNodes();
		std::unordered_map<entt::entity, std::unique_ptr<EntityNode>>& GetPrefabNodes();

		// Animation Data
		void LoadAnimatorData(SliceEngine::GUID guid);
		void ClearAnimatorData();
		AnimatorData* GetAnimatorData();

	};

}
#endif