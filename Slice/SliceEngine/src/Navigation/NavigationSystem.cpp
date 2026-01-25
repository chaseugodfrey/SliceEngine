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
			LoadNavMeshFromFile(e.navMeshPath);
		}
	}

	void NavigationSystem::LoadNavMeshFromBake(NavMeshObj newNavMesh)
	{
		ClearNavMesh();
		navMeshInstance = std::make_optional<NavMeshObj>(newNavMesh);
		navMeshDebugInfo = std::make_optional<NavMeshDebugObj>(NavMeshUtilities::CreateDebugMesh(newNavMesh));
	}

	void NavigationSystem::LoadNavMeshFromFile(const std::string& filePath)
	{
		std::string path_to_load = filePath;

		if (path_to_load.empty())
		{
			path_to_load = "Resources/output_navmesh.bin";
			SLICE_LOG("NavSystem: No specific navmesh found in meta. Loading default: " + path_to_load);
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
			navMeshDebugInfo = std::make_optional<NavMeshDebugObj>(NavMeshUtilities::CreateDebugMesh(newNavMesh.value()));
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

		if (!navMeshInstance)
		{
			SLICE_LOG("No Nav Mesh Data detected.");
			return;
		}

		auto &navMeshObj = navMeshInstance.value();
		auto &agent = reg.get<NavAgent>(entity);
		auto &transform = reg.get<Transform>(entity);

		if (!agent.componentEnabled)
			return;

		//detourCrowd stuff
		if (agent.crowdAgentID == -1)
		{
			dtCrowdAgentParams ap;
			memset(&ap, 0, sizeof(ap));
			ap.radius = 1.0f; // Agent Radius (Physics size)
			ap.height = 2.0f;
			ap.maxAcceleration = 8.0f;
			ap.maxSpeed = agent.speed;
			ap.collisionQueryRange = ap.radius * 12.0f;
			ap.pathOptimizationRange = ap.radius * 30.0f;

			ap.updateFlags = DT_CROWD_ANTICIPATE_TURNS | DT_CROWD_OPTIMIZE_VIS |
				DT_CROWD_OPTIMIZE_TOPO | DT_CROWD_OBSTACLE_AVOIDANCE;
			ap.obstacleAvoidanceType = 3;
			ap.separationWeight = 2.0f;

			float pos[3] = { transform.position.x, transform.position.y, transform.position.z };
			agent.crowdAgentID = navMeshObj.navMeshCrowd->addAgent(pos, &ap);
		}
		if (agent.hasNewTarget && agent.crowdAgentID != -1)
		{
			float targetPos[3] = { agent.target.x, agent.target.y, agent.target.z };
			float extents[3] = { 10.0f, 10.0f, 10.0f };
			dtPolyRef targetRef;
			float targetPosOnMesh[3];
			dtQueryFilter filter;

			// Find nearest polygon to target
			navMeshObj.navMeshQuery->findNearestPoly(targetPos, extents, &filter, &targetRef, targetPosOnMesh);

			if (targetRef)
			{
				//SLICE_LOG("Target Poly Found! Requesting move.");
				// Tell Crowd Agent to move there
				navMeshObj.navMeshCrowd->requestMoveTarget(agent.crowdAgentID, targetRef, targetPosOnMesh);
				agent.hasNewTarget = false;
			}
			else
			{
				SLICE_LOG_ERROR("NavSystem: Could not find NavMesh polygon near target position!");
			}
		}
		if (agent.crowdAgentID != -1)
		{
			// 1. UPDATE PARAMS FIRST
			// Ensure C# changes (speed) propagate immediately.
			dtCrowdAgent *editableAg = navMeshObj.navMeshCrowd->getEditableAgent(agent.crowdAgentID);
			if (editableAg)
			{
				if (agent.speed <= 0.0f) agent.speed = 5.0f; // Safety
				editableAg->params.maxSpeed = agent.speed;
				editableAg->params.maxAcceleration = 20.0f; // High accel
				// Ensure separation is enabled
				editableAg->params.updateFlags |= DT_CROWD_SEPARATION;
			}

			// 2. READ STATE
			const dtCrowdAgent *ag = navMeshObj.navMeshCrowd->getAgent(agent.crowdAgentID);
			if (ag && ag->active)
			{
				if (ag->state == DT_CROWDAGENT_STATE_WALKING)
				{
					if (ag->ncorners > 0)
					{
						bool isNextOffMesh = (ag->cornerFlags[0] & DT_STRAIGHTPATH_OFFMESH_CONNECTION);

						if (isNextOffMesh)
						{
							SLICE_LOG("Approaching OffMesh Link! Distance: {}", glm::distance(transform.position, glm::vec3(ag->cornerVerts[0], ag->cornerVerts[1], ag->cornerVerts[2])));
						}
					}
					// Check if the next corner is an off-mesh connection
					if (ag->ncorners > 0 && (ag->cornerFlags[0] & DT_STRAIGHTPATH_OFFMESH_CONNECTION))
					{
						// cornerVerts[0] is the Start of the link (where we are going now)
						// cornerVerts[1] is the End of the link (where we need to jump to)
						glm::vec3 endLinkPos(
							ag->cornerVerts[3],
							ag->cornerVerts[4],
							ag->cornerVerts[5]
						);

						m_agentOffMeshTargets[agent.crowdAgentID] = endLinkPos;
					}
				}

				// ---------------------------------------------------------
				// 2. HANDLE OFF-MESH MOVEMENT
				// ---------------------------------------------------------
				if (ag->state == DT_CROWDAGENT_STATE_OFFMESH)
				{
					// Retrieve the cached target
					glm::vec3 targetPos = transform.position; // Default fallback
					if (m_agentOffMeshTargets.find(agent.crowdAgentID) != m_agentOffMeshTargets.end())
					{
						targetPos = m_agentOffMeshTargets[agent.crowdAgentID];
					}

					auto rb = reg.try_get<RigidBody>(entity);
					auto &physicsSys = SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PhysicsSystem>();

					if (rb)
					{
						glm::vec3 currentPos = transform.position;
						glm::vec3 dir = targetPos - currentPos;

						// CHECK: Stop if we are close enough to the end
						float dist = glm::length(dir);
						if (dist < 0.5f)
						{
							// We reached the end. 
							// DetourCrowd will automatically switch back to WALKING 
							// once we update npos to be at the end.
						}

						// NORMALIZE THE VECTOR
						if (dist > 0.001f)
							dir = glm::normalize(dir);

						// You might want a specific jump speed or arc here
						float moveSpeed = 10.0f;

						JPH::Vec3 newVelocity(dir.x * moveSpeed, dir.y * moveSpeed, dir.z * moveSpeed);
						physicsSys.SetLinearVelocity(entity, newVelocity);

						// Sync Physics pos back to Crowd Agent
						float pos[3] = { transform.position.x, transform.position.y, transform.position.z };
						dtCrowdAgent *editableAg = navMeshObj.navMeshCrowd->getEditableAgent(agent.crowdAgentID);
						if (editableAg) memcpy(editableAg->npos, pos, sizeof(float) * 3);
					}
					else
					{
						// Teleport if no physics
						transform.position = targetPos;

						// Sync immediately to complete the jump
						float pos[3] = { targetPos.x, targetPos.y, targetPos.z };
						dtCrowdAgent *editableAg = navMeshObj.navMeshCrowd->getEditableAgent(agent.crowdAgentID);
						if (editableAg) memcpy(editableAg->npos, pos, sizeof(float) * 3);
					}
					return; // Skip standard walking update below
				}

				glm::vec3 desiredVel(ag->vel[0], ag->vel[1], ag->vel[2]);

				// DEBUG: Print State and Velocity
				// State 0: Invalid, 1: Walking, 2: Offmesh
				// SLICE_LOG("Agent ID: {}, State: {}, Vel: ({}, {}, {}), SpeedParam: {}", 
				//    agent.crowdAgentID, (int)ag->state, desiredVel.x, desiredVel.y, desiredVel.z, ag->params.maxSpeed);

				auto rb = reg.try_get<RigidBody>(entity);
				auto &physicsSys = SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::PhysicsSystem>();


				if (rb)
				{
					// 3. APPLY PHYSICS
					glm::vec3 currentPhysicsVel = physicsSys.GetLinearVelocity(entity);

					JPH::Vec3 newVelocity(
						desiredVel.x,
						currentPhysicsVel.y,
						desiredVel.z
					);

					// Force wake up the body just in case
					// physicsSys.ActivateBody(entity); // Use if available
					physicsSys.SetLinearVelocity(entity, newVelocity);

					// 4. SYNC BACK
					float pos[3] = { transform.position.x, transform.position.y, transform.position.z };
					if (editableAg)
					{
						memcpy(editableAg->npos, pos, sizeof(float) * 3);
					}
				}
				else
				{
					// Fallback for non-physics objects
					transform.position.x = ag->npos[0];
					transform.position.y = ag->npos[1];
					transform.position.z = ag->npos[2];
				}
			}
			//}

			//if (agent.hasNewTarget)
			//{
			//	SLICE_LOG_DEBUG("Agent computing path from {} to {}");
			//	glm::vec3 start = transform.position;
			//	glm::vec3 end = agent.target;

			//	agent.currentPath.clear();
			//	// to do : change this when we start using the nav mesh instance
			//	//nav->FindPath(&start.x, &end.x, agent.currentPath);
			//	if (NavMeshUtilities::FindPath(navMeshObj, &start.x, &end.x, agent.currentPath))
			//	{
			//		agent.hasNewTarget = false;
			//		agent.currentPathIndex = 0;

			//		// Clear old path
			//		if (activePathDebugInfo.has_value())
			//		{
			//			auto &data = activePathDebugInfo.value().data;
			//			if (data[0].vao) glDeleteVertexArrays(1, &data[0].vao);
			//			if (data[0].vbo) glDeleteBuffers(1, &data[0].vbo);
			//			activePathDebugInfo.reset();
			//		}
			//		// Create new path debug
			//		activePathDebugInfo = std::make_optional<NavMeshDebugObj>(NavMeshUtilities::CreateDebugPathMesh(agent.currentPath));
			//		// -----------------------------------------
			//	}
			//}

			//if (!agent.currentPath.empty())
			//{
			//	glm::vec3 targetPt = agent.currentPath[agent.currentPathIndex];
			//	glm::vec3 currentPos = transform.position;

			//	glm::vec3 flatTarget(targetPt.x, 0.0f, targetPt.z);
			//	glm::vec3 flatCurrent(currentPos.x, 0.0f, currentPos.z);

			//	if (glm::distance(flatCurrent, flatTarget) < 0.01f)
			//	{
			//		if (glm::distance(currentPos, targetPt) < 0.15f)
			//		{
			//			agent.currentPathIndex++;
			//			if (agent.currentPathIndex >= agent.currentPath.size())
			//				agent.currentPath.clear();
			//		}
			//		return;
			//	}

			//	glm::vec3 dir = glm::normalize(flatTarget - flatCurrent);

			//	glm::vec3 nextPos = currentPos + (dir * agent.speed * dt);

			//	float height = 0.0f;
			//	if (NavMeshUtilities::GetNavMeshHeightAtPos(navMeshObj, nextPos, height))
			//	{
			//		nextPos.y = height;
			//	}

			//	transform.position = nextPos;
			//}
		}
	}
}