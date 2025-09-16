
#include <pch.h>
#include "PhysicsSystem.h"
#include "PhysicsDebug.h"

namespace SliceEngine 
{

	bool PhysicsSystem::Initialize(JPH::uint maxBodie, JPH::uint numBodyMutex, JPH::uint maxContactConstraint)
	{
		if (isInitialized)
		{
			return false;
		}

		SLICE_LOG("Register default allocator for Jolt Function Pointer");
		//Jolt uses function pointers for memory allocation, sets up the function pointers Jolt uses internally.
		JPH::RegisterDefaultAllocator();

		SLICE_LOG("Hook Jolt Tracer to SliceEngine Logger");
		//Jolt has a global function pointer "Trace" for debugging and logging messages
		//Hook Jolt Trace to SliceEngines logger.
		JPH::Trace = JoltTraceImpl;
		JPH::JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)


		isInitialized = true;
		return true;
	}

	bool IsInitialized() { return IsInitialized; }

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
