/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        SceneViewManager.h

 author:	  Chase Rodgrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the SceneViewManager class, which manages the data of the scene view window of the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef SCENE_VIEW_MANAGER_H
#define SCENE_VIEW_MANAGER_H

#include "../Core/IBaseManager.h"
#include "../WindowManager/ICreateWindow.h"
#include "../Selection/ISelectionListener.h"

namespace SliceEngine
{
	class RenderManager;
}

namespace SliceEditor
{
	class Registry;

	class SceneViewManager : public IBaseManager, public ICreateWindow
	{
		float mCameraSpeed = 0.01f;
		ImGuizmo::OPERATION mGuizmoOperation{ ImGuizmo::OPERATION::TRANSLATE };
		ImGuizmo::MODE mGuizmoMode{ ImGuizmo::MODE::LOCAL };
		bool isUsing = false;

		std::variant<glm::vec3, glm::quat> mGizmoStartTransformValue;

	public:

		SceneViewManager(Registry& reg) : IBaseManager(reg) {};
		~SceneViewManager() = default;

		void Init() override;

		std::unique_ptr<EditorWindow> CreateEditorWindow() override;

		float GetCameraSpeed();

		void ChangeCameraSpeed(float speed);

		void SetGizmoOperation(ImGuizmo::OPERATION op);
		void SetGizmoMode(ImGuizmo::MODE mode);
		void CheckGizmoStatus(bool isUse);
		void StartUsingGizmo(std::variant<glm::vec3, glm::quat> startValue);

		ImGuizmo::OPERATION GetGizmoOperation() const;
		ImGuizmo::MODE GetGizmoMode() const;
	};
}
#endif
