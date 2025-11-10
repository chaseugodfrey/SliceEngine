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

namespace SliceEngine
{
	struct canvasEntity {};

	struct CanvasSystem : BaseSystem<canvasEntity, Canvas, Transform>
	{
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override {};
		void EntityOnExit(entt::registry& reg, entt::entity entity) override {};
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override {};

		constexpr static int targetHeight{ 1080 }, maxWidth{ 1920 };

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

	struct RectTransform {
		enum HoriPivot{
			LEFT,
			CENTER,
			RIGHT,
			STRETCH
		};
		enum VertPivot {
			TOP,
			MIDDLE,
			BOTTOM,
			STRETCH
		};

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

		//Settings only for imgui's display and component function calls
		//old pivot serves as a flag to know how to update intermediate values during the update call
		HoriPivot hori_pivot{ CENTER }, old_hori{ CENTER };
		VertPivot vert_pivot{ MIDDLE }, old_vert{ MIDDLE };

		//Intermediate settings used by imgui, all in local space
		int pos_x{}, pos_y{};			//pixel coord
		int width{100}, height{100};//pixel size
		int left{}, right{}, top{}, bot{};		//only used when pivots are stretch

		//Actual settings used to draw
		int final_x{}, final_y{};				//position with center of quad as position
		int final_width{100}, final_height{100};

		/*
		* For now no need reference to canvas i think
		* infact might not even need reference to parent since im just gona go down the scenegraph tree
		*/
		//Parent/Canvas reference - done via passing param through the recursive func call maybe
		void Update(Canvas const& ctx, int parent_x, int parent_y, int parent_width, int parent_height) {
			//x axis
			int half_width = parent_width / 2;

			int parent_left = parent_x - half_width;
			int parent_right = parent_x + half_width;

			//handle intermediate value conversion
			/*
			* case
			* 
			* hori = left
			* posx = 100
			* width = 50;
			* parent width = 300
			* final_x = 100
			* 
			* someone types
			* hori = right
			* posx = 300
			* 
			* final_x should calc to 0
			*/
			if (old_hori != hori_pivot) {

				old_hori = hori_pivot;
			}
			if (old_vert != vert_pivot) {
				old_vert = vert_pivot;
			}


			if (hori_pivot == HoriPivot::STRETCH) {
				int left_ref = parent_left + left;	//apply left pad
				int right_ref = parent_right - right;	//apply right pad

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
			int half_height = parent_height / 2;
			int parent_top = parent_y + half_height;
			int parent_bot = parent_y - half_height;
			if (vert_pivot == VertPivot::STRETCH) {
				int top_ref = parent_top - top;		//apply top pad
				int bot_ref = parent_bot + bot;		//apply bot pad

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


	};
	struct SpriteRenderer {
		Handle<SliceEngineTypes::Texture> textureHandle;	//resource handle for texture
	};
}

#endif