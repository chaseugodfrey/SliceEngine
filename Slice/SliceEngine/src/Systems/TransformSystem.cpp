#include <pch.h>
#include "TransformSystem.h"

namespace SliceEngine
{
	void TransformSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		auto& transform = reg.get<Transform>(entity);
		if(transform.scale.x == 0.f && transform.scale.y == 0.f && transform.scale.z == 0.f)
			transform.scale = glm::vec3(1.f);
	}
	void TransformSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
	}
	void TransformSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
	}
}