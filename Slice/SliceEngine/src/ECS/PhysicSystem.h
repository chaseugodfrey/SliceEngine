#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "BaseSystem.h"
#include "ECSTypes.h"
#include <iostream>
// for testing purpose
// ill create the struct here to see if systems work
namespace SliceEngine
{
	//struct Transform
	//{
	//	float rot{};
	//};

	struct RigidBody
	{
		bool isKinematic{};
	};

	// for keeping track of entities that belong to physics system
	struct PhysicEntity {};

	struct PhysicSystem : BaseSystem<PhysicEntity, Transform, RigidBody>
	{
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override
		{
			std::cout << "Entity entering physics system" << std::endl;
		}

		void EntityOnExit(entt::registry& reg, entt::entity entity) override
		{
			std::cout << "Entity exiting physics system" << std::endl;
		}

		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override
		{
			auto& transform = reg.get<Transform>(entity);

			//std::cout << "Update Entity " << transform.rot << std::endl;
		}
	};


}

#endif