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
#include "CameraSystem.h"

#include <glm/gtc/type_ptr.hpp>

namespace SliceEngine {

	namespace {
		uint64_t sprite_shader = 11505317983061001815;
		uint64_t ui_sprite_eid = 13043535478215287923;
		uint64_t font_shader = 0;
		uint64_t ui_font_eid = 0;
	}

	void CanvasSystem::Init() {
		glCreateFramebuffers(1, &fbo);

		glCreateTextures(GL_TEXTURE_2D, 1, &raycast_tex);
		CheckGLError();

		glTextureStorage2D(raycast_tex, 1, GL_R32UI, target_width, target_height);
		glTextureParameteri(raycast_tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(raycast_tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		CheckGLError();

		auto core = Core::GetInstance();
		auto rm = core->GetResourceManager();
		sprite_shader = rm->mFileNameToGUID.at("Shaders/uiSprite.shader").GetGUID();
		ui_sprite_eid = rm->mFileNameToGUID.at("Shaders/uiSpriteEID.shader").GetGUID();
		font_shader = rm->mFileNameToGUID.at("Shaders/uiFont.shader").GetGUID();
		//ui_sprite_eid = rm->mFileNameToGUID.at("Shaders/uiSpriteEID.shader").GetGUID();
		
		eid_shader_map[sprite_shader] = ui_sprite_eid;
		eid_shader_map[font_shader] = ui_font_eid;
	}
	void CanvasSystem::Release() {
		glDeleteTextures(1, &raycast_tex);
		glDeleteFramebuffers(1, &fbo);
		CheckGLError();
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

	//thrs prob a more efficient way for this but not gona bother for now
	Entity CanvasSystem::Raycast(unsigned int x, unsigned int y) const {
		//simply check the raycast texture and sample
		if (x >= target_width || y >= target_height) {
			return entt::null;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		CheckGLError();
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, raycast_tex, 0);
		CheckGLError();

		glReadBuffer(GL_COLOR_ATTACHMENT1);
		CheckGLError();
		Entity e = entt::null;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &e);
		CheckGLError();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return e;
	}

	void CanvasSystem::DrawOverlay() {
		//rendermanager resets all settings so i can just set it here too
		CheckGLError();
		glDisable(GL_DEPTH_TEST);
		CheckGLError();

		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);	//need to make this premultiplied(one day) - maybe inside texture compiler
		CheckGLError();
		const GLuint null_eid = entt::null;

		//clear the raycast buffer to entt null

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

		auto const& cam_sys = core->GetSystem<CameraSystem>();
		auto main_cam = cam_sys.mainCam.value_or(entt::null);
		if (main_cam == entt::null) {
			return;	//not a game camera
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		auto& cam = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(main_cam);
		glViewport(0, 0, cam.width, cam.height);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cam.textureID, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, raycast_tex, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			return;
		}

		GLenum render_targets[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		GLenum render_color[] = {GL_COLOR_ATTACHMENT0};
		GLenum render_eid[] = {GL_COLOR_ATTACHMENT1};

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



		std::vector<std::pair<Entity, uint64_t>> entities_to_draw{};
		for (auto entity : overlay_canvas) {
			auto const& canvas = mRegistry->get<Canvas>(entity);
			if (canvas.componentEnabled) {
				//Get the entities to be drawn
				get_node_render(entities_to_draw, entity);
			}
		}

		//glDrawBuffers(1, render_color);
		glDrawBuffers(2, render_targets);
		CheckGLError();
		for (auto entity : overlay_canvas) {
			render_ui_overlay(entity, main_cam, entities_to_draw);
		}
		CheckGLError();

		glClearTexImage(raycast_tex, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &null_eid);
		CheckGLError();
		//glDrawBuffers(1, render_eid);
		CheckGLError();
		for (auto entity : overlay_canvas) {
			render_ui_eids(entity, main_cam, entities_to_draw);
		}
		CheckGLError();

		glDisable(GL_BLEND);	//idk ngl why this needs to be here, means i need to predict the settings(?)


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void CanvasSystem::render_ui_overlay(Entity canvas, Entity camera, std::vector<std::pair<Entity, uint64_t>> const& elements) {
		if (elements.empty()) {
			return;
		}


		auto core = SliceEngine::Core::GetInstance();
		auto const& rm = core->GetResourceManager();

		auto& cam = core->GetRegistry().get<SliceEngine::Camera>(camera);
		auto const& canv = core->GetRegistry().get<Canvas>(canvas);

		glm::mat4 canvas_to_ndc = glm::scale(glm::identity<glm::mat4>(), glm::vec3{ 2.f / cam.width, 2.f / cam.height, 1.f });

		uint64_t shader_guid = elements[0].second;
		GLuint shader = rm->get<SliceEngineTypes::Shader>((GUID)shader_guid).get()->s;
		glUseProgram(shader);
		CheckGLError();
		int uniform_loc = glGetUniformLocation(shader, "canvas_to_ndc");
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(canvas_to_ndc));
		/*uniform_loc = glGetUniformLocation(shader, "raycast");
		glUniform1ui(uniform_loc, canv.graphic_raycastable);
		glBindTextureUnit(1, raycast_tex);
		CheckGLError();*/

		for (auto const& element : elements) {
			if (element.second != shader_guid) {
				shader_guid = element.second;
				shader = rm->get<SliceEngineTypes::Shader>((GUID)shader_guid).get()->s;
				glUseProgram(shader);
				uniform_loc = glGetUniformLocation(shader, "canvas_to_ndc");
				glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(canvas_to_ndc));
				/*uniform_loc = glGetUniformLocation(shader, "raycast");
				glUniform1ui(uniform_loc, canv.graphic_raycastable);*/
				CheckGLError();
			}

			auto const& rect = mRegistry->get<RectTransform>(element.first);
			uniform_loc = glGetUniformLocation(shader, "M");
			glm::mat4 model = rect.ToMatrix();
			glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(model));
			CheckGLError();

			if (shader_guid == sprite_shader) {	//sprite
				auto const& sprite = mRegistry->get<SpriteRenderer>(element.first);
				auto const& res = rm->get<SliceEngineTypes::Texture>(sprite.textureHandle);

				glBindTextureUnit(0, res.get()->texture_id);
				uniform_loc = glGetUniformLocation(shader, "rgba");
				glUniform4fv(uniform_loc, 1, glm::value_ptr(sprite.rgba));
				CheckGLError();

				//Get quad
				auto const& quad = *rm->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT).get();
				auto const& quad_mesh = quad.meshes[0];
				glBindVertexArray(quad_mesh.vao);
				glDrawElements(quad_mesh.drawMode, quad_mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
				CheckGLError();
			}
			else if (shader == font_shader) {	//font
				auto const& font_render = mRegistry->get<FontRenderer>(element.first);
				auto const& font = rm->get<SliceEngineTypes::Font_Data>(font_render.fontHandle);
			}
		}

		CheckGLError();
	}
	void CanvasSystem::render_ui_eids(Entity canvas, Entity camera, std::vector<std::pair<Entity, uint64_t>> const& elements) {
		if (elements.empty()) {
			return;
		}

		auto core = SliceEngine::Core::GetInstance();
		auto const& rm = core->GetResourceManager();

		auto& cam = core->GetRegistry().get<SliceEngine::Camera>(camera);
		auto const& canv = core->GetRegistry().get<Canvas>(canvas);

		glm::mat4 canvas_to_ndc = glm::scale(glm::identity<glm::mat4>(), glm::vec3{ 2.f / cam.width, 2.f / cam.height, 1.f });

		uint64_t shader_guid = elements[0].second;
		GLuint shader = rm->get<SliceEngineTypes::Shader>((GUID)eid_shader_map.at(shader_guid)).get()->s;
		glUseProgram(shader);
		CheckGLError();
		int uniform_loc = glGetUniformLocation(shader, "canvas_to_ndc");
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(canvas_to_ndc));
		uniform_loc = glGetUniformLocation(shader, "raycast");
		glUniform1ui(uniform_loc, canv.graphic_raycastable);
		glBindTextureUnit(1, raycast_tex);
		CheckGLError();

		for (auto const& element : elements) {
			if (element.second != shader_guid) {
				shader_guid = element.second;
				shader = rm->get<SliceEngineTypes::Shader>((GUID)eid_shader_map.at(shader_guid)).get()->s;
				glUseProgram(shader);
				uniform_loc = glGetUniformLocation(shader, "canvas_to_ndc");
				glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(canvas_to_ndc));
				uniform_loc = glGetUniformLocation(shader, "raycast");
				glUniform1ui(uniform_loc, canv.graphic_raycastable);
				CheckGLError();
			}

			auto const& rect = mRegistry->get<RectTransform>(element.first);
			uniform_loc = glGetUniformLocation(shader, "M");
			glm::mat4 model = rect.ToMatrix();
			glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(model));
			CheckGLError();

			if (shader_guid == sprite_shader) {	//sprite
				auto const& sprite = mRegistry->get<SpriteRenderer>(element.first);
				auto const& res = rm->get<SliceEngineTypes::Texture>(sprite.textureHandle);

				glBindTextureUnit(0, res.get()->texture_id);
				uniform_loc = glGetUniformLocation(shader, "raycast_target");
				glUniform1ui(uniform_loc, sprite.raycast_target);
				uniform_loc = glGetUniformLocation(shader, "entity");
				glUniform1ui(uniform_loc, (GLuint)element.first);
				uniform_loc = glGetUniformLocation(shader, "alphaThresh");
				glUniform1f(uniform_loc, sprite.alphathreshold);
				CheckGLError();

				//Get quad
				auto const& quad = *rm->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT).get();
				auto const& quad_mesh = quad.meshes[0];
				glBindVertexArray(quad_mesh.vao);
				glDrawElements(quad_mesh.drawMode, quad_mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
				CheckGLError();
			}
			else if (shader == font_shader) {	//font

			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		CheckGLError();
	}

	void CanvasSystem::get_node_render(std::vector<std::pair<Entity, uint64_t>>& render, Entity node) {
		auto core = SliceEngine::Core::GetInstance();
		auto const& rm = core->GetResourceManager();
		if (!mRegistry->any_of<RectTransform>(node)) {
			return;
		}
		auto* sprite = mRegistry->try_get<SpriteRenderer>(node);
		if (sprite && sprite->componentEnabled) {
			render.push_back({ node, sprite_shader });	//eid and shader resource handle
		}
		if (auto font = mRegistry->try_get<FontRenderer>(node)) {
			//GUID font_guid = rm->mFileNameToGUID["Shaders/uiFont.shader"];
			render.push_back({ node, font_shader });
		}

		if (auto scene_graph = mRegistry->try_get<SceneGraph>(node)) {
			entt::entity child = scene_graph->neighbours[SceneGraph::DOWN];
			while (child != entt::null)
			{
				get_node_render(render, child);
				child = mRegistry->get<SceneGraph>(child).neighbours[SceneGraph::RIGHT];
			}
		}
	}

	void CanvasSystem::get_child_ui(Canvas const& ctx, RectTransform const& parent, Entity node) {
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