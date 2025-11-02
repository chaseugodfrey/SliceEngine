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
namespace SliceEngine
{
	void BoneSystem::Update_Scenegraph() const {
		auto core = Core::GetInstance();
		auto view = core->GetRegistry().view<Bone_Entity>();

		for (auto entity : view)
		{
			auto const& bone = core->GetRegistry().get<Bone>(entity);
			Entity root_entity = bone.skeleton_root;
			
			if (!core->GetRegistry().any_of<Animator>(root_entity)) {
				SLICE_LOG_ERROR("Invalid root entity for bone component");
				continue;
			}

			auto const& animator = core->GetRegistry().get<Animator>(root_entity);
			auto& transform = core->GetRegistry().get<Transform>(entity);

			//some pseudo code
			/*
			* SliceEngineTypes::Frame const& frame = animator.skeleton.getframe(bone.frame_idx);
			* transform.position = frame.position;
			* transform.rotation = frame.rotation;
			* transform.scale = frame.scale;
			*/

			//if is a renderer, tell skeleton to calculate inverse for this index
			if (core->GetRegistry().any_of<Renderer>(entity)) {
				//animator.skeleton.setinverseflag(bone.frame_idx);
			}
		}
	}
}