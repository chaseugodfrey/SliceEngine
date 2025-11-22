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

		glCreateTextures(GL_TEXTURE_2D, 1, &raycast_tex);
		CheckGLError();

		glTextureStorage2D(raycast_tex, 1, GL_R32UI, target_width, target_width);
		glTextureParameteri(raycast_tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(raycast_tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		CheckGLError();
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

		//clear the raycast buffer to entt null
		const GLuint null_eid = entt::null;
		glClearTexImage(raycast_tex, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &null_eid);
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

		glDisable(GL_BLEND);	//idk ngl why this needs to be here, means i need to predict the settings(?)
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
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, raycast_tex, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			return;
		}

		GLenum render_targets[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, render_targets);
		glViewport(0, 0, cam.width, cam.height);
		CheckGLError();




		auto const& rm = Core::GetInstance()->GetResourceManager();

		auto const& canv = core->GetRegistry().get<Canvas>(canvas);
		glm::mat4 canvas_to_ndc = glm::scale(glm::identity<glm::mat4>(), glm::vec3{ 2.f / cam.width, 2.f / cam.height, 1.f });

		GUID shader_guid = elements[0].second;
		GLuint shader = rm->get<SliceEngineTypes::Shader>(shader_guid).get()->s;
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
				shader = rm->get<SliceEngineTypes::Shader>(shader_guid).get()->s;
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

			if (shader_guid == (GUID)15255338910698563845) {	//sprite
				auto const& sprite = mRegistry->get<SpriteRenderer>(element.first);
				auto const& res = rm->get<SliceEngineTypes::Texture>(sprite.textureHandle);
				//auto id = sprite.textureHandle.get()->texture_id;
				glBindTextureUnit(0, res.get()->texture_id);
				uniform_loc = glGetUniformLocation(shader, "rgba");
				glUniform4fv(uniform_loc, 1, glm::value_ptr(sprite.rgba));
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


	//Set the color/sprite guid of the image depending on state
	void ButtonSystem::update_button(Entity button_entity, Events event) {
		auto& button = mRegistry->get<Button>(button_entity);

		switch (event) {
		case Highlight:
			button.state = Button::Highlighted;
			break;
		case Click: {
			button.state = Button::Pressed;
			OnButtonClickEvent event;
			event.entity = button_entity;
			EventManager::GetInstance()->Publish<OnButtonClickEvent>(event);
		}
			break;
		case LeaveHighlight:
			button.state = Button::Normal;
			break;
		case Release: {
			button.state = Button::Normal;

			OnButtonReleaseEvent event;
			event.entity = button_entity;
			EventManager::GetInstance()->Publish<OnButtonReleaseEvent>(event);
		}
			break;
		case Cancel:
			std::cout << "Cancel event" << std::endl;
			button.state = Button::Normal;
			break;
		}

		;	//change to target graphic if we doing that feature
		if (auto image = mRegistry->try_get<SpriteRenderer>(button_entity))
		{
			switch (button.transition) {
				//no dirty flag for now
				//also not going to keep a 'local' copy of color/tex in sprite renderer
				//just reset it if button component gets removed, also this case is super rare
			case Button::Color:
				image->rgba = button.color_transitions[button.state];
				break;
			case Button::Sprite:
				image->textureHandle = button.sprite_transitions[button.state];
				break;
			}
		}
		

	}

	/*
	* there are 2 possibilities
	* mouse is down/up
	* 
	* 4 events to care about:
	* highlight - mouse was up and hovering a button
	* click		- mouse was up, but pressed down while hovering a button
	* 
	* cancel	- mouse clicked a button, but released outside of button
	* release	- mouse clicked a button, and released inside of button
	*/
	void ButtonSystem::HandleMouse(InputSystem& input, CanvasSystem const& canvas) {
		Entity temp_button = entt::null;
		ButtonSystem::Events mouse_event = Events::None;

		//somehow convert to pixel coord
		glm::vec2 mouse_coord = input.GetMousePosition();
		//std::cout << "mouse coord: " << mouse_coord.x << ", " << mouse_coord.y << std::endl;
		//for now im just gona directly convert to game screen coord
		unsigned int mouse_x = (unsigned int)mouse_coord.x;
		unsigned int mouse_y = CanvasSystem::target_height - (unsigned int)mouse_coord.y;

		//for now im gona use a key to simulate mouse clicks

		if (current_button == entt::null) {
			temp_button = canvas.Raycast(mouse_x, mouse_y);
			if (temp_button == entt::null || !mRegistry->any_of<Button>(temp_button)) {
				return;
			}
			auto& t_button = mRegistry->get<Button>(temp_button);
			if (!input.IsMouseDown(MouseButtons::LEFT)) {		//hover
				update_button(temp_button, Highlight);
				current_button = temp_button;
			}
			else if (input.IsMousePressed(MouseButtons::LEFT)) {	//click same frame u hover
				update_button(temp_button, Click);
				current_button = temp_button;
			}
		}
		else {
			auto& c_button = mRegistry->get<Button>(current_button);

			if (c_button.state == Button::Highlighted) {
				temp_button = canvas.Raycast(mouse_x, mouse_y);
				if (!input.IsMouseDown(MouseButtons::LEFT)) {
					if (temp_button != current_button) {
						update_button(current_button, LeaveHighlight);
						current_button = entt::null;
					}
				}
				else {
					if (temp_button == current_button) {
						update_button(current_button, Click);
					}
					else {
						update_button(current_button, LeaveHighlight);
						if (mRegistry->any_of<Button>(temp_button)) {
							auto& t_button = mRegistry->get<Button>(temp_button);
							update_button(temp_button, Click);	//click same frame u leave highlight
							current_button = temp_button;
						}
						else {
							current_button = entt::null;
						}
					}
				}
			}
			else {
				if (input.IsMouseReleased(MouseButtons::LEFT)) {
					temp_button = canvas.Raycast(mouse_x, mouse_y);
					if (temp_button != current_button) {
						update_button(current_button, Cancel);
					}
					else {
						update_button(current_button, Release);
					}
					current_button = entt::null;
				}
			}







			//if (input.IsKeyReleased(Keys::KEY_BACKSLASH)) {
			//	if (temp_button == current_button) {
			//		update_button(c_button, Release);
			//	}
			//	else if (c_button.state == Button::Pressed) {
			//		update_button(c_button, Cancel);
			//	}
			//	else {
			//		update_button(c_button, LeaveHighlight);
			//	}
			//}
			//else {
			//	auto& t_button = mRegistry->get<Button>(temp_button);

			//	if (current_button == temp_button) {
			//		if (c_button.state == Button::Highlighted) {
			//			mouse_event = Click;
			//		}
			//	}

			//	if (current_button != temp_button) {
			//		if (c_button.state == Button::Highlighted) {
			//			update_button(c_button, Events::LeaveHighlight);
			//			update_button(t_button, Events::Click);
			//		}
			//	}
			//}
		}


	//	update_button();
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