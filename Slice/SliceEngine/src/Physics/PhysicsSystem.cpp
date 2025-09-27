
#include <pch.h>
#include "../Core/Core.h"
#include "PhysicsSystem.h"
#include "PhysicsDebug.h"
#include "../Graphics/TransformHelper.h"

namespace SliceEngine 
{

	PhysicsSystem::~PhysicsSystem()
	{
		Shutdown();
		SLICE_LOG("Physics System Shutdown");
	}

	bool PhysicsSystem::Initialize(JPH::uint maxBodies, JPH::uint numBodyMutex, JPH::uint maxContactConstraints, JPH::uint threadCount)
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

			tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024); //temporary use 10mb

			jobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, threadCount);

			broadphaseLayerInterface = std::make_unique<BPLayerInterfaceImpl>();
			objectVsBroadphaseLayerFilter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
		    objectLayerPairFilter = std::make_unique<ObjectLayerPairFilterImpl>();

			physicsSystem = std::make_unique<JPH::PhysicsSystem>();
			physicsSystem->Init(maxBodies, numBodyMutex,maxBodies, maxContactConstraints,
								*broadphaseLayerInterface,
								*objectVsBroadphaseLayerFilter,
								*objectLayerPairFilter);

			isInitialized = true;
			SLICE_LOG("Physics System Initialized");
			return true;
		}
		catch (const std::exception& e)
		{
			SLICE_LOG_ERROR("Physic System failed to initalize");
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

	JPH::ShapeRefC PhysicsSystem::CreateShapeFromCollider(const ColliderShape& collider) const
	{
		switch (collider.type)
		{

		case ColliderShape::ColliderType::Box:
		{
			const ColliderShape::BoxData& boxData = std::get<ColliderShape::BoxData>(collider.shapeData);
			JPH::BoxShapeSettings shapeSetting(boxData.halfExtend);

			auto result = shapeSetting.Create();

			if (result.HasError())
			{
				SLICE_LOG_ERROR("Failed to get Box Data: " + std::string(result.GetError()));
				return nullptr;
			}

			return result.Get();
		}	
		case ColliderShape::ColliderType::Sphere:
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
		default:
				SLICE_LOG_ERROR("Unsupported Collider Shape");
				return nullptr;

		}
	}

	void PhysicsSystem::SyncECSToPhysics(Transform& transform, ColliderShape& colliderShape) const
	{
		JPH::Vec3 pos(transform.position.x, transform.position.y, transform.position.z);
		glm::quat rot = Vec3ToQuat(transform.rotation);
		JPH::Quat rotation(rot.x, rot.y, rot.z, rot.w);

		physicsSystem->GetBodyInterface().SetPosition(colliderShape.bodyID, pos, JPH::EActivation::DontActivate);
		physicsSystem->GetBodyInterface().SetRotation(colliderShape.bodyID, rotation, JPH::EActivation::DontActivate);
 
	}

	void PhysicsSystem::SyncPhysicsToECS(Transform& transform, ColliderShape& colliderShape) const
	{
		JPH::Vec3 pos = physicsSystem->GetBodyInterface().GetPosition(colliderShape.bodyID);
		JPH::Quat rotation = physicsSystem->GetBodyInterface().GetRotation(colliderShape.bodyID);

		glm::vec3 rot = QuatToVec3(glm::quat(rotation.GetX(), rotation.GetY(), rotation.GetZ(), rotation.GetW()));

		transform.position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());//i will create helper function for converservion of glm and jolt data types
		transform.rotation = rot;
	}


	void PhysicsSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		auto& transform = reg.get<Transform>(entity);
		auto& colliderShape = reg.get<ColliderShape>(entity);

		bool isRigibody = false;

		GameObject checkEntity = Core::GetInstance()->mFactory.GetGOByEntity(entity);
		if (checkEntity.HasComponent<RigidBody>())
		{
			auto& rigidBody = reg.get<RigidBody>(entity);
			isRigibody = true;
		}

		//Create shape based on collider
		JPH::ShapeRefC shape = CreateShapeFromCollider(colliderShape);
		if (!shape)
		{
			SLICE_LOG_ERROR("Failed to create Shape for entity");
			return;
		}

		//Convert transform data
		JPH::Vec3 position(transform.position.x, transform.position.y, transform.position.z);
		glm::quat rot = Vec3ToQuat(transform.rotation);
		JPH::Quat rotation(rot.x, rot.y, rot.z, rot.w);

		JPH::BodyCreationSettings bodySettings;

		//Create body
		if (isRigibody)
		{
			auto& rigidBody = reg.get<RigidBody>(entity);
			if(rigidBody.isKinematic)
			{
			bodySettings = JPH::BodyCreationSettings(shape, position, rotation, JPH::EMotionType::Kinematic, colliderShape.layer);
			}
			else
			{
			bodySettings = JPH::BodyCreationSettings(shape, position, rotation, JPH::EMotionType::Dynamic, colliderShape.layer);
			}

			//Set physics properties
			if(!rigidBody.isKinematic)
			{
				bodySettings.mMotionQuality = rigidBody.CollisionDetection;
				bodySettings.mMassPropertiesOverride.mMass = rigidBody.mass;
				bodySettings.mFriction = rigidBody.friction;
				bodySettings.mRestitution = rigidBody.restitution;
				bodySettings.mLinearDamping = rigidBody.linearDamping;
				bodySettings.mAngularDamping = rigidBody.angularDamping;
			}
			if(rigidBody.isKinematic)
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

	}

	void PhysicsSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{

		auto& colliderShape = reg.get<ColliderShape>(entity);

		// Remove body form physics world
		physicsSystem->GetBodyInterface().RemoveBody(colliderShape.bodyID);

		// Destroy the body from the physics world
		physicsSystem->GetBodyInterface().DestroyBody(colliderShape.bodyID);
	}

	void PhysicsSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		auto& transform = reg.get<Transform>(entity);
		auto& colliderShape = reg.get<ColliderShape>(entity);

		SyncECSToPhysics(transform, colliderShape);
		physicsSystem->Update(dt, 1, tempAllocator.get(), jobSystem.get());
		SyncPhysicsToECS(transform, colliderShape);
	}

}
