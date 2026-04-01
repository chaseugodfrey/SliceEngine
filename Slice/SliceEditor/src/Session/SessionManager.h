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

		std::unordered_map<entt::entity, std::unique_ptr<EntityNode>> mEntityNodes;
		std::unordered_map<entt::entity, std::unique_ptr<EntityNode>> mPrefabNodes; //For Hierarchy
		Entity mPrefabRootEntity; //The Most-parented entity in the prefab
		std::vector<Entity> mHighlightedGameObjects; //The Most-parented entity in the prefab
		SliceEngine::GUID mInspectedPrefabGUID;
		SliceEngine::GUID mInspectedShaderGraphGUID{};

		bool mPrefabInspected;
		bool mShowHierarchyEntityIDs;
		bool mHighlightGOs;

		std::unique_ptr<AnimatorData> mAnimatorData;

		std::atomic<bool> isSavingScene{ false };
		std::atomic<bool> isSavingDone{ false };
		double duration = 0.0f;
		//float mGOScriptTimer = 0.0f;

	public:
		SessionManager(Registry& reg);
		~SessionManager();

		void Init() override;
		void Update() override;


		//Node Settings
		void SetNodeAsPrefab(EntityNode* entity, bool isPrefab);

		//Editor Hierarchy
		//void CreateEntityNodes();
		void UpdateEntityNodes();
		void AddEntityNode(entt::entity entity);
		void RemoveEntityNode(entt::entity entity);
		void HighlightGameObjects(const GameObjectScriptSelectedUpdate& event);

		//Scene Functions
		void OnSceneSave(OnSceneSaveEvent);
		void OnSceneChange(const OnSceneLoadedEvent& event);
		void OnSceneStop(const OnSceneStopEvent& event);


		void OnAssetFileChanged(const AssetFileChangedEvent& event);

		//Prefab Inspector Functions
		void PrefabInspected(const PrefabInspectedEvent& event);
		void BuildPrefabTree(Entity node);
		bool IsPrefabInspected();
		Entity GetPrefabEntityInspected();
		SliceEngine::GUID GetPrefabGUIDInspected();

		SliceEngine::GUID GetShaderGraphInspected();

		//Custom Shader Inspector Functions
		void ShaderGraphInspected(const ShaderGraphInspectedEvent& event);

		//Debug Settings
		void ToggleHierarchyEntityIDs();
		bool GetHierarchyEntityIDs();
		std::unordered_map<entt::entity, std::unique_ptr<EntityNode>>& GetEntityNodes();
		std::unordered_map<entt::entity, std::unique_ptr<EntityNode>>& GetPrefabNodes();

		// Animation Data
		void LoadAnimatorData(SliceEngine::GUID guid);
		void ClearAnimatorData();
		AnimatorData* GetAnimatorData();

		// Animator
		SliceEngine::Animator* currentAnimator;
	};

}
#endif