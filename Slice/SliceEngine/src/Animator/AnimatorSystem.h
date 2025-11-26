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

	class AnimatorSystem : public BaseSystem<animatorEntity, Animator>
	{
	public:
		AnimatorSystem();
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;

		void UpdateAnimation(entt::registry& reg, entt::entity entity, Animator& animator, float dt);
		void BoneUpdate();
		void InitSystem();
		void InitAnimatorEntity(entt::registry& reg, entt::entity entity);
	};

	

}




#endif
