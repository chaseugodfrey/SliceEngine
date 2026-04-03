/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        InspectorWindow.cpp

 author:	  Chase Rodgrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the InspectorWindow class, which is responsible for drawing the Inspector window and its contents.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include <glm/gtc/type_ptr.hpp>
#include "InspectorWindow.h"
#include "Core/Registry.h"
#include "Selection/SelectionManager.h"
#include "Session/SessionManager.h"
#include "ComponentPropertiesGUI.h"
#include "ComponentMultipleSelection.h"

#include <Resource/GUID.h>
#include <Scripting/ScriptSystem.h>
#include <Scripting/ScriptObject.h>
#include <Graphics/TransformHelper.h>
#include <Serializer/JSONSerializer.h>
#include <Systems/LayerManager.h>
#include <WindowManager/WindowManager.h>
#include <Systems/PrefabSystem.h>
#include <Animator/AnimatorWindow.h>
#include <Graphics/RenderManager.h>

namespace SliceEditor
{
	void InspectorWindow::Init()
	{
		mBaseFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowOverlap;
	}

	void InspectorWindow::Draw()
	{

		ImGui::Begin("Inspector");

		auto& selected_nodes = mRegistry.GetManager<SelectionManager>("Selection")->GetSelectedNodes();

		if (selected_nodes.size() <= 0)
		{
			ImGui::End();
			return;
		}

		// check what type selected nodes are

		auto lastSelectedNode = mRegistry.GetManager<SelectionManager>("Selection")->GetLastSelectedNode();
		auto type = lastSelectedNode->type;

		switch (type)
		{
		case SelectionType::ENTITY:
			DisplayEntity(static_cast<EntityNode*>(lastSelectedNode));
			break;
		case SelectionType::MATERIAL:
			DisplayMaterial(static_cast<DirectoryNode*>(lastSelectedNode));
			break;
		case SelectionType::PREFAB_ENTITY:
			DisplayPrefab(static_cast<EntityNode*>(lastSelectedNode));
			break;
		case SelectionType::STATE:
			DisplayState(static_cast<StateNode*>(lastSelectedNode));
			break;
		case SelectionType::TRANSITION:
			DisplayTransition(static_cast<TransitionLinkNode*>(lastSelectedNode));
			break;
		}

		ImGui::End();
	}
	
	void InspectorWindow::DisplayEntityData(entt::entity entity)
	{
		auto core = SliceEngine::Core::GetInstance();
		auto& slice = core->GetRegistry().get<SliceEngine::SliceEntity>(entity);
		auto original_name = SliceEngine::FactoryInstance.GetGOByEntity(entity).GetName();
		auto original_tag = SliceEngine::FactoryInstance.GetGOByEntity(entity).GetTag();
		bool isActive = !core->GetRegistry().any_of<SliceEngine::InactiveEntity>(entity);
		auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");
		bool isMultipleSelection = selectionManager->GetSelectedNodes().size() > 1 ? true : false;

		
		auto layer_manager = core->GetLayerManager();
		auto layer_name_list = layer_manager->GetLayerNameList();

		if (BoolInput(mRegistry, "##isActive", isActive))
		{
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
					if (isActive)
					{
						SliceEngine::Core::GetInstance()->GetRegistry().remove<SliceEngine::InactiveEntity>(currentEntity);
						slice.mActive = true;
					}
					else
					{
						slice.mActive = false;
						SliceEngine::Core::GetInstance()->GetRegistry().emplace<SliceEngine::InactiveEntity>(currentEntity);
					}
				}
			}
		}
		ImGui::SameLine();

		std::string editable_name = original_name;
		std::string editable_tag = original_tag;

		std::function<void(std::string name)> func = [&](std::string name)
			{
				SliceEngine::FactoryInstance.GetGOByEntity(entity).SetName(name);
			};
		
		StringInputHeader(mRegistry, "Name: ", "##name", editable_name, ImGui::GetContentRegionAvail().x,false, func);

		ImGui::Text("Entity ID: %d", entity);

		if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Prefab>(entity))
		{
			auto& prefabComponent = SliceEngine::FactoryInstance.GetGOByEntity(entity).GetComponent<SliceEngine::Prefab>();
			auto& assetManager = mRegistry.GetAssetManager();
			ImGui::Text("Prefab: ");
			ImGui::SameLine(150.f);
			if (assetManager.mGUIDtoFilename.find(prefabComponent.prefabGUID) != assetManager.mGUIDtoFilename.end())
			{
				ImGui::Text(assetManager.mGUIDtoFilename[prefabComponent.prefabGUID].c_str());
			}
			else
			{
				std::string prefabGUID = prefabComponent.prefabGUID.toString();
				ImGui::Text(prefabGUID.c_str());
			}
		}

		std::function<void(std::string name)> funcTag = [&](std::string name)
			{
				SliceEngine::FactoryInstance.GetGOByEntity(entity).SetTag(name);
			};

		//Multi-select for Tags
		//Do the different checks here for now.
		//TODO: Move to a different file maybe
		
		if (StringInputHeader(mRegistry, "Tag: ", "##tag", editable_tag, ImGui::GetContentRegionAvail().x, true, funcTag, StringMultipleSelection(selectionManager, editable_tag, isMultipleSelection)))
		{
			//Multi-Selection Setting for Tags
			if (isMultipleSelection)
			{
				for (auto selectedNode : selectionManager->GetSelectedNodes())
				{
					if (selectedNode->type == SelectionType::ENTITY)
					{
						SliceEngine::FactoryInstance.GetGOByEntity(static_cast<EntityNode*>(selectedNode)->entity).SetTag(editable_tag);
					}
				}
			}

		}
		//Game Object Tags:
		/*if (StringInputHeader(mRegistry, "Tag: ", "##entityTag", slice.mTag))
		{
			SliceEngine::FactoryInstance.GetGOByEntity(entity).SetTag(slice.mTag);
		}*/

		// currently tags are unused
		/*int tag = 0;
		std::vector<std::string> tags {"unused"};*/
		std::function<uint32_t(Entity)> funcLayer =
			[&](Entity e)
			{
				return SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SliceEntity>(e).mLayer;
			};

		core->GetInstance()->GetRegistry().patch<SliceEngine::SliceEntity>(entity, [&](SliceEngine::SliceEntity& slicePatch)
		{
			if(slicePatch.mLayer >= layer_name_list.size())
			{
				SLICE_LOG_WARNING("why u deletus fetus the physics asset");
				layer_manager->AssignLayer("Default", entity); //in case the physics asset was deleted
			}

			if (LayerHeader(mRegistry, "Layer", "##layer", slicePatch.mLayer, layer_name_list, false, ComboMultipleSelection(selectionManager, slicePatch.mLayer, isMultipleSelection, funcLayer)))
			{
				//Multi-Selection Setting for Layers
				if (isMultipleSelection)
				{
					for (auto selectedNode : selectionManager->GetSelectedNodes())
					{
						if (selectedNode->type == SelectionType::ENTITY)
						{
							Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
							core->GetInstance()->GetRegistry().patch<SliceEngine::SliceEntity>(currentEntity, [&](SliceEngine::SliceEntity& currentSlice)
								{
									if (currentSlice.mLayer >= layer_name_list.size())
									{
										SLICE_LOG_WARNING("why u deletus fetus the physics asset");
										layer_manager->AssignLayer("Default", currentEntity); //in case the physics asset was deleted
									}
									currentSlice.mLayer = slicePatch.mLayer;
								});
						}
					}
				}
			}
		});
		ImGui::Separator();

	}

	void InspectorWindow::DisplayTransform(entt::entity entity)
	{
		if (ImGui::TreeNodeEx("Transform", mBaseFlags))
		{
			auto& tr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(entity);
			auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");
			bool isMultipleSelection = selectionManager->GetSelectedNodes().size() > 1 ? true : false;
			std::array<bool, 3> editedAxis= std::array<bool, 3>{ false,false,false };

			DisplayComponentHeader<SliceEngine::Transform>(entity, false);

			std::function<glm::vec3(Entity)> getterPos =
				[](Entity e)
				{
					return SliceEngine::Core::GetInstance()
						->GetRegistry()
						.get<SliceEngine::Transform>(e)
						.position;
				};

			if (DragVec3InputHeader(mRegistry, "Position", "##t", tr.position, 0.0f, 0.0f, Vector3MultipleSelection(selectionManager, tr.position, isMultipleSelection, getterPos), &editedAxis))
			{

				if (isMultipleSelection)
				{
					for (auto selectedNode : selectionManager->GetSelectedNodes())
					{
						if (selectedNode->type == SelectionType::ENTITY)
						{
							Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
							auto& currentPosition = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(currentEntity).position;

							//Set the X axis if changed
							if (editedAxis[0])
							{
								currentPosition.x = tr.position.x;
							}

							//Set the Y axis if changed
							if (editedAxis[1])
							{
								currentPosition.y = tr.position.y;
							}

							//Set the Z axis if changed
							if (editedAxis[2])
							{
								currentPosition.z = tr.position.z;
							}
						}
					}
				}
			}

			std::function<glm::vec3(Entity)> getterRot =
				[](Entity e)
				{
					return SliceEngine::Core::GetInstance()
						->GetRegistry()
						.get<SliceEngine::Transform>(e)
						.eulerAnglesHint;
				};

			if (DragRotationInputHeader(mRegistry, "Rotation", "##r", tr.rotation, tr.eulerAnglesHint, Vector3MultipleSelection(selectionManager, tr.eulerAnglesHint, isMultipleSelection, getterRot), &editedAxis))
			{
				if (isMultipleSelection)
				{
					for (auto selectedNode : selectionManager->GetSelectedNodes())
					{
						if (selectedNode->type == SelectionType::ENTITY)
						{
							Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
							auto& currentRot = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(currentEntity).rotation;
							auto& currentEulerAngleHint = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(currentEntity).eulerAnglesHint;

							//Set the X axis if changed
							if (editedAxis[0])
							{
								currentEulerAngleHint.x = tr.eulerAnglesHint.x;
							}

							//Set the Y axis if changed
							if (editedAxis[1])
							{
								currentEulerAngleHint.y = tr.eulerAnglesHint.y;
							}

							//Set the Z axis if changed
							if (editedAxis[2])
							{
								currentEulerAngleHint.z = tr.eulerAnglesHint.z;
							}

							currentRot = SliceEngine::Vec3ToQuat(tr.eulerAnglesHint);
						}
					}
				}
			}

			std::function<glm::vec3(Entity)> getterScale =
				[](Entity e)
				{
					return SliceEngine::Core::GetInstance()
						->GetRegistry()
						.get<SliceEngine::Transform>(e)
						.scale;
				};
			if (DragVec3InputHeader(mRegistry, "Scale", "##s", tr.scale, 0.0f, 0.0f, Vector3MultipleSelection(selectionManager, tr.scale, isMultipleSelection, getterScale), &editedAxis))
			{
				if (isMultipleSelection)
				{
					for (auto selectedNode : selectionManager->GetSelectedNodes())
					{
						if (selectedNode->type == SelectionType::ENTITY)
						{
							Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
							auto& currentScale = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(currentEntity).scale;

							//Set the X axis if changed
							if (editedAxis[0])
							{
								currentScale.x = tr.scale.x;
							}

							//Set the Y axis if changed
							if (editedAxis[1])
							{
								currentScale.y = tr.scale.y;
							}

							//Set the Z axis if changed
							if (editedAxis[2])
							{
								currentScale.z = tr.scale.z;
							}
						}
					}
				}
			}

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayRectTransform(entt::entity entity)
	{
		if (ImGui::TreeNodeEx("RectTransform", mBaseFlags))
		{
			auto& rect = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::RectTransform>(entity);

			DisplayComponentHeader<SliceEngine::RectTransform>(entity, true);

			static std::vector<std::string> hori_enums{ "Left", "Center", "Right", "Stretch" };
			static std::vector<std::string> vert_enums{ "Top", "Middle", "Bottom", "Stretch" };
			ComboHeader<SliceEngine::RectTransform::HoriPivot>(mRegistry, "Hori Pivot", "##horipivot", rect.hori_pivot, hori_enums);
			ComboHeader<SliceEngine::RectTransform::VertPivot>(mRegistry, "Vert Pivot", "##vertpivot", rect.vert_pivot, vert_enums);

			if (rect.hori_pivot != SliceEngine::RectTransform::HoriPivot::STRETCH_H) {
				DragFloatInputHeader(mRegistry, "Pos X", "##posx", rect.pos_x, "X: %.1f", -2000, 2000);	//some random ass min max
				DragFloatInputHeader(mRegistry, "Width", "##width", rect.width, "W: %.1f", -2000, 2000);	//some random ass min max
			}
			else {
				DragFloatInputHeader(mRegistry, "Left", "##left", rect.left, "L: %.1f", -2000, 2000);	//some random ass min max
				DragFloatInputHeader(mRegistry, "Right", "##right", rect.right, "R: %.1f", -2000, 2000);	//some random ass min max
			}

			if (rect.vert_pivot != SliceEngine::RectTransform::VertPivot::STRETCH_V) {
				DragFloatInputHeader(mRegistry, "Pos Y", "##posy", rect.pos_y, "Y: %.1f", -2000, 2000);	//some random ass min max
				DragFloatInputHeader(mRegistry, "Height", "##height", rect.height, "H: %.1f", -2000, 2000);	//some random ass min max
			}
			else {
				DragFloatInputHeader(mRegistry, "Top", "##top", rect.top, "T: %.1f", -2000, 2000);	//some random ass min max
				DragFloatInputHeader(mRegistry, "Bot", "##bot", rect.bot, "B: %.1f", -2000, 2000);	//some random ass min max
			}

			DragFloatInputHeader(mRegistry, "Rotation", "##rect_rot", rect.final_rot, "R: %f", 0.f, 360.f);
			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplaySpriteRenderer(entt::entity entity)
	{
		if (ImGui::TreeNodeEx("SpriteRenderer", mBaseFlags))
		{
			auto& sprite = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SpriteRenderer>(entity);

			DisplayComponentHeader<SliceEngine::SpriteRenderer>(entity, true);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", sprite.componentEnabled);
			//glm::vec3 rgb;
			//rgb.r = sprite.rgba.r; rgb.g = sprite.rgba.g; rgb.b = sprite.rgba.b;
			//DragColorInputHeader(mRegistry, "RGB", "##rgb", rgb);

			DragColor4InputHeader(mRegistry, "Color", "##uicolor", sprite.rgba);
			//sprite.rgba.r = rgb.r;sprite.rgba.g = rgb.g;sprite.rgba.b = rgb.b;
			BoolInputHeader(mRegistry, "Raycast Target", "##raycasttarget", sprite.raycast_target);

			DragFloatInputHeader(mRegistry, "Alpha Threshold", "##alphathreshold", sprite.alphathreshold, "%.01f", 0.f, 1.f);

			SliceEngine::GUID tex_guid = sprite.textureHandle;
			GUIDDragDropInputHeader(mRegistry, "Image", "##spriteimage", tex_guid, "Texture");
			sprite.textureHandle = tex_guid;

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplaySpriteRendererGammaOverride(entt::entity entity)
	{
		if (ImGui::TreeNodeEx("SpriteRendererGammaOverride", mBaseFlags))
		{
			auto& sprite = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SpriteRendererGammaOverride>(entity);

			DisplayComponentHeader<SliceEngine::SpriteRendererGammaOverride>(entity, true);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", sprite.componentEnabled);

			DragFloatInputHeader(mRegistry, "Gamma Override", "##gammaoverride", sprite.gamma, "%.1f", 0.001f, 100.0f);

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplaySpriteAnimator(entt::entity entity)
	{
		if (ImGui::TreeNodeEx("SpriteAnimator", mBaseFlags))
		{
			auto& sprite_anim = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SpriteAnimator>(entity);

			DisplayComponentHeader<SliceEngine::SpriteAnimator>(entity, true);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", sprite_anim.componentEnabled);

			BoolInputHeader(mRegistry, "Playing", "##spriteanimplaying", sprite_anim.is_playing);
			BoolInputHeader(mRegistry, "Loop", "##spriteanimloop", sprite_anim.loop);
			//i cant be bothered to make a draguint8 or wtv so ill just do this
			unsigned int temps[3];
			temps[0] = sprite_anim.row;
			temps[1] = sprite_anim.col;
			temps[2] = sprite_anim.num_frames;
			DragUInt32InputHeader(mRegistry, "Row", "##spriterow", temps[0], "%d", 1, 16);	//16 is just some random cap
			DragUInt32InputHeader(mRegistry, "Col", "##spritecol", temps[1], "%d", 1, 16);	//16 is just some random cap
			DragUInt32InputHeader(mRegistry, "Num Frames", "##spriteframes", temps[2], "%d", 1, 256);	//256 is 16*16
			sprite_anim.row = temps[0];
			sprite_anim.col = temps[1];
			sprite_anim.num_frames = temps[2];

			DragFloatInputHeader(mRegistry, "FPS", "##spritefps", sprite_anim.fps, "%.2f", 0.f, 60.f, 0.01f);

		/*	unsigned int curr_temp = (unsigned int)sprite_anim.curr_frame;
			DragUInt32InputHeader(mRegistry, "Curr Frame", "##spriteframe", curr_temp, "%d", 0, sprite_anim.num_frames - 1);
			sprite_anim.curr_frame = curr_temp + FLT_EPSILON;*/

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayFontRenderer(entt::entity entity)
	{
		if (ImGui::TreeNodeEx("FontRenderer", mBaseFlags))
		{
			auto& font = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::FontRenderer>(entity);

			DisplayComponentHeader<SliceEngine::FontRenderer>(entity, true);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", font.componentEnabled);

			/*
			* will need to update this token_updated from scripts too
			*/

			if (StringInputHeader(mRegistry, "Font Text: ", "##font_text", font.text, ImGui::GetContentRegionAvail().x)) {
				font.token_updated = false;
			}

			DragColor4InputHeader(mRegistry, "Color", "##uicolor", font.rgba);

			if (DragFloatInputHeader(mRegistry, "Font Size", "##font_size", font.font_size, "%.1f", 1.f, 300.f)) {
				font.token_updated = false;
			}
			DragFloatInputHeader(mRegistry, "Line Spacing", "##line_spacing", font.line_spacing, "%.01f", 0.9f, 3.f);
			
			SliceEngine::GUID font_guid = font.fontHandle;
			if (GUIDDragDropInputHeader(mRegistry, "Font", "##fonttexture", font_guid, "Font")) {
				font.fontHandle = font_guid;
				font.token_updated = false;
			}
			

			static std::vector<std::string> alignment_enums{ "Left", "Center", "Right"};
			ComboHeader<SliceEngine::FontRenderer::Alignment>(mRegistry, "Alignment", "##font_alignment", font.alignment, alignment_enums);
			

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayCanvas(entt::entity entity) {
		if (ImGui::TreeNodeEx("Canvas", mBaseFlags))
		{
			auto& canvas = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Canvas>(entity);

			DisplayComponentHeader<SliceEngine::Canvas>(entity, true);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", canvas.componentEnabled);

			static std::vector<std::string> canvas_types{ "Overlay", "World Space"};
			ComboHeader<SliceEngine::Canvas::Type>(mRegistry, "Canvas Type", "##canvastype", canvas.canvas_type, canvas_types);

			DragUInt32InputHeader(mRegistry, "Sort Order", "##canvas_order", canvas.sort_order, "X: %u", 0, 128);	//random max

			BoolInputHeader(mRegistry, "Graphics Raycaster", "##graphicsraycaster", canvas.graphic_raycastable);

			ImGui::TreePop();
		}
	}
	
	void InspectorWindow::DisplayButton(entt::entity entity) {
		if (ImGui::TreeNodeEx("Button", mBaseFlags))
		{
			auto& button = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Button>(entity);

			DisplayComponentHeader<SliceEngine::Button>(entity, true);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", button.componentEnabled);

			static std::vector<std::string> transitions{ "Color", "Sprite" };
			ComboHeader<SliceEngine::Button::Transition>(mRegistry, "Button Transitions", "##btntransitions", button.transition, transitions);

			switch (button.transition) {
			case SliceEngine::Button::Color:
				DragColor4InputHeader(mRegistry, "Normal", "##btncolor1", button.color_transitions[SliceEngine::Button::Normal]);
				DragColor4InputHeader(mRegistry, "Highlighted", "##btncolor2", button.color_transitions[SliceEngine::Button::Highlighted]);
				DragColor4InputHeader(mRegistry, "Pressed", "##btncolor3", button.color_transitions[SliceEngine::Button::Pressed]);
				break;
			case SliceEngine::Button::Sprite:
				//auto sprite_states = button.sprite_transitions;
				GUIDDragDropInputHeader(mRegistry, "Normal", "##btnsprite1", button.sprite_transitions[SliceEngine::Button::Normal], "Texture");
				GUIDDragDropInputHeader(mRegistry, "Highlighted", "##btnsprite2", button.sprite_transitions[SliceEngine::Button::Highlighted], "Texture");
				GUIDDragDropInputHeader(mRegistry, "Pressed", "##btnsprite3", button.sprite_transitions[SliceEngine::Button::Pressed], "Texture");
				//sprite.textureHandle = tex_guid;
				break;
			}

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplaySlider(entt::entity entity) {
		if (ImGui::TreeNodeEx("Slider", mBaseFlags))
		{
			auto& slider = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Slider>(entity);

			DisplayComponentHeader<SliceEngine::Slider>(entity, false);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", slider.componentEnabled);
			BoolInputHeader(mRegistry, "Contained", "##slidercontained", slider.contained);

			static std::vector<std::string> axis_enums{ "X Axis", "Y Axis" };
			static std::vector<std::string> direction_enums{ "Positive", "Negative" };
			ComboHeader<SliceEngine::Slider::Axis>(mRegistry, "Axis", "##slideraxis", slider.axis, axis_enums);
			ComboHeader<SliceEngine::Slider::Direction>(mRegistry, "Direction", "##sliderdirection", slider.direction, direction_enums);

			EntityInputHeader(mRegistry, "Fill", "##sliderfill", slider.fill);
			EntityInputHeader(mRegistry, "Handle", "##sliderhandle", slider.handle);

			float new_val = slider.GetValue();
			if (SliderFloatInputHeader(mRegistry, "Value", "##sliderVal", new_val, "%.1f", 0.0, 1.0)) {
				slider.SetValue(new_val, entity);
			}

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayAudioSource(entt::entity entity)
	{
		auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
		if (ImGui::TreeNodeEx("Audio Source", mBaseFlags))
		{
			DisplayComponentHeader<SliceEngine::AudioSource>(entity);
			/*if (!DisplayComponentHeader<SliceEngine::AudioSource>(entity))
			{*/
			if (reg.all_of<SliceEngine::AudioSource>(entity))
			{

				reg.patch<SliceEngine::AudioSource>(entity, [&](auto& as)
				{
					BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", as.componentEnabled);

					GUIDDragDropInputHeader(mRegistry, "Audio Clip", "##audio_clip", as.soundGUID, "Audio");

					static std::vector<std::string> soundCategoryTypes{ "SFX", "BGM", "UI"};

					ComboHeader<SliceEngine::AudioSource::Category>(mRegistry, "Sound Category", "##soundCategory", as.category, soundCategoryTypes);

					DragIntInputHeader(mRegistry, "Priority", "##priority", as.priority, "%d", 0, 256);
					BoolInputHeader(mRegistry, "Is Mute", "##Mute", as.isMute);
					BoolInputHeader(mRegistry, "Play On Awake", "##playOnAwake", as.playOnAwake);
					BoolInputHeader(mRegistry, "Is Loop", "##looping", as.isLoop);
					BoolInputHeader(mRegistry, "Is Paused", "##isPaused", as.isPaused);
					SliderFloatInputHeader(mRegistry, "Volume", "##currVol", as.currentVolume, "%.1f", 0.0, 1.0);
					SliderFloatInputHeader(mRegistry, "Pitch", "##pitch", as.pitch, "%.1f", -3.0, 3.0);
					SliderFloatInputHeader(mRegistry, "Stereo Pan", "##stereoPan", as.stereoPan, "%.1f", -1.0, 1.0);
					SliderFloatInputHeader(mRegistry, "Spatial Blend", "##spatialBlend", as.spatialBlend, "%.1f", 0.0, 1.0);
					BoolInputHeader(mRegistry, "Enable Pathfinding", "##enablePathfinding", as.enablePathfinding);
					SliderFloatInputHeader(mRegistry, "Direct Occlusion", "##directOcclusion", as.directOcclusion, "%.1f", 0.0, 1.0);
					SliderFloatInputHeader(mRegistry, "Reverb Occlusion", "##reverbOcclusion", as.reverbOcclusion, "%.1f", 0.0, 1.0);
					if (ImGui::CollapsingHeader("3D Sound Settings", mBaseFlags))
					{
						SliderFloatInputHeader(mRegistry, "Doppler Level", "##dopplerLevel", as.dopplerLevel, "%.1f", 0.0, 5.0);
						SliderFloatInputHeader(mRegistry, "Spread", "##spread", as.spread, "%.1f", 0.0, 360.0);
						//To add volume rolloff dropdown
						SliderFloatInputHeader(mRegistry, "Min Distance", "##minDistance", as.minDistance, "%.1f", 0.0f, as.maxDistance);
						SliderFloatInputHeader(mRegistry, "Max Distance", "##maxDistance", as.maxDistance, "%.1f", as.minDistance, 1000.0f);
					}
						


					//Someone help disable this button when scene is running pwease ;^;
					ImGui::Text("Play Preview");
					ImGui::SameLine(150);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::Button(as.playPreview ? "Stop Preview" : "Play Preview"))
						as.playPreview = !as.playPreview;
						
				});
			}
			//}
			ImGui::TreePop();
		}

	}

	void InspectorWindow::DisplayAudioListener(entt::entity entity)
	{
		//auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
		auto& al = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::AudioListener>(entity);


		if (ImGui::TreeNodeEx("Audio Listener", mBaseFlags))
		{
			DisplayComponentHeader<SliceEngine::AudioListener>(entity);
			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", al.componentEnabled);
			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayMeshRenderer(entt::entity entity)
	{
		auto& rend = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Renderer>(entity);
		auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");
		bool isMultipleSelection = selectionManager->GetSelectedNodes().size() > 1 ? true : false;

		if (ImGui::TreeNodeEx("Renderer", mBaseFlags))
		{
			DisplayComponentHeader<SliceEngine::Renderer>(entity);

			if (BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", rend.componentEnabled))
			{
				if (isMultipleSelection)
				{
					for (auto selectedNode : selectionManager->GetSelectedNodes())
					{
						if (selectedNode->type == SelectionType::ENTITY)
						{
							Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
							if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Renderer>(currentEntity))
							{
								auto& currentRenderer = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Renderer>(currentEntity);
								currentRenderer.componentEnabled = rend.componentEnabled;
							}
						}
					}
				}
			}
			
			std::function<SliceEngine::GUID(Entity)> modelFunc =
				[&](Entity e)
				{
					if(SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Renderer>(e))
					{
						return SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Renderer>(e).modelHandle.getGUID();
					}
					else
					{
						return SliceEngine::GUID(0);
					}
				};
			
			if (HandleDragDropInputHeader<SliceEngine::SliceEngineTypes::Model>(mRegistry, "Mesh", "##rend_mesh", rend.modelHandle, "Model", nullptr, isMultipleSelection, modelFunc))
			{
				if (isMultipleSelection)
				{
					for (auto selectedNode : selectionManager->GetSelectedNodes())
					{
						if (selectedNode->type == SelectionType::ENTITY)
						{
							Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
							if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Renderer>(currentEntity))
							{
								auto& currentRenderer = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Renderer>(currentEntity);
								currentRenderer.modelHandle = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::Model>(rend.modelHandle.getGUID());
							}
						}
					}
				}
			}

			std::function<SliceEngine::GUID(Entity)> materialFunc =
				[&](Entity e)
				{
					if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Renderer>(e))
					{
						return SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Renderer>(e).materialHandle.getGUID();
					}
					else
					{
						return SliceEngine::GUID(0);
					}
				};

			if (HandleDragDropInputHeader<SliceEngine::SliceEngineTypes::Material>(mRegistry, "Material", "##rend_mat", rend.materialHandle, "Material", nullptr, isMultipleSelection, materialFunc))
			{
				if (isMultipleSelection)
				{
					for (auto selectedNode : selectionManager->GetSelectedNodes())
					{
						if (selectedNode->type == SelectionType::ENTITY)
						{
							Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
							if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Renderer>(currentEntity))
							{
								auto& currentRenderer = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Renderer>(currentEntity);
								currentRenderer.materialHandle = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::Material>(rend.materialHandle.getGUID());
							}
						}
					}
				}
			}
			auto const mdl = rend.modelHandle.get();
			if (mdl) {
				uint32_t temp = rend.meshOffset; //cant be bothered with a uint8
				DragUInt32InputHeader(mRegistry, "Mesh Index", "##mesh_index", temp, "Mesh: %u", 0, mdl->meshes.size() - 1);	//[min,max]
				rend.meshOffset = temp;
			}
			BoolInputHeader(mRegistry, "Cast Shadows", "##casts_shadow", rend.castShadow);

			ImGui::TreePop();
		}

	}

	void InspectorWindow::DisplayCamera(entt::entity entity)
	{		
		auto& cam = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(entity);

		if (ImGui::TreeNodeEx("Camera", mBaseFlags))
		{
			DisplayComponentHeader<SliceEngine::Camera>(entity);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", cam.componentEnabled);

			if (BoolInputHeader(mRegistry, "Is Main Camera", "##main_camera", cam.isMainCamera))
			{
				if (cam.isMainCamera)
					SliceEngine::Core::GetInstance()->GetRenderManager()->SetMainGameCamera(entity);
				else
					SliceEngine::Core::GetInstance()->GetRenderManager()->GetGameCamera().reset();
			}


			DragFloatInputHeader(mRegistry, "FOV", "##cam_fov", cam.pov, "%.1f", 1.0f, FLT_MAX);
			ImGui::Text("Clipping Planes");
			DragFloatInputHeader(mRegistry, "Near", "##cam_near", cam.near, "%.1f", 0.1f, FLT_MAX);
			DragFloatInputHeader(mRegistry, "Far", "##cam_far", cam.far, "%.1f", 1.f, FLT_MAX);

			ImGui::SeparatorText("Post-Processing FX");

			DragFloatInputHeader(mRegistry, "Exposure", "##cam_exposure", cam.exposure, "%.1f", 0.1f, 50.0f);
			if (DragFloatInputHeader(mRegistry, "Gamma", "##cam_gamma", cam.gamma, "%.1f", 0.001f, 100.0f))
			{
				SliceEngine::Core::GetInstance()->GetRenderManager()->SetSessionGamma(cam.gamma);
			}
			DragFloatInputHeader(mRegistry, "White Cutoff", "##cam_white_cutoff", cam.whiteBalance, "%.1f", 0.001f, 100.0f);
			DragFloatInputHeader(mRegistry, "Min Luminance", "##cam_min_luminance", cam.minLuminance, "%.2f", 0.001f, FLT_MAX, 0.01f);
			DragFloatInputHeader(mRegistry, "Max Luminance", "##cam_max_luminance", cam.maxLuminance, "%.2f", 0.001f, FLT_MAX, 0.01f);
			DragFloatInputHeader(mRegistry, "Luminance Learning Rate", "##cam_luminanceLearnRate", cam.luminanceLearningRate, "%.1f", 0.1f, 1000.0f);
			using RenderTag = SliceEngine::RENDER_TAG;

			bool isBloom = cam.postRenderToggles & RenderTag::RENDER_BLOOM;
			bool isGodray = cam.postRenderToggles & RenderTag::RENDER_GODRAY;
			bool isFog = cam.postRenderToggles & RenderTag::RENDER_FOG;
			bool isVignette = cam.postRenderToggles & RenderTag::RENDER_VIGNETTE;
			bool isImpact = cam.postRenderToggles & RenderTag::RENDER_IMPACT;
			bool isGroundCloud = cam.postRenderToggles & RenderTag::RENDER_GROUND_CLOUD;

			ImGui::Text("Bloom");
			ImGui::SameLine(150.0f);
			if (ImGui::Checkbox("##cam_isBloom", &isBloom))
			{
				SetBit(cam.postRenderToggles, RenderTag::RENDER_BLOOM, isBloom);
			}

			if (isBloom)
			{
				DragFloatInputHeader(mRegistry, "Bloom Radius", "##cam_bloom_radius", cam.bloomFilterRadius, "%.f", 0.0f, FLT_MAX);
				DragFloatInputHeader(mRegistry, "Bloom Strength", "##cam_bloom_strength", cam.bloomStrength, "%.1f", 0.1f, FLT_MAX);
				DragFloatInputHeader(mRegistry, "Bloom Limit", "##cam_bloom_Limit", cam.bloomLimit, "%.1f", 0.1f, FLT_MAX);
			}

			ImGui::Text("Godrays");
			ImGui::SameLine(150.0f);
			if (ImGui::Checkbox("##cam_isGodray", &isGodray))
			{
				SetBit(cam.postRenderToggles, RenderTag::RENDER_GODRAY, isGodray);
			}

			if (isGodray)
			{
				DragFloatInputHeader(mRegistry, "Godray Radius", "##cam_god_ray_radius", cam.godRayFilterRadius, "%.f", 0.0f, FLT_MAX);
				DragFloatInputHeader(mRegistry, "Godray Strength", "##cam_god_ray_strength", cam.godRayStrength, "%.1f", 0.1f, FLT_MAX);
			}


			ImGui::Text("Fog");
			ImGui::SameLine(150.0f);
			if (ImGui::Checkbox("##cam_isFog", &isFog))
			{
				SetBit(cam.postRenderToggles, RenderTag::RENDER_FOG, isFog);
			}

			if (isFog)
			{
				DragColor3InputHeader(mRegistry, "Fog Color", "##cam_fog_color", cam.fogColor);
				float tempIntensity = cam.fogIntensity * 100.f;
				if (DragFloatInputHeader(mRegistry, "Fog Intensity", "##cam_fog_intensity", tempIntensity, "%.1f", 0.0f, FLT_MAX))
					cam.fogIntensity = tempIntensity / 100.f;
			}

			ImGui::Text("Impact");
			ImGui::SameLine(150.0f);
			if (ImGui::Checkbox("##cam_isImpact", &isImpact))
			{
				SetBit(cam.postRenderToggles, RenderTag::RENDER_IMPACT, isImpact);
			}

			if (isImpact)
			{
				DragVec3InputHeader(mRegistry, "Impact Position", "##cam_impact_position", cam.impactPos);
				DragColor3InputHeader(mRegistry, "Impact Color", "##cam_impact_color", cam.impactColor);
				DragColor3InputHeader(mRegistry, "Impact Color 2", "##cam_impact_color2", cam.impactColor2);
				DragFloatInputHeader(mRegistry, "Impact Angle ?", "##cam_impact_angle", cam.impactAngle, "%.1f", 0.0f, FLT_MAX);
				BoolInputHeader(mRegistry, "Impact Is Smooth", "##cam_impact_smooth", cam.impactSmooth);
				DragFloatInputHeader(mRegistry, "Impact Flash Rate", "##cam_impact_epilepsy", cam.impactEpilepsy, "%.2f", -FLT_MAX, FLT_MAX, 0.01f);
				DragFloatInputHeader(mRegistry, "Impact Sharpness", "##cam_impact_noise1", cam.impactNoise1, "%.1f", 0.0f, FLT_MAX);
				DragFloatInputHeader(mRegistry, "Impact Density", "##cam_impact_noise2", cam.impactNoise2, "%.1f", 0.0f, FLT_MAX);
				DragFloatInputHeader(mRegistry, "Impact Blend", "##cam_impact_blend", cam.impactBlend, "%.2f", 0.0f, 1.0f, 0.01f);
			}

			ImGui::Text("Vignette");
			ImGui::SameLine(150.0f);
			if (ImGui::Checkbox("##cam_isVignette", &isVignette))
			{
				SetBit(cam.postRenderToggles, RenderTag::RENDER_VIGNETTE, isVignette);
			}

			if (isVignette)
			{
				DragVec2InputHeader(mRegistry, "Vignette Center", "##cam_vignette_center", cam.vignetteCenter);
				DragFloatInputHeader(mRegistry, "Vignette Intensity", "##cam_vignette_intensity", cam.vignetteIntensity, "%.1f", 0.0f, FLT_MAX);
				DragFloatInputHeader(mRegistry, "Vignette Smoothness", "##cam_vignette_smoothness", cam.vignetteSmoothness, "%.1f", 0.0f, FLT_MAX);
			}

			ImGui::Text("Ground Clouds");
			ImGui::SameLine(150.0f);
			if (ImGui::Checkbox("##cam_hasGroundClouds", &isGroundCloud))
			{
				SetBit(cam.postRenderToggles, RenderTag::RENDER_GROUND_CLOUD, isGroundCloud);
			}

			if (isGroundCloud)
			{
				DragFloatInputHeader(mRegistry, "Clouds Y Pos", "##cam_ground_clouds_height", cam.cloudsHeight, "%.1f", -FLT_MAX, FLT_MAX);
				DragColor4InputHeader(mRegistry, "Clouds Color", "##cam_ground_clouds_color", cam.cloudsColor);
				DragFloatInputHeader(mRegistry, "Clouds Amplitude", "##cam_ground_clouds_amplitude", cam.cloudsAmplitude, "%.1f", 0.0f, FLT_MAX);
				DragFloatInputHeader(mRegistry, "Clouds Intensity", "##cam_ground_clouds_intensity", cam.cloudsIntensity, "%.1f", 0.0f, FLT_MAX);
				float tempCutoff = cam.cloudsCutoff * 100.f;
				if (DragFloatInputHeader(mRegistry, "Clouds Alpha Cutoff", "##cam_ground_clouds_cutoff", tempCutoff, "%.1f", 0.0f, 100.0f))
					cam.cloudsCutoff = tempCutoff / 100.f;
				float tempSmoothness = cam.cloudsSmoothness * 10000.f;
				if (DragFloatInputHeader(mRegistry, "Clouds Smoothness", "##cam_ground_clouds_smoothness", tempSmoothness, "%.1f", 0.0f, FLT_MAX))
					cam.cloudsSmoothness = tempSmoothness / 10000.f;
				DragVec3InputHeader(mRegistry, "Clouds Second", "##cam_secondCloudOffset", cam.cloudsSecondCloudOffset);
				DragColor4InputHeader(mRegistry, "Clouds Color", "##cam_ground_second_clouds_color", cam.cloudsSecondColor);
				DragFloatInputHeader(mRegistry, "Second Clouds Amplitude", "##cam_ground_second_clouds_amplitude", cam.cloudsSecondCloudAmplitude, "%.1f", 0.0f, FLT_MAX);
				DragFloatInputHeader(mRegistry, "Second Clouds Intensity", "##cam_ground_second_clouds_intensity", cam.cloudsSecondCloudIntensity, "%.1f", 0.0f, FLT_MAX);
				tempSmoothness = cam.cloudsSecondCloudSmoothness * 10000.f;
				if (DragFloatInputHeader(mRegistry, "Second Clouds Smoothness", "##cam_ground_second_clouds_smoothness", tempSmoothness, "%.1f", 0.0f, FLT_MAX))
					cam.cloudsSecondCloudSmoothness = tempSmoothness / 10000.f;
			}

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayRigidbody(entt::entity entity)
	{
		auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();

		const char* arr[2] = { "Discrete", "Continuous" };
		if (ImGui::TreeNodeEx("Rigidbody", mBaseFlags))
		{
			if (!DisplayComponentHeader<SliceEngine::RigidBody>(entity))
			{
				reg.patch<SliceEngine::RigidBody>(entity, [&](SliceEngine::RigidBody& rb)
					{

						DragFloatInputHeader(mRegistry, "Mass", "##mass", rb.mass, "%.3f", 0.1f, FLT_MAX);

						DragFloatInputHeader(mRegistry, "Gravity", "##gravity", rb.gravityFactor, "%.3f", 0.0f, FLT_MAX);

						BoolInputHeader(mRegistry, "Is Kinematic?", "##isKinematic", rb.isKinematic);

						DragFloatInputHeader(mRegistry, "Linear Damping", "##linearDamp", rb.linearDamping, "%.3f", 0.0f, FLT_MAX);

						DragFloatInputHeader(mRegistry, "Angular Damping", "##angularDamp", rb.angularDamping, "%.3f", 0.0f, FLT_MAX);

						DragFloatInputHeader(mRegistry, "Friction", "##friction", rb.friction, "%.3f", 0.1f, FLT_MAX);

						DragFreezeOptionsInputHeader(mRegistry, "Freeze Position", "##freezePos", rb.freezePosition);

						DragFreezeOptionsInputHeader(mRegistry, "Freeze Rotation", "##freezeRot", rb.freezeRotation);

						static std::vector<std::string> colDetectNames{ "Discrete", "Continuous" };
						ComboHeader<JPH::EMotionQuality>(mRegistry, "Col Detection", "##colDetect", rb.CollisionDetection, colDetectNames);

					});
			}
			ImGui::TreePop();
		}

	}

	void InspectorWindow::DisplayCollider3D(entt::entity entity)
	{
		auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
		auto& colliderData = reg.get<SliceEngine::ColliderShape>(entity);
		auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");
		bool isMultipleSelection = selectionManager->GetSelectedNodes().size() > 1 ? true : false;

		const char* arr[2] = { "Non-Moving" ,"Moving" };
		std::string colliderName;
		
		std::visit([&](auto&& data)
			{
				using T = std::decay_t<decltype(data)>;
				if constexpr (std::is_same_v<T, SliceEngine::ColliderShape::BoxData>)
					colliderName = "Box Collider";
				else if constexpr (std::is_same_v<T, SliceEngine::ColliderShape::SphereData>)
					colliderName = "Sphere Collider";
				else if constexpr (std::is_same_v<T, SliceEngine::ColliderShape::CapsuleData>)
					colliderName = "Capsule Collider";
				else if constexpr (std::is_same_v<T, SliceEngine::ColliderShape::MeshData>)
					colliderName = "Mesh Collider";
				else if constexpr (std::is_same_v<T, SliceEngine::ColliderShape::CylinderData>)
					colliderName = "Cylinder Collider";
			}, colliderData.shapeData);

		if (ImGui::TreeNodeEx(colliderName.c_str(), mBaseFlags))
		{
			if(!DisplayComponentHeader<SliceEngine::ColliderShape>(entity))
			{
				reg.patch<SliceEngine::ColliderShape>(entity, [&](SliceEngine::ColliderShape& col)
				{
						if (BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", col.componentEnabled))
						{
							if (isMultipleSelection)
							{
								for (auto selectedNode : selectionManager->GetSelectedNodes())
								{
									if (selectedNode->type == SelectionType::ENTITY)
									{
										Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
										if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::ColliderShape>(currentEntity))
										{
											reg.patch<SliceEngine::ColliderShape>(currentEntity, [&](SliceEngine::ColliderShape& currentCol)
												{
													currentCol.componentEnabled = col.componentEnabled;
												});
										}
									}
								}
							}
						}

					BoolInputHeader(mRegistry, "Is Trigger", "##isTrigger", col.isTrigger);

					glm::vec3 glm3 = JPHtoGLM(col.offSet);
					if (DragVec3InputHeader(mRegistry, "Offset", "##colOffset", glm3))
					{
						col.offSet = GLMtoJPH(glm3);
					}

					if (std::holds_alternative<SliceEngine::ColliderShape::BoxData>(col.shapeData))
					{
						glm::vec3 glm3boxData = JPHtoGLM(std::get<SliceEngine::ColliderShape::BoxData>(col.shapeData).scale);
						if (DragVec3InputHeader(mRegistry, "Scale", "##boxScale3D", glm3boxData, 0.0, FLT_MAX))
						{
							col.SetBoxData(SliceEngine::ColliderShape::BoxData(GLMtoJPH(glm3boxData)));
						}
					}

					else if (std::holds_alternative<SliceEngine::ColliderShape::SphereData>(col.shapeData))
					{
						float radius = std::get<SliceEngine::ColliderShape::SphereData>(col.shapeData).radius;
						if (DragFloatInputHeader(mRegistry, "Radius", "##sphereRadius", radius))
						{
							col.SetSphereData(SliceEngine::ColliderShape::SphereData(radius));
						}
					}

					else if (std::holds_alternative<SliceEngine::ColliderShape::CapsuleData>(col.shapeData))
					{
						float radius = std::get<SliceEngine::ColliderShape::CapsuleData>(col.shapeData).radius;
						float height = std::get<SliceEngine::ColliderShape::CapsuleData>(col.shapeData).height;
						if (DragFloatInputHeader(mRegistry, "Radius", "##capsuleRadius", radius))
						{
							col.SetCapsuleData(SliceEngine::ColliderShape::CapsuleData(radius,height));
						}

						if (DragFloatInputHeader(mRegistry, "Height", "##capsuleHeight", height))
						{
							col.SetCapsuleData(SliceEngine::ColliderShape::CapsuleData(radius, height));
						}
					}
					else if (std::holds_alternative < SliceEngine::ColliderShape::MeshData>(col.shapeData))
					{
						// nothing for now UwU
					}
					else if (std::holds_alternative<SliceEngine::ColliderShape::CylinderData>(col.shapeData))
					{
						float radius = std::get<SliceEngine::ColliderShape::CylinderData>(col.shapeData).radius;
						float height = std::get<SliceEngine::ColliderShape::CylinderData>(col.shapeData).height;
						if (DragFloatInputHeader(mRegistry, "Radius", "##capsuleRadius", radius))
						{
							col.SetCylinderData(SliceEngine::ColliderShape::CylinderData(radius, height));
						}

						if (DragFloatInputHeader(mRegistry, "Height", "##capsuleHeight", height))
						{
							col.SetCylinderData(SliceEngine::ColliderShape::CylinderData(radius, height));
						}
					}

				});
			}
			ImGui::TreePop();
		}
	}


	void InspectorWindow::DisplaySliceScript(entt::entity entity)
	{
		auto& script = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(entity);
		auto selectionManager = mRegistry.GetManager<SelectionManager>("Selection");
		bool isMultipleSelection = selectionManager->GetSelectedNodes().size() > 1 ? true : false;


		if (ImGui::TreeNodeEx("Script", mBaseFlags))
		{
			DisplayComponentHeader<SliceEngine::Script>(entity);

			auto& scriptMap = SliceEngine::gScriptSystem->mEntityClasses;
			std::string script_name = script.scriptName;
			std::vector<std::string> scriptList{};
			int selectedIndex = 0;

			for (auto& [key, value] : scriptMap)
			{
				scriptList.push_back(value->mClassName);
			}

			if (script.scriptName.empty())
			{
				//script_name = "Empty";
				scriptList.push_back("Empty");
				selectedIndex = (int)scriptList.size() - 1;
			}
			else
			{
				//ngl its not the most robust method probably but its a fix for now
				std::string searchName = script.scriptName;
				if (script.scriptName.starts_with("SliceEngine."))
				{
					searchName = script.scriptName.substr(std::string("SliceEngine.").size());
				}

				auto it = std::find(scriptList.begin(), scriptList.end(), searchName);
				if (it != scriptList.end())
				{
					selectedIndex = (int)std::distance(scriptList.begin(), it);
				}
			}

			//if(!script_name.empty())
			//{
			//	ImGui::BeginDisabled();
			//}

			ImGui::PushID((int)entity);
			if (ComboHeader<int>(mRegistry, "Script Class:", "##scriptClassID", selectedIndex, scriptList, true))
			{
				script.scriptName = "SliceEngine.";
				script.scriptName += scriptList[selectedIndex];
				SliceEngine::gScriptSystem->ReloadEntityScript(entity);
			}
			ImGui::PopID();
			//if(!script_name.empty())
			//{
			//	ImGui::EndDisabled();
			//}

			// Script Variables

			if(!script.scriptName.empty())
			{
				auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(entity);

				if (scriptRef != nullptr)
				{
					const auto& fields = scriptRef->GetScriptClass()->mFields;
					for (const auto& it : fields)
					{

						#pragma region Array Variables
						if (it.second.mContainerType == SliceEngine::ScriptFieldType::Array)
						{
							if (it.second.mType == SliceEngine::ScriptFieldType::Float)
							{
								auto data = scriptRef->GetArrayFieldValue<float>(it.second.mName);

								std::function<void(std::string, std::vector<float>)> func = [sp = scriptRef](std::string name, std::vector<float> val)
									{
										sp->SetArrayFieldValue(name, val);
									};

								if (DragFloatArrayScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetArrayFieldValue(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}

							else if (it.second.mType == SliceEngine::ScriptFieldType::String)
							{
								auto data = scriptRef->GetArrayFieldValue<std::string>(it.second.mName);

								std::function<void(std::string, std::vector<std::string>)> func = [sp = scriptRef](std::string name, std::vector<std::string> val)
									{
										sp->SetArrayFieldValue(name, val);
									};

								if (StringArrayScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetArrayFieldValue(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}

							else if (it.second.mType == SliceEngine::ScriptFieldType::Int)
							{
								auto data = scriptRef->GetArrayFieldValue<int>(it.second.mName);

								std::function<void(std::string, std::vector<int>)> func = [sp = scriptRef](std::string name, std::vector<int> val)
									{
										sp->SetArrayFieldValue(name, val);
									};

								if (DragIntArrayScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetArrayFieldValue(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}

							else if (it.second.mType == SliceEngine::ScriptFieldType::Vector3)
							{
								auto data = scriptRef->GetArrayFieldValue<glm::vec3>(it.second.mName);

								std::function<void(std::string, std::vector<glm::vec3>)> func = [sp = scriptRef](std::string name, std::vector<glm::vec3> val)
									{
										sp->SetArrayFieldValue(name, val);
									};

								if (DragVec3ArrayScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetArrayFieldValue(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}
						}

						#pragma endregion

						#pragma region List Variables
						else if (it.second.mContainerType == SliceEngine::ScriptFieldType::List)
						{
							if (it.second.mType == SliceEngine::ScriptFieldType::Float)
							{
								auto data = scriptRef->GetListFieldValue<float>(it.second.mName);
								std::vector<MultiSelect> changedVars;
								std::function<void(const char*, std::string, std::vector<float>, float, int)> func = [sp = scriptRef](const char* funcToExec, std::string name, std::vector<float> list, float val, int index)
									{
										if (std::strcmp(funcToExec, "Edit") == 0)
										{
											sp->SetListField(name, list);
										}
										else if (std::strcmp(funcToExec, "Add") == 0)
										{
											sp->AddListFieldValue(name, val);
										}
										else if (std::strcmp(funcToExec, "Remove") == 0)
										{
											sp->RemoveListField(name, index);
										}
									};

								if (FloatListScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data, "%.3f",0.1f, 0.f,0.f, ScriptFloatListElementDifferent(selectionManager, script.scriptName, it.second.mName, data,isMultipleSelection), changedVars))
								{
									//scriptRef->SetListField(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
									auto multiSetData = scriptRef->GetListFieldValue<float>(it.second.mName);
									if (isMultipleSelection)
									{
										ScriptFloatListMultiSet(selectionManager, script.scriptName, it.second.mName, multiSetData, changedVars);
									}
								}
							}

							else if (it.second.mType == SliceEngine::ScriptFieldType::String)
							{
								auto data = scriptRef->GetListFieldValue<std::string>(it.second.mName);

								std::function<void(const char*, std::string, std::vector<std::string>, std::string, int)> func = [sp = scriptRef](const char* funcToExec, std::string name, std::vector<std::string> list, std::string val, int index)
									{
										if (std::strcmp(funcToExec, "Edit") == 0)
										{
											sp->SetListField(name, list);
										}
										else if (std::strcmp(funcToExec, "Add") == 0)
										{
											sp->AddListFieldValue(name, val);
										}
										else if (std::strcmp(funcToExec, "Remove") == 0)
										{
											sp->RemoveListField(name, index);
										}
									};

								if (StringListScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetListField(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}

							else if (it.second.mType == SliceEngine::ScriptFieldType::Int)
							{
								auto data = scriptRef->GetListFieldValue<int>(it.second.mName);

								std::function<void(const char*, std::string, std::vector<int>, int, int)> func = [sp = scriptRef](const char* funcToExec, std::string name, std::vector<int> list, int val, int index)
									{
										if (std::strcmp(funcToExec, "Edit") == 0)
										{
											sp->SetListField(name, list);
										}
										else if (std::strcmp(funcToExec, "Add") == 0)
										{
											sp->AddListFieldValue(name, val);
										}
										else if (std::strcmp(funcToExec, "Remove") == 0)
										{
											sp->RemoveListField(name, index);
										}
									};

								if (IntListScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetListField(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}

							else if (it.second.mType == SliceEngine::ScriptFieldType::Vector3)
							{
								auto data = scriptRef->GetListFieldValue<glm::vec3>(it.second.mName);

								std::function<void(const char*, std::string, std::vector<glm::vec3>, glm::vec3, int)> func = [sp = scriptRef](const char* funcToExec, std::string name, std::vector<glm::vec3> list, glm::vec3 val, int index)
									{
										if (std::strcmp(funcToExec, "Edit") == 0)
										{
											sp->SetListField(name, list);
										}
										else if (std::strcmp(funcToExec, "Add") == 0)
										{
											sp->AddListFieldValue(name, val);
										}
										else if (std::strcmp(funcToExec, "Remove") == 0)
										{
											sp->RemoveListField(name, index);
										}
									};

								if (DragVec3ListScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetListField(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}

							else if (it.second.mType == SliceEngine::ScriptFieldType::GameObject)
							{
								auto data = scriptRef->GetListFieldValue<SliceEngine::GameObject>(it.second.mName);
								std::vector<MultiSelect> changedVars;

								std::function<void(const char*, std::string, std::vector<SliceEngine::GameObject>, SliceEngine::GameObject, int)> func = [sp = scriptRef](const char* funcToExec, std::string name, std::vector<SliceEngine::GameObject> list, SliceEngine::GameObject val, int index)
									{
										if (std::strcmp(funcToExec, "Edit") == 0)
										{
											sp->SetListField(name, list);
										}
										else if (std::strcmp(funcToExec, "Add") == 0)
										{
											sp->AddListFieldValue(name, val);
										}
										else if (std::strcmp(funcToExec, "Remove") == 0)
										{
											sp->RemoveListField(name, index);
										}
									};

								if (GameObjectListScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data, ScriptGameObjectListElementDifferent(selectionManager, script.scriptName, it.second.mName, data, isMultipleSelection), changedVars))
								{
									//scriptRef->SetListField(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
									auto multiSetData = scriptRef->GetListFieldValue<SliceEngine::GameObject>(it.second.mName);
									if (isMultipleSelection)
									{
										ScriptGameObjectListMultiSet(selectionManager, script.scriptName, it.second.mName, multiSetData, changedVars);
									}
								}
							}
						}

						#pragma endregion

						#pragma region Normal Variables
						else
						{
							if (it.second.mType == SliceEngine::ScriptFieldType::Float)
							{
								float data = scriptRef->GetFieldValue<float>(it.second.mName);

								std::function<void(std::string, float)> func = [sp = scriptRef](std::string name, float val)
									{
										sp->SetFieldValue(name, val);
									};

								if (DragFloatInputScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data, "%.3f", 0.0f,0.0f, ScriptFloatMultipleSelection(selectionManager, script.scriptName, it.second.mName, data, isMultipleSelection)))
								{
									scriptRef->SetFieldValue(it.second.mName, data);
									if(isMultipleSelection)
									{
										ScriptFloatMultiSet(selectionManager, script.scriptName, it.second.mName, data);
									}
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}
							else if (it.second.mType == SliceEngine::ScriptFieldType::Bool)
							{
								bool data = scriptRef->GetFieldValue<bool>(it.second.mName);
								std::function<void(std::string, bool)> func = [sp = scriptRef](std::string name, bool val)
									{
										sp->SetFieldValue(name, val);
									};
								if (BoolInputScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data, ScriptBoolMultipleSelection(selectionManager, script.scriptName, it.second.mName, data, isMultipleSelection)))
								{
									scriptRef->SetFieldValue(it.second.mName, data);
									if (isMultipleSelection)
									{
										ScriptBoolMultiSet(selectionManager, script.scriptName, it.second.mName, data);
									}
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}
							else if (it.second.mType == SliceEngine::ScriptFieldType::String)
							{
								std::string str = scriptRef->GetFieldValue<std::string>(it.second.mName);
								std::function<void(std::string, std::string)> func = [sp = scriptRef](std::string name, std::string val)
									{
										sp->SetFieldValue(name, val);
									};

								if (StringInputScriptHeader(mRegistry,func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), str, ScriptStringMultipleSelection(selectionManager, script.scriptName, it.second.mName, str, isMultipleSelection)))
								{
									scriptRef->SetFieldValue<std::string>(it.second.mName, str);
									if (isMultipleSelection)
									{
										ScriptStringMultiSet(selectionManager, script.scriptName, it.second.mName, str);
									}
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}
							else if (it.second.mType == SliceEngine::ScriptFieldType::Int)
							{
								int data = scriptRef->GetFieldValue<int>(it.second.mName);
								std::function<void(std::string, int)> func = [sp = scriptRef](std::string name, int val)
									{
										sp->SetFieldValue(name, val);
									};

								if (DragIntInputScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data, "%d", 0,0, ScriptIntMultipleSelection(selectionManager, script.scriptName, it.second.mName, data, isMultipleSelection)))
								{
									scriptRef->SetFieldValue(it.second.mName, data);
									if (isMultipleSelection)
									{
										ScriptIntMultiSet(selectionManager, script.scriptName, it.second.mName, data);
									}
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}
							else if (it.second.mType == SliceEngine::ScriptFieldType::Vector3)
							{
								glm::vec3 data = scriptRef->GetFieldValue<glm::vec3>(it.second.mName);
								std::array<bool, 3> changedAxis{ false,false,false };
								std::function<void(std::string, glm::vec3)> func = [sp = scriptRef](std::string name, glm::vec3 val)
									{
										sp->SetFieldValue(name, val);
									};

								if (DragVec3InputScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data,"%.3f",0.1f,0.0f,0.0f, ScriptVector3MultipleSelection(selectionManager, script.scriptName, it.second.mName, data, isMultipleSelection), &changedAxis))
								{
									scriptRef->SetFieldValue(it.second.mName, data);
									if (isMultipleSelection)
									{
										ScriptVector3MultiSet(selectionManager, script.scriptName, it.second.mName, data, changedAxis);
									}
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}
							else if (it.second.mType == SliceEngine::ScriptFieldType::GameObject)
							{
								SliceEngine::GameObject data = scriptRef->GetFieldValue<SliceEngine::GameObject>(it.second.mName);

								
								std::function<void(std::string, SliceEngine::GameObject)> func = [sp = scriptRef](std::string name, SliceEngine::GameObject val)
									{
										sp->SetFieldValue(name, val);
									};

								if (GameObjectInputScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data, ScriptGameObjMultipleSelection(selectionManager, script.scriptName, it.second.mName, data, isMultipleSelection)))
								{
									scriptRef->SetFieldValue(it.second.mName, data);
									if (isMultipleSelection)
									{
										ScriptGameObjMultiSet(selectionManager, script.scriptName, it.second.mName, data);
									}
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}
							else if (it.second.mType == SliceEngine::ScriptFieldType::Prefab)
							{
								auto data = scriptRef->GetFieldValue<SliceEngine::PrefabVar>(it.second.mName);
								
								std::function<void(std::string, SliceEngine::PrefabVar)> func = [sp = scriptRef](std::string name, SliceEngine::PrefabVar val)
									{
										sp->SetFieldValue(name, val);
									};

								if (PrefabInputScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetFieldValue(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							
							}
						}
					#pragma endregion
					}
				}

			}

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayAnimator(entt::entity entity)
	{
		auto& animator = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Animator>(entity);

		if (ImGui::TreeNodeEx("Animator", mBaseFlags))
		{
			if (!DisplayComponentHeader<SliceEngine::Animator>(entity))
			{

				//Drag Drop for the controller when its not set
				if(!animator.Handle_stateMachine.IsValid())
				{
					//ImGui::Text("Dont Drag a Controller in Here\nunless ur debugging the crash that \nhappens when you drop a controller!");
					if (HandleDragDropInputHeader(mRegistry, "Controller: ", "##controller", animator.Handle_stateMachine, "Controller"))
					{
						animator.stateMachine.EFSM.stateMap.clear();
						animator.stateMachine.EFSM = *animator.Handle_stateMachine.get();
						animator.stateMachine.InitState(animator.curr_anim_pkg);
						// hmm sussy
					}
				}
				//Controller has been set, should be changable
				else
				{
					if(HandleDragDropInputHeader(mRegistry, "Controller: ", "##controller", animator.Handle_stateMachine, "Controller")) //For changing
					{
						animator.stateMachine.EFSM.stateMap.clear();
						animator.stateMachine.EFSM = *animator.Handle_stateMachine.get();
						if (animator.Handle_skeleton.IsValid())
							animator.stateMachine.InitState(animator.curr_anim_pkg);
						else
							animator.stateMachine.InitState(animator.curr_anims);


						OnAnimatorChangedEvent eventNow{};
						eventNow.ent = entity;
						EventManager::GetInstance()->Publish<OnAnimatorChangedEvent>(eventNow);
					}

					BoolInputHeader(mRegistry, "Playing: ", "##animIsPlaying", animator.timeline.isPlaying);

					//Idk what this is for so im hiding it first
					/*std::string anim_file{};
					ImGui::InputText("##anim", &anim_file, ImGuiInputTextFlags_ReadOnly);*/

					BoolInputHeader(mRegistry, "Loop: ", "##animIsLoop", animator.timeline.isLoop);

					// i have to check, what if an entity has no animation pkg, like the states deal w animation so do i just start balling?
					if(animator.Handle_curr_anim_pkg.IsValid())
					{
						ImGui::Text("Next: ");
						ImGui::SameLine(150.f);
						if (ImGui::Button("##anim_Next", ImVec2(50, 25)))
						{
							animator.stateMachine.EFSM.currState->curr_anim_idx = (animator.stateMachine.EFSM.currState->curr_anim_idx + 1) % animator.curr_anim_pkg.animations.size();
						}

						std::string currStateName{ animator.stateMachine.EFSM.currState->stateName };
						size_t charPos = currStateName.find('|');

						if (charPos != std::string::npos)
						{
							currStateName = currStateName.substr(charPos);
						}

						ImGui::Text("Current Animation: %s , ID: %d", currStateName.c_str(), animator.stateMachine.EFSM.currState->curr_anim_idx);

						ImGui::Text("Prev: ");
						ImGui::SameLine(150.f);
						if (ImGui::Button("##anim_Prev", ImVec2(50, 25)))
						{
							if (animator.stateMachine.EFSM.currState->curr_anim_idx == 0)
								animator.stateMachine.EFSM.currState->curr_anim_idx = static_cast<unsigned int>(animator.curr_anim_pkg.animations.size() - 1);
							else
								animator.stateMachine.EFSM.currState->curr_anim_idx--;
						}
					}
				}
			}
			ImGui::TreePop();
		}
	}

	// particle system helper
	void ButtonValueTypePopup(SliceEngine::ParticleSystem::ValueType& value_type, std::string name)
	{
		std::string btn_name = "v##" + name;
		std::string popup_name = "value_type" + name;

		if (ImGui::Button(btn_name.c_str()))
		{
			ImGui::OpenPopup(popup_name.c_str());
		}

		if (ImGui::BeginPopup(popup_name.c_str()))
		{
			if (ImGui::Selectable("Constant"))
			{
				value_type = SliceEngine::ParticleSystem::ValueType::CONSTANT;
			}
		
			//if (ImGui::Selectable("Curve"))
			//{
			//	value_type = SliceEngine::ParticleSystem::ValueType::CURVE;
			//}

			if (ImGui::Selectable("Random from 2 Constants"))
			{
				value_type = SliceEngine::ParticleSystem::ValueType::TWO_CONSTANTS;
			}

			ImGui::EndPopup();

		}
	}
	
	void InspectorWindow::DisplayParticleSystem(entt::entity entity)
	{
		static std::vector<std::string> value_type_names{ "Constant", "Curve", "Random between 2 constants" };
		if (ImGui::TreeNodeEx("Particle System", mBaseFlags))
		{
			auto& ps = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::ParticleSystem>(entity);

			DisplayComponentHeader<SliceEngine::ParticleSystem>(entity);

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::Button(ps.playPreview ? "Stop Preview" : "Play Preview"))
			{
				ps.playPreview = !ps.playPreview;
			}
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::Button(ps.pausePreview ? "Unpause System" : "Pause System"))
			{
				ps.pausePreview = !ps.pausePreview;
			}
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::Button("Reset System"))
			{
				ps.resetPreview = true;
			}			

			if (ImGui::CollapsingHeader("Initialization", ImGuiTreeNodeFlags_DefaultOpen))
			{
				// Initial Delay
				DragFloatInputHeader(mRegistry, "Initial Delay", "##initialDelay", ps.initialDelay, "%.2f", 0.0f, 100.f);

				// Duration
				DragFloatInputHeader(mRegistry, "Duration", "##duration", ps.duration, "%.2f", 0.0f, 100.f);
				
				// Looping
				BoolInputHeader(mRegistry, "Looping", "##looping", ps.isRepeating);
				
				// Follow GameObject Transform
				BoolInputHeader(mRegistry, "Follow Transform Rotation", "##followTransformRotation", ps.followTransformRotation);

				// Start Speed
				switch (ps.speedValueType)
				{
				case SliceEngine::ParticleSystem::ValueType::CONSTANT:
					DragFloatInputHeader(mRegistry, "Start Speed", "##startSpeed", ps.speed, "%.2f", 0.0f);
					break;

				case SliceEngine::ParticleSystem::ValueType::TWO_CONSTANTS:
					DragFloatInputHeader(mRegistry, "Min Start Speed", "##minStartSpeed", ps.minRandomSpeed, "%.2f", 0.0f);
					DragFloatInputHeader(mRegistry, "Max Start Speed", "##maxStartSpeed", ps.maxRandomSpeed, "%.2f", 0.0f);
					break;
				default:
					break;
				}
				ImGui::SameLine();
				ButtonValueTypePopup(ps.speedValueType, "speed");

				// Start Lifetime
				switch (ps.initialLifetimeType)
				{
				case SliceEngine::ParticleSystem::ValueType::CONSTANT:
					DragFloatInputHeader(mRegistry, "Start Lifetime", "##startLifetime", ps.lifetime, "%.2f", 0.0f, 0.f);
					break;
				case SliceEngine::ParticleSystem::ValueType::TWO_CONSTANTS:
					DragFloatInputHeader(mRegistry, "Min Lifetime", "##minLifetime", ps.minParticleLifetime, "%.2f", 0.0f, 0.f);
					DragFloatInputHeader(mRegistry, "Max Lifetime", "##maxLifetime", ps.maxParticleLifetime, "%.2f", 0.0f, 0.f);
					break;
				default:
					break;
				}
				ImGui::SameLine();
				ButtonValueTypePopup(ps.initialLifetimeType, "lifetime");

				// Start Size
				switch (ps.scaleType)
				{
				case SliceEngine::ParticleSystem::ValueType::CONSTANT:
					DragVec3InputHeader(mRegistry, "Start Size", "##startSize", ps.scale);
					break;

				case SliceEngine::ParticleSystem::ValueType::TWO_CONSTANTS:
					DragVec3InputHeader(mRegistry, "Min Start Size", "##minStartSize", ps.minRandomScale);
					DragVec3InputHeader(mRegistry, "Max Start Size", "##maxStartSize", ps.maxRandomScale);
					break;
				default:
					break;
				}
				ImGui::SameLine();
				ButtonValueTypePopup(ps.scaleType, "size");

				// Start Position
				switch (ps.posValueType)
				{
				case SliceEngine::ParticleSystem::ValueType::CONSTANT:
					DragVec3InputHeader(mRegistry, "Pos Offset", "##startPos", ps.spawnPos);
					break;

				case SliceEngine::ParticleSystem::ValueType::TWO_CONSTANTS:
					DragVec3InputHeader(mRegistry, "Min Pos Offset", "##minStartPos", ps.minRandomSpawnPos);
					DragVec3InputHeader(mRegistry, "Max Pos Offset", "##maxStartPos", ps.maxRandomSpawnPos);
					break;

				default:
					break;
				}
				ImGui::SameLine();
				ButtonValueTypePopup(ps.posValueType, "position");

				// Start Rotation			
				switch (ps.initialRotationType)
				{
				case SliceEngine::ParticleSystem::ValueType::CONSTANT:
					if (ps.isRotation3D)
					{
						DragVec3InputHeader(mRegistry, "Rotation", "##rot3D", ps.rotation3DHint);
					}
					else 
					{
						DragFloatInputHeader(mRegistry, "Rotation", "##rot", ps.rotation, "%.2f", 0.0f, 360.f);
					}					
					break;
				case SliceEngine::ParticleSystem::ValueType::TWO_CONSTANTS:
					if (ps.isRotation3D)
					{
						DragVec3InputHeader(mRegistry, "Min Rotation", "##minRot3D", ps.minRotation3DHint);
						DragVec3InputHeader(mRegistry, "Max Rotation", "##maxRot3D", ps.maxRotation3DHint);
					}
					else 
					{
						DragFloatInputHeader(mRegistry, "Min Rotation", "##minRot", ps.minRandomRotation, "%.2f", 0.f, 360.f);
						DragFloatInputHeader(mRegistry, "Max Rotation", "##maxRot", ps.maxRandomRotation, "%.2f", 0.f, 360.f);
					}					
					break;
				default:
					break;
				}
				ImGui::SameLine();
				ButtonValueTypePopup(ps.initialRotationType, "rotation");
				BoolInputHeader(mRegistry, "3D Rotation", "##is3DRotation", ps.isRotation3D);

				// Start Colour			
				switch (ps.colourValueType)
				{
				case SliceEngine::ParticleSystem::ValueType::CONSTANT:
					DragColor4InputHeader(mRegistry, "Start Colour", "##colorStart", ps.colour);
					break;
				case SliceEngine::ParticleSystem::ValueType::TWO_CONSTANTS:
					DragColor4InputHeader(mRegistry, "Min Colour", "##colorMinStart", ps.minRandomColour);
					DragColor4InputHeader(mRegistry, "Max Colour", "##colorMaxStart", ps.maxRandomColour);
					break;
				default:
					break;
				}
				ImGui::SameLine();
				ButtonValueTypePopup(ps.colourValueType, "colour");

				// Gravity
				DragFloatInputHeader(mRegistry, "Gravity Modifier", "##gravityModifier", ps.gForce, "%.2f", 0.0f, 100.f);

				// Collision
				BoolInputHeader(mRegistry, "Has Collision", "##hasCollision", ps.hasCollision);
				if (ps.hasCollision)
				{
					DragFloatInputHeader(mRegistry, "Friction", "##frictionModifier", ps.friction, "%.2f", 0.0f, 1.0f);
					DragFloatInputHeader(mRegistry, "Bounciness", "##bouncinessModifier", ps.bounciness, "%.2f", 0.0f, 1.0f);
					DragFloatInputHeader(mRegistry, "BounceDampening", "##bounceDampening", ps.bounceDampening, "%.2f", 0.0f, 1.0f);
					DragFloatInputHeader(mRegistry, "Stickiness", "##stickinessModifier", ps.stickiness, "%.2f", 0.0f, 1.0f);
				}

				// Max Particles
				DragUInt64InputHeader(mRegistry, "Max Particles", "##maxParticles", ps.maxParticles, "%llu", 0, 5000);

				// Simulation Space
				BoolInputHeader(mRegistry, "Is Local Space", "##isLocalSpace", ps.isLocalSpace);

				// Destroy
				BoolInputHeader(mRegistry, "Destroy OnEnd", "##destroyOnEnd", ps.destroyOnExpire);
			}

			if (ImGui::CollapsingHeader("Shape"))
			{
				static std::vector<std::string> shapeTypes{ "Sphere", "Cone", "Cube", "Circle", "Rect"};
				// Shape Type Enum
				ComboHeader<SliceEngine::ParticleSystem::ShapeType>(mRegistry, "Shape", "##shapeType", ps.shapeType, shapeTypes);

				switch (ps.shapeType)
				{
				case SliceEngine::ParticleSystem::ShapeType::SPHERE:
					DragFloatInputHeader(mRegistry, "Arc", "##sphereArc", ps.sphereArc, "%.2f", 0.0f, 180.0f);
					DragFloatInputHeader(mRegistry, "Radius", "##sphereRadius", ps.shapeRadius, "%.2f", 0.0f, std::numeric_limits<float>::max());
					DragFloatInputHeader(mRegistry, "Inner Radius", "##innerSphereRadius", ps.innerShapeRadius, "%.2f", 0.0f, ps.shapeRadius);
					break;
				case SliceEngine::ParticleSystem::ShapeType::CONE:
					DragFloatInputHeader(mRegistry, "Arc", "##coneArc", ps.coneArc, "%.2f", 0.0f, 90.0f);
					DragFloatInputHeader(mRegistry, "Radius", "##coneRadius", ps.shapeRadius, " % .2f", 0.0f, std::numeric_limits<float>::max());
					DragFloatInputHeader(mRegistry, "Inner Radius", "##innerConeRadius", ps.innerShapeRadius, "%.2f", 0.0f, ps.shapeRadius);
					break;
				case SliceEngine::ParticleSystem::ShapeType::CUBE:
					DragVec3InputHeader(mRegistry, "Scale", "##cubeScale", ps.shapeScale);
					break;
				case SliceEngine::ParticleSystem::ShapeType::CIRCLE:
					DragFloatInputHeader(mRegistry, "Circle", "##circleRadius", ps.shapeRadius, " % .2f", 0.0f, std::numeric_limits<float>::max());
					DragFloatInputHeader(mRegistry, "Inner Radius", "##innerCircleRadius", ps.innerShapeRadius, "%.2f", 0.0f, ps.shapeRadius);
					break;
				case SliceEngine::ParticleSystem::ShapeType::RECT:					
					DragVec2InputHeader(mRegistry, "Scale", "##rectScale", ps.rectScale);
					break;
				default:
					break;
				}
			}

			if (ImGui::CollapsingHeader("Emission"))
			{
				// Emission Rate
				DragFloatInputHeader(mRegistry, "Emission Rate", "##emission_rate", ps.emissionRate, "%.2f", 0.0f, 0.f);
				
				// Bursts
				if (DragUInt64InputHeader(mRegistry, "Number of Bursts", "##numBursts", ps.numBursts, "%llu", 0, 100)) { ps.bursts.resize(ps.numBursts); };
				if (ImGui::BeginTable("Bursts", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
				{
					ImGui::TableSetupColumn("Time");
					ImGui::TableSetupColumn("Count");
					ImGui::TableSetupColumn("Cycle");
					ImGui::TableSetupColumn("Interval");
					ImGui::TableHeadersRow();
					int counter = 0;
					float itemWidth = 50.0f;
					for (auto& burst : ps.bursts)
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						float columnWidth = ImGui::GetColumnWidth();
						ImGui::SetNextItemWidth(itemWidth);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
						std::string triggerTimeID = ("##burst_triggerTime" + std::to_string(counter));
						DragFloatInput(mRegistry, triggerTimeID.c_str(), burst.triggerTime, "%.2f", 0.0f, ps.duration);
						ImGui::TableNextColumn();
						ImGui::SetNextItemWidth(itemWidth);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
						std::string burstCountID = ("##burst_count" + std::to_string(counter));
						DragUInt64Input(mRegistry, burstCountID.c_str(), burst.numParticles, "%llu", 0, UINT_MAX);
						ImGui::TableNextColumn();
						ImGui::SetNextItemWidth(itemWidth);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
						std::string burstCycleID = ("##burst_cycle" + std::to_string(counter));
						DragUInt64Input(mRegistry, burstCycleID.c_str(), burst.burstRepetitions, "%llu", 0, UINT_MAX);
						ImGui::TableNextColumn();
						ImGui::SetNextItemWidth(itemWidth);
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
						std::string burstIntervalID = ("##burst_interval" + std::to_string(counter));
						DragFloatInput(mRegistry, burstIntervalID.c_str(), burst.burstPeriod, "%.2f", 0.0f, FLT_MAX);
						++counter;
					}
					ImGui::EndTable();
				}
			}

			if (ImGui::CollapsingHeader("Size Over Lifetime"))
			{
				BoolInputHeader(mRegistry, "Size Over Lifetime", "##sizeOverLifetime", ps.sizeOverLifetime);
				if (ps.sizeOverLifetime)
				{
					BoolInputHeader(mRegistry, "Separate Axis", "##sizeSeparateAxis", ps.sizeSeparateAxis);					

					auto num = ps.sizeMapIntermediary.size();
					if (DragUInt64InputHeader(mRegistry, "Number of Points", "##numSizePoints", num, "%llu", 0, 10))
					{ 
						ps.sizeMapIntermediary.resize(num);
					};


					if (ImGui::BeginTable("Size Over Lifetime", ps.sizeSeparateAxis ? 4 : 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
					{
						ImGui::TableSetupColumn("Time");

						if (ps.sizeSeparateAxis)
						{
							ImGui::TableSetupColumn("X");
							ImGui::TableSetupColumn("Y");
							ImGui::TableSetupColumn("Z");
						}
						else 
						{
							ImGui::TableSetupColumn("Size Multiplier");
						}
						
						ImGui::TableHeadersRow();

						int counter = 0;
						float itemWidth = 50.0f;
						bool modified{ false };
						for (auto& kv : ps.sizeMapIntermediary)
						{
							auto& time = kv.first;
							auto& value = kv.second;

							ImGui::TableNextRow();

							ImGui::TableNextColumn();
							float columnWidth = ImGui::GetColumnWidth();
							ImGui::SetNextItemWidth(itemWidth);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
							std::string sizeTime = ("##sizeMap_Time" + std::to_string(counter));
							modified |= DragFloatInput(mRegistry, sizeTime.c_str(), time, "%.2f", 0.0f, 1.0f);

							if (ps.sizeSeparateAxis)
							{
								ImGui::TableNextColumn();
								ImGui::SetNextItemWidth(itemWidth);
								ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
								std::string sizeX = ("##sizeMap_X" + std::to_string(counter));
								modified |= DragFloatInput(mRegistry, sizeX.c_str(), value.x, "%.2f", 0.0f, FLT_MAX);

								ImGui::TableNextColumn();
								ImGui::SetNextItemWidth(itemWidth);
								ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
								std::string sizeY = ("##sizeMap_Y" + std::to_string(counter));
								modified |= DragFloatInput(mRegistry, sizeY.c_str(), value.y, "%.2f", 0.0f, FLT_MAX);
							}

							ImGui::TableNextColumn();
							ImGui::SetNextItemWidth(itemWidth);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
							std::string sizeZ = ("##sizeMap_Z" + std::to_string(counter));
							modified |= DragFloatInput(mRegistry, sizeZ.c_str(), value.z, "%.2f", 0.0f, FLT_MAX);

							++counter;
						}
						ImGui::EndTable();
						
						// If there is any change, rebuild the map
						if (modified)
						{
							ps.sizeMap.clear();
							for (const auto& kv : ps.sizeMapIntermediary)
							{
								ps.sizeMap.insert_or_assign(kv.first, kv.second);
							}
						}												
					}
				}
			}

			if (ImGui::CollapsingHeader("Rotate Over Lifetime"))
			{
				BoolInputHeader(mRegistry, "Rotate Over Lifetime", "##rotateOverLifetime", ps.rotateOverLifetime);
				if (ps.rotateOverLifetime)
				{
					BoolInputHeader(mRegistry, "Separate Axis", "##rotateSeparateAxis", ps.rotateSeparateAxis);
					if (ps.rotateSeparateAxis)
					{
						DragVec3InputHeader(mRegistry, "Rotate Velocity", "##rotateVelocity3D", ps.rotateVelocity);
					}
					else 
					{
						DragFloatInputHeader(mRegistry, "Rotate Velocity", "##rotateVelocity", ps.rotateVelocity.z, "%.1f", 0.0f, FLT_MAX);
					}
				}
			}

			if (ImGui::CollapsingHeader("Color Over Lifetime"))
			{
				BoolInputHeader(mRegistry, "Colour Over Lifetime", "##colourOverLifetime", ps.colourOverLifetime);
				if (ps.colourOverLifetime)
				{
					auto num = ps.colourMapIntermediary.size();
					if (DragUInt64InputHeader(mRegistry, "Number of Points", "##numColPoints", num, "%llu", 0, 10))
					{
						auto oldSize = ps.colourMapIntermediary.size();
						ps.colourMapIntermediary.resize(num);

						for (size_t i = oldSize; i < num; ++i)
						{
							ps.colourMapIntermediary[i].first = 1.0f;              // or i / (num - 1)
							ps.colourMapIntermediary[i].second = glm::vec4(1.0f);  // white
						}
					};

					if (ImGui::BeginTable("Colour Over Lifetime", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
					{
						ImGui::TableSetupColumn("Time");
						ImGui::TableSetupColumn("Colour");
						ImGui::TableHeadersRow();

						int counter = 0;
						float itemWidth = 50.0f;
						bool modified{ false };
						for (auto& kv : ps.colourMapIntermediary)
						{
							auto& time = kv.first;
							auto& value = kv.second;

							ImGui::TableNextRow();

							ImGui::TableNextColumn();
							float columnWidth = ImGui::GetColumnWidth();
							ImGui::SetNextItemWidth(itemWidth);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
							std::string colTime = ("##colourMap_Time" + std::to_string(counter));
							modified |= DragFloatInput(mRegistry, colTime.c_str(), time, "%.2f", 0.0f, 1.0f);

							ImGui::TableNextColumn();
							ImGui::SetNextItemWidth(itemWidth);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
							std::string colVal = ("##colourMap_Colour" + std::to_string(counter));
							modified |= DragColor4InputHeader(mRegistry, "", colVal.c_str(), value);
							
							++counter;
						}
						ImGui::EndTable();

						// If there is any change, rebuild the map
						if (modified)
						{
							ps.colourLifetimeMap.clear();
							for (const auto& kv : ps.colourMapIntermediary)
							{
								ps.colourLifetimeMap[kv.first] = kv.second;
							}
						}
					}
				}
			}

			if (ImGui::CollapsingHeader("Velocity Over Lifetime"))
			{
				BoolInputHeader(mRegistry, "Velocity Over Lifetime", "##velocityOverLifetime", ps.velocityOverLifetime);
				if (ps.velocityOverLifetime)
				{
					BoolInputHeader(mRegistry, "Separate Axis", "##velocitySeparateAxis", ps.velocitySeparateAxis);

					auto num = ps.velocityMapIntermediary.size();
					if (DragUInt64InputHeader(mRegistry, "Number of Points", "##numVelPoints", num, "%llu", 0, 10))
					{
						auto oldSize = ps.velocityMapIntermediary.size();
						ps.velocityMapIntermediary.resize(num);

						for (size_t i = oldSize; i < num; ++i)
						{
							ps.velocityMapIntermediary[i].first = 1.0f;              // or i / (num - 1)
							ps.velocityMapIntermediary[i].second = glm::vec3(1.0f);  // white
						}
					};

					if (ImGui::BeginTable("Velocity Over Lifetime", ps.velocitySeparateAxis ? 4 : 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
					{
						ImGui::TableSetupColumn("Time");

						if (ps.velocitySeparateAxis)
						{
							ImGui::TableSetupColumn("X");
							ImGui::TableSetupColumn("Y");
							ImGui::TableSetupColumn("Z");
						}
						else
						{
							ImGui::TableSetupColumn("Velocity Multiplier");
						}

						ImGui::TableHeadersRow();

						int counter = 0;
						float itemWidth = 50.0f;
						bool modified{ false };
						for (auto& kv : ps.velocityMapIntermediary)
						{
							auto& time = kv.first;
							auto& value = kv.second;

							ImGui::TableNextRow();

							ImGui::TableNextColumn();
							float columnWidth = ImGui::GetColumnWidth();
							ImGui::SetNextItemWidth(itemWidth);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
							std::string velTime = ("##velocityMap_Time" + std::to_string(counter));
							modified |= DragFloatInput(mRegistry, velTime.c_str(), time, "%.2f", 0.0f, 1.0f);

							if (ps.velocitySeparateAxis)
							{
								ImGui::TableNextColumn();
								ImGui::SetNextItemWidth(itemWidth);
								ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
								std::string velocityX = ("##velocityMap_X" + std::to_string(counter));
								modified |= DragFloatInput(mRegistry, velocityX.c_str(), value.x, "%.2f", -FLT_MAX, FLT_MAX);

								ImGui::TableNextColumn();
								ImGui::SetNextItemWidth(itemWidth);
								ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
								std::string velocityY = ("##velocityMap_Y" + std::to_string(counter));
								modified |= DragFloatInput(mRegistry, velocityY.c_str(), value.y, "%.2f", -FLT_MAX, FLT_MAX);
							}

							ImGui::TableNextColumn();
							ImGui::SetNextItemWidth(itemWidth);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - itemWidth) * 0.5f);
							std::string velocityZ = ("##velocityMap_Z" + std::to_string(counter));
							modified |= DragFloatInput(mRegistry, velocityZ.c_str(), value.z, "%.2f", -FLT_MAX, FLT_MAX);

							++counter;
						}
						ImGui::EndTable();

						// If there is any change, rebuild the map
						if (modified)
						{
							ps.velocityMap.clear();
							for (const auto& kv : ps.velocityMapIntermediary)
							{
								ps.velocityMap.insert_or_assign(kv.first, kv.second);
							}
						}
					}					
				}
			}

			if (ImGui::CollapsingHeader("Orbit Over Lifetime"))
			{
				BoolInputHeader(mRegistry, "Orbit Over Lifetime", "##orbitOverLifetime", ps.orbitOverLifetime);
				if (ps.orbitOverLifetime)
				{
					DragVec3InputHeader(mRegistry, "Orbit Axis", "##orbitAxis", ps.orbitAxis);
					DragVec3InputHeader(mRegistry, "Start Velocity", "##startOrbitVelocity", ps.startOrbitVelocity);
					DragVec3InputHeader(mRegistry, "End Velocity", "##endOrbitVelocity", ps.endOrbitVelocity);
				}
			}

			if (ImGui::CollapsingHeader("Renderer"))
			{
				static std::vector<std::string> render_mode_names = { "Billboard", "Mesh" };
				ComboHeader(mRegistry, "Render Mode", "##ps_render_mode", ps.renderMode, render_mode_names);

				BoolInputHeader(mRegistry, "Billboard", "##alwaysFaceCamera", ps.alwaysFaceCamera);

				SliceEngine::GUID tex_guid = ps.textureGUID;
				switch (ps.renderMode)
				{
					case SliceEngine::ParticleSystem::RenderMode::BILLBOARD:						
						BoolInputHeader(mRegistry, "Ignore Lights", "##ignoreLighting", ps.ignoreLights);
						GUIDDragDropInputHeader(mRegistry, "Image", "##spriteimage", tex_guid, "Texture");
						ps.textureGUID = tex_guid;
						break;					
					case SliceEngine::ParticleSystem::RenderMode::MESH:
						HandleDragDropInputHeader<SliceEngine::SliceEngineTypes::Model>(mRegistry, "Mesh", "##ps_mesh", ps.modelHandle, "Model");
						HandleDragDropInputHeader<SliceEngine::SliceEngineTypes::Material>(mRegistry, "Material", "##ps_mat", ps.materialHandle, "Material", nullptr);
						break;
					default:
						break;
				}
				auto core = SliceEngine::Core::GetInstance();
				auto layer_manager = core->GetLayerManager();
				auto layer_name_list = layer_manager->GetLayerNameList();

				ComboHeader(mRegistry, "Particle Layer", "##particle_layer", ps.particleLayer, layer_name_list);
			}
			if (ImGui::CollapsingHeader("Post-Processing Effects"))
			{
				BoolInputHeader(mRegistry, "Glow", "##Glow", ps.glow);
				if (ps.glow)
				{
					switch (ps.glowValueType)
					{
					case SliceEngine::ParticleSystem::ValueType::CONSTANT:
						DragFloatInputHeader(mRegistry, "GlowIntensity", "##glowIntensity", ps.glowIntensity, "%.2f", 0.0f, FLT_MAX);
						break;

					case SliceEngine::ParticleSystem::ValueType::TWO_CONSTANTS:
						DragFloatInputHeader(mRegistry, "minGlowIntensity", "##minGlowIntensity", ps.minGlowIntensity, "%.2f", 0.0f, FLT_MAX);
						DragFloatInputHeader(mRegistry, "maxGlowIntensity", "##maxGlowIntensity", ps.maxGlowIntensity, "%.2f", 0.0f, FLT_MAX);
						break;
					default:
						break;
					}
					ImGui::SameLine();
					ButtonValueTypePopup(ps.glowValueType, "glowValueType");
				}
			}

			ImGui::TreePop();
		}

	}


	void InspectorWindow::DisplayLight(entt::entity entity)
	{
		if (ImGui::TreeNodeEx("Light", mBaseFlags))
		{
			auto& light = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Light>(entity);

			DisplayComponentHeader<SliceEngine::Light>(entity);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", light.componentEnabled);
			BoolInputHeader(mRegistry, "Casts Shadow", "##lightCastsShadow", light.castsShadow);

			//DragVec3InputHeader(mRegistry, "Colour", "##c", light.color);
			DragColor3InputHeader(mRegistry, "Colour", "##lightColor", light.color);

			DragFloatInputHeader(mRegistry, "Intensity", "##intensity", light.intensity, "%.2f", 0.0f, 1000.f);

			static std::vector<std::string> lightTypes { "Directional Light", "Point Light", "Spot Light" };

			ComboHeader<SliceEngine::Light::LightType>(mRegistry, "Light Type", "##lightType", light.type, lightTypes);

			if (light.type == 2)
				DragFloatInputHeader(mRegistry, "angle", "##light_angle", light.angle, "%.2f", 0.0f, 90.f);

			ImGui::TreePop();
		}
	}

	void InspectorWindow::AddComponentButton(entt::entity entity)
	{

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("##add_component_list");
		}

		if (ImGui::BeginPopupContextItem("##add_component_list"))
		{
			auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
			auto selectedGO = SliceEngine::FactoryInstance.GetGOByEntity(entity);

			if(!selectedGO.HasComponent<SliceEngine::Renderer>())
			{
				if (ImGui::Selectable("Add Renderer"))
				{
					reg.emplace<SliceEngine::Renderer>(entity);
				}
			}

			if (!selectedGO.HasComponent<SliceEngine::RigidBody>())
			{
				if (ImGui::Selectable("Add Rigidbody"))
				{
					reg.emplace<SliceEngine::RigidBody>(entity);
				}
			}


			//if (!selectedGO.HasComponent<SliceEngine::NavMeshLink>())
			//{
			//	if (ImGui::Selectable("Add Nav Mesh Link"))
			//	{
			//		reg.emplace<SliceEngine::NavMeshLink>(entity);
			//	}
			//}


			if(!selectedGO.HasComponent<SliceEngine::ColliderShape>())
			{
				if (ImGui::Selectable("Add Box Collider"))
				{
					SliceEngine::ColliderShape boxData{};
					boxData.shapeData = SliceEngine::ColliderShape::BoxData{};
					/*auto& col = */reg.emplace<SliceEngine::ColliderShape>(entity,boxData);

				}

				if (ImGui::Selectable("Add Sphere Collider"))
				{
					SliceEngine::ColliderShape sphereData{};
					sphereData.shapeData = SliceEngine::ColliderShape::SphereData{};
					/*auto& col =*/ reg.emplace<SliceEngine::ColliderShape>(entity, sphereData);
				}

				if (ImGui::Selectable("Add Capsule Collider"))
				{
					SliceEngine::ColliderShape capsuleData{};
					capsuleData.shapeData = SliceEngine::ColliderShape::CapsuleData{};
					/*auto& col =*/ reg.emplace<SliceEngine::ColliderShape>(entity,capsuleData);
					
				}

				if (ImGui::Selectable("Add Mesh Collider"))
				{
					SliceEngine::ColliderShape meshData{};
					meshData.shapeData = SliceEngine::ColliderShape::MeshData{};
					/*auto& col =*/ reg.emplace<SliceEngine::ColliderShape>(entity, meshData);

				}

				if (ImGui::Selectable("Add Cylinder Collider"))
				{
					SliceEngine::ColliderShape cylinderData{};
					cylinderData.shapeData = SliceEngine::ColliderShape::CylinderData{};
					/*auto& col =*/ reg.emplace<SliceEngine::ColliderShape>(entity, cylinderData);

				}
			}
			
			if(!selectedGO.HasComponent<SliceEngine::Script>())
			{
				if (ImGui::Selectable("Add Script Container"))
				{
					reg.emplace<SliceEngine::Script>(entity);
				}
			}

			if(!selectedGO.HasComponent<SliceEngine::AudioSource>())
			{
				if (ImGui::Selectable("Add AudioSource"))
				{
					SliceEngine::Core::GetInstance()->GetRegistry().emplace<SliceEngine::AudioSource>(entity);
				}
			}

			if (!selectedGO.HasComponent<SliceEngine::AudioListener>())
			{
				if (ImGui::Selectable("Add AudioListener"))
				{
					SliceEngine::Core::GetInstance()->GetRegistry().emplace<SliceEngine::AudioListener>(entity);
				}
			}

			if (!selectedGO.HasComponent<SliceEngine::Light>())
			{
				if (ImGui::Selectable("Add LightSource"))
				{
					SliceEngine::Core::GetInstance()->GetRegistry().emplace<SliceEngine::Light>(entity);
				} 
			}

			if (!selectedGO.HasComponent<SliceEngine::ParticleSystem>())
			{
				if (ImGui::Selectable("Add Particle System"))
				{
					SliceEngine::Core::GetInstance()->GetRegistry().emplace<SliceEngine::ParticleSystem>(entity);
				}
			}

			if (!selectedGO.HasComponent<SliceEngine::Animator>())
			{
				if (ImGui::Selectable("Add Animator"))
				{
					SliceEngine::Core::GetInstance()->GetRegistry().emplace<SliceEngine::Animator>(entity);
				}
			}
			if (!selectedGO.HasComponent<SliceEngine::RectTransform>())
			{
				if (ImGui::Selectable("Add Rect Transform"))
				{
					reg.emplace<SliceEngine::RectTransform>(entity);
				}
			}
			if (!selectedGO.HasComponent<SliceEngine::SpriteRenderer>() && selectedGO.HasComponent<SliceEngine::RectTransform>())
			{
				if (ImGui::Selectable("Add Sprite"))
				{
					//auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
					reg.emplace<SliceEngine::SpriteRenderer>(entity);
					auto& ui_sprite = reg.get<SliceEngine::SpriteRenderer>(entity);
					ui_sprite.rgba = { 1.f,1.f,1.f,1.f };
					ui_sprite.textureHandle = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::COLOR_DEADED_DEFAULT;
				}
			}

			if (!selectedGO.HasComponent<SliceEngine::SpriteAnimator>()
				&& selectedGO.HasComponent<SliceEngine::RectTransform>()
				&& selectedGO.HasComponent<SliceEngine::SpriteRenderer>())
			{
				if (ImGui::Selectable("Add Sprite Gamma Override"))
				{
					reg.emplace<SliceEngine::SpriteRendererGammaOverride>(entity);
				}
			}

			if (!selectedGO.HasComponent<SliceEngine::SpriteAnimator>() 
				&& selectedGO.HasComponent<SliceEngine::RectTransform>()
				&& selectedGO.HasComponent<SliceEngine::SpriteRenderer>())
			{
				if (ImGui::Selectable("Add Sprite Animator"))
				{
					reg.emplace<SliceEngine::SpriteAnimator>(entity);
				}
			}

			if (!selectedGO.HasComponent<SliceEngine::FontRenderer>() && selectedGO.HasComponent<SliceEngine::RectTransform>())
			{
				if (ImGui::Selectable("Add Font"))
				{
					//auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
					reg.emplace<SliceEngine::FontRenderer>(entity);
					auto& ui_font = reg.get<SliceEngine::FontRenderer>(entity);
					ui_font.rgba = { 0.f,0.f,0.f,1.f };
					ui_font.font_size = 50;
					ui_font.line_spacing = 1.25f;
					ui_font.fontHandle = (SliceEngine::GUID)SliceEngine::DefaultResourceIDs::FONT_BLANK_DEFAULT;
				}
			}

			ImGui::EndPopup();
		}
	}

	void InspectorWindow::DisplayEntity(EntityNode* node)
	{
		auto sessionManager = mRegistry.GetManager<SessionManager>("Session");
		if (node->entity == SliceEngine::FactoryInstance.GetRootEntity())
		{
			return;
		}
		if (!SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Prefab>(node->entity))
		{
			if (ImGui::Button("Create New Prefab"))
			{
				SliceEngine::GameObject go = SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(node->entity);
				mRegistry.GetAssetManager().CreatePrefab(go);
				mRegistry.GetManager<SessionManager>("Session")->SetNodeAsPrefab(node, true);
			}
		}

		else
		{
			if(!sessionManager->IsPrefabInspected())
			{
				if (ImGui::Button("Remove Prefab Component"))
				{
					EditorUtilities::GameObject_Unprefab(node->entity);
					mRegistry.GetManager<SessionManager>("Session")->SetNodeAsPrefab(node, false);
				}
			}
		}

		DisplayEntityData(node->entity);

		//Loop through registered components and display them if they exist on the selected entity

		//for (auto&& [typeID, storage] : SliceEngine::Core::GetInstance()->GetRegistry().storage())
		//{

		//}

		// to do : use gamefactory component view
		if (SliceEngine::Core::GetInstance()->GetRegistry().valid(node->entity))
		{
			auto entity = node->entity;
			DisplayTransform(node->entity);
			ImGui::Separator();

			//DisplaySceneGraph();
			//ImGui::Separator();

			if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::RectTransform>(entity))
			{
				DisplayRectTransform(node->entity);
				ImGui::Separator();
			}

			if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::SpriteRenderer>(entity))
			{
				DisplaySpriteRenderer(node->entity);
				ImGui::Separator();
			}
			if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::SpriteRendererGammaOverride>(entity))
			{
				DisplaySpriteRendererGammaOverride(node->entity);
				ImGui::Separator();
			}
			if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::SpriteAnimator>(entity))
			{
				DisplaySpriteAnimator(node->entity);
				ImGui::Separator();
			}
			if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::FontRenderer>(entity))
			{
				DisplayFontRenderer(node->entity);
				ImGui::Separator();
			}

			if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Canvas>(entity))
			{
				DisplayCanvas(node->entity);
				ImGui::Separator();
			}

			if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Button>(entity))
			{
				DisplayButton(node->entity);
				ImGui::Separator();
			}

			if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Slider>(entity))
			{
				DisplaySlider(node->entity);
				ImGui::Separator();
			}

			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Camera>(entity))
			{
				DisplayCamera(node->entity);
				ImGui::Separator();
			}

			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Light>(entity))
			{
				DisplayLight(node->entity);
				ImGui::Separator();
			}

			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Renderer>(entity))
			{
				DisplayMeshRenderer(node->entity);
				ImGui::Separator();
			}

			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::ColliderShape>(entity))
			{
				DisplayCollider3D(node->entity);
				ImGui::Separator();
			}

			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::RigidBody>(entity))
			{
				DisplayRigidbody(node->entity);
				ImGui::Separator();
			}


			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::AudioSource>(entity))
			{
				DisplayAudioSource(node->entity);
				ImGui::Separator();
			}

			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::AudioListener>(entity))
			{
				DisplayAudioListener(node->entity);
				ImGui::Separator();
			}

			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::ParticleSystem>(entity))
			{
				DisplayParticleSystem(node->entity);
				ImGui::Separator();
			}

			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Animator>(entity))
			{
				DisplayAnimator(node->entity);
				ImGui::Separator();
			}

			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Script>(entity))
			{
				DisplaySliceScript(node->entity);
				ImGui::Separator();
			}

			/*if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::SceneGraph>(entity))
			{
				DisplaySceneGraph(node->entity);
				ImGui::Separator();
			}*/
            
			AddComponentButton(node->entity);
		}

	}

	void InspectorWindow::DisplayMaterial(DirectoryNode* node)
	{
		MaterialData mat;
		std::filesystem::path mat_path = node->fileName;
		std::string buffer{};
		static float f_buffer{};

		ImGui::BeginGroup();
		ImGui::Text("Material Name: ");
		ImGui::SameLine();
		ImGui::Text(node->fileName.c_str());
		ImGui::EndGroup();
		
		//auto metapath = SliceEngine::Core::GetInstance()->GetResourceManager()->GetResourcePath(mat_path.stem().string());

		//if (metapath.has_value())
		mat.DeserializeAsset(node->fullPath);
		if (GUIDDragDropInputHeader(mRegistry, "Custom Shader:", "##customshdr", mat.shader, "CustomShader"))
		{
			mat.SerializeAsset(node->fullPath);
			return;
		}

		if (BoolInputHeader(mRegistry, "Is Translucent", "##mat_Translucency", mat.isTranslucent))
		{
			mat.SerializeAsset(node->fullPath);
		}

		if (BoolInputHeader(mRegistry, "Ignore Lights", "##mat_ignore_lights", mat.isIgnoreLighting))
		{
			mat.SerializeAsset(node->fullPath);
		}
		
		if (DragColor4InputHeader(mRegistry, "Material Colour", "##mat_color", mat.color))
		{
			mat.SerializeAsset(node->fullPath);
		}

		if (DragColor4InputHeader(mRegistry, "Material Emission Colour", "##mat_emission_color", mat.color2))
		{
			mat.SerializeAsset(node->fullPath);
		}
		auto shdr = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::CustomShader>(mat.shader);
		for (auto& i : shdr.get()->dataIn)
		{
			auto datInMat = mat.data.find(i.name);
			if (datInMat == mat.data.end())
				continue;

			switch (i.dataType)
			{
			case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::BOOL:
			{
				std::string s = "##Material_Bool_" + i.name;
				bool tempBool{ std::get<bool>(datInMat->second) };
				if (BoolInputHeader(mRegistry, i.name.c_str(), s.c_str(), tempBool))
				{
					mat.data[i.name] = tempBool;
					mat.SerializeAsset(node->fullPath);
				}
				break;
			}
			case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::UINT:
			{
				std::string s = "##Material_Uint_" + i.name;
				if(DragUInt32InputHeader(mRegistry, i.name.c_str(), s.c_str(), std::get<uint32_t>(datInMat->second), "%.u", 0, UINT_MAX))
					mat.SerializeAsset(node->fullPath);
				break;
			}
			case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::INT:
			{
				std::string s = "##Material_Int_" + i.name;
				if(DragIntInputHeader(mRegistry, i.name.c_str(), s.c_str(), std::get<int32_t>(datInMat->second), "%.d", -INT_MAX, INT_MAX))
					mat.SerializeAsset(node->fullPath);
				break;
			}
			case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::FLOAT:
			{
				std::string s = "##Material_Float_" + i.name;
				if(DragFloatInputHeader(mRegistry, i.name.c_str(), s.c_str(), std::get<float>(datInMat->second), "%.2f", 0.0f, FLT_MAX, 0.01f))
					mat.SerializeAsset(node->fullPath);
				break;
			}
			case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::TEXTURE:
			{
				std::string s = "##Material_Texture_" + i.name;
				if (GUIDDragDropInputHeader(mRegistry, i.name.c_str(), s.c_str(), std::get<SliceEngine::GUID>(datInMat->second), "Texture"))
					mat.SerializeAsset(node->fullPath);
				break;
			}
			default:
			{
				ImGui::Text(i.name.c_str());
				ImGui::SameLine(150.f);
			}
			}
		}

		//std::string mat_file_name{};
		//if (mRegistry.GetAssetManager().mGUIDtoFilename.find(mat.albedo) != mRegistry.GetAssetManager().mGUIDtoFilename.end())
		//{
		//	mat_file_name = mRegistry.GetAssetManager().mGUIDtoFilename[mat.albedo];
		//}
		//else
		//{
		//	mat_file_name = "GUID not in map";
		//}

		//ImGui::Text("Albedo");
		//ImGui::SameLine(150.0f);
		//ImGui::InputText("##albedo", &mat_file_name, ImGuiInputTextFlags_ReadOnly);

		//if (ImGui::BeginDragDropTarget())
		//{
		//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Texture"))
		//	{
		//		SliceEngine::GUID recievedPayload(*(SliceEngine::GUID*)payload->Data);
		//		//auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
		//		mat.albedo =recievedPayload;
		//		mat.SerializeAsset(node->path);
		//		// update the handle after
		//	}
		//	ImGui::EndDragDropTarget();
		//}

		//if (DragFloatInputHeader(mRegistry, "Roughness", "##roughness", mat.roughness, "%.2f", 0.0f, 1.0f))
		//{
		//	mat.SerializeAsset(node->fullPath);
		//}

		//if (DragFloatInputHeader(mRegistry, "Metallic", "##metallic", mat.metallic, "%.2f", 0.0f, 1.0f))
		//{
		//	mat.SerializeAsset(node->fullPath);
		//}
	}

	void InspectorWindow::DisplayState(StateNode* node)
	{
		auto anim_data = mRegistry.GetManager<SessionManager>("Session")->GetAnimatorData();
		auto animator = mRegistry.GetManager<SessionManager>("Session")->currentAnimator;

		ImGui::SeparatorText("State");

		if (!anim_data)
			return;

		if (!animator)
			return;

		/*auto state_it = anim_data->mStateMachineAsset->stateMap.find(node->name);
		if (state_it == anim_data->mStateMachineAsset->stateMap.end())
			return;

		auto& state = state_it->second;*/

		auto state_it = animator->stateMachine.EFSM.stateMap.find(node->name);
		if (state_it == animator->stateMachine.EFSM.stateMap.end())
			return;

		auto& state = state_it->second;

		StringInputHeader(mRegistry, "Name", "##state_name", state.stateName);

		BoolInputHeader(mRegistry, "isLoop", "##state_is_loop", state.isLoop);

		
		float speedBuffer{ 1 };

		//mCurrentAnimator->Handle_skeleton.IsValid()

		if (!animator->Handle_skeleton.IsValid())
		{
			speedBuffer = animator->stateMachine.EFSM.stateMap[state.stateName].animationSpeed;
		}
		else
		{
			speedBuffer = animator->stateMachine.EFSM.stateMap[state.stateName].animationSpeed;

		}

		DragFloatInputHeader(mRegistry, "Speed:", "##anim_speed", speedBuffer, "%0.3f", 0.1f, 10.0f);

		if (!animator->Handle_skeleton.IsValid())
		{
			if (std::abs(speedBuffer - animator->stateMachine.EFSM.stateMap[state.stateName].animationSpeed) > FLT_EPSILON)
			{
				animator->stateMachine.EFSM.stateMap[state.stateName].animationSpeed = speedBuffer;
			}
		}
		else
		{
			if (std::abs(speedBuffer - animator->stateMachine.EFSM.stateMap[state.stateName].animationSpeed) > FLT_EPSILON)
			{
				animator->stateMachine.EFSM.stateMap[state.stateName].animationSpeed = speedBuffer;
			}
		}
		

		ImGui::SeparatorText("Transitions");

		for (auto& transition : state.transitions)
		{
			if (ImGui::TreeNodeEx(transition.targetState.c_str()))
			{
				BoolInputHeader(mRegistry, "Has Exit Time: ", "##hasExitTime", transition.hasExitTime);
				DragFloatInputHeader(mRegistry, "Entry Time: ", "##entryTime", transition.entryTime, "%.2f", 0.0f, 1.0f, 0.1f);
				DragFloatInputHeader(mRegistry, "Exit Time: ", "##exitTime", transition.entryTime, "%.2f", 0.0f, 1.0f, 0.1f);

				ImGui::TreePop();
			}
		}
	

		//auto state = node->state;
		//
		//ImGui::Text("State");
		//StringInputHeader(mRegistry, "Name", "##state_name", state->stateName);
		//
		//for (auto& transition : state->transitions)
		//{
		//	
		//}
	}

	void InspectorWindow::DisplayTransition(TransitionLinkNode* node)
	{
		auto anim_data = mRegistry.GetManager<SessionManager>("Session")->GetAnimatorData();

		ImGui::SeparatorText("Transition");

		if (!anim_data)
			return;
		
		auto stateOpt = anim_data->GetState(node->source_id);

		if (!stateOpt.has_value())
			return;
		auto transitionOpt = anim_data->GetTransition(stateOpt.value(), node->id);

		if (!transitionOpt.has_value())
			return;

		//auto& transition = transitionOpt.value().get();

		ImGui::Text("Source State");
		ImGui::SameLine(150.f);
		ImGui::Text(transitionOpt->get().sourceState.c_str());

		ImGui::Text("Target State");
		ImGui::SameLine(150.f);
		ImGui::Text(transitionOpt->get().targetState.c_str());

		auto params = anim_data->GetParameters();

		/*auto& condition = transition.condition;

		if (condition.is_type<float>())
		{
			DragFloatInputHeader(mRegistry, stateOpt.value().get().stateName.c_str(), "##condition", condition.get_value<float>());
		}

		else if (condition.is_type<int>())
		{
			DragIntInputHeader(mRegistry, stateOpt.value().get().stateName.c_str(), "##condition", condition.get_value<int>());
		}

		else if (condition.is_type<bool>())
		{
			BoolInputHeader(mRegistry, stateOpt.value().get().stateName.c_str(), "##condition", condition.get_value<bool>());
		}*/

		//auto& params = anim_data->mStateMachineAsset->parameters;
		//auto& transition = anim_data->mTransitionNodes.at(node->id);
		////auto& state = anim_data->mStateMachineAsset->stateMap.at(node->name);

		//ImGui::Text("Target State");
		//ImGui::Text(transition.targetState.c_str());

		//std::string id = "##param" + std::to_string(transition.id);

		//if (transition.condition.is_type<float>())
		//	DragFloatInputHeader(mRegistry, transition.parameterName.c_str(), id.c_str(), transition.condition.get_value<float>(), "%.2f", 0.0f, 1.0f);

		//if (transition.condition.is_type<int>())
		//	DragIntInputHeader(mRegistry, transition.parameterName.c_str(), id.c_str(), transition.condition.get_value<int>());

		//if (transition.condition.is_type<float>())
		//	BoolInputHeader(mRegistry, transition.parameterName.c_str(), id.c_str(), transition.condition.get_value<bool>());
	}

	void InspectorWindow::DisplayPrefab(EntityNode* node)
	{
		//Save Prefab
		auto historyManager = mRegistry.GetManager<HistoryManager>("History");
		if (ImGui::Button("Save Prefab"))
		{
			auto mSession = mRegistry.GetManager<SessionManager>("Session");

			//Publish the engine events:
			OnPrefabModifiedEvent modifiedEvent(mSession->GetPrefabEntityInspected(), mSession->GetPrefabGUIDInspected());
			OnPrefabSerializedEvent serializedEvent(mSession->GetPrefabEntityInspected(), mSession->GetPrefabGUIDInspected());
			EventManager::GetInstance()->Publish<OnPrefabSerializedEvent>(serializedEvent);

			EventManager::GetInstance()->Publish<OnPrefabModifiedEvent>(modifiedEvent);

			////Serialise the Prefab
			SliceEngine::JSONSerializer::SerializePrefab(mSession->GetPrefabEntityInspected());

			//historyManager->ClearFromCheckpoint();

			//For editor handling (only enable if we close the prefab viewer on Saving
			//PrefabInspectedEvent event;
			//event.prefabBeingInspected = false;
			//EventManager::GetInstance()->Publish<PrefabInspectedEvent>(event);
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			historyManager->ClearFromCheckpoint();
			PrefabInspectedEvent event;
			event.prefabBeingInspected = false;
			EventManager::GetInstance()->Publish<PrefabInspectedEvent>(event);
			return;
		}
		
		DisplayEntity(node);
	}

	/*void InspectorWindow::DisplaySceneGraph(entt::entity entity)
	{
		auto& sg = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(entity);

		entt::entity ent_display{};
		ImGui::Text("Parent:");
		ImGui::SameLine(150.0f);
		ent_display = sg.neighbours[SliceEngine::SceneGraph::UP];
		ImGui::Text(std::to_string((uint64_t)ent_display).c_str());

		ImGui::Text("Child:");
		ImGui::SameLine(150.0f);
		ent_display = sg.neighbours[SliceEngine::SceneGraph::DOWN];
		ImGui::Text(std::to_string((uint64_t)ent_display).c_str());

		ImGui::Text("Previous Sibling:");
		ImGui::SameLine(150.0f);
		ent_display = sg.neighbours[SliceEngine::SceneGraph::LEFT];
		ImGui::Text(std::to_string((uint64_t)ent_display).c_str());

		ImGui::Text("Next Sibling:");
		ImGui::SameLine(150.0f);
		ent_display = sg.neighbours[SliceEngine::SceneGraph::RIGHT];
		ImGui::Text(std::to_string((uint64_t)ent_display).c_str());

		if (entity == SliceEngine::FactoryInstance.GetRootEntity())
		{
			auto& registry = SliceEngine::Core::GetInstance()->GetRegistry();
			auto& sceneGraph = registry.get<SliceEngine::SceneGraph>(entity);

			if (sceneGraph.neighbours[SliceEngine::SceneGraph::UP] != entt::null)
			{
				if (sceneGraph.neighbours[SliceEngine::SceneGraph::UP] == SliceEngine::FactoryInstance.GetRootEntity())
				{
					ImGui::Text("Parent: Root Entity");
				}
				else
				{
					auto parentGO = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::UP]);
					ImGui::Text("Parent: %s", parentGO.GetName().c_str());
					ImGui::SameLine();
					ImGui::Text("ID: %d", sceneGraph.neighbours[SliceEngine::SceneGraph::UP]);
				}
			}
			else
			{
				ImGui::Text("Parent: --");
			}

			if (sceneGraph.neighbours[SliceEngine::SceneGraph::LEFT] != entt::null)
			{
				auto leftSibling = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::LEFT]);
				ImGui::Text("Left: %s", leftSibling.GetName().c_str());
				ImGui::Text("ID: %d", sceneGraph.neighbours[SliceEngine::SceneGraph::LEFT]);
			}
			else
			{
				ImGui::Text("Left: --");
			}

			if (sceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT] != entt::null)
			{
				auto rightSibling = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT]);
				ImGui::Text("Right: %s", rightSibling.GetName().c_str());
				ImGui::Text("ID: %d", sceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT]);
			}
			else
			{
				ImGui::Text("Right: --");
			}
			//Scene Graph Down is first child
			if (sceneGraph.neighbours[SliceEngine::SceneGraph::DOWN] != entt::null)
			{
				auto firstChild = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::DOWN]);
				ImGui::Text("First Child: %s", firstChild.GetName().c_str());
				ImGui::Text("ID: %d", sceneGraph.neighbours[SliceEngine::SceneGraph::DOWN]);
			}
			else
			{
				ImGui::Text("First Child: --");
			}
		}
		auto go = SliceEngine::FactoryInstance.GetGOByEntity(entity);
		if (go.HasComponent<SliceEngine::SceneGraph>())
		{
			auto& sceneGraph = go.GetComponent<SliceEngine::SceneGraph>();
			ImGui::Text("Entity: %s", go.GetName().c_str());
			ImGui::Text("Entity ID: %d", (uint32_t)entity);
			if (sceneGraph.neighbours[SliceEngine::SceneGraph::UP] != entt::null)
			{
				if (sceneGraph.neighbours[SliceEngine::SceneGraph::UP] == SliceEngine::FactoryInstance.GetRootEntity())
				{
					ImGui::Text("Parent: Root Entity");
				}
				else
				{
					auto parentGO = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::UP]);
					ImGui::Text("Parent: %s", parentGO.GetName().c_str());
					ImGui::SameLine();
					ImGui::Text("ID: %d", sceneGraph.neighbours[SliceEngine::SceneGraph::UP]);
				}
			}
			else
			{
				ImGui::Text("Parent: --");
			}

			if (sceneGraph.neighbours[SliceEngine::SceneGraph::LEFT] != entt::null)
			{
				auto leftSibling = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::LEFT]);
				ImGui::Text("Left: %s", leftSibling.GetName().c_str());
				ImGui::SameLine();
				ImGui::Text("ID: %d", sceneGraph.neighbours[SliceEngine::SceneGraph::LEFT]);
			}
			else
			{
				ImGui::Text("Left: --");
			}

			if (sceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT] != entt::null)
			{
				auto rightSibling = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT]);
				ImGui::Text("Right: %s", rightSibling.GetName().c_str());
				ImGui::SameLine();
				ImGui::Text("ID: %d", sceneGraph.neighbours[SliceEngine::SceneGraph::RIGHT]);
			}
			else
			{
				ImGui::Text("Right: --");
			}
			//Scene Graph Down is first child
			if (sceneGraph.neighbours[SliceEngine::SceneGraph::DOWN] != entt::null)
			{
				auto firstChild = SliceEngine::FactoryInstance.GetGOByEntity(sceneGraph.neighbours[SliceEngine::SceneGraph::DOWN]);
				ImGui::Text("First Child: %s", firstChild.GetName().c_str());
				ImGui::SameLine();
				ImGui::Text("ID: %d", sceneGraph.neighbours[SliceEngine::SceneGraph::DOWN]);
			}
			else
			{
				ImGui::Text("First Child: --");
			}
		}
	}*/
}