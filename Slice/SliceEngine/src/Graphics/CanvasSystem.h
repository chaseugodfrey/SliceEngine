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
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override {};

		//make these static constexpr first
		constexpr static int target_height{ 1080 }, target_width{ 1920 };

		/*
		* yea im just gona go through the whole tree twice
		*/
		void UpdateHierachy();
		void DrawOverlay();

		void Init();
		void Release();
	private:
		void get_child_ui(/*std::vector<std::pair<Entity, int>>& entities_to_draw, */Canvas const& ctx, RectTransform const& parent, Entity node);

		void get_node_render(std::vector<std::pair<Entity, GUID>>&, Entity);

		void render_ui_overlay(Entity canvas, std::vector<std::pair<Entity, GUID>> const& elements);

		//k i realised how render manager uses fbo now
		unsigned int fbo{};
	};

	//do i need a system for 2d, prob no for now
	//struct canvasEntity {};
	/*
	* Idea behind rect transform component
	* -bypass transform component's scenegraph settings
	* -sets transform component's values according to canvas system
	* -rect transform will store values that are used by canvas system
	* -dosent just contain position, but also the target rect area in the framebuffer to draw whatever renderer is used
	* 
	* Sprite Renderer
	* -contains handle for the texture to draw(ignore font for now, will be a font renderer component)
	* -will be used during canvas.render to draw to the framebuffer
	* -uses rect transform to know how to display the texture
	*/
	/*
	* Rect Transform Imgui display:
	* pos z, only used for sorting
	*
	* if no hori stretch
	*	pos x, x distance in pixels from pivot
	*	width, pixel width of the UI element
	* else
	*	left, x distance in pixels from left pivot
	*	right, x distance in pixels from right pivot
	*
	* if no vert stretch
	*	same as above except pos y/height/top/bot
	*
	* ignore rotation and scale for now, work only in abso pixels
	*
	*/
}

#endif