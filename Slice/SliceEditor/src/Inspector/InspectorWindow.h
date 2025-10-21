/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        InspectorWindow.h

 author:	  Chase Rodgrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the InspectorWindow class, which is responsible for drawing the Inspector window and its contents.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef INSPECTOR_WINDOW_H
#define INSPECTOR_WINDOW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEngine
{

}

namespace SliceEditor
{
	using ComponentDrawer = std::function<void(rttr::variant&)>;

	class Registry;

	class InspectorWindow : public EditorWindow
	{

		void DisplayEntityData(entt::entity entity);

		// temp component header
		template <typename ComponentType>
		void DisplayComponentHeader(entt::entity entity, bool closeable = true)
		{
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("-").x);
			if (ImGui::Button("-"))
			{
				ImGui::OpenPopup("ComponentContextMenu");  // Open the popup when button is clicked
			}

			if (ImGui::BeginPopup("ComponentContextMenu"))  // Check if popup is open
			{
				if (!closeable)
					ImGui::BeginDisabled();

				if (ImGui::MenuItem("Remove Component"))
				{
					SliceEngine::Core::GetInstance()->GetRegistry().remove<ComponentType>(entity);
				}
				
				if (!closeable)
					ImGui::EndDisabled();

				ImGui::EndPopup();
			}

			ImGui::Separator();
		}

		template<typename T>
		void DisplayComponentData(entt::entity, T& component)
		{
			// This function can be specialized for different component types
			// For example:
			// if constexpr (std::is_same_v<T, Transform>) { ... }
			// else if constexpr (std::is_same_v<T, Rigidbody>) { ... }
			// etc.
		}
		//Template Specialisation for Transform
		template<>
		void DisplayComponentData(entt::entity entity, SliceEngine::Transform& tr)
		{
			if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{

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
		// to do in m2 : use rttr to read types.

		template<>
		void DisplayComponentData(entt::entity entity, SliceEngine::AudioSource)
		{

		}

		void DisplayTransform(entt::entity entity);
		void DisplaySceneGraph(entt::entity entity);
		void DisplayAudioSource(entt::entity entity);
		void DisplayMeshRenderer(entt::entity entity);
		void DisplayRigidbody(entt::entity entity);
		void DisplayCollider3D(entt::entity entity);
		void DisplaySliceScript(entt::entity entity);
		void AddComponentButton(entt::entity entity);


	public:

		InspectorWindow(Registry& reg) : EditorWindow(reg) {};
		~InspectorWindow() = default;
		void Init() override;
		void Draw() override final;
	};
}

#endif