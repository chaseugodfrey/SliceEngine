#ifndef PREFAB_SYSTEM_H
#define PREFAB_SYSTEM_H
#include "ECS/BaseSystem.h"
#include "ECS/GameObject.h"
#include "Resource/Prefab.h"
#include "Resource/ResourceManager.h"

namespace SliceEngine
{
	struct PrefabEntity {};

	struct PrefabComparator
	{
		bool operator()(const Handle<SliceEngineTypes::Prefab>& lhs,
			const Handle<SliceEngineTypes::Prefab>& rhs) const
		{
			return lhs.getGUID().GetGUID() < rhs.getGUID().GetGUID();
		}
	};

	class PrefabSystem : public BaseSystem<PrefabEntity, Prefab>
	{
	private:
		// Keep track of prefab GUID to which entity when created
		// since the component keeps track of the handle
		// we just need to know which GUID they reference from when its modified
		// then update from there
		std::unordered_map<GUID, std::vector<Entity>> mPrefabMap;
		//std::map<Handle<SliceEngineTypes::Prefab>, Entity, PrefabComparator> mPrefabToEntity;
		// when a prefab is modified, send an event to the prefab system
		// itll check which entities is made from the prefab
		std::unordered_map<GUID, Entity> mPrefabBaseEntities;

	public:
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
		GameObject CreatePrefab(const GUID& prefabGUID, bool isEditor = false);
		void UpdatePrefabChild(Entity entity, GUID const& guid, bool isEditor = false);
		void InitEvent();
		void OnPrefabModified(const OnPrefabModifiedEvent& event);
		void OnPrefabDeleted(const OnPrefabDeletedEvent& event);
		/// <summary>
		/// Add a prefab component to the entity after turning it into a prefab
		/// </summary>
		/// <param name="entity"></param>
		void MakePrefab(Entity entity);

		void UpdatePrefabComponent(Entity entity, GUID guid);

		/// <summary>
		/// Add an object to an existing prefab
		/// </summary>
		/// <param name="entity"></param>
		/// <param name="rootNode"></param>
		void AddToPrefab(Entity entity, Entity rootNode);

		bool IsNewGO(Entity entity, unsigned int prefabID);
	};
}


#endif