#include <pch.h>
#include "SceneViewWindow.h"
#include "SceneViewManager.h"
#include "../../SliceEngine/src/Graphics/RenderManager.h"
#include "../../SliceEngine/src/Graphics/CameraSystem.h"
#include "Core/Registry.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace SliceEditor
{
	void GlmHelper_FloatPtrToVec3(glm::vec3& vec, const float* arr)
	{
		vec.x = arr[0];
		vec.y = arr[1];
		vec.z = arr[2];
	}

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

		// Draw Utility Bar

		ImGui::BeginGroup();
		ImGui::Text("Speed:");
		ImGui::SameLine();
		ImGui::Text("%.3f", mManager.GetCameraSpeed());
		ImGui::EndGroup();

		auto size = ImGui::GetContentRegionAvail();
		ImVec2 pos = ImGui::GetCursorScreenPos();

		auto& cam = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(camObj.GetEntity());
		auto& cam_tr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(camObj.GetEntity());

		glm::vec3 forward{}, right{}, up{};
		cam.renderTag = SliceEngine::RENDER_TAG::DEBUG_OBJ_TAG | SliceEngine::RENDER_TAG::DEBUG_FRUSTRUM_TAG | SliceEngine::RENDER_TAG::DEBUG_GRID_TAG;

		SliceEngine::Core::GetInstance()->GetRenderManager()->GetCameraAxis(camObj, forward, right, up);

		if (ImGui::IsWindowFocused())
		{
			auto& io = ImGui::GetIO();
			if (io.KeyShift)
			{
				if (ImGui::IsKeyDown(ImGuiKey_W))
				{
					cam_tr.position += forward * mManager.GetCameraSpeed();
				}

				if (ImGui::IsKeyDown(ImGuiKey_S))
				{
					cam_tr.position -= forward * mManager.GetCameraSpeed();
				}

				if (ImGui::IsKeyDown(ImGuiKey_A))
				{
					cam_tr.position -= right * mManager.GetCameraSpeed();
				}

				if (ImGui::IsKeyDown(ImGuiKey_D))
				{
					cam_tr.position += right * mManager.GetCameraSpeed();
				}

				if (ImGui::IsKeyDown(ImGuiKey_Q))
				{
					cam_tr.position -= up * mManager.GetCameraSpeed();
				}

				if (ImGui::IsKeyDown(ImGuiKey_E))
				{
					cam_tr.position += up * mManager.GetCameraSpeed();
				}

				//Camera Speed Change
				if (io.MouseWheel > 0.0f)
				{
					mManager.ChangeCameraSpeed(0.01f);
				}
				else if (io.MouseWheel < 0.0f)
				{
					mManager.ChangeCameraSpeed(-0.01f);
				}
			}

			static ImVec2 pos{};
			static bool isRotating = false;
			static ImVec2 init_rot{};

			if (ImGui::IsWindowHovered())
			{
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					init_rot.x = cam_tr.rotation.y;
					init_rot.y = cam_tr.rotation.z;
					pos = ImGui::GetMousePos();
					isRotating = true;
				}

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
				{
					isRotating = false;
				}
			}

			if (isRotating)
			{
				if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
				{
					ImVec2 mouse_diff = ImGui::GetMousePos() - pos;
					cam_tr.rotation.y = init_rot.x - mouse_diff.x;
					cam_tr.rotation.z = init_rot.y - mouse_diff.y;
				}
			}
		}

		// Btw for rotation
		//camera.rotation.y -= (newMousePos.x - mousePos.x);
		//camera.rotation.z = std::clamp(camera.rotation.z - (newMousePos.y - mousePos.y), -89.f, 89.f);


		// Drawing cam texture
		ImGui::GetWindowDrawList()->AddImage(
			(void*)cam.textureID,
			ImVec2(pos.x, pos.y),
			ImVec2(pos.x + ImGui::GetContentRegionAvail().x, pos.y + ImGui::GetContentRegionAvail().y),
			ImVec2(0, 1),
			ImVec2(1, 0)
		);

		// ======= IMGUIZMO =======
		auto* drawlist = ImGui::GetWindowDrawList();

		ImGuizmo::SetDrawlist(drawlist);
		ImGuizmo::Enable(true);

		static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
		static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);

		auto& set = mManager.GetRegistry().GetSelectionSystem().GetSelectedEntities();

		if (set.size() == 0)
		{
			ImGui::End();
			return;
		}

		// get cam view & perspective
		glm::mat4 V = glm::lookAt(cam_tr.position, cam_tr.position + forward, up);
		glm::mat4 P = glm::perspective(glm::radians(cam.pov), static_cast<float>(cam.width) / static_cast<float>(cam.height), cam.near, cam.far);

		// get entities & transform components
		auto entt = *set.begin();
		auto& tmp_tr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(entt);

		// set gizmo limits to window
		ImVec2 window_pos = ImGui::GetWindowPos();
		ImGuizmo::SetRect(window_pos.x, window_pos.y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

		// get transforms
		glm::mat4 world_tr = tmp_tr.transform; // already computed as parent * local
		glm::mat4 new_world_tr = world_tr;

		if (ImGuizmo::Manipulate(glm::value_ptr(V), glm::value_ptr(P),
			mCurrentGizmoOperation, mCurrentGizmoMode,
			glm::value_ptr(new_world_tr), NULL))
		{
			auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();

			// Convert back to local if we have a parent
			if (auto scene_graph = reg.try_get<SliceEngine::SceneGraph>(entt)) {
				auto parent_entity = scene_graph->neighbours[SliceEngine::SceneGraph::UP];
				if (parent_entity != entt::null && parent_entity != entt::entity{0}) {
					auto& parent_tr = reg.get<SliceEngine::Transform>(parent_entity);
					glm::mat4 parent_world = parent_tr.transform;
					tmp_tr.transform_local = glm::inverse(parent_world) * new_world_tr;
				}
				else {
					tmp_tr.transform_local = new_world_tr; // root entity
				}
			}
			else {
				tmp_tr.transform_local = new_world_tr;
			}

			// Decompose new local transform into position/rotation/scale
			float matrixTranslation[3], matrixRotation[3], matrixScale[3];
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(tmp_tr.transform_local),
				matrixTranslation, matrixRotation, matrixScale);
			GlmHelper_FloatPtrToVec3(tmp_tr.position, matrixTranslation);
			GlmHelper_FloatPtrToVec3(tmp_tr.rotation, matrixRotation);
			GlmHelper_FloatPtrToVec3(tmp_tr.scale, matrixScale);
		}

		ImGui::End();

	}
}
