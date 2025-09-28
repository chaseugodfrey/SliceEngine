#include <pch.h>
#include "SceneViewWindow.h"

#include "SceneViewManager.h"
#include "../../SliceEngine/src/Graphics/RenderManager.h"

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

		auto& transform = mManager.mRenderManager.GetMainCameraTransform();
		glm::vec3 forward, right, up;
		
		mManager.mRenderManager.GetMainCameraAxis(forward, right, up);

		if (ImGui::IsKeyDown(ImGuiKey_W))
		{
			transform.position += forward * 0.1f;
		}

		if (ImGui::IsKeyDown(ImGuiKey_S))
		{
			transform.position -= forward * 0.1f;
		}

		if (ImGui::IsKeyDown(ImGuiKey_A))
		{
			transform.position -= right * 0.1f;
		}

		if (ImGui::IsKeyDown(ImGuiKey_D))
		{
			transform.position += right * 0.1f;
		}

		// Btw for rotation
		//camera.rotation.y -= (newMousePos.x - mousePos.x);
		//camera.rotation.z = std::clamp(camera.rotation.z - (newMousePos.y - mousePos.y), -89.f, 89.f);


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
