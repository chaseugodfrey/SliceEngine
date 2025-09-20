#include <pch.h>
#include "SceneViewWindow.h"
#include "../../SliceEngine/src/Graphics/CameraSystem.h"

namespace SliceEditor
{
	

	SceneViewWindow::SceneViewWindow(SceneViewManager& manager) : mManager(manager)
	{
	}

	void SceneViewWindow::SetCameraTexture(GLuint texture_id)
	{
		tex_id = texture_id;
	}

	void SceneViewWindow::Draw()
	{
		ImGui::Begin("Scene");

		auto size = ImGui::GetContentRegionAvail();
		ImVec2 pos = ImGui::GetCursorScreenPos();

		if (ImGui::IsKeyDown(ImGuiKey_W))
		{
			
		}

		if (ImGui::IsKeyDown(ImGuiKey_S))
		{
			
		}

		if (ImGui::IsKeyDown(ImGuiKey_A))
		{
			
		}

		if (ImGui::IsKeyDown(ImGuiKey_D))
		{
			
		}

		ImGui::GetWindowDrawList()->AddImage(
			//(void*)editorState.renderManager->GetTexture(), // Placeholder texture ID
			(void*)tex_id,
			ImVec2(pos.x, pos.y),
			ImVec2(pos.x + ImGui::GetContentRegionAvail().x, pos.y + ImGui::GetContentRegionAvail().y),
			ImVec2(0, 1),
			ImVec2(1, 0)
		);

		ImGui::End();
	}

}
