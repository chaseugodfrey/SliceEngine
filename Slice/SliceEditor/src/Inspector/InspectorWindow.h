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
	class InspectorManager;

	class InspectorWindow : public EditorWindow
	{
		InspectorManager& mManager;

		// to do: change later
		std::optional<entt::entity> selectedEntity;

		void DisplayEntityData();

		// temp component header
		template <typename ComponentType>
		void DisplayComponentHeader(bool closeable = true)
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
					SliceEngine::Core::GetInstance()->GetRegistry().remove<ComponentType>(selectedEntity.value());
				}
				
				if (!closeable)
					ImGui::EndDisabled();

				ImGui::EndPopup();
			}

			ImGui::Separator();
		}

		template<typename T>
		void DisplayComponentData(T& component)
		{
			// This function can be specialized for different component types
			// For example:
			// if constexpr (std::is_same_v<T, Transform>) { ... }
			// else if constexpr (std::is_same_v<T, Rigidbody>) { ... }
			// etc.
		}

		template<>
		void DisplayComponentData<SliceEngine::Transform>(SliceEngine::Transform& component)
		{

		}
		// to do in m2 : use rttr to read types.
		void DisplayTransform();
		void DisplaySceneGraph();
		void DisplayAudioSource();
		void DisplayMeshRenderer();
		void DisplayRigidbody(); 
		void DisplayCollider3D();
		void DisplaySliceScript();
		void AddComponentButton();


	public:

		InspectorWindow(InspectorManager& manager);
		~InspectorWindow() = default;
		void Draw() override final;
	};
}

#endif