/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        NavigationSystem.cpp

 author:	  Crystal Koh Qiao Wei

 email:       k.crystalqiaowei@digipen.edu

 brief:       Defines the NavigationSystem class and related structures for handling pathfinding
			  and AI movement within the engine. This system manages the loading, rendering,
			  and updating of Navigation Meshes (NavMesh), calculates paths, and updates
			  NavAgent entities to navigate the environment.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "NavigationSystem.h"
#include "../Core/Core.h"
#include "Physics/PhysicsSystem.h"
namespace SliceEngine
{

	void NavigationSystem::Init()
	{
		EventManager::GetInstance()->Subscribe<OnSceneLoadedEvent, &NavigationSystem::LoadNavMeshOnSceneLoad>(this);
	}

	void NavigationSystem::Update(float dt)
	{
		if (navMeshInstance.has_value() && navMeshInstance->navMeshCrowd)
		{
			navMeshInstance->navMeshCrowd->update(dt, nullptr);
		}
		BaseSystem::Update(dt);
	}

	void NavigationSystem::Unbind()
	{
		ClearNavMesh();
		BaseSystem::Unbind();
	}

	void NavigationSystem::ClearNavMesh()
	{
		if (navMeshInstance.has_value())
		{
			NavMeshObj &obj = navMeshInstance.value();

			if (obj.navMeshCrowd)
			{
				dtFreeCrowd(obj.navMeshCrowd);
				obj.navMeshCrowd = nullptr;
			}

			if (obj.navMeshQuery)
			{
				dtFreeNavMeshQuery(obj.navMeshQuery);
				obj.navMeshQuery = nullptr;
			}

			if (obj.navMesh)
			{
				dtFreeNavMesh(obj.navMesh);
				obj.navMesh = nullptr;
			}
		}

		if (navMeshDebugInfo.has_value())
		{
			auto &data = navMeshDebugInfo.value().data;
			for (int i{}; i < 2; ++i)
			{
				if (data[i].vao)
				{
					glDeleteVertexArrays(1, &data[i].vao);
					data[i].vao = 0;
				}
				if (data[i].vbo)
				{
					glDeleteBuffers(1, &data[i].vbo);
					data[i].vbo = 0;
				}
			}
			navMeshDebugInfo.reset();
		}

		if (activePathDebugInfo.has_value())
		{
			auto &data = activePathDebugInfo.value().data;
			if (data[0].vao) glDeleteVertexArrays(1, &data[0].vao);
			if (data[0].vbo) glDeleteBuffers(1, &data[0].vbo);
			activePathDebugInfo.reset();
		}

		navMeshInstance.reset();
	}

	void NavigationSystem::LoadNavMeshOnSceneLoad(OnSceneLoadedEvent &e)
	{
		if (e.isSceneLoaded)
		{
			LoadNavMeshFromFile(e.navMeshBinPath);
		}
	}

	void NavigationSystem::LoadNavMeshFromBake(NavMeshObj newNavMesh)
	{
		ClearNavMesh();
		navMeshInstance = std::make_optional<NavMeshObj>(newNavMesh);
		navMeshDebugInfo = NavMeshUtilities::CreateDebugMesh(newNavMesh);
	}

	void NavigationSystem::LoadNavMeshFromFile(const std::string &filePath)
	{
		std::string path_to_load = filePath + ".bin";

		//path_to_load += ".scene";

		if (path_to_load.empty() || path_to_load == "Resources/0.bin")
		{
			//path_to_load = "Resources/output_navmesh.bin";
			ClearNavMesh();
			SLICE_LOG("NavSystem: No specific navmesh found in meta");

			return;
		}
		else
		{
			SLICE_LOG("NavSystem: Loading specific navmesh from meta: " + path_to_load);
		}

		auto &&newNavMesh = NavMeshUtilities::LoadNavMesh(path_to_load);
		if (newNavMesh.has_value())
		{
			ClearNavMesh();
			navMeshInstance = std::make_optional<NavMeshObj>(newNavMesh.value());
			navMeshDebugInfo = NavMeshUtilities::CreateDebugMesh(newNavMesh.value());
		}
	}

	std::optional<NavMeshObj> &NavigationSystem::GetNavMeshObj()
	{
		return navMeshInstance;
	}

	std::optional<NavMeshDebugObj> &NavigationSystem::GetNavMeshDebugData()
	{
		return navMeshDebugInfo;
	}

	void NavigationSystem::EntityOnEnter(entt::registry &reg, entt::entity entity)
	{
	}
	void NavigationSystem::EntityOnExit(entt::registry &reg, entt::entity entity)
	{
	}
	void NavigationSystem::EntityOnUpdate(entt::registry &reg, entt::entity entity, float dt)
	{
		if (!navMeshInstance) return;

		auto &navMeshObj = navMeshInstance.value();
		auto &agent = reg.get<NavAgent>(entity);
		auto &transform = reg.get<Transform>(entity);

		if (!agent.componentEnabled) return;

		InitializeAgent(agent, transform, navMeshObj);
		UpdateAgentTarget(agent, navMeshObj);
		ApplyAgentPhysics(reg, entity, agent, transform, navMeshObj);
	}

	// Initialize Detour Crowd Agent
	void NavigationSystem::InitializeAgent(NavAgent &agent, const Transform &transform, NavMeshObj &navMeshObj)
	{
		if (agent.crowdAgentID != -1) return;

		dtCrowdAgentParams ap;
		memset(&ap, 0, sizeof(ap));
		ap.radius = 1.0f;
		ap.height = 2.0f;
		ap.maxAcceleration = 8.0f;
		ap.maxSpeed = agent.speed;
		ap.collisionQueryRange = ap.radius * 12.0f;
		ap.pathOptimizationRange = ap.radius * 30.0f;
		ap.updateFlags = DT_CROWD_ANTICIPATE_TURNS | DT_CROWD_OPTIMIZE_VIS | DT_CROWD_OPTIMIZE_TOPO | DT_CROWD_OBSTACLE_AVOIDANCE;
		ap.obstacleAvoidanceType = 3;
		ap.separationWeight = 2.0f;

		float pos[3] = { transform.position.x, transform.position.y, transform.position.z };
		agent.crowdAgentID = navMeshObj.navMeshCrowd->addAgent(pos, &ap);
	}

	//Request New Path Target
	void NavigationSystem::UpdateAgentTarget(NavAgent &agent, NavMeshObj &navMeshObj)
	{
		if (!agent.hasNewTarget || agent.crowdAgentID == -1) return;

		float targetPos[3] = { agent.target.x, agent.target.y, agent.target.z };
		float extents[3] = { 10.0f, 10.0f, 10.0f };
		dtPolyRef targetRef;
		float targetPosOnMesh[3];
		dtQueryFilter filter;

		navMeshObj.navMeshQuery->findNearestPoly(targetPos, extents, &filter, &targetRef, targetPosOnMesh);

		if (targetRef)
		{
			navMeshObj.navMeshCrowd->requestMoveTarget(agent.crowdAgentID, targetRef, targetPosOnMesh);
			agent.hasNewTarget = false;
		}
	}

	//Sync Physics & Detour Parameters
	void NavigationSystem::ApplyAgentPhysics(entt::registry &reg, entt::entity entity, NavAgent &agent, Transform &transform, NavMeshObj &navMeshObj)
	{
		if (agent.crowdAgentID == -1) return;

		// 1. Update Dynamic C# Parameters
		dtCrowdAgent *editableAg = navMeshObj.navMeshCrowd->getEditableAgent(agent.crowdAgentID);
		if (editableAg)
		{
			if (agent.speed <= 0.0f) agent.speed = 5.0f;
			editableAg->params.maxSpeed = agent.speed;
			editableAg->params.maxAcceleration = 20.0f;
			editableAg->params.updateFlags |= DT_CROWD_SEPARATION;
		}

		// 2. Fetch Crowd Result and Apply to Physics
		const dtCrowdAgent *ag = navMeshObj.navMeshCrowd->getAgent(agent.crowdAgentID);
		if (ag && ag->active && ag->state == DT_CROWDAGENT_STATE_WALKING)
		{
			glm::vec3 desiredVel(ag->vel[0], ag->vel[1], ag->vel[2]);

			auto rb = reg.try_get<RigidBody>(entity);
			auto &physicsSys = SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PhysicsSystem>();

			if (rb)
			{
				glm::vec3 currentPhysicsVel = physicsSys.GetLinearVelocity(entity);
				JPH::Vec3 newVelocity(desiredVel.x, currentPhysicsVel.y, desiredVel.z); // Keep gravity
				physicsSys.SetLinearVelocity(entity, newVelocity);

				// Sync exact position back to crowd so it doesn't fight physics
				float pos[3] = { transform.position.x, transform.position.y, transform.position.z };
				if (editableAg) memcpy(editableAg->npos, pos, sizeof(float) * 3);
			}
			else
			{
				// Fallback if no physics attached
				transform.position.x = ag->npos[0];
				transform.position.y = ag->npos[1];
				transform.position.z = ag->npos[2];
			}
		}
	}

}