/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			TransformSystem.cpp
 author:		Chase Roderigues
 email:			roderigues.i@digipen.edu
 brief:			Handles Transformations of Game Objects

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "TransformSystem.h"
#include <Core/Core.h>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"

namespace SliceEngine
{
	void TransformSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		auto& transform = reg.get<Transform>(entity);
		if (transform.scale.x == 0.f && transform.scale.y == 0.f && transform.scale.z == 0.f)
		{
			transform.scale = glm::vec3(1.f);
		}
			
	}
	void TransformSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
	}
	void TransformSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		auto& tr = reg.get<Transform>(entity);
		
		// -------------------------------------------------------------
		// Calc the Transformation Matrix
		// -------------------------------------------------------------
		glm::mat4x4 M(1.f);
		M = glm::translate(M, tr.position);
		M *= glm::mat4_cast(tr.rotation);
		//M *= glm::eulerAngleXYZ(glm::radians(tr.rotation.x), glm::radians(tr.rotation.y), glm::radians(tr.rotation.z));
		M = glm::scale(M, tr.scale);

		tr.transform_local = M;
	}

	void TransformSystem::UpdateTransforms()
	{
		UpdateSceneTransforms(Core::FactoryInstance.GetRootEntity(), glm::mat4(1.0f));
		UpdateEngineEntityTransforms();
	}

	void TransformSystem::UpdateEngineEntityTransforms()
	{
		auto entities = mRegistry->view<EngineEntity>();

		for (auto entity : entities)
		{
			auto& tr = mRegistry->get<Transform>(entity);
			tr.transform = tr.transform_local;
		}

	}

	void TransformSystem::UpdateSceneTransforms(entt::entity entity, const glm::mat4& parentWorld)
	{
		auto& tr = mRegistry->get<Transform>(entity);
		tr.transform = parentWorld * tr.transform_local;
		
	    if (auto scene_graph = mRegistry->try_get<SceneGraph>(entity)) {
	        entt::entity child = scene_graph->neighbours[SceneGraph::DOWN];
	        while (child != entt::null) 
			{
	            UpdateSceneTransforms(child, tr.transform);
	            child = mRegistry->get<SceneGraph>(child).neighbours[SceneGraph::RIGHT];
	        }
	    }
	}
}