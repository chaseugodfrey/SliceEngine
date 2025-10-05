#include <pch.h>
#include "InspectorManager.h"
#include "InspectorWindow.h"
#include "ComponentPropertiesGUI.h"
#include "../../SliceEngine/src/Scripting/ScriptSystem.h"

namespace SliceEditor
{
	InspectorWindow::InspectorWindow(InspectorManager& manager) : mManager(manager)
	{

	}

	void InspectorWindow::Draw()
	{
		ImGui::Begin("Inspector");

		auto& entities = mManager.GetSelectedEntities();

		if (entities.size() > 0)
			selected_entity = *entities.begin();

		else
			selected_entity.reset();

		if (!selected_entity.has_value())
		{
			ImGui::End();
			return;
		}

		DisplayEntityData();

		// to do : use gamefactory component view
		if (SliceEngine::Core::GetInstance()->GetRegistry().valid(selected_entity.value()))
		{
			auto entity = selected_entity.value();
			DisplayTransform();
			ImGui::Separator();
			DisplaySceneGraph();
			ImGui::Separator();

			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::ColliderShape>(entity))
			{
				DisplayCollider3D();
				ImGui::Separator();
			}

			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Renderer>(entity))
			{
				DisplayMeshRenderer();
				ImGui::Separator();
			}

			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::RigidBody>(entity))
			{
				DisplayRigidbody();
				ImGui::Separator();
			}

			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::AudioSource>(entity))
			{
				DisplayAudioSource();
				ImGui::Separator();
			}

			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::Script>(entity))
			{
				DisplaySliceScript();
				ImGui::Separator();
			}

			AddComponentButton();
		}

		ImGui::End();
	}

	void R()
	{
		
	}

	//void InspectorWindow::DisplayComponentHeader(std::string const component_name)

	
	void InspectorWindow::DisplayEntityData()
	{
		//static bool is_active = false;
		//ImGui::Checkbox("##is_active", &is_active);
		//ImGui::SameLine();

		auto original_name = SliceEngine::FactoryInstance.GetGOByEntity(selected_entity.value()).GetName();
		std::string editable_name = original_name;
		if (ImGui::InputText("##name", &editable_name))
		{
			if (editable_name != original_name)
				SliceEngine::FactoryInstance.GetGOByEntity(selected_entity.value()).SetName(editable_name);
		}
		
		ImGui::SameLine();
		ImGui::Text(std::to_string((uint64_t)selected_entity.value()).c_str());
		ImGui::Separator();

	}

	//void InspectorWindow::DisplayComponentHeader(bool closeable)
	//{
	//}

	void InspectorWindow::DisplayTransform()
	{
		if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& tr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(selected_entity.value());

			DisplayComponentHeader<SliceEngine::Transform>(false);

			DragVec3InputHeader("Position", "##t", tr.position);
			DragVec3InputHeader("Rotation", "##r", tr.rotation);
			DragVec3InputHeader("Scale", "##s", tr.scale);

			// for testing purposes
			ImGui::BeginDisabled();
			ImGui::Text("Parent: ");
			auto& scene_graph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(selected_entity.value());
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

	void InspectorWindow::DisplaySceneGraph()
	{
		auto& sg = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(selected_entity.value());

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

	void InspectorWindow::DisplayAudioSource()
	{
		auto& as = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::AudioSource>(selected_entity.value());

		auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
		auto entity = selected_entity.value();

		reg.patch<SliceEngine::AudioSource>(entity, [&](auto& as) {
			if (ImGui::TreeNodeEx("AudioSource"))
			{
				DisplayComponentHeader<SliceEngine::AudioSource>();

				ImGui::Text("Audio Clip");
				ImGui::SameLine(150);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::InputText("##audio_file", &as.soundName, ImGuiInputTextFlags_ReadOnly);

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

				ImGui::TreePop();
			}
			});

	}

	void InspectorWindow::DisplayMeshRenderer()
	{
		auto& rend = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Renderer>(selected_entity.value());

		if (ImGui::TreeNodeEx("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DisplayComponentHeader<SliceEngine::Renderer>();

			ImGui::Text("Mesh");
			ImGui::SameLine(150.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			std::string model_guid_string = std::to_string(rend.model.GetGUID());
			if (ImGui::InputText("##mesh", &model_guid_string, ImGuiInputTextFlags_ReadOnly))
			{
				rend.model = SliceEngine::GUID(std::stoll(model_guid_string));
			}

			ImGui::Text("Texture");
			ImGui::SameLine(150.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			std::string texture_guid_string = std::to_string(rend.model.GetGUID());
			if (ImGui::InputText("##texture", &texture_guid_string, ImGuiInputTextFlags_ReadOnly))
			{
				rend.model = SliceEngine::GUID(std::stoll(texture_guid_string));
			}

			ImGui::TreePop();
		}

	}

	void InspectorWindow::DisplayRigidbody()
	{
		auto& rb = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::RigidBody>(selected_entity.value());

		if (ImGui::TreeNodeEx("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DisplayComponentHeader<SliceEngine::RigidBody>();

			ImGui::Text("Friction");
			ImGui::SameLine(150.0f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat("##friction", &rb.friction);

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayCollider3D()
	{
		auto& col3d = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::ColliderShape>(selected_entity.value());

		if (ImGui::TreeNodeEx("Collider3D", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DisplayComponentHeader<SliceEngine::ColliderShape>();

			ImGui::Text("Trigger");
			ImGui::SameLine(150.0f);
			ImGui::Checkbox("##trigger", &col3d.isTrigger);

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplaySliceScript()
	{
		auto& script = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Script>(selected_entity.value());

		if (ImGui::TreeNodeEx("Script", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DisplayComponentHeader<SliceEngine::Script>();

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

	void InspectorWindow::AddComponentButton()
	{

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("##add_component_list");
		}

		if (ImGui::BeginPopupContextItem("##add_component_list"))
		{
			auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
			auto selectedGO = SliceEngine::FactoryInstance.GetGOByEntity(selected_entity.value());

			if(!selectedGO.HasComponent<SliceEngine::Renderer>())
			{
				if (ImGui::Selectable("Add Renderer"))
				{
					reg.emplace<SliceEngine::Renderer>(selected_entity.value());
				}
			}

			if (!selectedGO.HasComponent<SliceEngine::RigidBody>())
			{
				if (ImGui::Selectable("Add Rigidbody"))
				{
					reg.emplace<SliceEngine::RigidBody>(selected_entity.value());
				}
			}

			if(!selectedGO.HasComponent<SliceEngine::ColliderShape>())
			{
				if (ImGui::Selectable("Add Collider3D"))
				{
					reg.emplace<SliceEngine::ColliderShape>(selected_entity.value());
				}
			}
			
			if(!selectedGO.HasComponent<SliceEngine::Script>())
			{
				if (ImGui::Selectable("Add Script Container"))
				{
					reg.emplace<SliceEngine::Script>(selected_entity.value());
				}
			}

			if(!selectedGO.HasComponent<SliceEngine::AudioSource>())
			{
				if (ImGui::Selectable("Add AudioSource"))
				{
					SliceEngine::Core::GetInstance()->GetRegistry().emplace<SliceEngine::AudioSource>(selected_entity.value());
				}
			}

			ImGui::EndPopup();
		}
	}
}