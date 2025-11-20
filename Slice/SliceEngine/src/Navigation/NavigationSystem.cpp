#include "pch.h"
#include "NavigationSystem.h"
#include "../Core/Core.h"

namespace SliceEngine
{

	void NavigationSystem::Init()
	{
		EventManager::GetInstance()->Subscribe<OnSceneLoadedEvent, &NavigationSystem::LoadNavMeshOnSceneLoad>(this);
	}

	//void NavigationSystem::Update(float dt)
	//{

	//}

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

		navMeshInstance.reset();
	}

	void NavigationSystem::LoadNavMeshOnSceneLoad(OnSceneLoadedEvent& e)
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
		auto&& newNavMesh = NavMeshUtilities::LoadNavMesh("Resources/output_navmesh.bin");
		if (newNavMesh.has_value())
		{
			ClearNavMesh();
			navMeshInstance = std::make_optional<NavMeshObj>(newNavMesh.value());
		}
	}

	std::optional<NavMeshObj>& NavigationSystem::GetNavMeshObj()
	{
		return navMeshInstance;
	}

	std::optional<NavMeshDebugObj>& NavigationSystem::GetNavMeshDebugData()
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

		auto& navMeshObj = navMeshInstance.value();
		auto &agent = reg.get<NavAgent>(entity);
		auto &transform = reg.get<Transform>(entity);

		if (agent.hasNewTarget)
		{
			SLICE_LOG_DEBUG("Agent computing path from {} to {}");
			glm::vec3 start = transform.position;
			glm::vec3 end = agent.target;

			agent.currentPath.clear();
			// to do : change this when we start using the nav mesh instance
			//nav->FindPath(&start.x, &end.x, agent.currentPath);
			if (NavMeshUtilities::FindPath(navMeshObj, &start.x, &end.x, agent.currentPath))
			{
				agent.hasNewTarget = false;
				agent.currentPathIndex = 0;
			}
		}

		if (!agent.currentPath.empty())
		{
			glm::vec3 targetPt = agent.currentPath[agent.currentPathIndex];
			glm::vec3 currentPos = transform.position;

			// 1. Calculate Direction ignoring Y (Height)
			// This prevents the "flying" effect and ensures we just move across the map horizontally
			glm::vec3 flatTarget(targetPt.x, 0.0f, targetPt.z);
			glm::vec3 flatCurrent(currentPos.x, 0.0f, currentPos.z);

			// Prevent NaN if we are effectively at the target
			if (glm::distance(flatCurrent, flatTarget) < 0.01f)
			{
				// Logic to increment path index (moved from below)
				if (glm::distance(currentPos, targetPt) < 0.15f)
				{
					agent.currentPathIndex++;
					if (agent.currentPathIndex >= agent.currentPath.size())
						agent.currentPath.clear();
				}
				return;
			}

			glm::vec3 dir = glm::normalize(flatTarget - flatCurrent);

			// 2. Move the agent's X and Z
			glm::vec3 nextPos = currentPos + (dir * agent.speed * dt);

			// 3. SNAP TO NAVMESH HEIGHT (The Critical Fix)
			// We need to query Detour to find exactly what the Y value is at 'nextPos.x, nextPos.z'
			float height = 0.0f;
			if (NavMeshUtilities::GetNavMeshHeightAtPos(navMeshObj, nextPos, height))
			{
				nextPos.y = height;
			}

			// 4. Apply the new position
			transform.position = nextPos;
		}

	}
}