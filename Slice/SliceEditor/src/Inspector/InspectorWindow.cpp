#include <pch.h>
#include "InspectorManager.h"
#include "InspectorWindow.h"
#include "ComponentPropertiesGUI.h"

namespace SliceEditor
{
	InspectorWindow::InspectorWindow(InspectorManager& manager) : mManager(manager)
	{

	}

	void InspectorWindow::Draw()
	{
		ImGui::Begin("Inspector");

		UpdateSelectedEntity(entt::entity{ 1 });

		if (SliceEngine::Core::GetInstance()->GetRegistry().valid(selected_entity))
		{
			DisplayTransform();

			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::RigidBody>(selected_entity))
			{
				DisplayRigidbody();
			}

			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::ColliderShape>(selected_entity))
			{
				DisplayCollider3D();
			}

			ImGui::Separator();

			AddComponentButton();
		}
		


		ImGui::End();
	}

	void InspectorWindow::UpdateSelectedEntity(entt::entity entity)
	{
		selected_entity = entity;
	}

	void InspectorWindow::DisplayEntityData()
	{
		static bool is_active = false;
		ImGui::Checkbox("#is_active", &is_active);
		ImGui::SameLine();

		std::string name = "name";
		ImGui::InputText("##name", &name);

	}

	void InspectorWindow::DisplayTransform()
	{
		auto& tr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(selected_entity);

		DragVec3InputHeader("Translation", "t", tr.position);
		//DragVec2InputHeader(service, "Scale", "s", transform.localScale);
		//DragDoubleInputHeader(service, "Rotation", "##r", transform.localRotation, "%.3f");
	}

	void InspectorWindow::DisplayAudioSource()
	{
		auto& as = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::AudioSource>(selected_entity);

		/*if (ImGui::TreeNodeEx("AudioSource"))
		{
			ImGui::BeginCombo("Sound Name", )
		}*/
	}

	void InspectorWindow::DisplayMeshRenderer()
	{

	}

	void InspectorWindow::DisplayRigidbody()
	{
		auto& rb = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::RigidBody>(selected_entity);

		if (ImGui::TreeNodeEx("Rigidbody"))
		{
			ImGui::DragFloat("Friction", &rb.friction);

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayCollider3D()
	{
		auto& col3d = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::ColliderShape>(selected_entity);

		if (ImGui::TreeNodeEx("Collider3D"))
		{
			ImGui::Checkbox("Is Trigger", &col3d.isTrigger);

			ImGui::TreePop();
		}
	}

	void InspectorWindow::AddComponentButton()
	{
		const char* arr[5] =
		{
			"a",
			"b",
			"c",
			"d",
			"e"
		};

		static int index = 0;
		static bool selected = false;

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("##add_component_list");
		}

		if (ImGui::BeginPopupContextItem("##add_component_list"))
		{
			if (ImGui::Selectable("Add Rigidbody"))
			{
				SliceEngine::Core::GetInstance()->GetRegistry().emplace<SliceEngine::RigidBody>(selected_entity);
			}

			if (ImGui::Selectable("Add Collider3D"))
			{
				SliceEngine::Core::GetInstance()->GetRegistry().emplace<SliceEngine::ColliderShape>(selected_entity);
			}

			ImGui::EndPopup();
		}
	}
}