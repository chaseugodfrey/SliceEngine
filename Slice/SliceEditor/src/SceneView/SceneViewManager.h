#ifndef SCENE_VIEW_MANAGER_H
#define SCENE_VIEW_MANAGER_H

#include "../Core/IBaseManager.h"
#include "../WindowManager/ICreateWindow.h"
#include "../SelectionSystem/ISelectionListener.h"

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

	public:

		SceneViewManager(Registry& reg) : IBaseManager(reg) {};
		~SceneViewManager() = default;

		void Init() override;

		std::unique_ptr<EditorWindow> CreateEditorWindow() override;

		float GetCameraSpeed();

		void ChangeCameraSpeed(float speed);

		void SetGizmoOperation(ImGuizmo::OPERATION op);
		void SetGizmoMode(ImGuizmo::MODE mode);
		ImGuizmo::OPERATION GetGizmoOperation() const;
		ImGuizmo::MODE GetGizmoMode() const;
	};
}
#endif
