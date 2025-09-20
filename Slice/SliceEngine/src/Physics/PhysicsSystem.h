#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include <pch.h>
#include <iostream>

#include "ECS/BaseSystem.h"
#include "ECS/ECSTypes.h"
#include "CollisionLayer.h"


namespace SliceEngine
{
	// for keeping track of entities that belong to physics system
	struct PhysicEntity {};

	struct PhysicsSystem final: BaseSystem<PhysicEntity, Transform, RigidBody>
	{
	private:

		std::unique_ptr<JPH::PhysicsSystem> physicsSystem;
		std::unique_ptr<JPH::JobSystemThreadPool> jobSystem;
		std::unique_ptr<BPLayerInterfaceImpl> broadphaseLayerInterface;
		std::unique_ptr<ObjectVsBroadPhaseLayerFilterImpl> objectVsBroadphaseLayerFilter;
		std::unique_ptr<ObjectLayerPairFilterImpl> objectLayerPairFilter;
		std::unique_ptr <JPH::TempAllocatorImpl> tempAllocator;
		bool isInitialized = false; 

	public:

		PhysicsSystem() = default;

		PhysicsSystem(const PhysicsSystem&) = delete;

		PhysicsSystem& operator=(const PhysicsSystem&) = delete;

		~PhysicsSystem();

		void Shutdown();

		// may be redundant might remove return bool and change to void
		bool Initialize(JPH::uint maxBodies = 65536, JPH::uint numBodyMutex = 0, JPH::uint maxContactConstraint = 1024, JPH::uint threadCount = 0);

		bool IsInitialized();

		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;

		void EntityOnExit(entt::registry& reg, entt::entity entity) override;

		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
	};


}

#endif
