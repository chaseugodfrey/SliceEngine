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
		std::unordered_map<entt::entity, std::unique_ptr<PrefabNode>> mPrefabNodes;

		std::unique_ptr<AnimatorData> mAnimatorData;

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

		void CreateEntityNodes();
		void OnSceneChange(const OnSceneLoadedEvent& event);
		void OnSceneStop(const OnSceneStopEvent& event);
		void OnAssetFileChanged(const AssetFileChangedEvent& event);
		std::unordered_map<entt::entity, std::unique_ptr<EntityNode>>& GetEntityNodes();

		// Animation Data
		void LoadAnimatorData(SliceEngine::GUID guid);
		void ClearAnimatorData();
		AnimatorData* GetAnimatorData();

	};

}
#endif