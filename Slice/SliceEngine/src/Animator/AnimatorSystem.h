/*!
\file		AnimatorSystem.h
\author		Muhammad Rayan (muhammadrayan.b@digipen.edu)
\co-author
\brief
	Functions that deal with the Animator for animations

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/

#ifndef ANIMATORSYSTEM_H
#define	ANIMATORSYSTEM_H

#include "../Resource/Model.h"
#include "../Resource/Skeleton.h"
#include "../ECS/BaseSystem.h"
#include "../ECS/ECSTypes.h"
#include "FSMSystem.h"

namespace SliceEngine
{
	//include the real one once done
	struct animatorEntity {};

	class AnimatorSystem : BaseSystem<animatorEntity, Animator>
	{
	public:
		AnimatorSystem();
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;

		void UpdateAnimation(float dt);
		void BoneUpdate();

		//tbh these 2 set_x stuff shld be taking in a guid/handle to these resources, then creating and instance of it
		void SetAnimationPackage(SliceEngineTypes::AnimationPackage*);
		void SetSkeleton(SliceEngineTypes::Skeleton*);
		void PlayAnimation(unsigned int);

		std::vector<glm::mat4> const& GetFinalTform() const 
		{
			return final_tforms;
		}
		bool is_bone{ true };
		float current_time{};
	private:
		std::vector<glm::mat4> final_tforms;
		SliceEngineTypes::AnimationPackage* curr_anim_pkg{};
		SliceEngineTypes::Skeleton* skeleton{};
		unsigned int curr_anim_idx{};
	};

	

}




#endif
