
#include <pch.h>
#include "../Core/Core.h"
#include "PhysicsSystem.h"
#include "PhysicsDebug.h"
#include "../Graphics/TransformHelper.h"
#include "../Core/EventManager.h"
#include "../ECS/GOFactory.h"

#define EPSILON 0.0001f

namespace SliceEngine
{

	PhysicsSystem::~PhysicsSystem()
	{
		Shutdown();
		SLICE_LOG("Physics System Shutdown");
	}

	bool PhysicsSystem::Initialize(float fixedDt, JPH::uint maxBodies, JPH::uint numBodyMutex, JPH::uint maxContactConstraints, JPH::uint threadCount)
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

			tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024); //temporary use 10mb

			jobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, threadCount);

			broadphaseLayerInterface = std::make_unique<BPLayerInterfaceImpl>();
			objectVsBroadphaseLayerFilter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
			objectLayerPairFilter = std::make_unique<ObjectLayerPairFilterImpl>();

			physicsSystem = std::make_unique<JPH::PhysicsSystem>();
			physicsSystem->Init(maxBodies, numBodyMutex, maxBodies, maxContactConstraints,
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

	void PhysicsSystem::OnColliderAdd(const ColliderShapeAddedEvent& event)
	{
		std::cout << "LMOA OI ADDED Collider\n";
	}

	void PhysicsSystem::OnColliderRemove(const ColliderShapeRemovedEvent& event)
	{
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

				mp->ScaleToMass(rigidBody.mass);
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
		auto& rigidBody = mRegistry->get<RigidBody>(event.entity);
		auto& colliderShape = mRegistry->get<ColliderShape>(event.entity);


		physicsSystem->GetBodyInterface().SetMotionType(colliderShape.bodyID, JPH::EMotionType::Static, JPH::EActivation::DontActivate);

		float mass = 1.0f;
		float friction = 0.5f;
		float restitution = 0.0f;
		float linearDamping = 0.05f;
		float angularDamping = 0.05f;

		physicsSystem->GetBodyInterface().SetMotionQuality(colliderShape.bodyID, rigidBody.CollisionDetection);

		physicsSystem->GetBodyInterface().SetFriction(colliderShape.bodyID, friction);
		physicsSystem->GetBodyInterface().SetRestitution(colliderShape.bodyID, restitution);

		JPH::BodyLockWrite lock(physicsSystem->GetBodyLockInterface(), colliderShape.bodyID);
		if (lock.Succeeded())
		{
			JPH::Body& body = lock.GetBody();
			JPH::MotionProperties* mp = body.GetMotionProperties();

			mp->ScaleToMass(mass);
			mp->SetLinearDamping(linearDamping);
			mp->SetAngularDamping(angularDamping);
		}

	}

	void PhysicsSystem::UpdateShapeFromTransform(Entity entity)
	{
		auto& transform = mRegistry->get<Transform>(entity);
		auto& colliderShape = mRegistry->get<ColliderShape>(entity);

			if (transform.scale == transform.previousScale)
				return;

			// Rebuild only if Box (sphere generally uses radius; you could scale radius by max component if desired)
			if (colliderShape.type == ColliderShape::ColliderType::Box)
			{
				const auto& boxData = std::get<ColliderShape::BoxData>(colliderShape.shapeData);
				JPH::Vec3 newHalf(
					boxData.scale.GetX() * transform.scale.x,
					boxData.scale.GetY() * transform.scale.y,
					boxData.scale.GetZ() * transform.scale.z
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
					// Update shape; true => update mass properties (you can pass false then re-apply custom mass if needed)
					physicsSystem->GetBodyInterface().SetShape(colliderShape.bodyID, colliderShape.shape, true, JPH::EActivation::DontActivate);

					// If you have overridden mass, re-apply:
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

			transform.previousScale = transform.scale;	

	}

	JPH::ShapeRefC PhysicsSystem::CreateShapeFromCollider(const ColliderShape& collider) const
	{
		switch (collider.type)
		{

		case ColliderShape::ColliderType::Box:
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

		glm::vec3 rot = QuatToVec3(glm::quat(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ())); // glm store as w,x,y,z

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
				bodySettings.mMassPropertiesOverride.mMass = rigidBody.mass;
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

		UpdateShapeFromTransform(entity);

		SyncECSToPhysics(transform, colliderShape);
		physicsSystem->Update(dt, collisionSteps, tempAllocator.get(), jobSystem.get());
		SyncPhysicsToECS(transform, colliderShape);
	}

	void PhysicsSystem::SubscribeToCollisionEvents()
	{
		// Get the EventManager instance and subscribe our member functions.
		auto* eventManager = EventManager::GetInstance();

		// Subscribe to the PlayerJumpedEvent
		eventManager->Subscribe<ColliderShapeAddedEvent, &PhysicsSystem::OnColliderAdd>(this);

		// Subscribe to the EnemyDefeatedEvent
		eventManager->Subscribe<ColliderShapeRemovedEvent, &PhysicsSystem::OnColliderRemove>(this);

		// Subscribe to the PlayerJumpedEvent
		eventManager->Subscribe<RigidBodyAddedEvent, &PhysicsSystem::OnRigidBodyAdd>(this);

		// Subscribe to the EnemyDefeatedEvent
		eventManager->Subscribe<RigidBodyRemovedEvent, &PhysicsSystem::OnRigidBodyRemove>(this);
	}

}
