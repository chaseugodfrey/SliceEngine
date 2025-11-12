/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			CanvasSystem.cpp
 author:		Elton Leosantosa
 email:			leosantosa.e@digipen.edu
 brief:			Canvas system for 2D rendering layouts

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "CanvasSystem.h"
#include "../Core/Core.h"

namespace SliceEngine {

	/*
	* allocate frame buffer, currently just do it naively, each canvas element will have its own framebuffer
	* even tho still redraw everything because there is no dirty flag
	*/
	void CanvasSystem::EntityOnEnter(entt::registry& reg, entt::entity entity) {
		auto& canvas = reg.get<Canvas>(entity);
		//glCreateFramebuffers(1, &canvas.fbo);
		//glnamedframebuffer

		//actually now that i think about it, dont really need a framebuffer, just draw it directly
	}
	void CanvasSystem::EntityOnExit(entt::registry& reg, entt::entity entity) {
		auto& canvas = reg.get<Canvas>(entity);
		//glDeleteFramebuffers(1, &canvas.fbo);
	}

	void CanvasSystem::UpdateAndDraw() {
		//list of pair of entity and what type of rendering
		std::vector<std::pair<Entity, int>> entities_to_draw;

		auto core = Core::GetInstance();
		auto view = core->GetRegistry().view<canvasEntity>();

		RectTransform empty{};	//zeroed out rect transform for canvas elements to reference from
		empty.final_height = target_height; empty.final_width = target_width;
		empty.width = 0; empty.height = 0;

		for (auto entity : view) {
			auto const& canvas = mRegistry->get<Canvas>(entity);
			get_child_ui(entities_to_draw, canvas, empty, entity);
		}


		//draw to screen - ill think about world next time, just deal with overlay, also no sorting order so i can get it working first
		
	}

	void CanvasSystem::get_child_ui(std::vector<std::pair<Entity, int>>& entities_to_draw,
		Canvas const& ctx, RectTransform const& parent, Entity node) {
		/*
		*	assumptions
		*	all children have rect transform
		*	if no rect transform return
		*/
		if (!mRegistry->any_of<RectTransform>(node)) {
			return;
		}
		auto& rect = mRegistry->get<RectTransform>(node);
		rect.Update(ctx, parent);

		//add node to list to draw first
		if (auto sprite = mRegistry->try_get<SpriteRenderer>(node)) {
			entities_to_draw.push_back({ node, 1 });
		}
		
		//if(auto font = mRegistry->try_get<FontRenderer>(node))
		//entities_to_draw.push_back({ node, 2 });


		if (auto scene_graph = mRegistry->try_get<SceneGraph>(node)) {
			entt::entity child = scene_graph->neighbours[SceneGraph::DOWN];
			while (child != entt::null)
			{
				get_child_ui(entities_to_draw, ctx, rect, child);
				child = mRegistry->get<SceneGraph>(child).neighbours[SceneGraph::RIGHT];
			}
		}
	}

	void RectTransform::Update(Canvas const& ctx, RectTransform const& parent) {
		const int parent_x = parent.final_x;
		const int parent_y = parent.final_y;
		const int parent_width = parent.final_width;
		const int parent_height = parent.final_height;
		//x axis
		const int half_width = parent_width / 2;

		const int parent_left = parent_x - half_width;
		const int parent_right = parent_x + half_width;

		//if (old_hori != hori_pivot) {
		//	old_hori = hori_pivot;
		//}
		//if (old_vert != vert_pivot) {
		//	old_vert = vert_pivot;
		//}

		if (hori_pivot == HoriPivot::STRETCH_H) {
			const int left_ref = parent_left + left;	//apply left pad
			const int right_ref = parent_right - right;	//apply right pad

			final_width = right_ref - left_ref;
			final_x = left_ref + final_width / 2;
		}
		else {
			final_width = width;
			switch (hori_pivot) {
			case LEFT:
				final_x = parent_left + pos_x;
				break;
			case CENTER:
				final_x = parent_x + pos_x;
				break;
			case RIGHT:
				final_x = parent_right + pos_x;
				break;
			}
		}

		//y axis
		const int half_height = parent_height / 2;
		const int parent_top = parent_y + half_height;
		const int parent_bot = parent_y - half_height;
		if (vert_pivot == VertPivot::STRETCH_V) {
			const int top_ref = parent_top - top;		//apply top pad
			const int bot_ref = parent_bot + bot;		//apply bot pad

			final_height = top_ref - bot_ref;
			final_y = bot_ref + final_height / 2;
		}
		else {
			final_height = height;
			switch (vert_pivot) {
			case TOP:
				final_y = parent_top + pos_y;
				break;
			case MIDDLE:
				final_y = parent_y + pos_y;
				break;
			case BOTTOM:
				final_y = parent_bot + pos_y;
				break;
			}
		}
	}
}