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
	void BoneSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		Update_Bones(reg, entity);
	}

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

			//if (!animator.stateMachine.EFSM.IsValid()) return;
			if (!animator.IsValid()) continue;

			if (!animator.timeline.isPlaying)
				continue;

			if (animator.curr_anim_pkg.animations.size() < animator.stateMachine.EFSM.currState->curr_anim_idx)
				continue;

			//some pseudo code
			glm::mat4 const& frame = animator.GetFinalTform()[bone.frame_idx];
			glm::vec3 translation, scale, skew;
			glm::vec4 perspective;
			glm::quat rotation;
			glm::decompose(frame, scale, rotation, translation, skew, perspective);
			transform.position = translation;
			transform.rotation = rotation;
			transform.scale = scale;

			//glm::mat4x4 M(1.f);
			//M = glm::translate(M, translation);
			//M *= glm::mat4_cast(rotation);
			////M *= glm::eulerAngleXYZ(glm::radians(tr.rotation.x), glm::radians(tr.rotation.y), glm::radians(tr.rotation.z));
			//M = glm::scale(M, scale);

			//transform.transform_local = M;
			
			//if is a renderer, tell skeleton to calculate inverse for this index
			if (core->GetRegistry().any_of<Renderer>(entity)) {
				animator.inverse_flags.set(bone.frame_idx);
			}
		}
	}
	void BoneSystem::Update_Bones(entt::registry& reg, entt::entity entity)
	{
		auto core = Core::GetInstance();
		//auto view = core->GetRegistry().view<Bone_Entity>();

		//for (auto entity : view)
		{
			auto const& bone = core->GetRegistry().get<Bone>(entity);
			Entity root_entity = bone.skeleton_root;
			if (root_entity == entity) {
				return;
			}

			if (!core->GetRegistry().any_of<Animator>(root_entity)) {
				//SLICE_LOG_ERROR("Invalid root entity for bone component");
				return;
			}

			auto& animator = core->GetRegistry().get<Animator>(root_entity);
			//auto& transform = core->GetRegistry().get<Transform>(entity);

			if(animator.Handle_curr_anim_pkg.IsValid() && animator.Handle_skeleton.IsValid())
			//if(animator.IsValid())
			{
				if (Core::GetInstance()->GetRegistry().any_of<Renderer>(entity)) {
					animator.inverse_flags.set(bone.frame_idx);
					animator.SetInverseRoot(bone.frame_idx);
				}
			}

			

			//animator.SetInverseRoots();
		}
	}
}