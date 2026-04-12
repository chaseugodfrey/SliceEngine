/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			SpriteAnimationSystem.h
 author:		Elton Leosantosa
 email:			leosantosa.e@digipen.edu
 brief:			Sprite Animation System for UI sprite renderers
				Uses sprite renderer's texture as a spritesheet

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef SPRITE_SYSTEM_H
#define SPRITE_SYSTEM_H

#include "../ECS/BaseSystem.h"
#include "../ECS/ECSTypes.h"

extern void _CheckGLError(const char* file, int line);

#define CheckGLError() _CheckGLError(__FILE__, __LINE__)

namespace SliceEngine
{
	struct spriteanimEntity {};

	//struct RectTransform;
	struct SpriteAnimationSystem : BaseSystem<spriteanimEntity, RectTransform, SpriteAnimator, SpriteRenderer>
	{
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override {};
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;

	};
}

#endif