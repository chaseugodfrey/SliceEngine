#include <pch.h>
#include "TransformSystem.h"

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
			transform.previousScale = transform.scale;
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
		M *= glm::eulerAngleXYZ(glm::radians(tr.rotation.x), glm::radians(tr.rotation.y), glm::radians(tr.rotation.z));
		M = glm::scale(M, tr.scale);

		tr.transform_local = M;
		tr.transform = tr.transform_local;

		if (auto scene_graph = reg.try_get<SceneGraph>(entity)) {
			auto parent_entity = scene_graph->neighbours[SceneGraph::UP];
			if (parent_entity != entt::null && parent_entity != entt::entity{0}) {
				auto& parent_tr = reg.get<Transform>(parent_entity);
				tr.transform = parent_tr.transform * tr.transform_local;
			}
		}
	}

}