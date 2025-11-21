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
#include "ComponentPropertiesGUI.h"

#include <Resource/GUID.h>
#include <Scripting/ScriptSystem.h>
#include <Scripting/ScriptObject.h>
#include <Graphics/TransformHelper.h>
#include <Serializer/JSONSerializer.h>
#include <Systems/LayerManager.h>

namespace SliceEditor
{
	void InspectorWindow::Init()
	{
		mBaseFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed;
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
			if (ImGui::Button("Prefab Create"))
			{
				SliceEngine::JSONSerializer::SerializePrefab(static_cast<EntityNode*>(*selected_nodes.begin())->entity);
			}
			DisplayEntity(static_cast<EntityNode*>(*selected_nodes.begin())); 
			break;
		case SelectionType::MATERIAL:
			DisplayMaterial(static_cast<DirectoryNode*>(*selected_nodes.begin())); 
			break;
		}

		ImGui::End();
	}

	//void InspectorWindow::DisplayComponentHeader(std::string const component_name)

	
	void InspectorWindow::DisplayEntityData(entt::entity entity)
	{
		auto core = SliceEngine::Core::GetInstance();
		auto& slice = core->GetRegistry().get<SliceEngine::SliceEntity>(entity);
		auto original_name = SliceEngine::FactoryInstance.GetGOByEntity(entity).GetName();

		auto layer_manager = core->GetLayerManager();
		auto layer_name_list = layer_manager->GetLayerNameList();

		ImGui::Checkbox("##is_active", &slice.mActive);
		ImGui::SameLine();

		std::string editable_name = original_name;
		if (StringInput(mRegistry, "##name", editable_name, ImGui::GetContentRegionAvail().x))
		{
			if (editable_name != original_name)
				SliceEngine::FactoryInstance.GetGOByEntity(entity).SetName(editable_name);
		}

		// currently tags are unused
		int tag = 0;
		std::vector<std::string> tags {"unused"};

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
				DragIntInputHeader(mRegistry, "Width", "##width", rect.width, "X: %d", -2000, 2000);	//some random ass min max
			}
			else {
				DragIntInputHeader(mRegistry, "Left", "##left", rect.left, "X: %d", -2000, 2000);	//some random ass min max
				DragIntInputHeader(mRegistry, "Right", "##right", rect.right, "X: %d", -2000, 2000);	//some random ass min max
			}

			if (rect.vert_pivot != SliceEngine::RectTransform::VertPivot::STRETCH_V) {
				DragIntInputHeader(mRegistry, "Pos Y", "##posy", rect.pos_y, "X: %d", -2000, 2000);	//some random ass min max
				DragIntInputHeader(mRegistry, "Height", "##height", rect.height, "X: %d", -2000, 2000);	//some random ass min max
			}
			else {
				DragIntInputHeader(mRegistry, "Top", "##top", rect.top, "X: %d", -2000, 2000);	//some random ass min max
				DragIntInputHeader(mRegistry, "Bot", "##bot", rect.bot, "X: %d", -2000, 2000);	//some random ass min max
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

			//glm::vec3 rgb;
			//rgb.r = sprite.rgba.r; rgb.g = sprite.rgba.g; rgb.b = sprite.rgba.b;
			//DragColorInputHeader(mRegistry, "RGB", "##rgb", rgb);

			DragColor4InputHeader(mRegistry, "Color", "##uicolor", sprite.rgba);
			//sprite.rgba.r = rgb.r;sprite.rgba.g = rgb.g;sprite.rgba.b = rgb.b;
			BoolInputHeader(mRegistry, "Raycast Target", "##raycasttarget", sprite.raycast_target);

			ImGui::Text("Image");
			ImGui::SameLine(150.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			std::string texture_guid_string = std::to_string(sprite.textureHandle.GetGUID());
			std::string textureFileName;
			if (mRegistry.GetAssetManager().mGUIDtoFilename.find(sprite.textureHandle) != mRegistry.GetAssetManager().mGUIDtoFilename.end())
			{
				textureFileName = mRegistry.GetAssetManager().mGUIDtoFilename[sprite.textureHandle];
			}
			else //Its a default model
			{
				textureFileName = texture_guid_string;
			}
			ImGui::InputText("##Image", &textureFileName, ImGuiInputTextFlags_ReadOnly);
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Image"))
				{
					SliceEngine::GUID recievedPayload(*(SliceEngine::GUID*)payload->Data);
					sprite.textureHandle = recievedPayload;
					// update the handle after
				}
			}


			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayCanvas(entt::entity entity) {
		if (ImGui::TreeNodeEx("Canvas", mBaseFlags))
		{
			auto& canvas = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Canvas>(entity);

			DisplayComponentHeader<SliceEngine::Canvas>(entity, false);


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

	void InspectorWindow::DisplayAudioSource(entt::entity entity)
	{
		auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
		if (ImGui::TreeNodeEx("Audio Source", mBaseFlags))
		{
			if (!DisplayComponentHeader<SliceEngine::AudioSource>(entity))
			{
				reg.patch<SliceEngine::AudioSource>(entity, [&](auto& as)
					{
						std::function<void(SliceEngine::GUID)> func = [&](SliceEngine::GUID guid)
							{
								as.soundGUID = guid;
							};

						GUIDDragDropInputHeader(mRegistry, "Audio Clip", "##audio_clip", as.soundGUID, "Audio", func);

						SliderFloatInputHeader(mRegistry, "Volume", "##currVol", as.currentVolume, "%.1f", 0.0, 1.0);
						BoolInputHeader(mRegistry, "Is Mute", "##Mute", as.isMute);
						BoolInputHeader(mRegistry, "Is Loop", "##looping", as.isLoop);
						BoolInputHeader(mRegistry, "Is Paused", "##isPaused", as.isPaused);

						ImGui::Text("Play Preview");
						ImGui::SameLine(150);
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
						if (ImGui::Button(as.playPreview ? "Stop Preview" : "Play Preview"))
							as.playPreview = !as.playPreview;
					});
			}
			ImGui::TreePop();
		}

	}

	void InspectorWindow::DisplayMeshRenderer(entt::entity entity)
	{
		auto& rend = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Renderer>(entity);

		if (ImGui::TreeNodeEx("Renderer", mBaseFlags))
		{
			DisplayComponentHeader<SliceEngine::Renderer>(entity);

			HandleDragDropInputHeader<SliceEngine::SliceEngineTypes::Model>(mRegistry, "Mesh", "##rend_mesh", rend.modelHandle, "Model");
			HandleDragDropInputHeader<SliceEngine::SliceEngineTypes::Material>(mRegistry, "Material", "##rend_mat", rend.materialHandle, "Material");

			ImGui::TreePop();
		}

	}

	void InspectorWindow::DisplayCamera(entt::entity entity)
	{		
		if (ImGui::TreeNodeEx("Camera", mBaseFlags))
		{
			DisplayComponentHeader<SliceEngine::Camera>(entity);

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

						DragFloatInputHeader(mRegistry, "Linear Damping", "##linearDamp", rb.linearDamping);

						DragFloatInputHeader(mRegistry, "Angular Damping", "##angularDamp", rb.angularDamping);

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

					BoolInputHeader(mRegistry, "Is Trigger", "##isTrigger", col.isTrigger);

					glm::vec3 glm3 = JPHtoGLM(col.offSet);
					if (DragVec3InputHeader(mRegistry, "Offset", "##colOffset", glm3))
					{
						col.offSet = GLMtoJPH(glm3);
					}

					//static std::vector<std::string> colLayerNames{ "Non-Moving","Moving" };

					//ComboHeader<JPH::ObjectLayer>(mRegistry, "Collider Layer", "##colDetect", col.layer, colLayerNames);
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

			std::string script_name = script.scriptName;
			if (script_name.empty())
				script_name = "(Empty)";

			// Script Name

			ImGui::Text("Script Class: ");
			ImGui::SameLine(150.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputText("##script_name", &script_name, ImGuiInputTextFlags_ReadOnly);

			ImGui::Separator();

			if (script.scriptName.empty())
			{
				if (ImGui::Button("Add Script"))
				{
					ImGui::OpenPopup("script_list_popup");
				}

				if (ImGui::BeginPopupContextItem("script_list_popup"))
				{					
					auto& script_map = SliceEngine::gScriptSystem->mEntityClasses;

					std::vector<const char*> script_list{};

					static int list_index = 1;

					for (auto& [key, value] : script_map)
					{
						script_list.push_back(value->mClassName.c_str());
					}

					//std::string selected_script_class{};

					if (ImGui::BeginListBox("##script_list"))
					{
						for (size_t i = 0; i < script_list.size(); i++)
						{
							if (ImGui::Selectable(script_list[i]))
							{
								script.scriptName = "SliceEngine.";
								script.scriptName += script_list[i];
								ImGui::CloseCurrentPopup();
							}
						}

						ImGui::EndListBox();
					}

					ImGui::EndPopup();
				}
			}

			// Script Variables

			else
			{
				auto scriptRef = SliceEngine::gScriptSystem->GetScriptInstance(entity);

				if (scriptRef != nullptr)
				{
					const auto& fields = scriptRef->GetScriptClass()->mFields;
					for (const auto& it : fields)
					{
						if (it.second.mType == SliceEngine::ScriptFieldType::Float)
						{
							float data = scriptRef->GetFieldValue<float>(it.second.mName);
							//if (DragFloatInputHeader(mRegistry, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
							//{
							//	scriptRef->SetFieldValue(it.second.mName, data);
							//	SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
							//}
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
							if (BoolInputHeader(mRegistry, it.second.mName.c_str(), ("##" + it.second.mName).c_str(), data))
							{
								scriptRef->SetFieldValue(it.second.mName, data);
								SliceEngine::gScriptSystem->UpdateScriptComponent(entity);
							}
						}
						else if (it.second.mType == SliceEngine::ScriptFieldType::String)
						{
							std::string str = scriptRef->GetFieldValue<std::string>(it.second.mName);
							/*char buffer[128];
							std::strncpy(buffer, str.c_str(), sizeof(buffer) - 1);
							buffer[sizeof(str)] = '\0';*/

							if (StringInputHeader(mRegistry, it.second.mName.c_str(), ("##" + it.second.mName).c_str(),str))
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
			if(!DisplayComponentHeader<SliceEngine::Animator>(entity))
			{
				ImGui::Text("Controller: ");
				ImGui::SameLine(150.0f);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::Text("A00");

				ImGui::Text("Playing: ");
				ImGui::SameLine(150.f);
				ImGui::Checkbox("##anim_isPlaying", &animator.timeline.isPlaying);

				std::string anim_file{};
				ImGui::InputText("##anim", &anim_file, ImGuiInputTextFlags_ReadOnly);

				ImGui::Text("Loop: ");
				ImGui::SameLine(150.f);
				ImGui::Checkbox("##anim_isLoop", &animator.timeline.isLoop);

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

			if (ImGui::Selectable("Curve"))
			{
				value_type = SliceEngine::ParticleSystem::ValueType::CURVE;
			}

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
				// To do : Add Value Type Enum
				DragFloatInputHeader(mRegistry, "Start Speed", "##startSpeed", ps.speed, "%.1f", 0.0f, 100.f);

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
				// To do : Add Value Type Enum
				DragVec3InputHeader(mRegistry, "Start Size", "##startSize", ps.scale);

				// Start Rotation
				// To do : Add Value Type Enum
				BoolInputHeader(mRegistry, "3D Rotation", "##is3Drot", ps.isInitialRotation3D);
				
				if (ps.isInitialRotation3D)
				{
					switch (ps.initialRotationType)
					{
					case SliceEngine::ParticleSystem::ValueType::CONSTANT:
						DragRotationInputHeader(mRegistry, "Start Rotation", "##startRot", ps.rotation, ps.eulerHint);
						break;
					case SliceEngine::ParticleSystem::ValueType::TWO_CONSTANTS:
						DragRotationInputHeader(mRegistry, "Min Rotation", "##minStartRot", ps.minRandomRotation, ps.minEulerHint);
						DragRotationInputHeader(mRegistry, "Max Rotation", "##maxStartRot", ps.maxRandomRotation, ps.maxEulerHint);
						break;
					default:
						break;
					}
				}

				else
				{
					switch (ps.initialRotationType)
					{
					case SliceEngine::ParticleSystem::ValueType::CONSTANT:
						DragFloatInputHeader(mRegistry, "Start Rotation", "##startRot", ps.rotation.x, "&.1f", 0.f, 360.f);
						break;
					case SliceEngine::ParticleSystem::ValueType::TWO_CONSTANTS:
						DragFloatInputHeader(mRegistry, "Min Rotation", "##minLifetime", ps.minRandomRotation.x, "&.1f", 0.f, 360.f);
						DragFloatInputHeader(mRegistry, "Max Rotation", "##maxLifetime", ps.maxRandomRotation.x, "&.1f", 0.f, 360.f);
						break;
					default:
						break;
					}
				}

				ImGui::SameLine();
				ButtonValueTypePopup(ps.initialLifetimeType, "rotation");
				//ComboHeader< SliceEngine::ParticleSystem::ValueType>(mRegistry, "", "##lifetime_valuetype", ps.initialLifetimeType, value_type_names);

				// Start Color
				// To do : Add Value Type Enum

				switch (ps.colorValueType)
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

				ButtonValueTypePopup(ps.colorValueType, "color");

				// Gravity
				DragFloatInputHeader(mRegistry, "Gravity Modifier", "##gravityModifier", ps.gForce, "%.1f", 0.0f, 100.f);

				// Max Particles
				DragUInt64InputHeader(mRegistry, "Max Particles", "##maxParticles", ps.maxParticles, "", 0, 0);

				// Simulation Space
				BoolInputHeader(mRegistry, "Is Local Space", "##isLocalSpace", ps.isLocalSpace);

				// Destroy
				BoolInputHeader(mRegistry, "Destroy OnEnd", "##destroyOnEnd", ps.destroyOnExpire);
			}

			if (ImGui::CollapsingHeader("Shape"))
			{
				static std::vector<std::string> shapeTypes{ "Cone" };
				// Shape Type Enum
				ComboHeader<SliceEngine::ParticleSystem::ShapeType>(mRegistry, "Shape", "##shapeType", ps.shapeType, shapeTypes);

				switch (ps.shapeType)
				{
				case SliceEngine::ParticleSystem::ShapeType::CONE:
					DragFloatInputHeader(mRegistry, "Angle", "##coneAngle", ps.coneAngle, "%.1f", 0.0f, 90.0f);
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
				if (ImGui::BeginTable("Bursts", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
				{
					ImGui::TableSetupColumn("Time");
					ImGui::TableSetupColumn("Count");
					ImGui::TableSetupColumn("Cycle");
					ImGui::TableSetupColumn("Interval");
					ImGui::TableHeadersRow();
					for (auto& burst : ps.bursts)
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						DragFloatInputHeader(mRegistry, "##burst_time", "##burst_time", burst.triggerTime, "%.2f", 0.0f, 0.0f);
						ImGui::TableNextColumn();
						DragUInt64InputHeader(mRegistry, "##burst_count", "##burst_count", burst.numParticles, "", 0, 0);
						ImGui::TableNextColumn();
						DragUInt64InputHeader(mRegistry, "##burst_cycle", "##burst_cycle", burst.burstRepetitions, "", 0, 0);
						ImGui::TableNextColumn();
						DragFloatInputHeader(mRegistry, "##burst_interval", "##burst_interval", burst.burstPeriod, "", 0.0f, 0.0f);
					}
					ImGui::EndTable();
				}
			}

			if (ImGui::CollapsingHeader("Color Over Lifetime"))
			{

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
						if (StringInputHeader(mRegistry, "Texture", "##ps_texture", texture))
						{
							ps.textureGUID = SliceEngine::GUID::FromString(texture);
						}
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
					}
						break;
					default:
						break;
				}

				// LEGACY
				ImGui::SeparatorText("Legacy");
				std::string textureID = std::to_string(ps.textureID);
				if (StringInputHeader(mRegistry, "Texture", "##ps_texture_legacy", textureID))
				{
					ps.textureID = std::stoul(textureID);
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
		DisplayEntityData(node->entity);

		//Loop through registered components and display them if they exist on the selected entity

		for (auto&& [typeID, storage] : SliceEngine::Core::GetInstance()->GetRegistry().storage())
		{

		}

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
            
			AddComponentButton(node->entity);
		}

	}

	void InspectorWindow::DisplayMaterial(DirectoryNode* node)
	{
		std::string buffer{};
		static float f_buffer{};

		ImGui::BeginGroup();
		ImGui::Text("Material Name: ");
		ImGui::SameLine();
		ImGui::Text(node->fileName.c_str());
		ImGui::EndGroup();
		
		MaterialData mat;
		std::filesystem::path mat_path = node->fileName;
		//auto metapath = SliceEngine::Core::GetInstance()->GetResourceManager()->GetResourcePath(mat_path.stem().string());

		//if (metapath.has_value())
		mat.DeserializeAsset(node->path);

		std::string mat_file_name{};
		if (mRegistry.GetAssetManager().mGUIDtoFilename.find(mat.albedo) != mRegistry.GetAssetManager().mGUIDtoFilename.end())
		{
			mat_file_name = mRegistry.GetAssetManager().mGUIDtoFilename[mat.albedo];
		}
		else
		{
			mat_file_name = "GUID not in map";
		}

		ImGui::Text("Albedo");
		ImGui::SameLine(150.0f);
		ImGui::InputText("##albedo", &mat_file_name, ImGuiInputTextFlags_ReadOnly);

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Texture"))
			{
				SliceEngine::GUID recievedPayload(*(SliceEngine::GUID*)payload->Data);
				//auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
				mat.albedo =recievedPayload;
				mat.SerializeAsset(node->path);
				// update the handle after
			}
			ImGui::EndDragDropTarget();
		}

		if (DragFloatInputHeader(mRegistry, "Roughness", "##roughness", mat.roughness, "%.2f", 0.0f, 1.0f))
		{
			mat.SerializeAsset(node->path);
		}

		
		if (DragFloatInputHeader(mRegistry, "Metallic", "##metallic", mat.metallic, "%.2f", 0.0f, 1.0f))
		{
			mat.SerializeAsset(node->path);
		}
	}

	void InspectorWindow::DisplaySceneGraph(entt::entity entity)
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
	}
}