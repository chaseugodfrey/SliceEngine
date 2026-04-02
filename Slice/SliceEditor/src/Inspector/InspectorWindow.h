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
		ImGuiTreeNodeFlags mBaseFlags;

#pragma region Entitiy Inspection
		// Displaying Entities
		void DisplayEntityData(entt::entity entity);

		// temp component header
		template <typename ComponentType>
		bool DisplayComponentHeader(entt::entity entity, bool closeable = true)
		{
			bool isDeleted = false;
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
					isDeleted = true;
				}
				
				if (!closeable)
					ImGui::EndDisabled();

				ImGui::EndPopup();
			}

			ImGui::Separator();

			return isDeleted;
		}

		void DisplayTransform(entt::entity entity);
		void DisplaySceneGraph(entt::entity entity);
		void DisplayAudioSource(entt::entity entity);
		void DisplayAudioListener(entt::entity entity);
		void DisplayMeshRenderer(entt::entity entity);
		void DisplayCamera(entt::entity entity);
		void DisplayRigidbody(entt::entity entity);
		void DisplayCollider3D(entt::entity entity);
		void DisplaySliceScript(entt::entity entity);
		void DisplayLight(entt::entity entity);
		void DisplayAnimator(entt::entity entity);
		void DisplayParticleSystem(entt::entity entity);
		void AddComponentButton(entt::entity entity);


		void DisplayRectTransform(entt::entity entity);
		void DisplaySpriteRenderer(entt::entity entity);
		void DisplaySpriteRendererGammaOverride(entt::entity entity);
		void DisplaySpriteAnimator(entt::entity entity);
		void DisplayFontRenderer(entt::entity entity);
		void DisplayCanvas(entt::entity entity);
		void DisplayButton(entt::entity entity);
		void DisplaySlider(entt::entity entity);
#pragma endregion

#pragma region Main Draw Functions
		void DisplayEntity(EntityNode* node);
		void DisplayMaterial(DirectoryNode* node);
		void DisplayPrefab(EntityNode* node);
		void DisplayState(StateNode* node);
		void DisplayTransition(TransitionLinkNode* node);
#pragma endregion
	public:

		InspectorWindow(Registry& reg) : EditorWindow(reg) {};
		~InspectorWindow() = default;
		void Init() override;
		void Draw() override final;
	};
}

#endif