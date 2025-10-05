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
		std::optional<entt::entity> selected_entity;

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
					SliceEngine::Core::GetInstance()->GetRegistry().remove<ComponentType>(selected_entity.value());
				}
				
				if (!closeable)
					ImGui::EndDisabled();

				ImGui::EndPopup();
			}

			ImGui::Separator();
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

		void R();



	public:

		InspectorWindow(InspectorManager& manager);
		~InspectorWindow() = default;
		void Draw() override final;
	};
}

#endif