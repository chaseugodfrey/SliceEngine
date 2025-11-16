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

#include <glm/gtc/type_ptr.hpp>

namespace SliceEngine {

	void CanvasSystem::Init() {
		glCreateFramebuffers(1, &fbo);
	}
	void CanvasSystem::Release() {
		glDeleteFramebuffers(1, &fbo);
	}

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

	void CanvasSystem::UpdateHierachy() {
		//list of pair of entity and what type of rendering - split into 2 funcs for now
		//std::vector<std::pair<Entity, int>> entities_to_draw;

		auto core = Core::GetInstance();
		auto view = core->GetRegistry().view<canvasEntity>();

		RectTransform empty{};	//zeroed out rect transform for canvas elements to reference from
		empty.final_height = target_height; empty.final_width = target_width;
		empty.width = 0; empty.height = 0;

		for (auto entity : view) {
			auto const& canvas = mRegistry->get<Canvas>(entity);
			get_child_ui(/*entities_to_draw, */canvas, empty, entity);
		}
	}

	void CanvasSystem::DrawOverlay() {
		//rendermanager resets all settings so i can just set it here too
		CheckGLError();
		glDisable(GL_DEPTH_TEST);
		CheckGLError();

		auto core = Core::GetInstance();
		auto view = core->GetRegistry().view<canvasEntity>();

		std::vector<Entity> overlay_canvas{};
		for (auto entity : view) {
			auto const& canvas = mRegistry->get<Canvas>(entity);
			if (canvas.canvas_type == Canvas::OVERLAY) {
				overlay_canvas.push_back(entity);
			}
		}

		std::sort(overlay_canvas.begin(), overlay_canvas.end(), [&](Entity lhs, Entity rhs) {
			auto const& l_canvas = mRegistry->get<Canvas>(lhs);
			auto const& r_canvas = mRegistry->get<Canvas>(rhs);

			return l_canvas.sort_order < r_canvas.sort_order;
			});

		for (auto entity : overlay_canvas) {
			auto const& canvas = mRegistry->get<Canvas>(entity);
			std::vector<std::pair<Entity, GUID>> entities_to_draw{};
			//Get the entities to be drawn
			get_node_render(entities_to_draw, entity);
			//sort - skip for now
			//draw
			render_ui_overlay(entity, entities_to_draw);
		}
	}

	void CanvasSystem::render_ui_overlay(Entity canvas, std::vector<std::pair<Entity, GUID>> const& elements) {
		if (elements.empty()) {
			return;
		}
		/*
		* Things to note:
		* currently only the last camera that was added in scene view is used as camera,(GameViewWindow.cpp)
		* this camera is the very first entity within the view(idk why its a stack)
		* 
		* the camera that is used for editor is accessed via scene camera (SceneViewWindow.cpp)
		* 		auto& cam = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(go.GetEntity());
				camObj = std::make_unique<SceneCamera>(go.GetEntity(), go, cam);
		* 
		* for now just draw game camera, deal with scene view later
		*/
		auto core = SliceEngine::Core::GetInstance();
		auto view = core->GetRegistry().view<SliceEngine::Camera>();

		if (view.size() == 0)
		{
			return;
		}

		auto first_cam = *view.begin();
		if (!core->GetRegistry().any_of<SliceEngine::SceneGraph>(first_cam)) {
			return;	//not a game camera
		}
		
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		auto& cam = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(first_cam);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cam.textureID, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			return;
		}

		glViewport(0, 0, cam.width, cam.height);
		CheckGLError();





		//currently locked target width/height
		glm::mat4 canvas_to_ndc = glm::scale(glm::identity<glm::mat4>(), glm::vec3{ 2.f / target_width, 2.f / target_height, 1.f });

		GUID shader_guid = elements[0].second;
		GLuint shader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>(shader_guid).get()->s;
		glUseProgram(shader);
		CheckGLError();
		int uniform_loc = glGetUniformLocation(shader, "canvas_to_ndc");
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(canvas_to_ndc));
		CheckGLError();

		for (auto const& element : elements) {
			if (element.second != shader_guid) {
				shader_guid = element.second;
				shader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>(shader_guid).get()->s;
				glUseProgram(shader);
				int uniform_loc = glGetUniformLocation(shader, "canvas_to_ndc");
				glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(canvas_to_ndc));
				CheckGLError();
			}

			auto const& rect = mRegistry->get<RectTransform>(element.first);
			uniform_loc = glGetUniformLocation(shader, "M");
			glm::mat4 model = rect.ToMatrix();
			glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(model));
			CheckGLError();

			if (shader_guid == (GUID)15255338910698563845) {	//sprite
				auto const& sprite = mRegistry->get<SpriteRenderer>(element.first);
				auto id = sprite.textureHandle.get()->texture_id;
				glBindTextureUnit(0, sprite.textureHandle.get()->texture_id);
				uniform_loc = glGetUniformLocation(shader, "rgba");
				glUniform4fv(uniform_loc, 1, glm::value_ptr(sprite.rgba));
				CheckGLError();

				//Get quad
				auto& quad = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT).get();
				auto const& quad_mesh = quad.meshes[0];
				glBindVertexArray(quad_mesh.vao);
				glDrawElements(quad_mesh.drawMode, quad_mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
				CheckGLError();
			}
			else if (shader == 2) {	//font

			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		CheckGLError();
	}

	void CanvasSystem::get_node_render(std::vector<std::pair<Entity, GUID>>& render, Entity node) {
		if (!mRegistry->any_of<RectTransform>(node)) {
			return;
		}

		if (auto sprite = mRegistry->try_get<SpriteRenderer>(node)) {
			render.push_back({ node, (GUID)15255338910698563845 });	//eid and shader resource handle
		}
		//if (auto font = mRegistry->try_get<FontRenderer>(node)) {
		//	render.push_back({ node, 2 });
		//}

		if (auto scene_graph = mRegistry->try_get<SceneGraph>(node)) {
			entt::entity child = scene_graph->neighbours[SceneGraph::DOWN];
			while (child != entt::null)
			{
				get_node_render(render, child);
				child = mRegistry->get<SceneGraph>(child).neighbours[SceneGraph::RIGHT];
			}
		}
	}

	void CanvasSystem::get_child_ui(/*std::vector<std::pair<Entity, int>>& entities_to_draw,*/
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


		if (auto scene_graph = mRegistry->try_get<SceneGraph>(node)) {
			entt::entity child = scene_graph->neighbours[SceneGraph::DOWN];
			while (child != entt::null)
			{
				get_child_ui(/*entities_to_draw, */ctx, rect, child);
				child = mRegistry->get<SceneGraph>(child).neighbours[SceneGraph::RIGHT];
			}
		}
	}

	glm::mat4 RectTransform::ToMatrix() const {
		return {
			{final_width, 0.f, 0.f, 0.f},
			{0.f, final_height, 0.f, 0.f},
			{0.f, 0.f, 1.f, 0.f},
			{final_x, final_y, 0.f, 1.f}
		};
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


	void _CheckGLError(const char* file, int line)
	{
#ifndef _DEBUG
		return;
#endif // only do this on debug

		GLenum err(glGetError());

		while (err != GL_NO_ERROR)
		{
			std::string error;
			switch (err)
			{
			case GL_INVALID_OPERATION:  error = "INVALID_OPERATION";      break;
			case GL_INVALID_ENUM:       error = "INVALID_ENUM";           break;
			case GL_INVALID_VALUE:      error = "INVALID_VALUE";          break;
			case GL_OUT_OF_MEMORY:      error = "OUT_OF_MEMORY";          break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
			}
			std::cout << "GL_" << error.c_str() << " - " << file << ":" << line << std::endl;
			err = glGetError();
		}

		return;
	}