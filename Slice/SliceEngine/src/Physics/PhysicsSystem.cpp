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
#include <glm/gtx/matrix_decompose.hpp>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>

#define EPSILON 0.0001f
#define GLM_ENABLE_EXPERIMENTAL


namespace SliceEngine
{
	namespace helpers
	{
		glm::vec3 JPHtoglm(JPH::Vec3 vec)
		{
			return glm::vec3(vec.GetX(), vec.GetY(), vec.GetZ());
		}

		JPH::Vec3 glmtoJPH(glm::vec3 vec)
		{
			return JPH::Vec3(vec.x, vec.y, vec.z);
		}
	}

	PhysicsSystem::~PhysicsSystem()
	{
		Shutdown();
		SLICE_LOG("Physics System Shutdown");
	}

	bool PhysicsSystem::Initialize( size_t tempAllocatorSize, JPH::uint maxBodies, JPH::uint numBodyMutex, JPH::uint maxContactConstraints)
	{
		if (isInitialized)
		{
			return false;
		}
		try
		{
			//if (threadCount == 0)
			//{
			//	threadCount = std::thread::hardware_concurrency() - 1;
			//	if (threadCount == 0)
			//	{
			//		threadCount = 2;  // Fallback if hardware_concurrency() returns 0
			//	}
			//}

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

			jobSystem = std::make_unique<JPH::JobSystemSingleThreaded>(JPH::cMaxPhysicsJobs);

			broadphaseLayerInterface = std::make_unique<BPLayerInterfaceImpl>();
			objectVsBroadphaseLayerFilter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>(broadphaseLayerInterface.get());
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
			//mRegistry->on_update<ColliderShape>().template connect<&NotifyColliderShapeModified>();
			mRegistry->on_update<ColliderShape>().connect<&PhysicsSystem::OnColliderModified>(this);

			mRegistry->on_construct<InactiveEntity>().connect<&PhysicsSystem::OnEntityDisabled>(this);
			mRegistry->on_destroy<InactiveEntity>().connect<&PhysicsSystem::OnEntityEnabled>(this);

			mRegistry->on_destroy<ColliderShape>().connect<&PhysicsSystem::OnColliderRemove>(this);

			mRegistry->on_update<SliceEntity>().connect<&PhysicsSystem::OnSliceEntityModified>(this);

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
		physicsSystem->OptimizeBroadPhase();
	}

	void PhysicsSystem::OnColliderRemove(entt::registry& reg, entt::entity entity)
	{
		auto& colliderShape = reg.get<ColliderShape>(entity);

		if (!colliderShape.componentEnabled || reg.any_of<InactiveEntity>(entity) || colliderShape.bodyID.IsInvalid())
			return;

		if (physicsSystem->GetBodyInterface().IsAdded(colliderShape.bodyID))
		{
			// Remove body form physics world
			physicsSystem->GetBodyInterface().RemoveBody(colliderShape.bodyID);

			// Destroy the body from the physics world
			physicsSystem->GetBodyInterface().DestroyBody(colliderShape.bodyID);

			isBroadPhaseDirty = true;
		}
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

		//Temp fix for gravity factor and motion quality
		physicsSystem->GetBodyInterface().SetGravityFactor(colliderShape.bodyID, rigidBody.gravityFactor);
		physicsSystem->GetBodyInterface().SetMotionQuality(colliderShape.bodyID, rigidBody.CollisionDetection);

		physicsSystem->GetBodyInterface().SetFriction(colliderShape.bodyID, rigidBody.friction);
		physicsSystem->GetBodyInterface().SetRestitution(colliderShape.bodyID, rigidBody.restitution);
		//Temp fix for mass properties

		//Set physics properties
		if (!rigidBody.isKinematic)
		{


			JPH::BodyLockWrite lock(physicsSystem->GetBodyLockInterface(), colliderShape.bodyID);
			if (lock.Succeeded())
			{
				JPH::Body& body = lock.GetBody();
				JPH::MotionProperties* mp = body.GetMotionProperties();

				JPH::RefConst<JPH::Shape> shape = body.GetShape();
				JPH::MassProperties massProps = shape->GetMassProperties();

				massProps.ScaleToMass(rigidBody.mass);

				//handle freeze position
				JPH::EAllowedDOFs allowedDofs = AllowedDOFs(rigidBody);

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
			JPH::BodyLockWrite lock(physicsSystem->GetBodyLockInterface(), colliderShape.bodyID);
			if (lock.Succeeded())
			{
				JPH::Body& body = lock.GetBody();
				body.SetCollideKinematicVsNonDynamic(true);

			}
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

	// componeent enable check
	void PhysicsSystem::OnColliderModified(entt::registry& reg, entt::entity entity)
	{
		GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(entity);
		if (!checkEntity.HasComponent<ColliderShape>())
			return;

		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		auto& transform = mRegistry->get<Transform>(entity);
		//auto& slice = mRegistry->get<SliceEntity>(event.entity);

		if(colliderShape.shape == nullptr)
			return;

		if (colliderShape.componentEnabled && !mRegistry->any_of<InactiveEntity>(entity))
		{
			if (colliderShape.bodyID.IsInvalid())
			{
				CreateJoltBody(entity);
			}
		}
		else if (!colliderShape.componentEnabled && !mRegistry->any_of<InactiveEntity>(entity))
		{
			if (!colliderShape.bodyID.IsInvalid())
			{
				DeleteJoltBody(entity);
			}
		}

		sliceEngineVariantShape shapeData = colliderShape.shapeData;

		if (colliderShape.isTrigger && !physicsSystem->GetBodyInterface().IsSensor(colliderShape.bodyID))
		{
			physicsSystem->GetBodyInterface().SetIsSensor(colliderShape.bodyID, true);
		}
		else if (!colliderShape.isTrigger && physicsSystem->GetBodyInterface().IsSensor(colliderShape.bodyID))
		{
			physicsSystem->GetBodyInterface().SetIsSensor(colliderShape.bodyID, false);
		}

		if (std::holds_alternative<ColliderShape::BoxData>(shapeData))
		{
			const JPH::Shape* shape = colliderShape.shape.GetPtr();
			const JPH::RotatedTranslatedShape* wrappedShape = static_cast<const JPH::RotatedTranslatedShape*>(shape);
			const JPH::BoxShape* boxShape = static_cast<const JPH::BoxShape*>(wrappedShape->GetInnerShape());

			JPH::Vec3 halfExtents = boxShape->GetHalfExtent();

			auto& boxData = std::get<ColliderShape::BoxData>(colliderShape.shapeData);

			JPH::Vec3 scl = helpers::glmtoJPH(transform.GetWorldScale());
			JPH::Vec3 tempScale = boxData.scale * scl;


			////std::cout << "halfExtends<" << halfExtents.GetX() << "," << halfExtents.GetY() << "," << halfExtents.GetZ()  << ">" << std::endl;
			////std::cout << "tempScale<" << tempScale.GetX() << "," << tempScale.GetY() << "," << tempScale.GetZ() << ">" << std::endl;
			if ((tempScale == halfExtents) && (colliderShape.offSet == colliderShape.prevOffSet)) // in case there is issue look here future me
			{
				return;
			}

			// Scale the offset by the transform's world scale
			JPH::Vec3 scaledOffset(
				colliderShape.offSet.GetX() * scl.GetX(),
				colliderShape.offSet.GetY() * scl.GetY(),
				colliderShape.offSet.GetZ() * scl.GetZ()
			);
			// ensure minimum size for each dimension
			const float minSize = JPH::cDefaultConvexRadius * 2.0f; //min size just in case
			tempScale.SetX(JPH::max(tempScale.GetX(), minSize));
			tempScale.SetY(JPH::max(tempScale.GetY(), minSize));
			tempScale.SetZ(JPH::max(tempScale.GetZ(), minSize));

			JPH::BoxShapeSettings* settings = new JPH::BoxShapeSettings(tempScale);
			JPH::RotatedTranslatedShapeSettings newShape = JPH::RotatedTranslatedShapeSettings(
																	scaledOffset,
																	JPH::Quat::sIdentity(),
																	settings);

			auto result = newShape.Create();
			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to rebuild scaled box: " + std::string(result.GetError()));
				return;
			}

			colliderShape.prevOffSet = colliderShape.offSet;
			colliderShape.shape = result.Get();
			// Replace shape on body if it already exists
			if (!colliderShape.bodyID.IsInvalid())
			{
				physicsSystem->GetBodyInterface().SetShape(colliderShape.bodyID, colliderShape.shape, true, JPH::EActivation::DontActivate);

				// end debug later delete
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
			const JPH::Shape* shape = colliderShape.shape.GetPtr();
			const JPH::RotatedTranslatedShape* wrappedShape = static_cast<const JPH::RotatedTranslatedShape*>(shape);
			const JPH::SphereShape* sphereShape = static_cast<const JPH::SphereShape*>(wrappedShape->GetInnerShape());

			float sphereRadius = sphereShape->GetRadius();

			auto& sphereData = std::get<ColliderShape::SphereData>(colliderShape.shapeData);
			float tempScaleX = sphereData.radius * fabs(transform.scale.x);
			float tempScaleY = sphereData.radius * fabs(transform.scale.y);
			float tempScaleZ = sphereData.radius * fabs(transform.scale.z);

			if (tempScaleX == sphereRadius && tempScaleY == sphereRadius && tempScaleZ == sphereRadius && colliderShape.offSet == colliderShape.prevOffSet)
			{
				return;
			}

			float biggestScale = std::max({ fabs(transform.scale.x), fabs(transform.scale.y), fabs(transform.scale.z) });

			JPH::Vec3 scaledOffset(
				colliderShape.offSet.GetX() * tempScaleX,
				colliderShape.offSet.GetY() * tempScaleY,
				colliderShape.offSet.GetZ() * tempScaleZ
			);

			JPH::SphereShapeSettings* settings = new JPH::SphereShapeSettings(sphereData.radius * fabs(biggestScale));
			JPH::RotatedTranslatedShapeSettings newShape = JPH::RotatedTranslatedShapeSettings(
																		scaledOffset,
																		JPH::Quat::sIdentity(),
																		settings);
			auto result = newShape.Create();
			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to rebuild scaled Sphere: " + std::string(result.GetError()));
				return;
			}
			colliderShape.prevOffSet = colliderShape.offSet;
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
			const JPH::Shape* shape = colliderShape.shape.GetPtr();
			const JPH::RotatedTranslatedShape* wrappedShape = static_cast<const JPH::RotatedTranslatedShape*>(shape);
			const JPH::CapsuleShape* capsuleShape = static_cast<const JPH::CapsuleShape*>(wrappedShape->GetInnerShape());

			float capsuleRadius = capsuleShape->GetRadius();
			float capsuleHeight = capsuleShape->GetHalfHeightOfCylinder();

			auto& capsuleData = std::get < ColliderShape::CapsuleData > (colliderShape.shapeData);
			float tempScaleX = capsuleData.radius * fabs(transform.scale.x);
			float tempScaleZ = capsuleData.radius * fabs(transform.scale.z);
			float tempScaleHeight = capsuleData.height * fabs(transform.scale.y);

			if (tempScaleX == capsuleRadius && tempScaleZ == capsuleRadius && tempScaleHeight == capsuleHeight && colliderShape.offSet == colliderShape.prevOffSet)
			{
				return;
			}

			float biggestScaleRad = std::max({ fabs(transform.scale.x), fabs(transform.scale.z) });

			JPH::Vec3 scaledOffset(
				colliderShape.offSet.GetX()* tempScaleX,
				colliderShape.offSet.GetY()* tempScaleHeight,
				colliderShape.offSet.GetZ()* tempScaleZ
			);

			JPH::CapsuleShapeSettings *settings = new JPH::CapsuleShapeSettings(tempScaleHeight,capsuleData.radius * fabs(biggestScaleRad));
			JPH::RotatedTranslatedShapeSettings newShape = JPH::RotatedTranslatedShapeSettings(
																			scaledOffset,
																			JPH::Quat::sIdentity(),
																			settings);
			auto result = newShape.Create();
			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to rebuild scaled Capsule: " + std::string(result.GetError()));
				return;
			}
			colliderShape.prevOffSet = colliderShape.offSet;
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
		else if (std::holds_alternative<ColliderShape::MeshData>(shapeData))
		{
			return;
		}
		else if (std::holds_alternative<ColliderShape::CylinderData>(shapeData))
		{
			const JPH::Shape* shape = colliderShape.shape.GetPtr();
			const JPH::RotatedTranslatedShape* wrappedShape = static_cast<const JPH::RotatedTranslatedShape*>(shape);
			const JPH::CylinderShape* cylinderShape = static_cast<const JPH::CylinderShape*>(wrappedShape->GetInnerShape());

			float cylinderRadius = cylinderShape->GetRadius();
			float cylinderHeight = cylinderShape->GetHalfHeight();

			auto& cylinderData = std::get < ColliderShape::CylinderData >(colliderShape.shapeData);
			float tempScaleX = cylinderData.radius * fabs(transform.scale.x);
			float tempScaleZ = cylinderData.radius * fabs(transform.scale.z);
			float tempScaleHeight = cylinderData.height * fabs(transform.scale.y);

			if (tempScaleX == cylinderRadius && tempScaleZ == cylinderRadius && tempScaleHeight == cylinderHeight && colliderShape.offSet == colliderShape.prevOffSet)
			{
				return;
			}

			float biggestScaleRad = std::max({ fabs(transform.scale.x), fabs(transform.scale.z) });

			JPH::Vec3 scaledOffset(
				colliderShape.offSet.GetX()* tempScaleX,
				colliderShape.offSet.GetY()* tempScaleHeight,
				colliderShape.offSet.GetZ()* tempScaleZ
			);

			JPH::CylinderShapeSettings* settings = new JPH::CylinderShapeSettings(tempScaleHeight, cylinderData.radius * fabs(biggestScaleRad));
			JPH::RotatedTranslatedShapeSettings newShape = JPH::RotatedTranslatedShapeSettings(
				scaledOffset,
				JPH::Quat::sIdentity(),
				settings);


			auto result = newShape.Create();
			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to rebuild scaled Cylinder: " + std::string(result.GetError()));
				return;
			}
			colliderShape.prevOffSet = colliderShape.offSet;
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

	void PhysicsSystem::OnRigidBodyModified(RigidBodyModifiedEvent& event)
	{
		GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(event.entity);
		if (!checkEntity.HasComponent<ColliderShape>())
			return;

		auto& rigidBody = mRegistry->get<RigidBody>(event.entity);
		auto& colliderShape = mRegistry->get<ColliderShape>(event.entity);

		JPH::EMotionType motionType = physicsSystem->GetBodyInterface().GetMotionType(colliderShape.bodyID);

		physicsSystem->GetBodyInterface().SetFriction(colliderShape.bodyID, rigidBody.friction);
		physicsSystem->GetBodyInterface().SetRestitution(colliderShape.bodyID, rigidBody.restitution);

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
			body.SetCollideKinematicVsNonDynamic(true);

			//handle freeze position
			JPH::EAllowedDOFs allowedDofs = AllowedDOFs(rigidBody);

			mp->SetMassProperties(allowedDofs, massProps);
			//mp->ScaleToMass(rigidBody.mass);
			mp->SetLinearDamping(rigidBody.linearDamping);
			mp->SetAngularDamping(rigidBody.angularDamping);
		}

		////std::cout << (int)event.entity <<"Rigidbody modified\n";
	}

	//void PhysicsSystem::OnEntityEnabled(entt::registry& reg, entt::entity entity)
	//{
	//	GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(event.entity);
	//	if (!checkEntity.HasComponent<ColliderShape>())
	//		return;

	//	auto& slice = mRegistry->get<SliceEntity>(event.entity);
	//	auto& colliderShape = mRegistry->get<ColliderShape>(event.entity);

	//	if (slice.mActive && colliderShape.componentEnabled)
	//	{
	//		if (physicsSystem->GetBodyInterface().GetObjectLayer(colliderShape.bodyID) != slice.mLayer)
	//		{
	//			physicsSystem->GetBodyInterface().SetObjectLayer(colliderShape.bodyID, slice.mLayer);
	//		}
	//	}
	//	if(!slice.mActive)
	//	{
	//		physicsSystem->GetBodyInterface().SetObjectLayer(colliderShape.bodyID, Layers::COLLISION_OFF);
	//	}


	void PhysicsSystem::OnEntityEnabled(entt::registry& reg, entt::entity entity)
	{
		if (!reg.any_of<SliceEntity>(entity) || !reg.any_of<ColliderShape>(entity))
		{
			return;
		}

		auto& colliderShape = reg.get<ColliderShape>(entity);
		//auto& slice = reg.get<SliceEntity>(entity);


		if (colliderShape.componentEnabled)
		{
			if (colliderShape.bodyID.IsInvalid())
			{
				CreateJoltBody(entity);
			}
		}
		
	}

	void  PhysicsSystem::OnEntityDisabled(entt::registry& reg, entt::entity entity)
	{
		if (!reg.any_of<SliceEntity>(entity) || !reg.any_of<ColliderShape>(entity))
		{
			return;
		}
		//auto& slice = reg.get<SliceEntity>(entity);
		auto& colliderShape = reg.get<ColliderShape>(entity);

		if (colliderShape.componentEnabled)
		{
			if (!colliderShape.bodyID.IsInvalid())
			{
				DeleteJoltBody(entity);
			}
		}
		
	}

	void PhysicsSystem::OnSliceEntityModified(entt::registry& reg, entt::entity entity)
	{
		if (!reg.any_of<SliceEntity>(entity) || !reg.any_of<ColliderShape>(entity))
		{
			return;
		}

		auto& colliderShape = reg.get<ColliderShape>(entity);
		auto& slice = reg.get<SliceEntity>(entity);

		if (!physicsSystem->GetBodyInterface().IsAdded(colliderShape.bodyID))
		{
			return;
		}

		if (physicsSystem->GetBodyInterface().GetObjectLayer(colliderShape.bodyID) != slice.mLayer)
		{
			physicsSystem->GetBodyInterface().SetObjectLayer(colliderShape.bodyID, slice.mLayer);
		}

	}

	void PhysicsSystem::UpdateShapeFromTransform(Entity entity)
	{
		auto& transform = mRegistry->get<Transform>(entity);
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		sliceEngineVariantShape shapeData = colliderShape.shapeData;

		if (std::holds_alternative<ColliderShape::BoxData>(shapeData))
		{
			const JPH::RotatedTranslatedShape* wrappedShape = static_cast<const JPH::RotatedTranslatedShape*>(colliderShape.shape.GetPtr());
			const JPH::BoxShape* boxShape = static_cast<const JPH::BoxShape*>(wrappedShape->GetInnerShape());
			JPH::Vec3 halfExtents = boxShape->GetHalfExtent();
			JPH::Vec3 scl = helpers::glmtoJPH(transform.GetWorldScale());

			auto& boxData = std::get<ColliderShape::BoxData>(colliderShape.shapeData);
			JPH::Vec3 tempScale = boxData.scale * scl;

			if (tempScale == halfExtents)
				return;

			//JPH::Vec3 newHalf(boxData.scale * scl);

			// ensure minimum size for each dimension
			const float minSize = JPH::cDefaultConvexRadius * 2.0f; //min size just in case
			tempScale.SetX(JPH::max(tempScale.GetX(), minSize));
			tempScale.SetY(JPH::max(tempScale.GetY(), minSize));
			tempScale.SetZ(JPH::max(tempScale.GetZ(), minSize));

			JPH::Vec3 scaledOffset(
				colliderShape.offSet.GetX() * scl.GetX(),
				colliderShape.offSet.GetY() * scl.GetY(),
				colliderShape.offSet.GetZ() * scl.GetZ()
			);

			//JPH::BoxShapeSettings *settings = new JPH::BoxShapeSettings(newHalf);
			JPH::BoxShapeSettings *settings = new JPH::BoxShapeSettings(tempScale);
			JPH::RotatedTranslatedShapeSettings newShape = JPH::RotatedTranslatedShapeSettings(
				scaledOffset,
				JPH::Quat::sIdentity(),
				settings);

			auto result = newShape.Create();
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
			const JPH::RotatedTranslatedShape* wrappedShape = static_cast<const JPH::RotatedTranslatedShape*>(colliderShape.shape.GetPtr());
			const JPH::SphereShape* sphereShape = static_cast<const JPH::SphereShape*>(wrappedShape->GetInnerShape());
			float sphereRadius = sphereShape->GetRadius();

			auto& sphereData = std::get<ColliderShape::SphereData>(colliderShape.shapeData);
			JPH::Vec3 scl = helpers::glmtoJPH(transform.GetWorldScale());
			float tempScaleX = sphereData.radius * fabs(scl.GetX());
			float tempScaleY = sphereData.radius * fabs(scl.GetY());
			float tempScaleZ = sphereData.radius * fabs(scl.GetZ());

			if (tempScaleX == sphereRadius && tempScaleY == sphereRadius && tempScaleZ == sphereRadius)
			{
				return;
			}

			float biggestScale = std::max({ fabs(scl.GetX()), fabs(scl.GetY()), fabs(scl.GetZ()) });

			JPH::Vec3 scaledOffset(
				colliderShape.offSet.GetX() * scl.GetX(),
				colliderShape.offSet.GetY() * scl.GetY(),
				colliderShape.offSet.GetZ() * scl.GetZ()
			);

			JPH::SphereShapeSettings *settings = new JPH::SphereShapeSettings(sphereData.radius * fabs(biggestScale));
			JPH::RotatedTranslatedShapeSettings newShape = JPH::RotatedTranslatedShapeSettings(
				scaledOffset,
				JPH::Quat::sIdentity(),
				settings);

			auto result = newShape.Create();
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
			JPH::Vec3 scl = helpers::glmtoJPH(transform.GetWorldScale());

			float tempScaleX = capsuleData.radius * fabs(scl.GetX());
			float tempScaleZ = capsuleData.radius * fabs(scl.GetZ());
			float tempScaleHeight = capsuleData.height * fabs(scl.GetY());

			if (tempScaleX == capsuleRadius && tempScaleZ == capsuleRadius && tempScaleHeight == capsuleHeight)
			{
				return;
			}

			float biggestScaleRad = std::max({ fabs(scl.GetX()), fabs(scl.GetZ()) });

			JPH::Vec3 scaledOffset(
				colliderShape.offSet.GetX()* scl.GetX(),
				colliderShape.offSet.GetY()* scl.GetY(),
				colliderShape.offSet.GetZ()* scl.GetZ()
			);

			JPH::CapsuleShapeSettings *settings = new JPH::CapsuleShapeSettings(tempScaleHeight, capsuleData.radius * fabs(biggestScaleRad));
			JPH::RotatedTranslatedShapeSettings newShape = JPH::RotatedTranslatedShapeSettings(
				scaledOffset,
				JPH::Quat::sIdentity(),
				settings);

			auto result = newShape.Create();
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
		else if (std::holds_alternative<ColliderShape::MeshData>(shapeData))
		{
			// Create new scaled shape
			JPH::Vec3 newScale = helpers::glmtoJPH(transform.GetWorldScale());
			JPH::ScaledShapeSettings scaledSettings(colliderShape.shape, newScale);
			JPH::ShapeRefC finalShape = scaledSettings.Create().Get();

			// Actually set it on the body
			physicsSystem->GetBodyInterface().SetShape(colliderShape.bodyID, finalShape, false, JPH::EActivation::Activate);
		}
		else if (std::holds_alternative<ColliderShape::CylinderData>(shapeData))
		{
			const JPH::Shape* shape = colliderShape.shape.GetPtr();
			const JPH::RotatedTranslatedShape* wrappedShape = static_cast<const JPH::RotatedTranslatedShape*>(shape);
			const JPH::CylinderShape* cylinderShape = static_cast<const JPH::CylinderShape*>(wrappedShape->GetInnerShape());

			float cylinderRadius = cylinderShape->GetRadius();
			float cylinderHeight = cylinderShape->GetHalfHeight();
			JPH::Vec3 scl = helpers::glmtoJPH(transform.GetWorldScale());

			auto& cylinderData = std::get < ColliderShape::CylinderData >(colliderShape.shapeData);
			float tempScaleX = cylinderData.radius * fabs(scl.GetX());
			float tempScaleZ = cylinderData.radius * fabs(scl.GetZ());
			float tempScaleHeight = cylinderData.height * fabs(scl.GetY());

			if (tempScaleX == cylinderRadius && tempScaleZ == cylinderRadius && tempScaleHeight == cylinderHeight && colliderShape.offSet == colliderShape.prevOffSet)
			{
				return;
			}

			float biggestScaleRad = std::max({ fabs(scl.GetX()), fabs(scl.GetZ()) });

			JPH::Vec3 scaledOffset(
				colliderShape.offSet.GetX()* scl.GetX(),
				colliderShape.offSet.GetY()* scl.GetY(),
				colliderShape.offSet.GetZ()* scl.GetZ()
			);

			JPH::CylinderShapeSettings* settings = new JPH::CylinderShapeSettings(tempScaleHeight, cylinderData.radius * fabs(biggestScaleRad));
			JPH::RotatedTranslatedShapeSettings newShape = JPH::RotatedTranslatedShapeSettings(
				scaledOffset,
				JPH::Quat::sIdentity(),
				settings);

			auto result = newShape.Create();
			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to rebuild scaled Cylinder: " + std::string(result.GetError()));
				return;
			}
			colliderShape.prevOffSet = colliderShape.offSet;
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

	JPH::ShapeRefC PhysicsSystem::CreateShapeFromCollider(Entity entity) const
	{
		auto& collider = mRegistry->get<ColliderShape>(entity);

		sliceEngineVariantShape shapeData = collider.shapeData;

		JPH::ShapeRefC shapeReference = nullptr;

		if (std::holds_alternative<ColliderShape::BoxData>(shapeData))
		{
			shapeReference = CreateBoxShape(collider);
		}
		else if (std::holds_alternative<ColliderShape::SphereData>(shapeData))
		{
			shapeReference = CreateSphereShape(collider);
		}
		else if (std::holds_alternative<ColliderShape::CapsuleData>(shapeData))
		{
			shapeReference = CreateCapsuleShape(collider);
		}
		else if (std::holds_alternative<ColliderShape::MeshData>(shapeData))
		{
			auto& renderer = mRegistry->get<Renderer>(entity);
			shapeReference = CreateMeshShape(renderer);
		}
		else if(std::holds_alternative<ColliderShape::CylinderData>(shapeData))
		{
			shapeReference = CreateCylinderShape(collider);
		}
		else
		{ 
			SLICE_LOG_ERROR("Unsupported Collider Shape");
		}
		
		return shapeReference;

	}

	void PhysicsSystem::SyncECSToPhysics(Transform& transform, ColliderShape& colliderShape) const
	{
		//JPH::Vec3 pos(transform.position.x, transform.position.y, transform.position.z);
		//glm::quat rot = transform.rotation;//Vec3ToQuat(transform.rotation);
		//JPH::Quat rotation(rot.x, rot.y, rot.z, rot.w);

		glm::vec3 pos = transform.GetWorldPosition();
		glm::quat rot = transform.GetWorldRotation();

		JPH::Vec3 jph_pos{ pos.x, pos.y, pos.z };
		JPH::Quat jph_rot{ rot.x, rot.y, rot.z, rot.w };

		physicsSystem->GetBodyInterface().SetPosition(colliderShape.bodyID, jph_pos, JPH::EActivation::DontActivate);
		physicsSystem->GetBodyInterface().SetRotation(colliderShape.bodyID, jph_rot.Normalized(), JPH::EActivation::DontActivate);
	}

	void PhysicsSystem::SyncPhysicsToECS(Transform& transform, ColliderShape& colliderShape) const
	{

		JPH::Vec3 pos = physicsSystem->GetBodyInterface().GetPosition(colliderShape.bodyID);
		JPH::Quat rotation = physicsSystem->GetBodyInterface().GetRotation(colliderShape.bodyID);

		//glm::vec3 rot = QuatToVec3(glm::quat(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ())); // glm store as w,x,y,z

		//transform.position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());//i will create helper function for converservion of glm and jolt data types
		//transform.rotation = glm::quat(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ());
		
		transform.transform = 
			glm::translate(glm::mat4(1.0f), glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ())) *
			glm::mat4_cast(glm::quat(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ())) *
			glm::scale(glm::mat4(1.0f), transform.GetWorldScale());

	}

	void PhysicsSystem::HandleRemovedContacts()
	{
		if (!contactListener->GetLastStep())
			return;
		bool pass = true;

		for (auto& bodyPair : contactListener->GetBodiesInContact())
		{
			// do this later aloysius
			JPH::BodyLockRead lock1(physicsSystem->GetBodyLockInterface(), bodyPair.GetBody1ID());

			JPH::uint64 ent1{};
			JPH::uint64 ent2{};

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

			if (pass)
			{
				GameObject checkEntity1 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(ent1));
				GameObject checkEntity2 = Core::GetInstance()->mFactory.GetGOByEntity(static_cast<Entity>(ent2));

				if (!checkEntity1.HasComponent<ColliderShape>() || !checkEntity2.HasComponent<ColliderShape>())
				{
					std::string errorMsg = "Contact removed between ";
					errorMsg += std::to_string((unsigned int)checkEntity1.GetEntity());
					errorMsg += " and ";
					errorMsg += std::to_string((unsigned int)checkEntity2.GetEntity());
					SLICE_LOG_ERROR(errorMsg);
					continue;
				}

				colliderShape1 = checkEntity1.GetComponent<ColliderShape>();
				colliderShape2 = checkEntity2.GetComponent<ColliderShape>();

				std::pair<GameObject, GameObject> collisionPair = contactListener->MakeOrderedPair(checkEntity1, checkEntity2);
				//contactListener->RemoveContactPair(collisionPair);

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

	JPH::EAllowedDOFs PhysicsSystem::AllowedDOFs(const RigidBody& rigidBody) const
	{

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

		return allowedDofs;
	}

	JPH::ShapeRefC PhysicsSystem::CreateBoxShape(const ColliderShape& collider) const
	{
		const ColliderShape::BoxData& boxData = std::get<ColliderShape::BoxData>(collider.shapeData);
		JPH::BoxShapeSettings* shapeSetting = new JPH::BoxShapeSettings(boxData.scale);
		JPH::RotatedTranslatedShapeSettings newShape = JPH::RotatedTranslatedShapeSettings(
			collider.offSet,
			JPH::Quat::sIdentity(),
			shapeSetting);

		auto result = newShape.Create();

		if (result.HasError())
		{
			SLICE_LOG_ERROR("Failed to get Box Data: " + std::string(result.GetError()));
			return nullptr;
		}

		return result.Get();
	}

	JPH::ShapeRefC PhysicsSystem::CreateSphereShape( const ColliderShape& collider) const
	{
		//TRS
		const ColliderShape::SphereData& sphereData = std::get<ColliderShape::SphereData>(collider.shapeData);
		JPH::SphereShapeSettings* shapeSetting = new JPH::SphereShapeSettings(sphereData.radius);
		JPH::RotatedTranslatedShapeSettings newShape = JPH::RotatedTranslatedShapeSettings(
			collider.offSet,
			JPH::Quat::sIdentity(),
			shapeSetting);

		auto result = newShape.Create();

		if (result.HasError())
		{
			SLICE_LOG_ERROR("Failed to get Sphere Data: " + std::string(result.GetError()));
			return nullptr;
		}

		return result.Get();
	}

	JPH::ShapeRefC PhysicsSystem::CreateCapsuleShape(const ColliderShape& collider) const
	{
		const ColliderShape::CapsuleData& capsuleData = std::get<ColliderShape::CapsuleData>(collider.shapeData);
		JPH::CapsuleShapeSettings* shapeSetting = new JPH::CapsuleShapeSettings(capsuleData.height, capsuleData.radius);
		JPH::RotatedTranslatedShapeSettings newShape = JPH::RotatedTranslatedShapeSettings(
			collider.offSet,
			JPH::Quat::sIdentity(),
			shapeSetting);

		auto result = newShape.Create();

		if (result.HasError())
		{
			SLICE_LOG_ERROR("Failed to get Capsule Data: " + std::string(result.GetError()));
			return nullptr;
		}

		return result.Get();
	}

	JPH::ShapeRefC PhysicsSystem::CreateMeshShape(const Renderer& renderComponent) const
	{
		Handle<SliceEngineTypes::Model> model = renderComponent.modelHandle;

		JPH::VertexList vertices;
		JPH::IndexedTriangleList triangles;

		uint32_t vertexOffset = 0;

		//loops through the diff meshes in the model
		for (const auto& mesh : model->meshes)
		{
			// Add vertices
			for (const auto& vertex : mesh.vertices)
			{
				vertices.push_back(JPH::Float3(
					vertex.position.x,
					vertex.position.y,
					vertex.position.z
				));
			}

			// Add triangles - iterate over INDICES, not meshes!
			for (size_t i = 0; i < mesh.indices.size(); i += 3)
			{
				//i dont fking know whats going on anymore
				uint32_t i0 = vertexOffset + mesh.indices[i];
				uint32_t i1 = vertexOffset + mesh.indices[i + 1];
				uint32_t i2 = vertexOffset + mesh.indices[i + 2];

				// Get the three vertices
				JPH::Vec3 v0(vertices[i0].x, vertices[i0].y, vertices[i0].z);
				JPH::Vec3 v1(vertices[i1].x, vertices[i1].y, vertices[i1].z);
				JPH::Vec3 v2(vertices[i2].x, vertices[i2].y, vertices[i2].z);

				// Check for degenerate triangle
				JPH::Vec3 normal = (v1 - v0).Cross(v2 - v0);
				if (normal.LengthSq() < 1e-10f)
				{
					//std::cout << "[DEGENERATE TRIANGLE SKIPPED] at ("
						//<< v0.GetX() << "," << v0.GetY() << "," << v0.GetZ() << ")"
						//<< std::endl;
					continue; // skip this triangle
				}

				triangles.push_back(JPH::IndexedTriangle(i0, i1, i2));
			}

			vertexOffset += static_cast<uint32_t>(mesh.vertices.size());
		}

		
		JPH::MeshShapeSettings* shapeSetting = new JPH::MeshShapeSettings(vertices, triangles);
		JPH::Vec3 offSet{ 0.f,0.f,0.f }; // using this as no offset for mesh collider for now

		JPH::RotatedTranslatedShapeSettings newShape = JPH::RotatedTranslatedShapeSettings(
			offSet,
			JPH::Quat::sIdentity(),
			shapeSetting);

		auto result = newShape.Create();
		if (result.HasError())
		{
			SLICE_LOG_ERROR("Failed to get MeshShape Data: " + std::string(result.GetError()));
			return nullptr;
		}

		return result.Get();
	}

	JPH::ShapeRefC PhysicsSystem::CreateCylinderShape(const ColliderShape& collider) const
	{
		const ColliderShape::CylinderData& cylinderData = std::get<ColliderShape::CylinderData>(collider.shapeData);
		JPH::CylinderShapeSettings* shapeSetting = new JPH::CylinderShapeSettings(cylinderData.height, cylinderData.radius);
		JPH::RotatedTranslatedShapeSettings newShape = JPH::RotatedTranslatedShapeSettings(
			collider.offSet,
			JPH::Quat::sIdentity(),
			shapeSetting);

		auto result = newShape.Create();

		if (result.HasError())
		{
			SLICE_LOG_ERROR("Failed to get Cylinder Data: " + std::string(result.GetError()));
			return nullptr;
		}

		return result.Get();
	}

	// componeent enable check
	void PhysicsSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		auto& collider = mRegistry->get<ColliderShape>(entity);

		CreateJoltBody(entity);

		UpdateShapeFromTransform(entity);
		OnColliderModified(reg, entity);

	}

	void PhysicsSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		//auto& colliderShape = reg.get<ColliderShape>(entity);

		//if (!colliderShape.componentEnabled || reg.any_of<InactiveEntity>(entity) || colliderShape.bodyID.IsInvalid())
		//	return;

		//// Remove body form physics world
		//physicsSystem->GetBodyInterface().RemoveBody(colliderShape.bodyID);

		//// Destroy the body from the physics world
		//physicsSystem->GetBodyInterface().DestroyBody(colliderShape.bodyID);

		//physicsSystem->OptimizeBroadPhase();
	}

	void PhysicsSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		//auto& transform = reg.get<Transform>(entity);
		//auto& colliderShape = reg.get<ColliderShape>(entity);

		//SyncECSToPhysics(transform, colliderShape);

		////physicsSystem->Update(dt, collisionSteps, tempAllocator.get(), jobSystem.get());
		////SyncPhysicsToECS(transform, colliderShape);
		////HandleRemovedContacts();	
	}

	void PhysicsSystem::PreStepSync()
	{
		auto view = mRegistry->view<Transform, ColliderShape>();
		// Safe, iterator-free iteration
		for (auto [e, t, c] : view.each())
		{
			if (!c.componentEnabled)
			{
				continue;
			}

			UpdateShapeFromTransform(e);
			SyncECSToPhysics(t, c);
		}
	}

	void PhysicsSystem::ClearCollisionPairs()
	{
		contactListener->clearCollisionsPairs();
	}

	void PhysicsSystem::DeleteJoltBody(Entity entity)
	{
		GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(entity);
		if (!checkEntity.HasComponent<ColliderShape>())
			return;

		auto& colliderShape = mRegistry->get<ColliderShape>(entity);

		if (colliderShape.bodyID.IsInvalid())
			return;
		
		// Remove body form physics world
		physicsSystem->GetBodyInterface().RemoveBody(colliderShape.bodyID);
		// Destroy the body from the physics world
		physicsSystem->GetBodyInterface().DestroyBody(colliderShape.bodyID);
		colliderShape.bodyID = JPH::BodyID();
		physicsSystem->OptimizeBroadPhase();
	}

	void PhysicsSystem::CreateJoltBody(Entity entity)
	{

		//if (entity == entt::entity(1048793))
		//{
		//	int a = 2;
		//}

		GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(entity);
		if (!checkEntity.HasComponent<ColliderShape>())
			return;

		auto& slice = mRegistry->get<SliceEntity>(entity);
		auto& transform = mRegistry->get<Transform>(entity);
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);

		bool isRigibody = false;

		if (checkEntity.HasComponent<RigidBody>())
		{
			isRigibody = true;
		}

		//Create shape based on collider
		JPH::ShapeRefC shape = CreateShapeFromCollider(entity);
		if (!shape)
		{
			SLICE_LOG_ERROR("Failed to create Shape for entity");
			return;
		}
		colliderShape.shape = shape;

		//Convert transform data
		JPH::Vec3 position(transform.position.x, transform.position.y, transform.position.z);
		glm::quat rot = transform.rotation;
		JPH::Quat rotation(rot.x, rot.y, rot.z, rot.w);

		JPH::BodyCreationSettings bodySettings;

		//Create body
		if (isRigibody)
		{
			auto& rigidBody = mRegistry->get<RigidBody>(entity);
			if (rigidBody.isKinematic)
			{
				JPH::ObjectLayer layer = static_cast<JPH::ObjectLayer>(colliderShape.componentEnabled ? slice.mLayer : Layers::COLLISION_OFF);

				bodySettings = JPH::BodyCreationSettings(shape, position, rotation, JPH::EMotionType::Kinematic, layer);
			}
			else
			{
				JPH::ObjectLayer layer = static_cast<JPH::ObjectLayer>(colliderShape.componentEnabled ? slice.mLayer : Layers::COLLISION_OFF);

				bodySettings = JPH::BodyCreationSettings(shape, position, rotation, JPH::EMotionType::Dynamic, layer);
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
				bodySettings.mCollideKinematicVsNonDynamic = true;
			}

			//handle freeze position
			JPH::EAllowedDOFs allowedDofs = AllowedDOFs(rigidBody);

			bodySettings.mAllowedDOFs = allowedDofs;

			bodySettings.mMotionQuality = rigidBody.CollisionDetection;
		}
		else if (!isRigibody)
		{
			JPH::ObjectLayer layer = static_cast<JPH::ObjectLayer>(colliderShape.componentEnabled ? slice.mLayer : Layers::COLLISION_OFF);

			bodySettings = JPH::BodyCreationSettings(shape, position, rotation, JPH::EMotionType::Static, layer);
			//bodySettings.mFriction = 0.6f;
			bodySettings.mRestitution = 0.0f;

		}

		//Set as sensor for triggers
		if (colliderShape.isTrigger)
		{
			bodySettings.mIsSensor = true;
		}

		bodySettings.mAllowDynamicOrKinematic = true; // allow changing motion type at runtime

		//Store entity ID in user data for collision callbacks
		bodySettings.mUserData = static_cast<uint64_t>(entity);

		//this portion is cause mesh collider  does not have mass caluclated by jolt
		bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
		bodySettings.mMassPropertiesOverride.mMass = 1.0f;
		bodySettings.mMassPropertiesOverride.mInertia = JPH::Mat44::sScale(1.0f); // Simplified inertia

		//Create and add the body
		JPH::Body* body = physicsSystem->GetBodyInterface().CreateBody(bodySettings);
		if (!body || !mRegistry->valid(entity))
		{
			SLICE_LOG_ERROR("Failed to create Jolt body for entity");
			return;
		}

		//Add to physics world and store bodyID in rigidbody
		colliderShape.bodyID = body->GetID();
		physicsSystem->GetBodyInterface().AddBody(colliderShape.bodyID, isRigibody ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);

		//SLICE_LOG("Created Jolt body with ID: " + std::to_string(colliderShape.bodyID.GetIndexAndSequenceNumber()));
		physicsSystem->OptimizeBroadPhase();
	}

	void PhysicsSystem::SetLastStep(bool isLastStep)
	{
		contactListener->SetLastStep(isLastStep);
	}

	void PhysicsSystem::StepWorld(float dt)
	{
		//JPH::BodyID testBodyID;
		//
		//if (testBodyID.IsInvalid())
		//{
		//	//std::cout << "ALOYSISU INVALID BODYID 67676767\n";
		//}


		physicsSystem->Update(dt, collisionSteps, tempAllocator.get(), jobSystem.get());

		if (isBroadPhaseDirty)
		{
			physicsSystem->OptimizeBroadPhase();
			isBroadPhaseDirty = false;
		}
	}

	void PhysicsSystem::PostStepSync()
	{
		auto view = mRegistry->view<Transform, ColliderShape>();

		// Safe, iterator-free iteration
		for (auto [e, t, c] : view.each())
		{
			if (!c.componentEnabled)
			{
				continue;
			}
			SyncPhysicsToECS(t, c);
		}

		HandleRemovedContacts();
	}

	void PhysicsSystem::AddForceToEntity(Entity entity, const JPH::Vec3& force)
	{
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		physicsSystem->GetBodyInterface().AddForce(colliderShape.bodyID, force);
	}

	void PhysicsSystem::AddImpulseToEntity(Entity entity, const JPH::Vec3& impulse)
	{
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		physicsSystem->GetBodyInterface().AddImpulse(colliderShape.bodyID, impulse);
	}

	void PhysicsSystem::AddVelocityChangeToEntity(Entity entity, const JPH::Vec3& deltaVelocity)
	{
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		physicsSystem->GetBodyInterface().AddLinearVelocity(colliderShape.bodyID, deltaVelocity);
	}

	void PhysicsSystem::AddAccelerationToEntity(Entity entity, const JPH::Vec3& deltaVelocity)
	{
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		JPH::Vec3 accelerationDelta = deltaVelocity * static_cast<float>(1.f/60.f);

		physicsSystem->GetBodyInterface().AddLinearVelocity(colliderShape.bodyID, accelerationDelta);
	}

	void PhysicsSystem::SubscribeToEvents()
	{
		// Get the EventManager instance and subscribe our member functions.
		auto* eventManager = EventManager::GetInstance();

		// Subscribe to the ColliderShapeAddedEvent
		eventManager->Subscribe<ColliderShapeAddedEvent, &PhysicsSystem::OnColliderAdd>(this);

		// Subscribe to the ColliderShapeRemovedEvent
		//eventManager->Subscribe<ColliderShapeRemovedEvent, &PhysicsSystem::OnColliderRemove>(this);

		// Subscribe to the RigidBodyAddedEvent
		eventManager->Subscribe<RigidBodyAddedEvent, &PhysicsSystem::OnRigidBodyAdd>(this);

		// Subscribe to the RigidBodyRemovedEvent
		eventManager->Subscribe<RigidBodyRemovedEvent, &PhysicsSystem::OnRigidBodyRemove>(this);

		//eventManager->Subscribe<ColliderShapeModifiedEvent, &PhysicsSystem::OnColliderModified>(this);

		eventManager->Subscribe<RigidBodyModifiedEvent, &PhysicsSystem::OnRigidBodyModified>(this);

		//eventManager->Subscribe<SliceEntityModifiedEvent, &PhysicsSystem::OnSliceEntityModified>(this);

	}

	void PhysicsSystem::SetCollisionMask(uint32_t layer, uint32_t mask)
	{
		if (layer == Layers::COLLISION_OFF) // cannot set collision mask for COLLISION_OFF layer
			return;

		objectLayerPairFilter->SetCollisionMask(static_cast<JPH::ObjectLayer>(layer), mask);
	}

	void PhysicsSystem::SetBodyLayer(Entity entity, uint32_t layer)
	{
		GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(entity);
		if (!checkEntity.HasComponent<ColliderShape>())
			return;

		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		auto& slice = mRegistry->get<SliceEntity>(entity);

		//when component is disabled we dont change layer( Leave it as COLLISION_OFF layer)
		if (!colliderShape.componentEnabled)
		{
			return; // might change it to set to COLLISION_OFF but that should be handled when disabling component
		}

		if (physicsSystem->GetBodyInterface().GetObjectLayer(colliderShape.bodyID) != slice.mLayer)
		{
			physicsSystem->GetBodyInterface().SetObjectLayer(colliderShape.bodyID, static_cast<JPH::ObjectLayer>(slice.mLayer));
		}
	}

	void PhysicsSystem::SetObjectBroadPhaseLayer(uint32_t layer, JPH::BroadPhaseLayer bpLayer)
	{
		broadphaseLayerInterface->SetObjectToBroadPhaseLayer(static_cast<JPH::ObjectLayer>(layer), bpLayer);
	}

	JPH::uint PhysicsSystem::GetNumBroadPhaseLayers()
	{
		return broadphaseLayerInterface->GetNumBroadPhaseLayers();
	}

	JPH::BroadPhaseLayer PhysicsSystem::GetBroadPhaseLayer(uint32_t layer)
	{
		return broadphaseLayerInterface->GetBroadPhaseLayer(static_cast<JPH::ObjectLayer>(layer));
	}

	glm::vec3 PhysicsSystem::GetPosition(Entity entity)
	{
		GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(entity);
		if (!checkEntity.HasComponent<ColliderShape>())
			return glm::vec3();

		auto& colliderShape = mRegistry->get<ColliderShape>(entity);

		JPH::Vec3 pos = physicsSystem->GetBodyInterface().GetPosition(colliderShape.bodyID);

		return glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
	}

	glm::quat PhysicsSystem::GetRotation(Entity entity)
	{
		GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(entity);
		if (!checkEntity.HasComponent<ColliderShape>())
			return glm::quat();

		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		JPH::Quat rotation = physicsSystem->GetBodyInterface().GetRotation(colliderShape.bodyID);
		return glm::quat(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ());
	}

	glm::vec3 PhysicsSystem::GetScale(Entity entity)
	{
		GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(entity);
		if (!checkEntity.HasComponent<ColliderShape>())
			return glm::vec3();

		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		const JPH::Shape* shape = colliderShape.shape.GetPtr();
		if (const JPH::BoxShape* boxShape = dynamic_cast<const JPH::BoxShape*>(shape))
		{
			JPH::Vec3 halfExtents = boxShape->GetHalfExtent();
			return glm::vec3(halfExtents.GetX() * 2.0f, halfExtents.GetY() * 2.0f, halfExtents.GetZ() * 2.0f);
		}
		else if (const JPH::SphereShape* sphereShape = dynamic_cast<const JPH::SphereShape*>(shape))
		{
			float radius = sphereShape->GetRadius();
			return glm::vec3(radius * 2.0f); // Uniform scale for sphere
		}
		else if (const JPH::CapsuleShape* capsuleShape = dynamic_cast<const JPH::CapsuleShape*>(shape))
		{
			float radius = capsuleShape->GetRadius();
			float height = capsuleShape->GetHalfHeightOfCylinder() * 2.0f;
			return glm::vec3(radius * 2.0f, height, radius * 2.0f); // Assuming Y-axis is the height
		}
		return glm::vec3();
	}

	int PhysicsSystem::GetCollisionSteps() const
	{
		return collisionSteps;
	}

	void PhysicsSystem::SetCollisionSteps(int steps)
	{
		collisionSteps = steps;
	}

	float PhysicsSystem::GetGravityFactor(Entity entity) const
	{
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		return physicsSystem->GetBodyInterface().GetGravityFactor(colliderShape.bodyID);

	}

	void PhysicsSystem::SetGravityFactor(Entity entity, float factor)
	{
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		physicsSystem->GetBodyInterface().SetGravityFactor(colliderShape.bodyID, factor);
	}

	void PhysicsSystem::OffGravity(Entity entity, bool condition)
	{
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		if (condition)
		{
			physicsSystem->GetBodyInterface().SetGravityFactor(colliderShape.bodyID,0.0f);
		}
		else if (!condition)
		{
			physicsSystem->GetBodyInterface().SetGravityFactor(colliderShape.bodyID, 1.0f);
		}
	}

	bool PhysicsSystem::IsGravityOff(Entity entity) const
	{
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);
		 float factor = physicsSystem->GetBodyInterface().GetGravityFactor(colliderShape.bodyID);

		 return factor > 0.0f ? false : true;
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

	bool PhysicsSystem::PSystemRayCast(const glm::vec3 origin, const glm::vec3 direction,uint32_t& bodyHitID, glm::vec3& hitPos, glm::vec3& normal, bool triggerInteraction, uint32_t mask)
	{	
		JPH::Vec3 ori = helpers::glmtoJPH(origin);
		JPH::Vec3 dir = helpers::glmtoJPH(direction);

		JPH::RRayCast inRay(ori, dir);
		JPH::RayCastResult ioHit; // only reference rest is const
		const JPH::BroadPhaseLayerFilter& inBroadPhaseLayerFilter = { };
		ObjectLayerFilterImpl filterLayer(mask);

		JPH::BodyFilter inBodyFilter = {};
		BodyFilterIgnore ignoreFilter;
		bool didRayHit = false;

		if (triggerInteraction)
		{
			didRayHit = physicsSystem->GetNarrowPhaseQuery().CastRay(inRay, ioHit, inBroadPhaseLayerFilter, filterLayer, inBodyFilter);
		}
		else
		{
			didRayHit = physicsSystem->GetNarrowPhaseQuery().CastRay(inRay, ioHit, inBroadPhaseLayerFilter, filterLayer, ignoreFilter);
		}

		if (!ioHit.mBodyID.IsInvalid())
		{
			hitPos = origin + direction * ioHit.mFraction;

			// do this later aloysius
			JPH::BodyLockRead lock1(physicsSystem->GetBodyLockInterface(), ioHit.mBodyID);

			if (lock1.Succeeded())
			{
				const JPH::Body& body = lock1.GetBody();
				bodyHitID = static_cast<JPH::uint32>(body.GetUserData());
				normal = helpers::JPHtoglm(body.GetWorldSpaceSurfaceNormal(ioHit.mSubShapeID2, helpers::glmtoJPH(hitPos)));
			}
		}
		else
		{
			didRayHit = false;
		}

		return didRayHit;
	}
	bool PhysicsSystem::PSystemSphereCast(const glm::vec3 origin, const glm::vec3 direction, float radius,
		uint32_t& bodyHitID, glm::vec3& hitPos, glm::vec3& normal, bool triggerInteraction, uint32_t mask)
	{
		JPH::Vec3 ori = helpers::glmtoJPH(origin);
		JPH::Vec3 dir = helpers::glmtoJPH(direction);

		// Build the sphere shape
		JPH::SphereShape sphereShape(radius);

		// ShapeCast needs a starting transform (just translation, no rotation needed for sphere)
		JPH::RShapeCast shapeCast = JPH::RShapeCast::sFromWorldTransform(
			&sphereShape,
			JPH::Vec3::sReplicate(1.0f),        // scale
			JPH::RMat44::sTranslation(ori),      // start transform
			dir                                   // cast direction (length = distance)
		);

		JPH::ShapeCastSettings castSettings;
		//castSettings.mBackFaceModeTriangles = JPH::EBackFaceMode::IgnoreBackFaces;
		//castSettings.mBackFaceModeConvex = JPH::EBackFaceMode::IgnoreBackFaces;

		// Use ClosestHit collector to mirror raycast behaviour (first/closest hit)
		JPH::ClosestHitCollisionCollector<JPH::CastShapeCollector> collector;

		const JPH::BroadPhaseLayerFilter& inBroadPhaseLayerFilter = {};
		ObjectLayerFilterImpl filterLayer(mask);
		JPH::BodyFilter inBodyFilter = {};
		BodyFilterIgnore ignoreFilter;

		// Base offset for float precision (same trick as Jolt docs recommend)
		JPH::RVec3 baseOffset = ori;

		if (triggerInteraction)
		{
			physicsSystem->GetNarrowPhaseQuery().CastShape(
				shapeCast, castSettings, baseOffset, collector,
				inBroadPhaseLayerFilter, filterLayer, inBodyFilter);
		}
		else
		{
			physicsSystem->GetNarrowPhaseQuery().CastShape(
				shapeCast, castSettings, baseOffset, collector,
				inBroadPhaseLayerFilter, filterLayer, ignoreFilter);
		}

		if (!collector.HadHit())
			return false;

		const JPH::ShapeCastResult& hit = collector.mHit;

		if (hit.mBodyID2.IsInvalid())
			return false;

		// Reconstruct world hit position
		// baseOffset + contact point (which is relative to baseOffset)
		glm::vec3 contactPoint = helpers::JPHtoglm(JPH::Vec3(baseOffset) + hit.mContactPointOn2);
		hitPos = contactPoint;

		JPH::BodyLockRead lock(physicsSystem->GetBodyLockInterface(), hit.mBodyID2);
		if (lock.Succeeded())
		{
			const JPH::Body& body = lock.GetBody();
			bodyHitID = static_cast<JPH::uint32>(body.GetUserData());
			normal = helpers::JPHtoglm(body.GetWorldSpaceSurfaceNormal(hit.mSubShapeID2, helpers::glmtoJPH(hitPos)));
		}

		return true;
	}


}
