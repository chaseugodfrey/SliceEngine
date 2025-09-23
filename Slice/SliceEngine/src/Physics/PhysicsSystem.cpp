
#include <pch.h>
#include "PhysicsSystem.h"
#include "PhysicsDebug.h"

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

	void PhysicsSystem::CreateBodyFromComponent(entt::entity entity, const Transform& transform, RigidBody& rigidBody, const ColliderShape& colliderShape) const
	{
		// wait for transformcomponent to be finalized


		//Create shape based on collider
		JPH::ShapeRefC shape = CreateShapeFromCollider(colliderShape);
		if (!shape)
		{
			SLICE_LOG_ERROR("Failed to create Shape for entity");
			return;
		}

		//Convert transform data
		JPH::Vec3 position(transform.position.x,transform.position.y,transform.position.z);
		JPH::Quat rotation;

		//Create body
		JPH::BodyCreationSettings bodySettings(shape, position, rotation, rigidBody.motionType, rigidBody.layer);

		//Set physics properties
		if (rigidBody.motionType == JPH::EMotionType::Dynamic) 
		{
			bodySettings.mMassPropertiesOverride.mMass = rigidBody.mass;
			bodySettings.mFriction = rigidBody.friction;
			bodySettings.mRestitution = rigidBody.restitution;
			bodySettings.mLinearDamping = rigidBody.linearDamping; // Jolt uses single value
			bodySettings.mAngularDamping = rigidBody.angularDamping;
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
		rigidBody.bodyID = body->GetID();
		//body_interface->AddBody(bodyID, rigidBody.isActive ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);

		SLICE_LOG("Created Jolt body with ID: " + std::to_string(rigidBody.bodyID.GetIndexAndSequenceNumber()));

	}

	void PhysicsSystem::SyncECSToPhysics(Transform& transform, RigidBody& rigidBody, ColliderShape& colliderShape) const
	{
		// wait for transformcomponent to be finalized

	}

	void PhysicsSystem::SyncPhysicsToECS(Transform& transform, RigidBody& rigidBody, ColliderShape& colliderShape) const
	{
		// wait for transformcomponent to be finalized

	}


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
