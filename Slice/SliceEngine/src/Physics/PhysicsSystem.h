#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "ECS/BaseSystem.h"
#include "ECS/ECSTypes.h"
#include <iostream>
namespace SliceEngine
{
	// for keeping track of entities that belong to physics system
	struct PhysicEntity {};

	struct PhysicsSystem : BaseSystem<PhysicEntity, Transform, RigidBody>
	{
		PhysicsSystem();

		PhysicsSystem(const PhysicsSystem&) = delete;

		PhysicsSystem& operator=(const PhysicsSystem&) = delete;

		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;

		void EntityOnExit(entt::registry& reg, entt::entity entity) override;

		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
	};


}

#endif
