/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			CanvasSystem.h
 author:		Elton Leosantosa
 email:			leosantosa.e@digipen.edu
 brief:			Canvas system for 2D rendering layouts

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef CANVAS_SYSTEM_H
#define CANVAS_SYSTEM_H

#include "../ECS/BaseSystem.h"
#include "../ECS/ECSTypes.h"

#include "../Input/InputSystem.h"

#include <set>

/*
* Brief description of a canvas
* 
* Each Canvas will have its own framebuffer,
* and canvas elements will draw onto that framebuffer
* 
* Depending on canvas mode: world, camera, overlay
* will change how its drawn
* 
* only focus on overlay for now
* 
* most likely what will happen
* -each canvas will search through its children and grab all 2d sprite element components(can look into dirty flag nxt time)
* -iterate through each component and draw to a framebuffer
* -finally draw the completed framebuffer to the final framebuffer
* 
* likely there is only 1 shared framebuffer for overlay, that follows the reference pixel size
* for now lock the 
*/

void _CheckGLError(const char* file, int line);

#define CheckGLError() _CheckGLError(__FILE__, __LINE__)

namespace SliceEngine
{
	struct canvasEntity {};

	//struct RectTransform;
	struct CanvasSystem : BaseSystem<canvasEntity, Canvas, RectTransform>
	{
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override {};
		void EntityOnExit(entt::registry& reg, entt::entity entity) override {};
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override {};

		//make these static constexpr first
		constexpr static int target_height{ 1080 }, target_width{ 1920 };

		/*
		* yea im just gona go through the whole tree twice
		*/
		void UpdateHierachy(bool force = false);
		void DrawOverlay();


		void Init();
		void Release();

		std::set<Entity> const& Get_World_UI() const;
		/*
		* fires a ray into the list of overlay canvases and finds the element that is hit
		* bot left corner is 0,0
		* top right corner is width, height
		*/
		Entity Raycast(unsigned int x, unsigned int y) const;
	private:
		void get_child_ui(Entity canvas, Entity parent, Entity node, RectTransform const& prect, bool force);

		void get_node_render(std::vector<std::pair<Entity, uint64_t>>&, Entity);

		void render_ui_overlay(Entity camera, std::vector<std::pair<Entity, uint64_t>> const& elements);
		void render_ui_eids(Entity camera, std::vector<std::pair<Entity, uint64_t>> const& elements);


		//k i realised how render manager uses fbo now
		unsigned int fbo{};
		unsigned int raycast_tex{};
		std::unordered_map<uint64_t, uint64_t> eid_shader_map;
		std::set<Entity> world_space_ui;
		float world_space_z{};
		float cam_gamma{};	//store it local

		glm::quat billboard{};

		static constexpr unsigned int Font_Max_Instance = 200;

		struct Font_Instance {
			glm::mat4 model_to_ndc{};
			glm::vec4 atlas_uv{};
		} font_Instances[Font_Max_Instance];
		unsigned int font_ssbo{};
		static constexpr unsigned int font_binding_index = 3;
	};
}

#endif