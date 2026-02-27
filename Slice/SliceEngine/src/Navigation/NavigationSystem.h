/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        NavigationSystem.h

 author:      Crystal Koh Qiao Wei

 email:       k.crystalqiaowei@digipen.edu

 brief:       Defines the NavigationSystem class and related structures for handling pathfinding 
              and AI movement within the engine. This system manages the loading, rendering, 
              and updating of Navigation Meshes (NavMesh), calculates paths, and updates 
              NavAgent entities to navigate the environment.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


#pragma once
#ifndef NAVIGATION_SYSTEM_H
#define NAVIGATION_SYSTEM_H

#include "ECS/BaseSystem.h"
#include "ECS/ECSTypes.h"
#include "NavMeshUtilities.h"

namespace SliceEngine
{

	struct NavAgentEntity {};

	class NavigationSystem : public BaseSystem<NavAgentEntity, NavAgent, Transform>
	{
		std::optional<NavMeshObj> navMeshInstance;
		std::optional<NavMeshDebugObj> navMeshDebugInfo;

		std::optional<NavMeshDebugObj> activePathDebugInfo;

		std::unordered_map<int, glm::vec3> m_agentOffMeshTargets;

	public:

		/**
		* @brief Initializes the navigation system and any core resources.
		*/
		void Init();

		/**
		* @brief Updates the navigation system, including all active NavAgents.
		* * @param dt The delta time for the current frame.
		*/
		void Update(float dt);

		/**
		* @brief Cleans up and unbinds resources used by the navigation system.
		*/
		void Unbind() override;

		void ClearNavMesh();
		void LoadNavMeshOnSceneLoad(OnSceneLoadedEvent &e);
		void LoadNavMeshFromBake(NavMeshObj obj);
		void LoadNavMeshFromFile(const std::string &filePath = "");
		std::optional<NavMeshObj> &GetNavMeshObj();
		std::optional<NavMeshDebugObj> &GetNavMeshDebugData();

		/**
		* @brief Called when an entity with NavAgent and Transform components enters the system.
		*
		* @param reg    Reference to the ECS registry.
		* @param entity The entity that entered.
		*/
		void EntityOnEnter(entt::registry &reg, entt::entity entity) override;

		/**
		* @brief Called when an entity with NavAgent and Transform components exits the system.
		*
		* @param reg    Reference to the ECS registry.
		* @param entity The entity that exited.
		*/
		void EntityOnExit(entt::registry &reg, entt::entity entity) override;

		/**
		* @brief Updates a specific entity's navigation state per frame.
		*
		* @param reg    Reference to the ECS registry.
		* @param entity The entity being updated.
		* @param dt     The delta time for the current frame.
		*/
		void EntityOnUpdate(entt::registry &reg, entt::entity entity, float dt) override;

	private:
		void InitializeAgent(NavAgent &agent, const Transform &transform, NavMeshObj &navMeshObj);
		void UpdateAgentTarget(NavAgent &agent, NavMeshObj &navMeshObj);
		void ApplyAgentPhysics(entt::registry &reg, entt::entity entity, NavAgent &agent, Transform &transform, NavMeshObj &navMeshObj);
	};
}


#endif