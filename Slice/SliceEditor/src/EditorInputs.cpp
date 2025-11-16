#include <pch.h>
#include "EditorInputs.h"

namespace SliceEditor
{
	void EditorInputs::Update()
	{
		if (!isActive)
			return;

		auto& io = ImGui::GetIO();
		//Normal Inputs
		if (ImGui::IsKeyPressed(ImGuiKey_Delete))
		{
			EventManager::GetInstance()->Publish<DeleteSelectedEntities>();
		}


		// ctrl inputs

		if (io.KeyCtrl)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_S))
			{
				EditorUtilities::Scene_Save();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_D))
			{

			}

			if (ImGui::IsKeyPressed(ImGuiKey_C))
			{

			}

			if (ImGui::IsKeyPressed(ImGuiKey_V))
			{

			}

			if (ImGui::IsKeyPressed(ImGuiKey_Z))
			{
				if (io.KeyShift)
				{
					EventManager::GetInstance()->Publish<RedoEvent>();
				}

				else
				{
					EventManager::GetInstance()->Publish<UndoEvent>();
				}
			}

			if (ImGui::IsKeyPressed(ImGuiKey_Y))
			{

			}

		}
	}
}