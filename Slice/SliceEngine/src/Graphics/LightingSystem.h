#ifndef LIGHTING_SYSTEM_H
#define LIGHTING_SYSTEM_H

#include "../ECS/BaseSystem.h"
#include "../ECS/ECSTypes.h"

namespace SliceEngine
{
	struct lightingEntity {};

	struct LightingSystem : BaseSystem<lightingEntity, Transform, Light>
	{
		void SetLightingParams(GLuint shader);

		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
	};
}

#endif