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
		if (!navMeshInstance)
			return;

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
			SLICE_LOG_DEBUG("Path size = {}", agent.currentPath.size());
			auto targetPt = agent.currentPath[agent.currentPathIndex];
			glm::vec3 dir;

			// on the scenario that the target position IS it's own position
			// maybe because the start of path finding starts from their own position
			// so to prevent NAN from normalizing a zero vector, we set it to 0 if its the same pos
			if (targetPt == transform.position)
			{
				dir = glm::vec3(0, 0, 0);
			}
			else
			{
				dir = glm::normalize(targetPt - transform.position);
			}

			transform.position += dir * agent.speed * dt;

			if (glm::distance(transform.position, targetPt) < 0.15f)
			{
				agent.currentPathIndex++;
				if (agent.currentPathIndex >= agent.currentPath.size())
					agent.currentPath.clear();
			}
		}

	}
}