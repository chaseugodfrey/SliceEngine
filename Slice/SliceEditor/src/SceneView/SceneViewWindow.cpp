#include <pch.h>
#include "SceneViewWindow.h"
#include "SceneViewManager.h"
#include "../../SliceEngine/src/Graphics/RenderManager.h"
#include "../../SliceEngine/src/Graphics/CameraSystem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/euler_angles.hpp"

namespace SliceEditor
{

	SceneViewWindow::SceneViewWindow(SceneViewManager& manager, SliceEngine::GameObject cam) : mManager(manager), camObj(cam)
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

		//auto& transform = mManager.mRenderManager.GetMainCameraTransform();
		//glm::vec3 forward, right, up;
		
		//mManager.mRenderManager.GetMainCameraAxis(forward, right, up);

		if (ImGui::IsKeyDown(ImGuiKey_W))
		{
			//transform.position += forward * 0.1f;
		}

		if (ImGui::IsKeyDown(ImGuiKey_S))
		{
			//transform.position -= forward * 0.1f;
		}

		if (ImGui::IsKeyDown(ImGuiKey_A))
		{
			//transform.position -= right * 0.1f;
		}

		if (ImGui::IsKeyDown(ImGuiKey_D))
		{
			//transform.position += right * 0.1f;
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

		auto* drawlist = ImGui::GetForegroundDrawList();

		ImGuizmo::SetDrawlist(drawlist);
		ImGuizmo::Enable(true);
		static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
		static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

		

		// need to pass in cam and store as reference member
		// need to get selected entity

		//auto& camera = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(camObj.GetEntity());
		//auto& camTrans = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(camObj.GetEntity());

		//glm::vec3 target{ 1.f, 0.f, 0.f }, up{ 0.f, 1.f, 0.f };
		//glm::mat3 rot = glm::eulerAngleXYZ(glm::radians(camTrans.rotation.x), glm::radians(camTrans.rotation.y), glm::radians(camTrans.rotation.z));
		//glm::mat4 V = glm::lookAt(camTrans.position, camTrans.position + rot * target, rot * up);
		//glm::mat4 P = glm::perspective(glm::radians(camera.pov), static_cast<float>(camera.width) / static_cast<float>(camera.height), camera.near, camera.far);

		//entt::entity tmp{ 2 };


		//if (SliceEngine::Core::GetInstance()->GetRegistry().valid(tmp))
		//{
		//	auto& tr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(tmp);

		//	//float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		//	//ImGuizmo::DecomposeMatrixToComponents(&tr.transform[0][0], matrixTranslation, matrixRotation, matrixScale);
		//	//ImGui::InputFloat3("Tr", matrixTranslation);
		//	//ImGui::InputFloat3("Rt", matrixRotation);
		//	//ImGui::InputFloat3("Sc", matrixScale);
		//	//ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, &tr.transform[0][0]);

		//	//ImGuiIO& io = ImGui::GetIO();
		//	ImGuizmo::SetRect(0, 0, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
		//	ImGuizmo::Manipulate(&V[0][0], &P[0][0], mCurrentGizmoOperation, mCurrentGizmoMode, &tr.transform[0][0], NULL);


		//}

		ImGui::End();
	}

}
