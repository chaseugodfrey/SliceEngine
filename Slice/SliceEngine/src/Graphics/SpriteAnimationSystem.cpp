/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			SpriteAnimationSystem.cpp
 author:		Elton Leosantosa
 email:			leosantosa.e@digipen.edu
 brief:			Sprite Animation System for animating sprite renderers

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "../Core/Core.h"
#include "SpriteAnimationSystem.h"

#include <glm/gtc/type_ptr.hpp>

namespace SliceEngine {

	void SpriteAnimationSystem::EntityOnExit(entt::registry& reg, entt::entity entity) {
		if (auto render = reg.try_get<SpriteRenderer>(entity)) {
			render->uv = glm::vec4(0.f, 1.f, 0.f, 1.f);

		}
	};

	void SpriteAnimationSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) {
		auto& sprite_anim = reg.get<SpriteAnimator>(entity);
		if (!sprite_anim.row || !sprite_anim.col || !sprite_anim.num_frames) {
			return;
		}
		sprite_anim.curr_frame += sprite_anim.fps * dt;
		while (sprite_anim.curr_frame > sprite_anim.num_frames) {
			sprite_anim.curr_frame -= sprite_anim.num_frames;
		}

		auto& render = reg.get<SpriteRenderer>(entity);
		float x_offset = 1.f / sprite_anim.col;
		float y_offset = 1.f / sprite_anim.row;
		unsigned char frame = (unsigned char)sprite_anim.curr_frame;
		unsigned char u = frame % sprite_anim.row;
		unsigned char v = frame / sprite_anim.row;
		render.uv = glm::vec4(u * x_offset, (u + 1) * x_offset, v * y_offset, (v + 1) * y_offset);

	};
}