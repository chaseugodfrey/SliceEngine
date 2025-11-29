/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			PhysicsSystem.h
 author:		Aloysius Teo
 email:			teo.k@digipen.edu
 brief:			Handles all physics

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include <pch.h>
#include <iostream>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

#include "ECS/BaseSystem.h"
#include "ECS/ECSTypes.h"
#include "CollisionLayer.h"
#include "../Core/Events.h"
#include "ContactListener.h"

namespace 
{
	constexpr size_t TEN_MB = (10 * 1024 * 1024); //Jolt says 10mb is for typical usage;
}


namespace SliceEngine
{
	// for keeping track of entities that belong to physics system
	struct PhysicEntity {};

	class PhysicsSystem final: public BaseSystem<PhysicEntity, Transform, ColliderShape>
	{
	private:

		std::unique_ptr<JPH::PhysicsSystem> physicsSystem;
		std::unique_ptr<JPH::JobSystemThreadPool> jobSystem;
		std::unique_ptr<BPLayerInterfaceImpl> broadphaseLayerInterface;
		std::unique_ptr<ObjectVsBroadPhaseLayerFilterImpl> objectVsBroadphaseLayerFilter;
		std::unique_ptr<ObjectLayerPairFilterImpl> objectLayerPairFilter;
		std::unique_ptr <JPH::TempAllocatorImpl> tempAllocator;
		std::unique_ptr<MyContactListener> contactListener;
		bool isInitialized = false; 
		int collisionSteps{4};

	private:

		void Shutdown();

		void OnColliderAdd(const ColliderShapeAddedEvent& event);

		void OnColliderRemove(const ColliderShapeRemovedEvent& event);

		void OnRigidBodyAdd(const RigidBodyAddedEvent& event);

		void OnRigidBodyRemove(const RigidBodyRemovedEvent& event);

		void OnColliderModified(const ColliderShapeModifiedEvent& event);

		void OnRigidBodyModified( RigidBodyModifiedEvent& event);

		void OnSliceEntityModified(SliceEntityModifiedEvent& event);

		void UpdateShapeFromTransform(Entity entity);

		void SyncECSToPhysics(Transform& transform, ColliderShape& rigidBody) const;

		void SyncPhysicsToECS(Transform& transform, ColliderShape& rigidBody) const;

		void HandleRemovedContacts();

	public:

		PhysicsSystem() = default;

		PhysicsSystem(const PhysicsSystem&) = delete;

		PhysicsSystem& operator=(const PhysicsSystem&) = delete;

		~PhysicsSystem();

		// may be redundant might remove return bool and change to void
		bool Initialize(size_t tempAllocatorSize = TEN_MB, JPH::uint maxBodies = 65536, JPH::uint numBodyMutex = 0, JPH::uint maxContactConstraint = 1024, JPH::uint threadCount = 0);

		bool IsInitialized() const;

		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;

		void EntityOnExit(entt::registry& reg, entt::entity entity) override;

		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;

		void SubscribeToEvents();

		glm::vec3 GetLinearVelocity(Entity entity);

		void SetLinearVelocity(Entity entity, JPH::Vec3 vel);

		void StepWorld(float dt);

		void PostStepSync();

		void AddForceToEntity(Entity entity, const JPH::Vec3& force);

		void AddImpulseToEntity(Entity entity, const JPH::Vec3& impulse);

		void AddVelocityChangeToEntity(Entity entity, const JPH::Vec3& deltaVelocity);

		void AddAccelerationToEntity(Entity entity, const JPH::Vec3& acceleration);

		void SetCollisionMask(uint32_t layer, uint32_t mask);

		void SetBodyLayer(Entity entity, uint32_t layer);

		void SetObjectBroadPhaseLayer(uint32_t layer, JPH::BroadPhaseLayer bpLayer);

		JPH::uint GetNumBroadPhaseLayers();

		JPH::BroadPhaseLayer GetBroadPhaseLayer(uint32_t layer);

		glm::vec3 GetPosition(Entity entity);

		glm::quat GetRotation(Entity entity);

		glm::vec3 GetScale(Entity entity);

		int GetCollisionSteps() const;

		void SetCollisionSteps(int steps);

		JPH::ShapeRefC CreateShapeFromCollider(const ColliderShape& collider, const Transform& transform) const;

		float GetGravityFactor(Entity entity) const;

		void SetGravityFactor(Entity entity, float factor);

		void OffGravity(Entity entity, bool condition);

		bool IsGravityOff(Entity entity) const;
	};
}

#endif
