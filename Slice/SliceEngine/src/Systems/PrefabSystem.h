#ifndef PREFAB_SYSTEM_H
#define PREFAB_SYSTEM_H
#include "ECS/BaseSystem.h"
#include "ECS/GameObject.h"

namespace SliceEngine
{
	struct PrefabEntity {};

	class PrefabSystem : public BaseSystem<PrefabEntity, Prefab>
	{
	private:
		// Keep track of prefab GUID to which entity when created
		std::map<GUID, Entity> mGUIDToEntity; 
		// when a prefab is modified, send an event to the prefab system
		// itll check which entities is made from the prefab
		

	public:
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
		GameObject CreatePrefab(const GUID& prefabGUID);

	};
}


#endif