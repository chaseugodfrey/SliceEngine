#include "pch.h"
#include "NavigationSystem.h"
#include "../Core/Core.h"

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
			LoadNavMeshFromFile();
		}
	}

	void NavigationSystem::LoadNavMeshFromBake(NavMeshObj newNavMesh)
	{
		ClearNavMesh();
		navMeshInstance = std::make_optional<NavMeshObj>(newNavMesh);
		navMeshDebugInfo = std::make_optional<NavMeshDebugObj>(NavMeshUtilities::CreateDebugMesh(newNavMesh));
	}

	void NavigationSystem::LoadNavMeshFromFile()
	{
		auto &&newNavMesh = NavMeshUtilities::LoadNavMesh("Resources/output_navmesh.bin");
		if (newNavMesh.has_value())
		{
			ClearNavMesh();
			navMeshInstance = std::make_optional<NavMeshObj>(newNavMesh.value());
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
			float extents[3] = { 2.0f, 4.0f, 2.0f };
			dtPolyRef targetRef;
			float targetPosOnMesh[3];
			dtQueryFilter filter;

			// Find nearest polygon to target
			navMeshObj.navMeshQuery->findNearestPoly(targetPos, extents, &filter, &targetRef, targetPosOnMesh);

			if (targetRef)
			{
				// Tell Crowd Agent to move there
				navMeshObj.navMeshCrowd->requestMoveTarget(agent.crowdAgentID, targetRef, targetPosOnMesh);
				agent.hasNewTarget = false;
			}
		}
		if (agent.crowdAgentID != -1)
		{
			const dtCrowdAgent *ag = navMeshObj.navMeshCrowd->getAgent(agent.crowdAgentID);
			if (ag && ag->active)
			{
				transform.position.x = ag->npos[0];
				transform.position.y = ag->npos[1];
				transform.position.z = ag->npos[2];
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