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
		void Draw() override final;
	};
}

#endif