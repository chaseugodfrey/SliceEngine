
#include <pch.h>
#include "PhysicsSystem.h"
namespace SliceEngine {
	void PhysicsSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		std::cout << "Entity entering physics system" << std::endl;
	}

	void PhysicsSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		std::cout << "Entity exiting physics system" << std::endl;
	}

	void PhysicsSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		auto& transform = reg.get<Transform>(entity);

		//std::cout << "Update Entity " << transform.rot << std::endl;
	}

}
