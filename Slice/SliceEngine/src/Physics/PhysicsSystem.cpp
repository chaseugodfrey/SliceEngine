/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			PhysicsSystem.cpp
 author:		Aloysius Teo
 email:			teo.k@digipen.edu
 brief:			Handles all physics

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "../Core/Core.h"
#include "PhysicsSystem.h"
#include "PhysicsDebug.h"
#include "../Graphics/TransformHelper.h"
#include "../Core/EventManager.h"
#include "../ECS/GOFactory.h"
#include "../Core/ComponentModified.h"

#define EPSILON 0.0001f

namespace SliceEngine
{


	PhysicsSystem::~PhysicsSystem()
	{
		Shutdown();
		SLICE_LOG("Physics System Shutdown");
	}

	bool PhysicsSystem::Initialize(float fixedDt, size_t tempAllocatorSize, JPH::uint maxBodies, JPH::uint numBodyMutex, JPH::uint maxContactConstraints, JPH::uint threadCount)
	{
		if (isInitialized)
		{
			return false;
		}
		try
		{
			if (threadCount == 0)
			{
				threadCount = std::thread::hardware_concurrency() - 1;
				if (threadCount == 0)
				{
					threadCount = 2;  // Fallback if hardware_concurrency() returns 0
				}
			}

			collisionSteps = static_cast<int>(ceil(fixedDt / (1.0f / 60.f)));

			//Jolt uses function pointers for memory allocation, sets up the function pointers Jolt uses internally.
			JPH::RegisterDefaultAllocator();
			SLICE_LOG("Register default allocator for Jolt Function Pointer");

			//Jolt has a global function pointer "Trace" for debugging and logging messages
			//Hook Jolt Trace to SliceEngines logger.
			JPH::Trace = JoltTraceImpl;
			JPH::JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)
				SLICE_LOG("Hook Jolt Tracer to SliceEngine Logger");

			JPH::Factory::sInstance = new JPH::Factory;
			JPH::RegisterTypes();

			tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(tempAllocatorSize);

			jobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, threadCount);

			broadphaseLayerInterface = std::make_unique<BPLayerInterfaceImpl>();
			objectVsBroadphaseLayerFilter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
			objectLayerPairFilter = std::make_unique<ObjectLayerPairFilterImpl>();

			physicsSystem = std::make_unique<JPH::PhysicsSystem>();
			physicsSystem->Init(maxBodies, numBodyMutex, maxBodies, maxContactConstraints,
				*broadphaseLayerInterface,
				*objectVsBroadphaseLayerFilter,
				*objectLayerPairFilter);

			contactListener = std::make_unique<MyContactListener>();

			physicsSystem->SetContactListener(contactListener.get());

			// Connect entt component update signals to publish modification events (need 'template' keyword because of dependent context)
			mRegistry->on_update<RigidBody>().template connect<&NotifyRigidBodyModified>();
			mRegistry->on_update<ColliderShape>().template connect<&NotifyColliderShapeModified>();

			isInitialized = true;
			SLICE_LOG("Physics System Initialized");
			return true;
		}
		catch (const std::exception& e)
		{
			const char* errorMessageCStr = e.what();
			SLICE_LOG_ERROR("Physic System failed to initalize: %s" + std::string(errorMessageCStr));
			return false;
		}
	}

	bool PhysicsSystem::IsInitialized() const { return isInitialized; }

	void PhysicsSystem::Shutdown()
	{
		if (isInitialized)
		{
			physicsSystem.reset();
			jobSystem.reset();
			// Cleanup other resources
			JPH::UnregisterTypes();
			delete JPH::Factory::sInstance;
			JPH::Factory::sInstance = nullptr;
			isInitialized = false;
		}
	}

	void PhysicsSystem::OnColliderAdd(const ColliderShapeAddedEvent& event)
	{
		std::cout << "LMOA OI ADDED Collider\n";
		physicsSystem->OptimizeBroadPhase();
	}

	void PhysicsSystem::OnColliderRemove(const ColliderShapeRemovedEvent& event)
	{
		physicsSystem->OptimizeBroadPhase();
	}

	void PhysicsSystem::OnRigidBodyAdd(const RigidBodyAddedEvent& event)
	{
		GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(event.entity);
		if (!checkEntity.HasComponent<ColliderShape>())
			return;

		auto& rigidBody = mRegistry->get<RigidBody>(event.entity);
		auto& colliderShape = mRegistry->get<ColliderShape>(event.entity);


		if (rigidBody.isKinematic)
		{
			physicsSystem->GetBodyInterface().SetMotionType(colliderShape.bodyID, JPH::EMotionType::Kinematic, JPH::EActivation::Activate);
		}
		else
		{
			physicsSystem->GetBodyInterface().SetMotionType(colliderShape.bodyID, JPH::EMotionType::Dynamic, JPH::EActivation::Activate);
		}



		//Set physics properties
		if (!rigidBody.isKinematic)
		{
			physicsSystem->GetBodyInterface().SetGravityFactor(colliderShape.bodyID, rigidBody.gravityFactor);
			physicsSystem->GetBodyInterface().SetMotionQuality(colliderShape.bodyID, rigidBody.CollisionDetection);

			physicsSystem->GetBodyInterface().SetFriction(colliderShape.bodyID, rigidBody.friction);
			physicsSystem->GetBodyInterface().SetRestitution(colliderShape.bodyID, rigidBody.restitution);

			JPH::BodyLockWrite lock(physicsSystem->GetBodyLockInterface(), colliderShape.bodyID);
			if (lock.Succeeded())
			{
				JPH::Body& body = lock.GetBody();
				JPH::MotionProperties* mp = body.GetMotionProperties();

				JPH::RefConst<JPH::Shape> shape = body.GetShape();
				JPH::MassProperties massProps = shape->GetMassProperties();

				massProps.ScaleToMass(rigidBody.mass);

				//handle freeze position
				JPH::EAllowedDOFs allowedDofs = JPH::EAllowedDOFs::None;

				if (!rigidBody.freezePosition.freezeX)
					allowedDofs |= JPH::EAllowedDOFs::TranslationX;
				if(!rigidBody.freezePosition.freezeY)
					allowedDofs |= JPH::EAllowedDOFs::TranslationY;
				if (!rigidBody.freezePosition.freezeZ)
					allowedDofs |= JPH::EAllowedDOFs::TranslationZ;

				//handle freeze rotation
				if (!rigidBody.freezeRotation.freezeX)
					allowedDofs |= JPH::EAllowedDOFs::RotationX;
				if (!rigidBody.freezeRotation.freezeY)
					allowedDofs |= JPH::EAllowedDOFs::RotationY;
				if (!rigidBody.freezeRotation.freezeZ)
					allowedDofs |= JPH::EAllowedDOFs::RotationZ;

				mp->SetMassProperties(allowedDofs, massProps);
				//mp->ScaleToMass(rigidBody.mass);
				mp->SetLinearDamping(rigidBody.linearDamping);
				mp->SetAngularDamping(rigidBody.angularDamping);

			}

		}
		if (rigidBody.isKinematic)
		{
			physicsSystem->GetBodyInterface().SetFriction(colliderShape.bodyID, rigidBody.friction);
			physicsSystem->GetBodyInterface().SetRestitution(colliderShape.bodyID, rigidBody.restitution);
		}

	}

	void PhysicsSystem::OnRigidBodyRemove(const RigidBodyRemovedEvent& event)
	{
		GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(event.entity);
		if (!checkEntity.HasComponent<ColliderShape>())
			return;
		auto& colliderShape = mRegistry->get<ColliderShape>(event.entity);

		physicsSystem->GetBodyInterface().SetMotionType(colliderShape.bodyID, JPH::EMotionType::Static, JPH::EActivation::DontActivate);

		float friction = 0.5f; // default friction value
		float restitution = 0.0f; // default restitution value

		physicsSystem->GetBodyInterface().SetFriction(colliderShape.bodyID, friction);
		physicsSystem->GetBodyInterface().SetRestitution(colliderShape.bodyID, restitution);

	}

	void PhysicsSystem::OnColliderModified(const ColliderShapeModifiedEvent& event)
	{
		auto& colliderShape = mRegistry->get<ColliderShape>(event.entity);
		auto& transform = mRegistry->get<Transform>(event.entity);
		std::variant<ColliderShape::BoxData, ColliderShape::SphereData,ColliderShape::CapsuleData> shapeData = colliderShape.shapeData;

		if (std::holds_alternative<ColliderShape::BoxData>(shapeData))
		{
			const JPH::BoxShape* boxShape = static_cast<const JPH::BoxShape*>(colliderShape.shape.GetPtr());
			JPH::Vec3 halfExtents = boxShape->GetHalfExtent();

			auto& boxData = std::get<ColliderShape::BoxData>(colliderShape.shapeData);
			JPH::Vec3 tempScale = boxData.scale * JPH::Vec3(fabs(transform.scale.x),
															fabs(transform.scale.y),
															fabs(transform.scale.z));

			if (tempScale == halfExtents) // in case there is issue look here future me
			{
				return;
			}

			JPH::BoxShapeSettings settings(tempScale);
			auto result = settings.Create();
			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to rebuild scaled box: " + std::string(result.GetError()));
				return;
			}
			colliderShape.shape = result.Get();
			// Replace shape on body if it already exists
			if (!colliderShape.bodyID.IsInvalid())
			{

				physicsSystem->GetBodyInterface().SetShape(colliderShape.bodyID, colliderShape.shape, true, JPH::EActivation::DontActivate);

				if (mRegistry->any_of<RigidBody>(event.entity))
				{
					auto& rb = mRegistry->get<RigidBody>(event.entity);
					if (!rb.isKinematic)
					{
						JPH::BodyLockWrite lock(physicsSystem->GetBodyLockInterface(), colliderShape.bodyID);
						if (lock.Succeeded())
						{
							JPH::Body& body = lock.GetBody();
							if (auto* mp = body.GetMotionProperties())
							{
								mp->ScaleToMass(rb.mass);
								mp->SetLinearDamping(rb.linearDamping);
								mp->SetAngularDamping(rb.angularDamping);
							}
						}
					}
				}
			}


		}
		else if (std::holds_alternative<ColliderShape::SphereData>(shapeData))
		{
			//if we add sphereData
			const JPH::SphereShape* sphereShape = static_cast<const JPH::SphereShape*>(colliderShape.shape.GetPtr());
			float sphereRadius = sphereShape->GetRadius();

			auto& sphereData = std::get<ColliderShape::SphereData>(colliderShape.shapeData);
			float tempScaleX = sphereData.radius * fabs(transform.scale.x);
			float tempScaleY = sphereData.radius * fabs(transform.scale.y);
			float tempScaleZ = sphereData.radius * fabs(transform.scale.z);

			if (tempScaleX == sphereRadius && tempScaleY == sphereRadius && tempScaleZ == sphereRadius)
			{
				return;
			}

			float biggestScale = std::max({ fabs(transform.scale.x), fabs(transform.scale.y), fabs(transform.scale.z) });

			JPH::SphereShapeSettings settings(sphereData.radius * fabs(biggestScale));
			auto result = settings.Create();
			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to rebuild scaled Sphere: " + std::string(result.GetError()));
				return;
			}

			colliderShape.shape = result.Get();
			// Replace shape on body if it already exists
			if (!colliderShape.bodyID.IsInvalid())
			{

				physicsSystem->GetBodyInterface().SetShape(colliderShape.bodyID, colliderShape.shape, true, JPH::EActivation::DontActivate);

				if (mRegistry->any_of<RigidBody>(event.entity))
				{
					auto& rb = mRegistry->get<RigidBody>(event.entity);
					if (!rb.isKinematic)
					{
						JPH::BodyLockWrite lock(physicsSystem->GetBodyLockInterface(), colliderShape.bodyID);
						if (lock.Succeeded())
						{
							JPH::Body& body = lock.GetBody();
							if (auto* mp = body.GetMotionProperties())
							{
								mp->ScaleToMass(rb.mass);
								mp->SetLinearDamping(rb.linearDamping);
								mp->SetAngularDamping(rb.angularDamping);
							}
						}
					}
				}
			}

		}
		else if (std::holds_alternative<ColliderShape::CapsuleData>(shapeData))
		{
			//if we add sphereData
			const JPH::CapsuleShape* capsuleShape = static_cast<const JPH::CapsuleShape*>(colliderShape.shape.GetPtr());
			float capsuleRadius = capsuleShape->GetRadius();
			float capsuleHeight = capsuleShape->GetHalfHeightOfCylinder();

			auto& capsuleData = std::get < ColliderShape::CapsuleData > (colliderShape.shapeData);
			float tempScaleX = capsuleData.radius * fabs(transform.scale.x);
			float tempScaleZ = capsuleData.radius * fabs(transform.scale.z);
			float tempScaleHeight = capsuleData.height * fabs(transform.scale.y);

			if (tempScaleX == capsuleRadius && tempScaleZ == capsuleRadius && tempScaleHeight == capsuleHeight)
			{
				return;
			}

			float biggestScaleRad = std::max({ fabs(transform.scale.x), fabs(transform.scale.z) });

			JPH::CapsuleShapeSettings settings(tempScaleHeight,capsuleData.radius * fabs(biggestScaleRad));
			auto result = settings.Create();
			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to rebuild scaled Capsule: " + std::string(result.GetError()));
				return;
			}

			colliderShape.shape = result.Get();
			// Replace shape on body if it already exists
			if (!colliderShape.bodyID.IsInvalid())
			{

				physicsSystem->GetBodyInterface().SetShape(colliderShape.bodyID, colliderShape.shape, true, JPH::EActivation::DontActivate);

				if (mRegistry->any_of<RigidBody>(event.entity))
				{
					auto& rb = mRegistry->get<RigidBody>(event.entity);
					if (!rb.isKinematic)
					{
						JPH::BodyLockWrite lock(physicsSystem->GetBodyLockInterface(), colliderShape.bodyID);
						if (lock.Succeeded())
						{
							JPH::Body& body = lock.GetBody();
							if (auto* mp = body.GetMotionProperties())
							{
								mp->ScaleToMass(rb.mass);
								mp->SetLinearDamping(rb.linearDamping);
								mp->SetAngularDamping(rb.angularDamping);
							}
						}
					}
				}
			}

		}


		if (physicsSystem->GetBodyInterface().GetObjectLayer(colliderShape.bodyID) != colliderShape.layer)
		{
			physicsSystem->GetBodyInterface().SetObjectLayer(colliderShape.bodyID, colliderShape.layer);
		}

		if (colliderShape.isTrigger && !physicsSystem->GetBodyInterface().IsSensor(colliderShape.bodyID))
		{
			physicsSystem->GetBodyInterface().SetIsSensor(colliderShape.bodyID, true);
		}
		else if (!colliderShape.isTrigger && physicsSystem->GetBodyInterface().IsSensor(colliderShape.bodyID))
		{
			physicsSystem->GetBodyInterface().SetIsSensor(colliderShape.bodyID, false);
		}

	}

	void PhysicsSystem::OnRigidBodyModified(RigidBodyModifiedEvent& event)
	{
		GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(event.entity);
		if (!checkEntity.HasComponent<ColliderShape>())
			return;

		auto& rigidBody = mRegistry->get<RigidBody>(event.entity);
		auto& colliderShape = mRegistry->get<ColliderShape>(event.entity);

		JPH::EMotionType motionType = physicsSystem->GetBodyInterface().GetMotionType(colliderShape.bodyID);
		if (rigidBody.isKinematic && motionType != JPH::EMotionType::Kinematic)
		{
			physicsSystem->GetBodyInterface().SetMotionType(colliderShape.bodyID, JPH::EMotionType::Kinematic, JPH::EActivation::Activate);
		}
		else if (!rigidBody.isKinematic && motionType != JPH::EMotionType::Dynamic)
		{
			physicsSystem->GetBodyInterface().SetMotionType(colliderShape.bodyID, JPH::EMotionType::Dynamic, JPH::EActivation::Activate);
		}

		if (physicsSystem->GetBodyInterface().GetGravityFactor(colliderShape.bodyID) != rigidBody.gravityFactor)
		{
			physicsSystem->GetBodyInterface().SetGravityFactor(colliderShape.bodyID, rigidBody.gravityFactor);
		}

		if (physicsSystem->GetBodyInterface().GetMotionQuality(colliderShape.bodyID) != rigidBody.CollisionDetection)
		{
			physicsSystem->GetBodyInterface().SetMotionQuality(colliderShape.bodyID, rigidBody.CollisionDetection);
		}

		JPH::BodyLockWrite lock(physicsSystem->GetBodyLockInterface(), colliderShape.bodyID);
		if (lock.Succeeded())
		{
			JPH::Body& body = lock.GetBody();
			JPH::MotionProperties* mp = body.GetMotionProperties();

			JPH::RefConst<JPH::Shape> shape = body.GetShape();
			JPH::MassProperties massProps = shape->GetMassProperties();

			massProps.ScaleToMass(rigidBody.mass);

			//handle freeze position
			JPH::EAllowedDOFs allowedDofs = JPH::EAllowedDOFs::None;

			if (!rigidBody.freezePosition.freezeX)
				allowedDofs |= JPH::EAllowedDOFs::TranslationX;
			if (!rigidBody.freezePosition.freezeY)
				allowedDofs |= JPH::EAllowedDOFs::TranslationY;
			if (!rigidBody.freezePosition.freezeZ)
				allowedDofs |= JPH::EAllowedDOFs::TranslationZ;

			//handle freeze rotation
			if (!rigidBody.freezeRotation.freezeX)
				allowedDofs |= JPH::EAllowedDOFs::RotationX;
			if (!rigidBody.freezeRotation.freezeY)
				allowedDofs |= JPH::EAllowedDOFs::RotationY;
			if (!rigidBody.freezeRotation.freezeZ)
				allowedDofs |= JPH::EAllowedDOFs::RotationZ;

			mp->SetMassProperties(allowedDofs, massProps);
			//mp->ScaleToMass(rigidBody.mass);
			mp->SetLinearDamping(rigidBody.linearDamping);
			mp->SetAngularDamping(rigidBody.angularDamping);
		}

		//std::cout << (int)event.entity <<"Rigidbody modified\n";
	}

	void PhysicsSystem::UpdateShapeFromTransform(Entity entity)
	{
		auto& transform = mRegistry->get<Transform>(entity);
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		std::variant<ColliderShape::BoxData, ColliderShape::SphereData, ColliderShape::CapsuleData> shapeData = colliderShape.shapeData;

		if (std::holds_alternative<ColliderShape::BoxData>(shapeData))
		{
			const JPH::BoxShape* boxShape = static_cast<const JPH::BoxShape*>(colliderShape.shape.GetPtr());
			JPH::Vec3 halfExtents = boxShape->GetHalfExtent();

			auto& boxData = std::get<ColliderShape::BoxData>(colliderShape.shapeData);
			JPH::Vec3 tempScale = boxData.scale * JPH::Vec3(fabs(transform.scale.x),
															fabs(transform.scale.y),
															fabs(transform.scale.z));

			if (tempScale == halfExtents)
				return;

			JPH::Vec3 newHalf(
				boxData.scale.GetX() * fabs(transform.scale.x),
				boxData.scale.GetY() * fabs(transform.scale.y),
				boxData.scale.GetZ() * fabs(transform.scale.z)
			);

			JPH::BoxShapeSettings settings(newHalf);
			auto result = settings.Create();
			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to rebuild scaled box: " + std::string(result.GetError()));
				return;
			}
			colliderShape.shape = result.Get();

			// Replace shape on body if it already exists
			if (!colliderShape.bodyID.IsInvalid())
			{

				physicsSystem->GetBodyInterface().SetShape(colliderShape.bodyID, colliderShape.shape, true, JPH::EActivation::DontActivate);

				if (mRegistry->any_of<RigidBody>(entity))
				{
					auto& rb = mRegistry->get<RigidBody>(entity);
					if (!rb.isKinematic)
					{
						JPH::BodyLockWrite lock(physicsSystem->GetBodyLockInterface(), colliderShape.bodyID);
						if (lock.Succeeded())
						{
							JPH::Body& body = lock.GetBody();
							if (auto* mp = body.GetMotionProperties())
							{
								mp->ScaleToMass(rb.mass);
								mp->SetLinearDamping(rb.linearDamping);
								mp->SetAngularDamping(rb.angularDamping);
							}
						}
					}
				}
			}
		}
		else if (std::holds_alternative<ColliderShape::SphereData>(shapeData))
		{
			const JPH::SphereShape* sphereShape = static_cast<const JPH::SphereShape*>(colliderShape.shape.GetPtr());
			float sphereRadius = sphereShape->GetRadius();

			auto& sphereData = std::get<ColliderShape::SphereData>(colliderShape.shapeData);
			float tempScaleX = sphereData.radius * fabs(transform.scale.x);
			float tempScaleY = sphereData.radius * fabs(transform.scale.y);
			float tempScaleZ = sphereData.radius * fabs(transform.scale.z);

			if (tempScaleX == sphereRadius && tempScaleY == sphereRadius && tempScaleZ == sphereRadius)
			{
				return;
			}

			float biggestScale = std::max({ fabs(transform.scale.x), fabs(transform.scale.y), fabs(transform.scale.z) });

			JPH::SphereShapeSettings settings(sphereData.radius * fabs(biggestScale));
			auto result = settings.Create();
			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to rebuild scaled Sphere: " + std::string(result.GetError()));
				return;
			}

			colliderShape.shape = result.Get();
			// Replace shape on body if it already exists
			if (!colliderShape.bodyID.IsInvalid())
			{

				physicsSystem->GetBodyInterface().SetShape(colliderShape.bodyID, colliderShape.shape, true, JPH::EActivation::DontActivate);

				if (mRegistry->any_of<RigidBody>(entity))
				{
					auto& rb = mRegistry->get<RigidBody>(entity);
					if (!rb.isKinematic)
					{
						JPH::BodyLockWrite lock(physicsSystem->GetBodyLockInterface(), colliderShape.bodyID);
						if (lock.Succeeded())
						{
							JPH::Body& body = lock.GetBody();
							if (auto* mp = body.GetMotionProperties())
							{
								mp->ScaleToMass(rb.mass);
								mp->SetLinearDamping(rb.linearDamping);
								mp->SetAngularDamping(rb.angularDamping);
							}
						}
					}
				}
			}
		}
		else if (std::holds_alternative<ColliderShape::CapsuleData>(shapeData))
		{
			//if we add sphereData
			const JPH::CapsuleShape* capsuleShape = static_cast<const JPH::CapsuleShape*>(colliderShape.shape.GetPtr());
			float capsuleRadius = capsuleShape->GetRadius();
			float capsuleHeight = capsuleShape->GetHalfHeightOfCylinder();

			auto& capsuleData = std::get < ColliderShape::CapsuleData >(colliderShape.shapeData);
			float tempScaleX = capsuleData.radius * fabs(transform.scale.x);
			float tempScaleZ = capsuleData.radius * fabs(transform.scale.z);
			float tempScaleHeight = capsuleData.height * fabs(transform.scale.y);

			if (tempScaleX == capsuleRadius && tempScaleZ == capsuleRadius && tempScaleHeight == capsuleHeight)
			{
				return;
			}

			float biggestScaleRad = std::max({ fabs(transform.scale.x), fabs(transform.scale.z) });

			JPH::CapsuleShapeSettings settings(tempScaleHeight, capsuleData.radius * fabs(biggestScaleRad));
			auto result = settings.Create();
			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to rebuild scaled Capsule: " + std::string(result.GetError()));
				return;
			}

			colliderShape.shape = result.Get();
			// Replace shape on body if it already exists
			if (!colliderShape.bodyID.IsInvalid())
			{

				physicsSystem->GetBodyInterface().SetShape(colliderShape.bodyID, colliderShape.shape, true, JPH::EActivation::DontActivate);

				if (mRegistry->any_of<RigidBody>(entity))
				{
					auto& rb = mRegistry->get<RigidBody>(entity);
					if (!rb.isKinematic)
					{
						JPH::BodyLockWrite lock(physicsSystem->GetBodyLockInterface(), colliderShape.bodyID);
						if (lock.Succeeded())
						{
							JPH::Body& body = lock.GetBody();
							if (auto* mp = body.GetMotionProperties())
							{
								mp->ScaleToMass(rb.mass);
								mp->SetLinearDamping(rb.linearDamping);
								mp->SetAngularDamping(rb.angularDamping);
							}
						}
					}
				}
			}

		}

	}

	JPH::ShapeRefC PhysicsSystem::CreateShapeFromCollider(const ColliderShape& collider, const Transform& transform) const
	{
		std::variant<ColliderShape::BoxData, ColliderShape::SphereData, ColliderShape::CapsuleData> shapeData = collider.shapeData;

		if (std::holds_alternative<ColliderShape::BoxData>(shapeData))
		{
			const ColliderShape::BoxData& boxData = std::get<ColliderShape::BoxData>(collider.shapeData);
			JPH::BoxShapeSettings shapeSetting(boxData.scale);

			auto result = shapeSetting.Create();

			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to get Box Data: " + std::string(result.GetError()));
				return nullptr;
			}

			return result.Get();
		}
		else if (std::holds_alternative<ColliderShape::SphereData>(shapeData))
		{
			const ColliderShape::SphereData& sphereData = std::get<ColliderShape::SphereData>(collider.shapeData);
			JPH::SphereShapeSettings shapeSetting(sphereData.radius);

			auto result = shapeSetting.Create();

			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to get Sphere Data: " + std::string(result.GetError()));
				return nullptr;
			}

			return result.Get();
		}
		else if (std::holds_alternative<ColliderShape::CapsuleData>(shapeData))
		{
			const ColliderShape::CapsuleData& capsuleData = std::get<ColliderShape::CapsuleData>(collider.shapeData);
			JPH::CapsuleShapeSettings shapeSetting(capsuleData.height, capsuleData.radius);

			auto result = shapeSetting.Create();

			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to get Capsule Data: " + std::string(result.GetError()));
				return nullptr;
			}

			return result.Get();
		}

		SLICE_LOG_ERROR("Unsupported Collider Shape");
		return nullptr;


	}

	void PhysicsSystem::SyncECSToPhysics(Transform& transform, ColliderShape& colliderShape) const
	{
		//auto motionType = physicsSystem->GetBodyInterface().GetMotionType(colliderShape.bodyID);
		//if (motionType == JPH::EMotionType::Dynamic)
		//	return; // skip, let physics handle dynamic motion

		JPH::Vec3 pos(transform.position.x, transform.position.y, transform.position.z);
		glm::quat rot = transform.rotation;//Vec3ToQuat(transform.rotation);
		JPH::Quat rotation(rot.x, rot.y, rot.z, rot.w);


		physicsSystem->GetBodyInterface().SetPosition(colliderShape.bodyID, pos, JPH::EActivation::DontActivate);
		physicsSystem->GetBodyInterface().SetRotation(colliderShape.bodyID, rotation, JPH::EActivation::DontActivate);
	}

	void PhysicsSystem::SyncPhysicsToECS(Transform& transform, ColliderShape& colliderShape) const
	{


		JPH::Vec3 pos = physicsSystem->GetBodyInterface().GetPosition(colliderShape.bodyID);
		JPH::Quat rotation = physicsSystem->GetBodyInterface().GetRotation(colliderShape.bodyID);

		//glm::vec3 rot = QuatToVec3(glm::quat(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ())); // glm store as w,x,y,z

		transform.position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());//i will create helper function for converservion of glm and jolt data types
		transform.rotation = glm::quat(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ());

	}

	void PhysicsSystem::HandleRemovedContacts()
	{
		bool pass = true;

		for (auto& bodyPair : contactListener->GetBodiesInContact())
		{
			// do this later aloysius
			JPH::BodyLockRead lock1(physicsSystem->GetBodyLockInterface(), bodyPair.GetBody1ID());

			JPH::uint64 ent1;
			JPH::uint64 ent2;

			ColliderShape colliderShape1;
			ColliderShape colliderShape2;

			if (lock1.Succeeded())
			{
				const JPH::Body& body1 = lock1.GetBody();
				ent1 = static_cast<JPH::uint64>(body1.GetUserData());
			}
			else
			{
				pass = false;
			}
			lock1.ReleaseLock();

			JPH::BodyLockRead lock2(physicsSystem->GetBodyLockInterface(), bodyPair.GetBody2ID());
			if (lock2.Succeeded())
			{
				const JPH::Body& body2 = lock2.GetBody();
				ent2 = static_cast<JPH::uint64>(body2.GetUserData());
			}
			else
			{
				pass = false;
			}
			lock2.ReleaseLock();

			if (pass = true)
			{
				GameObject checkEntity1 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(ent1));
				GameObject checkEntity2 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(ent2));
				colliderShape1 = checkEntity1.GetComponent<ColliderShape>();
				colliderShape2 = checkEntity2.GetComponent<ColliderShape>();

				if (colliderShape1.isTrigger || colliderShape2.isTrigger)
				{
					OnTriggerExitEvent triggerEvent1;
					OnTriggerExitEvent triggerEvent2;

					triggerEvent1.entity = checkEntity1.GetEntity();
					triggerEvent1.other = checkEntity2.GetEntity();

					triggerEvent2.entity = checkEntity2.GetEntity();
					triggerEvent2.other = checkEntity1.GetEntity();

					EventManager::GetInstance()->Publish<OnTriggerExitEvent>(triggerEvent1);
					EventManager::GetInstance()->Publish<OnTriggerExitEvent>(triggerEvent2);
				}
				else
				{
					OnCollisionExitEvent collisionEvent1;
					OnCollisionExitEvent collisionEvent2;

					collisionEvent1.entity = checkEntity1.GetEntity();
					collisionEvent1.other = checkEntity2.GetEntity();

					collisionEvent2.entity = checkEntity2.GetEntity();
					collisionEvent2.other = checkEntity1.GetEntity();

					EventManager::GetInstance()->Publish<OnCollisionExitEvent>(collisionEvent1);
					EventManager::GetInstance()->Publish<OnCollisionExitEvent>(collisionEvent2);

				}
			}
			else
			{
				pass = true;
				continue;
			}
		}
		contactListener->clearBodiesInContact();
	}

	void PhysicsSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		auto& transform = reg.get<Transform>(entity);
		auto& colliderShape = reg.get<ColliderShape>(entity);

		bool isRigibody = false;

		GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(entity);
		if (checkEntity.HasComponent<RigidBody>())
		{
			isRigibody = true;
		}

		//Create shape based on collider
		JPH::ShapeRefC shape = CreateShapeFromCollider(colliderShape,transform);
		if (!shape)
		{
			SLICE_LOG_ERROR("Failed to create Shape for entity");
			return;
		}
		colliderShape.shape = shape;
		//Convert transform data
		JPH::Vec3 position(transform.position.x, transform.position.y, transform.position.z);
		glm::quat rot = transform.rotation;//Vec3ToQuat(transform.rotation);
		JPH::Quat rotation(rot.x, rot.y, rot.z, rot.w);

		JPH::BodyCreationSettings bodySettings;

		//Create body
		if (isRigibody)
		{
			auto& rigidBody = reg.get<RigidBody>(entity);
			if (rigidBody.isKinematic)
			{
				bodySettings = JPH::BodyCreationSettings(shape, position, rotation, JPH::EMotionType::Kinematic, colliderShape.layer);
			}
			else
			{
				bodySettings = JPH::BodyCreationSettings(shape, position, rotation, JPH::EMotionType::Dynamic, colliderShape.layer);
			}

			//Set physics properties
			if (!rigidBody.isKinematic)
			{
				bodySettings.mGravityFactor = rigidBody.gravityFactor;
				bodySettings.mMotionQuality = rigidBody.CollisionDetection;
				//bodySettings.mMassPropertiesOverride.mMass = rigidBody.mass;
				bodySettings.mFriction = rigidBody.friction;
				bodySettings.mRestitution = rigidBody.restitution;
				bodySettings.mLinearDamping = rigidBody.linearDamping;
				bodySettings.mAngularDamping = rigidBody.angularDamping;
			}
			if (rigidBody.isKinematic)
			{
				bodySettings.mFriction = rigidBody.friction;
				bodySettings.mRestitution = rigidBody.restitution;
			}
		}
		else if (!isRigibody)
		{
			bodySettings = JPH::BodyCreationSettings(shape, position, rotation, JPH::EMotionType::Static, colliderShape.layer);
		}

		//Set as sensor for triggers
		if (colliderShape.isTrigger)
		{
			bodySettings.mIsSensor = true;
		}

		bodySettings.mAllowDynamicOrKinematic = true; // allow changing motion type at runtime

		//Store entity ID in user data for collision callbacks
		bodySettings.mUserData = static_cast<uint64_t>(entity);

		//Create and add the body
		JPH::Body* body = physicsSystem->GetBodyInterface().CreateBody(bodySettings);
		if (!body)
		{
			SLICE_LOG_ERROR("Failed to create Jolt body for entity");
			return;
		}

		//Add to physics world and store bodyID in rigidbody
		colliderShape.bodyID = body->GetID();
		physicsSystem->GetBodyInterface().AddBody(colliderShape.bodyID, isRigibody ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);

		SLICE_LOG("Created Jolt body with ID: " + std::to_string(colliderShape.bodyID.GetIndexAndSequenceNumber()));
		physicsSystem->OptimizeBroadPhase();
	}

	void PhysicsSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{

		auto& colliderShape = reg.get<ColliderShape>(entity);

		// Remove body form physics world
		physicsSystem->GetBodyInterface().RemoveBody(colliderShape.bodyID);

		// Destroy the body from the physics world
		physicsSystem->GetBodyInterface().DestroyBody(colliderShape.bodyID);

		physicsSystem->OptimizeBroadPhase();
	}

	void PhysicsSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{

		//if (entity == static_cast<Entity>(6U));
		auto& transform = reg.get<Transform>(entity);
		auto& colliderShape = reg.get<ColliderShape>(entity);

		UpdateShapeFromTransform(entity);

		SyncECSToPhysics(transform, colliderShape);
		//physicsSystem->Update(dt, collisionSteps, tempAllocator.get(), jobSystem.get());
		//SyncPhysicsToECS(transform, colliderShape);

		//HandleRemovedContacts();

		
	}

	void PhysicsSystem::StepWorld(float dt)
	{
		physicsSystem->Update(dt, collisionSteps, tempAllocator.get(), jobSystem.get());
	}

	void PhysicsSystem::PostStepSync()
	{
		HandleRemovedContacts();

		auto view = mRegistry->view<Transform, ColliderShape>();

		// Safe, iterator-free iteration
		for (auto [e, t, c] : view.each())
		{
			SyncPhysicsToECS(t, c);
		}
	}

	void PhysicsSystem::SubscribeToEvents()
	{
		// Get the EventManager instance and subscribe our member functions.
		auto* eventManager = EventManager::GetInstance();

		// Subscribe to the ColliderShapeAddedEvent
		eventManager->Subscribe<ColliderShapeAddedEvent, &PhysicsSystem::OnColliderAdd>(this);

		// Subscribe to the ColliderShapeRemovedEvent
		eventManager->Subscribe<ColliderShapeRemovedEvent, &PhysicsSystem::OnColliderRemove>(this);

		// Subscribe to the RigidBodyAddedEvent
		eventManager->Subscribe<RigidBodyAddedEvent, &PhysicsSystem::OnRigidBodyAdd>(this);

		// Subscribe to the RigidBodyRemovedEvent
		eventManager->Subscribe<RigidBodyRemovedEvent, &PhysicsSystem::OnRigidBodyRemove>(this);

		eventManager->Subscribe<ColliderShapeModifiedEvent, &PhysicsSystem::OnColliderModified>(this);

		eventManager->Subscribe<RigidBodyModifiedEvent, &PhysicsSystem::OnRigidBodyModified>(this);

	}

	glm::vec3 PhysicsSystem::GetLinearVelocity(Entity entity)
	{
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		JPH::Vec3 vel = physicsSystem->GetBodyInterface().GetLinearVelocity(colliderShape.bodyID);

		glm::vec3 velocity(vel.GetX(), vel.GetY(), vel.GetZ());

		return velocity;
	}


	void PhysicsSystem::SetLinearVelocity(Entity entity, JPH::Vec3 vel)
	{
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		physicsSystem->GetBodyInterface().SetLinearVelocity(colliderShape.bodyID, vel);
	}

}
