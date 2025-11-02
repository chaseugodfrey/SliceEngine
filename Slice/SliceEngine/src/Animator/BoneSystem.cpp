/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			BoneSystem.cpp
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			System that handles updating of scenegraph transforms from animated skeleton bones

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "BoneSystem.h"
#include "Resource/Skeleton.h"

#include "../Core/Core.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>	//just to get it working for now

namespace SliceEngine
{
	void BoneSystem::Update_Scenegraph() const {
		auto core = Core::GetInstance();
		auto view = core->GetRegistry().view<Bone_Entity>();

		for (auto entity : view)
		{
			auto const& bone = core->GetRegistry().get<Bone>(entity);
			Entity root_entity = bone.skeleton_root;
			if (root_entity == entity) {
				continue;
			}
			
			if (!core->GetRegistry().any_of<Animator>(root_entity)) {
				//SLICE_LOG_ERROR("Invalid root entity for bone component");
				continue;
			}

			auto& animator = core->GetRegistry().get<Animator>(root_entity);
			auto& transform = core->GetRegistry().get<Transform>(entity);

			//some pseudo code
			glm::mat4 const& frame = animator.GetFinalTform()[bone.frame_idx];
			glm::vec3 translation, scale, skew;
			glm::vec4 perspective;
			glm::quat rotation;
			glm::decompose(frame, scale, rotation, translation, skew, perspective);
			transform.position = translation;
			transform.rotation = rotation;
			transform.scale = scale;
			
			//if is a renderer, tell skeleton to calculate inverse for this index
			if (core->GetRegistry().any_of<Renderer>(entity)) {
				animator.inverse_flags.set(bone.frame_idx);
			}
		}
	}
}