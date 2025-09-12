#ifndef TRANSFORM_MANAGER_H
#define TRANSFORM_MANAGER_H

#include "ECS/BaseSystem.h"
#include "ECS/ECSTypes.h"

namespace SliceEngine
{
	struct transformEntity {};

	struct TransformSystem : BaseSystem<transformEntity, Transform>
	{
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
	};
}


#endif