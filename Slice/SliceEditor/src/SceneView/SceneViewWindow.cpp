/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        SceneViewWindow.cpp

 author:	  Chase Rodgrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the SceneViewWindow class, which is responsible for drawing the Scene View window and its contents.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "SceneViewWindow.h"
#include "SceneViewManager.h"
#include "../../SliceEngine/src/Graphics/RenderManager.h"
#include "../../SliceEngine/src/Graphics/CameraSystem.h"
#include "Core/Registry.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace SliceEditor
{
	// Convert Euler angles (in degrees) to quaternion
	glm::quat EulerToQuaternion(const glm::vec3& euler_degrees) {
		glm::vec3 euler_radians = glm::radians(euler_degrees);
		return glm::quat(euler_radians);
	}

	// Convert quaternion to Euler angles (in degrees)
	glm::vec3 QuaternionToEuler(const glm::quat& q) {
		glm::vec3 euler_radians = glm::eulerAngles(q);
		return glm::degrees(euler_radians);
	}

	glm::vec3 ExtractEulerXYZ(const glm::mat4& transform) {
		glm::vec3 euler;

		// Extract rotation part
		glm::mat3 rotMtx(transform);

		// Assuming XYZ intrinsic rotation order
		euler.y = glm::degrees(asin(glm::clamp(rotMtx[0][2], -1.0f, 1.0f)));

		if (cos(glm::radians(euler.y)) > 0.0001f) {
			euler.x = glm::degrees(atan2(-rotMtx[1][2], rotMtx[2][2]));
			euler.z = glm::degrees(atan2(-rotMtx[0][1], rotMtx[0][0]));
		}
		else {
			euler.x = glm::degrees(atan2(rotMtx[2][1], rotMtx[1][1]));
			euler.z = 0.0f;
		}

		return euler;
	}

	void GlmHelper_FloatPtrToVec3(glm::vec3& vec, const float* arr)
	{
		vec.x = arr[0];
		vec.y = arr[1];
		vec.z = arr[2];
	}

	glm::vec3 NormalizeEulerAngles(const glm::vec3& euler)
	{
		glm::vec3 result = euler;
		for (int i = 0; i < 3; i++)
		{
			// wrap to -180..180
			while (result[i] > 180.f) result[i] -= 360.f;
			while (result[i] < -180.f) result[i] += 360.f;

			// optional: zero very small values
			if (fabs(result[i]) < 0.0001f) result[i] = 0.f;
		}
		return result;
	}

	SceneViewWindow::SceneViewWindow(SceneViewManager& manager, SliceEngine::GameObject cam) : camObj(cam), mManager(manager), tex_id(0)
	{
	}

	void SceneViewWindow::SetCameraTexture(GLuint texture_id)
	{
		tex_id = texture_id;
	}

	void SceneViewWindow::Draw()
	{

		ImGui::Begin("Scene");

		auto& io = ImGui::GetIO();

		// Draw Utility Bar

		ImGui::BeginGroup();
		ImGui::Text("Speed:");
		ImGui::SameLine();
		ImGui::Text("%.3f", mManager.GetCameraSpeed());
		ImGui::EndGroup();


#pragma region IO Calculation

		auto size = ImGui::GetContentRegionAvail();
		ImVec2 pos = ImGui::GetCursorScreenPos();

		auto& cam = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(camObj.GetEntity());
		auto& cam_tr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(camObj.GetEntity());

		ImVec2 window_pos = ImGui::GetWindowPos();
		ImVec2 window_size = ImGui::GetWindowSize();

		int screen_width, screen_height;
		GLFWwindow* mWindow = SliceEngine::Core::GetInstance()->GetWindow();
		glfwGetWindowSize(mWindow, &screen_width, &screen_height);

		ImVec2 scene_window_pos = ImGui::GetCursorScreenPos();
		ImVec2 scene_window_size = { window_size.x, window_size.y - (scene_window_pos.y - window_pos.y) };

		float mouse_relative_x = io.MousePos.x - scene_window_pos.x;
		float mouse_relative_y = io.MousePos.y - scene_window_pos.y;

		float mouse_scaled_x = mouse_relative_x / window_size.x * screen_width;
		float mouse_scaled_y = mouse_relative_y / window_size.y * screen_height;
		mouse_scaled_y = cam.height - mouse_scaled_y;

#pragma endregion

		glm::vec3 forward{}, right{}, up{};
		cam.renderTag = SliceEngine::RENDER_TAG::DEBUG_OBJ_TAG | SliceEngine::RENDER_TAG::DEBUG_FRUSTRUM_TAG | SliceEngine::RENDER_TAG::DEBUG_GRID_TAG;

		SliceEngine::Core::GetInstance()->GetRenderManager()->GetCameraAxis(camObj, forward, right, up);

		if (ImGui::IsWindowFocused())
		{
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

			if (ImGui::IsKeyDown(ImGuiKey_W))
			{
				mManager.SetGizmoOperation(ImGuizmo::OPERATION::TRANSLATE);
			}

			if (ImGui::IsKeyDown(ImGuiKey_E))
			{
				mManager.SetGizmoOperation(ImGuizmo::OPERATION::ROTATE);
			}

			if (ImGui::IsKeyDown(ImGuiKey_R))
			{
				mManager.SetGizmoOperation(ImGuizmo::OPERATION::SCALE);
			}

			static ImVec2 rotate_anchor{};
			static bool isRotating = false;
			static ImVec2 init_rot{};

			if (ImGui::IsWindowHovered())
			{
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					init_rot.x = cam_tr.rotation.y;
					init_rot.y = cam_tr.rotation.z;
					rotate_anchor = ImGui::GetMousePos();
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
					ImVec2 mouse_diff = ImGui::GetMousePos() - rotate_anchor;
					cam_tr.rotation.y = init_rot.x - mouse_diff.x;
					cam_tr.rotation.z = init_rot.y - mouse_diff.y;
				}
			}
		}

#pragma region Scene Drawing

		// Btw for rotation
		//camera.rotation.y -= (newMousePos.x - mousePos.x);
		//camera.rotation.z = std::clamp(camera.rotation.z - (newMousePos.y - mousePos.y), -89.f, 89.f);


		ImTextureID tex = reinterpret_cast<ImTextureID>(static_cast<intptr_t>(cam.textureID));

		// Drawing cam texture
		ImGui::GetWindowDrawList()->AddImage(
			tex,
			ImVec2(pos.x, pos.y),
			ImVec2(pos.x + ImGui::GetContentRegionAvail().x, pos.y + ImGui::GetContentRegionAvail().y),
			ImVec2(0, 1),
			ImVec2(1, 0)
		);

#pragma endregion

#pragma region ImGuizmos
		// ======= IMGUIZMO =======
		auto* drawlist = ImGui::GetWindowDrawList();

		ImGuizmo::SetDrawlist(drawlist);
		ImGuizmo::Enable(true);

		auto& set = mManager.GetRegistry().GetSelectionSystem().GetSelectedEntities();

		if (set.size() > 0)
		{
			// get cam view & perspective
			glm::mat4 V = glm::lookAt(cam_tr.position, cam_tr.position + forward, up);
			glm::mat4 P = glm::perspective(glm::radians(cam.pov), static_cast<float>(cam.width) / static_cast<float>(cam.height), cam.near, cam.far);

			// get entities & transform components
			auto entt = *set.begin();
			auto& tmp_tr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(entt);

			// set gizmo limits to window

			ImGuizmo::SetRect(scene_window_pos.x, scene_window_pos.y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

			// get transforms
			glm::mat4 world_tr = tmp_tr.transform; // already computed as parent * local
			glm::mat4 new_world_tr = world_tr;

			if (ImGuizmo::Manipulate(glm::value_ptr(V), glm::value_ptr(P),
				mManager.GetGizmoOperation(), mManager.GetGizmoMode(),
				glm::value_ptr(new_world_tr), NULL))
			{
				auto& reg = SliceEngine::Core::GetInstance()->GetRegistry();
				auto operation = mManager.GetGizmoOperation();

				// Convert world transform to local if we have a parent
				glm::mat4 new_local_tr;
				if (auto scene_graph = reg.try_get<SliceEngine::SceneGraph>(entt)) {
					auto parent_entity = scene_graph->neighbours[SliceEngine::SceneGraph::UP];
					if (parent_entity != entt::null && parent_entity != entt::entity{ 0 }) {
						auto& parent_tr = reg.get<SliceEngine::Transform>(parent_entity);
						glm::mat4 parent_world = parent_tr.transform;
						new_local_tr = glm::inverse(parent_world) * new_world_tr;
					}
					else {
						new_local_tr = new_world_tr; // root entity
					}
				}
				else {
					new_local_tr = new_world_tr;
				}

				// Extract all components from local transform
				glm::vec3 translation, rotation_radians, scale;
				glm::extractEulerAngleXYZ(new_local_tr, rotation_radians.x, rotation_radians.y, rotation_radians.z);
				translation = glm::vec3(new_local_tr[3]);
				scale.x = glm::length(glm::vec3(new_local_tr[0]));
				scale.y = glm::length(glm::vec3(new_local_tr[1]));
				scale.z = glm::length(glm::vec3(new_local_tr[2]));

				// Only update what changed
				if (operation == ImGuizmo::TRANSLATE) {
					tmp_tr.position = translation;
				}
				else if (operation == ImGuizmo::ROTATE) {
					tmp_tr.rotation = glm::degrees(rotation_radians);
				}
				else if (operation == ImGuizmo::SCALE) {
					tmp_tr.scale = scale;
				}
				// Don't touch the matrices - let transform system rebuild them
			}
		}


#pragma endregion


#pragma region Object Picking

		if (ImGui::IsWindowHovered())
		{
			if (!ImGuizmo::IsOver() || !ImGuizmo::IsUsingAny())
			{
				auto renderer = SliceEngine::Core::GetInstance()->GetRenderManager();
				renderer->SelectCamIDPick(camObj.GetEntity());
				unsigned int entt_id = renderer->ObjectPick(static_cast<int>(mouse_scaled_x), static_cast<int>(mouse_scaled_y));
				entt::entity selected_entity{ entt_id };

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					if (io.KeyCtrl)
					{
						mManager.SelectObject((entt::entity)entt_id);
					}

					else
					{
						if (selected_entity == entt::null || entt_id == 0)
						{
							mManager.ClearObject();
						}

						else
						{
							mManager.ClearObject();
							mManager.SelectObject((entt::entity)entt_id);
						}
					}
				}
			}
		}

#pragma endregion


		ImGui::End();

	}
}
