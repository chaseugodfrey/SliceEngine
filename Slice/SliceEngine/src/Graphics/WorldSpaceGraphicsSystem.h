#ifndef WORLD_SPACE_GRAPHICS_SYSTEM_H
#define WORLD_SPACE_GRAPHICS_SYSTEM_H

#include "ResourceManager.h"
#include "../ECS/BaseSystem.h"
#include "../ECS/ECSTypes.h"

namespace SliceEngine
{
	struct renderEntity {};

	struct WorldSpaceGraphicsSystem : BaseSystem<renderEntity, Transform, Renderer>
	{
		void UseShader(ResourceManager* rcManager);
		void Render(ResourceManager* rcManager);

		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;

		Shader mShader;
		Model tempModel;
	};
}

#endif