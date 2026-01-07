/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        SceneViewWindow.h

 author:	  Chase Rodgrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the SceneViewWindow class, which is responsible for drawing the Scene View window and its contents.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef EDITOR_SCENE_VIEW_H
#define EDITOR_SCENE_VIEW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class SceneViewManager;

	class SceneViewWindow : public EditorWindow
	{
		struct SceneCamera
		{
			entt::entity entity;
			SliceEngine::GameObject gameobject;
			SliceEngine::Camera& camera;
		};

		struct GizmoUseTracker
		{
			ImGuizmo::OPERATION operation;
			glm::vec3 startValue{};
			glm::vec3 endValue{};

			GizmoUseTracker(ImGuizmo::OPERATION op, glm::vec3 ref) : operation(op), startValue(ref) {}
		};

		std::optional<GizmoUseTracker> mGizmoTracker;

		float mCameraSpeed = 0.1f;
		ImGuizmo::OPERATION mGuizmoOperation{ ImGuizmo::OPERATION::TRANSLATE };
		ImGuizmo::MODE mGuizmoMode{ ImGuizmo::MODE::LOCAL };

		std::unique_ptr<SceneCamera> camObj;
		std::unique_ptr<SceneCamera> prefabCamObj;
		void UpdateCam();

	public:

		SceneViewWindow(Registry& reg) : EditorWindow(reg) {};
		~SceneViewWindow() = default;

		void Init() override;
		void Draw() override final;
		void MenuToggleBit(const char* label, unsigned char& mask, unsigned char bit);
		void DebugDrawTogglePopup();
	};
}

#endif