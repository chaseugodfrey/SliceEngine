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

	namespace SliceEngineTypes {
		/*
		* a token is a string that should be treated as 1 unit when dealing with text wrapping
		* 
		* to handle wrapping in text box
		* first tokenize according to spaces and line breaks
		* calculate the size of each word and ensure the line does not exceed limit
		* 
		* potentially do this only when text gets changed, and calculate only size at runtime
		* see if this really cooks performance first
		*/
		void Tokenize(FontRenderer& font_render, Font_Data const& font) {
			float relative_size = font_render.font_size / font.font_size;
			
			auto const& font_text = font_render.text;
			std::vector<FontRenderer::Token> token_list{};
			token_list.reserve(50);	//probably less then 50 words and spaces in 1 component, just a heuristic

			for (size_t pos = 0; pos < font_text.size(); ++pos) {
				FontRenderer::Token token{};
				token.pos = (unsigned int)pos;

				const char* pattern = " \n\t";
				switch (font_text[pos]) {
				case '\n':
					token.size = 0;
					token.char_cnt = 1;
					break;
				case ' ':
					token.size = font.glyph_datas.at(font_text[pos]).advance * relative_size;
					token.char_cnt = 1;
					break;
				case '\t':
					token.size = font.glyph_datas.at(' ').advance * relative_size * 4;	//1 tab is 4 spaces
					token.char_cnt = 1;
					break;
				default:
					{
					size_t next = font_text.find_first_of(pattern, pos);
					if (next == std::string::npos) {
						next = font_text.size();
					}
					
					for (size_t ch = pos; ch < next; ++ch) {
						token.size += font.glyph_datas.at(font_text[ch]).advance * relative_size;
					}
					token.char_cnt = (unsigned int)(next - pos);
					pos += next - pos - 1;	//-1 because of loop increments
					}
					break;
				}

				token_list.push_back(std::move(token));
			}

			font_render.token_list.swap(token_list);
		}
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

		glCreateBuffers(1, &font_ssbo);
		GLbitfield flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;
		glNamedBufferStorage(font_ssbo, sizeof(Font_Instance) * Font_Max_Instance, nullptr, flags);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, font_binding_index, font_ssbo);
	}
	void CanvasSystem::Release() {

		glDeleteBuffers(1, &font_ssbo);
		glDeleteTextures(1, &raycast_tex);
		glDeleteFramebuffers(1, &fbo);
		CheckGLError();
	}
	std::set<Entity> const& CanvasSystem::Get_World_UI() const {
		return world_space_ui;
	}
	void CanvasSystem::UpdateHierachy(bool force) {
		//list of pair of entity and what type of rendering - split into 2 funcs for now
		//std::vector<std::pair<Entity, int>> entities_to_draw;

		auto core = Core::GetInstance();
		auto view = core->GetRegistry().view<canvasEntity>();

		RectTransform empty{};	//zeroed out rect transform for canvas elements to reference from
		empty.final_height = (float)target_height; empty.final_width = (float)target_width;
		empty.width = 0.f; empty.height = 0.f;

		world_space_ui.clear();
		for (auto entity : view) {
			world_space_z = 0.f;
			get_child_ui(entity, entity, entity, empty, force);
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

		auto core = SliceEngine::Core::GetInstance();

		auto const& cam_sys = core->GetSystem<CameraSystem>();
		auto main_cam = cam_sys.mainCam.value_or(entt::null);
		if (main_cam == entt::null) {
			return;	//not a game camera
		}
		
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		auto& cam = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(main_cam);
		cam_gamma = cam.gamma / 100.f;
		
		glViewport(0, 0, cam.width, cam.height);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cam.textureID, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, raycast_tex, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			return;
		}

		GLenum render_targets[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		//GLenum render_color[] = {GL_COLOR_ATTACHMENT0};
		//GLenum render_eid[] = {GL_COLOR_ATTACHMENT1};

		auto view = core->GetRegistry().view<canvasEntity>(entt::exclude<InactiveEntity>);

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
		//entities_to_draw.reserve(100);
		for (auto entity : overlay_canvas) {
			auto const& canvas = mRegistry->get<Canvas>(entity);
			if (canvas.componentEnabled) {
				//Get the entities to be drawn
				get_node_render(entities_to_draw, entity);
			}
		}

		glDrawBuffers(2, render_targets);
		CheckGLError();

		render_ui_overlay(main_cam, entities_to_draw);
		CheckGLError();

		glClearTexImage(raycast_tex, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &null_eid);
		CheckGLError();

		render_ui_eids(main_cam, entities_to_draw);
		CheckGLError();

		glDisable(GL_BLEND);	//idk ngl why this needs to be here, means i need to predict the settings(?)

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void CanvasSystem::render_ui_overlay(Entity camera, std::vector<std::pair<Entity, uint64_t>> const& elements) {
		if (elements.empty()) {
			return;
		}
		CheckGLError();


		auto core = SliceEngine::Core::GetInstance();
		auto const& rm = core->GetResourceManager();

		auto& cam = core->GetRegistry().get<SliceEngine::Camera>(camera);

		float cam_canv_width = (float)cam.width / target_width;
		float cam_canv_height = (float)cam.height / target_height;

		//map canvas width/height to camera width/height
		glm::mat4 canvas_to_ndc = glm::scale(glm::identity<glm::mat4>()
			, glm::vec3{ 2.f * cam_canv_width / cam.width, 2.f * cam_canv_height / cam.height, 1.f });

		uint64_t shader_guid = elements[0].second;
		GLuint shader = rm->get<SliceEngineTypes::Shader>((GUID)shader_guid).get()->s;
		glUseProgram(shader);
		CheckGLError();
		int uniform_loc = glGetUniformLocation(shader, "canvas_to_ndc");
		glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(canvas_to_ndc));


		//Get quad
		auto const& quad = *rm->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT).get();
		auto const& quad_mesh = quad.meshes[0];
		glBindVertexArray(quad_mesh.vao);

		for (auto const& element : elements) {
			if (element.second != shader_guid) {
				shader_guid = element.second;
				shader = rm->get<SliceEngineTypes::Shader>((GUID)shader_guid).get()->s;
				glUseProgram(shader);
				uniform_loc = glGetUniformLocation(shader, "canvas_to_ndc");
				glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(canvas_to_ndc));
				CheckGLError();
			}

			if (shader_guid == sprite_shader) {	//sprite
				auto const& rect = mRegistry->get<RectTransform>(element.first);
				uniform_loc = glGetUniformLocation(shader, "M");
				glm::mat4 model = rect.ToMatrix();
				glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(model));

				auto const& sprite = mRegistry->get<SpriteRenderer>(element.first);
				auto const& res = rm->get<SliceEngineTypes::Texture>(sprite.textureHandle);

				glBindTextureUnit(0, res.get()->texture_id);
				uniform_loc = glGetUniformLocation(shader, "rgba");
				glUniform4fv(uniform_loc, 1, glm::value_ptr(sprite.rgba));

				uniform_loc = glGetUniformLocation(shader, "gamma");

				if (auto* gamma_override = mRegistry->try_get<SpriteRendererGammaOverride>(element.first)) {
					if(gamma_override->componentEnabled)
						glUniform1f(uniform_loc, gamma_override->gamma / 100.f);
					else
						glUniform1f(uniform_loc, cam_gamma);
				}
				else {
					glUniform1f(uniform_loc, cam_gamma);
				}

				uniform_loc = glGetUniformLocation(shader, "uv");
				if (auto* anim = mRegistry->try_get<SpriteAnimator>(element.first)) {
					glUniform4fv(uniform_loc, 1, glm::value_ptr(sprite.uv));
				}
				else {
					glUniform4fv(uniform_loc, 1, glm::value_ptr(glm::vec4{ 0,1,0,1 }));
				}

				glDrawElements(quad_mesh.drawMode, quad_mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
			}
			else if (shader_guid == font_shader) {	//font
				auto const& rect = mRegistry->get<RectTransform>(element.first);
				auto const& font_render = mRegistry->get<FontRenderer>(element.first);

				if (font_render.fontHandle.GetGUID() == DefaultResourceIDs::FONT_BLANK_DEFAULT) {
					continue;
				}

				//Use rect to format the font characters
				unsigned int instance_count = 0;
				auto const& font = rm->get<SliceEngineTypes::Font_Data>(font_render.fontHandle).get();

				uniform_loc = glGetUniformLocation(shader, "rgba");
				glUniform4fv(uniform_loc, 1, glm::value_ptr(font_render.rgba));

				float relative_scale = font_render.font_size / font->font_size;

				uniform_loc = glGetUniformLocation(shader, "relative_scale");
				glUniform1f(uniform_loc, relative_scale);

				glBindTextureUnit(0, font->atlas_texture);

				CheckGLError();


				//fit into a line
				struct Line {
					unsigned char token_count;
					float line_width{};
				};
				std::vector<Line> lines{};
				float total_width = (float)rect.final_width;
				float current_width = 0.f;
				Line temp_line{};

				const std::string& text = font_render.text;

				for (auto const& token : font_render.token_list) {
					assert(token.char_cnt > 0);
					if (text[token.pos] == '\n') {	//if token is a line break
						temp_line.token_count++;
						temp_line.line_width = current_width;
						lines.push_back(temp_line);

						current_width = 0;
						temp_line.token_count = 0;
					}
					else if (current_width + token.size > total_width) {	//next token cant fit, carry over
						temp_line.line_width = current_width;
						lines.push_back(temp_line);

						current_width = token.size;
						temp_line.token_count = 1;
					}
					else {	//token can fit, append to current line
						temp_line.token_count++;
						current_width += token.size;
					}
				}

				if (temp_line.token_count) {	//any left over carried over tokens
					temp_line.line_width = current_width;
					lines.push_back(temp_line);
				}

				//Use rect as the text box
				//position the pen
				float left_ref = rect.final_x -(float)rect.final_width / 2;
				float top_ref = rect.final_y +(float)rect.final_height / 2;
				float x_pen = left_ref;
				float y_pen = top_ref - font_render.font_size;

				size_t tokens_cnt = 0;
				for (Line const& line : lines) {
					switch (font_render.alignment) {
					case FontRenderer::LEFT: {
						x_pen = left_ref;
					}
						break;
					case FontRenderer::CENTER: {
						x_pen = left_ref + (float)rect.final_width / 2 - line.line_width / 2;
					}
						break;
					case FontRenderer::RIGHT: {
						x_pen = left_ref + (float)rect.final_width - line.line_width;
					}
						break;
					}

					for (size_t tok = 0; tok < line.token_count; ++tok, ++tokens_cnt) {
						FontRenderer::Token const& curr_token = font_render.token_list[tokens_cnt];
						for (unsigned int ch_it = 0; ch_it < curr_token.char_cnt; ++ch_it) {
							char ch = text[curr_token.pos + ch_it];
							if (ch == '\n') {
								continue;
							}
							SliceEngineTypes::GlyphData const& glyph = font->glyph_datas.at(ch);

							float x = x_pen + glyph.xoff * relative_scale;
							float y = y_pen - glyph.yoff * relative_scale;
							float w = glyph.w * relative_scale;
							float h = glyph.h * relative_scale;

							x_pen += glyph.advance * relative_scale;

							if (w == 0) {
								continue;
							}

							RectTransform temp_rect;
							temp_rect.final_width = w;
							temp_rect.final_height = h;
							temp_rect.final_x = x;
							temp_rect.final_y = y;

							Font_Instance instance_data;

							instance_data.model_to_ndc = temp_rect.ToMatrix();
							SliceEngineTypes::Atlas_UV uv = font->atlas_uvs.at(ch);
							instance_data.atlas_uv = { uv.u_start,uv.u_end,uv.v_start,uv.v_end };
							//instance_data.atlas_uv = { 0.f,1.f,0.f,1.f };
							font_Instances[instance_count] = instance_data;
							++instance_count;

							if (instance_count >= Font_Max_Instance) {
								glNamedBufferSubData(font_ssbo, 0, sizeof(Font_Instance) * Font_Max_Instance, font_Instances);
								CheckGLError();
								glDrawElementsInstanced(quad_mesh.drawMode, quad_mesh.drawCnt, GL_UNSIGNED_INT, nullptr, Font_Max_Instance);
								CheckGLError();
								instance_count = 0;
							}
						}
					}

					y_pen -= font_render.line_spacing * font_render.font_size;
				}

				if (instance_count) {
					glNamedBufferSubData(font_ssbo, 0, sizeof(Font_Instance) * instance_count, font_Instances);
					CheckGLError();
					glDrawElementsInstanced(quad_mesh.drawMode, quad_mesh.drawCnt, GL_UNSIGNED_INT, nullptr, instance_count);
					CheckGLError();
				}
			}
		}

		CheckGLError();
	}

	void CanvasSystem::render_ui_eids(Entity camera, std::vector<std::pair<Entity, uint64_t>> const& elements) {
		if (elements.empty()) {
			return;
		}
		//return;
		auto core = SliceEngine::Core::GetInstance();
		auto const& rm = core->GetResourceManager();

		auto& cam = core->GetRegistry().get<SliceEngine::Camera>(camera);

		float cam_canv_width = (float)cam.width / target_width;
		float cam_canv_height = (float)cam.height / target_height;

		//map canvas width/height to camera width/height
		glm::mat4 canvas_to_ndc = glm::scale(glm::identity<glm::mat4>()
			, glm::vec3{ 2.f * cam_canv_width / cam.width, 2.f * cam_canv_height / cam.height, 1.f });

		uint64_t shader_guid = 0; 
		GLuint shader = 0;
		CheckGLError();
		int uniform_loc = 0;
		glBindTextureUnit(1, raycast_tex);
		CheckGLError();

		//Get quad
		auto const& quad = *rm->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::QUAD_DEFAULT).get();
		auto const& quad_mesh = quad.meshes[0];
		glBindVertexArray(quad_mesh.vao);

		for (auto const& element : elements) {
			if (element.second == font_shader) {
				continue;
			}

			if (element.second != shader_guid) {
				shader_guid = element.second;
				shader = rm->get<SliceEngineTypes::Shader>((GUID)eid_shader_map.at(shader_guid)).get()->s;
				glUseProgram(shader);
				uniform_loc = glGetUniformLocation(shader, "canvas_to_ndc");
				glUniformMatrix4fv(uniform_loc, 1, false, glm::value_ptr(canvas_to_ndc));
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

				uniform_loc = glGetUniformLocation(shader, "uv");
				if (auto* anim = mRegistry->try_get<SpriteAnimator>(element.first)) {
					glUniform4fv(uniform_loc, 1, glm::value_ptr(sprite.uv));
				}
				else {
					glUniform4fv(uniform_loc, 1, glm::value_ptr(glm::vec4{ 0,0,0,0 }));
				}

				glDrawElements(quad_mesh.drawMode, quad_mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
				CheckGLError();
			}
			//else if (shader == font_shader) {	//font

			//}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		CheckGLError();
	}

	void CanvasSystem::get_node_render(std::vector<std::pair<Entity, uint64_t>>& render, Entity node) {
		auto core = SliceEngine::Core::GetInstance();
		auto const& rm = core->GetResourceManager();
		if (!mRegistry->any_of<RectTransform>(node) || mRegistry->any_of<InactiveEntity>(node)) {
			return;
		}
		auto* sprite = mRegistry->try_get<SpriteRenderer>(node);
		if (sprite && sprite->componentEnabled) {
			render.push_back({ node, sprite_shader });	//eid and shader resource handle
		}
		auto font = mRegistry->try_get<FontRenderer>(node);
		if (font && font->componentEnabled) {
			//GUID font_guid = rm->mFileNameToGUID["Shaders/uiFont.shader"];

			//tokenize the font string to fit into text box
			if (!font->token_updated) {
				auto const& font_data = rm->get<SliceEngineTypes::Font_Data>(font->fontHandle);
				SliceEngineTypes::Tokenize(*font, *font_data.get());
				font->token_updated = true;
				
			}
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

	void CanvasSystem::get_child_ui(Entity canvas_entity, Entity parent, Entity node, RectTransform const& p_rect, bool force) {
		/*
		*	assumptions
		*	all children have rect transform
		*	if no rect transform return
		*/
		if (!mRegistry->any_of<RectTransform>(node) || (!force && mRegistry->any_of<InactiveEntity>(node))) {
			return;
		}
		auto& rect = mRegistry->get<RectTransform>(node);
		//auto const& p_rect = mRegistry->get<RectTransform>(parent);
		//if(parent != node)
		rect.Update(p_rect);	//get position of rect relative to parent

		if (glm::epsilonEqual(rect.final_width, 0.f, FLT_EPSILON) ||
			glm::epsilonEqual(rect.final_height, 0.f, FLT_EPSILON)) {
			return;
		}

		auto& ctx = mRegistry->get<Canvas>(canvas_entity);
		if (ctx.canvas_type == Canvas::WORLD) {

			auto& canvas_rect = mRegistry->get<RectTransform>(canvas_entity);
			if (node == parent) {	//canvas
				auto const& canvas_tform = mRegistry->get<Transform>(canvas_entity);
				//convert canvas space to world space
				canvas_rect.scale_x = 1.f;
				canvas_rect.scale_y = 1.f;// (1.f / canvas_rect.final_height) / canvas_tform.scale.y;
			}
			else {					//child of canvas
				//update world pos?
				//rotation of child is always 0
				//scale of child is relative to immediate parent
				//pos of child is child - parent
				auto& c_tform = mRegistry->get<Transform>(node);
				//auto& p_tform = mRegistry->get<Transform>(parent);

				c_tform.rotation = glm::identity<glm::quat>();
				c_tform.eulerAnglesHint = glm::vec3();
				c_tform.scale.x = rect.final_width / p_rect.final_width; 
				c_tform.scale.y = rect.final_height / p_rect.final_height; 
				c_tform.scale.z = 1.f;
				c_tform.position.x = (rect.final_x - p_rect.final_x) * p_rect.scale_x / canvas_rect.final_width;
				c_tform.position.y = (rect.final_y - p_rect.final_y) * p_rect.scale_y / canvas_rect.final_height;
				c_tform.position.z = world_space_z;
				rect.scale_x = p_rect.scale_x / c_tform.scale.x;
				rect.scale_y = p_rect.scale_y / c_tform.scale.y;
				world_space_z += 0.0001f;
				if (mRegistry->any_of<SpriteRenderer, FontRenderer>(node)) {
					world_space_ui.insert(node);
				}

			}
		}


		if (auto scene_graph = mRegistry->try_get<SceneGraph>(node)) {
			entt::entity child = scene_graph->neighbours[SceneGraph::DOWN];
			while (child != entt::null)
			{
				get_child_ui(/*entities_to_draw, */canvas_entity, node, child, rect, force);
				child = mRegistry->get<SceneGraph>(child).neighbours[SceneGraph::RIGHT];
			}
		}
	}

	glm::mat4 RectTransform::ToMatrix() const noexcept {
		float rad = glm::radians(final_rot);
		float c = cosf(rad);
		float s = sinf(rad);
		return {
			{final_width * c, final_width * s, 0.f, 0.f},
			{final_height * (-s), final_height * c, 0.f, 0.f},
			{0.f, 0.f, 1.f, 0.f},
			{final_x, final_y, 0.f, 1.f}
		};
	}
	void RectTransform::Update(RectTransform const& parent) {
		const float parent_x = parent.final_x;
		const float parent_y = parent.final_y;
		const float parent_width = parent.final_width;
		const float parent_height = parent.final_height;
		//x axis
		const float half_width = parent_width / 2;

		const float parent_left = parent_x - half_width;
		const float parent_right = parent_x + half_width;

		if (hori_pivot == HoriPivot::STRETCH_H) {
			const float left_ref = parent_left + left;	//apply left pad
			const float right_ref = parent_right - right;	//apply right pad

			final_width = right_ref - left_ref;
			final_x = left_ref + final_width / 2;

			////std::cout << "left: " << left_ref << ", right: " << right_ref << std::endl;
		}
		else {
			final_width = (float)width;
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
		const float half_height = parent_height / 2;
		const float parent_top = parent_y + half_height;
		const float parent_bot = parent_y - half_height;
		if (vert_pivot == VertPivot::STRETCH_V) {
			const float top_ref = parent_top - top;		//apply top pad
			const float bot_ref = parent_bot + bot;		//apply bot pad

			final_height = top_ref - bot_ref;
			final_y = bot_ref + final_height / 2;
		}
		else {
			final_height = (float)height;
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
			//std::cout << "GL_" << error.c_str() << " - " << file << ":" << line << std::endl;
			err = glGetError();
		}

		return;
	}