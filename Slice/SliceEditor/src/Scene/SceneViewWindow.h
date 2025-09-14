#ifndef EDITOR_SCENE_VIEW_H
#define EDITOR_SCENE_VIEW_H

#include "../WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class SceneViewWindow : public EditorWindow
	{
	public:

		SceneViewWindow();
		void Draw() override final;
	};
}

#endif