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
		float mCameraSpeed = 0.01f;
		ImGuizmo::OPERATION mGuizmoOperation{ ImGuizmo::OPERATION::TRANSLATE };
		ImGuizmo::MODE mGuizmoMode{ ImGuizmo::MODE::LOCAL };

		bool isUsing = false;

		std::variant<glm::vec3, glm::quat> mGizmoStartTransformValue;
		SliceEngine::GameObject camObj;

	public:

		SceneViewWindow(Registry& reg) : EditorWindow(reg) {};
		~SceneViewWindow() = default;

		void Draw() override final;
	};
}

#endif