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
#include <Graphics/RenderManager.h>
#include <Graphics/CameraSystem.h>
#include <Graphics/TransformHelper.h>

#include "Core/Registry.h"
#include "Selection/SelectionManager.h"
#include "Session/SessionManager.h"
#include "WindowManager/WindowManager.h"
#include "History/HistoryManager.h"
#include "Inspector/ComponentPropertiesGUI.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace SliceEditor
{
	#pragma region GLM Function Helpers
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

	glm::mat4 ConvertToEulerMatrix(const glm::mat4& transform)
	{
		glm::vec3 scale, translation, skew;
		glm::quat rotationQuat;
		glm::vec4 perspective;

		// Decompose the matrix into translation, rotation (quaternion), scale, etc.
		glm::decompose(transform, scale, rotationQuat, translation, skew, perspective);

		// Convert the quaternion to Euler angles (in degrees)
		glm::vec3 euler = QuaternionToEuler(rotationQuat);

		// Normalize Euler angles (to prevent gimbal lock issues)
		euler = NormalizeEulerAngles(euler);

		// Recompose the matrix from the Euler angles
		glm::mat4 new_transform = glm::mat4(1.0f);  // Start with an identity matrix
		new_transform = glm::translate(new_transform, translation); // Apply translation
		new_transform = glm::rotate(new_transform, glm::radians(euler.x), glm::vec3(1.0f, 0.0f, 0.0f)); // Apply rotation for X
		new_transform = glm::rotate(new_transform, glm::radians(euler.y), glm::vec3(0.0f, 1.0f, 0.0f)); // Apply rotation for Y
		new_transform = glm::rotate(new_transform, glm::radians(euler.z), glm::vec3(0.0f, 0.0f, 1.0f)); // Apply rotation for Z
		new_transform = glm::scale(new_transform, scale); // Apply scale

		return new_transform;
	}
	#pragma endregion

	void SceneViewWindow::UpdateCam()
	{

	}

	void SceneViewWindow::Init()
	{
		auto mRender = SliceEngine::Core::GetInstance()->GetRenderManager();
		SliceEngine::GameObject go = mRender->CreateCamera();
		SliceEngine::GameObject prefabCamGO = mRender->CreatePrefabCam();
		auto& cam = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(go.GetEntity());
		auto& prefabCam = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Camera>(prefabCamGO.GetEntity());
		camObj = std::make_unique<SceneCamera>(go.GetEntity(), go, cam);
		camObj->camera.debugRenderToggles = SliceEngine::DEBUG_ALL_DEBUG;
		prefabCamObj = std::make_unique<SceneCamera>(prefabCamGO.GetEntity(), prefabCamGO, prefabCam);
	}

	void SceneViewWindow::Draw()
	{
		ImGui::Begin("Scene");

		auto& io = ImGui::GetIO();
		auto mSelection = mRegistry.GetManager<SelectionManager>("Selection");
		auto mSession = mRegistry.GetManager<SessionManager>("Session");

		// Draw Utility Bar
		ImGui::BeginGroup();
		if(!mSession->IsPrefabInspected())
		{
			if (ImGui::Button("Debug Options"))
			{
				ImGui::OpenPopup("Debug Lines");
			}
			//float height = ImGui::GetItemRectSize().y;
			DebugDrawTogglePopup();
			ImGui::SameLine();
		}
		ImGui::Text("Camera Speed: ");
		ImGui::SameLine();
		SliderFloatInput(mRegistry, "##sceneCamSpeed", mCameraSpeed, "%.3f", 0.f, 5.f);
		//ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); //Set Disabled for Click without changing how it looks
		//ImGui::Button(ss.str().c_str()); //Speed Display
		//ImGui::PopItemFlag(); //End of Set Disabled
		//Debug Drawing Settings:
		ImGui::EndGroup();


#pragma region IO Calculation

		//auto size = ImGui::GetContentRegionAvail();
		ImVec2 pos = ImGui::GetCursorScreenPos();

		auto* cam_tr = &SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(camObj->entity);
		if (mSession->IsPrefabInspected())
		{
			cam_tr = &SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(prefabCamObj->entity);
		}

		ImVec2 window_pos = ImGui::GetWindowPos();
		ImVec2 window_size = ImGui::GetWindowSize(); // I actually dk what this is, I'm guessing the Whole region available for my scene to draw??

		int screen_width, screen_height;
		GLFWwindow* mWindow = SliceEngine::Core::GetInstance()->GetWindow();
		glfwGetWindowSize(mWindow, &screen_width, &screen_height); // When in editor, refers to the whole window (including the spaces occupied by inspector window etc

		ImVec2 scene_window_pos = ImGui::GetCursorScreenPos();
		ImVec2 scene_window_size = { window_size.x, window_size.y - (scene_window_pos.y - window_pos.y) };

		float mouse_relative_x = io.MousePos.x - scene_window_pos.x; // Correct, refers to the mouse position (in screen space), starting with (0,0) at the top left of the section you want
		float mouse_relative_y = io.MousePos.y - scene_window_pos.y;

		//float mouse_scaled_x = mouse_relative_x / window_size.x * screen_width; // Converts from idk coordinates, to relative to the whole window (not just scene part) coordinates
		float mouse_scaled_y = mouse_relative_y / window_size.y * screen_height;
		mouse_scaled_y = camObj->camera.height - mouse_scaled_y;

#pragma endregion

		glm::vec3 forward{}, right{}, up{};
		//camObj->camera.renderTag = SliceEngine::RENDER_TAG::DEBUG_ALL_DEBUG;

		if (!mSession->IsPrefabInspected())
		{
			SliceEngine::Core::GetInstance()->GetRenderManager()->GetCameraAxis(camObj->gameobject, forward, right, up);
		}
		else
		{
			SliceEngine::Core::GetInstance()->GetRenderManager()->GetCameraAxis(prefabCamObj->gameobject, forward, right, up);
		}

		if (ImGui::IsWindowFocused())
		{
			if (ImGui::IsKeyDown(ImGuiKey_F))
			{
				auto& selectedEntities = mSelection->GetSelectedNodes();
				if(!selectedEntities.empty())
				{
					auto& inspectedNode = *selectedEntities.begin();
					if (inspectedNode->type == SelectionType::ENTITY || inspectedNode->type == SelectionType::PREFAB_ENTITY)
					{
						SliceEngine::GameObject go = SliceEngine::FactoryInstance.GetGOByEntity(static_cast<EntityNode*>(inspectedNode)->entity);

						if (go.HasComponent<SliceEngine::Transform>())
						{
							auto targetTr = go.GetComponent<SliceEngine::Transform>();
							
							glm::vec3 camPos = targetTr.position + glm::vec3(-2.0f, 0.0f, 0.0f);

							cam_tr->position = camPos;
						}
					}
				}
			}

			if (io.KeyShift || ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				if (ImGui::IsKeyDown(ImGuiKey_W))
				{
					cam_tr->position += forward * mCameraSpeed;
				}

				if (ImGui::IsKeyDown(ImGuiKey_S))
				{
					cam_tr->position -= forward * mCameraSpeed;
				}

				if (ImGui::IsKeyDown(ImGuiKey_A))
				{
					cam_tr->position -= right * mCameraSpeed;
				}

				if (ImGui::IsKeyDown(ImGuiKey_D))
				{
					cam_tr->position += right * mCameraSpeed;
				}

				if (ImGui::IsKeyDown(ImGuiKey_Q))
				{
					cam_tr->position -= up * mCameraSpeed;
				}

				if (ImGui::IsKeyDown(ImGuiKey_E))
				{
					cam_tr->position += up * mCameraSpeed;
				}

				//Camera Speed Change
				if (io.MouseWheel > 0.0f)
				{
					mCameraSpeed += 0.01f;
				}
				else if (io.MouseWheel < 0.0f)
				{
					mCameraSpeed -= 0.01f;
				}

				mCameraSpeed = std::clamp(mCameraSpeed, 0.0f, 5.0f);
			}

			else
			{
				if (ImGui::IsKeyDown(ImGuiKey_W))
				{
					mGuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
				}

				if (ImGui::IsKeyDown(ImGuiKey_E))
				{
					mGuizmoOperation = ImGuizmo::OPERATION::ROTATE;
				}

				if (ImGui::IsKeyDown(ImGuiKey_R))
				{
					mGuizmoOperation = ImGuizmo::OPERATION::SCALE;
				}
			}


			//static ImVec2 rotate_anchor{};
			static bool isRotating = false;
			//static ImVec2 init_rot{};
			static ImVec2 lastMousePos{};
			static float cameraYaw = 0.0f;
			static float cameraPitch = 0.0f;

			if (ImGui::IsWindowHovered())
			{
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					//init_rot.x = cam_tr->rotation.y;
					//init_rot.y = cam_tr->rotation.z;
					lastMousePos = ImGui::GetMousePos();
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
					ImVec2 currMouse = ImGui::GetMousePos();
					ImVec2 mouse_diff(currMouse.x - lastMousePos.x, currMouse.y - lastMousePos.y);

					float sensitivity = 0.005f;
					cameraYaw -= mouse_diff.x * sensitivity;
					cameraPitch -= mouse_diff.y * sensitivity;

					cameraPitch = glm::clamp(cameraPitch, glm::radians(-89.0f), glm::radians(89.0f));

					glm::quat yawRotation = glm::angleAxis(cameraYaw, glm::vec3(0.0f, 1.0f, 0.0f));
					glm::quat pitchRotation = glm::angleAxis(cameraPitch, glm::vec3(0.0f, 0.0f, 1.0f));

					cam_tr->rotation = yawRotation * pitchRotation;

					lastMousePos = currMouse;
				}
			}

			// update 
		}

#pragma region Scene Drawing

		// Btw for rotation
		//camera.rotation.y -= (newMousePos.x - mousePos.x);
		//camera.rotation.z = std::clamp(camera.rotation.z - (newMousePos.y - mousePos.y), -89.f, 89.f);
		ImTextureID tex;
		if(!mSession->IsPrefabInspected())
		{
			 tex = static_cast<ImTextureID>(camObj->camera.textureID);
		}
		else
		{
			tex = static_cast<ImTextureID>(prefabCamObj->camera.textureID);
		}

		// win as in the scene Window
		ImVec2 winScreenTL{ pos.x, pos.y };
		ImVec2 winOffset{ 0.f,0.f };
		ImVec2 totalWinScreenDim = ImGui::GetContentRegionAvail();
		ImVec2 winScreenDim = totalWinScreenDim;
		float winScreenAR = winScreenDim.x / winScreenDim.y;
		// worldSpace as in what the camera renders (i'm going to change this val when I crop out parts of the image when streching the height)
		ImVec2 worldSpaceDim;
		if(!mSession->IsPrefabInspected())
		{
			worldSpaceDim = ImVec2{static_cast<float>(camObj->camera.width),static_cast<float>(camObj->camera.height)};
		}
		else
		{
			worldSpaceDim = ImVec2{static_cast<float>(prefabCamObj->camera.width),static_cast<float>(prefabCamObj->camera.height)};
		}
		float worldSpaceAR = worldSpaceDim.x / worldSpaceDim.y;
		float worldSpaceOffsetX{ 0.f };

		ImVec2 uvCropMin{ 0.f, 1.f }; // it's like this cuz it's flipped
		ImVec2 uvCropMax{ 1.f, 0.f };

		if (winScreenAR > worldSpaceAR) // Stretch the width
		{
			winScreenDim.x = totalWinScreenDim.y * worldSpaceAR;
			winOffset.x = (totalWinScreenDim.x - winScreenDim.x) / 2.f;
		}
		else if (winScreenAR < worldSpaceAR) // Stretch the height
		{
			//winScreenDim.y = totalWinScreenDim.x / worldSpaceAR;
			//winOffset.y = (totalWinScreenDim.y - winScreenDim.y) / 2.f;
			
			// world space: y & x dim stays the same
			float wouldBeWinWidth = totalWinScreenDim.y * worldSpaceAR;
			float percentScreenShown = totalWinScreenDim.x / wouldBeWinWidth;
			uvCropMin.x += 0.5f - 0.5f * percentScreenShown;
			uvCropMax.x -= 0.5f - 0.5f * percentScreenShown;

			worldSpaceOffsetX = uvCropMin.x * worldSpaceDim.x;
			worldSpaceDim.x *= percentScreenShown;
		}

		ImVec2 worldSpaceMouse{ (mouse_relative_x - winOffset.x) / winScreenDim.x * worldSpaceDim.x + worldSpaceOffsetX, worldSpaceDim.y - ((mouse_relative_y - winOffset.y) / winScreenDim.y * worldSpaceDim.y) };

		pos += winOffset;
		float scene_x = pos.x + winScreenDim.x; // Refers to the bottom right point of the scene window in screen space
		float scene_y = pos.y + winScreenDim.y;
		// Drawing cam texture
		ImGui::GetWindowDrawList()->AddImage(
			tex,
			ImVec2(pos.x, pos.y),
			ImVec2(scene_x, scene_y),
			uvCropMin,
			uvCropMax
		);


#pragma endregion

#pragma region Dropping Into the Scene Directly

		ImVec2 p0 = ImGui::GetCursorScreenPos();
		ImVec2 p1 = p0 + ImVec2(scene_x,scene_y);
		ImGuiID id = ImGui::GetCurrentWindow()->GetID("SceneCanvasPassive");
		ImRect rect(p0, p1);

		if (ImGui::BeginDragDropTargetCustom(rect,id))
		{
			if (ImGui::AcceptDragDropPayload("Model"))
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Model"))
				{
					SliceEngine::GUID recievedPayload(*(SliceEngine::GUID*)payload->Data);
					//EditorUtilities::GameObject_CreateModel(recievedPayload, entt::null, mRegistry.GetManager<HistoryManager>("History"));
				}
			}
			ImGui::EndDragDropTarget();
		}
	#pragma endregion

#pragma region ImGuizmos
		// ======= IMGUIZMO =======
		auto* drawlist = ImGui::GetWindowDrawList();

		ImGuizmo::SetDrawlist(drawlist);
		ImGuizmo::Enable(true);

		auto& set = mRegistry.GetManager<SelectionManager>("Selection")->GetSelectedNodes();

		if (!set.empty() && (set.begin().operator*()->type == SelectionType::ENTITY || set.begin().operator*()->type == SelectionType::PREFAB_ENTITY))
		{
			// get entities & transform components
			auto entity = static_cast<EntityNode*>(*set.begin())->entity;
			auto& tr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(entity);
			auto& sceneGraph = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(entity);
			auto parentEntity = sceneGraph.neighbours[sceneGraph.UP];

			// get cam view & perspective
			glm::mat4 V = glm::lookAt(cam_tr->position, cam_tr->position + forward, up);
			glm::mat4 P;

			if(!mSession->IsPrefabInspected())
			{
				P = glm::perspective(
					glm::radians(camObj->camera.pov), worldSpaceDim.x / worldSpaceDim.y, camObj->camera.near, camObj->camera.far);
			}
			else
			{
				P = glm::perspective(
					glm::radians(prefabCamObj->camera.pov), worldSpaceDim.x / worldSpaceDim.y, prefabCamObj->camera.near, prefabCamObj->camera.far);
			}


			// set gizmo limits to window
			ImGuizmo::SetRect(pos.x, pos.y, scene_x - pos.x, scene_y - pos.y);

			// get transforms
			glm::mat4 world_tr = tr.transform;
			//glm::mat4 new_world_tr = ConvertToEulerMatrix(world_tr);

			ImGuizmo::Manipulate(glm::value_ptr(V), glm::value_ptr(P), mGuizmoOperation, mGuizmoMode, glm::value_ptr(world_tr));

			if (ImGuizmo::IsUsing())
			{
				if (!mGizmoTracker.has_value())
				{
					switch (mGuizmoOperation)
					{
					case ImGuizmo::OPERATION::TRANSLATE:
						mGizmoTracker = GizmoUseTracker(mGuizmoOperation, tr.position);
						break;
					case ImGuizmo::OPERATION::ROTATE:
						mGizmoTracker = GizmoUseTracker(mGuizmoOperation, tr.eulerAnglesHint);
						break;
					case ImGuizmo::OPERATION::SCALE:
						mGizmoTracker = GizmoUseTracker(mGuizmoOperation, tr.scale);
						break;
					}
				}

				//auto& parentTr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(parentEntity);

				//// 1. Get the Inverse of the Parent World Matrix
				//glm::mat4 invParentMatrix = glm::inverse(parentTr.transform);

				//// 2. Transform the manipulated world_tr into local space
				//// This gives us the exact local matrix relative to the parent
				//glm::mat4 localMatrix = invParentMatrix * world_tr;

				// 3. Decompose the matrix

				glm::mat4 parentWorldTr{ 1 };
				glm::vec3 scale, euler, translation, skew;
				glm::vec4 persp;
				glm::quat rot;
				static glm::vec3 prev_euler{};

				if (parentEntity != entt::null)
				{
					/*auto& parentTr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(parentEntity);
					parentWorldTr = parentTr.transform;
					world_tr *= glm::inverse(parentWorldTr);*/

					auto& parentTr = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::Transform>(parentEntity);

					// 1. Get the Inverse of the Parent World Matrix
					glm::mat4 invParentMatrix = glm::inverse(parentTr.transform);

					// 2. Transform the manipulated world_tr into local space
					// This gives us the exact local matrix relative to the parent
					glm::mat4 localMatrix = invParentMatrix * world_tr;

					glm::decompose(localMatrix, scale, rot, translation, skew, persp);
				}
				else
				{
					glm::decompose(world_tr, scale, rot, translation, skew, persp);
				}

				if (mGuizmoOperation == ImGuizmo::OPERATION::TRANSLATE)
				{
					tr.position = translation;
					mGizmoTracker->endValue = translation;
				}
				
				if (mGuizmoOperation == ImGuizmo::OPERATION::ROTATE)
				{
					euler = SliceEngine::QuatToVec3(rot);
					tr.rotation = rot;
					tr.eulerAnglesHint = euler;
					mGizmoTracker->endValue = euler;
				}

				if (mGuizmoOperation == ImGuizmo::OPERATION::SCALE)
				{
					tr.scale = scale;
					mGizmoTracker->endValue = scale;
				}
			}

			else
			{
				if (mGizmoTracker.has_value())
				{
					switch (mGizmoTracker->operation)
					{
					case ImGuizmo::OPERATION::TRANSLATE:
						mRegistry.GetManager<HistoryManager>("History")->AddCommand(
							std::make_unique<ValueCommand<glm::vec3>>(tr.position,mGizmoTracker->startValue, mGizmoTracker->endValue));
						break;
					case ImGuizmo::OPERATION::ROTATE:
					{
						mRegistry.GetManager<HistoryManager>("History")->AddCommand(
							std::make_unique<FunctionSetsValueCommand<glm::vec3>>(mGizmoTracker->startValue, mGizmoTracker->endValue,
								[&](glm::vec3 newEuler)
								{
									tr.eulerAnglesHint = newEuler;
									tr.rotation = EulerToQuaternion(newEuler);
								}));
						}
						break;
					case ImGuizmo::OPERATION::SCALE:
						mRegistry.GetManager<HistoryManager>("History")->AddCommand(
							std::make_unique<ValueCommand<glm::vec3>>(tr.scale, mGizmoTracker->startValue, mGizmoTracker->endValue));
						break;
					}
					mGizmoTracker.reset();
				}
			}
		}


#pragma endregion


#pragma region Object Picking

		if (ImGui::IsWindowHovered())
		{
			

			if (!ImGuizmo::IsOver() || !ImGuizmo::IsUsingAny())
			{
				auto renderer = SliceEngine::Core::GetInstance()->GetRenderManager();
				if(!mSession->IsPrefabInspected())
				{
					renderer->SelectCamIDPick(camObj->entity);
				}
				else
				{
					renderer->SelectCamIDPick(prefabCamObj->entity);
				}
				unsigned int entt_id = renderer->ObjectPick(static_cast<int>(worldSpaceMouse.x), static_cast<int>(worldSpaceMouse.y));
				/*
				std::cout << std::setprecision(5) << std::setw(5) 
					<< "Mouse:[" << mouse_relative_x << ", " << mouse_relative_y
					//<< "], [" << ImGui::GetContentRegionAvail().x << ", " << ImGui::GetContentRegionAvail().y
					//<< "], [" << window_size.x << ", " << window_size.y 
					<< "], [" << worldSpaceMouse.x << ", " << worldSpaceMouse.y
					<< "] " << std::endl;
				*/
				
				entt::entity selectedEntity{ entt_id };

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					if (io.KeyCtrl)
					{
						mSelection->SelectSingleAdd(selectedEntity);
					}

					else
					{
						if (selectedEntity == entt::null || entt_id == 0)
						{

						}

						else
						{
							mSelection->SelectSingle(selectedEntity);
						}
					}
				}
			}
		}

#pragma endregion


		ImGui::End();

	}

	//Can put a version of this in ComponentPropertiesGUI
	void SceneViewWindow::MenuToggleBit(const char* label, unsigned char& mask, unsigned char bit)
	{
		bool checked = (mask & bit) != 0;

		if (ImGui::MenuItem(label, nullptr, checked))
		{
			if (checked)
				mask &= ~bit;
			else
				mask |= bit;
		}
	}

	void SceneViewWindow::DebugDrawTogglePopup()
	{
		if (ImGui::BeginPopupContextItem("Debug Lines"))
		{
			auto& tag = camObj->camera.debugRenderToggles;

			MenuToggleBit("Debug All", tag, SliceEngine::RENDER_TAG::DEBUG_ALL_DEBUG);
			ImGui::Separator();
			MenuToggleBit("Obj", tag, SliceEngine::RENDER_TAG::DEBUG_OBJ_TAG);
			MenuToggleBit("Frustum", tag, SliceEngine::RENDER_TAG::DEBUG_FRUSTRUM_TAG);
			MenuToggleBit("Grid", tag, SliceEngine::RENDER_TAG::DEBUG_GRID_TAG);
			MenuToggleBit("Navmesh", tag, SliceEngine::RENDER_TAG::DEBUG_NAVMESH_TAG);
			MenuToggleBit("Outline", tag, SliceEngine::RENDER_TAG::DEBUG_OUTLINE_SELECTED_TAG);
			MenuToggleBit("Draw Rays", tag, SliceEngine::RENDER_TAG::DEBUG_DRAW_RAY_TAG);
			
			ImGui::Text("Copy Main Cam");
			ImGui::SameLine(150.0f);
			if(ImGui::Button("##copy_mainCam", ImVec2(20, 0)))
				SliceEngine::Core::GetInstance()->GetRenderManager()->CopyMainCamSettings(camObj->camera);
			


			DragFloatInputHeader(mRegistry, "Translucent Cut", "##transDebug", camObj->camera.translucentSelectCutoff, "%.3f", 0.0f, 1.0f, 0.01f);
			DragFloatInputHeader(mRegistry, "Exposure", "##cam_exposure", camObj->camera.exposure, "%.1f", 0.1f, 50.0f);
			DragFloatInputHeader(mRegistry, "Gamma", "##cam_gamma", camObj->camera.gamma, "%.1f", 0.001f, 100.0f);
			DragFloatInputHeader(mRegistry, "White Cutoff", "##cam_white_cutoff", camObj->camera.whiteBalance, "%.1f", 0.001f, 100.0f);
			DragFloatInputHeader(mRegistry, "Min Luminance", "##cam_min_luminance", camObj->camera.minLuminance, "%.2f", 0.001f, FLT_MAX, 0.01f);
			DragFloatInputHeader(mRegistry, "Max Luminance", "##cam_max_luminance", camObj->camera.maxLuminance, "%.2f", 0.001f, FLT_MAX, 0.01f);
			DragFloatInputHeader(mRegistry, "Luminance Rate", "##cam_luminanceLearningRate", camObj->camera.luminanceLearningRate, "%.1f", 0.1f, 1000.0f);

			bool isBloom = camObj->camera.postRenderToggles & SliceEngine::RENDER_TAG::RENDER_BLOOM;
			ImGui::Text("Bloom");
			ImGui::SameLine(150.0f);
			if (ImGui::Checkbox("##cam_isBloom", &isBloom))
				SetBit(camObj->camera.postRenderToggles, SliceEngine::RENDER_TAG::RENDER_BLOOM, isBloom);
			if (isBloom)
			{
				DragFloatInputHeader(mRegistry, "Bloom Radius", "##cam_bloom_radius", camObj->camera.bloomFilterRadius, "%.f", 0.0f, FLT_MAX);
				DragFloatInputHeader(mRegistry, "Bloom Strength", "##cam_bloom_strength", camObj->camera.bloomStrength, "%.1f", 0.1f, FLT_MAX);
				DragFloatInputHeader(mRegistry, "Bloom Limit", "##cam_bloom_Limit", camObj->camera.bloomLimit, "%.1f", 0.1f, FLT_MAX);
			}

			ImGui::EndPopup();
		}
	}
}
