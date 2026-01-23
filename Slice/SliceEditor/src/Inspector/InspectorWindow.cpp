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

#include <Resource/GUID.h>
#include <Scripting/ScriptSystem.h>
#include <Scripting/ScriptObject.h>
#include <Graphics/TransformHelper.h>
#include <Serializer/JSONSerializer.h>
#include <Systems/LayerManager.h>
#include <WindowManager/WindowManager.h>
#include <Systems/PrefabSystem.h>

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

		auto type = selected_nodes.begin().operator*()->type;

		switch (type)
		{
		case SelectionType::ENTITY:
			DisplayEntity(static_cast<EntityNode*>(*selected_nodes.begin())); 
			break;
		case SelectionType::MATERIAL:
			DisplayMaterial(static_cast<DirectoryNode*>(*selected_nodes.begin())); 
			break;
		case SelectionType::PREFAB_ENTITY:
			DisplayPrefab(static_cast<EntityNode*>(*selected_nodes.begin()));
			break;
		case SelectionType::STATE:
			DisplayState(static_cast<StateNode*>(*selected_nodes.begin()));
			break;
		case SelectionType::TRANSITION:
			DisplayTransition(static_cast<TransitionLinkNode*>(*selected_nodes.begin()));
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

		
		auto layer_manager = core->GetLayerManager();
		auto layer_name_list = layer_manager->GetLayerNameList();

		if (BoolInput(mRegistry, "##isActive", isActive))
		{
			if (isActive)
			{
				SliceEngine::Core::GetInstance()->GetRegistry().remove<SliceEngine::InactiveEntity>(entity);
				slice.mActive = true;
			}
			else
			{
				slice.mActive = false;
				SliceEngine::Core::GetInstance()->GetRegistry().emplace<SliceEngine::InactiveEntity>(entity);
			}
		}
		ImGui::SameLine();

		std::string editable_name = original_name;
		std::string editable_tag = original_tag;

		std::function<void(std::string name)> func = [&](std::string name)
			{
				SliceEngine::FactoryInstance.GetGOByEntity(entity).SetName(name);
			};
		
		StringInputHeader(mRegistry, "Name: ", "##name", editable_name, ImGui::GetContentRegionAvail().x, func);

		ImGui::Text("Entity ID: %d", entity);

		//Temp solution
		if (SliceEngine::Core::GetInstance()->GetRegistry().any_of<SliceEngine::Prefab>(entity))
		{
			auto& prefabComponent = SliceEngine::FactoryInstance.GetGOByEntity(entity).GetComponent<SliceEngine::Prefab>();
			ImGui::Text("Is Prefab");
			if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
			{
				if(ImGui::BeginTooltip())
				{
					ImGui::Text("Prefab GUID: ");
					ImGui::SameLine(150.f);
					std::string prefabGUID = prefabComponent.prefabGUID.toString();
					std::string prefabHandle = prefabComponent.prefabHandle.getGUID().toString();
					ImGui::Text(prefabGUID.c_str());
					ImGui::Text("Prefab Handle GUID: ");
					ImGui::SameLine(150.f);
					ImGui::Text(prefabHandle.c_str());
					ImGui::EndTooltip();
				}
			}
		}

		std::function<void(std::string name)> funcTag = [&](std::string name)
			{
				SliceEngine::FactoryInstance.GetGOByEntity(entity).SetTag(name);
			};

		StringInputHeader(mRegistry, "Tag: ", "##tag", editable_tag, ImGui::GetContentRegionAvail().x, funcTag);
		//Game Object Tags:
		/*if (StringInputHeader(mRegistry, "Tag: ", "##entityTag", slice.mTag))
		{
			SliceEngine::FactoryInstance.GetGOByEntity(entity).SetTag(slice.mTag);
		}*/

		// currently tags are unused
		/*int tag = 0;
		std::vector<std::string> tags {"unused"};*/

		//ImGui::BeginDisabled();
		//ComboHeader(mRegistry, "Tags", "##tags", tag, tags);
		//ImGui::EndDisabled();
		//ImGui::SameLine();

		ComboHeader(mRegistry, "Layer", "##layer", slice.mLayer, layer_name_list);
		ImGui::Separator();

	}

	void InspectorWindow::DisplayTransform(entt::entity entity)
	{
		if (ImGui::TreeNodeEx("Transform", mBaseFlags))
		{
			auto& tr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(entity);

			DisplayComponentHeader<SliceEngine::Transform>(entity, false);
			DragVec3InputHeader(mRegistry, "Position", "##t", tr.position);			
			DragRotationInputHeader(mRegistry, "Rotation", "##r", tr.rotation, tr.eulerAnglesHint);
			DragVec3InputHeader(mRegistry, "Scale", "##s", tr.scale);

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayRectTransform(entt::entity entity)
	{
		if (ImGui::TreeNodeEx("RectTransform", mBaseFlags))
		{
			auto& rect = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::RectTransform>(entity);

			DisplayComponentHeader<SliceEngine::RectTransform>(entity, false);

			static std::vector<std::string> hori_enums{ "Left", "Center", "Right", "Stretch" };
			static std::vector<std::string> vert_enums{ "Top", "Middle", "Bottom", "Stretch" };
			ComboHeader<SliceEngine::RectTransform::HoriPivot>(mRegistry, "Hori Pivot", "##horipivot", rect.hori_pivot, hori_enums);
			ComboHeader<SliceEngine::RectTransform::VertPivot>(mRegistry, "Vert Pivot", "##vertpivot", rect.vert_pivot, vert_enums);

			if (rect.hori_pivot != SliceEngine::RectTransform::HoriPivot::STRETCH_H) {
				DragIntInputHeader(mRegistry, "Pos X", "##posx", rect.pos_x, "X: %d", -2000, 2000);	//some random ass min max
				DragIntInputHeader(mRegistry, "Width", "##width", rect.width, "W: %d", -2000, 2000);	//some random ass min max
			}
			else {
				DragIntInputHeader(mRegistry, "Left", "##left", rect.left, "L: %d", -2000, 2000);	//some random ass min max
				DragIntInputHeader(mRegistry, "Right", "##right", rect.right, "R: %d", -2000, 2000);	//some random ass min max
			}

			if (rect.vert_pivot != SliceEngine::RectTransform::VertPivot::STRETCH_V) {
				DragIntInputHeader(mRegistry, "Pos Y", "##posy", rect.pos_y, "Y: %d", -2000, 2000);	//some random ass min max
				DragIntInputHeader(mRegistry, "Height", "##height", rect.height, "H: %d", -2000, 2000);	//some random ass min max
			}
			else {
				DragIntInputHeader(mRegistry, "Top", "##top", rect.top, "T: %d", -2000, 2000);	//some random ass min max
				DragIntInputHeader(mRegistry, "Bot", "##bot", rect.bot, "B: %d", -2000, 2000);	//some random ass min max
			}
			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplaySpriteRenderer(entt::entity entity)
	{
		if (ImGui::TreeNodeEx("SpriteRenderer", mBaseFlags))
		{
			auto& sprite = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SpriteRenderer>(entity);

			DisplayComponentHeader<SliceEngine::SpriteRenderer>(entity, false);

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

	void InspectorWindow::DisplayFontRenderer(entt::entity entity)
	{
		if (ImGui::TreeNodeEx("FontRenderer", mBaseFlags))
		{
			auto& font = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::FontRenderer>(entity);

			DisplayComponentHeader<SliceEngine::FontRenderer>(entity, false);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", font.componentEnabled);

			/*
			* will need to update this token_updated from scripts too
			*/

			if (StringInput(mRegistry, "##font_text", font.text, 150.f)) {
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

			DisplayComponentHeader<SliceEngine::Canvas>(entity, false);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", canvas.componentEnabled);

			static std::vector<std::string> canvas_types{ "Overlay" };
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

			DisplayComponentHeader<SliceEngine::Button>(entity, false);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", button.componentEnabled);

			static std::vector<std::string> transitions{ "Color, Sprite" };
			ComboHeader<SliceEngine::Button::Transition>(mRegistry, "Button Transitions", "##btntransitions", button.transition, transitions);

			switch (button.transition) {
			case SliceEngine::Button::Color:
				DragColor4InputHeader(mRegistry, "Normal", "##btncolor1", button.color_transitions[SliceEngine::Button::Normal]);
				DragColor4InputHeader(mRegistry, "Highlighted", "##btncolor2", button.color_transitions[SliceEngine::Button::Highlighted]);
				DragColor4InputHeader(mRegistry, "Pressed", "##btncolor3", button.color_transitions[SliceEngine::Button::Pressed]);
				break;
			case SliceEngine::Button::Sprite:	//i didnt test this
			{
				const char* state_names[] = { "Normal", "Highlighted", "Pressed" };
				for (int i = 0; i < 3; ++i) {
					auto& btn_sprites = button.sprite_transitions;
					ImGui::Text(state_names[i]);
					ImGui::SameLine(150.0f);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

					auto& texture_guid = btn_sprites[SliceEngine::Button::Normal];
					std::string texture_guid_string = std::to_string(texture_guid.GetGUID());
					std::string textureFileName;
					if (mRegistry.GetAssetManager().mGUIDtoFilename.find(texture_guid) != mRegistry.GetAssetManager().mGUIDtoFilename.end())
					{
						textureFileName = mRegistry.GetAssetManager().mGUIDtoFilename[texture_guid];
					}
					else //Its a default texture
					{
						textureFileName = texture_guid_string;
					}
					ImGui::InputText(state_names[i], &textureFileName, ImGuiInputTextFlags_ReadOnly);
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(state_names[i]))
						{
							SliceEngine::GUID recievedPayload(*(SliceEngine::GUID*)payload->Data);
							texture_guid = recievedPayload;
							// update the handle after
						}
					}
				}
			}
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

			static std::vector<std::string> axis_enums{ "X Axis", "Y Axis" };
			static std::vector<std::string> direction_enums{ "Positive", "Negative" };
			ComboHeader<SliceEngine::Slider::Axis>(mRegistry, "Axis", "##slideraxis", slider.axis, axis_enums);
			ComboHeader<SliceEngine::Slider::Direction>(mRegistry, "Direction", "##sliderdirection", slider.direction, direction_enums);

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

					DragIntInputHeader(mRegistry, "Priority", "##priority", as.priority, "%d", 0, 256);
					BoolInputHeader(mRegistry, "Is Mute", "##Mute", as.isMute);
					BoolInputHeader(mRegistry, "Play On Awake", "##playOnAwake", as.playOnAwake);
					BoolInputHeader(mRegistry, "Is Loop", "##looping", as.isLoop);
					BoolInputHeader(mRegistry, "Is Paused", "##isPaused", as.isPaused);
					SliderFloatInputHeader(mRegistry, "Volume", "##currVol", as.currentVolume, "%.1f", 0.0, 1.0);
					SliderFloatInputHeader(mRegistry, "Pitch", "##pitch", as.pitch, "%.1f", -3.0, 3.0);
					SliderFloatInputHeader(mRegistry, "Stereo Pan", "##stereoPan", as.stereoPan, "%.1f", -1.0, 1.0);
					SliderFloatInputHeader(mRegistry, "Spatial Blend", "##spatialBlend", as.spatialBlend, "%.1f", 0.0, 1.0);
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
		auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
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

		if (ImGui::TreeNodeEx("Renderer", mBaseFlags))
		{
			DisplayComponentHeader<SliceEngine::Renderer>(entity);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", rend.componentEnabled);

			HandleDragDropInputHeader<SliceEngine::SliceEngineTypes::Model>(mRegistry, "Mesh", "##rend_mesh", rend.modelHandle, "Model");
			HandleDragDropInputHeader<SliceEngine::SliceEngineTypes::Material>(mRegistry, "Material", "##rend_mat", rend.materialHandle, "Material", nullptr);

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

			DragFloatInputHeader(mRegistry, "FOV", "##cam_fov", cam.pov, "%.1f", 1.0f, FLT_MAX);
			ImGui::Text("Clipping Planes");
			DragFloatInputHeader(mRegistry, "Near", "##cam_near", cam.near, "%.1f", 0.1f, FLT_MAX);
			DragFloatInputHeader(mRegistry, "Far", "##cam_far", cam.far, "%.1f", 1.f, FLT_MAX);

			ImGui::SeparatorText("Post-Processing FX");

			using RenderTag = SliceEngine::RENDER_TAG;

			bool isBloom = cam.postRenderToggles & RenderTag::RENDER_BLOOM;
			bool isFog = cam.postRenderToggles & RenderTag::RENDER_FOG;
			bool isVignette = cam.postRenderToggles & RenderTag::RENDER_VIGNETTE;

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
				DragFloatInputHeader(mRegistry, "Exposure", "##cam_bloom_exposure", cam.exposure, "%.1f", 0.1f, 50.0f);
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
				DragFloatInputHeader(mRegistry, "Fog Intensity", "##cam_fog_intensity", cam.fogIntensity, "%.1f", 0.0f, FLT_MAX);
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
			}, colliderData.shapeData);

		if (ImGui::TreeNodeEx(colliderName.c_str(), mBaseFlags))
		{
			if(!DisplayComponentHeader<SliceEngine::ColliderShape>(entity))
			{
				reg.patch<SliceEngine::ColliderShape>(entity, [&](SliceEngine::ColliderShape& col)
				{
					BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", col.componentEnabled);

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
				});
			}
			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayNavAgent(entt::entity entity)
	{
		auto& agent = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::NavAgent>(entity);

		if (ImGui::TreeNodeEx("Nav Agent", mBaseFlags))
		{
			DisplayComponentHeader<SliceEngine::NavAgent>(entity);

			BoolInputHeader(mRegistry, "Is Enabled", "##isEnabled", agent.componentEnabled);

			DragFloatInputHeader(mRegistry, "Speed", "#agent_speed", agent.speed, "%.1f");

			ImGui::TreePop();
		}

	}

	void InspectorWindow::DisplaySliceScript(entt::entity entity)
	{
		auto& script = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(entity);

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
				selectedIndex = scriptList.size() - 1;
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
					selectedIndex = std::distance(scriptList.begin(), it);
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

								std::function<void(const char*, std::string, std::vector<float>, float, int)> func = [sp = scriptRef](const char* funcToExec, std::string name, std::vector<float> list, float val, int index)
									{
										if (funcToExec == "Edit")
										{
											sp->SetListField(name, list);
										}
										else if (funcToExec == "Add")
										{
											sp->AddListFieldValue(name, val);
										}
										else if (funcToExec == "Remove")
										{
											sp->RemoveListField(name, index);
										}
									};

								if (FloatListScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetListField(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}

							else if (it.second.mType == SliceEngine::ScriptFieldType::String)
							{
								auto data = scriptRef->GetListFieldValue<std::string>(it.second.mName);

								std::function<void(const char*, std::string, std::vector<std::string>, std::string, int)> func = [sp = scriptRef](const char* funcToExec, std::string name, std::vector<std::string> list, std::string val, int index)
									{
										if (funcToExec == "Edit")
										{
											sp->SetListField(name, list);
										}
										else if (funcToExec == "Add")
										{
											sp->AddListFieldValue(name, val);
										}
										else if (funcToExec == "Remove")
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
										if (funcToExec == "Edit")
										{
											sp->SetListField(name, list);
										}
										else if (funcToExec == "Add")
										{
											sp->AddListFieldValue(name, val);
										}
										else if (funcToExec == "Remove")
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
										if (funcToExec == "Edit")
										{
											sp->SetListField(name, list);
										}
										else if (funcToExec == "Add")
										{
											sp->AddListFieldValue(name, val);
										}
										else if (funcToExec == "Remove")
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

								std::function<void(const char*, std::string, std::vector<SliceEngine::GameObject>, SliceEngine::GameObject, int)> func = [sp = scriptRef](const char* funcToExec, std::string name, std::vector<SliceEngine::GameObject> list, SliceEngine::GameObject val, int index)
									{
										if (funcToExec == "Edit")
										{
											sp->SetListField(name, list);
										}
										else if (funcToExec == "Add")
										{
											sp->AddListFieldValue(name, val);
										}
										else if (funcToExec == "Remove")
										{
											sp->RemoveListField(name, index);
										}
									};

								if (GameObjectListScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetListField(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
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

								if (DragFloatInputScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetFieldValue(it.second.mName, data);
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
								if (BoolInputScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetFieldValue(it.second.mName, data);
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

								if (StringInputScriptHeader(mRegistry,func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), str))
								{
									scriptRef->SetFieldValue<std::string>(it.second.mName, str);
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

								if (DragIntInputScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetFieldValue(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
							}
							else if (it.second.mType == SliceEngine::ScriptFieldType::Vector3)
							{
								glm::vec3 data = scriptRef->GetFieldValue<glm::vec3>(it.second.mName);
								std::function<void(std::string, glm::vec3)> func = [sp = scriptRef](std::string name, glm::vec3 val)
									{
										sp->SetFieldValue(name, val);
									};

								if (DragVec3InputScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetFieldValue(it.second.mName, data);
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

								if (GameObjectInputScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetFieldValue(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}
								/*if (DragVec3InputScriptHeader(mRegistry, func, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
								{
									scriptRef->SetFieldValue(it.second.mName, data);
									SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
								}*/
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
		//if (!animator.IsValid())
		//{
		//	if (ImGui::TreeNodeEx("Animator", mBaseFlags))
		//	{
		//		ImGui::Text("Animator is not valid \n :deadge_1");
		//		ImGui::TreePop();
		//	}
		//}
		//else
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
					}
				}
				//Controller has been set, should be changable
				else
				{
					if(HandleDragDropInputHeader(mRegistry, "Controller: ", "##controller", animator.Handle_stateMachine, "Controller")) //For changing
					{
						animator.stateMachine.EFSM.stateMap.clear();
						animator.stateMachine.EFSM = *animator.Handle_stateMachine.get();
						animator.stateMachine.InitState(animator.curr_anim_pkg);
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

						ImGui::Text("Cuurent Animation: %d", animator.stateMachine.EFSM.currState->curr_anim_idx);

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

			if (ImGui::CollapsingHeader("Initialization", ImGuiTreeNodeFlags_DefaultOpen))
			{
				// Duration
				DragFloatInputHeader(mRegistry, "Duration", "##duration", ps.duration, "%.1f", 0.0f, 100.f);
				
				// Looping
				BoolInputHeader(mRegistry, "Looping", "##looping", ps.isRepeating);
				
				// Start Speed
				switch (ps.speedValueType)
				{
				case SliceEngine::ParticleSystem::ValueType::CONSTANT:
					DragFloatInputHeader(mRegistry, "Start Speed", "##startSpeed", ps.speed, "%.1f", 0.0f);
					break;

				case SliceEngine::ParticleSystem::ValueType::TWO_CONSTANTS:
					DragFloatInputHeader(mRegistry, "Min Start Speed", "##minStartSpeed", ps.minRandomSpeed, "%.1f", 0.0f);
					DragFloatInputHeader(mRegistry, "Max Start Speed", "##maxStartSpeed", ps.maxRandomSpeed, "%.1f", 0.0f);
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
					DragFloatInputHeader(mRegistry, "Start Lifetime", "##startLifetime", ps.lifetime, "%.1f", 0.0f, 0.f);
					break;
				case SliceEngine::ParticleSystem::ValueType::TWO_CONSTANTS:
					DragFloatInputHeader(mRegistry, "Min Lifetime", "##minLifetime", ps.minParticleLifetime, "%.1f", 0.0f, 0.f);
					DragFloatInputHeader(mRegistry, "Max Lifetime", "##maxLifetime", ps.maxParticleLifetime, "%.1f", 0.0f, 0.f);
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
					DragFloatInputHeader(mRegistry, "Rotation", "##r", ps.rotation, "%.1f", 0.0f, 360.f);
					break;
				case SliceEngine::ParticleSystem::ValueType::TWO_CONSTANTS:
					DragFloatInputHeader(mRegistry, "Min Rotation", "##minLifetime", ps.minRandomRotation, "&.1f", 0.f, 360.f);
					DragFloatInputHeader(mRegistry, "Max Rotation", "##maxLifetime", ps.maxRandomRotation, "&.1f", 0.f, 360.f);
					break;
				default:
					break;
				}
				ImGui::SameLine();
				ButtonValueTypePopup(ps.initialLifetimeType, "rotation");

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
				DragFloatInputHeader(mRegistry, "Gravity Modifier", "##gravityModifier", ps.gForce, "%.1f", 0.0f, 100.f);

				// Max Particles
				DragUInt64InputHeader(mRegistry, "Max Particles", "##maxParticles", ps.maxParticles, "%llu", 0, 5000);

				// Simulation Space
				BoolInputHeader(mRegistry, "Is Local Space", "##isLocalSpace", ps.isLocalSpace);

				// Destroy
				BoolInputHeader(mRegistry, "Destroy OnEnd", "##destroyOnEnd", ps.destroyOnExpire);
			}

			if (ImGui::CollapsingHeader("Shape"))
			{
				static std::vector<std::string> shapeTypes{ "Sphere", "Cone"};
				// Shape Type Enum
				ComboHeader<SliceEngine::ParticleSystem::ShapeType>(mRegistry, "Shape", "##shapeType", ps.shapeType, shapeTypes);

				switch (ps.shapeType)
				{
				case SliceEngine::ParticleSystem::ShapeType::SPHERE:
					DragFloatInputHeader(mRegistry, "Sphere Radius", "##sphereRadius", ps.sphereRadius, "%.1f", 0.1f, std::numeric_limits<float>::max());
					break;
				case SliceEngine::ParticleSystem::ShapeType::CONE:
					DragFloatInputHeader(mRegistry, "Cone Arc Angle", "##coneArcAngle", ps.coneArc, "%.1f", 0.0f, 90.0f);
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
						DragFloatInput(mRegistry, triggerTimeID.c_str(), burst.triggerTime, "%.2f", 0.0f, FLT_MAX);
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

			if (ImGui::CollapsingHeader("Color Over Lifetime"))
			{
				// Colour Over Lifetime
				BoolInputHeader(mRegistry, "Colour Over Lifetime", "##colourOverLifetime", ps.colourOverLifetime);
				if (ps.colourOverLifetime)
				{
					DragColor4InputHeader(mRegistry, "Colour Over Lifetime End", "##colourOverLifetimeEnd", ps.colourOverLifetimeEnd);
				}
			}

			if (ImGui::CollapsingHeader("Renderer"))
			{
				static std::vector<std::string> render_mode_names = { "Billboard", "Mesh" };
				ComboHeader(mRegistry, "Render Mode", "##ps_render_mode", ps.renderMode, render_mode_names);

				switch (ps.renderMode)
				{
					case SliceEngine::ParticleSystem::RenderMode::BILLBOARD:
					{
						std::string texture = ps.textureGUID.toString();

						// to do : change this to asset drag and drop gui header
						SliceEngine::GUID tex_guid = ps.textureGUID;
						GUIDDragDropInputHeader(mRegistry, "Image", "##spriteimage", tex_guid, "Texture");
						ps.textureGUID = tex_guid;
					}
						break;
					case SliceEngine::ParticleSystem::RenderMode::MESH:
					{
						std::string mesh = ps.textureGUID.toString();
						std::string material = ps.textureGUID.toString();

						// to do : change this to asset drag and drop gui header
						if (StringInputHeader(mRegistry, "Mesh", "##ps_mesh", mesh))
						{
							ps.textureGUID = SliceEngine::GUID::FromString(mesh);
						}

						// to do : change this to asset drag and drop gui header
						if (StringInputHeader(mRegistry, "Material", "##ps_material", material))
						{
							ps.textureGUID = SliceEngine::GUID::FromString(material);
						}

						SliceEngine::GUID tex_guid = ps.textureGUID;
						GUIDDragDropInputHeader(mRegistry, "Image", "##spriteimage", tex_guid, "Texture");
						ps.textureGUID = tex_guid;
					}
						break;
					default:
						break;
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

			//DragVec3InputHeader(mRegistry, "Colour", "##c", light.color);
			DragColor3InputHeader(mRegistry, "Colour", "##lightColor", light.color);

			DragFloatInputHeader(mRegistry, "Intensity", "##intensity", light.intensity, "%.2f", 0.0f, 10.f);

			static std::vector<std::string> lightTypes { "Directional Light", "Point Light", "Spot Light" };

			ComboHeader<SliceEngine::Light::LightType>(mRegistry, "Light Type", "##lightType", light.type, lightTypes);

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

			if (!selectedGO.HasComponent<SliceEngine::NavAgent>())
			{
				if (ImGui::Selectable("Add Nav Agent"))
				{
					reg.emplace<SliceEngine::NavAgent>(entity);
				}
			}

			if(!selectedGO.HasComponent<SliceEngine::ColliderShape>())
			{
				if (ImGui::Selectable("Add Box Collider"))
				{
					auto& col = reg.emplace<SliceEngine::ColliderShape>(entity);
					col.shapeData = SliceEngine::ColliderShape::BoxData{};
				}

				if (ImGui::Selectable("Add Sphere Collider"))
				{
					auto& col = reg.emplace<SliceEngine::ColliderShape>(entity);
					col.shapeData = SliceEngine::ColliderShape::SphereData{};
				}

				if (ImGui::Selectable("Add Capsule Collider"))
				{
					auto& col = reg.emplace<SliceEngine::ColliderShape>(entity);
					col.shapeData = SliceEngine::ColliderShape::CapsuleData{};
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

			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::NavAgent>(entity))
			{
				DisplayNavAgent(node->entity);
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
		if (GUIDDragDropInputHeader(mRegistry, "Custom Shader:", "##customshdr", mat.shader, "Custom Shader"))
		{
			mat.SerializeAsset(node->fullPath);
		}

		if (GUIDDragDropInputHeader(mRegistry, "Albedo", "##albedo", mat.albedo, "Texture"))
		{
			mat.SerializeAsset(node->fullPath);
		}		
		
		if (DragColor4InputHeader(mRegistry, "Material Colour", "##mat_color", mat.color))
		{
			mat.SerializeAsset(node->fullPath);
		}
		auto shdr = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::CustomShader>(mat.shader);
		int floatCnt{}, intCnt{}, uintCnt{}, boolCnt{};
		for (auto& i : shdr.get()->dataIn)
		{
			switch (i.dataType)
			{
			case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::BOOL:
			{
				std::string s = "##Material_Bool_" + i.name;
				bool tempBool{};
				if (BoolInputHeader(mRegistry, i.name.c_str(), s.c_str(), tempBool))
				{
					mat.boolDat[boolCnt] = tempBool;
					mat.SerializeAsset(node->fullPath);
				}
				++boolCnt;
				break;
			}
			case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::UINT:
			{
				std::string s = "##Material_Uint_" + i.name;
				if(DragUInt32InputHeader(mRegistry, i.name.c_str(), s.c_str(), mat.uintDat[uintCnt], "%.u", 0, UINT_MAX))
					mat.SerializeAsset(node->fullPath);
				++uintCnt;
				break;
			}
			case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::INT:
			{
				std::string s = "##Material_Int_" + i.name;
				if(DragIntInputHeader(mRegistry, i.name.c_str(), s.c_str(), mat.intDat[intCnt], "%.d", -INT_MAX, INT_MAX))
					mat.SerializeAsset(node->fullPath);
				++intCnt;
				break;
			}
			case SliceEngine::SliceEngineTypes::CustomShader::SP_TYPE::FLOAT:
			{
				std::string s = "##Material_Float_" + i.name;
				if(DragFloatInputHeader(mRegistry, i.name.c_str(), s.c_str(), mat.floatDat[floatCnt], "%.2f", 0.0f, FLT_MAX, 0.01f))
					mat.SerializeAsset(node->fullPath);
				++floatCnt;
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

		ImGui::SeparatorText("State");

		if (!anim_data)
			return;

		auto& state = anim_data->mStateMachineAsset->stateMap.at(node->name);

		StringInputHeader(mRegistry, "Name", "##state_name", state.stateName);
	

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
		auto anim_data = mRegistry.GetManager<SessionManager>("SessionManager")->GetAnimatorData();

		ImGui::SeparatorText("Transition");

		if (!anim_data)
			return;
		
		auto stateOpt = anim_data->GetState(node->source_id);
		auto transitionOpt = anim_data->GetTransition(stateOpt.value(), node->id);

		if (!transitionOpt.has_value())
			return;

		auto& transition = transitionOpt.value().get();

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

	void InspectorWindow::DisplaySceneGraph(entt::entity entity)
	{
		/*auto& sg = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(entity);

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
		ImGui::Text(std::to_string((uint64_t)ent_display).c_str());*/

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
	}
}