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

		if (SliceEngine::Core::GetInstance()->GetRegistry().valid(selected_entity.value()))
		{
			auto entity = selected_entity.value();
			DisplayTransform();

			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::RigidBody>(entity))
			{
				DisplayRigidbody();
			}

			// to do: change to better format
			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::ColliderShape>(entity))
			{
				DisplayCollider3D();
			}

			if (SliceEngine::Core::GetInstance()->GetRegistry().try_get<SliceEngine::AudioSource>(entity))
			{
				DisplayAudioSource();
			}

			ImGui::Separator();

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
		auto& tr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(selected_entity.value());

		DragVec3InputHeader("Translation", "t", tr.position);
		//DragVec2InputHeader(service, "Scale", "s", transform.localScale);
		//DragDoubleInputHeader(service, "Rotation", "##r", transform.localRotation, "%.3f");
	}

	void InspectorWindow::DisplayAudioSource()
	{
		auto& as = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::AudioSource>(selected_entity.value());

		std::vector<std::string> items = {"BGM_MainMenu_Mix1, 3DAudioTest"};
		static int current_item = 0;

		if (ImGui::TreeNodeEx("AudioSource"))
		{
			
			if (ImGui::BeginCombo("Sound Name", items[current_item].c_str()))
			{
				for (int i = 0; i < items.size(); i++)
				{
					bool is_selected = (current_item == i);

					if (ImGui::Selectable(items[i].c_str(), is_selected))
					{
						current_item = i;
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
			}

			ImGui::EndCombo();

			ImGui::SliderFloat("Volume", &as.currentVolume, 0.0f, 1.0f);


			ImGui::Checkbox("isLoop", &as.isLoop);

			ImGui::Checkbox("is3D", &as.is3D);
		}

		ImGui::TreePop();
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

			if (ImGui::Selectable("Add AudioSource"))
			{
				SliceEngine::Core::GetInstance()->GetRegistry().emplace<SliceEngine::AudioSource>(selected_entity.value());
			}

			ImGui::EndPopup();
		}
	}
}