/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			UI_Interactible.cpp
 author:		Elton Leosantosa
 email:			leosantosa.e@digipen.edu
 brief:			Contains the various systems that handle user's interaction with UI system

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "UI_Interactible.h"
#include "../Core/Core.h"

namespace SliceEngine {
	
#pragma region Slider

	//sets the value, positions the handle and fill, and calls c# callback
	void Slider::SetValue(float val, Entity self) {
		if (val > 1.f || val < 0.f) {
			return;
		}

		assert(self != entt::null);
		auto& reg = Core::GetInstance()->GetRegistry();
		value = val;

		auto& rect = reg.get<RectTransform>(self);

		///auto& self_node = reg.get<SceneGraph>(self);
	
		float handle_pos{};
		if (axis == X_Axis) {
			handle_pos = rect.final_width * value;

			if (handle != entt::null && reg.valid(handle) && reg.any_of<RectTransform>(handle)) {			
				auto& handle_rect = reg.get<RectTransform>(handle);
				if (contained) {
					handle_pos = (rect.final_width - handle_rect.final_width) * value + handle_rect.final_width * 0.5f;
				}

				if (direction == Positive) {
					handle_rect.hori_pivot = RectTransform::LEFT;
					handle_rect.pos_x = (int)handle_pos;
				}
				else {
					handle_rect.hori_pivot = RectTransform::RIGHT;
					handle_rect.pos_x = -(int)handle_pos;
				}
			}

			if (fill != entt::null && reg.valid(handle) && reg.any_of<RectTransform>(fill)) {
			
				auto& fill_rect = reg.get<RectTransform>(fill);
				fill_rect.vert_pivot = RectTransform::STRETCH_V;
				fill_rect.hori_pivot = RectTransform::STRETCH_H;
				if (direction == Positive) {
					fill_rect.left = 0;
					fill_rect.right = rect.final_width - handle_pos;
				}
				else {
					fill_rect.right = 0;
					fill_rect.left = rect.final_width - handle_pos;
				}
			}
		}
		else {
			handle_pos = rect.final_height * value;

			if (handle != entt::null && reg.valid(handle) && reg.any_of<RectTransform>(handle)) {
		
				auto& handle_rect = reg.get<RectTransform>(handle);
				if (contained) {
					handle_pos = (rect.final_height - handle_rect.final_height) * value + handle_rect.final_height * 0.5f;
				}

				if (direction == Positive) {
					handle_rect.vert_pivot = RectTransform::BOTTOM;
					handle_rect.pos_y = handle_pos;
				}
				else {
					handle_rect.vert_pivot = RectTransform::TOP;
					handle_rect.pos_y = -handle_pos;
				}
			}

			if (fill != entt::null && reg.valid(handle) && reg.any_of<RectTransform>(fill)) {
		
				auto& fill_rect = reg.get<RectTransform>(fill);
				fill_rect.hori_pivot = RectTransform::STRETCH_H;
				fill_rect.vert_pivot = RectTransform::STRETCH_V;
				if (direction == Positive) {
					fill_rect.bot = 0;
					fill_rect.top = rect.final_height - handle_pos;
				}
				else {
					fill_rect.top = 0;
					fill_rect.bot = rect.final_height - handle_pos;
				}
			}
		}


		OnSliderValueEvent event;
		event.entity = self;
		event.value = val;
		EventManager::GetInstance()->Publish<OnSliderValueEvent>(event);
	}

	float Slider::GetValue() const {
		return value;
	}

	/*
	* Slider system probably dosent need to care about mouse states, only handles click event
	*/
	void SliderSystem::HandleMouse(InputSystem& input, Entity raycast_entity) {
		/*
		* so apparently,
		* ismousereleased = mouse up
		* ismousedown = ismousepressed
		*/
		////std::cout << "released: " << input.IsMouseReleased(MouseButtons::LEFT) << std::endl;
		if (!input.IsMouseDown(MouseButtons::LEFT) || !mRegistry->any_of<Slider>(raycast_entity)) {
			return;
		}
		////std::cout << "handling" << std::endl;
		auto& slider = mRegistry->get<Slider>(raycast_entity);
		if (!slider.componentEnabled) {
			return;
		}
		////std::cout << "value: " << slider.GetValue() << std::endl;
		auto const& rect = mRegistry->get<RectTransform>(raycast_entity);

		glm::vec2 direction{};
		switch (slider.axis) {
		case Slider::X_Axis:
			direction.x = 1.f;
			break;
		case Slider::Y_Axis:
			direction.y = 1.f;
			break;
		}

		if (slider.direction == Slider::Negative) {
			direction *= -1.f;
		}

		glm::vec2 mouse_coord = input.GetMousePosition();
		glm::vec2 mouse_NDC = input.GetMouseNDC();


		//for now im just gona directly convert to game screen coord
		int mouse_x = (int)(mouse_NDC.x * CanvasSystem::target_width);//(unsigned int)mouse_coord.x;
		int mouse_y = (int)(CanvasSystem::target_height - mouse_NDC.y * CanvasSystem::target_height);// (unsigned int)mouse_coord.y;

		//int mouse_x = (int)mouse_coord.x;
		//int mouse_y = CanvasSystem::target_height - (int)mouse_coord.y;

		//First convert mouse into canvas coord - 0,0 is center
		mouse_x -= CanvasSystem::target_width / 2;
		mouse_y += CanvasSystem::target_height / 2;

		//find the relative mouse coord
		float rel_x = mouse_x - (rect.final_x - (float)rect.final_width / 2);
		float target_value = rel_x / rect.final_width;

		//assert(target_value <= 1.f && target_value >= 0.f);
		target_value = std::clamp(target_value, 0.f, 1.f);
		slider.SetValue(target_value, raycast_entity);
	}

#pragma endregion

#pragma region Button

	void ButtonSystem::InitSystem() {
		current_button = entt::null;


		auto core = Core::GetInstance();
		auto view = core->GetRegistry().view<buttonEntity>(entt::exclude<InactiveEntity>);

		auto default_event = ButtonSystem::Events::Cancel;
		for (auto entity : view) {
			update_button(entity, default_event);
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
	void ButtonSystem::HandleMouse(InputSystem& input, Entity raycast_entity) {
		//ButtonSystem::Events mouse_event = Events::None;

		if (current_button == entt::null) {
			if (raycast_entity == entt::null || !mRegistry->any_of<Button>(raycast_entity)) {
				return;
			}
			auto& t_button = mRegistry->get<Button>(raycast_entity);
			if (!t_button.componentEnabled) {
				return;
			}
			if (!input.IsMouseDown(MouseButtons::LEFT)) {		//hover
				update_button(raycast_entity, Highlight);
				current_button = raycast_entity;
			}
			else if (input.IsMousePressed(MouseButtons::LEFT)) {	//click same frame u hover
				update_button(raycast_entity, Click);
				current_button = raycast_entity;
			}
		}
		else {
			auto& c_button = mRegistry->get<Button>(current_button);

			if (!c_button.componentEnabled) {
				return;
			}

			if (c_button.state == Button::Highlighted) {
				if (!input.IsMouseDown(MouseButtons::LEFT)) {
					if (raycast_entity != current_button) {
						update_button(current_button, LeaveHighlight);
						current_button = entt::null;
					}
				}
				else {
					if (raycast_entity == current_button) {
						update_button(current_button, Click);
					}
					else {
						update_button(current_button, LeaveHighlight);
						if (mRegistry->any_of<Button>(raycast_entity)) {
							//auto& t_button = mRegistry->get<Button>(raycast_entity);
							update_button(raycast_entity, Click);	//click same frame u leave highlight
							current_button = raycast_entity;
						}
						else {
							current_button = entt::null;
						}
					}
				}
			}
			else {
				if (input.IsMouseReleased(MouseButtons::LEFT)) {
					if (raycast_entity != current_button) {
						update_button(current_button, Cancel);
					}
					else {
						update_button(current_button, Release);
					}
					current_button = entt::null;
				}
			}
		}
	}

	//Set the color/sprite guid of the image depending on state
	void ButtonSystem::update_button(Entity button_entity, Events event) {
		auto& button = mRegistry->get<Button>(button_entity);
		if (!button.componentEnabled) {
			return;
		}


		switch (event) {
		case Highlight:
			button.state = Button::Highlighted;
			OnButtonHoverEvent hover_event;
			hover_event.entity = button_entity;
			EventManager::GetInstance()->Publish<OnButtonHoverEvent>(hover_event);
			break;
		case Click: {
			button.state = Button::Pressed;
			OnButtonClickEvent click_event;
			click_event.entity = button_entity;
			EventManager::GetInstance()->Publish<OnButtonClickEvent>(click_event);
		}
			break;
		case LeaveHighlight:
			button.state = Button::Normal;
			OnButtonExitHoverEvent exithover_event;
			exithover_event.entity = button_entity;
			EventManager::GetInstance()->Publish<OnButtonExitHoverEvent>(exithover_event);
			break;
		case Release: {
			button.state = Button::Normal;

			OnButtonReleaseEvent release_event;
			release_event.entity = button_entity;
			EventManager::GetInstance()->Publish<OnButtonReleaseEvent>(release_event);
		}
			break;
		case Cancel:
		default:
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

#pragma endregion

}