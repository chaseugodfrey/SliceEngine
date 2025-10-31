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
#include "ComponentPropertiesGUI.h"
#include "Core/Registry.h"
#include "Selection/SelectionManager.h"
#include "../../SliceEngine/src/Scripting/ScriptSystem.h"
#include <Graphics/TransformHelper.h>

namespace SliceEditor
{
	void InspectorWindow::Init()
	{
		mBaseFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowItemOverlap;
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
		case SelectionNode::SelectionType::ENTITY:
			DisplayEntity(static_cast<EntityNode*>(*selected_nodes.begin())); break;
		case SelectionNode::SelectionType::FILE:
			DisplayMaterial(static_cast<DirectoryNode*>(*selected_nodes.begin())); break;
		}

		ImGui::End();
	}

	//void InspectorWindow::DisplayComponentHeader(std::string const component_name)

	
	void InspectorWindow::DisplayEntityData(entt::entity entity)
	{
		//static bool is_active = false;
		//ImGui::Checkbox("##is_active", &is_active);
		//ImGui::SameLine();

		auto original_name = SliceEngine::FactoryInstance.GetGOByEntity(entity).GetName();
		std::string editable_name = original_name;
		if (ImGui::InputText("##name", &editable_name))
		{
			if (editable_name != original_name)
				SliceEngine::FactoryInstance.GetGOByEntity(entity).SetName(editable_name);
		}
		
		ImGui::SameLine();
		ImGui::Text(std::to_string((uint64_t)entity).c_str());
		ImGui::Separator();

	}

	//void InspectorWindow::DisplayComponentHeader(bool closeable)
	//{
	//}

	void InspectorWindow::DisplayTransform(entt::entity entity)
	{
		if (ImGui::TreeNodeEx("Transform", mBaseFlags))
		{
			auto& tr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(entity);

			DisplayComponentHeader<SliceEngine::Transform>(entity, false);
			DragVec3InputHeader(mRegistry, "Position", "##t", tr.position);
			glm::vec3 euler = SliceEngine::QuatToVec3(tr.rotation);
			if (DragVec3InputHeader(mRegistry, "Rotation", "##r", euler))
			{
				tr.rotation = SliceEngine::Vec3ToQuat(euler);
			}
			DragVec3InputHeader(mRegistry, "Scale", "##s", tr.scale);

			// for testing purposes
			ImGui::BeginDisabled();
			ImGui::Text("Parent: ");
			auto& scene_graph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(entity);
			auto parent_entity = scene_graph.neighbours[SliceEngine::SceneGraph::UP];
			std::string name{ "--" };

			if (parent_entity != SliceEngine::FactoryInstance.GetRootEntity())
			{
				auto go = SliceEngine::FactoryInstance.GetGOByEntity(parent_entity);
				name = go.GetName();
			}

			ImGui::Text(name.c_str());
			ImGui::EndDisabled();

			ImGui::TreePop();
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

	void InspectorWindow::DisplayAudioSource(entt::entity entity)
	{
		auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();

		reg.patch<SliceEngine::AudioSource>(entity, [&](auto& as) {
			if (ImGui::TreeNodeEx("AudioSource"))
			{
				DisplayComponentHeader<SliceEngine::AudioSource>(entity);

				/*ImGui::Text("Audio Clip");
				ImGui::SameLine(150);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::InputText("##audio_file", &as.soundGUID, ImGuiInputTextFlags_ReadOnly);*/

				ImGui::Text("Volume");
				ImGui::SameLine(150);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				float volume = as.currentVolume;
				if (ImGui::SliderFloat("##vol", &volume, 0.0f, 1.0f))
					as.currentVolume = volume; // mark dirty via patch

				ImGui::Text("Is Loop");
				ImGui::SameLine(150);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				bool loop = as.isLoop;
				if (ImGui::Checkbox("##isloop", &loop))
					as.isLoop = loop;

				ImGui::Text("Is 3D");
				ImGui::SameLine(150);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				bool is3D = as.is3D;
				if (ImGui::Checkbox("##is3d", &is3D))
					as.is3D = is3D;

				ImGui::Text("Is Paused");
				ImGui::SameLine(150);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				bool paused = as.isPaused;
				if (ImGui::Checkbox("##ispaused", &paused))
					as.isPaused = paused;

				ImGui::Text("Play Preview");
				ImGui::SameLine(150);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				if (ImGui::Button(as.playPreview ? "Stop Preview" : "Play Preview"))
					as.playPreview = !as.playPreview;

				ImGui::TreePop();
			}
			});

	}

	void InspectorWindow::DisplayMeshRenderer(entt::entity entity)
	{
		auto& rend = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Renderer>(entity);

		if (ImGui::TreeNodeEx("Renderer", mBaseFlags))
		{
			DisplayComponentHeader<SliceEngine::Renderer>(entity);

			ImGui::Text("Mesh");
			ImGui::SameLine(150.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			std::string model_guid_string = std::to_string(rend.model.GetGUID());
			if (ImGui::InputText("##mesh", &model_guid_string))
			{
				rend.model = SliceEngine::GUID(std::stoll(model_guid_string));
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Model"))
				{
					SliceEngine::GUID recievedPayload(*(SliceEngine::GUID*)payload->Data);
					rend.model = recievedPayload;
				}
			}

			ImGui::Text("Material");
			ImGui::SameLine(150.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			std::string material_guid_string = std::to_string(rend.material.GetGUID());
			if (ImGui::InputText("##material", &material_guid_string, ImGuiInputTextFlags_ReadOnly))
			{
				rend.material = SliceEngine::GUID(std::stoll(material_guid_string));
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Material"))
				{
						SliceEngine::GUID recievedPayload(*(SliceEngine::GUID*)payload->Data);
						rend.material = recievedPayload;
				}
			}

			ImGui::TreePop();
		}

	}

	void InspectorWindow::DisplayRigidbody(entt::entity entity)
	{
		auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();

		const char* arr[2] = { "Discrete", "Continuous" };

		reg.patch<SliceEngine::RigidBody>(entity, [&](SliceEngine::RigidBody& rb)
			{
				if (ImGui::TreeNodeEx("Rigidbody", mBaseFlags))
				{
					DisplayComponentHeader<SliceEngine::RigidBody>(entity);

					ImGui::Text("Mass");
					ImGui::SameLine(150.0f);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::DragFloat("##mass", &rb.mass);

					ImGui::Text("Gravity");
					ImGui::SameLine(150.0f);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::DragFloat("##gravity", &rb.gravityFactor);

					ImGui::Text("Is Kinematic");
					ImGui::SameLine(150.0f);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::Checkbox("##isKinematic", &rb.isKinematic);

					ImGui::Text("Linear Damping");
					ImGui::SameLine(150.0f);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::DragFloat("##linearDamp", &rb.linearDamping);

					ImGui::Text("Angular Damping");
					ImGui::SameLine(150.0f);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::DragFloat("##angularDamp", &rb.angularDamping);

					const char* currentLabel = arr[(int)rb.CollisionDetection];

					ImGui::Text("Collision Detection");
					ImGui::SameLine(150.0f);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::BeginCombo("##detection", currentLabel))
					{
						for (int i = 0; i < 2; i++)
						{
							bool isSelected = (rb.CollisionDetection == (JPH::EMotionQuality)i);
							if (ImGui::Selectable(arr[i], isSelected))
							{
								rb.CollisionDetection = (JPH::EMotionQuality)i;
							}

							// Highlight current item
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					ImGui::Text("Friction");
					ImGui::SameLine(150.0f);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::DragFloat("##friction", &rb.friction);

					ImGui::TreePop();
				}
			});

	}

	void InspectorWindow::DisplayCollider3D(entt::entity entity)
	{
		auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
		auto& colliderData = reg.get<SliceEngine::ColliderShape>(entity);

		const char* arr[2] = { "Moving", "Non-Moving" };
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

		reg.patch<SliceEngine::ColliderShape>(entity, [&](SliceEngine::ColliderShape& col)
			{
				if (ImGui::TreeNodeEx(colliderName.c_str(), mBaseFlags))
				{
					DisplayComponentHeader<SliceEngine::ColliderShape>(entity);

					ImGui::Text("Is Trigger");
					ImGui::SameLine(150.0f);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::Checkbox("##isTrigger", &col.isTrigger);

					ImGui::Text("Offset");
					ImGui::SameLine(150.0f);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x/3.0f);
					float buffer = col.offSet.GetX();
					if (ImGui::DragFloat("##offset_x", &buffer))
					{
						col.offSet.SetX(buffer);
					}
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 2.0f);
					buffer = col.offSet.GetY();
					if (ImGui::DragFloat("##offset_y", &buffer))
					{
						col.offSet.SetY(buffer);
					}
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					buffer = col.offSet.GetZ();
					if (ImGui::DragFloat("##offset_z", &buffer))
					{
						col.offSet.SetZ(buffer);
					}

					const char* currentLabel = arr[(int)col.layer];

					ImGui::Text("Collision Layer");
					ImGui::SameLine(150.0f);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::BeginCombo("##detection", currentLabel))
					{
						for (int i = 0; i < 2; i++)
						{
							bool isSelected = (col.layer == (JPH::ObjectLayer)i);
							if (ImGui::Selectable(arr[i], isSelected))
							{
								col.layer = (JPH::ObjectLayer)i;
							}

							// Highlight current item
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					ImGui::TreePop();
				}
			});
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
				auto& script_map = SliceEngine::gScriptSystem->mEntityClasses;
				auto& script_class = script_map.at(script.scriptName);
				auto& script_vars = script_class->mFields;

				for (auto& var : script_vars)
				{
					ImGui::Text(var.first.c_str());
					ImGui::SameLine(150.0f);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					std::string temp{ 0 };
					if (ImGui::InputText(("##" + var.first).c_str(), &temp))
					{

					}
				}

			}

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

	}
}