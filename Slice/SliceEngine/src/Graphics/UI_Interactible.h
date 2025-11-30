/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			UI_Interactible.h
 author:		Elton Leosantosa
 email:			leosantosa.e@digipen.edu
 brief:			Contains the various systems that handle user's interaction with UI system

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef UI_INTERACTIBLE_H
#define UI_INTERACTIBLE_H

#include "CanvasSystem.h"

namespace SliceEngine {

	struct sliderEntity {};
	struct SliderSystem : BaseSystem<sliderEntity, Slider, RectTransform> {
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override {};
		void EntityOnExit(entt::registry& reg, entt::entity entity) override {};
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override {};

		//i feel this along with button system's one can be abstracted out
		void HandleMouse(InputSystem&, Entity raycast_target);
		/*
		* goes to every slider and ensures that the handle and fill rect,
		* if exists, is parented and positioned correctly
		* if multiple sliders refer to the same handle/fill, jesus takes the wheel
		*/
		void UpdateSliders();
	};

	/*
	* logically only 1 button should ever be pressed at a time, gona go with this constraint for now
	* this also means only 1 button should ever need to update, which depends on input system
	*/
	struct buttonEntity {};
	struct ButtonSystem : BaseSystem<buttonEntity, Button, RectTransform, SpriteRenderer>
	{
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override {};
		void EntityOnExit(entt::registry& reg, entt::entity entity) override {};
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override {};

		//updates the current button and its state depending on input system and canvas raycast
		void HandleMouse(InputSystem&, Entity raycast_target);

	private:
		Entity current_button{ entt::null };

		enum Events {
			None,
			Highlight,
			LeaveHighlight,
			Click,
			Cancel,
			Release
		};

		//updates the image state of the current button if there is one
		void update_button(Entity, Events);
	};
}
#endif