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

		// to do : use gamefactory component view
		if (SliceEngine::Core::GetInstance()->GetRegistry().valid(selected_entity.value()))
		{
			auto entity = selected_entity.value();
			DisplayTransform();
			ImGui::Separator();

			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::ColliderShape>(entity))
			{
				DisplayCollider3D();
				ImGui::Separator();
			}

			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::RigidBody>(entity))
			{
				DisplayRigidbody();
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
		static bool is_active = false;
		ImGui::Checkbox("#is_active", &is_active);
		ImGui::SameLine();

		std::string name = "name";
		ImGui::InputText("##name", &name);

	}

	void InspectorWindow::DisplayTransform()
	{
		if (ImGui::TreeNodeEx("Transform"))
		{
			auto& tr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(selected_entity.value());

			DragVec3InputHeader("Translation", "##t", tr.position);
			DragVec3InputHeader("Rotation", "##r", tr.rotation);
			DragVec3InputHeader("Scale", "##s", tr.scale);

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayMeshRenderer()
	{

	}

	void InspectorWindow::DisplayRigidbody()
	{
		auto& rb = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::RigidBody>(selected_entity.value());

		if (ImGui::TreeNodeEx("Rigidbody"))
		{
			ImGui::DragFloat("Friction", &rb.friction);

			ImGui::TreePop();
		}
	}

	void InspectorWindow::DisplayCollider3D()
	{
		auto& col3d = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::ColliderShape>(selected_entity.value());

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
				SliceEngine::Core::GetInstance()->GetRegistry().emplace<SliceEngine::RigidBody>(selected_entity.value());
			}

			if (ImGui::Selectable("Add Collider3D"))
			{
				SliceEngine::Core::GetInstance()->GetRegistry().emplace<SliceEngine::ColliderShape>(selected_entity.value());
			}

			ImGui::EndPopup();
		}
	}
}