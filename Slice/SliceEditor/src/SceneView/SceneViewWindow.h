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
		GLuint tex_id;

	public:

		SceneViewWindow(SceneViewManager& manager);
		~SceneViewWindow() = default;

		// TO DO: replace this with proper camera attaching
		void SetCameraTexture(GLuint texture_id);
		void Draw() override final;
	};
}

#endif