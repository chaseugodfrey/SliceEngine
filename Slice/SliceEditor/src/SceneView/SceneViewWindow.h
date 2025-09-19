#ifndef EDITOR_SCENE_VIEW_H
#define EDITOR_SCENE_VIEW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEngine
{

}

namespace SliceEditor
{
	class SceneViewManager;

	class SceneViewWindow : public EditorWindow
	{
		SceneViewManager& mManager;

	public:

		SceneViewWindow(SceneViewManager& manager);
		~SceneViewWindow() = default;

		// TO DO: replace this with proper camera attaching
		void SetCameraTexture(void* id);
		void Draw() override final;
	};
}

#endif